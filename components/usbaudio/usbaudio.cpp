#include "usbaudio.h"
#include "esphome/core/log.h"
#include "driver/gpio.h"

// Inclure les en-têtes USB nécessaires
#ifdef USE_ESP_IDF
#include "esp_private/usb_phy.h"
#include "hal/usb_hal.h"
#endif

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

// Méthode améliorée pour détecter la présence USB
bool USBAudioComponent::detect_usb_audio_device_() {
  bool vbus_present = false;
  
#ifdef USE_ESP_IDF
  // Utiliser l'API USB de l'ESP-IDF si disponible
  usb_phy_handle_t phy_hdl;
  usb_phy_config_t phy_config = USB_PHY_CONFIG_DEFAULT();
  
  if (usb_new_phy(&phy_config, &phy_hdl) == ESP_OK) {
    // Vérifier le statut VBUS via l'API USB
    vbus_present = usb_phy_check_vbus_valid(phy_hdl);
    // Libérer les ressources
    usb_del_phy(phy_hdl);
  } else {
    // Fallback: utiliser GPIO19 si l'API USB échoue
    gpio_config_t io_conf = {};
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << GPIO_NUM_19);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    
    vbus_present = gpio_get_level(GPIO_NUM_19);
  }
#else
  // Fallback pour les versions Arduino
  gpio_set_direction(GPIO_NUM_19, GPIO_MODE_INPUT);
  vbus_present = gpio_get_level(GPIO_NUM_19);
#endif

  ESP_LOGD(TAG, "Détection USB VBUS: %s", vbus_present ? "Présent" : "Absent");
  return vbus_present;
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






