#include "usbaudio.h"
#include "esphome/core/log.h"
#include "usb/usb_host.h"
#include "driver/gpio.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// Variables globales pour le client USB Host
static usb_host_client_handle_t client_hdl = nullptr;
static bool usb_host_initialized = false;

// Configuration pour le GPIO du haut-parleur interne
#ifdef USE_ESP32S3_BOX_3
static const int INTERNAL_SPEAKER_ENABLE_GPIO = 38; // Remplacez par le GPIO correct
#endif

// Définitions pour les descripteurs USB
#define USB_DESCRIPTOR_TYPE_INTERFACE 0x04
#define USB_CLASS_AUDIO 0x01

// Callback pour les événements USB
static void usb_event_callback(const usb_host_client_event_msg_t *event_msg, void *arg) {
  USBAudioComponent *component = static_cast<USBAudioComponent *>(arg);
  switch (event_msg->event) {
    case USB_HOST_CLIENT_EVENT_NEW_DEV:
      ESP_LOGD(TAG, "Nouvel appareil USB détecté");
      component->handle_device_connection();
      break;
    case USB_HOST_CLIENT_EVENT_DEV_GONE:
      ESP_LOGD(TAG, "Appareil USB déconnecté");
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
    ESP_LOGD(TAG, "USB Host non initialisé, impossible de détecter les périphériques");
    return false;
  }

  bool audio_device_present = false;
  usb_device_handle_t dev_hdl = nullptr;
  esp_err_t err;

  const uint16_t expected_vendor_id = 0x1234; // Remplacer par le Vendor ID de votre appareil
  const uint16_t expected_product_id = 0x5678; // Remplacer par le Product ID de votre appareil

  // Tentative d'ouverture du périphérique plusieurs fois
  for (int attempt = 0; attempt < 3; ++attempt) {
    ESP_LOGD(TAG, "Tentative de détection USB audio, essai %d", attempt + 1);
    err = usb_host_device_open(client_hdl, 1, &dev_hdl); // Essayer l'adresse 1
    if (err == ESP_OK) {
      break; // Périphérique ouvert avec succès
    } else {
      ESP_LOGW(TAG, "Erreur lors de l'ouverture du périphérique USB (essai %d): %s", attempt + 1, esp_err_to_name(err));
      vTaskDelay(pdMS_TO_TICKS(500)); // Attendre avant de réessayer
    }
  }

  if (err != ESP_OK) {
    ESP_LOGD(TAG, "Aucun périphérique USB détecté après plusieurs tentatives");
    return false;
  }

  // Obtenir le descripteur du périphérique
  const usb_device_desc_t *dev_desc;
  err = usb_host_get_device_descriptor(dev_hdl, &dev_desc);
  if (err != ESP_OK) {
    ESP_LOGW(TAG, "Erreur lors de l'obtention du descripteur: %s", esp_err_to_name(err));
    usb_host_device_close(client_hdl, dev_hdl);
    return false;
  }

    // Vérifier le Vendor ID et le Product ID
    if (dev_desc->idVendor != expected_vendor_id || dev_desc->idProduct != expected_product_id) {
        ESP_LOGW(TAG, "Périphérique USB trouvé, mais Vendor ID ou Product ID incorrect.");
        usb_host_device_close(client_hdl, dev_hdl);
        return false;
    }

  // Vérifier d'abord si c'est directement un périphérique audio
  if (dev_desc->bDeviceClass == 0x01) {
    audio_device_present = true;
    ESP_LOGD(TAG, "Périphérique USB Audio détecté (classe 0x01)");
  } else {
    // Parcourir les configurations pour vérifier les interfaces audio
    const usb_config_desc_t *config_desc = nullptr;
    if (usb_host_get_active_config_descriptor(dev_hdl, &config_desc) == ESP_OK) {
      ESP_LOGD(TAG, "Descripteur de configuration active obtenu");
      uint8_t curr_idx = 0;
      const uint8_t *ptr = config_desc->val;

      while (curr_idx < config_desc->wTotalLength) {
        uint8_t desc_len = ptr[curr_idx];
        uint8_t desc_type = ptr[curr_idx + 1];

        if (desc_type == USB_DESCRIPTOR_TYPE_INTERFACE && desc_len >= 9) {
          uint8_t intf_class = ptr[curr_idx + 5];
          uint8_t intf_subclass = ptr[curr_idx + 6];
          uint8_t intf_protocol = ptr[curr_idx + 7];
          ESP_LOGD(TAG, "Descripteur d'interface trouvé: Class=0x%02X, SubClass=0x%02X, Protocol=0x%02X", intf_class, intf_subclass, intf_protocol);

          if (intf_class == USB_CLASS_AUDIO) {
            ESP_LOGI(TAG, "Interface audio détectée (Class: 0x%02X)", USB_CLASS_AUDIO);
            audio_device_present = true;
            break;
          }
        }

        curr_idx += desc_len;
        if (desc_len == 0) {
          ESP_LOGE(TAG, "Longueur de descripteur invalide, sortie de la boucle");
          break;
        }
      }
    } else {
      ESP_LOGW(TAG, "Impossible d'obtenir le descripteur de configuration active");
    }
  }

  // Libérer les ressources
  usb_host_device_close(client_hdl, dev_hdl);

  if (!audio_device_present) {
    ESP_LOGD(TAG, "Périphérique USB détecté, mais ce n'est pas un périphérique audio");
  }

  return audio_device_present;
}

void USBAudioComponent::handle_device_connection() {
  bool was_connected = usb_audio_connected_;
  usb_audio_connected_ = detect_usb_audio_device_();
  if (usb_audio_connected_ && !was_connected) {
    ESP_LOGI(TAG, "Périphérique audio USB connecté");
    apply_audio_output_();
    update_text_sensor();
  }
}

