#include "Dispensador.h"
#include <Arduino.h>

Dispensador::Dispensador(int pinServo, int canalPWM, int freq, int resolucioBits)
  : _pinServo(pinServo), _canal(canalPWM), _freq(freq), _resolucio(resolucioBits) {} 

void Dispensador::iniciar() { 
  ledcSetup(_canal, _freq, _resolucio);
  ledcAttachPin(_pinServo, _canal);
  moureAngle(0);  // Posició inicial
}

// Quan volem obrir el dispensador hem de moure el servomotor 180º
void Dispensador::obrir() {
  moureAngle(180);  // Mètode definit a la línia 23 del codi
}

// Quan volem tencar el dispensador hem de tornar el servomotor a la seva posició originalS
void Dispensador::tancar() {
  moureAngle(0); // Mètode definit a la línia 23 del codi
}

void Dispensador::moureAngle(int angle) {
  float ampladaPols = 0.5 + (angle / 180.0) * 2.0;  // en ms
  int duty = (ampladaPols / (1000.0 / _freq)) * (1 << _resolucio);
  ledcWrite(_canal, duty);
}
