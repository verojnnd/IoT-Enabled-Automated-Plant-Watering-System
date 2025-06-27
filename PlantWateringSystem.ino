#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <BlynkSimpleEsp8266.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Pin Definitions
const int soilMoisturePin = A0;
const int buttonCabe = D7;
const int buttonJagung = D6;
const int buttonSemangka = D5;
const int buttonRumput = D4;
const int buttonBawang = D3;
const int switchPin = D8;  // Pin untuk saklar
const int relayPin = RX;    // Pin untuk relay

// Moisture thresholds for each plant
int moistureLimits[5][2] = {
  {60, 75},   // Cabe
  {75, 85},   // Jagung
  {60, 70},   // Semangka
  {60, 70},   // Rumput
  {50, 65}    // Bawang
};

int selectedPlant = -1; // -1 means no plant selected
bool systemOn = false;

void setup() {
  Serial.begin(115200);
  Blynk.begin("your_auth_token", "SSID", "PASSWORD"); // Ganti dengan token Blynk dan kredensial hotspot
  display.begin(SSD1306_I2C_ADDRESS, OLED_RESET);
  pinMode(switchPin, INPUT_PULLUP);
  pinMode(relayPin, OUTPUT);
  digitalWrite(relayPin, LOW); // Pompa mati saat awal
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
}

void loop() {
  Blynk.run();
  
  if (digitalRead(switchPin) == LOW) { // jika saklar dihidupkan
    displayMenu();
    systemOn = true;
  }

  if (selectedPlant != -1) {
    int soilMoistureValue = soilMoistureSensor();
    managePump(soilMoistureValue);
    displayMoisture(soilMoistureValue);
    
    // Kirim data ke Blynk
    Blynk.virtualWrite(V0, soilMoistureValue); // Ganti V0 sesuai dengan virtual pin yang Anda gunakan di Blynk
    delay(1000);
  }
}

void displayMenu() {
  display.clearDisplay();
  display.setCursor(0, 0);
  display.println("Pilih Tanaman");
  display.println("1. Cabe");
  display.println("2. Jagung");
  display.println("3. Semangka");
  display.println("4. Rumput");
  display.println("5. Bawang");
  display.display();
  
  if (digitalRead(buttonCabe) == LOW) selectedPlant = 0;
  else if (digitalRead(buttonJagung) == LOW) selectedPlant = 1;
  else if (digitalRead(buttonSemangka) == LOW) selectedPlant = 2;
  else if (digitalRead(buttonRumput) == LOW) selectedPlant = 3;
  else if (digitalRead(buttonBawang) == LOW) selectedPlant = 4;
}

void displayMoisture(int moistureValue) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.print("Tanaman: ");
  display.println(selectedPlant == 0 ? "Cabe" : selectedPlant == 1 ? "Jagung" : selectedPlant == 2 ? "Semangka" : selectedPlant == 3 ? "Rumput" : "Bawang");
  display.print("Kelembaban: ");
  display.print(moistureValue);
  display.print("%");
  display.display();
}

int soilMoistureSensor() {
  int soilMoistureValue = analogRead(soilMoisturePin);
  soilMoistureValue = map(soilMoistureValue, 0, 1024, 0, 100);
  soilMoistureValue = (soilMoistureValue - 100) * -1; // 0 = Basah, 100 = Kering
  return soilMoistureValue;
}

void managePump(int moistureValue) {
  if (moistureValue < moistureLimits[selectedPlant][0]) {
    digitalWrite(relayPin, HIGH); // Nyalakan pompa
  } else if (moistureValue > moistureLimits[selectedPlant][1]) {
    digitalWrite(relayPin, LOW); // Matikan pompa
  }
}
