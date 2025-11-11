#include "PowerMode.h"
#include "SystemManager.h"   // para acessar leituras/intervalos
#include <Arduino.h>
#include "esp_sleep.h"
#include "esp_wifi.h"
#include <memory>

// Fábrica: mapeia % de bateria -> subclasse
std::unique_ptr<PowerMode> PowerMode::select_for(SystemManager* sm, float pct) {
    if (pct >= 90.0f)                       return std::unique_ptr<PowerMode>(new ActiveMode(sm));
    else if (pct >= 80.0f && pct <= 89.0f)  return std::unique_ptr<PowerMode>(new ModemSleepMode(sm));
    else if (pct >= 71.0f && pct <= 79.0f)  return std::unique_ptr<PowerMode>(new LightSleepMode(sm));
    else if (pct >= 30.0f && pct <= 70.0f)  return std::unique_ptr<PowerMode>(new DeepSleepMode(sm));
    else                                    return std::unique_ptr<PowerMode>(new HibernateMode(sm));
  }

// --------- IMPLEMENTAÇÕES BÁSICAS DOS MODOS ----------

void ActiveMode::enter_mode() {
    // Modo de maior desempenho
    Serial.println("[Power] Active");
    setCpuFrequencyMhz(240);        // clock máximo (ajuste se quiser)
    esp_wifi_set_ps(WIFI_PS_NONE);  // sem power-save no Wi-Fi (se estiver habilitado)
    // TODO: ligar periféricos necessários (OLED, sensores, rádios etc.)
}

void ModemSleepMode::enter_mode() {
    // CPU ativa; rádios dormem quando ocioso
    Serial.println("[Power] Modem-sleep");
    setCpuFrequencyMhz(80);                   // reduz clock para economizar
    esp_wifi_set_ps(WIFI_PS_MIN_MODEM);       // Wi-Fi em modem-sleep
    // TODO: reduzir TX power, desligar periféricos não essenciais, diminuir taxa de leitura/envio
}

// Implementação padrão vazia da base (evita vtable pendente)
void ModemSleepMode::run_tasks() {}

void LightSleepMode::enter_mode() {
  // CPU pausa; RTC/ULP ativos. Acorde por timer/pino conforme sua app.
  Serial.println("[Power] Light-sleep");
  esp_sleep_enable_timer_wakeup(5ULL * 1000ULL * 1000ULL); // exemplo: 5s
  esp_light_sleep_start();
}

void DeepSleepMode::enter_mode() {
  // Desliga quase tudo; RTC timer/pinos acordam
  // Ajuste o período a partir das políticas do SystemManager (ex.: sensor_send_interval)
  Serial.println("[Power] Deep-sleep");
  Serial.flush();
  esp_wifi_stop(); // garante rádios parados
  uint64_t us = 60ULL * 1000ULL * 1000ULL; // exemplo: acordar em 60s
  esp_sleep_enable_timer_wakeup(us);
  esp_deep_sleep_start();
}

void HibernateMode::enter_mode() {
  // Consumo mínimo; mantenha o menor número de fontes de wake
  Serial.println("[Power] Hibernate");
  Serial.flush();
  esp_wifi_stop();
  esp_sleep_disable_wakeup_source(ESP_SLEEP_WAKEUP_ALL);      // limpa todas as fontes
  esp_sleep_enable_timer_wakeup(30ULL * 60ULL * 1000000ULL);  // acorda em 30 min
  esp_deep_sleep_start(); // não retorna
}