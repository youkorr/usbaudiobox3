#include "usbaudio.h"
#include "esphome/core/log.h"
#include "driver/gpio.h"
#include "usb/usb_host.h"
#include "usb/usb_types_ch9.h"
#include "usb/usb_types_stack.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// UAC Descriptor Types
#define CS_INTERFACE 0x24
#define HEADER 0x01
#define INPUT_TERMINAL 0x02
#define OUTPUT_TERMINAL 0x03

// USB Host variables
static usb_host_client_handle_t client_handle;
static usb_device_handle_t device_handle;
static bool device_connected = false;

// Callback for USB events
static void usb_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg) {
  USBAudioComponent *component = (USBAudioComponent *)arg;
  
  switch (event_msg->event) {
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
      if (usb_host_device_open(client_handle, event_msg->new_dev.address, &device_handle) == ESP_OK) {
        device_connected = true;
        component->on_usb_audio_connected();  // Use public method
      }
      break;
      
    case USB_HOST_CLIENT_EVENT_DEV_GONE:
      device_connected = false;
      component->on_usb_audio_disconnected();  // Use public method
      break;
  }
}

void USBAudioComponent::configure_gpio() {
  if (dminus_pin_ && dplus_pin_) {
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_INPUT;
    io_conf.pin_bit_mask = (1ULL << dminus_pin_->get_pin()) | (1ULL << dplus_pin_->get_pin());
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_config(&io_conf);
    ESP_LOGD(TAG, "Configured GPIO pins: D-=%d, D+=%d", dminus_pin_->get_pin(), dplus_pin_->get_pin());
  } else {
    ESP_LOGE(TAG, "GPIO pins not configured");
  }
}

bool USBAudioComponent::detect_usb_audio_device_() {
  if (!device_connected) return false;

  // Get device descriptor
  const usb_device_desc_t *device_desc;
  if (usb_host_get_device_descriptor(device_handle, &device_desc) != ESP_OK) {
    return false;
  }

  // Check for Audio Interface Class
  const usb_config_desc_t *config_desc;
  if (usb_host_get_active_config_descriptor(device_handle, &config_desc) == ESP_OK) {
    for (int i = 0; i < config_desc->bNumInterfaces; i++) {
      const usb_intf_desc_t *intf_desc = &config_desc->interface[i].altsetting[0];
      if (intf_desc->bInterfaceClass == USB_CLASS_AUDIO &&
          intf_desc->bInterfaceSubClass == 0x01) { // Audio Control
        return true;
      }
    }
  }
  return false;
}

void USBAudioComponent::on_usb_audio_connected() {
  bool current_state = detect_usb_audio_device_();
  if (current_state != usb_audio_connected_) {
    usb_audio_connected_ = current_state;
    apply_audio_output_();
    update_text_sensor();
  }
}

void USBAudioComponent::on_usb_audio_disconnected() {
  usb_audio_connected_ = false;
  apply_audio_output_();
  update_text_sensor();
}

void USBAudioComponent::apply_audio_output_() {
  if (audio_output_mode_ == AudioOutputMode::AUTO_SELECT) {
    if (usb_audio_connected_) {
      switch_to_usb_audio();
    } else {
      switch_to_speaker();
    }
  } else if (audio_output_mode_ == AudioOutputMode::USB_HEADSET) {
    switch_to_usb_audio();
  } else {
    switch_to_speaker();
  }
}

void USBAudioComponent::update_text_sensor() {
  if (text_sensor_) {
    if (usb_audio_connected_) {
      text_sensor_->publish_state("USB Headset Connected");
    } else {
      text_sensor_->publish_state("Internal Speaker");
    }
  }
}

void USBAudioComponent::switch_to_usb_audio() {
  ESP_LOGD(TAG, "Switching to USB audio output");
  // Configure USB audio endpoints
  // Implement actual audio routing using ESP-IDF APIs
}

void USBAudioComponent::switch_to_speaker() {
  ESP_LOGD(TAG, "Switching to internal speaker");
  // Configure internal DAC/I2S output
  // Implement actual audio routing using ESP-IDF APIs
}

void USBAudioComponent::setup() {
  // Configure GPIO first
  configure_gpio();

  // Initialize USB Host
  usb_host_config_t host_config = {
    .intr_flags = ESP_INTR_FLAG_LEVEL1
  };
  
  if (usb_host_install(&host_config) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to install USB host");
    return;
  }

  // Register client
  usb_host_client_config_t client_config = {
    .is_synchronous = false,
    .max_num_event_msg = 5,
    .async = {
      .client_event_callback = usb_event_callback,
      .callback_arg = this
    }
  };
  
  if (usb_host_client_register(&client_config, &client_handle) != ESP_OK) {
    ESP_LOGE(TAG, "Failed to register USB client");
    return;
  }

  ESP_LOGD(TAG, "USB Audio initialized");
}

void USBAudioComponent::loop() {
  // Handle USB events
  usb_host_client_handle_events(client_handle, portMAX_DELAY);
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "USB Audio:");
  ESP_LOGCONFIG(TAG, "  Audio Output Mode: %d", static_cast<int>(audio_output_mode_));
  ESP_LOGCONFIG(TAG, "  D- Pin: %d", dminus_pin_ ? dminus_pin_->get_pin() : -1);
  ESP_LOGCONFIG(TAG, "  D+ Pin: %d", dplus_pin_ ? dplus_pin_->get_pin() : -1);
}

}  // namespace usbaudio
}  // namespace esphome
















