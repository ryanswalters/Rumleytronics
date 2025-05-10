#include <Wire.h>
#include "SensirionI2CSps30.h"



#define SDA_PIN 8
#define SCL_PIN 9

SensirionI2CSps30 sps30;

void setup() {
  Serial.begin(115200);
  delay(1000);

  Wire.begin(SDA_PIN, SCL_PIN);
  sps30.begin(Wire);

  // Wake-up (optional if device was in sleep)
  Serial.println("Waking up SPS30...");
  sps30.wakeUp();

  delay(100); // Let it stabilize

  // Start measurements
  uint8_t ret = sps30.startMeasurement();
  if (ret == 0) {
    Serial.println("SPS30 measurement started.");
  } else {
    Serial.print("Start measurement failed! Error code: ");
    Serial.println(ret);
    while (1); // Die loud
  }

  Serial.println("Waiting for first data...");
}

void loop() {
  struct sps30_measurement measurement;

  // Check if new data is ready
  bool dataReady;
  if (sps30.readDataReadyFlag(dataReady) != 0 || !dataReady) {
    delay(1000);
    return;
  }

  // Read measurement
  int16_t ret = sps30.readMeasurement(measurement);
  if (ret < 0) {
    Serial.println("Error reading measurement.");
    delay(1000);
    return;
  }

  Serial.println("SPS30 Measurement:");
  Serial.print("PM1.0: "); Serial.print(measurement.mc_1p0); Serial.println(" µg/m³");
  Serial.print("PM2.5: "); Serial.print(measurement.mc_2p5); Serial.println(" µg/m³");
  Serial.print("PM4.0: "); Serial.print(measurement.mc_4p0); Serial.println(" µg/m³");
  Serial.print("PM10 : "); Serial.print(measurement.mc_10p0); Serial.println(" µg/m³");
  Serial.print("Number Concentration >0.5µm: "); Serial.println(measurement.nc_0p5);
  Serial.print("Typical Particle Size: "); Serial.println(measurement.typical_particle_size);

  Serial.println("-----------");

  delay(2000); // adjust as needed
}
