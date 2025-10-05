#include <Arduino.h>
#include <AutoOTA.h>
AutoOTA ota("1.00", "eu1abg/Analiz_CO2"); // eu1abg/Analiz_CO2   https://github.com/eu1abg/Analiz_CO2

 #include <WiFi.h>
 #include <WiFiUdp.h>

 #include <TFT_eSPI.h>
 TFT_eSPI tft = TFT_eSPI();
 

#include <SimplePortal.h>
#include <EEPROM.h>
//==================================================================


//=========================================================================================================================================== 
#include <NTPClient.h>
// --- Настройки NTP ---
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 3 * 3600, 60000); 
// GMT+3 (Беларусь/Москва), обновление каждую минуту

//===========================================================================================================================================
//  #include <NTPClient_Generic.h>          // https://github.com/khoih-prog/NTPClient_Generic
// #include <WiFiUdp.h>
// WiFiUDP ntpUDP;
//  #define TIME_ZONE_OFFSET_HRS            (3)
// #define SECS_IN_HR                (3600L)
// NTPClient timeClient(ntpUDP);
//=========================================================================================================================================== 
#include "DHT.h"
#define DHTPIN 22 // Тот самый номер пина,
DHT dht(DHTPIN, DHT11);
//===========================================================================================================================================
#include <TimerMs.h>
TimerMs tmr1(3000, 1, 0);   // измеряем температуру
// TimerMs tmr2(30000, 1, 0);   // запись
//TimerMs tmr3(1500, 0, 1);
TimerMs tmr4;   
TimerMs tmr5; 
// TimerMs tmr6(500, 0, 1);
// TimerMs tmr7(5000, 1, 0);   // делаем измерения CO2 co
// TimerMs tmr8(15000, 0, 1);   // прогрев СО
// TimerMs tmr9(15000, 1, 0);   //  возвр экр
// TimerMs tmr10(7000, 0, 1); // переключ экр
// TimerMs tmr11(300000, 1, 0); //  переподкл вайфай
// TimerMs tmr12(86400000, 1, 0); // вочдог
 TimerMs tmr13(60000, 1, 0); // обновление
//=====================================================
#include <MQUnifiedsensor.h>

//Definitions
//#define placa "esp-32"
//#define Voltage_Resolution 5
//#define pin 34 //Analog input 0 of your arduino
//#define type "MQ-135" //MQ135
//#define ADC_Bit_Resolution 12 // For arduino UNO/MEGA/NANO
#define RatioMQ135CleanAir 3.6//RS / R0 = 3.6 ppm 
//#define RatioMQ7CleanAir 27.5 //RS / R0 = 27.5 ppm  
//#define PWMPin 13 // Pin connected to mosfet
//MQUnifiedsensor MQ7("esp-32", 3.3, 12, 34, "MQ-7");
MQUnifiedsensor MQ135("esp-32", 3.3, 12, 35, "MQ-135");
//=========================================================
void obnovl();
void initco2();
void wifisel();
void ekr1();
void ekr2();
void ekr3();
void ekr4();
void ekr5();
void displayTime();
void flash();
void drawOpenWindow(int x, int y, int size,
                    float angleDeg,
                    uint16_t frameColor,
                    uint16_t glassColor,
                    uint16_t sashColor ,
                    bool showBreeze );
                
