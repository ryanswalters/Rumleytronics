#include <Wire.h>
#include <Adafruit_BMP3XX.h>
#include "SparkFun_SCD4x_Arduino_Library.h"

#define SDA_PIN 8
#define SCL_PIN 9
#define PCA_ADDR 0x70
#define BMP_CHANNEL 0
#define SCD_CHANNEL 1

Adafruit_BMP3XX bmp;
SCD4x scd41;

unsigned long lastCheck = 0;
uint8_t lastFound[128] = {0};

void selectPCAChannel(uint8_t channel) {
  Wire.beginTransmission(PCA_ADDR);
  Wire.write(1 << channel);
  Wire.endTransmission();
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  Serial.println("📡 Voltage + PCA + BMP390 + SCD41 Scanner Initialized");
  Serial.println("======================================================");

  // Init BMP390 on channel 0
  selectPCAChannel(BMP_CHANNEL);
  if (bmp.begin_I2C(0x76)) {
    Serial.println("📈 BMP390 initialized.");
  } else {
    Serial.println("❌ BMP390 failed to initialize.");
  }

  // Init SCD41 on channel 1
  selectPCAChannel(SCD_CHANNEL);
  if (scd41.begin() && scd41.startPeriodicMeasurement()) {
    Serial.println("🌿 SCD41 initialized and measurement started.");
  } else {
    Serial.println("❌ SCD41 failed to initialize.");
  }
}

void loop() {
  if (millis() - lastCheck >= 5000) {
    Serial.print("🧪 SDA: "); Serial.print(digitalRead(SDA_PIN) ? "HIGH" : "LOW");
    Serial.print(" | SCL: "); Serial.println(digitalRead(SCL_PIN) ? "HIGH" : "LOW");

    // I²C scan
    for (uint8_t addr = 1; addr < 127; addr++) {
      Wire.beginTransmission(addr);
      if (Wire.endTransmission() == 0 && !lastFound[addr]) {
        Serial.print("🟢 Device FOUND at 0x"); Serial.println(addr, HEX);
        lastFound[addr] = 1;
      } else if (Wire.endTransmission() != 0 && lastFound[addr]) {
        Serial.print("🔴 Device LOST at 0x"); Serial.println(addr, HEX);
        lastFound[addr] = 0;
      }
    }

    // BMP390 - PCA channel 0
    selectPCAChannel(BMP_CHANNEL);
    if (bmp.performReading()) {
      float tempF = bmp.temperature * 1.8 + 32;
      Serial.print("📈 BMP390 Temp: "); Serial.print(tempF, 1);
      Serial.print(" °F | Pressure: "); Serial.print(bmp.pressure / 100.0, 2); Serial.println(" hPa");
    } else {
      Serial.println("⚠️ BMP390 failed to read.");
    }

    // SCD41 - PCA channel 1
    selectPCAChannel(SCD_CHANNEL);
    if (scd41.readMeasurement()) {
      float tempF = scd41.getTemperature() * 1.8 + 32;
      Serial.print("🫁 CO₂: "); Serial.print(scd41.getCO2());
      Serial.print(" ppm | 💧 RH: "); Serial.print(scd41.getHumidity(), 1);
      Serial.print(" % | 🌡 Temp: "); Serial.print(tempF, 1); Serial.println(" °F");
    } else {
      Serial.println("⚠️ Waiting for SCD41 data...");
    }

    lastCheck = millis();
  }
}
