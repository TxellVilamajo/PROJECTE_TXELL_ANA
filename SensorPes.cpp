#include "SensorPes.h"

SensorPes::SensorPes(uint8_t pinDOUT, uint8_t pinSCK, float factorCalibracio, float capacitatMax)
  : _dout(pinDOUT), _sck(pinSCK), _factor(factorCalibracio), _capacitatMax(capacitatMax) {}

void SensorPes::iniciar_sensor_pes() {
  hx.begin(_dout, _sck);
  hx.set_scale(_factor);

  Serial.println("Tarant el sensor (assegura't que no hi ha pes damunt)...");
  delay(3000);
  hx.tare();
  Serial.println("Sensor preparat!");
}

float SensorPes::llegir_pes() {
  float pes = hx.get_units(10);
  return (pes < 0) ? 0 : pes;
}

float SensorPes::comprovar_nivell() {
  float pes = llegir_pes();
  float percentatge = (pes / _capacitatMax) * 100.0;
  return constrain(percentatge, 0, 100); // Assegura que no passa del 100%
}

float SensorPes::percentatge_a_pes(float percentatge) {
  return (_capacitatMax * percentatge) / 100.0;
}
