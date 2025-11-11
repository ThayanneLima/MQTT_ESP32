#pragma once
#include <memory>
#include "SystemManager.h"

class PowerMode {
public:
  explicit PowerMode(SystemManager* sm) : sys(sm) {}
  virtual ~PowerMode(){}

  // Entra no modo (configura clocks, rádios, sleep, etc.)
  virtual void enter_mode() = 0;

  // Opcional: tarefas periódicas específicas do modo
  virtual void run_tasks() {}

  //Seleciona a subclasse adequada a partir da % da bateria
  static std::unique_ptr<PowerMode> select_for(SystemManager* sm, float pct);

protected:
  SystemManager* sys;
};

// ---------- Subclasses ----------
class ActiveMode : public PowerMode {
public: using PowerMode::PowerMode;
  void enter_mode() override;
};

class ModemSleepMode : public PowerMode {
public: using PowerMode::PowerMode;
  void enter_mode() override;
  void run_tasks() override; // manter funções essenciais
};

class LightSleepMode : public PowerMode {
public: using PowerMode::PowerMode;
  void enter_mode() override;
};

class DeepSleepMode : public PowerMode {
public: using PowerMode::PowerMode;
  void enter_mode() override;   // configura timer/pinos e dorme
};

class HibernateMode : public PowerMode {
public: using PowerMode::PowerMode;
  void enter_mode() override;   // hiberna (deep com mínimos recursos)
};