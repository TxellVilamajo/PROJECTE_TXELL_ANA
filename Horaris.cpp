#include "Horaris.h"
#include <NTPClient.h> // o el que facis servir per obtenir l'hora
#include <WiFiUdp.h>

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3600, 60000); // ajusta segons zona

void Horaris::iniciarHora() {
  timeClient.begin();
}

void Horaris::mostraHoraActual() {
  timeClient.update();
  Serial.printf("🕒 Hora actual: %02d:%02d\n", getHora(), getMinut());
}

bool Horaris::esHoraActual(int hora, int minut) {
  timeClient.update();
  return getHora() == hora && getMinut() == minut;
}

int Horaris::getHora() {
  return timeClient.getHours();
}

int Horaris::getMinut() {
  return timeClient.getMinutes();
}
