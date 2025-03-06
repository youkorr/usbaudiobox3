#include "usbaudio.h"
#include "esphome/core/log.h"
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
  // Détection basée sur une broche GPIO spécifique à l'ESP32-S3-BOX-3
  const int detect_pin = 3;  // Broche GPIO3 pour la détection USB
  
  // Configuration de la broche en entrée avec résistance pull-down
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_INPUT;
  io_conf.pin_bit_mask = (1ULL << detect_pin);
  io_conf.pull_down_en = GPIO_PULLDOWN_ENABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  gpio_config(&io_conf);
  
  // Lecture de l'état de la broche
  bool detected = gpio_get_level(static_cast<gpio_num_t>(detect_pin)) == 1;
  
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
      // Logique pour les haut-parleurs internes
      break;
    case AudioOutputMode::USB_HEADSET:
      ESP_LOGD(TAG, "Activation du casque USB");
      // Logique pour le casque USB
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
  
  // Vérification périodique toutes les 500ms
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


