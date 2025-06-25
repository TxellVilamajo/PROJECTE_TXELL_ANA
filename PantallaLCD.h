#ifndef PANTALLALCD_H
#define PANTALLALCD_H

#include <Arduino.h>
#include <Wire.h>
#include "LiquidCrystal_I2C.h"

class PantallaLCD {
public:
  PantallaLCD(uint8_t adrecaI2C, uint8_t columnes, uint8_t files, uint8_t pinSDA, uint8_t pinSCL);

  void iniciar_pantalla();
  void mostrar_missatge(const String& missatge);

private:
  LiquidCrystal_I2C _lcd;
  uint8_t _sda;
  uint8_t _scl;
};

#endif
