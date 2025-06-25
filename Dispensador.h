#ifndef DISPENSADOR_H
#define DISPENSADOR_H

#include <Arduino.h>

class Dispensador {
public:
  // Constructor amb tots els paràmetres configurables del servomotor
  Dispensador(int pinServo, int canalPWM, int freq = 50, int resolucioBits = 12);

  void iniciar();
  void obrir();   // Mou a 180 graus
  void tancar();  // Torna a 0 graus

private:
  void moureAngle(int angle);  //Mètode que permet obrir i tencar el servomotor especificant l'angle desitjat

  int _pinServo;
  int _canal;
  int _freq;
  int _resolucio;
};

#endif
