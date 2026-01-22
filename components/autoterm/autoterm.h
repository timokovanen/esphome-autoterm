
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#ifdef USE_MQTT
#include "esphome/components/mqtt/mqtt_client.h"
#endif

#include "esphome/components/sensor/sensor.h"
#include "esphome/components/text_sensor/text_sensor.h"

#include "autoterm_switch.h"
#include "autoterm_number.h"
#include "autoterm_select.h"

#include <vector>
#include <string>

namespace esphome {
namespace autoterm {

class AUTOTerm : public Component {
 public:

  static const uint8_t BUF_AUTOTERM_TO_HEATER_SIZE = 13;

  static const uint8_t MIN_POWER_LEVEL = 0;
  static const uint8_t MAX_POWER_LEVEL = 9;
  static const uint8_t MIN_TEMPERATURE_SETPOINT = 1;
  static const uint8_t MAX_TEMPERATURE_SETPOINT = 30;

  static const uint8_t PREAMBLE = 0xaa;
  static const uint8_t SENDER_PANEL = 0x03;
  static const uint8_t SENDER_HEATER = 0x04;
  static const uint8_t MODE_BY_HEATER = 0x01;
  static const uint8_t MODE_BY_PANEL = 0x02;
  static const uint8_t MODE_BY_EXTERNAL = 0x03;
  static const uint8_t MODE_BY_POWER = 0x04;
  static const uint8_t STATE_OFF = 0x00;
  static const uint8_t STATE_STARTING = 0x01;
  static const uint8_t STATE_RUNNING = 0x04;
  static const uint8_t STATE_SHUTTING_DOWN = 0x05;
  static const uint8_t STATE_TESTING = 0x06;
  static const uint8_t STATE_VENTILATION = 0x08;
  static const uint8_t VENTILATION_ON = 0x01;
  static const uint8_t VENTILATION_OFF = 0x02;
  static const uint8_t CMD_START = 0x01;
  static const uint8_t CMD_SET = 0x02;
  static const uint8_t CMD_SHUTDOWN = 0x03;
  static const uint8_t CMD_STATUS = 0x0f;
  static const uint8_t CMD_PANEL_TEMP_REPORT = 0x11;

  // --- setters ---
  void set_uart_panel(uart::UARTComponent *uart) { uart_panel_ = uart; }
  void set_uart_heater(uart::UARTComponent *uart) { uart_heater_ = uart; }
  void set_timeout_ms(uint32_t timeout_ms) { timeout_ms_ = timeout_ms; }
  void set_buffer_size(size_t buffer_size) { buffer_size_ = buffer_size; }
  void set_mqtt_json_enabled(bool enabled) { mqtt_json_enabled_ = enabled; }
  void set_publish_topic(const std::string &publish_topic) { publish_topic_ = publish_topic; }
  void set_subscribe_topic(const std::string &subscribe_topic) { subscribe_topic_ = subscribe_topic; }
  void set_refresh_ms(uint32_t refresh_ms) { refresh_ms_ = refresh_ms; }

  // --- sensor setters ---
  void set_heater_temperature_sensor(sensor::Sensor *sensor) { heater_temperature_sensor_ = sensor; }
  void set_panel_temperature_sensor(sensor::Sensor *sensor) { panel_temperature_sensor_ = sensor; }
  void set_external_temperature_sensor(sensor::Sensor *sensor) { external_temperature_sensor_ = sensor; }
  void set_battery_voltage_sensor(sensor::Sensor *sensor) { this->battery_voltage_sensor_ = sensor; }
  void set_operating_state_sensor(text_sensor::TextSensor *sensor) { operating_state_sensor_ = sensor; }

  // --- select setters --
  void set_operating_mode_select(select::Select *s) { this->operating_mode_select_ = s; }
  bool apply_operating_mode(const std::string &value);

