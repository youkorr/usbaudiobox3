#include "usbaudio.h"
#include "esphome/core/log.h"
#include "driver/usb_device.h"

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
  // Vérification si un périphérique USB est connecté via le port USB natif
  usb_device_status_t usb_status;
  esp_err_t err = usb_device_get_status(&usb_status);

  if (err != ESP_OK) {
    ESP_LOGD(TAG, "Erreur de détection USB: %s", esp_err_to_name(err));
    return false;
  }

  bool detected = (usb_status == USB_DEVICE_STATUS_POWERED) || (usb_status == USB_DEVICE_STATUS_CONFIGURED);
  ESP_LOGD(TAG, "État de détection USB: %s", detected ? "Détecté" : "Non détecté");

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
      // Ajouter la logique pour les haut-parleurs internes
      break;
    case AudioOutputMode::USB_HEADSET:
      ESP_LOGD(TAG, "Activation du casque USB");
      // Ajouter la logique pour rediriger l'audio vers USB
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
  static uint32_t last_check = 0;
  uint32_t now = millis();

  if (now - last_check > 500) {
    last_check = now;
    bool current_state = detect_usb_audio_device_();
    if (current_state != usb_audio_connected_) {
      usb_audio_connected_ = current_state;
      apply_audio_output_();
      update_text_sensor();
    }
  }
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "USB Audio:");
  ESP_LOGCONFIG(TAG, "  Mode: %d", static_cast<int>(audio_output_mode_));
  ESP_LOGCONFIG(TAG, "  Casque USB connecté: %s", usb_audio_connected_ ? "OUI" : "NON");
}

void USBAudioComponent::update_text_sensor() {
  if (text_sensor_ != nullptr) {
    text_sensor_->publish_state(usb_audio_connected_ ? "Connecté" : "Déconnecté");
  }
}

}  // namespace usbaudio
}  // namespace esphome




