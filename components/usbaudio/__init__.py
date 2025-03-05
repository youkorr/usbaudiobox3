import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.const import CONF_ID, CONF_NAME
from esphome.components import i2s

DEPENDENCIES = ["esp32"]
AUTO_LOAD = ["i2s"]

CONF_AUDIO_OUTPUT_MODE = "audio_output_mode"

AUDIO_OUTPUT_MODES = {
    "internal_speakers": 0,
    "usb_headset": 1,
    "auto_select": 2,
}

usbaudio_ns = cg.esphome_ns.namespace('usbaudio')
USBAudioComponent = usbaudio_ns.class_('USBAudioComponent', cg.Component)

CONFIG_SCHEMA = cv.Schema({
    cv.GenerateID(): cv.declare_id(USBAudioComponent),
    cv.Required(CONF_NAME): cv.string,
    cv.Optional(CONF_AUDIO_OUTPUT_MODE, default="auto_select"): cv.enum(AUDIO_OUTPUT_MODES),
}).extend(cv.COMPONENT_SCHEMA).extend(i2s.i2s_device_schema())

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    
    # Set the audio output mode
    output_mode = AUDIO_OUTPUT_MODES[config[CONF_AUDIO_OUTPUT_MODE]]
    cg.add(var.set_audio_output_mode(output_mode))

    # Add ESP-IDF dependencies
    cg.add_library("esp_private/usb_host", None)
    cg.add_library("driver/i2s", None)

    # Setup I2S if needed
    await i2s.setup_i2s_device(var, config)

