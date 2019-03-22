/**
 * Homie Node for Maxime Temperature sensors.
 *
 * This Node supports the following devices :
 *  * DS18B20
 *  * DS18S20 - Please note there appears to be an issue with this series.
 *  * DS1822
 *  * DS1820
 *  * MAX31820
 *
 * You will need a pull-up resistor of about 5 KOhm between the 1-Wire data line and your 5V power.
 * If you are using the DS18B20, ground pins 1 and 3. The centre pin is the data line '1-wire'.
 *
 * Used lib:
 * https://github.com/milesburton/Arduino-Temperature-Control-Library
 * https://www.milesburton.com/Dallas_Temperature_Control_Library
 *
 */
#include "DallasTemperatureNode.hpp"

DallasTemperatureNode::DallasTemperatureNode(const char* id, const char* name, const uint8_t pin, const int measurementInterval)
    : HomieNode(id, name, "temperature") {

  _pin                 = pin;
  _measurementInterval = (measurementInterval > MIN_INTERVAL) ? measurementInterval : MIN_INTERVAL;
  _lastMeasurement     = 0;

  oneWire = new OneWire(_pin);
  sensor  = new DallasTemperature(oneWire);
}

/**
 *
 */
void DallasTemperatureNode::setup() {
  // Start up the library
  sensor->begin();
  // set global resolution to 9, 10, 11, or 12 bits
  sensor->setResolution(12);

  advertise(cStatus).setName(cStatusName);
  advertise(cTemperature).setName(cTemperatureName).setDatatype("float").setUnit(cTemperatureUnit);
}

/**
 *
 */
void DallasTemperatureNode::onReadyToOperate() {

  // Grab a count of devices on the wire
  numberOfDevices = sensor->getDeviceCount();
  // report parasite power requirements
  Homie.getLogger() << cIndent << F("Parasite power is: ") << sensor->isParasitePowerMode() << endl;

  if (numberOfDevices > 0) {
    Homie.getLogger() << cIndent << numberOfDevices << F(" devices found on PIN ") << _pin << endl;

    for (uint8_t i = 0; i < numberOfDevices; i++) {
      // Search the wire for address
      DeviceAddress tempDeviceAddress;  // We'll use this variable to store a found device address

      if (sensor->getAddress(tempDeviceAddress, i)) {
        String adr = address2String(tempDeviceAddress);
        Homie.getLogger() << cIndent << F("PIN ") << _pin << F(": ") << F("Device ") << i << F(" using address ") << adr << endl;
      }
    }
  } else {
    Homie.getLogger() << F("✖ No sensors found on pin ") << _pin << endl;
    setProperty(cStatus).send("no sensors found");
  }
}

/**
 *
 */
void DallasTemperatureNode::loop() {
  if (millis() - _lastMeasurement >= _measurementInterval * 1000UL || _lastMeasurement == 0) {
    _lastMeasurement = millis();

    if (numberOfDevices > 0) {
      Homie.getLogger() << F("〽 Sending Temperature: ") << getId() << endl;
      // call sensors.requestTemperatures() to issue a global temperature
      // request to all devices on the bus
      sensor->requestTemperatures();  // Send the command to get temperature readings
      for (uint8_t i = 0; i < numberOfDevices; i++) {
        uint8_t cnt = 0;

        DeviceAddress tempDeviceAddress;
        if (sensor->getAddress(tempDeviceAddress, i)) {
          do {
            temperature = sensor->getTempC(tempDeviceAddress);

            cnt++;
            delay(1);

            if (cnt > 5) {
              temperature = NAN;
              Homie.getLogger() << F(" Error reading sensor: ") << getId() << F(" request count: ") << cnt << endl;
              setProperty(cStatus).send("Error reading sensor");

              return;
            }
          } while (temperature >= getUpper() || temperature <= getLower());
        }

        Homie.getLogger() << cIndent << F("Status=ok") << endl;
        setProperty(cStatus).setRetained(true).send("ok");

        Homie.getLogger() << cIndent << F("Temperature=") << temperature << endl;
        setProperty(cTemperature).send(String(temperature));
      }
    } else {

      Homie.getLogger() << "No Sensor found!" << endl;
      setProperty(cStatus).send("no sensor found.");

      //retry to get
      numberOfDevices = sensor->getDeviceCount();
    }
  }
}

/**
 *
 */
void DallasTemperatureNode::printCaption() {
  Homie.getLogger() << cCaption << endl;
}

/**
 *
 */
String DallasTemperatureNode::address2String(const DeviceAddress deviceAddress) {
  String adr;

  for (uint8_t i = 0; i < 8; i++) {
    // zero pad the address if necessary

    if (deviceAddress[i] < 16) {
      adr = adr + "0";
    }
    adr = adr + String(deviceAddress[i], HEX);
  }

  return adr;
}
