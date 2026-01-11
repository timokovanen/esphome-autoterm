
#pragma once

#include "esphome/core/component.h"
#include "esphome/components/uart/uart.h"
#ifdef USE_MQTT
#include "esphome/components/mqtt/mqtt_client.h"
#endif
#include <vector>
#include <string>

namespace esphome {
namespace autoterm {

class AUTOTerm : public Component {
 public:

  static const uint8_t PREAMPLE = 0xaa;
  static const uint8_t SENDER_PANEL = 0x03;
  static const uint8_t SENDER_HEATER = 0x04;
  static const uint8_t STATE_OFF = 0x00;
  static const uint8_t STATE_RUNNING = 0x04;
  static const uint8_t STATE_VENTILATION = 0x08;
  static const uint8_t CMD_START = 0x01;
  static const uint8_t CMD_SET = 0x02;
  static const uint8_t CMD_SHUTDOWN = 0x03;
  static const uint8_t CMD_STATUS = 0x0f;
  static const uint8_t CMD_PANEL_TEMP_REPORT = 0x11;

  // --- options ---
  void set_uart_panel(uart::UARTComponent *uart) { uart_panel_ = uart; }
  void set_uart_heater(uart::UARTComponent *uart) { uart_heater_ = uart; }
  void set_timeout_ms(uint32_t timeout_ms) { timeout_ms_ = timeout_ms; }
  void set_buffer_size(size_t buffer_size) { buffer_size_ = buffer_size; }
  void set_publish_topic(const std::string &publish_topic) { publish_topic_ = publish_topic; }
  void set_subscribe_topic(const std::string &subscribe_topic) { subscribe_topic_ = subscribe_topic; }
  void set_refresh_ms(uint32_t refresh_ms) { refresh_ms_ = refresh_ms; }

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
  uint32_t last_mqtt_pub_{0};
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
  static const char *const TAG;
};

}  // namespace autoterm
}  // namespace esphome
