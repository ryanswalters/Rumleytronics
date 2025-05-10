#include <Wire.h>
#include <Adafruit_TCS34725.h>

#define SDA_PIN 8
#define SCL_PIN 9

Adafruit_TCS34725 tcs = Adafruit_TCS34725(
  TCS34725_INTEGRATIONTIME_50MS,
  TCS34725_GAIN_4X
);

float getHue(float r, float g, float b) {
  float maxVal = max(r, max(g, b));
  float minVal = min(r, min(g, b));
  float delta = maxVal - minVal;

  float hue = 0.0;

  if (delta == 0) return 0;
  if (maxVal == r) hue = 60 * fmod(((g - b) / delta), 6);
  else if (maxVal == g) hue = 60 * (((b - r) / delta) + 2);
  else if (maxVal == b) hue = 60 * (((r - g) / delta) + 4);

  if (hue < 0) hue += 360;
  return hue;
}

float getSaturation(float r, float g, float b) {
  float maxVal = max(r, max(g, b));
  float minVal = min(r, min(g, b));
  if (maxVal == 0) return 0;
  return (maxVal - minVal) / maxVal;
}

float getValue(float r, float g, float b) {
  return max(r, max(g, b));
}


String getColorName(uint16_t r, uint16_t g, uint16_t b, uint16_t c) {
  if (c == 0) return "Black";

  float R = (float)r / c;
  float G = (float)g / c;
  float B = (float)b / c;

  float hue = getHue(R, G, B);
  float sat = getSaturation(R, G, B);
  float val = getValue(R, G, B);

  if (val < 0.2 || sat < 0.15) return "Gray";  // too dark or too muted

  if (hue < 15 || hue > 345) return "Red";
  if (hue >= 15 && hue < 45) return "Orange";
  if (hue >= 45 && hue < 65) return "Yellow";
  if (hue >= 65 && hue < 170) return "Green";
  if (hue >= 170 && hue < 250) return "Blue";
  if (hue >= 250 && hue < 290) return "Purple";
  if (hue >= 290 && hue < 345) return "Magenta";

  return "Unknown";
}




String rgbToHex(uint16_t r, uint16_t g, uint16_t b) {
  uint8_t R = constrain((r * 255) / 1024, 0, 255);
  uint8_t G = constrain((g * 255) / 1024, 0, 255);
  uint8_t B = constrain((b * 255) / 1024, 0, 255);

  char hex[8];
  sprintf(hex, "#%02X%02X%02X", R, G, B);
  return String(hex);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(SDA_PIN, SCL_PIN);
  delay(300);

  if (!tcs.begin(0x29, &Wire)) {
    Serial.println("TCS34725 not found. Check wiring.");
    while (1);
  }

  Serial.println("TCS34725 ready!");
}

void loop() {
  uint16_t r, g, b, c;
  float lux, colorTemp;

  tcs.getRawData(&r, &g, &b, &c);
  lux = tcs.calculateLux(r, g, b);
  colorTemp = tcs.calculateColorTemperature_dn40(r, g, b, c);

  String colorName = getColorName(r, g, b, c);
  String hexColor = rgbToHex(r, g, b);
  float R = (float)r / c;
  float G = (float)g / c;
  float B = (float)b / c;

  Serial.print("R: "); Serial.print(r);
  Serial.print(" G: "); Serial.print(g);
  Serial.print(" B: "); Serial.print(b);
  Serial.print(" C: "); Serial.print(c);
  Serial.print("  Lux: "); Serial.print(lux);
  Serial.print("  CT: "); Serial.print(colorTemp);
  Serial.print(" K  →  ");
  Serial.print("Color: "); Serial.print(colorName);
  Serial.print("  HEX: "); Serial.println(hexColor);
  Serial.print(" [Norm RGB: ");
  Serial.print(r / (float)c, 2);
  Serial.print(", ");
  Serial.print(g / (float)c, 2);
  Serial.print(", ");
  Serial.print(b / (float)c, 2);
  Serial.println("]");
  Serial.print(" [Scaled RGB: ");
  Serial.print((int)(R * 255)); Serial.print(", ");
  Serial.print((int)(G * 255)); Serial.print(", ");
  Serial.print((int)(B * 255)); Serial.println("]");

  delay(1000);
}
