#include "usbaudio.h"
#include "esphome/core/log.h"
#include "usb/usb_host.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// Variables globales pour le client USB Host
static usb_host_client_handle_t client_hdl = nullptr;
static bool usb_host_initialized = false;

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
  if (!usb_host_initialized) {
    return false;
  }

  bool device_present = false;
  usb_device_handle_t dev_hdl;
  
  // Obtenir le handle du premier périphérique connecté
  if (usb_host_device_open(client_hdl, 0, &dev_hdl) == ESP_OK) {
    device_present = true;
    ESP_LOGD(TAG, "Périphérique USB détecté");
    usb_host_device_close(client_hdl, dev_hdl);
  }

  return device_present;
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

  // Initialisation du client USB Host
  usb_host_client_config_t client_config = {
      .is_synchronous = false,
      .max_num_event_msg = 5,
      .async = {
          .client_event_callback = nullptr,
          .callback_arg = nullptr,
      }};
  
  esp_err_t err = usb_host_client_register(&client_config, &client_hdl);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Erreur d'initialisation USB Host: %s", esp_err_to_name(err));
    usb_host_initialized = false;
  } else {
    usb_host_initialized = true;
  }

  // Détection initiale
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






