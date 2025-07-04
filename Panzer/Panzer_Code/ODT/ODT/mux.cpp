#include <Wire.h>               // I2C-kommunikation
#include "mux.h"                // Egna definitioner och typer (t.ex. MUX_ADDR)
#include "vl53l4cd_class.h"     // Bibliotek för VL53L4CD-sensorer
#include "evade.h"

// Två sensorer anslutna via multiplexer – kanal 0 (vänster) och kanal 3 (höger)
VL53L4CD vl53_left(&Wire, -1);   // Vänster sensor
VL53L4CD vl53_right(&Wire, -1);  // Höger sensor

// Global array för sparade mätvärden [0]=vänster, [1]=höger
uint16_t vl53Distances[2] = {0, 0};

void selectMuxChannel(uint8_t channel) {
  // Aktiverar en specifik kanal på I2C-multiplexern
  Wire.beginTransmission(MUX_ADDR);   // Startar I2C-kommunikation med mux
  Wire.write(1 << channel);           // Sätter bit som motsvarar vald kanal
  Wire.endTransmission();             // Avslutar överföring
}

void initVL53Sensors() {
  // === Initiera VL53 vänster (kanal 0) ===
  selectMuxChannel(0);                         // Välj kanal 0 på mux
  vl53_left.VL53L4CD_Off();                    // Nollställ sensorn
  vl53_left.VL53L4CD_On();                     // Starta sensorn
  if (vl53_left.InitSensor() == 0) {           // Initiering lyckades
    vl53_left.VL53L4CD_SetRangeTiming(15, 0);  // Snabb mätning (15 ms)
    vl53_left.VL53L4CD_StartRanging();         // Starta kontinuerlig mätning
    Serial.println("✅ VL53 vänster initierad");
  }

  // === Initiera VL53 höger (kanal 3) ===
  selectMuxChannel(3);                         // Välj kanal 3 på mux
  vl53_right.VL53L4CD_Off();                   // Nollställ sensorn
  vl53_right.VL53L4CD_On();                    // Starta sensorn
  if (vl53_right.InitSensor() == 0) {          // Initiering lyckades
    vl53_right.VL53L4CD_SetRangeTiming(15, 0); // Snabb mätning
    vl53_right.VL53L4CD_StartRanging();        // Starta kontinuerlig mätning
    Serial.println("✅ VL53 höger initierad");
  }
}

MuxStatus checkVL53Obstacles() {
  // === Tröskelvärden för avstånd och signalstyrka ===
  const uint16_t CRIT_LEFT  = VL53_CRIT_LEFT;    // t.ex. 100 mm
  const uint16_t CRIT_RIGHT = VL53_CRIT_RIGHT;   // t.ex. 100 mm
  const uint16_t SIGNAL_MIN = 2;                // Minsta godkända signalstyrka (kcps)

  VL53L4CD_Result_t result;  // Struktur för mätresultat
  uint8_t ready = 0;         // Flagga för datatillgänglighet

  // === Läs vänster sensor ===
  selectMuxChannel(0);                                 // Välj kanal 0 (vänster)
  vl53_left.VL53L4CD_CheckForDataReady(&ready);        // Kontrollera om data är redo
  if (ready) {
    vl53_left.VL53L4CD_GetResult(&result);             // Läs mätresultat
    vl53_left.VL53L4CD_ClearInterrupt();               // Rensa avbrott

    // Skriv ut debug-info
    Serial.print("📏 VL53 V: ");
    Serial.print(result.distance_mm);
    Serial.print(" mm | Signal: ");
    Serial.println(result.signal_per_spad_kcps);

    // Kontrollera signalstyrka och spara avstånd
    if (result.signal_per_spad_kcps >= SIGNAL_MIN) {
      vl53Distances[0] = result.distance_mm;
    } else {
      Serial.println("⚠️ VL53 vänster: signal ");
      Serial.println(SIGNAL_MIN);
      Serial.println(" → mätning ignoreras");
      vl53Distances[0] = INVALID_DISTANCE;  // Ignorera ogiltig mätning
    }

    // Kontrollera om avståndet är kritiskt
    if (vl53Distances[0] > 0 && vl53Distances[0] < CRIT_LEFT) {
      return MUX_CRITICAL_LEFT;
    }
  }

  // === Läs höger sensor ===
  ready = 0;                                           // Återställ ready-flagga
  selectMuxChannel(3);                                // Välj kanal 3 (höger)
  vl53_right.VL53L4CD_CheckForDataReady(&ready);
  if (ready) {
    vl53_right.VL53L4CD_GetResult(&result);
    vl53_right.VL53L4CD_ClearInterrupt();

    Serial.print("📏 VL53 H: ");
    Serial.print(result.distance_mm);
    Serial.print(" mm | Signal: ");
    Serial.println(result.signal_per_spad_kcps);

    if (result.signal_per_spad_kcps >= SIGNAL_MIN) {
      vl53Distances[1] = result.distance_mm;
    } else {
      Serial.println("⚠️ VL53 höger: signal < ");
      Serial.println(SIGNAL_MIN);
      Serial.println(" → mätning ignoreras");
      vl53Distances[1] = INVALID_DISTANCE;
    }

    if (vl53Distances[1] > 0 && vl53Distances[1] < CRIT_RIGHT) {
      return MUX_CRITICAL_RIGHT;
    }
  }

  return MUX_CLEAR;  // Inget kritiskt hinder detekterat
}
