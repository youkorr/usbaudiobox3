#include "usbaudio.h"
#include "esphome/core/log.h"
#include "driver/usb_serial_jtag.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

void USBAudioComponent::set_audio_output_mode(AudioOutputMode mode) {
  if (audio_output_mode_ != mode) {
    audio_output_mode_ = mode;
    apply_audio_output_();
  }
}

void USBAudioComponent::set_audio_output_mode(int mode) {
  AudioOutputMode new_mode = static_cast<AudioOutputMode>(mode);
  if (new_mode != audio_output_mode_) {
    audio_output_mode_ = new_mode;
    apply_audio_output_();
  }
}

bool USBAudioComponent::detect_usb_audio_device_() {
  // Utilisation de l'interface USB Serial JTAG pour détecter la présence USB
  uint8_t dtr, rts;
  esp_err_t err = usb_serial_jtag_get_line_state(&dtr, &rts);
  
  if (err != ESP_OK) {
    ESP_LOGD(TAG, "Erreur de détection USB: %s", esp_err_to_name(err));
    return false;
  }

  // Une simple détection basée sur l'état de la ligne
  bool detected = (dtr == 1 || rts == 1);
  ESP_LOGD(TAG, "État USB - DTR: %d, RTS: %d", dtr, rts);
  return detected;
}

void USBAudioComponent::apply_audio_output_() {
  AudioOutputMode effective_mode = audio_output_mode_;
  
  if (effective_mode == AudioOutputMode::AUTO_SELECT) {
    effective_mode = usb_audio_connected_ ? AudioOutputMode::USB_HEADSET 
                                         : AudioOutputMode::INTERNAL_SPEAKERS;
  }

  switch (effective_mode) {
    case AudioOutputMode::INTERNAL_SPEAKERS:
      ESP_LOGD(TAG, "Activation des haut-parleurs internes");
      // Ajouter ici la logique pour les haut-parleurs internes
      break;
    case AudioOutputMode::USB_HEADSET:
      ESP_LOGD(TAG, "Activation du casque USB");
      // Ajouter ici la logique pour le casque USB
      break;
    default:
      ESP_LOGE(TAG, "Mode audio inconnu");
      break;
  }
}

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initialisation du composant USB Audio");
  usb_audio_connected_ = detect_usb_audio_device_();
  apply_audio_output_();
}

void USBAudioComponent::loop() {
  bool current_state = detect_usb_audio_device_();
  if (current_state != usb_audio_connected_) {
    usb_audio_connected_ = current_state;
    apply_audio_output_();
    update_text_sensor();
  }
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "USB Audio:");
  ESP_LOGCONFIG(TAG, "  Mode: %d", static_cast<int>(audio_output_mode_));
  ESP_LOGCONFIG(TAG, "  Casque USB connecté: %s", 
               usb_audio_connected_ ? "OUI" : "NON");
}

void USBAudioComponent::update_text_sensor() {
  if (text_sensor_ != nullptr) {
    text_sensor_->publish_state(usb_audio_connected_ ? "Connecté" : "Déconnecté");
  }
}

}  // namespace usbaudio
}  // namespace esphome


