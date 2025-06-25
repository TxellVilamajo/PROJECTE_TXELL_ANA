#ifndef HORARIS_H
#define HORARIS_H

#include <Arduino.h>
#include <time.h>

class Horaris {
  public:
    void iniciarHora();
    void mostraHoraActual();
    bool esHoraActual(int hora, int minut);

    int getHora();
    int getMinut();

    int horaAnterior = -1;
    int minutAnterior = -1;
};
#endif
