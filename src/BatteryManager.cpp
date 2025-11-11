#include "BatteryManager.h"

BatteryManager::BatteryManager(int pin, float vmin, float vmax)
: _adcPin(pin), _vmin(vmin), _vmax(vmax) {
  analogReadResolution(12); // ESP32 ADC 12 bits
}

void BatteryManager::update() {
  _lastVbat = readVbatVolts();
  // Clamp de segurança
  float v = _lastVbat;
  if (std::isnan(v)) { _levelPercent = 0.0f; _mode = Mode::Critical; return; }

  // Converte tensão -> % aproximada de LiPo 1S
  float pct = lipoPercent(v);
  // Garantir [0,100]
  if (pct < 0) pct = 0; else if (pct > 100) pct = 100;

  _levelPercent = pct;
  _mode = classify(pct);
}

float BatteryManager::readVbatVolts() const {
  // Usa API Arduino-ESP32; retorna mV medidos no pino do ADC
  // (com atenuação padrão da core; se quiser, configure attenuation separadamente)
  uint32_t mv = analogReadMilliVolts(_adcPin);
  float v_adc = mv / 1000.0f;
  return v_adc * _divider; // corrige pelo divisor 390k/100k (4.9x)
}

// Curva compacta (repouso) para célula 1S: aproximação piecewise
float BatteryManager::lipoPercent(float v) {
  if (v <= 3.30f) return 0.f;
  if (v >= 4.20f) return 100.f;
  if (v < 3.60f) return map((long)(v*1000), 3300, 3600, 0, 15);
  if (v < 3.80f) return map((long)(v*1000), 3600, 3800, 15, 55);
  if (v < 4.00f) return map((long)(v*1000), 3800, 4000, 55, 85);
  return map((long)(v*1000), 4000, 4200, 85, 100);
}

BatteryManager::Mode BatteryManager::classify(float p) const {
  if (p >= (_lowThreshold + 1.0f)) return Mode::Normal;   // ≥71%
  if (p >= 30.0f && p <= _lowThreshold) return Mode::Economy; // 30..70
  return Mode::Critical;                                   // ≤29
}

const char* BatteryManager::toString(BatteryManager::Mode m) {
  switch (m) {
    case Mode::Normal:   return "Normal";
    case Mode::Economy:  return "Economia";
    case Mode::Critical: return "Crítico";
    default:             return "Desconhecido";
  }
}