//-=================
float h; float t; bool on=0; int p;float co;int co2; int n; int n1; int day; int month; int year; int n2;
uint32_t sec; uint32_t timer; uint32_t minutes; uint32_t seconds; uint32_t hours;
int tonePin = 14;bool portal=0;uint32_t timerwifi;float Alcohol; bool fl; bool led;
unsigned long lastUpdate = 0;
const unsigned long updateInterval = 1000; // 1 секунда 
String dayName;
String monthName;
// --- Русские месяцы и дни ---
const char* monthsRus[] = {
  "Янв", "Фев", "Март", "Апр", "Май", "Июнь",
  "Июль", "Авг", "Сент", "Окт", "Нояб", "Дек"
};
const char* daysRus[] = {
  "Вос", "Пон", "Вт", "Ср",
  "Чет", "Пят", "Суб"
};
//=========================================================
void setup() { 
  Serial.begin(115200); EEPROM.begin(500); dht.begin();
  pinMode(14, OUTPUT); // пищалка
//==============================================================================================================================
  tft.init();tft.setRotation(4);tft.fillScreen(TFT_BLACK);
  //TFT_SET_BL(50);
  tft.setTextColor(TFT_YELLOW,TFT_BLUE );tft.setTextSize(4);
  tft.setCursor(30, 100);tft.print("Анализ.");
  tft.setCursor(20, 140);tft.print("Воздуха!"); 
  tft.setCursor(75, 180);tft.print(ota.version()); tft.setTextColor(TFT_YELLOW,TFT_BLACK );
  tft.setCursor(0,200);tft.setTextSize(1);delay(3000);tft.fillScreen(TFT_BLACK);
//=======================================WIFI=======================================================================================

wifisel();
   
//========================================================================================
//==============================================================================================================================
   MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
   //MQ135.setA(102.2); MQ135.setB(-2.473);
   //MQ135.setRL(1);
    MQ135.init(); 
   //MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
   //MQ7.setA(99.042); MQ7.setB(-1.518); // Configure the equation to calculate CO concentration value
   //MQ7.setRL(1);
   //MQ7.init();
   initco2();


//================================================================
  timeClient.begin();
  timeClient.update();
  //================================================================
  tmr4.setTimerMode();
  tmr4.setTime(1000);tmr4.start();
  tmr5.setTimerMode();

}
//================================================
void loop() { flash();
    unsigned long currentMillis = millis();
  if (currentMillis - lastUpdate > updateInterval) {lastUpdate = currentMillis; timeClient.update(); displayTime();}
if (tmr1.tick()) 
{
   t =  dht.readTemperature();    h = dht.readHumidity();
  if (isnan(h) || isnan(t)) {  // Проверка. Если не удается считать показания, выводится «Ошибка считывания», и программа завершает работу
    Serial.println("Ошибка считывания");return;
  }
  
//====================================================== CO CO2 T H P ===================================================================================== 
  // MQ7.update();  co = MQ7.readSensor(); if(co < 10000) {  co =co; } else co = 9999;
   MQ135.update(); 
  // co2=MQ135.readSensor(); if(co2 < 10000) { co2 = co2; } else co2 = 9999;
 MQ135.setA(605.18); MQ135.setB(-3.937);  co = MQ135.readSensor(); if(co < 10000) {  co =co; } else co = 9999;
 MQ135.setA(110.47); MQ135.setB(-2.862);  co2=400+MQ135.readSensor(); if(co2 < 10000) { co2 = co2; } else co2 = 9999;                                                                          
 MQ135.setA(77.255); MQ135.setB(-3.18);   Alcohol = MQ135.readSensor(); 
  //    MQ135.setA(44.947); MQ135.setB(-3.445); float Toluen = MQ135.readSensor(); 
  //     MQ135.setA(102.2 ); MQ135.setB(-2.473); float NH4 = MQ135.readSensor(); 
  //      MQ135.setA(34.668); MQ135.setB(-3.369); float Aceton = MQ135.readSensor(); 

if(co2 < 500) n=0; 
else if(co2>500 && co2<800) n=1;
else if (co2>800) n=2;

if(co>20 && co <50) n=3;
else if (co>50) n=4;
 //n=4;
}  



//======================================================================================  //n=2;
switch (n) {
//case 0:  n=1; tmr4.setTime(200);tmr4.start();   break; //tmr2.setTime(sek);tmr2.start();
    case 0: if(n1==0 or n1==2 or n1==3 or n1==4 or n1==5 ) {tft.fillScreen(TFT_BLACK); n1=1; fl=0; n2=0; }                        ekr1();  break;
    case 1: if(n1==0 or n1==1 or n1==3 or n1==4 or n1==5 ) {tft.fillScreen(TFT_BLACK); n1=2; fl=1; n2=0; }                        ekr2();  break;
    case 2: if(n1==0 or n1==1 or n1==2 or n1==4 or n1==5) {tft.fillScreen(TFT_BLACK); n1=3; fl=1;  n2=0; }                         ekr3();  break;
    case 3: if(n1==0 or n1==1 or n1==2 or n1==3 or n1==5) {tft.fillScreen(TFT_BLACK); n1=4; fl=1;  n2=0; }                         ekr4();  break;
    case 4: if(n1==0 or n1==1 or n1==2 or n1==3 or n1==4) {tft.fillScreen(TFT_BLACK); n1=5; fl=1;  n2=0; }                         ekr5();  break;
    case 5: break;
    default:                                        break; }
//===========================================================================================
if(tmr13.tick()) obnovl();  // обновление
}
//================================================================
//===========================================================================================================================================
 void initco2() { 
   float calcR0 = 0;
  for(int i = 1; i<=10; i ++)
   {MQ135.update(); calcR0 += MQ135.calibrate(RatioMQ135CleanAir);Serial.print(".");}
   MQ135.setR0(calcR0/10);
     if(isinf(calcR0)) { tft.setCursor(15, 10);tft.setTextSize(2);tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK); 
     tft.print("Warning MQ135: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply");
     Serial.println("Warning MQ135: Conection issue, R0 is infinite (Open circuit detected) please check your wiring and supply"); while(1);
     }
   if(calcR0 == 0){ tft.setCursor(15, 10);tft.setTextSize(2);tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK); 
    tft.print("Warning MQ135: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply");
    Serial.println("Warning MQ135: Conection issue found, R0 is zero (Analog pin shorts to ground) please check your wiring and supply"); while(1);
   }
  calcR0 = 0;
  // for(int i = 1; i<=10; i ++)
  //  { MQ7.update(); calcR0 += MQ7.calibrate(RatioMQ7CleanAir);Serial.print("."); }
  //  MQ7.setR0(calcR0/10);
  //  Serial.println("  done!.");
  
  //  if(isinf(calcR0)) { tft.setCursor(15, 200);tft.setTextSize(2);tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
  //   tft.print("Warning MQ7: Проблема с подключением, R0 бесконечен (обнаружен обрыв цепи) пожалуйста, проверьте вашу проводку и источник питания");
  //   Serial.println("Warning MQ7: Проблема с подключением, R0 бесконечен (обнаружен обрыв цепи) пожалуйста, проверьте вашу проводку и источник питания"); while(1);
  //  }
  //  if(calcR0 == 0){  tft.setCursor(15, 200);tft.setTextSize(2);tft.setTextColor(TFT_ORANGE, TFT_BLACK); 
  //   tft.print("Warning MQ7: Обнаружена проблема с подключением, R0 равен нулю (аналоговый вывод замыкается на землю) пожалуйста, проверьте проводку и источник питания");
  //   Serial.println("Warning MQ7: Обнаружена проблема с подключением, R0 равен нулю (аналоговый вывод замыкается на землю) пожалуйста, проверьте проводку и источник питания");while(1);
  // }

  // MQ7.serialDebug(true);
}
//==============================================================================================================================
void wifisel(){
label0:
 if(portal==0){
   EEPROM.get(0, portalCfg.SSID); EEPROM.get(150, portalCfg.pass); WiFi.mode(WIFI_STA); WiFi.begin(portalCfg.SSID, portalCfg.pass); 
    timerwifi = millis(); tft.fillScreen(TFT_BLACK);tft.setTextSize(3);tft.setCursor(0, 50);tft.print(" Подключение ");tft.println(portalCfg.SSID);tft.println(portalCfg.pass);
  while (WiFi.status() != WL_CONNECTED) {tft.print("."); delay(200);
    if((millis()-timerwifi) > 15000) { portal=1; WiFi.disconnect(); tft.fillScreen(TFT_BLACK);tft.setTextSize(3);tft.setCursor(0, 50);tft.print("Портал старт."); goto label0;} }
    }
    if (portal==1) {portalRun(180000); 
   switch (portalStatus()) {
        case SP_SUBMIT: portal=0; 
  EEPROM.put(0,portalCfg.SSID);
  EEPROM.put(150,portalCfg.pass); EEPROM.commit();
   char SSI[32];
    EEPROM.get(0, SSI);tft.setCursor(0, 200);tft.print(SSI);
    EEPROM.get(150, SSI);tft.setCursor(0, 250);tft.print(SSI); delay(5000);
   goto label0;  break;

        case SP_SWITCH_AP: portal=2;WiFi.mode(WIFI_AP); WiFi.softAP("SMARTfen", "12345678"); break;  
        case SP_SWITCH_LOCAL: portal=0; break;
        case SP_EXIT:  portal=0; goto label0;    break;
        case SP_TIMEOUT: portal=2;  goto label0;    break;                    /// WiFi.mode(WIFI_AP); WiFi.softAP("SMARTfen", "12345678");   break;
        case SP_ERROR:   portal=1; goto label0;  break;
 }
}
 
    
}

