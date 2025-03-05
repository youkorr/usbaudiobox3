import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import usb_host
from esphome.const import CONF_ID, CONF_NAME

CODEOWNERS = ["@your_github_username"]
DEPENDENCIES = ["usb_host"]
AUTO_LOAD = ["usb_host"]

CONF_AUDIO_OUTPUT_MODE = "audio_output_mode"

AUDIO_OUTPUT_MODES = {
    "internal_speakers": 0,
    "usb_headset": 1,
    "auto_select": 2,
}

usbaudio_ns = cg.esphome_ns.namespace('usbaudio')
USBAudioComponent = usbaudio_ns.class_('USBAudioComponent', cg.Component)

CONFIG_SCHEMA = usb_host.USB_HOST_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(USBAudioComponent),
    cv.Required(CONF_NAME): cv.string,
    cv.Optional(CONF_AUDIO_OUTPUT_MODE, default="auto_select"): cv.enum(AUDIO_OUTPUT_MODES),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    cg.add(var.set_audio_output_mode(AUDIO_OUTPUT_MODES[config[CONF_AUDIO_OUTPUT_MODE]]))