void USBAudioComponent::handle_device_disconnection() {
  bool was_connected = usb_audio_connected_;
  usb_audio_connected_ = false;  // On suppose qu'il est déconnecté
  if (was_connected) {
    ESP_LOGI(TAG, "Périphérique audio USB déconnecté");
    apply_audio_output_();
    update_text_sensor();
  }
}

void USBAudioComponent::apply_audio_output_() {
  if (audio_output_mode_ != AudioOutputMode::AUTO_SELECT) {
    // Mode manuel
    switch (audio_output_mode_) {
      case AudioOutputMode::INTERNAL_SPEAKERS:
        ESP_LOGD(TAG, "Activation forcée des haut-parleurs internes");
        enable_internal_speaker_(true);
        break;
      case AudioOutputMode::USB_HEADSET:
        ESP_LOGD(TAG, "Activation forcée du casque USB");
        enable_internal_speaker_(false);
        break;
      default:
        break;
    }
    return;
  }

  // Mode automatique
  if (usb_audio_connected_) {
    ESP_LOGD(TAG, "Basculement vers le casque USB (mode automatique)");
    enable_internal_speaker_(false);
  } else {
    ESP_LOGD(TAG, "Basculement vers les haut-parleurs internes (mode automatique)");
    enable_internal_speaker_(true);
  }
}

void USBAudioComponent::enable_internal_speaker_(bool enable) {
#ifdef USE_ESP32S3_BOX_3
  ESP_LOGD(TAG, "Activation du haut-parleur interne: %s", enable ? "OUI" : "NON");
  gpio_set_level((gpio_num_t)INTERNAL_SPEAKER_ENABLE_GPIO, enable ? 1 : 0); // HIGH active le haut-parleur (à vérifier)
#else
  ESP_LOGW(TAG, "Le contrôle du haut-parleur interne n'est pris en charge que sur ESP32-S3-BOX-3. Définissez USE_ESP32S3_BOX_3 dans votre configuration.");
#endif
}

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Initialisation du composant USB Audio");

  // Initialisation du sous-système USB Host si ce n'est pas déjà fait
  const usb_host_config_t host_config = {
      .intr_flags = ESP_INTR_FLAG_LEVEL1,
  };

  esp_err_t err = usb_host_install(&host_config);
  if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
    // ESP_ERR_INVALID_STATE peut se produire si USB Host est déjà installé
    ESP_LOGE(TAG, "Erreur d'installation USB Host: %s", esp_err_to_name(err));
    usb_host_initialized = false;
    return;
  }

  // Initialisation du client USB Host
  usb_host_client_config_t client_config = {
      .is_synchronous = false,
      .max_num_event_msg = 5,
      .async = {
          .client_event_callback = usb_event_callback,
          .callback_arg = this,
      }};

  err = usb_host_client_register(&client_config, &client_hdl);
  if (err != ESP_OK) {
    ESP_LOGE(TAG, "Erreur d'initialisation du client USB Host: %s", esp_err_to_name(err));
    usb_host_initialized = false;
  } else {
    usb_host_initialized = true;
    ESP_LOGI(TAG, "Client USB Host initialisé avec succès");
  }

#ifdef USE_ESP32S3_BOX_3
  // Configuration du GPIO du haut-parleur interne
  gpio_config_t io_conf;
  io_conf.intr_type = GPIO_INTR_DISABLE;
  io_conf.mode = GPIO_MODE_OUTPUT;
  io_conf.pin_bit_mask = (1ULL << INTERNAL_SPEAKER_ENABLE_GPIO);
  io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
  io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
  io_conf.flag = 0;
  gpio_config(&io_conf);
  enable_internal_speaker_(false); // Désactiver le haut-parleur au démarrage
#endif

  // Détection initiale
  usb_audio_connected_ = detect_usb_audio_device_();
  if (usb_audio_connected_) {
    ESP_LOGI(TAG, "Périphérique audio USB détecté lors de l'initialisation");
  } else {
    ESP_LOGI(TAG, "Aucun périphérique audio USB détecté lors de l'initialisation");
  }

  apply_audio_output_();
  update_text_sensor();
}

void USBAudioComponent::loop() {
  if (!usb_host_initialized) {
    return;
  }

  // Traitement des événements USB
  usb_host_client_handle_events(client_hdl, 0);

  // Vérification périodique pour détecter les changements d'état
  static uint32_t last_check = 0;
  uint32_t now = esp_timer_get_time() / 1000;
  if (now - last_check > 2000) {  // Vérification toutes les 2 secondes
    last_check = now;
    bool current_state = detect_usb_audio_device_();
    if (current_state != usb_audio_connected_) {
      ESP_LOGD(TAG, "Changement d'état USB détecté par polling");
      usb_audio_connected_ = current_state;
      apply_audio_output_();
      update_text_sensor();
    }
  }
  vTaskDelay(pdMS_TO_TICKS(10)); // Délai pour éviter les problèmes de watchdog
}

void USBAudioComponent::dump_config() {
  ESP_LOGCONFIG(TAG, "USB Audio:");
  ESP_LOGCONFIG(TAG, "  Mode: %d", static_cast<int>(audio_output_mode_));
  ESP_LOGCONFIG(TAG, "  Casque USB connecté: %s", usb_audio_connected_ ? "OUI" : "NON");
  ESP_LOGCONFIG(TAG, "  USB Host initialisé: %s", usb_host_initialized ? "OUI" : "NON");
}

void USBAudioComponent::update_text_sensor() {
  if (text_sensor_ != nullptr) {
    text_sensor_->publish_state(usb_audio_connected_ ? "Connecté" : "Déconnecté");
  }
}

}  // namespace usbaudio
}  // namespace esphome