//===========================================================

//==============================================================================================================================
 void obnovl() { String ver, notes;
if (ota.checkUpdate(&ver, &notes)) { tone(tonePin, 2000, 1000);
tft.fillScreen(TFT_BLACK);
  tft.drawRoundRect(1, 152, 238, 50, 20, TFT_CYAN);
        tft.setCursor(15, 170);tft.setTextSize(2);tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.print("Update Ver. ");tft.print(ver);
  tft.drawRoundRect(1, 207, 238, 113, 20, TFT_MAGENTA);
        tft.setCursor(3, 220);tft.setTextSize(2);tft.setTextColor(TFT_YELLOW, TFT_BLACK); tft.println(" Notes:  ");tft.print(notes);
delay(5000); tft.fillScreen(TFT_BLACK);
tft.drawRoundRect(1, 152, 238, 50, 20, TFT_CYAN);
        tft.setCursor(17, 170);tft.setTextSize(2);tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.print("Update Begin !!!");tone(tonePin, 800, 3000); ota.updateNow();

  
}}
//===========================================================
void ekr1() {
   if (!led) return;
tft.drawRoundRect(1, 1, 238, 70, 20, TFT_RED); tft.setCursor(100, 6);tft.setTextSize(2);tft.setTextColor(TFT_PURPLE, TFT_BLACK);
   tft.print("Дата.");
tft.setCursor(10, 30);tft.setTextSize(3);tft.setTextColor(TFT_YELLOW, TFT_BLACK);
   tft.print(String(" ") + day + "." + monthName.c_str() + "." + year); 
tft.drawRoundRect(1, 75, 238, 70, 20, TFT_RED);tft.setCursor(100, 81);tft.setTextSize(2);tft.setTextColor(TFT_PURPLE, TFT_BLACK);tft.print("Время.");
tft.setCursor(50, 105);tft.setTextSize(3);tft.setTextColor(TFT_YELLOW, TFT_BLACK);tft.print(timeClient.getFormattedTime()); 
      tft.drawRoundRect(1, 152, 238, 168, 20, TFT_CYAN);
 tft.setCursor(15, 165);tft.setTextSize(3);tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.print("Тком. ");tft.setCursor(125, 165);tft.print("     ");tft.setCursor(125, 165);tft.print(t);
        tft.setCursor(15, 195);tft.setTextSize(3);tft.setTextColor(TFT_GREENYELLOW, TFT_BLACK); tft.print("Влаж. ");tft.setCursor(125, 195);tft.print("     ");tft.setCursor(125, 195);tft.print(h);
        tft.setCursor(15, 225);tft.setTextSize(3);tft.setTextColor(TFT_ORANGE, TFT_BLACK); tft.print("CO2 . ");tft.setCursor(125, 225);tft.print("     ");tft.setCursor(125, 225);tft.print(co2);
       tft.setCursor(15, 255);tft.setTextSize(3);tft.setTextColor(TFT_RED, TFT_BLACK); tft.print("CO  . ");tft.setCursor(125, 255);tft.print("     ");tft.setCursor(125, 255);tft.print(co);
       tft.setCursor(15, 285);tft.setTextSize(3);tft.setTextColor(TFT_WHITE, TFT_BLACK); tft.print("Alco. ");tft.setCursor(125, 285);tft.print("     ");tft.setCursor(125, 285);tft.print(Alcohol);   

    
       n=5;
 }
