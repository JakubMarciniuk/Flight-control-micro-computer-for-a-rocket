#include <Wire.h>
#include <SPI.h>
#include <SD.h>

// --- KONFIGURACJA ---
const int PIN_CS_SD = 10;   // Pin karty SD
const int PIN_BUZZER = 5;   // Buzzer
#define BARO_ADDR 0x77      // Adres MS5607
#define MPU_ADDR  0x68      // Adres MPU6050

// --- ZMIENNE ---
uint16_t C[7];              // Kalibracja barometru
float cisnienieBazowe = 0;
float offsetAccZ = 0;       
unsigned long timer = 0;
File plikSD;

void setup() {
  Serial.begin(9600);
  pinMode(PIN_BUZZER, OUTPUT);
  Wire.begin();
  
  // Sygnał startu zasilania
  tone(PIN_BUZZER, 4000, 100); delay(100); noTone(PIN_BUZZER);
  Serial.println("\n=== KOMPUTER LOTU v2.2 (Z CZASEM) ===");

  // 1. KARTA SD
  Serial.print("1. Karta SD... ");
  if (!SD.begin(PIN_CS_SD)) {
    Serial.println("BLAD KARTY!");
    alarm(1);
  }
  Serial.println("OK.");
  
  // Nagłówek pliku - ZMIANA: Czas[s] zamiast ms
  plikSD = SD.open("lot.txt", FILE_WRITE);
  if (plikSD) {
    plikSD.println("Czas[s],Temp[C],Cisn[hPa],Wys[m],Acc[m/s2]");
    plikSD.close();
  }

  // 2. BAROMETR (Inicjalizacja)
  Serial.print("2. Barometr... ");
  Wire.beginTransmission(BARO_ADDR); Wire.write(0x1E); Wire.endTransmission(); delay(100);
  for (int i = 0; i < 7; i++) {
    Wire.beginTransmission(BARO_ADDR); Wire.write(0xA0 + (i * 2)); Wire.endTransmission();
    Wire.requestFrom(BARO_ADDR, 2);
    if(Wire.available()>=2) C[i] = (Wire.read() << 8) | Wire.read();
  }
  if (C[1] > 0) Serial.println("OK.");
  else { Serial.println("BLAD BAROMETRU!"); alarm(2); }

  // 3. MPU6050
  Serial.print("3. Akcelerometr... ");
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x6B); Wire.write(0);
  if (Wire.endTransmission() == 0) Serial.println("OK.");
  else { Serial.println("BLAD MPU!"); alarm(3); }

  // 4. KALIBRACJA ZERA (3 sekundy)
  Serial.println("4. Kalibracja (Nie ruszaj!)...");
  float sumaP = 0, sumaA = 0;
  for(int i=0; i<50; i++) {
    float p, t;
    czytajBarometr(&p, &t);
    sumaP += p;
    sumaA += czytajAcc();
    delay(20);
  }
  cisnienieBazowe = sumaP / 50.0;
  offsetAccZ = sumaA / 50.0;
  
  Serial.println("GOTOWE. System dziala.");
  tone(PIN_BUZZER, 2000, 800); delay(800); noTone(PIN_BUZZER);
  
  Serial.println("\nMONITOR DANYCH:");
  Serial.println("Czas[s] | Temp[C] | Cisn[hPa] | Wys[m] | Acc[m/s2]");
}

void loop() {
  unsigned long czasTeraz = millis();

  // --- 1. SZYBKA FIZYKA (Dla Przyspieszenia) ---
  float accG = czytajAcc();
  float accMSS = (accG - offsetAccZ) * 9.81;
  
  // Deadzone
  if (abs(accMSS) < 0.2) accMSS = 0;
  
  // --- 2. ZAPIS I WYSWIETLANIE (Co 200ms) ---
  if (czasTeraz - timer >= 200) {
    timer = czasTeraz;

    // Przeliczenie czasu na sekundy
    float czasSekundy = czasTeraz / 1000.0;

    // Odczyt Barometru
    float pres, temp;
    czytajBarometr(&pres, &temp);
    
    // Obliczanie Wysokości
    if (pres == 0) pres = cisnienieBazowe; 
    float alt = 44330.0 * (1.0 - pow(pres / cisnienieBazowe, 0.1903));

    // A. Wyświetlanie na ekranie
    Serial.print(czasSekundy, 2); Serial.print("s   \t"); // Czas w sekundach
    Serial.print(temp, 1); Serial.print("\t  ");
    Serial.print(pres, 1); Serial.print("\t      ");
    Serial.print(alt, 1);  Serial.print("\t ");
    Serial.println(accMSS, 1);

    // B. Zapis na kartę SD
    plikSD = SD.open("lot.txt", FILE_WRITE);
    if (plikSD) {
      plikSD.print(czasSekundy, 2); plikSD.print(","); // Zapisujemy sekundy
      plikSD.print(temp, 2);   plikSD.print(",");
      plikSD.print(pres, 2);   plikSD.print(",");
      plikSD.print(alt, 2);    plikSD.print(",");
      plikSD.println(accMSS, 2);
      plikSD.close();
    }
  }
}

// --- FUNKCJE ---

void alarm(int n) {
  while(true) {
    for(int i=0; i<n; i++) { tone(PIN_BUZZER, 1000, 200); delay(300); }
    delay(1000);
  }
}

float czytajAcc() {
  Wire.beginTransmission(MPU_ADDR); Wire.write(0x3B + 4); Wire.endTransmission(false);
  Wire.requestFrom(MPU_ADDR, 2, true);
  int16_t raw = Wire.read()<<8 | Wire.read();
  return raw / 16384.0;
}

void czytajBarometr(float *p, float *t) {
  uint32_t D1=0, D2=0;
  
  Wire.beginTransmission(BARO_ADDR); Wire.write(0x48); Wire.endTransmission(); delay(10);
  Wire.beginTransmission(BARO_ADDR); Wire.write(0x00); Wire.endTransmission();
  Wire.requestFrom(BARO_ADDR, 3);
  if(Wire.available()>=3) D1 = ((uint32_t)Wire.read()<<16)|((uint32_t)Wire.read()<<8)|Wire.read();
  
  Wire.beginTransmission(BARO_ADDR); Wire.write(0x58); Wire.endTransmission(); delay(10);
  Wire.beginTransmission(BARO_ADDR); Wire.write(0x00); Wire.endTransmission();
  Wire.requestFrom(BARO_ADDR, 3);
  if(Wire.available()>=3) D2 = ((uint32_t)Wire.read()<<16)|((uint32_t)Wire.read()<<8)|Wire.read();

  if (D1 == 0 || D2 == 0) {
    *p = cisnienieBazowe; 
    *t = 0;
    return;
  }

  int64_t dT = (int64_t)D2 - ((int64_t)C[5] << 8);
  int64_t TEMP = 2000 + ((int64_t)dT * C[6]) / 8388608;
  int64_t OFF = ((int64_t)C[2] << 17) + (((int64_t)C[4] * dT) >> 6);
  int64_t SENS = ((int64_t)C[1] << 16) + (((int64_t)C[3] * dT) >> 7);
  int64_t P = (((D1 * SENS) >> 21) - OFF) >> 15;
  
  *t = (float)TEMP / 100.0;
  *p = (float)P / 100.0;
}