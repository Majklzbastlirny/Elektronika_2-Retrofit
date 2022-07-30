//sus amogus DCF hodinus de PVBus. Crazus Electronicus 2022us
#include "TimeLib.h" //zahrnutí modulů, knihoven
#include "Funkuhr.h"
#include <EEPROM.h>


int sck = 5;  //přiřazení vývodů
int sda = 3;
int rclk = 4;
int anody[] = {10, 8, 7, 6, 12, 11};
int znaky[] = {0b11110101, 0b00100001, 0b01110110, 0b00110111, 0b10100011, 0b10010111, 0b11010111, 0b00110001, 0b11110111, 0b10110111, 0b00000000}; //pole tvořící 7 SEG znaky
int shift = 0;  //číselný údaj pro seriový přenos
int zobraz[] = {0, 0, 0, 0, 0, 0}; //buffer displeje
int misto = 0;  //proměnná multiplexu 0-5
long multiplex = 0; //konstanta pro časovač řídící multiplex
long timer = 0; //konstanta pro časovač řídící blikání oddělovací LED
int dot[] = {0, 0, 0, 0, 0, 0}; //pole ovládání teček displejů
int mode = 0; //režim zobrazení
int secs = 0;
bool dot5 = 0;

int ALMpos1 = 0;
int ALMpos2 = 0;
int ALMpos3 = 0;
int ALMpos4 = 0;
int ALMcounter = 0;
int ALMidleflag = 0;
bool ALMaltflag = 0;

#define Button1 A4 // Vrchní tlačítko, nastavení minut alarmu
bool Button1state;
#define Button2 A5 //Střední tlačítko, nastavení hodin alarmu
bool Button2state;
#define Button3 A6 //Spodní tlačítko, restart nastavování budíku (možná free, uvidí se
bool Button3state;
#define Button4 A1 //Solo tlačítko zobrazení času alarmu a jeho nastavení
bool Button4state;
#define Izostat1 A3 //Velký izostat, vyp/zap budík
bool Izostat1state;
#define Izostat2 A2 //Malý izostat, nastavení jasu
bool Izostat2state;

#define Piezo 13




Funkuhr dcf(0, 2, 9, true);  //inicializace
struct Dcf77Time dt = { 0 };
uint8_t curSec;


