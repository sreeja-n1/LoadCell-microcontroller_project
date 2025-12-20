#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include "HX711.h"

// --- HX711 pins ---
#define DT 3
#define SCK 2

HX711 scale;
LiquidCrystal_I2C lcd(0x27, 16, 2); // Change 0x27 to 0x3F if your LCD doesn't display

// --- Calibration Factor (adjust after calibration) ---
float calibration_factor = 500.0;

// --- Item Weights in grams ---
const float lipbalm = 4.0;
const float biscuit = 120.0;
const float phone   = 170.0;
const float lotion  = 210.0;

// Total weight when all items are present (grams)
float expected_total_g = lipbalm + biscuit + phone + lotion;

// --- Tolerance to handle small sensor fluctuations ---
const float tolerance = 5.0; // grams

void setup() {
  Serial.begin(9600);
  lcd.init();
  lcd.backlight();

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Smart Fridge Init");
  delay(1500);

  // Initialize HX711
  scale.begin(DT, SCK);
  scale.set_scale(calibration_factor);
  scale.tare();  // Reset scale to 0 with empty shelf

  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Calibrating...");
  delay(2000);

  lcd.clear();
  lcd.print("Ready to use!");
  delay(1000);
}

void loop() {
  // Take average of multiple samples for smooth readings
  float current_weight_g = scale.get_units(15);
  if (current_weight_g < 0) current_weight_g = 0;

  // Calculate missing weight
  float missing_g = expected_total_g - current_weight_g;
  if (missing_g < tolerance) missing_g = 0; // Ignore tiny variations

  // Detect missing item
  String missing_item = "All Items OK";

  if (missing_g >= (lipbalm - tolerance) && missing_g < (biscuit - tolerance)) {
    missing_item = "Lip Balm Missing";
  }
  else if (missing_g >= (biscuit - tolerance) && missing_g < (biscuit + tolerance)) {
    missing_item = "Biscuit Missing";
  }
  else if (missing_g >= (biscuit + tolerance) && missing_g < (biscuit + phone - tolerance)) {
    missing_item = "Phone Missing";
  }
  else if (missing_g >= (biscuit + phone - tolerance) && missing_g < (biscuit + phone + lotion - tolerance)) {
    missing_item = "Lotion Missing";
  }
  else if (missing_g > (biscuit + phone + lotion - tolerance)) {
    missing_item = "Multiple Items Missing";
  }

  // --- Display results on LCD ---
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Weight: ");
  lcd.print(current_weight_g / 1000.0, 3);
  lcd.print("kg");

  lcd.setCursor(0, 1);
  lcd.print(missing_item);

  // --- Debug info on Serial Monitor ---
  Serial.print("Current: ");
  Serial.print(current_weight_g, 1);
  Serial.print("g | Missing: ");
  Serial.print(missing_g, 1);
  Serial.print("g | ");
  Serial.println(missing_item);

  delay(1500); // Update every 1.5s
}
