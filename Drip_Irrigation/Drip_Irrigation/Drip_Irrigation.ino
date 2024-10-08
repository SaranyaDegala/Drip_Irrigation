include <OneWire.h>
#include <DallasTemperature.h>
// GPIO pins
const int soilMoisturePin = 34; // Soil moisture sensor
const int oneWireBus = 4;       // DS18B20 temperature sensor
const int relayPin = 23;        // Relay for solenoid valve
const int flowSensorPin = 18;   // Flow sensor pin
// Flow sensor variables
volatile int pulseCount = 0;    // Count of pulses from flow sensor
unsigned long lastTime = 0;
float flowRate = 0.0;
float totalFlow = 0.0;          // Total water consumption
float calibrationFactor = 7.5;  // Calibration factor for your flow sensor (depends on the sensor)
// Time tracking variables
unsigned long valveOpenTime = 0;   // Time when valve opened
unsigned long totalOpenDuration = 0; // Total time valve has been open
bool valveOpen = false;
// Create OneWire instance for DS18B20 sensor
OneWire oneWire(oneWireBus);
DallasTemperature sensor(&oneWire);
// Interrupt function for flow sensor
void IRAM_ATTR pulseCounter() {
  pulseCount++;  // Increment pulse count every time an interrupt occurs
}
void setup() {
  Serial.begin(9600);
  // Initialize relay and flow sensor pin
  pinMode(relayPin, OUTPUT);
  pinMode(flowSensorPin, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(flowSensorPin), pulseCounter, FALLING);
  // Initialize relay as off (LOW)
  digitalWrite(relayPin, LOW);
  // Start the DS18B20 sensor
  sensor.begin();
}
void loop() {
  // Read soil moisture level
  int moisture = analogRead(soilMoisturePin);
  // Request temperature from DS18B20 sensor
  sensor.requestTemperatures();
  float temp = sensor.getTempCByIndex(0);
  // Print values for debugging
  Serial.print("Moisture: ");
  Serial.print(moisture);
  Serial.print(", Temp: ");
  Serial.println(temp);
  // Condition to control relay (turn on water only if both conditions are met)
  if (moisture < 500 && temp > 30.0) {
    if (!valveOpen) {
      valveOpen = true;
      valveOpenTime = millis();  // Record when the valve was opened
    }
    digitalWrite(relayPin, HIGH);  // Turn on water
    Serial.println("Watering ON");
    // Calculate flow rate and total water consumption
    if (millis() - lastTime > 1000) {  // Every 1 second
      flowRate = (pulseCount / calibrationFactor);  // L/min
      // Only add to totalFlow if there is actual flow
      if (flowRate > 0) {
        totalFlow += (flowRate / 60.0);  // Liters per second
      }
      // Reset pulse count and timer
      pulseCount = 0;
      lastTime = millis();
      // Print flow rate and total water consumption
      Serial.print("Flow Rate: ");
      Serial.print(flowRate);
      Serial.println(" L/min");
      Serial.print("Total Flow: ");
      Serial.print(totalFlow);
      Serial.println(" Liters");
    }
  } else {
    if (valveOpen) {
      valveOpen = false;
      totalOpenDuration += (millis() - valveOpenTime);  // Calculate total open time
    }
    digitalWrite(relayPin, LOW);  // Turn off water
    Serial.println("Watering OFF");
    // Print the time the valve was open in seconds
    Serial.print("Valve Open Time: ");
    Serial.print(totalOpenDuration / 1000);  // Convert milliseconds to seconds
    Serial.println(" seconds");
  }
  // Wait before the next reading
  delay(2000);
}
