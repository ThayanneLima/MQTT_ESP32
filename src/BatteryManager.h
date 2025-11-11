#ifndef BATTERY_MANAGER_H
#define BATTERY_MANAGER_H

#include <Arduino.h>
#include <cmath>

class BatteryManager {
public:
  enum class Mode : uint8_t { Normal, Economy, Critical };

  // Por padrão mantém seu "pin = 1" como você já usava; ajuste se o seu
  // mapeamento for GPIO36 (ADC1_CH0) em outra revisão de placa.
  BatteryManager(int pin = 1, float vmin = 3.3f, float vmax = 4.2f);

  // Atualiza leitura (tensão -> % -> modo)
  void update();

  // Acesso aos últimos valores calculados
  float voltage()   const { return _lastVbat; }     // Volts
  float percent()   const { return _levelPercent; } // 0..100
  Mode  mode()      const { return _mode; }

  // Utilitários
  static const char* toString(Mode m);

  // Ajustes finos (se quiser customizar thresholds em runtime)
  void setThresholds(float lowPct, float critPct) {
    _lowThreshold = lowPct; _critThreshold = critPct;
  }

private:
  // Conversões
  float readVbatVolts() const;         // Lê ADC e devolve VBAT (V)
  static float lipoPercent(float v);    // Aproximação 1S (repouso)

  // Classificação a partir da % atual
  Mode classify(float pct) const;

  // Config
  int   _adcPin;
  float _vmin, _vmax;                  // 1S LiPo: 3.3 .. 4.2 V
  float _divider = 4.9f;               // (100k+390k)/100k (Heltec)

  // Thresholds de % (padrão de sua especificação: ≥71 Normal; 30..70 Econ; ≤29 Crit)
  float _lowThreshold  = 70.0f;
  float _critThreshold = 29.0f;

  // Estado
  float _lastVbat      = NAN;
  float _levelPercent  = NAN;
  Mode  _mode          = Mode::Critical;
};

#endif