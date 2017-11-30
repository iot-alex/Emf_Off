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
