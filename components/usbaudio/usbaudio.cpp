#include "usbaudio.h"
#include "esphome/core/log.h"
#include "esp_private/usb_host.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initialisation de l'Audio USB pour ESP32-S3 Box 3");
  initialize_internal_speakers_();
  initialize_usb_audio_();
}

void USBAudioComponent::loop() {
  handle_usb_audio_connection_();
}

void USBAudioComponent::initialize_internal_speakers_() {
  ESP_LOGD(TAG, "Configuration des haut-parleurs internes via I2S");

  i2s_config_t i2s_config = {
      .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_TX),
      .sample_rate = 44100,
      .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT,
      .channel_format = I2S_CHANNEL_FMT_RIGHT_LEFT,
      .communication_format = I2S_COMM_FORMAT_STAND_I2S,
      .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
      .dma_buf_count = 8,
      .dma_buf_len = 64,
      .use_apll = false,
      .tx_desc_auto_clear = true,
      .fixed_mclk = 0
  };

  i2s_pin_config_t pin_config = {
      .bck_io_num = GPIO_NUM_41,
      .ws_io_num = GPIO_NUM_42,
      .data_out_num = GPIO_NUM_40,
      .data_in_num = I2S_PIN_NO_CHANGE
  };

  ESP_ERROR_CHECK(i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL));
  ESP_ERROR_CHECK(i2s_set_pin(I2S_NUM_0, &pin_config));
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


