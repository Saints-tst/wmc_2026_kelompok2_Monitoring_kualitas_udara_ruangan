
#define BLYNK_TEMPLATE_ID "TMPL6W5QU0tVt"
#define BLYNK_TEMPLATE_NAME "Sistem Monitoring Kualitas Udara"
#define BLYNK_AUTH_TOKEN "55gNdJ6XRgYmMnavmjbZMZoSTmpPX1wX"

#include <WiFi.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>

#define GAS_PIN     32
#define DHT_PIN     33
#define DHT_TYPE    DHT22
#define BUZZER_PIN  25

char ssid[] = "Wokwi-GUEST";
char pass[] = "";

const int BATAS_BAIK = 1200;
const int BATAS_SEDANG = 2500;

LiquidCrystal_I2C lcd(0x27, 16, 2);
DHT dht(DHT_PIN, DHT_TYPE);

BlynkTimer timer;

bool statusAlarm = false;

void kirimDataBlynk() {
  int nilaiGas = analogRead(GAS_PIN);
  float suhu = dht.readTemperature();
  float kelembapan = dht.readHumidity();

  String kualitas;
  int statusKualitas;
  
  if (nilaiGas < BATAS_BAIK) {
    kualitas = "BAIK";
    statusKualitas = 1;
  } 
  else if (nilaiGas < BATAS_SEDANG) {
    kualitas = "SEDANG";
    statusKualitas = 3;
  } 
  else {
    kualitas = "BURUK";
    statusKualitas = 5;
  }

  // Kirim suhu dan kelembapan
  if (!isnan(suhu) && !isnan(kelembapan)) {
    Blynk.virtualWrite(V0, suhu);
    Blynk.virtualWrite(V3, (int)round(kelembapan));
  }

  // Kirim nilai gas
  Blynk.virtualWrite(V4, nilaiGas);

  // IAQ perkiraan berdasarkan ADC sensor gas.
  // Bukan pengukuran IAQ terkalibrasi.
  int nilaiIAQ = map(nilaiGas, 0, 4095, 0, 500);
  Blynk.virtualWrite(V5, nilaiIAQ);

  // Kode status untuk widget Air Quality Image
  Blynk.virtualWrite(V6, statusKualitas);

  // LCD
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Udara: ");
  lcd.print(kualitas);

  lcd.setCursor(0, 1);
  lcd.print("Gas:");
  lcd.print(nilaiGas);

  if (!isnan(suhu)) {
    lcd.print(" T:");
    lcd.print(suhu, 0);
  }

  // Serial Monitor
  Serial.println("----------------------");
  Serial.print("Gas: ");
  Serial.println(nilaiGas);

  Serial.print("Kualitas: ");
  Serial.println(kualitas);

  if (!isnan(suhu) && !isnan(kelembapan)) {
    Serial.print("Suhu: ");
    Serial.println(suhu);

    Serial.print("Kelembapan: ");
    Serial.println(kelembapan);
  }

  Serial.print("IAQ perkiraan: ");
  Serial.println(nilaiIAQ);
  
  Serial.println("----------------------");

  // Alarm
  if (nilaiGas >= BATAS_SEDANG) {
    statusAlarm = true;
    tone(BUZZER_PIN, 2000);
  } else {
    statusAlarm = false;
    noTone(BUZZER_PIN);
  }
}

void setup() {
  Serial.begin(115200);

  Wire.begin(12, 13);

  lcd.init();
  lcd.backlight();

  dht.begin();

  pinMode(GAS_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  noTone(BUZZER_PIN);

  lcd.setCursor(0, 0);
  lcd.print("MONITOR UDARA");
  lcd.setCursor(0, 1);
  lcd.print("Menghubungkan");

  Serial.println("Menghubungkan ke Blynk...");

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);

  lcd.clear();
  lcd.print("Blynk Connected");

  Serial.println("Blynk Connected!");

  timer.setInterval(2000L, kirimDataBlynk);
}

void loop() {
  Blynk.run();
  timer.run();
}