//===========================================================
void ekr2() {
 if (!led) return;
  
tft.drawRoundRect(1, 1, 238, 70, 20, TFT_BLUE); tft.setCursor(10, 30);tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(3);tft.print(" Co2 "); tft.print(co2);tft.setTextSize(2);tft.print(" ppm");
 tft.setCursor(30, 130);tft.setTextSize(3);tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.print("Желательно");tft.setCursor(25, 160);tft.print("Проветрить!");
 if(n2==0)  {drawOpenWindow(70, 215, 100, 10, TFT_GREENYELLOW, TFT_SKYBLUE, TFT_LIGHTGREY, true); n2=1;} 
}
//===========================================================
void ekr3() {
 // if(fl==0) fl=1;
  
tft.drawRoundRect(1, 1, 238, 70, 20, TFT_BLUE); tft.setCursor(10, 30);tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(3);tft.print(" Co2 "); tft.print(co2);tft.setTextSize(2);tft.print(" ppm");
 tft.setCursor(60, 130);tft.setTextSize(3);tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.print("Срочно!");tft.setCursor(25, 160);tft.print("Проветрить!");
 if(n2==0)  {drawOpenWindow(70, 215, 100, 90, TFT_GREENYELLOW, TFT_SKYBLUE, TFT_LIGHTGREY, true); n2=1;} 
}
//===========================================================
void ekr4() {
 // if(fl==0) fl=1;
  
tft.drawRoundRect(1, 1, 238, 70, 20, TFT_BLUE); tft.setCursor(10, 30);tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(3);tft.print(" Co "); tft.print(co);tft.setTextSize(2);tft.print(" ppm");
 tft.setCursor(60, 130);tft.setTextSize(3);tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.print("Срочно!");tft.setCursor(25, 160);tft.print("Проветрить!");
 if(n2==0)  {drawOpenWindow(70, 215, 100, 70, TFT_GREENYELLOW, TFT_SKYBLUE, TFT_LIGHTGREY, true); n2=1;} 
}
//===========================================================
void ekr5() {
 // if(fl==0) fl=1;
  
tft.drawRoundRect(1, 1, 238, 70, 20, TFT_BLUE); tft.setCursor(10, 30);tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextSize(3);tft.print(" Co "); tft.print(co);tft.setTextSize(2);tft.print(" ppm");
 tft.setCursor(60, 110);tft.setTextSize(3);tft.setTextColor(TFT_MAGENTA, TFT_BLACK); tft.print("Срочно!");
tft.setCursor(5, 130);tft.print("Выйти комнаты");
 tft.setCursor(25, 160);tft.print("Проветрить!");
 if(n2==0)  {drawOpenWindow(70, 215, 100, 100, TFT_GREENYELLOW, TFT_SKYBLUE, TFT_LIGHTGREY, true); n2=1;} 
}

