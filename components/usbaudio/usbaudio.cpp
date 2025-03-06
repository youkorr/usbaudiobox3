#include "usbaudio.h"
#include "esphome/core/log.h"
#include "driver/usb_serial_jtag.h"
#include "tusb.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// Implémentation des méthodes manquantes
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
  // Vérification de la présence d'un périphérique USB
  if (!tud_ready()) {
    ESP_LOGD(TAG, "USB stack not ready");
    return false;
  }

  // Vérification des périphériques audio USB
  for (uint8_t i = 0; i < CFG_TUD_AUDIO; i++) {
    if (tud_audio_n_connected(i)) {
      ESP_LOGD(TAG, "USB audio device detected on interface %d", i);
      return true;
    }
  }

  ESP_LOGD(TAG, "No USB audio device detected");
  return false;
}

void USBAudioComponent::apply_audio_output_() {
  AudioOutputMode effective_mode = audio_output_mode_;
  
  if (effective_mode == AudioOutputMode::AUTO_SELECT) {
    effective_mode = usb_audio_connected_ ? AudioOutputMode::USB_HEADSET 
                                         : AudioOutputMode::INTERNAL_SPEAKERS;
  }

  // Application du mode de sortie audio
  switch (effective_mode) {
    case AudioOutputMode::INTERNAL_SPEAKERS:
      ESP_LOGD(TAG, "Switching to internal speakers");
      // Ajouter ici la logique pour activer les haut-parleurs internes
      break;
    case AudioOutputMode::USB_HEADSET:
      ESP_LOGD(TAG, "Switching to USB headset");
      // Ajouter ici la logique pour activer le casque USB
      break;
    default:
      ESP_LOGE(TAG, "Unknown audio output mode");
      break;
  }
}

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Setting up USB Audio component");
  // Initialisation de la pile USB
  if (!tusb_init()) {
    ESP_LOGE(TAG, "Failed to initialize USB stack");
    return;
  }

  usb_audio_connected_ = detect_usb_audio_device_();
  apply_audio_output_();
}

void USBAudioComponent::loop() {
  // Mise à jour de l'état USB
  tud_task();

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
  ESP_LOGCONFIG(TAG, "  USB Headset Connected: %s", 
               usb_audio_connected_ ? "YES" : "NO");
}

void USBAudioComponent::update_text_sensor() {
  if (text_sensor_ != nullptr) {
    text_sensor_->publish_state(usb_audio_connected_ ? "Connected" : "Disconnected");
  }
}

}  // namespace usbaudio
}  // namespace esphome


