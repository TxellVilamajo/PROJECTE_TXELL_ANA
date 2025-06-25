#include "PantallaLCD.h"
#include <Arduino.h>
PantallaLCD::PantallaLCD(uint8_t adrecaI2C, uint8_t columnes, uint8_t files, uint8_t pinSDA, uint8_t pinSCL)
  : _lcd(adrecaI2C, columnes, files), _sda(pinSDA), _scl(pinSCL) {}

void PantallaLCD::iniciar_pantalla() {
  Wire.begin(_sda, _scl);
  _lcd.init();
  _lcd.backlight();

  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print("Benvingut!");
  delay(2000);
  _lcd.clear();
}

// Aquesta funció permet mostrar un missatge entrat com a paràmetre per pentalla
void PantallaLCD::mostrar_missatge(const String& missatge) { 
  _lcd.clear();
  _lcd.setCursor(0, 0);
  _lcd.print(missatge);
}
