
#pragma once

#include "Rule.hpp"

class RuleManu : public Rule {
public:
  RuleManu();

  const char* getMode() { return "manu"; };

  virtual void loop();
};
