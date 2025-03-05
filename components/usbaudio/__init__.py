import esphome.codegen as cg
import esphome.config_validation as cv
from esphome.components import usb_host, media_player
from esphome.const import CONF_ID, CONF_NAME

CODEOWNERS = ["@your_github_username"]
DEPENDENCIES = ["usb_host"]  # Changé de media_player à usb_host
AUTO_LOAD = ["media_player"]  # Mais on charge toujours media_player

CONF_AUDIO_OUTPUT_MODE = "audio_output_mode"
AUDIO_OUTPUT_MODES = {
    "internal_speakers": 0,
    "usb_headset": 1,
    "auto_select": 2
}

usbaudio_ns = cg.esphome_ns.namespace('usbaudio')
USBAudioComponent = usbaudio_ns.class_('USBAudioComponent', media_player.MediaPlayer, cg.Component)

# Création du schéma sous usb_host au lieu de media_player
CONFIG_SCHEMA = usb_host.USB_HOST_SCHEMA.extend({
    cv.GenerateID(): cv.declare_id(USBAudioComponent),
    cv.Required(CONF_NAME): cv.string,  # Ajout du nom requis
    cv.Optional(CONF_AUDIO_OUTPUT_MODE, default="auto_select"): cv.enum(AUDIO_OUTPUT_MODES),
}).extend(cv.COMPONENT_SCHEMA)

async def to_code(config):
    var = cg.new_Pvariable(config[CONF_ID])
    await cg.register_component(var, config)
    await media_player.register_media_player(var, config)  # On l'enregistre toujours comme media_player
    
    # Set audio output mode
    audio_output_mode = config.get(CONF_AUDIO_OUTPUT_MODE)
    cg.add(var.set_audio_output_mode(AUDIO_OUTPUT_MODES[audio_output_mode]))
