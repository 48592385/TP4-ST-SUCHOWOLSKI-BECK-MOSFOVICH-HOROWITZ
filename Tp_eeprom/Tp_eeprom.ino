// Apellidos: Beck,Suchowolski,Horowitz,Mosfovich, Grupo:5
#include <Preferences.h>
#include <DHT.h>
#include <U8g2lib.h>

Preferences preferences;

// botones y sensores
#define DHTPIN 23
#define DHTTYPE DHT11
#define BOTON_MAS 35
#define BOTON_MENOS 34
#define LED_ALARMA 25

DHT dht(DHTPIN, DHTTYPE);
U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

// === Variables globales ===
float tempActual = 0;
int VU = 28; // valor umbral inicial
unsigned long lastTempMillis = 0;

// === Estados ===
enum Estado { P1, ESPERA1, P2, SUMA, RESTA, ESPERA2 };
Estado estado = P1;
unsigned long codeStartTime = 0;

// === Funciones ===
void mostrarPantallaPrincipal();
void mostrarPantallaUmbral();

void setup() {
  Serial.begin(115200);
  dht.begin();
  u8g2.begin();
  pinMode(BOTON_MAS, INPUT_PULLUP);
  pinMode(BOTON_MENOS, INPUT_PULLUP);
  pinMode(LED_ALARMA, OUTPUT);
  digitalWrite(LED_ALARMA, LOW);

  preferences.begin("umbral", false);
  VU = preferences.getUInt("VU", 28);  // 28 por defecto si no hay guardado
}

void loop() {
  unsigned long ms = millis();

  // Leer temperatura cada 5s
  if (ms - lastTempMillis >= 5000) {
    tempActual = dht.readTemperature();
    lastTempMillis = ms;

    if (!isnan(tempActual)) {
      if (tempActual > VU) {
        digitalWrite(LED_ALARMA, HIGH);
      } else {
        digitalWrite(LED_ALARMA, LOW);
      }
    }
  }

  switch (estado) {
    case P1:
       mostrarPantallaPrincipal();
      if (digitalRead(BOTON_MAS) == LOW) {
        if (codeStartTime == 0) {
          codeStartTime = ms;
        } else if (ms - codeStartTime >= 5000) {
          estado = ESPERA1;
          codeStartTime = 0;
        }
     
      break;

    case ESPERA1:
      if (digitalRead(BOTON_MAS) == HIGH) {
        estado = P2;
      }
      break;

    case P2:
      mostrarPantallaUmbral();
      // Manejo de presión larga para guardar el umbral
      if (digitalRead(BOTON_MENOS) == LOW ) {
        codeStartTime = ms;
        estado = RESTA;
      }

      else if (digitalRead(BOTON_MAS) == LOW) {
        estado = SUMA;
      }

      break;

    case SUMA:
      if (digitalRead(BOTON_MAS) == HIGH) {
        VU++;
        estado = P2;
      }
      break;

    case RESTA:
      if (digitalRead(BOTON_MENOS) == LOW && ms - codeStartTime >= 5000) {
        preferences.begin("umbral", false);
        preferences.putUInt("VU", VU);
        preferences.end();
        estado = ESPERA2;
        codeStartTime = 0;
      }
    else if (digitalRead(BOTON_MENOS) == HIGH) {
        VU--;
        estado = P2;
      }
      break;

    case ESPERA2:
      if (digitalRead(BOTON_MENOS) == HIGH) {
        estado = P1;
      }
      break;
  }
}
}

// === Funciones ===
void mostrarPantallaPrincipal() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 12, "Temp:");
  u8g2.setCursor(60, 12); u8g2.print(tempActual, 1);
  u8g2.drawStr(98, 12, "C");
  u8g2.drawStr(0, 40, "Umbral:");
  u8g2.setCursor(60, 40); u8g2.print(VU);
  u8g2.sendBuffer();
}

void mostrarPantallaUmbral() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_ncenB08_tr);
  u8g2.drawStr(0, 16, "AJUSTE UMBRAL");
  u8g2.drawStr(15, 40, "VU:");
  u8g2.setCursor(60, 40); u8g2.print(VU);
  u8g2.sendBuffer();
}
