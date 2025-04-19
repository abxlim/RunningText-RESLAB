#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include <Keypad.h>
#include "Font_Data.h"
#include "RTClib.h"

#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 16
#define MAX_ZONES 4
#define ZONE_SIZE (MAX_DEVICES / MAX_ZONES)

#define CLK_PIN D5
#define DATA_PIN D7
#define CS_PIN D8

RTC_DS3231 rtc;

char piket[100];
char displayTime[10];
char displayTimeFull[10];
char timer[5];

const byte ROWS = 4;  //4 baris
const byte COLS = 4;  //4 kolom
char keys[ROWS][COLS] = {
  { '1', '2', '3', 'A' },
  { '4', '5', '6', 'B' },
  { '7', '8', '9', 'C' },
  { '*', '0', '#', 'D' }
};

byte colPins[COLS] = { D3, D2, D1, D0 };  // pin 2,3,4,5 untuk pin kolom keypad (lihat gambar)
byte rowPins[ROWS] = { D10, D9, D6, D4 };  // pin 6,7,8,9 untuk pin baris keypad (lihat gambar)

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

unsigned long prevTime = 0;
unsigned long lastBlinkTime = 0;
unsigned long blinkInterval = 1000;  // interval blink setiap 500ms
bool startTimer = true;
int posisi = 1;
int day;
int s, m;
int durasi = 600;
int durasiTemp = durasi;

MD_Parola P = MD_Parola(HARDWARE_TYPE, CS_PIN, MAX_DEVICES);

void setup(void) {
  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
    Serial.flush();
    while (1) delay(10);
  }
  rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));

  Serial.begin(9600);
  delay(2000);
  P.begin(MAX_ZONES);
  P.setInvert(false);
  P.setIntensity(0);

  P.setZone(0, 0, 7);   //Bottom Right
  P.setZone(1, 8, 15);  //Upper Right
}

void loop(void) {
  char key = keypad.getKey();

  if (key) {
    keypadFunct(key);
  }

  switch (posisi) {
    case 1:
      P.setFont(0, NULL);
      P.setFont(1, NULL);
      getTime();

      switch (day) {
        case 1:
          sprintf(piket, "Asisten RESLAB Senin : Diana, Fadilla, Ella, Nisa Zul");
          break;
        case 2:
          sprintf(piket, "Asisten RESLAB Selasa : Fathia, Daffa, Ilham");
          break;
        case 3:
          sprintf(piket, "Asisten RESLAB Rabu : Semua Asisten");
          break;
        case 4:
          sprintf(piket, "Asisten RESLAB Kamis : Aini, Nisa Nur, Thifa");
          break;
        case 5:
          sprintf(piket, "Asisten RESLAB Jumat : Havis, Azzu, Dinda");
          break;
      }

      P.displayZoneText(1, displayTime, PA_CENTER, 40, 0, PA_PRINT, PA_NO_EFFECT);
      P.displayZoneText(0, piket, PA_CENTER, 40, 0, PA_SCROLL_LEFT, PA_SCROLL_LEFT);
      posisi++;
      break;

    case 2:
      P.setFont(0, NULL);
      P.setFont(1, NULL);
      P.displayZoneText(1, "Ruang", PA_CENTER, 30, 4000, PA_OPENING, PA_CLOSING);
      P.displayZoneText(0, "Asisten", PA_CENTER, 30, 4000, PA_OPENING, PA_CLOSING);
      posisi++;
      break;

    case 3:
      P.setFont(0, NULL);
      P.setFont(1, NULL);
      P.displayZoneText(1, "DILARANG", PA_CENTER, 40, 4000, PA_PRINT, PA_SCROLL_DOWN);
      P.displayZoneText(0, "MASUK", PA_CENTER, 40, 4000, PA_PRINT, PA_SCROLL_UP);
      posisi = 1;
      break;

    case 4:
      secondsToHMS(durasiTemp);
      sprintf(timer, "%02d:%02d", m, s);
      P.setFont(0, BigFontBottom);
      P.setFont(1, BigFontUp);
      P.displayZoneText(1, timer, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      P.displayZoneText(0, timer, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);

      if (startTimer) {
        if (millis() - prevTime > 1000) {
          durasiTemp--;
          prevTime = millis();
        }
        if (durasiTemp < 0) {
          posisi++;
          durasiTemp = durasi;
        }
      }
      break;

    case 5:
      P.setFont(0, NULL);
      P.setFont(1, NULL);
      P.displayZoneText(1, "WAKTU", PA_CENTER, 20, 10000, PA_OPENING, PA_CLOSING);
      P.displayZoneText(0, "HABIS", PA_CENTER, 20, 10000, PA_OPENING, PA_CLOSING);
      posisi++;
      break;

    case 6:
      P.setFont(0, NULL);
      P.setFont(1, NULL);
      P.displayZoneText(1, "PEMBAHASAN", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      P.displayZoneText(0, "RESPONSI", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      break;

    case 7:
      P.setFont(0, NULL);
      P.setFont(1, NULL);
      getTime();
      P.displayZoneText(1, "RESLAB", PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      P.displayZoneText(0, displayTimeFull, PA_CENTER, 0, 0, PA_PRINT, PA_NO_EFFECT);
      break;

    case 9:
      P.setFont(0, NULL);
      P.setFont(1, NULL);
      getTime();
      P.displayZoneText(1, "Dalam", PA_CENTER, 30, 4000, PA_OPENING, PA_CLOSING);
      P.displayZoneText(0, "Pengembangan", PA_CENTER, 30, 4000, PA_OPENING, PA_CLOSING);
      posisi = 1;
      break;
  }

  while (!P.getZoneStatus(0) || !P.getZoneStatus(1)) {
    char key = keypad.getKey();
    if (key) {
      keypadFunct(key);
      break;
    }
    P.displayAnimate();
    yield();
  }
}


void secondsToHMS(int seconds) {
  int t = seconds;

  s = t % 60;
  t = (t - s) / 60;
  m = t % 60;
}

void getTime() {
  DateTime now = rtc.now();
  day = now.dayOfTheWeek();
  sprintf(displayTime, "%02d:%02d", now.hour(), now.minute());
  sprintf(displayTimeFull, "%02d %02d %02d", now.hour(), now.minute(), now.second());
}

void keypadFunct(char keyValue) {

  if (keyValue != 'A' || keyValue != '#' || keyValue != '*') {
    startTimer = true;
  }

  switch (keyValue) {
    case '0':  // Instruksi untuk menampilkan karakter pada LCD dan Serial monitor
      posisi = 9;
      break;
    case '1':
      posisi = 1;
      break;
    case '2':
      posisi = 9;
      break;
    case '3':
      posisi = 9;
      break;
    case '4':
      posisi = 9;
      break;
    case '5':
      posisi = 9;
      break;
    case '6':
      posisi = 9;
      break;
    case '7':
      posisi = 9;
      break;
    case '8':
      posisi = 9;
      break;
    case '9':
      posisi = 9;
      break;
    case 'A':
      startTimer = !startTimer;
      posisi = 4;
      break;
    case 'B':
      posisi = 7;
      break;
    case 'C':
      posisi = 9;
      break;
    case 'D':
      posisi = 9;
      break;
    case '#':
      durasi = durasi + 60;
      durasiTemp = durasiTemp + 60;
      break;
    case '*':
      durasi = durasi - 60;
      durasiTemp = durasiTemp - 60;
      if (durasi < 60 || durasiTemp < 60) {
        durasi = 60;
        durasiTemp = 60;
      }
      break;
    default:
      break;
  }
}