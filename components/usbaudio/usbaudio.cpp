#include "usbaudio.h"
#include "esphome/core/log.h"
#include "usb/usb_host.h"
#include "driver/gpio.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initialisation du composant USB Audio");

  // Configuration des broches GPIO pour USB
  if (dp_pin_ != -1 && dm_pin_ != -1) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT_OUTPUT_OD;  // Mode open-drain pour USB
    io_conf.pin_bit_mask = (1ULL << dp_pin_) | (1ULL << dm_pin_);
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);

    ESP_LOGD(TAG, "Broches GPIO configurées pour USB : D+ (GPIO %d), D- (GPIO %d)", dp_pin_, dm_pin_);
  } else {
    ESP_LOGE(TAG, "Broches GPIO pour USB non configurées");
  }

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
  ESP_LOGCONFIG(TAG, "  Adaptateur USB connecté: %s", usb_audio_connected_ ? "OUI" : "NON");
}

void USBAudioComponent::update_text_sensor() {
  if (text_sensor_ != nullptr) {
    text_sensor_->publish_state(usb_audio_connected_ ? "Connecté" : "Déconnecté");
  }
}

}  // namespace usbaudio
}  // namespace esphome














