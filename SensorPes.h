#ifndef SENSORPES_H
#define SENSORPES_H

#include <Arduino.h>
#include "HX711.h"

class SensorPes {
  public:
    SensorPes(uint8_t pinDOUT, uint8_t pinSCK, float factorCalibracio, float capacitatMax);

    void iniciar_sensor_pes();
    float comprovar_nivell();   // Retorna % del pes respecte a capacitat màxima
    float llegir_pes();    // Retorna el pes en grams
    float percentatge_a_pes(float percentatge); // Converteix % a grams

  private:
    HX711 hx;
    uint8_t _dout, _sck;
    float _factor;
    float _capacitatMax; // grams màxims (ex: 300g)
};

#endif
