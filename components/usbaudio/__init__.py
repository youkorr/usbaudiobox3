import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import media_player

CODEOWNERS = ["@your_github_username"]
DEPENDENCIES = ["media_player"]
MULTI_CONF = True

CONF_AUDIO_OUTPUT_MODE = "audio_output_mode"
AUDIO_OUTPUT_MODES = {
    "INTERNAL_SPEAKERS": 0,
    "USB_HEADSET": 1,
    "AUTO_SELECT": 2
}

usbaudio_ns = cg.esphome_ns.namespace('usbaudio')
USBAudioComponent = usbaudio_ns.class_('USBAudioComponent', media_player.MediaPlayer, cg.Component)

CONFIG_SCHEMA = media_player.MEDIA_PLAYER_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(USBAudioComponent),
    cv.Optional(CONF_AUDIO_OUTPUT_MODE, default="AUTO_SELECT"): cv.enum(AUDIO_OUTPUT_MODES, lower=True),
})

def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    yield cg.register_component(var, config)
    yield media_player.register_media_player(var, config)
    
    # Set audio output mode
    audio_output_mode = config.get(CONF_AUDIO_OUTPUT_MODE)
    cg.add(var.set_audio_output_mode(usbaudio_ns.AudioOutputMode[audio_output_mode.upper()]))
