/**
 * Homie Node for Relay Module.
 *
 */
#include <Homie.hpp>
#include "RelayModuleNode.hpp"

RelayModuleNode::RelayModuleNode(const char* id, const char* name, const int pin, const int measurementInterval)
    : HomieNode(id, name, "switch") {
  _pin                 = pin;
  _measurementInterval = (measurementInterval > MIN_INTERVAL) ? measurementInterval : MIN_INTERVAL;
  _lastMeasurement     = 0;
}

void RelayModuleNode::printCaption() {
  Homie.getLogger() << cCaption << endl;
}

void RelayModuleNode::loop() {
  if (millis() - _lastMeasurement >= _measurementInterval * 1000UL || _lastMeasurement == 0) {
    Homie.getLogger() << "〽 Sending Switch status: " << getId() << endl;
    boolean state= relay->isOn();
    Homie.getLogger() << cIndent << "switch: " << state << endl;

    if(state) {
      setProperty("switch").send("on");
    } else {
      setProperty("switch").send("off");
    }

    _lastMeasurement = millis();
  }
}

void RelayModuleNode::onReadyToOperate() {

  advertise("switch").setName("Switch").setDatatype("boolean");
}

void RelayModuleNode::setup() {
  relay = new RelayModule(_pin);
}
