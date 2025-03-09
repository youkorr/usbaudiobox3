#include "usbaudio.h"
#include "esphome/core/log.h"
#include "usb/usb_host.h"
#include "driver/gpio.h"  // Required for GPIO control
#include "esp_timer.h"     // Required for esp_timer_get_time()
#include "freertos/FreeRTOS.h" // Required for FreeRTOS functions
#include "freertos/task.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// Global variables for the USB Host client
static usb_host_client_handle_t client_hdl = nullptr;
static bool usb_host_initialized = false;

// Configuration for internal speaker GPIO (adjust to your specific GPIO)
#ifdef USE_ESP32S3_BOX_3  // Only include if it's the ESP32-S3-BOX-3
static const int INTERNAL_SPEAKER_ENABLE_GPIO = 38; // Example GPIO, check your board
#endif

// USB Descriptor Types and Classes
#define USB_DESCRIPTOR_TYPE_INTERFACE 0x04
#define USB_CLASS_AUDIO 0x01

// Callback for USB events
static void usb_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg) {
  USBAudioComponent *component = static_cast<USBAudioComponent *>(arg);
  switch (event_msg->event) {
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
      ESP_LOGD(TAG, "New USB device detected");
      component->handle_device_connection();
      break;
    case USB_HOST_CLIENT_EVENT_DEV_GONE:
      ESP_LOGD(TAG, "USB device disconnected");
      component->handle_device_disconnection();
      break;
    default:
      break;
  }
}

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
    ESP_LOGD(TAG, "USB Host not initialized, cannot detect devices");
    return false;
  }

  bool audio_device_present = false;
  usb_device_handle_t dev_hdl = nullptr; // Initialize to nullptr

  // Get the handle of the first connected device
  esp_err_t err = usb_host_device_open(client_hdl, 0, &dev_hdl);
  if (err != ESP_OK) {
    ESP_LOGD(TAG, "No USB device detected: %s", esp_err_to_name(err));
    return false;
  }

  // Get the device descriptor
  const usb_device_desc_t *dev_desc;
  err = usb_host_get_device_descriptor(dev_hdl, &dev_desc);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Error getting device descriptor: %s", esp_err_to_name(err));
    usb_host_device_close(client_hdl, dev_hdl);
    return false;
  }

    // If not directly identified as Audio Device class, check interfaces within configurations
    const usb_config_desc_t *config_desc = nullptr;
    if (usb_host_get_active_config_descriptor(dev_hdl, &config_desc) == ESP_OK) {
        ESP_LOGD(TAG, "Active configuration descriptor obtained");
        uint8_t curr_idx = 0;
        const uint8_t *ptr = config_desc->val;

        while (curr_idx < config_desc->wTotalLength) {
            uint8_t desc_len = ptr[curr_idx];
            uint8_t desc_type = ptr[curr_idx + 1];

            if (desc_type == USB_DESCRIPTOR_TYPE_INTERFACE && desc_len >= 9) {
                uint8_t intf_class = ptr[curr_idx + 5];
                uint8_t intf_subclass = ptr[curr_idx + 6];
                uint8_t intf_protocol = ptr[curr_idx + 7];
                ESP_LOGD(TAG, "Interface descriptor found: Class=0x%02X, SubClass=0x%02X, Protocol=0x%02X", intf_class, intf_subclass, intf_protocol);

                if (intf_class == USB_CLASS_AUDIO) {
                    ESP_LOGI(TAG, "Audio interface detected (Class: 0x%02X)", USB_CLASS_AUDIO);
                    audio_device_present = true;
                    break;
                }
            }

            curr_idx += desc_len;
            if (desc_len == 0) {
              ESP_LOGE(TAG, "Invalid descriptor length, exiting loop");
              break;
            }
        }
    } else {
        ESP_LOGW(TAG, "Failed to get active config descriptor");
    }
  
  // Release resources
  usb_host_device_close(client_hdl, dev_hdl);
  
  if (!audio_device_present) {
    ESP_LOGD(TAG, "USB device detected, but not an audio device");
  }

  return audio_device_present;
}

void USBAudioComponent::handle_device_connection() {
  bool was_connected = usb_audio_connected_;
  usb_audio_connected_ = detect_usb_audio_device_();
  if (usb_audio_connected_ && !was_connected) {
    ESP_LOGI(TAG, "USB audio device connected");
    apply_audio_output_();
    update_text_sensor();
  }
}

void USBAudioComponent::handle_device_disconnection() {
  bool was_connected = usb_audio_connected_;
  usb_audio_connected_ = false;  // Assume disconnected
  if (was_connected) {
    ESP_LOGI(TAG, "USB audio device disconnected");
    apply_audio_output_();
    update_text_sensor();
  }
}

