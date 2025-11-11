#include "SystemManager.h"

void SystemManager::init() {
  battery.update();
  _currentMode = battery.mode();
  applyPolicy(_currentMode);
}

void SystemManager::tick() {
  battery.update();
  BatteryManager::Mode m = battery.mode();
  if (m != _currentMode && (millis() - _lastPolicyMs) > _policyDebounceMs) {
    _currentMode = m;
    applyPolicy(m);
  }
}

void SystemManager::applyPolicy(BatteryManager::Mode m) {
  _lastPolicyMs = millis();

  // Aqui você pode ligar/desligar periféricos, ajustar TX power etc.
  switch (m) {
    case BatteryManager::Mode::Normal:
      // operação normal
      break;
    case BatteryManager::Mode::Economy:
      // reduzir frequência de leitura/envio, desligar OLED/Vext, reduzir TX power…
      break;
    case BatteryManager::Mode::Critical:
      // preparar deep-sleep, salvar estado, etc.
      break;
  }
}