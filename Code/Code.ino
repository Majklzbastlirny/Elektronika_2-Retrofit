//sus amogus DCF hodinus de PVBus. Crazus Electronicus 2022us
#include "TimeLib.h" //zahrnutí modulů, knihoven
#include "Funkuhr.h"

int sck = 5;  //přiřazení vývodů
int sda = 3;
int rclk = 4;
int anody[] = {10,8,7,6,9,11};
int znaky[] = {0b11110101,0b00100001,0b01110110,0b00110111,0b10100011,0b10010111,0b11010111,0b00110001,0b11110111,0b10110011,0b00000000}; //pole tvořící 7 SEG znaky
int shift = 0;  //číselný údaj pro seriový přenos
int zobraz[] = {0,0,0,0,0,0}; //buffer displeje
int misto = 0;  //proměnná multiplexu 0-5 
long multiplex = 0; //konstanta pro časovač řídící multiplex
long timer = 0; //konstanta pro časovač řídící blikání oddělovací LED
int dot[] = {0,0,0,0,0,0}; //pole ovládání teček displejů
int mode = 0; //režim zobrazení
int secs = 0;
bool dot5 = 0;
bool alarm = 0;




Funkuhr dcf(0, 2, 13, true);  //inicializace
struct Dcf77Time dt = { 0 };
uint8_t curSec;


void setup() {
  
pinMode(sck,OUTPUT);
pinMode(sda,OUTPUT);
pinMode(rclk,OUTPUT);

dcf.init();

for (int i = 0; i <= 5; i++){
  pinMode(anody[i], OUTPUT);


  
}}

void loop() {
  //hlavní smyčka
dcf.getTime(dt);
time_t t = now(); //obnovení času z časovače

if(dcf.synced()){
  setTime(dt.hour,dt.min,dt.sec,dt.day,dt.month,dt.year); //synchronizace DCF a vnitřních hodin
}


 if (second(t) != secs){  //obládání blikání oddělovače. 
   dot5 = 1;
   timer = millis();
   secs = second(t);
 }
 
 if (millis() > timer + 100){ //100ms pro alarm flag = 1, 500ms pro alarm flag = 0
  dot5 = 0;
 }



 if (second(t) < 2){  //přepínání režimů zobrazení
  mode = 1;
 }
 else {
  mode = 0;
 }
 

 if (mode == 0){  //zápis údajů do bufferu displeje, čas
  zobraz[0] = hour(t) / 10;
  zobraz[1] = hour(t) % 10;
  zobraz[2] = minute(t) / 10;
  zobraz[3] = minute(t) % 10;
  zobraz[4] = second(t) / 10;
  zobraz[5] = second(t) % 10;
  dot[0] = 1;
  dot[1] = 1;
  dot[2] = 1;
  dot[3] = 1;
  dot[4] = 1;
  dot[5] = dot5;
}

if (mode == 1){ //zápis údajů do bufferu displeje, datum
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

if (mode == 2){ //zápis údajů do bufferu displeje, budííííííík
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

if (micros() > multiplex + 1000){ //časové řízení multiplexu
  multiplex = micros();

  misto++;  //inkrementace přístupu multiplexu 

  if (misto > 5){
    misto = 0;
    
  }
 
    
  
  if(dot[misto] == 1){  //bitová maska řízení teček (u data) a sestavení vysílaného údaje 
   shift = znaky[zobraz[misto]] | 0b00001000; 
  }
  else {
    shift = znaky[zobraz[misto]] ; 
  }
  
  shiftOut(sda,sck,LSBFIRST,shift); //seriový výstup dat k posuvnému registru
  
  for (int i = 0; i <= 5; i++){ //zhasnutí předešlé segmentovky
    digitalWrite(anody[i], HIGH);
  }
  
  digitalWrite(rclk,HIGH);  //obnovení údaje v posuvném registru pomocí zabudovaného střadače
  digitalWrite(rclk,LOW);
  digitalWrite(anody[misto], LOW); 

    
  
  
  
}}
