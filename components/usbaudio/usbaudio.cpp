#include "usbaudio.h"
#include "esphome/core/log.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initialisation de l'Audio USB pour ESP32-S3 Box 3");
}

void USBAudioComponent::loop() {
  handle_usb_audio_connection_();
}

void USBAudioComponent::handle_usb_audio_connection_() {
  bool new_usb_audio_state = detect_usb_audio_device_();
  if (new_usb_audio_state != usb_audio_connected_) {
    usb_audio_connected_ = new_usb_audio_state;
    apply_audio_output_();
  }
}

bool USBAudioComponent::detect_usb_audio_device_() {
  // Cette fonction devrait implémenter la logique de détection du périphérique USB
  // Pour l'instant, nous retournons une valeur fixe
  return false;
}

void USBAudioComponent::set_audio_output_mode(AudioOutputMode mode) {
  audio_output_mode_ = mode;
  apply_audio_output_();
}

void USBAudioComponent::set_audio_output_mode(int mode) {
  if (mode >= 0 && mode <= 2) {
    audio_output_mode_ = static_cast<AudioOutputMode>(mode);
    apply_audio_output_();
  } else {
    ESP_LOGE(TAG, "Mode audio invalide: %d", mode);
  }
}

void USBAudioComponent::apply_audio_output_() {
  AudioOutputMode effective_mode = audio_output_mode_;
  if (effective_mode == AudioOutputMode::AUTO_SELECT) {
    effective_mode = usb_audio_connected_ ? AudioOutputMode::USB_HEADSET : AudioOutputMode::INTERNAL_SPEAKERS;
  }

  ESP_LOGD(TAG, "Applying audio output: %d", static_cast<int>(effective_mode));
  switch (effective_mode) {
    case AudioOutputMode::INTERNAL_SPEAKERS:
      ESP_LOGD(TAG, "Sortie audio : Haut-parleurs internes");
      // Ajoutez ici le code pour basculer vers les haut-parleurs internes
      break;
    case AudioOutputMode::USB_HEADSET:
      ESP_LOGD(TAG, "Sortie audio : Casque USB");
      // Ajoutez ici le code pour basculer vers le casque USB
      break;
    default:
      ESP_LOGE(TAG, "Mode audio invalide");
      break;
  }
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Configuration Audio USB:");
  ESP_LOGCONFIG(TAG, "  Mode audio: %d", static_cast<int>(this->audio_output_mode_));
  ESP_LOGCONFIG(TAG, "  Casque USB connecté: %s", usb_audio_connected_ ? "Oui" : "Non");
}

}  // namespace usbaudio
}  // namespace esphome



