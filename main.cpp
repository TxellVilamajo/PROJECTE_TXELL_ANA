#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>

// INCOLEM TOTES LES CLASSES IMPLEMENTADES
#include "Dispensador.h"
#include "PantallaLCD.h"
#include "Horaris.h"
#include "SensorPes.h"

// DEFINIM EL NOM I LA CONTRASENYA DE LA XARXA WIFI CREADA PEL MICROPROCESSADOR ESP32-S3
const char* ap_ssid = "DISPENSADOR_ESP32";
const char* ap_password = "12345678";

// DEFINIM LES CREDENCIALS PER PODER CONNECTAR-SE AL WIFI
const char* sta_ssid = "FOCentelles_0631-ext";
const char* sta_password = "81551217";

// DEFINIM ELS OBJECTES DE CADA CLASSE QUE NECESSITEM I ELS PINS CORRESPONENTS
WebServer server(80);
Horaris horari;
Dispensador dispensador_menjar(9, 0);
Dispensador dispensador_aigua(10, 1);
PantallaLCD pantalla(0x27, 20, 4, 21, 19);

#define DOUT_MENJAR 4
#define SCK_MENJAR  5
#define DOUT_AIGUA  6
#define SCK_AIGUA   7

#define PES_MAX_MENJAR 300.0
#define PES_MAX_AIGUA  300.0
#define FACTOR_CALIBRACIO_MENJAR -15.35
#define FACTOR_CALIBRACIO_AIGUA -14.35

SensorPes sensorMenjar(DOUT_MENJAR, SCK_MENJAR, FACTOR_CALIBRACIO_MENJAR, PES_MAX_MENJAR);
SensorPes sensorAigua(DOUT_AIGUA, SCK_AIGUA, FACTOR_CALIBRACIO_AIGUA, PES_MAX_AIGUA);

// INICIALITZEM ELS HORARIS A -1 (HORARI INEXISTENT, PER TAL QUE AFECTI AL FUNICONAMENT)
int horaMenjar = -1, minutMenjar = -1;
int horaAigua = -1, minutAigua = -1;


unsigned long ultimaComprobacio = 0;
bool menjarDispensat = false;
bool aiguaDispensada = false;

unsigned int contadorMenjar = 0;
float gramsMenjarTotal = 0.0;
unsigned int contadorAigua = 0;
float mlAiguaTotal = 0.0;

