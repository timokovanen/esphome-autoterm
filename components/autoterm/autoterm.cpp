
#include "autoterm.h"
#include "esphome/core/log.h"

namespace esphome {
namespace autoterm {

const char *const AUTOTerm::TAG = "autoterm";

void AUTOTerm::setup() {
  ESP_LOGCONFIG(TAG, "Setting up AUTOTerm...");
  buf_panel_to_heater_.reserve(buffer_size_);
  buf_heater_to_panel_.reserve(buffer_size_);
  buf_autoterm_to_heater_.reserve(13);
}

void AUTOTerm::read_from_(uart::UARTComponent *src, std::vector<uint8_t> &buf, uint32_t &last_rx) {
  if (src == nullptr) return;
  while (src->available() && buf.size() < buffer_size_) {
    uint8_t byte;
    if (src->read_byte(&byte)) {
      buf.push_back(byte);
      last_rx = millis();
    } else {
      break;
    }
  }
}

void AUTOTerm::write_to_(uart::UARTComponent *dst, const std::vector<uint8_t> &buf, const char *tag) {
  if (dst == nullptr || buf.empty()) return;
  dst->write_array(buf.data(), buf.size());
#ifdef ESPHOME_LOG_HAS_DEBUG
  std::string msg;
  msg.reserve(buf.size() * 2);
  for (auto b : buf) {
    char hex[3];
    snprintf(hex, sizeof(hex), "%02X", b);
    msg += hex;
  }
  ESP_LOGD(TAG, "%s: %s", tag, msg.c_str());
#endif
}

#ifdef USE_MQTT
void AUTOTerm::publish_mqtt_() {
  if (!mqtt::global_mqtt_client || !mqtt::global_mqtt_client->is_connected()) return;

  std::string payload = esphome::json::build_json([&] ( JsonObject root) {
    root["state"] = this->autoterm_operating_state_;
    root["mode"] = this->autoterm_operating_mode_;
    root["temp_set"] = this->autoterm_temperature_setpoint_;
    root["power"] = this->autoterm_power_level_;
    root["vent"] = this->autoterm_ventilation_;
    root["temp_heater"] = this->autoterm_heater_temperature_;
    root["temp_ext"] = this->autoterm_external_temperature_;
    root["temp_panel"] = this->autoterm_panel_temperature_;
    root["batt"] = (float)this->autoterm_battery_voltage_/10.0f;
  });

  mqtt::global_mqtt_client->publish(publish_topic_, payload);
}

void AUTOTerm::on_json_message_(JsonObject &root) {
  if (root["cmd"] == nullptr || !root["cmd"].is<int>() || (root["cmd"] < 0x01) || (root["cmd"] > 0x03)) {
    ESP_LOGD(TAG, "invalid command format");
    return;
  }
  uint8_t cmd = root["cmd"].as<uint8_t>();
  uint8_t mode = this->autoterm_operating_mode_;
  uint8_t temp_set = this->autoterm_temperature_setpoint_;
  uint8_t power = this->autoterm_power_level_;
  uint8_t vent = this->autoterm_ventilation_;

  if (root["mode"] != nullptr) {
    if (!root["mode"].is<int>() || (root["mode"] < 0x01) || (root["mode"] > 0x04)) return;
    mode = root["mode"].as<uint8_t>();
  }

  if (root["temp_set"] != nullptr) {
    if (!root["temp_set"].is<int>() || (root["temp_set"] < 0x01) || (root["temp_set"] > 0x1e)) return;
    temp_set = root["temp_set"].as<uint8_t>();
  }

  if (root["power"] != nullptr) {
    if (!root["power"].is<int>() || (root["power"] < 0x00) || (root["power"] > 0x09)) return;
    power = root["power"].as<uint8_t>();
  }

  if (root["vent"] != nullptr) {
    if (!root["vent"].is<int>() || (root["vent"] < 0x01) || (root["vent"] > 0x02)) return;
    vent = root["vent"].as<uint8_t>();
  }

  uint16_t crc;

  switch(cmd) {
    case CMD_START: // start
      if (this->autoterm_operating_state_ != STATE_OFF) return;
      ESP_LOGD(TAG, "command start");
      this->buf_autoterm_to_heater_.assign({ PREAMBLE, SENDER_PANEL, 0x06, 0x00, CMD_START, 0xff, 0xff, mode, temp_set, vent, power });
      crc = this->crc16_modbus_(buf_autoterm_to_heater_.data(), buf_autoterm_to_heater_.size());
      this->buf_autoterm_to_heater_.push_back(static_cast<uint8_t>(crc >> 8) & 0xff);
      this->buf_autoterm_to_heater_.push_back(static_cast<uint8_t>(crc & 0xff));
      this->send_command_ = true;
      break;
    case CMD_SET: // set settings
      ESP_LOGD(TAG, "command set");
      this->buf_autoterm_to_heater_.assign({ PREAMBLE, SENDER_PANEL, 0x06, 0x00, CMD_SET, 0xff, 0xff, mode, temp_set, vent, power });
      crc = this->crc16_modbus_(buf_autoterm_to_heater_.data(), buf_autoterm_to_heater_.size());
      this->buf_autoterm_to_heater_.push_back(static_cast<uint8_t>(crc >> 8) & 0xff);
      this->buf_autoterm_to_heater_.push_back(static_cast<uint8_t>(crc & 0xff));
      this->send_command_ = true;
      break;
    case CMD_SHUTDOWN: // shutdown
      if (this->autoterm_operating_state_ != STATE_RUNNING && this->autoterm_operating_state_ != STATE_VENTILATION) return;
      ESP_LOGD(TAG, "command shutdown");
      this->buf_autoterm_to_heater_.assign({ PREAMBLE, SENDER_PANEL, 0x00, 0x00, CMD_SHUTDOWN, 0x5d, 0x7c });
      this->send_command_ = true;
      break;
  }
}

void AUTOTerm::subscribe_mqtt_() {
  bool connected = mqtt::global_mqtt_client && mqtt::global_mqtt_client->is_connected();

  if (!this->was_connected_ && connected) {
    this->subscribed_ = false;  // force re-subscribe
  }
  this->was_connected_ = connected;

  if (connected && !this->subscribed_) {
    mqtt::global_mqtt_client->subscribe_json(
      this->subscribe_topic_,
      [this](const std::string &topic, JsonObject root) {
        this->on_json_message_(root);
      }, 0);     
    this->subscribed_ = true;
  }
}
#endif

void AUTOTerm::parse_message_(const std::vector<uint8_t> &buf) {
  if (buf.size() < 7) return; // minimum message size
  if (buf[1] == SENDER_HEATER) {
    // heater messages
    //    header
    // 00 PREAMBLE '0xaa'
    // 01 sender '0x03' = control panel, '0x04' = heater
    // 02 payload length
    // 03 '00'
    // 04 command
    switch(buf[4]) {
      case CMD_START:
      case CMD_SET:
        if (buf.size() < 13) return; // message size
        // command '0x01' start heater | '0x02' get/set settings response
        //    payload
        // 05 PREAMBLE???
        // 06 PREAMBLE???
        // 07 operating mode '0x01' = by heater temp sensor, '0x02' = by control panel temp sensor, '0x03' = by external temp sensor, '0x04' = by power
        // 08 temp setpoint in celsius 0x01 - 0x1e (1-30C)
        // 09 ventilation '0x01' = on, '0x02' = off
        // 10 power level 0x01 - 0x09
        //    crc
        // 11 crc
        // 12 crc
        if (this->autoterm_operating_mode_ != buf[7] ) { this->autoterm_operating_mode_ = buf[7]; this->autoterm_data_changed_ = true;}
        if (this->autoterm_temperature_setpoint_ != buf[8]) { this->autoterm_temperature_setpoint_ = buf[8]; this->autoterm_data_changed_ = true;}
        if (this->autoterm_ventilation_ != buf[9]) { this->autoterm_ventilation_ = buf[9]; this->autoterm_data_changed_ = true;}
        if (this->autoterm_power_level_ != buf[10]) { this->autoterm_power_level_ = buf[10]; this->autoterm_data_changed_ = true;}
        break;
      case CMD_STATUS:
        if (buf.size() < 17) return; // message size
        // command '0x0f' get status response
        //    payload
        // 05 -
        // 06 -
        // 07 -
        // 08 heater temperature sensor temp in celsius
        // 09 external temperature sensor temp in celsius
        // 10 battery voltage MSB
        // 11 battery voltage LSB
        // 12
        // 13 heater core temp ???? 
        // 14 operating mode
        //    crc
        // 15 crc
        // 16 crc
        this->autoterm_heater_temperature_ = buf[8]; // no forced mqtt refresh
        this->autoterm_external_temperature_ = buf[9]; // no forced mqtt refresh
        this->autoterm_battery_voltage_ = (static_cast<uint16_t>(buf[10]) << 8) | static_cast<uint16_t>(buf[11]); // no forced mqtt refresh
        if (this->autoterm_operating_state_ != buf[14]) { this->autoterm_operating_state_ = buf[14]; this->autoterm_data_changed_ = true;}
        break;
      case CMD_PANEL_TEMP_REPORT:
        if (buf.size() < 8) return; // message size
        // command '0x11' heater's response to control panel temperature report, payload one byte, temperature in °C
        //    payload
        // 05 control panel temperature sensor temp in celsius
        //    crc
        // 06 crc
        // 07 crc
        this->autoterm_panel_temperature_ = buf[5]; // no forced mqtt refresh
        break;
    }
  }
}

void AUTOTerm::maybe_flush_(std::vector<uint8_t> &buf, uart::UARTComponent *dst, uint32_t last_rx, const char *tag) {
  if (buf.empty()) return;
  uint32_t now = millis();
  bool buffer_full = buf.size() >= this->buffer_size_;
  bool timed_out = (uint32_t)(now - last_rx) >= this->timeout_ms_;

  if (buffer_full || timed_out) {
    if (verify_crc16_modbus_(buf)) {
        this->write_to_(dst, buf, tag);
        this->parse_message_(buf);
      } else {
        ESP_LOGD(TAG, "%s: malformed message", tag);
      }
    buf.clear();
  }
  
  if (timed_out && tag == "heater" && send_command_) {
    this->write_to_(uart_heater_, buf_autoterm_to_heater_, "autoterm");
    send_command_ = false;
  }
}

uint16_t AUTOTerm::crc16_modbus_(const uint8_t* data, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t i = 0; i < len; ++i) {
        crc ^= data[i];
        for (int b = 0; b < 8; ++b) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool AUTOTerm::verify_crc16_modbus_(const std::vector<uint8_t> &buf) {
    if (buf.size() < 2) return false;

    const size_t data_len = buf.size() - 2;
    const uint16_t computed = this->crc16_modbus_(buf.data(), data_len);

    const uint16_t received =
        static_cast<uint16_t>(buf[data_len + 1]) |
        (static_cast<uint16_t>(buf[data_len]) << 8);

    return computed == received;
}

const char* AUTOTerm::state_to_string_(uint8_t state) {
  switch(state) {
    case STATE_OFF: return "off";
    case STATE_STARTING: return "Starting";
    case STATE_RUNNING: return "Running";
    case STATE_SHUTTING_DOWN: return "Shutting down";
    case STATE_TESTING: return "Testing";
    case STATE_VENTILATION: return "Ventilation";
    default: return "Unknown";
  }
}

const char* AUTOTerm::mode_to_string_(uint8_t mode) {
  switch(mode) {
    case 0x01: return "Heater sensor";
    case 0x02: return "Panel sensor";
    case 0x03: return "External sensor";
    case 0x04: return "Power";
    default: return "Unknown";
  }
}

const char* AUTOTerm::vent_to_string_(uint8_t vent) {
  switch(vent) {
    case 0x01: return "on";
    case 0x02: return "off";
    default: return "Unknown";
  }
}

void AUTOTerm::update_sensors_() {
  if (heater_temperature_sensor_) {
    heater_temperature_sensor_->publish_state(autoterm_heater_temperature_);
  }
  if (panel_temperature_sensor_) {
    panel_temperature_sensor_->publish_state(autoterm_panel_temperature_);
  }
  if (external_temperature_sensor_) {
    external_temperature_sensor_->publish_state(autoterm_external_temperature_);
  }
  if (battery_voltage_sensor_) {
    battery_voltage_sensor_->publish_state((float)autoterm_battery_voltage_ / 10.0f);
  }
  if (temperature_setpoint_sensor_) {
    temperature_setpoint_sensor_->publish_state(autoterm_temperature_setpoint_);
  }
  if (power_level_sensor_) {
    power_level_sensor_->publish_state(autoterm_power_level_);
  }
  if (operating_state_sensor_) {
    operating_state_sensor_->publish_state(state_to_string_(autoterm_operating_state_));
  }
  if (operating_mode_sensor_) {
    operating_mode_sensor_->publish_state(mode_to_string_(autoterm_operating_mode_));
  }
  if (ventilation_sensor_) {
    ventilation_sensor_->publish_state(vent_to_string_(autoterm_ventilation_));
  }
}

void AUTOTerm::loop() {
  uint32_t now = millis();

  this->read_from_(uart_panel_, buf_panel_to_heater_, last_panel_rx_);
  this->read_from_(uart_heater_, buf_heater_to_panel_, last_heater_rx_);
  this->maybe_flush_(buf_panel_to_heater_, uart_heater_, last_panel_rx_, "panel");
  this->maybe_flush_(buf_heater_to_panel_, uart_panel_, last_heater_rx_, "heater");
#ifdef USE_MQTT
  this->subscribe_mqtt_();
#endif
  if(((uint32_t)(now - this->last_publish_) >= this->refresh_ms_) || this->autoterm_data_changed_) {
    this->update_sensors_();
#ifdef USE_MQTT
    this->publish_mqtt_();
#endif
    this->last_publish_ = millis();
    this->autoterm_data_changed_ = false;
  }
}

void AUTOTerm::dump_config(){
  ESP_LOGCONFIG(TAG, "AUTOTerm");
  ESP_LOGCONFIG(TAG, "  timeout_ms = %i", this->timeout_ms_);
  ESP_LOGCONFIG(TAG, "  buffer_size = %i", this->buffer_size_);
#ifdef USE_MQTT
  ESP_LOGCONFIG(TAG, "  publish_topic = %s", this->publish_topic_.c_str());
  ESP_LOGCONFIG(TAG, "  subscribe_topic = %s", this->subscribe_topic_.c_str());
  ESP_LOGCONFIG(TAG, "  refresh_ms = %i", this->refresh_ms_);
#endif
}

}  // namespace autoterm
}  // namespace esphome
