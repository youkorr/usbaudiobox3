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
    if (audio_output_mode_ == AUTO_SELECT) {
      switch_audio_output_(usb_audio_connected_ ? USB_HEADSET : INTERNAL_SPEAKERS);
    }
  }
}

bool USBAudioComponent::detect_usb_audio_device_() {
  // Cette implémentation est un exemple et devra être adaptée à votre matériel spécifique
  return false;
}

void USBAudioComponent::switch_audio_output_(AudioOutputMode mode) {
  ESP_LOGD(TAG, "Changement de sortie audio : %d", mode);
  switch (mode) {
    case INTERNAL_SPEAKERS:
      ESP_LOGD(TAG, "Sortie audio : Haut-parleurs internes");
      break;
    case USB_HEADSET:
      ESP_LOGD(TAG, "Sortie audio : Casque USB");
      break;
    case AUTO_SELECT:
      switch_audio_output_(usb_audio_connected_ ? USB_HEADSET : INTERNAL_SPEAKERS);
      break;
  }
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Configuration Audio USB:");
  ESP_LOGCONFIG(TAG, "  Mode audio: %d", this->audio_output_mode_);
  ESP_LOGCONFIG(TAG, "  Casque USB connecté: %s", usb_audio_connected_ ? "Oui" : "Non");
}

}  // namespace usbaudio
}  // namespace esphome



