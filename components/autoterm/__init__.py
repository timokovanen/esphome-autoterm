import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import uart
from esphome.const import CONF_ID
from esphome.core import CORE

autoterm_ns = cg.esphome_ns.namespace('autoterm')
AUTOTerm = autoterm_ns.class_('AUTOTerm', cg.Component)

CONF_UART_PANEL = "uart_panel"
CONF_UART_HEATER = "uart_heater"
CONF_TIMEOUT_MS = "timeout_ms"
CONF_BUFFER_SIZE = "buffer_size"
CONF_PUBLISH_TOPIC = "publish_topic"
CONF_SUBSCRIBE_TOPIC = "subscribe_topic"
CONF_REFRESH_MS = "refresh_ms"

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(AUTOTerm),
    cv.Required(CONF_UART_PANEL): cv.use_id(uart.UARTComponent),
    cv.Required(CONF_UART_HEATER): cv.use_id(uart.UARTComponent),
    cv.Optional(CONF_TIMEOUT_MS, default=20): cv.int_range(min=1, max=2000),
    cv.Optional(CONF_BUFFER_SIZE, default=256): cv.int_range(min=16, max=4096),
    cv.Optional(CONF_PUBLISH_TOPIC): cv.string,
    cv.Optional(CONF_SUBSCRIBE_TOPIC): cv.string,
    cv.Optional(CONF_REFRESH_MS, default=30000): cv.int_range(min=5000, max=60000),
})

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    uart_panel = await cg.get_variable(config[CONF_UART_PANEL])
    uart_heater = await cg.get_variable(config[CONF_UART_HEATER])
    cg.add(var.set_uart_panel(uart_panel))
    cg.add(var.set_uart_heater(uart_heater))
    cg.add(var.set_timeout_ms(config[CONF_TIMEOUT_MS]))
    cg.add(var.set_buffer_size(config[CONF_BUFFER_SIZE]))
    publish_topic = config.get(CONF_PUBLISH_TOPIC, f"{CORE.name}/autoterm")
    cg.add(var.set_publish_topic(cg.std_string(publish_topic)))
    subscribe_topic = config.get(CONF_SUBSCRIBE_TOPIC, f"{CORE.name}/autoterm/cmd")
    cg.add(var.set_subscribe_topic(cg.std_string(subscribe_topic)))
    cg.add(var.set_refresh_ms(config[CONF_REFRESH_MS]))
