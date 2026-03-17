# Flight Control Computer (FCC) — Sounding Rocket Project

Projekt  komputera pokładowego (Flight Computer) zaprojektowanego do gromadzenia danych telemetrycznych podczas lotów rakiet modelarskich.

## 🚀 Główne Cele Projektu
* **Akwizycja danych:** Pomiar ciśnienia statycznego, temperatury oraz przyspieszenia liniowego w czasie rzeczywistym.
* **Rejestracja danych:** Zapis parametrów na kartę SD z częstotliwością 10 Hz w formacie CSV.
* **Wytrzymałość:** Konstrukcja przystosowana do pracy przy przeciążeniach rzędu 8-10 G.
* **Platforma testowa:** Zaprojektowanie rakiety nośnej oraz systemu odzysku do bezpiecznych testów elektroniki.

## 🛠 Architektura Systemu (Hardware)
Elektronika została wykonana w układzie "kanapkowym" (stack), co pozwoliło na znaczną miniaturyzację i zmieszczenie układu w korpusie o ograniczonej średnicy.

* **Jednostka obliczeniowa:** Arduino Nano (ATmega328P) taktowane zegarem 16 MHz.
* **Czujnik ciśnienia (Barometr):** MS5607 (komunikacja I2C) do precyzyjnego wyznaczania wysokości n.p.m.
* **Jednostka IMU:** MPU-6050 (3-osiowy akcelerometr i żyroskop) monitorujący dynamikę lotu.
* **Pamięć:** Moduł karty MicroSD (SPI) pełniący rolę "czarnej skrzynki".
* **Zasilanie:** Pakiet LiPo 2s 300 mAh.

## 💻 Oprogramowanie i Algorytmy
Firmware napisany w języku C++ kładzie nacisk na determinizm czasowy pętli pomiarowej.

* **Auto-kalibracja:** Przy starcie system wykonuje 50 pomiarów tła, ustalając poziom odniesienia ($H_0$) dla sensorów.
* **Fuzja danych:** Wykorzystanie modelu atmosfery wzorcowej do obliczeń wysokości:
  $$h=44330\cdot\left(1-\left(\frac{P_{aktualne}}{P_{bazowe}}\right)^{0.1903}\right)$$
* **Logowanie:** Dane są zapisywane na kartę SD w formacie CSV, co ułatwia późniejszą analizę w oprogramowaniu zewnętrznym.

## 📊 Wyniki Testów w Locie
System został zweryfikowany podczas lotu testowego z użyciem silnika klasy B4-4. Dane potwierdziły poprawność działania elektroniki w całym profilu lotu.

* **Apogeum:** Maksymalna zmierzona wysokość wyniosła 93,9 m (czas T+4,77 s).
* **Maksymalne przyspieszenie:** Akcelerometr zarejestrował fazę pracy silnika ze szczytową wartością $78,2~m/s^2$ (ok. 8 G).
* **Stabilność:** Nie odnotowano resetów mikrokontrolera ani przerw w zapisie danych mimo silnych wibracji.

| Faza lotu | Czas [s] | Wysokość [m] | Przyspieszenie Z [$m/s^2$] |
| :--- | :--- | :--- | :--- |
| Oczekiwanie | 0.00 | 0.00 | 0.05 |
| Zapłon silnika | 1.10 | 1.50 | 45.40 |
| Maksymalny ciąg | 1.20 | 5.40 | 78.20 |
| Apogeum | 5.77 | 93.90 | -9.80 |
| Opadanie (spadochron)| 15.00 | 41.00 | -0.20 |
| Przyziemienie | 22.10 | 0.00 | 4.50 |

## 📁 Struktura Repozytorium
* `/kontrola_lotu` — Pliki źródłowe firmware'u dla Arduino.
* `/Schemat_komputera_kontroli_lotu` — Schemat połączeń magistral I2C oraz SPI.
* `/FCC_raport` — Pełna dokumentacja techniczna i analiza danych.

---
*Projekt stanowi bazę do rozwoju systemów aktywnej stabilizacji lotu (np. TVC) oraz zaawansowanej telemetrii.*
