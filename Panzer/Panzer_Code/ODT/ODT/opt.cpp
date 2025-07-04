#include "opt.h"  // Inkluderar deklarationer för OPT3101-funktionalitet

OPT3101 opt;  // Skapar ett objekt för OPT3101-sensorn

// Array för att spara avståndsvärden från varje kanal
// [0] = vänster, [1] = framåt, [2] = höger
uint16_t optDistances[3] = {9999, 9999, 9999};  // Initieras till ogiltiga värden

void initOPT() {
  opt.setAddress(0x58);  // Standardadress för OPT3101
  opt.init();            // Initierar sensorn

  if (opt.getLastError()) {  // Kontrollera om initiering misslyckades
    Serial.println("❌ OPT3101 init misslyckades!");
    return;
  }

  opt.setFrameTiming(64);  // Sätter högre mätfrekvens
  opt.setBrightness(OPT3101Brightness::High);  // Max ljusstyrka (för bättre räckvidd)

  Serial.println("✅ OPT3101 initierad");
}

OptStatus checkOptObstacles() {
  // Tröskelvärden för när avståndet anses "kritiskt"
  const uint16_t CRIT_FRONT  = 300;  // mm
  const uint16_t CRIT_LEFT   = 450;
  const uint16_t CRIT_RIGHT  = 450;

  // Filtreringsgränser
  const uint16_t MIN_AMPLITUDE = 40;     // Lägsta giltiga signalstyrka
  const uint16_t MAX_DISTANCE  = 1000;   // Max giltigt avstånd

  OptStatus lastCritical = OPT_CLEAR;  // Standardläge – inget hinder

  // Loopar igenom alla tre kanaler: 0=LEFT, 1=FRONT, 2=RIGHT
  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);   // Välj aktuell kanal
    opt.sample();         // Utför mätning (blockerande)

    // Läs ut mätdata från sensorn
    uint16_t dist = opt.distanceMillimeters;
    uint16_t amp  = opt.amplitude;
    uint16_t amb  = opt.ambient;

    optDistances[ch] = dist;  // Spara distans i array

    // Skriv ut rådata för debug
    Serial.print("OPT ch ");
    Serial.print(ch);
    Serial.print(": dist = ");
    Serial.print(dist);
    Serial.print(" mm | amp = ");
    Serial.print(amp);
    Serial.print(" | amb = ");
    Serial.println(amb);

    // Ignorera mätningar med för svag signal eller utanför giltigt område
    if (amp < MIN_AMPLITUDE || dist > MAX_DISTANCE || dist == 0) {
      Serial.println("⚠️ Ogiltig mätning – ignorerar");
      continue;
    }

    // Om FRONT är inom kritiskt avstånd, flagga detta oavsett tidigare status
    if (ch == 1 && dist < CRIT_FRONT) {
      Serial.println("✅ OPT: FRONT kritisk");
      lastCritical = OPT_CRITICAL_FRONT;
    }

    // Om LEFT är kritisk men ingen annan redan flaggad
    if (ch == 0 && dist < CRIT_LEFT && lastCritical == OPT_CLEAR) {
      Serial.println("✅ OPT: VÄNSTER kritisk");
      lastCritical = OPT_CRITICAL_LEFT;
    }

    // Om RIGHT är kritisk men ingen annan redan flaggad
    if (ch == 2 && dist < CRIT_RIGHT && lastCritical == OPT_CLEAR) {
      Serial.println("✅ OPT: HÖGER kritisk");
      lastCritical = OPT_CRITICAL_RIGHT;
    }
  }

  return lastCritical;  // Returnera status: FRONT / LEFT / RIGHT / CLEAR
}

void readOPTSensors() {
  // Läs in alla tre kanaler för optiska avstånd
  for (int ch = 0; ch < 3; ch++) {
    opt.setChannel(ch);  // Välj kanal
    opt.sample();        // Mät avstånd (blockerande)

    uint16_t dist = opt.distanceMillimeters;
    uint16_t amp  = opt.amplitude;

    // Ogiltig mätning: för låg amplitud eller nollvärde
if (amp < 100 || dist == 0 || dist == 9999) {
  Serial.print("⚠️ OPT ch "); Serial.print(ch);
  Serial.println(": ogiltig mätning → antas mycket nära");
  optDistances[ch] = 50;  // Simulera "extremt nära" hinder
} else {
  optDistances[ch] = dist;  // Spara giltigt värde
}

    // Skriv ut resultat för aktuell kanal
    Serial.print("OPT ch ");
    Serial.print(ch);
    Serial.print(": dist = ");
    Serial.print(optDistances[ch]);
    Serial.println(" mm");
  }
}
