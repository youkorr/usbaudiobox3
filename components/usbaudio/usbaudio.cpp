#include "usbaudio.h"
#include "esphome/core/log.h"
#include "driver/gpio.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// GPIO pour la détection du casque
const int HEADPHONE_DETECT_PIN = GPIO_NUM_19;  // Remplacez par le GPIO réel utilisé

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

bool USBAudioComponent::detect_headphone_() {
  if (HEADPHONE_DETECT_PIN == -1) {
    ESP_LOGE(TAG, "Aucun GPIO configuré pour la détection du casque");
    return false;
  }

  // Lire l'état du GPIO
  bool headphone_connected = gpio_get_level((gpio_num_t)HEADPHONE_DETECT_PIN) == 1;
  ESP_LOGD(TAG, "État du casque : %s", headphone_connected ? "Connecté" : "Déconnecté");
  return headphone_connected;
}

void USBAudioComponent::apply_audio_output_() {
  if (audio_output_mode_ != AudioOutputMode::AUTO_SELECT) {
    // Mode manuel
    switch (audio_output_mode_) {
      case AudioOutputMode::INTERNAL_SPEAKERS:
        ESP_LOGD(TAG, "Activation forcée des haut-parleurs internes");
        break;
      case AudioOutputMode::USB_HEADSET:
        ESP_LOGD(TAG, "Activation forcée du casque USB");
        break;
      default:
        break;
    }
    return;
  }

  // Mode automatique
  if (usb_audio_connected_) {
    ESP_LOGD(TAG, "Basculement vers le casque USB (mode automatique)");
  } else {
    ESP_LOGD(TAG, "Basculement vers les haut-parleurs internes (mode automatique)");
  }
}

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initialisation du composant USB Audio");

  // Configurer le GPIO pour la détection du casque
  if (HEADPHONE_DETECT_PIN != -1) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << HEADPHONE_DETECT_PIN);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    ESP_LOGD(TAG, "GPIO %d configuré pour la détection du casque", HEADPHONE_DETECT_PIN);
  } else {
    ESP_LOGE(TAG, "Aucun GPIO configuré pour la détection du casque");
  }

  // Détection initiale
  usb_audio_connected_ = detect_headphone_();
  apply_audio_output_();
}

void USBAudioComponent::loop() {
  static uint32_t last_check = 0;
  uint32_t now = millis();

  if (now - last_check > 500) {
    last_check = now;
    bool current_state = detect_headphone_();
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
  ESP_LOGCONFIG(TAG, "  Casque connecté: %s", usb_audio_connected_ ? "OUI" : "NON");
}

void USBAudioComponent::update_text_sensor() {
  if (text_sensor_ != nullptr) {
    text_sensor_->publish_state(usb_audio_connected_ ? "Connecté" : "Déconnecté");
  }
}

}  // namespace usbaudio
}  // namespace esphome













