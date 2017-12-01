#include "Emf_Off_common.h"

bool lowBatteryVoltage = false;

bool batteryVoltageIsOk(int supplyVoltageSensePin) {
  analogReference(INTERNAL1V1);
  int rawAdcValue = analogRead(supplyVoltageSensePin);

  if (rawAdcValue < LOW_BATTERY_VOLTAGE_THRESHOLD_RAW_ADC_VALUE) {
    lowBatteryVoltage = true;
  }

  if (rawAdcValue > BATTERY_VOLTAGE_OK_THRESHOLD_RAW_ADC_VALUE) {
    lowBatteryVoltage = false;
  }

  return !lowBatteryVoltage;
}

void displayStatusOk(int statusLedPin) {
  digitalWrite(statusLedPin, HIGH);
  delay(POWER_ON_STATUS_DISPLAY_DURATION_MS);
  digitalWrite(statusLedPin, LOW);
}

void displayStatusLowBattery(int statusLedPin) {
  byte flashSpeedMilliseconds = 100;
  byte flashCount = POWER_ON_STATUS_DISPLAY_DURATION_MS / (flashSpeedMilliseconds * 2);

  for(byte i = 0; i < flashCount; i++) {
    digitalWrite(statusLedPin, HIGH);
    delay(flashSpeedMilliseconds);
    digitalWrite(statusLedPin, LOW);
    delay(flashSpeedMilliseconds);
  }
}