// CREEM LA PAGINA WEB
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html><html lang="ca"><head><meta charset="UTF-8">
<title>PetFeeder</title><style>
body { font-family: sans-serif; background: #f0f0f0; padding: 2em; }
input, button { padding: 10px; margin: 5px; }
.btn { background: #4caf50; color: white; border: none; border-radius: 5px; }
.btn-force { background: #f44336; color: white; border: none; border-radius: 5px; }
</style></head><body>
<h1>PetFeeder</h1>
<button class='btn' onclick="dispensar('menjar')">Afegir Menjar</button>
<button class='btn' onclick="dispensar('aigua')">Afegir Aigua</button>
<button class='btn-force' onclick="forcar('menjar')">For&ccedil;a Menjar</button>
<button class='btn-force' onclick="forcar('aigua')">For&ccedil;a Aigua</button><br><br>
<form action='/setHorari' method='GET'>
  Menjar (HH:MM): <input type='text' name='menjar'><br>
  Aigua (HH:MM): <input type='text' name='aigua'><br>
  <input class='btn' type='submit' value='Guardar horaris'>
</form>
<h3>🕒 Horaris programats:</h3>
<ul>
  <li>Menjar: %02d:%02d</li>
  <li>Aigua: %02d:%02d</li>
</ul>
<h3>📊 Estadístiques de consum:</h3>
<button class='btn' onclick="veureEstadistiques()">Veure estadístiques</button>
<button class='btn-force' onclick="resetEstadistiques()">Reiniciar estadístiques</button>
<!-- <canvas id="grafica" width="400" height="200"></canvas> -->
<pre id="estadistiques"></pre>
<p id="estat"></p>
<script>
function dispensar(tipus) {
  fetch('/' + tipus).then(res => {
    document.getElementById('estat').innerText = "S'ha enviat la comanda per afegir " + tipus;
  });
}
function forcar(tipus) {
  fetch('/forca_' + tipus).then(res => {
    document.getElementById('estat').innerText = "S'ha enviat la comanda for&ccedil;ada per " + tipus;
  });
}
function veureEstadistiques() {
  fetch('/estadistiques').then(res => res.text()).then(data => {
    document.getElementById('estadistiques').innerText = data;
  });
  /*
  // Comentat per no mostrar gràfic
  fetch('/estadistiques_json')
    .then(res => res.json())
    .then(dades => {
      const ctx = document.getElementById('grafica').getContext('2d');
      if (window.myChart) window.myChart.destroy();
      window.myChart = new Chart(ctx, {
        type: 'bar',
        data: {
          labels: ['Menjar (g)', 'Aigua (ml)'],
          datasets: [{
            label: 'Quantitat dispensada',
            data: [dades.menjar.quantitat, dades.aigua.quantitat],
            backgroundColor: ['#4caf50', '#2196f3']
          }]
        },
        options: {
          scales: {
            y: { beginAtZero: true }
          }
        }
      });
    });
  */
}
function resetEstadistiques() {
  fetch('/reset_estadistiques').then(res => {
    veureEstadistiques();
  });
}
</script></body></html>
)rawliteral";

// Funció que permet dispensar menjar o aigua tenint en comte el pes maxim que hi pot haver al recipient
void dispensarAmbPes(SensorPes& sensor, Dispensador& dispensador, const String& tipus, float pesMaxim) {
  float pes = sensor.llegir_pes();
  if (pes >= pesMaxim) {
    pantalla.mostrar_missatge(tipus + " ple");
    return;
  }
  pantalla.mostrar_missatge("Dispensant " + tipus + "...");
  dispensador.obrir();
  unsigned long inici = millis();
  while (millis() - inici < 5000) {
    pes = sensor.llegir_pes();
    if (pes >= pesMaxim) {
      pantalla.mostrar_missatge(tipus + " complet");
      break;
    }
    delay(100);
  }
  dispensador.tancar();
  pantalla.mostrar_missatge(tipus + " finalitzat");

  if (tipus == "menjar") {  // Per poder fer les estadístiques
    contadorMenjar++;
    gramsMenjarTotal += 10.0;
  } else if (tipus == "aigua") {
    contadorAigua++;
    mlAiguaTotal += 100.0;
  }
}

// Funió que permet dispensar menjar o aigua independentment del pes del recipient, això ens permet fer funcionar el sistema en cas que els sensors de pes no funcionin
void dispensarSenseControl(Dispensador& dispensador, const String& tipus) {
  pantalla.mostrar_missatge("FORÇANT " + tipus + "...");
  dispensador.obrir();
  if (tipus == "aigua") {
    delay(2000);
  } else {
    delay(5000);
  }
  dispensador.tancar();
  pantalla.mostrar_missatge(tipus + " forçat finalitzat");

  if (tipus == "menjar") {
    contadorMenjar++;
    gramsMenjarTotal += 10.0;
  } else if (tipus == "aigua") {
    contadorAigua++;
    mlAiguaTotal += 100.0;
  }
}

// FUNCIÓ PER DEMANAR LES CREDENCIALS PER ACCEDIR A LA WEB
void handleRoot() { 
  if (!server.authenticate("admin", "1234")) return server.requestAuthentication();
  char html[4096];
  snprintf(html, sizeof(html), index_html, horaMenjar, minutMenjar, horaAigua, minutAigua);
  server.send(200, "text/html", html);
}

// FUNCIÓ QUE MOSTRA ELS MISSATGES A LA WEB
void handleMenjar() {
  server.send(200, "text/plain", "Menjar activat");
  dispensarAmbPes(sensorMenjar, dispensador_menjar, "menjar", PES_MAX_MENJAR);
}

void handleAigua() {
  server.send(200, "text/plain", "Aigua activada");
  dispensarAmbPes(sensorAigua, dispensador_aigua, "aigua", PES_MAX_AIGUA);
}

void handleForcaMenjar() {
  server.send(200, "text/plain", "Menjar forçada activat");
  dispensarSenseControl(dispensador_menjar, "menjar");
}

void handleForcaAigua() {
  server.send(200, "text/plain", "Aigua forçada activada");
  dispensarSenseControl(dispensador_aigua, "aigua");
}


void handleSetHorari() {
  if (server.hasArg("menjar")) sscanf(server.arg("menjar").c_str(), "%d:%d", &horaMenjar, &minutMenjar);
  if (server.hasArg("aigua")) sscanf(server.arg("aigua").c_str(), "%d:%d", &horaAigua, &minutAigua);
  menjarDispensat = false;
  aiguaDispensada = false;
  server.sendHeader("Location", "/", true);
  server.send(302, "text/plain", "");
}

void handleEstadistiques() {
  String resposta = "\xf0\x9f\x93\x8a Estadístiques:\n";
  resposta += "• Cops dispensador menjar activat: " + String(contadorMenjar) + "\n";
  resposta += "• Total menjar dispensat: " + String(gramsMenjarTotal) + "g\n";
  resposta += "• Cops dispensador aigua activat: " + String(contadorAigua) + "\n";
  resposta += "• Total aigua dispensada: " + String(mlAiguaTotal) + "ml";
  server.send(200, "text/plain", resposta);
}

void handleEstadistiquesJSON() {
  String json = "{";
  json += "\"menjar\": {\"cops\": " + String(contadorMenjar) + ", \"quantitat\": " + String(gramsMenjarTotal) + "},";
  json += "\"aigua\": {\"cops\": " + String(contadorAigua) + ", \"quantitat\": " + String(mlAiguaTotal) + "}";
  json += "}";
  server.send(200, "application/json", json);
}

void handleResetEstadistiques() {
  contadorMenjar = 0;
  gramsMenjarTotal = 0.0;
  contadorAigua = 0;
  mlAiguaTotal = 0.0;
  server.send(200, "text/plain", "\xf0\x9f\x9a\x9a Estadístiques reiniciades");
}

void setupWiFiDual() {
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(ap_ssid, ap_password);
  Serial.println("✅ AP actiu. IP: " + WiFi.softAPIP().toString());
  WiFi.begin(sta_ssid, sta_password);
  int intent = 0;
  Serial.print("🔀 Connectant a WiFi");
  while (WiFi.status() != WL_CONNECTED && intent++ < 20) {
    delay(500);
    Serial.print(".");
  }
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ Connectat a WiFi. IP: " + WiFi.localIP().toString());
    horari.iniciarHora();
  } else Serial.println("\n❌ No connectat a WiFi.");
}

void setup() {
  Serial.begin(115200);
  dispensador_menjar.iniciar();
  dispensador_aigua.iniciar();
  pantalla.iniciar_pantalla();
  sensorMenjar.iniciar_sensor_pes();
  sensorAigua.iniciar_sensor_pes();
  setupWiFiDual();

  server.on("/", handleRoot);
  server.on("/menjar", handleMenjar);
  server.on("/aigua", handleAigua);
  server.on("/forca_menjar", handleForcaMenjar);
  server.on("/forca_aigua", handleForcaAigua);
  server.on("/setHorari", handleSetHorari);
  server.on("/estadistiques", handleEstadistiques);
  server.on("/estadistiques_json", handleEstadistiquesJSON);
  server.on("/reset_estadistiques", handleResetEstadistiques);
  server.begin();
  Serial.println("🌐 Servidor web actiu.");
}

void loop() {
  server.handleClient();
  unsigned long ara = millis();
  if (ara - ultimaComprobacio > 10000) {
    horari.mostraHoraActual();
    if (horaMenjar >= 0 && horari.esHoraActual(horaMenjar, minutMenjar)) {
      if (!menjarDispensat || horaMenjar != horari.horaAnterior || minutMenjar != horari.minutAnterior) {
        dispensarAmbPes(sensorMenjar, dispensador_menjar, "menjar", PES_MAX_MENJAR);
        menjarDispensat = true;
      }
    } else menjarDispensat = false;

    if (horaAigua >= 0 && horari.esHoraActual(horaAigua, minutAigua)) {
      if (!aiguaDispensada || horaAigua != horari.horaAnterior || minutAigua != horari.minutAnterior) {
        dispensarAmbPes(sensorAigua, dispensador_aigua, "aigua", PES_MAX_AIGUA);
        aiguaDispensada = true;
      }
    } 
    else 
      aiguaDispensada = false;

    horari.horaAnterior = horari.getHora();
    horari.minutAnterior = horari.getMinut();
    ultimaComprobacio = ara;
  }
}
