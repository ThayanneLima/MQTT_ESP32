#ifndef SYSTEM_MANAGER_H
#define SYSTEM_MANAGER_H

#include <Arduino.h>
#include "BatteryManager.h"

class SystemManager {
public:
  explicit SystemManager(BatteryManager& bm) : battery(bm) {}

  void init();
  void tick(); // chame periodicamente (loop ou timer)

  // “Herdar”/expor informação de bateria para outras camadas
  float batteryVoltage() const { return battery.voltage(); }
  float batteryPercent() const { return battery.percent(); }
  BatteryManager::Mode batteryMode() const { return battery.mode(); }

  // Estado do sistema (pode ser usado por PowerMode)
  BatteryManager::Mode currentMode() const { return _currentMode; }

  // Gancho para aplicar política quando o modo muda
  void applyPolicy(BatteryManager::Mode m);

  BatteryManager& battery;

private:
  BatteryManager::Mode _currentMode = BatteryManager::Mode::Critical;
  uint32_t _lastPolicyMs = 0;
  uint32_t _policyDebounceMs = 1000; // anti-flapping
};

#endif