void setup() {
  //Serial.begin(115200);
  pinMode(sck, OUTPUT);
  pinMode(sda, OUTPUT);
  pinMode(rclk, OUTPUT);

  dcf.init();
  ReviveAlarm();

  for (int i = 0; i <= 5; i++) {
    pinMode(anody[i], OUTPUT);
  }
}
void loop() {
  //hlavní smyčka
  dcf.getTime(dt);
  time_t t = now(); //obnovení času z časovače

  //delayMicroseconds(100);
  /*Serial.print(Button1state);
    Serial.print(Button2state);
    Serial.print(Button3state);
    Serial.print(Button4state);
    Serial.print(Izostat1state);
    Serial.println(Izostat2state);
  */
  if (dcf.synced()) {
    setTime(dt.hour, dt.min, dt.sec, dt.day, dt.month, dt.year); //synchronizace DCF a vnitřních hodin
  }

  ALMcounter++;
  if (ALMcounter > 80) {
    ALMcounter = 0;
  }

  if (ALMaltflag == 1) {
    if (ALMcounter == 1) {
      ALMidleflag++;
    }
  }


  if (Button3state == 1 && Button4state == 1) {
    ALMidleflag = 0;
    if (ALMaltflag == 0) {
      ALMaltflag = 1;
    }
    ALMpos1 = 0;
    ALMpos2 = 0;
    ALMpos3 = 0;
    ALMpos4 = 0;
    
  }

  if ( Button4state == 1 && Button1state == 1 && ALMcounter == 1) {
    ALMpos2++;
    ALMidleflag = 0;
    if (ALMaltflag == 0) {
      ALMaltflag = 1;
    }

    if (ALMpos2 > 9) {
      ALMpos2 = 0;
      ALMpos1++;
    }

    if (ALMpos1 == 2 && ALMpos2 == 4) {
      ALMpos1 = 0;
      ALMpos2 = 0;
    }
  }

  if ( Button4state == 1 && Button2state == 1 && ALMcounter == 1) {
    ALMpos4++;
    ALMidleflag = 0;
    if (ALMaltflag == 0) {
      ALMaltflag = 1;
    }

    if (ALMpos4 > 9) {
      ALMpos4 = 0;
      ALMpos3++;
    }

    if (ALMpos3 == 6 && ALMpos4 == 0) {
      ALMpos3 = 0;
      ALMpos4 = 0;
    }
  }

  if (ALMaltflag == 1 && ALMidleflag == 360) {
    ALMaltflag = 0;
    ALMidleflag = 0;
    UpdateAlarm();
  }



  if (second(t) != secs) { //obládání blikání oddělovače.
    dot5 = 1;
    timer = millis();
    secs = second(t);
  }

  if (Izostat1state == 1) {
    if (millis() > timer + 100) { //100ms pro alarm flag = 1, 500ms pro alarm flag = 0
      dot5 = 0;
    }
  }
  else {
    if (millis() > timer + 500) { //100ms pro alarm flag = 1, 500ms pro alarm flag = 0
      dot5 = 0;
    }
  }

  if (second(t) < 2) { //přepínání režimů zobrazení
    mode = 1;
  }
  else {
    mode = 0;
  }

  GetData();

  if (mode == 0) { //zápis údajů do bufferu displeje, čas
    zobraz[0] = hour(t) / 10;
    zobraz[1] = hour(t) % 10;
    zobraz[2] = minute(t) / 10;
    zobraz[3] = minute(t) % 10;
    zobraz[4] = second(t) / 10;
    zobraz[5] = second(t) % 10;
    dot[0] = Button1state;
    dot[1] = Button2state;
    dot[2] = Button3state;
    dot[3] = Button4state;
    dot[4] = Izostat2state;
    dot[5] = dot5;
  }

  if (mode == 1) { //zápis údajů do bufferu displeje, datum
    zobraz[0] = day(t) / 10;
    zobraz[1] = day(t) % 10;
    zobraz[2] = month(t) / 10;
    zobraz[3] = month(t) % 10;
    zobraz[4] = (year(t) % 100) / 10;
    zobraz[5] = (year(t) % 100) % 10;
    dot[0] = 0;
    dot[1] = 1;
    dot[2] = 0;
    dot[3] = 1;
    dot[4] = 0;
    dot[5] = 0;
  }

  if (mode == 2) { //zápis údajů do bufferu displeje, budííííííík
    zobraz[0] = ALMpos1;
    zobraz[1] = ALMpos2;
    zobraz[2] = ALMpos3;
    zobraz[3] = ALMpos4;
    zobraz[4] = 0;
    zobraz[5] = 0;
    dot[0] = Button1state;
    dot[1] = Button1state;
    dot[2] = Button2state;
    dot[3] = Button2state;
    dot[4] = 0;
    dot[5] = dot5;
  }

  if (micros() > multiplex + 100) { //časové řízení multiplexu
    multiplex = micros();
    delay(1);
    misto++;  //inkrementace přístupu multiplexu


    if (misto > 5) {
      misto = 0;

    }



    if (dot[misto] == 1) { //bitová maska řízení teček (u data) a sestavení vysílaného údaje
      shift = znaky[zobraz[misto]] | 0b00001000;
    }
    else {
      shift = znaky[zobraz[misto]] ;
    }

    shiftOut(sda, sck, LSBFIRST, shift); //seriový výstup dat k posuvnému registru

    for (int i = 0; i <= 5; i++) { //zhasnutí předešlé segmentovky
      digitalWrite(anody[i], HIGH);
    }

    digitalWrite(rclk, HIGH); //obnovení údaje v posuvném registru pomocí zabudovaného střadače
    digitalWrite(rclk, LOW);
    digitalWrite(anody[misto], LOW);




  }
}

void GetData() {
  if (analogRead(Button1) < 5) {
    Button1state = 1;
  }
  else {
    Button1state = 0;
  }

  if (analogRead(Button2) < 5) {
    Button2state = 1;
  }
  else {
    Button2state = 0;
  }

  if (analogRead(Button3) < 5) {
    Button3state = 1;
  }
  else {
    Button3state = 0;
  }

  if (analogRead(Button4) < 5) {
    Button4state = 1;
    mode = 2;
  }
  else {
    Button4state = 0;
  }

  if (analogRead(Izostat1) < 5) {
    Izostat1state = 1;
    //tone(Piezo, 250, 1000);
  }
  else {
    Izostat1state = 0;
  }
  if (analogRead(Izostat2) < 5) {
    Izostat2state = 1;
  }
  else {
    Izostat2state = 0;
  }
}

void ReviveAlarm() {
  ALMpos1 = EEPROM.read(0);
  ALMpos2 = EEPROM.read(1);
  ALMpos3 = EEPROM.read(2);
  ALMpos4 = EEPROM.read(3);
}

void UpdateAlarm() {
  EEPROM.update(0, ALMpos1);
  EEPROM.update(1, ALMpos2);
  EEPROM.update(2, ALMpos3);
  EEPROM.update(3, ALMpos4);
}
