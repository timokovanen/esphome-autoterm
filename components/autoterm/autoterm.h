
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"

#ifdef USE_MQTT
#include "esphome/components/mqtt/mqtt_client.h"
#endif

#ifdef USE_SENSOR
#include "esphome/components/sensor/sensor.h"
#endif

#ifdef USE_TEXT_SENSOR
#include "esphome/components/text_sensor/text_sensor.h"
#endif

#ifdef USE_BINARY_SENSOR
#include "esphome/components/binary_sensor/binary_sensor.h"
#endif

#include "esphome/components/number/number.h"

#include "ventilation_switch.h"
#include "power_level_number.h"

#include <vector>
#include <string>

namespace esphome {
namespace autoterm {

class AUTOTerm : public Component {
 public:

  static const uint8_t PREAMBLE = 0xaa;
  static const uint8_t SENDER_PANEL = 0x03;
  static const uint8_t SENDER_HEATER = 0x04;
  static const uint8_t STATE_OFF = 0x00;
  static const uint8_t STATE_STARTING = 0x01;
  static const uint8_t STATE_RUNNING = 0x04;
  static const uint8_t STATE_SHUTTING_DOWN = 0x05;
  static const uint8_t STATE_TESTING = 0x06;
  static const uint8_t STATE_VENTILATION = 0x08;
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
  void set_publish_topic(const std::string &publish_topic) { publish_topic_ = publish_topic; }
  void set_subscribe_topic(const std::string &subscribe_topic) { subscribe_topic_ = subscribe_topic; }
  void set_refresh_ms(uint32_t refresh_ms) { refresh_ms_ = refresh_ms; }

  // --- sensor setters ---
  void set_heater_temperature_sensor(sensor::Sensor *sensor) { heater_temperature_sensor_ = sensor; }
  void set_panel_temperature_sensor(sensor::Sensor *sensor) { panel_temperature_sensor_ = sensor; }
  void set_external_temperature_sensor(sensor::Sensor *sensor) { external_temperature_sensor_ = sensor; }
  void set_battery_voltage_sensor(sensor::Sensor *sensor) { battery_voltage_sensor_ = sensor; }
  void set_temperature_setpoint_sensor(sensor::Sensor *sensor) { temperature_setpoint_sensor_ = sensor; }
  void set_operating_state_sensor(text_sensor::TextSensor *sensor) { operating_state_sensor_ = sensor; }
  void set_operating_mode_sensor(text_sensor::TextSensor *sensor) { operating_mode_sensor_ = sensor; }
  void set_ventilation_sensor(binary_sensor::BinarySensor *sensor) { ventilation_sensor_ = sensor; }
  void set_ventilation_switch(switch_::Switch *sw) { this->ventilation_switch_ = sw; }

  void set_power_level_number(number::Number *num) { this->power_level_number_ = num; }
  void apply_power_level(uint8_t level);  // called by the number child


  void setup() override;
  void loop() override;
  void dump_config() override;

 protected:
  // --- UARTs & configuration ---
  uart::UARTComponent *uart_panel_{nullptr};
  uart::UARTComponent *uart_heater_{nullptr};
  uint32_t timeout_ms_{20};
  size_t buffer_size_{256};
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
  sensor::Sensor *temperature_setpoint_sensor_{nullptr};
  text_sensor::TextSensor *operating_state_sensor_{nullptr};
  text_sensor::TextSensor *operating_mode_sensor_{nullptr};
  binary_sensor::BinarySensor *ventilation_sensor_{nullptr};
  switch_::Switch *ventilation_switch_{nullptr};

  number::Number *power_level_number_{nullptr};

  // --- helpers ---
  void read_from_(uart::UARTComponent *src, std::vector<uint8_t> &buf, uint32_t &last_rx);
  void maybe_flush_(std::vector<uint8_t> &buf, uart::UARTComponent *dst, uint32_t last_rx, const char *tag);
  void write_to_(uart::UARTComponent *dst, const std::vector<uint8_t> &buf, const char *tag);
#ifdef USE_MQTT
  void subscribe_mqtt_();
  void publish_mqtt_();
  void on_json_message_(JsonObject &root);
#endif
  void parse_message_(const std::vector<uint8_t> &buf);
  uint16_t crc16_modbus_(const uint8_t* data, size_t len);
  bool verify_crc16_modbus_(const std::vector<uint8_t> &buf);
  void update_sensors_();
  const char* state_to_string_(uint8_t state);
  const char* mode_to_string_(uint8_t mode);
  bool vent_to_binary_(uint8_t vent);
  static const char *const TAG;
};

}  // namespace autoterm
}  // namespace esphome
