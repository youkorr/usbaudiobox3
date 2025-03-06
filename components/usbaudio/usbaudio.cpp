#include "usbaudio.h"
#include "esphome/core/log.h"

// Ajouter les bibliothèques USB nécessaires
#include "USB.h"
#include "USBH.h"
#include "usb/usb_host.h"

// Si vous utilisez TinyUSB
#include "tinyusb.h"
#include "tusb_config.h"
#include "class/audio/audio.h"

namespace esphome {
namespace usbaudio {

static const char *const TAG = "usbaudio";

// Constante pour la classe USB Audio
static const uint8_t USB_CLASS_AUDIO = 0x01;

bool USBAudioComponent::detect_usb_audio_device_() {
  // Détection réelle pour ESP32-S3 Box avec interface USB host
  ESP_LOGD(TAG, "Vérification des périphériques audio USB sur ESP32-S3 Box");
  
  // Si vous utilisez la bibliothèque USB-Host
  if (USB.connected()) {
    uint8_t dev_addr;
    uint8_t dev_count = USBH.getDeviceCount();
    
    ESP_LOGD(TAG, "Nombre de périphériques USB détectés: %d", dev_count);
    
    // Parcourir tous les périphériques USB connectés
    for (uint8_t i = 0; i < dev_count; i++) {
      dev_addr = USBH.getAddress(i);
      
      // Vérifier si le périphérique est de classe audio
      if (USBH.getDeviceClass(dev_addr) == USB_CLASS_AUDIO) {
        ESP_LOGI(TAG, "Périphérique audio USB détecté à l'adresse: %d", dev_addr);
        return true;
      }
      
      // Vérifier également les interfaces, car le périphérique audio pourrait être composé
      uint8_t num_interfaces = USBH.getInterfaceCount(dev_addr);
      for (uint8_t j = 0; j < num_interfaces; j++) {
        if (USBH.getInterfaceClass(dev_addr, j) == USB_CLASS_AUDIO) {
          ESP_LOGI(TAG, "Interface audio USB détectée à l'adresse: %d, interface: %d", dev_addr, j);
          return true;
        }
      }
    }
  } else {
    ESP_LOGD(TAG, "Pas de périphérique USB connecté");
  }
  
  // Vérification alternative avec TinyUSB (si applicable)
  #ifdef CONFIG_TINYUSB_ENABLED
  if (tud_mounted()) {
    for (uint8_t dev_addr = 0; dev_addr < CFG_TUH_DEVICE_MAX + 1; dev_addr++) {
      if (tuh_audio_mounted(dev_addr)) {
        ESP_LOGI(TAG, "TinyUSB: périphérique audio détecté à l'adresse: %d", dev_addr);
        return true;
      }
    }
  }
  #endif
  
  ESP_LOGD(TAG, "Aucun périphérique audio USB détecté");
  return false;
}

// Le reste de vos méthodes reste inchangé
/* ... */

void USBAudioComponent::setup() {
  ESP_LOGD(TAG, "Configuration du composant USB Audio pour ESP32-S3 Box");
  
  // Initialiser le système USB Host
  if (USB.begin()) {
    ESP_LOGI(TAG, "USB Host initialisé avec succès");
  } else {
    ESP_LOGE(TAG, "Échec de l'initialisation de USB Host");
  }
  
  // Vous pouvez également initialiser TinyUSB si nécessaire
  #ifdef CONFIG_TINYUSB_ENABLED
  ESP_LOGI(TAG, "Initialisation de TinyUSB");
  tinyusb_config_t tusb_cfg = {}; // Configuration par défaut
  tinyusb_driver_install(&tusb_cfg);
  #endif
  
  // Vérification initiale de la connexion
  usb_audio_connected_ = detect_usb_audio_device_();
  apply_audio_output_();
}

// Le reste de votre code reste inchangé...

}  // namespace usbaudio
}  // namespace esphome