void USBAudioComponent::apply_audio_output_() {
  if (audio_output_mode_ != AudioOutputMode::AUTO_SELECT) {
    // Manual mode
    switch (audio_output_mode_) {
      case AudioOutputMode::INTERNAL_SPEAKERS:
        ESP_LOGD(TAG, "Forcing internal speakers");
        enable_internal_speaker_(true);
        break;
      case AudioOutputMode::USB_HEADSET:
        ESP_LOGD(TAG, "Forcing USB headset");
        enable_internal_speaker_(false);
        break;
      default:
        break;
    }
    return;
  }

  // Auto mode
  if (usb_audio_connected_) {
    ESP_LOGD(TAG, "Switching to USB headset (auto mode)");
    enable_internal_speaker_(false);
  } else {
    ESP_LOGD(TAG, "Switching to internal speakers (auto mode)");
    enable_internal_speaker_(true);
  }
}

void USBAudioComponent::enable_internal_speaker_(bool enable) {
#ifdef USE_ESP32S3_BOX_3
  ESP_LOGD(TAG, "Setting internal speaker to %s", enable ? "enabled" : "disabled");
  gpio_set_level((gpio_num_t)INTERNAL_SPEAKER_ENABLE_GPIO, enable ? 1 : 0); // HIGH enables the speaker (adjust as needed)
#else
  ESP_LOGW(TAG, "Internal speaker control is only supported on ESP32-S3-BOX-3. Define USE_ESP32S3_BOX_3 in your configuration.");
#endif
}


void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initializing USB Audio component");

  // Initialize the USB Host subsystem if not already done
  const usb_host_config_t host_config = {
      .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };

  esp_err_t err = usb_host_install(&host_config);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    // ESP_ERR_INVALID_STATE can occur if USB Host is already installed
    ESP_LOGE(TAG, "Error installing USB Host: %s", esp_err_to_name(err));
    usb_host_initialized = false;
    return;
  }

  // Initialize the USB Host client
  usb_host_client_config_t client_config = {
      .is_synchronous = false,
      .max_num_event_msg = 5,
      .async = {
          .client_event_callback = usb_event_callback,
          .callback_arg = this,
      }};

  err = usb_host_client_register(&client_config, &client_hdl);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Error initializing USB Host client: %s", esp_err_to_name(err));
    usb_host_initialized = false;
  } else {
    usb_host_initialized = true;
    ESP_LOGI(TAG, "USB Host client initialized successfully");
  }

  // Initial device detection
  usb_audio_connected_ = detect_usb_audio_device_();
  if (usb_audio_connected_) {
    ESP_LOGI(TAG, "USB audio device detected at initialization");
  } else {
    ESP_LOGI(TAG, "No USB audio device detected at initialization");
  }

#ifdef USE_ESP32S3_BOX_3
  // Configure the internal speaker GPIO
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << INTERNAL_SPEAKER_ENABLE_GPIO);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.flag = 0;
  gpio_config(&io_conf);
  // Disable internal speaker on startup.  Adjust if needed.
  enable_internal_speaker_(false);
#endif

  apply_audio_output_();
  update_text_sensor();
}

void USBAudioComponent::loop() {
  if (!usb_host_initialized) {
    return;
  }

  // Process USB events
  usb_host_client_handle_events(client_hdl, 0);

  // Periodically check in case events are missed
  static uint32_t last_check = 0;
  uint32_t now = esp_timer_get_time() / 1000;
  if (now - last_check > 2000) {  // Check every 2 seconds
    last_check = now;
    bool current_state = detect_usb_audio_device_();
    if (current_state != usb_audio_connected_) {
      ESP_LOGD(TAG, "USB state change detected by polling");
      usb_audio_connected_ = current_state;
      apply_audio_output_();
      update_text_sensor();
    }
  }
  vTaskDelay(pdMS_TO_TICKS(10)); // Add a small delay to prevent watchdog timeouts
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "USB Audio:");
  ESP_LOGCONFIG(TAG, "  Mode: %d", static_cast<int>(audio_output_mode_));
  ESP_LOGCONFIG(TAG, "  USB Headset connected: %s", usb_audio_connected_ ? "YES" : "NO");
  ESP_LOGCONFIG(TAG, "  USB Host initialized: %s", usb_host_initialized ? "YES" : "NO");
}

void USBAudioComponent::update_text_sensor() {
  if (text_sensor_ != nullptr) {
    text_sensor_->publish_state(usb_audio_connected_ ? "Connecté" : "Déconnecté");
  }
}

}  // namespace usbaudio
}  // namespace esphome











