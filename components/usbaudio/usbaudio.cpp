#include "usbaudio.h"
#include "esphome/core/log.h"
#include "esp_private/usb_host.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initialisation de l'Audio USB pour ESP32-S3 Box 3");
  initialize_usb_audio_();
}

void USBAudioComponent::loop() {
  handle_usb_audio_connection_();
}

void USBAudioComponent::initialize_usb_audio_() {
  ESP_LOGD(TAG, "Initialisation du mode USB Host pour l'audio");
  const usb_host_config_t host_config = {
    .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };
  ESP_ERROR_CHECK(usb_host_install(&host_config));
}

void USBAudioComponent::handle_usb_audio_connection_() {
  if (detect_usb_audio_device_()) {
    if (!usb_audio_connected_) {
      usb_audio_connected_ = true;
      if (audio_output_mode_ == AUTO_SELECT) {
        switch_audio_output_(USB_HEADSET);
      }
    }
  } else {
    if (usb_audio_connected_) {
      usb_audio_connected_ = false;
      if (audio_output_mode_ == AUTO_SELECT) {
        switch_audio_output_(INTERNAL_SPEAKERS);
      }
    }
  }
}

bool USBAudioComponent::detect_usb_audio_device_() {
  // This is a placeholder. You'll need to implement proper USB audio device detection.
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
      if (usb_audio_connected_) {
        switch_audio_output_(USB_HEADSET);
      } else {
        switch_audio_output_(INTERNAL_SPEAKERS);
      }
      break;
  }
}

void USBAudioComponent::set_audio_output_mode(AudioOutputMode mode) {
  this->audio_output_mode_ = mode;
  switch_audio_output_(mode);
}

void USBAudioComponent::play() {
  ESP_LOGD(TAG, "Lecture audio...");
  // TODO: Implement audio playback
}

void USBAudioComponent::stop() {
  ESP_LOGD(TAG, "Arrêt de la lecture audio...");
  // TODO: Implement audio stop
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "Configuration Audio USB:");
  ESP_LOGCONFIG(TAG, "  Mode audio: %d", this->audio_output_mode_);
  ESP_LOGCONFIG(TAG, "  Casque USB connecté: %s", usb_audio_connected_ ? "Oui" : "Non");
}

bool USBAudioComponent::is_usb_headset_connected() {
  return usb_audio_connected_;
}

}  // namespace usbaudio
}  // namespace esphome


