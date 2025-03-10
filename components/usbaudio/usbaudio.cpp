#include "usbaudio.h"
#include "esphome/core/log.h"
#include "esphome/core/gpio.h"
#include "driver/gpio.h"

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
  if (dminus_pin_ == nullptr || dplus_pin_ == nullptr) {
    ESP_LOGE(TAG, "Broches D- et D+ non configurées");
    return false;
  }

  bool dminus_state = dminus_pin_->digital_read();
  bool dplus_state = dplus_pin_->digital_read();

  // Détection de la connexion USB
  return dminus_state && dplus_state;
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
      break;
    case AudioOutputMode::USB_HEADSET:
      ESP_LOGD(TAG, "Activation du casque USB");
      break;
    default:
      ESP_LOGE(TAG, "Mode audio inconnu");
      break;
  }
}

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initialisation du composant USB Audio");
  if (dminus_pin_ != nullptr && dplus_pin_ != nullptr) {
    dminus_pin_->setup();
    dplus_pin_->setup();
  }
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
    }
  }
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "USB Audio:");
  ESP_LOGCONFIG(TAG, "  Mode: %d", static_cast<int>(audio_output_mode_));
  ESP_LOGCONFIG(TAG, "  Casque USB connecté: %s", usb_audio_connected_ ? "OUI" : "NON");
  if (dminus_pin_ != nullptr && dplus_pin_ != nullptr) {
    ESP_LOGCONFIG(TAG, "  Broche D-:");
    dminus_pin_->dump_summary();
    ESP_LOGCONFIG(TAG, "  Broche D+:");
    dplus_pin_->dump_summary();


  }
}

}  // namespace usbaudio
}  // namespace esphome
