  // --- switch setters --
  void set_ventilation_switch(switch_::Switch *sw) { this->ventilation_switch_ = sw; }
  void set_power_switch(switch_::Switch *sw) { this->power_switch_ = sw; }
  bool apply_ventilation(bool state);
  bool apply_power(bool state);

  // --- number setters --
  void set_temperature_setpoint_number(number::Number *num) { this->temperature_setpoint_number_ = num; }
  void set_power_level_number(number::Number *num) { this->power_level_number_ = num; }
  bool apply_temperature_setpoint(uint8_t temp_set);
  bool apply_power_level(uint8_t power);

  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  // --- UARTs & configuration ---
  uart::UARTComponent *uart_panel_{nullptr};
  uart::UARTComponent *uart_heater_{nullptr};
  uint32_t timeout_ms_{20};
  size_t buffer_size_{256};
  bool mqtt_json_enabled_{false};
  std::string publish_topic_;
  std::string subscribe_topic_;
  uint32_t refresh_ms_{30000};
#ifdef USE_MQTT
  bool subscribed_{false};
  bool was_connected_{false};
#endif

  // --- buffers & timing ---
  std::vector<uint8_t> buf_panel_to_heater_;
  std::vector<uint8_t> buf_heater_to_panel_;
  std::vector<uint8_t> buf_autoterm_to_heater_;
  uint32_t last_panel_rx_{0};
  uint32_t last_heater_rx_{0};
  uint32_t last_publish_{0};
  bool send_command_{false};

  // --- parsed state ---
  bool autoterm_data_changed_{false};
  uint8_t autoterm_operating_mode_{0}; // settings
  uint8_t autoterm_temperature_setpoint_{0}; // settings
  uint8_t autoterm_ventilation_{0}; // settings
  uint8_t autoterm_power_level_{0}; // settings
  uint16_t autoterm_battery_voltage_{0}; // status
  uint8_t autoterm_operating_state_{0}; // status
  int8_t autoterm_panel_temperature_{0}; // panel temp sensor
  int8_t autoterm_heater_temperature_{0}; // heater temp sensor
  int8_t autoterm_external_temperature_{0}; // external temp sensor

  // --- sensors ---
  sensor::Sensor *heater_temperature_sensor_{nullptr};
  sensor::Sensor *panel_temperature_sensor_{nullptr};
  sensor::Sensor *external_temperature_sensor_{nullptr};
  sensor::Sensor *battery_voltage_sensor_{nullptr};
  text_sensor::TextSensor *operating_state_sensor_{nullptr};
  select::Select *operating_mode_select_{nullptr};
  switch_::Switch *power_switch_{nullptr};
  switch_::Switch *ventilation_switch_{nullptr};
  number::Number *temperature_setpoint_number_{nullptr};
  number::Number *power_level_number_{nullptr};

  // --- helpers ---
  void read_from_(uart::UARTComponent *src, std::vector<uint8_t> &buf, uint32_t &last_rx);
  void maybe_flush_(std::vector<uint8_t> &buf, uart::UARTComponent *dst, uint32_t last_rx, const char *tag);
  void write_to_(uart::UARTComponent *dst, const std::vector<uint8_t> &buf, const char *tag);
#ifdef USE_MQTT
  void subscribe_mqtt_json_();
  void publish_mqtt_json_();
  void on_json_message_(JsonObject &root);
#endif
  void command_to_heater_(uint8_t cmd, uint8_t mode, uint8_t temp_set, uint8_t power, uint8_t vent);
  void parse_message_(const std::vector<uint8_t> &buf);
  uint16_t crc16_modbus_(const uint8_t* data, size_t len);
  bool verify_crc16_modbus_(const std::vector<uint8_t> &buf);
  void update_sensors_();
  const char* state_to_string_(uint8_t state);
  const char* mode_to_string_(uint8_t mode);
  static const char *const TAG;
};

}  // namespace autoterm
}  // namespace esphome