//===========================================================
void displayTime() {
  time_t epochTime = timeClient.getEpochTime();
  struct tm *ptm = gmtime((time_t *)&epochTime);

   day = ptm->tm_mday;
   month = ptm->tm_mon; // 0–11
   year = ptm->tm_year + 1900;
   int hour = ptm->tm_hour + 3; // для точности (если нужно сместить)
  if (hour >= 24) hour -= 24;
   int minute = ptm->tm_min;
   int second = ptm->tm_sec;

  dayName = daysRus[timeClient.getDay()];
  monthName = monthsRus[month];

 if(timeClient.getHours() >=0 && timeClient.getHours() < 7 )          { if(led==1) {led=0; tft.fillScreen(TFT_BLACK);} } 
  else  if(timeClient.getHours() >= 7 && timeClient.getHours() < 23 ) {  led=1; }
   else  if(timeClient.getHours() >= 23  )                            { if(led==1) {led=0; tft.fillScreen(TFT_BLACK);} }

 // В Serial тоже выводим
  // Serial.printf("Дата: %02d %s %04d, %s %02d:%02d:%02d\n",
  //               day, monthName.c_str(), year,
  //               dayName.c_str(), hour, minute, second);

   // Serial.print(timeClient.getHours());            
  //led=1;
}
//=======================================================================================
void flash(){ if (!fl) return; 
   if(tmr4.tick()) { tft.drawRoundRect(1, 100, 238, 100, 20, TFT_YELLOW); tmr5.setTime(400);tmr5.start(); }
   if(tmr5.tick()) { tft.drawRoundRect(1, 100, 238, 100, 20, TFT_RED);  tmr4.setTime(400);tmr4.start(); }
  
}
//=======================================================================================
// Рисует иконку открытого окна
// x,y - верхний левый угол иконки
// size - базовый размер (иконка квадратная size x size)
// angleDeg - угол открытия створки (0 = закрыто, положительные = открыто вправо, градусы)
// frameColor - цвет рамы, glassColor - цвет стекла, sashColor - цвет створки
// showBreeze - рисовать волнистые линии ветра справа
void drawOpenWindow(int x, int y, int size,
                    float angleDeg = 40.0,
                    uint16_t frameColor = TFT_WHITE,
                    uint16_t glassColor = TFT_CYAN,
                    uint16_t sashColor  = TFT_LIGHTGREY,
                    bool showBreeze = true) {

  if (size < 24) size = 24; // минимальный размер
  // толщина рамы
  int th = max(2, size / 12);

  // внешняя рамка
  tft.drawRect(x, y, size, size, frameColor);

  // внутренняя "стеклянная" область
  int ix = x + th;
  int iy = y + th;
  int iw = size - 2 * th;
  int ih = size - 2 * th;

  // Немного скруглим внутреннюю область (просто заливка)
  tft.fillRect(ix, iy, iw, ih, glassColor);

  // Нарисуем вертикальную перемычку посередине (рама делит окно)
  int midX = ix + iw / 2;
  tft.fillRect(midX - th/2, iy, th, ih, frameColor);

  // --- Створка (left hinge) ---
  // створка занимает половину окна по ширине
  int sashW = iw / 2;
  int sashH = ih;

  // координаты "закрытой" створки (левая створка, крепится к left inner edge)
  int hx = ix;           // hinge x (левая внутренняя граница)
  int hy = iy;           // верх окна
  // corners до поворота:
  // p0 (top-left hinge) = (hx, hy)
  // p1 (top-right) = (hx + sashW, hy)
  // p2 (bot-right) = (hx + sashW, hy + sashH)
  // p3 (bot-left hinge) = (hx, hy + sashH)

  // pivot (вдоль вертикали) возьмём по центру по Y (чтобы створка вращалась посередине)
  float cx = hx;
  float cy = iy + sashH / 2.0f;

  // угол поворота в радианах: положительный — вправо (от себя)
  float theta = angleDeg * 3.14159265f / 180.0f;

  // Функция вращения точки (px,py) вокруг (cx,cy)
  auto rotate_point = [&](float px, float py, float &rx, float &ry) {
    float dx = px - cx;
    float dy = py - cy;
    rx = cx + cos(theta) * dx - sin(theta) * dy;
    ry = cy + sin(theta) * dx + cos(theta) * dy;
  };

  // точки после поворота
  float p0x = hx, p0y = hy; // hinge top (не меняется по X)
  float p3x = hx, p3y = hy + sashH; // hinge bottom
  float p1x, p1y, p2x, p2y;
  rotate_point(hx + sashW, hy,    p1x, p1y);
  rotate_point(hx + sashW, hy+sashH, p2x, p2y);

  // Нарисуем створку как два треугольника (чтобы заполнить четырёхугольник)
  tft.fillTriangle((int)p0x, (int)p0y, (int)p1x, (int)p1y, (int)p2x, (int)p2y, sashColor);
  tft.fillTriangle((int)p0x, (int)p0y, (int)p2x, (int)p2y, (int)p3x, (int)p3y, sashColor);

  // обводка створки
  tft.drawLine((int)p0x, (int)p0y, (int)p1x, (int)p1y, frameColor);
  tft.drawLine((int)p1x, (int)p1y, (int)p2x, (int)p2y, frameColor);
  tft.drawLine((int)p2x, (int)p2y, (int)p3x, (int)p3y, frameColor);
  tft.drawLine((int)p3x, (int)p3y, (int)p0x, (int)p0y, frameColor);

  // Ручка на створке (не далеко от "поворотного" внешнего края створки)
  // возьмём точку примерно на середине правой кромки створки (между p1 и p2)
  int handleX = (int)((p1x + p2x) / 2.0f);
  int handleY = (int)((p1y + p2y) / 2.0f);
  int hrad = max(2, size / 30);
  tft.fillCircle(handleX, handleY, hrad, frameColor);
  tft.fillCircle(handleX, handleY, hrad-1, sashColor);

  // Рисуем тонкую тень/обводку на стороне петли для объёма
  tft.drawLine(ix, iy, ix, iy+ih, frameColor);

  // --- Ветреные волны справа от окна ---
  if (showBreeze) {
    int startX = ix + iw + max(4, size/20); // немного правее окна
    int baseY = iy + ih/4;
    int lines = 3;
    for (int L = 0; L < lines; L++) {
      float amp = (size / 18.0f) * (0.6f + 0.2f * L);
      float freq = 2.4f + 0.6f * L;
      int steps = max(6, size / 8);
      int prevX = startX;
      float prevYf = baseY + L * (size/12.0f);
      int prevY = (int)prevYf;
      for (int i = 1; i <= steps; i++) {
        float t = (float)i / (float)steps;
        int nx = startX + i * (size / 6) / steps;
        float nyf = prevYf + sin(t * freq * 3.14159f) * amp;
        int ny = (int)nyf;
        tft.drawLine(prevX, prevY, nx, ny, TFT_WHITE);
        prevX = nx; prevY = ny; prevYf = nyf;
      }
    }
  }
}
//===============================================================================
