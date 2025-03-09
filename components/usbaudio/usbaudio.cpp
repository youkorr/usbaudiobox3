#include "usbaudio.h"
#include "esphome/core/log.h"
#include "usb/usb_host.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// Variables globales pour le client USB Host
static usb_host_client_handle_t client_hdl = nullptr;
static bool usb_host_initialized = false;

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
  usb_device_handle_t dev_hdl;
  
  // Obtenir le handle du premier périphérique connecté
  esp_err_t err = usb_host_device_open(client_hdl, 0, &dev_hdl);
  if (err != ESP_OK) {
    ESP_LOGD(TAG, "Aucun périphérique USB détecté: %s", esp_err_to_name(err));
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
  
  // Vérifier d'abord si c'est directement un périphérique audio
  if (dev_desc->bDeviceClass == 0x01) {
    audio_device_present = true;
    ESP_LOGD(TAG, "Périphérique USB Audio détecté (classe 0x01)");
  } else {
    // Parcourir les configurations pour vérifier les interfaces audio
    for (uint8_t i = 0; i < dev_desc->bNumConfigurations && !audio_device_present; i++) {
      const usb_config_desc_t *config_desc;
      if (usb_host_get_active_config_descriptor(dev_hdl, &config_desc) == ESP_OK) {
        // Parcourir toutes les interfaces dans cette configuration
        uint8_t curr_idx = 0;
        const uint8_t *ptr = config_desc->val;
        
        // Parcourir les descripteurs d'interface
        while (curr_idx < config_desc->wTotalLength && !audio_device_present) {
          uint8_t desc_len = ptr[curr_idx];
          uint8_t desc_type = ptr[curr_idx + 1];
          
          // Vérifier si c'est un descripteur d'interface
          if (desc_type == 0x04 && desc_len >= 9) {
            // C'est un descripteur d'interface
            uint8_t intf_class = ptr[curr_idx + 5];
            if (intf_class == 0x01) {  // Classe Audio
              audio_device_present = true;
              ESP_LOGD(TAG, "Interface USB Audio détectée (classe 0x01)");
              break;
            }
          }
          
          // Passer au descripteur suivant
          curr_idx += desc_len;
        }
        
        // Libérer le descripteur de configuration
        usb_host_release_config_descriptor(dev_hdl, config_desc);
      }
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
        // Code pour activer les haut-parleurs internes
        break;
      case AudioOutputMode::USB_HEADSET:
        ESP_LOGD(TAG, "Activation forcée du casque USB");
        // Code pour activer le casque USB
        break;
      default:
        break;
    }
    return;
  }
  
  // Mode automatique
  if (usb_audio_connected_) {
    ESP_LOGD(TAG, "Basculement vers le casque USB (mode automatique)");
    // Code pour basculer vers le casque USB
  } else {
    ESP_LOGD(TAG, "Basculement vers les haut-parleurs internes (mode automatique)");
    // Code pour basculer vers les haut-parleurs internes
  }
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
  
  // On continue de vérifier périodiquement au cas où les événements ne seraient pas détectés
  static uint32_t last_check = 0;
  uint32_t now = millis();
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






