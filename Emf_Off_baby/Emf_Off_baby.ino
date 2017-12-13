#include <Arduino.h>
#include <ReducedSoftwareSerial.h>
#include <Emf_Off_common.h>

static const int TX_PIN = 2;
static const int RX_PIN = 1;
static const int MICROPHONE_LEVEL_SENSE_PIN = A2;
static const int ALARM_THRESHOLD_SENSE_PIN = A0;
static const int STATUS_LED_PIN = 0;
static const int SUPPLY_VOLTAGE_SENSE_PIN = A3;

static const int ALERT_RETRY_INTERVAL_MILLISECONDS = 1000;
static const unsigned long LOW_BATTERY_CHECK_INTERVAL_MS = 120000;
static const int ALARM_THRESHOLD_MINIMUM = 500;
static const int NOISE_SENSE_THRESHOLD_ADC_MINIMUM = 490;
static const int ALARM_THRESHOLD_SENSE_SCALE_FACTOR = 4;
static const int MINIMUM_NOISE_COUNT_TO_TRIGER_ALERT = 25;


struct Alert {
  bool active = false;
  bool acknowledged = false;
  int millisecondsUntilRetryTransmission = 0;

  void reset() {
    active = false;
    acknowledged = false;
    millisecondsUntilRetryTransmission = 0;
  }
};

SoftwareSerial softSerial(RX_PIN, TX_PIN);
Alert alert;
unsigned long previousMillis = 0;
unsigned long millisSinceLowBatteryCheck = LOW_BATTERY_CHECK_INTERVAL_MS;
bool lowBatteryWarningAcknowledged = false;

int noiseCount = 0;
unsigned long noiseStartMillis = 0;

void setup() {
  softSerial.begin(9600);

  pinMode(STATUS_LED_PIN, OUTPUT);
  pinMode(SUPPLY_VOLTAGE_SENSE_PIN, INPUT_PULLUP);
  pinMode(ALARM_THRESHOLD_SENSE_PIN, INPUT_PULLUP);

  if (batteryVoltageIsOk(SUPPLY_VOLTAGE_SENSE_PIN)) {
    displayStatusOk(STATUS_LED_PIN);
  }
  else {
    displayStatusLowBattery(STATUS_LED_PIN);
  }
}

void loop() {
  int millisSinceLastLoop = millis() - previousMillis;
  previousMillis = millis();

  if (noiseDetected()) {
    alert.active = true;
    analogWrite(STATUS_LED_PIN, 15);
  }
  else {
    digitalWrite(STATUS_LED_PIN, LOW);
  }

  if (alert.active && !alert.acknowledged) {
    alert.millisecondsUntilRetryTransmission -= millisSinceLastLoop;

    if (alert.millisecondsUntilRetryTransmission <= 0) {
      transmitAlert();
      alert.millisecondsUntilRetryTransmission = ALERT_RETRY_INTERVAL_MILLISECONDS;
    }
  }

  millisSinceLowBatteryCheck += millisSinceLastLoop;

  if (millisSinceLowBatteryCheck > LOW_BATTERY_CHECK_INTERVAL_MS) {
    millisSinceLowBatteryCheck = 0;

    if (batteryVoltageIsOk(SUPPLY_VOLTAGE_SENSE_PIN)) {
      lowBatteryWarningAcknowledged = false;
    }
    else if(!lowBatteryWarningAcknowledged) {
      softSerial.print('B');
    }
  }

  respondToReceivedSerialData();
}

bool noiseDetected() {
  analogReference(DEFAULT);
  int microphoneLevel = analogRead(MICROPHONE_LEVEL_SENSE_PIN);

  //  TODO: Don't read the threshold every time as it is quite slow and will reduce our ability to detect sounds
  int rawAdc = analogRead(ALARM_THRESHOLD_SENSE_PIN);
  int scaledADC = (rawAdc - NOISE_SENSE_THRESHOLD_ADC_MINIMUM) / ALARM_THRESHOLD_SENSE_SCALE_FACTOR;
  int alarmThreshold = ALARM_THRESHOLD_MINIMUM + scaledADC;

  if (microphoneLevel > alarmThreshold) {
    if (millis() - noiseStartMillis > 500) {
      noiseStartMillis = millis();
      noiseCount = 0;
    }

    noiseCount++;

    if (noiseCount > MINIMUM_NOISE_COUNT_TO_TRIGER_ALERT) {
      return true;
    }
  }

  return false;
}

void transmitAlert() {
  softSerial.print('A');
}

void respondToReceivedSerialData() {
  while (softSerial.available()) {
    char receivedChar = softSerial.read();
    switch (receivedChar) {
      case 'A':
        if (alert.active) {
          alert.acknowledged = true;
        }
        break;
      case 'P': // Ping (connection check)
        softSerial.print('P');
        alert.reset();
        break;
      case 'B':
        lowBatteryWarningAcknowledged = true;
        break;
    }
  }
}
