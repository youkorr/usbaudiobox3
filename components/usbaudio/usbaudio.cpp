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
  // Détection basée sur une broche GPIO
  // Vous devez connecter un signal USB_DETECT à une broche GPIO
  const int detect_pin = 4;  // Changer selon votre configuration matérielle
  
  // Configuration de la broche en entrée
  gpio_set_direction(static_cast<gpio_num_t>(detect_pin), GPIO_MODE_INPUT);
  
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


