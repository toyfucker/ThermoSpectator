//Adding libraries to the project
#include <WiFi.h> 
#include <NTPClient.h> 
#include <WiFiUdp.h>
#include "OneWire.h" /*one-wire interface for getting data from DS18b20 sensors*/
#include "DallasTemperature.h" /* Thermo sensor support DS18b20 */
#include <WebServer.h> /* */
#include <Math.h> /* I forgot why I've add it*/
#include "driver/timer.h" /* I forgot why I've add it*/


#define ONE_WIRE_BUS 23 /* The Data-pin of DS18b20 has been connected to 23-th pin of ESP32*/
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

//Descrptions of values types
String formattedDate[289]; /* Fomatted date is string type for storing time values like following examples 00:01 05:16  07:03  23:59*/
String formDate; /* Try to delete it and test again. Seems it was added during debug process*/
String str_minutes; 
String str_hours;
float temp1;float temp2;float temp3;float temp4;
float Rx; /*WiFi Rx-level in dBm  (relation of 1.0 mW power)*/
float temp11[289];float temp22[289];float temp33[289];float temp44[289]; /*Array type for storing temperatures value in each 5-th minutes period. If divide 24 hours to 5-th minutes interval, we'll get 248 */
int hours; 
int minutes;
String ip;

//const char* ssid = "KOCTR KOMOB";  
// Set your Static IP address and set your Gateway IP address
const char* ssid = "MikroTik"; const char* password =  "independence";
WebServer server(80);
IPAddress local_IP(192, 168, 1, 48);
IPAddress gateway(192, 168, 1, 1);
IPAddress subnet(255, 255, 255, 0);

long previousMillis1 = 0;
long interval1 = 300000;

//PART I 
DeviceAddress sensor1 = {0x28, 0x66, 0xF4, 0x57, 0xE, 0x0, 0x0, 0x7C};//01.07.2023 сенсор в котельной COLD
DeviceAddress sensor2 = {0x28, 0xDF, 0x4C, 0x58, 0xE, 0x0, 0x0, 0x6}; //01.07.2023 сенсор в котельной HOT 
DeviceAddress sensor3 = {0x28, 0x84, 0xC8, 0x58, 0xE, 0x0, 0x0, 0x4}; //09.12.2023 сенсор в корпусе Linksys
DeviceAddress sensor4 = {0x28, 0x48, 0xD1, 0x2C, 0xE, 0x0, 0x0, 0xCF}; //01.07.2023 сенсор в котельной OUTDOOR
//PART II
//DeviceAddress sensor1 = {0x28, 0xD1, 0x89, 0x58, 0xE, 0x0, 0x0, 0xE5}; //01.07.2023 USB-TypeC сенсор на плате

void setup(void) {
  Serial.begin(9600);
  sensors.begin();
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);delay(100);digitalWrite(LED_BUILTIN, LOW);delay(100);digitalWrite(LED_BUILTIN, HIGH);delay(100);digitalWrite(LED_BUILTIN, LOW);delay(100);digitalWrite(LED_BUILTIN, HIGH);delay(100);digitalWrite(LED_BUILTIN, LOW);delay(100);digitalWrite(LED_BUILTIN, HIGH);delay(100);digitalWrite(LED_BUILTIN, LOW);delay(100);digitalWrite(LED_BUILTIN, HIGH);
  Serial.print("Connecting to SSID: ");Serial.print(ssid);
  //WiFi.begin(ssid,password);
  //WiFi.config(local_IP, gateway, subnet);

for (int ii = 1; ii<10; ii++) {
if (WiFi.status() != WL_CONNECTED) {delay(1000);WiFi.begin(ssid,password);Serial.println(WiFi.localIP());Serial.println(WiFi.RSSI());delay(1000);}
}

  delay(3000);
  //поменять while на цикл for, по истечению которого, если не удалось подключиться к WiFi, то выполнить ESP.Reset
  //while (WiFi.status() != WL_CONNECTED) { digitalWrite(LED_BUILTIN, LOW);WiFi.begin(ssid,password); Serial.print(".");delay(3000);digitalWrite(LED_BUILTIN, HIGH);}
  digitalWrite(LED_BUILTIN, LOW);
  Serial.print(" WiFi connected, IP=");
  Serial.print("IP address: "); Serial.println(WiFi.localIP());Serial.print("MAC=");Serial.println(WiFi.macAddress());Serial.print("Rx Level=");Serial.print(WiFi.RSSI());Serial.println("dBm"); Rx = (WiFi.RSSI());

  server.begin(); 
  server.on("/", handle_OnConnect);
  server.onNotFound(handle_NotFound);
  
  timeClient.begin();
  timeClient.setTimeOffset(25200);//GMT+7
  timeClient.update();
  hours = timeClient.getHours();Serial.println(hours);
  minutes = timeClient.getMinutes();Serial.println(minutes);
  
  if (hours<10) {str_hours='0'+String(hours);} else {str_hours=String(hours);}
  if (minutes<10) {str_minutes='0'+String(minutes);} else {str_minutes=String(minutes);}
  formattedDate[288] = str_hours+':'+str_minutes;
  Serial.print("formattedDate[288]=");Serial.println(formattedDate[288]);

  }


void loop(void) {
  server.handleClient(); 
 // if (WiFi.status() != WL_CONNECTED) {WiFi.begin(ssid,password);} 
  unsigned long currentMillis1 = millis(); 
  if(currentMillis1 - previousMillis1 > interval1) 
    {
    previousMillis1 = currentMillis1; 
    //сдвигаем данные в массиве
    for (int i=1;i<288;i++) {temp11[i]=temp11[i+1];temp22[i]=temp22[i+1];temp33[i]=temp33[i+1];temp44[i]=temp44[i+1];formattedDate[i]=formattedDate[i+1];}

    //опрашиваем датчики
    sensors.requestTemperatures();temp11[288]=sensors.getTempC(sensor1)+15;temp22[288]=sensors.getTempC(sensor2)+12;temp33[288]=sensors.getTempC(sensor3);temp44[288]=sensors.getTempC(sensor4);
    
    //поменять WiFi на цикл for (c 10-ти секундной задержкой на каждом шаге), по истечению которого, если не удалось подключиться к WiFi, то выполнить общий ESP.Reset
    if (WiFi.status() != WL_CONNECTED) {delay(10000);WiFi.begin(ssid,password);Serial.println(WiFi.localIP());Serial.println(WiFi.RSSI());delay(10000);}
    // 31.07.2023 !!! handle_OnConnect(); Не помню зачем я вызывал OnConnect. И не вспомню. Может для дебаггинга...
    
    //Блок получения времени по NTP и приведения значений времени в формат 00:00
    timeClient.update();
    hours = timeClient.getHours();Serial.println(hours); minutes = timeClient.getMinutes();Serial.println(minutes);
    if (hours<10) {str_hours='0'+String(hours);} else {str_hours=String(hours);} /*добавляем ноль перед цифрой если число меньше десяти. Это для часа.*/
    if (minutes<10) {str_minutes='0'+String(minutes);} else {str_minutes=String(minutes);} /*добавляем ноль перед цифрой если число меньше десяти. Это для минут*/
    formattedDate[288] = str_hours+':'+str_minutes; Serial.println(formattedDate[288]);
    }
}


//Основной вызов при GET 
void handle_OnConnect() {
sensors.requestTemperatures(); 
digitalWrite(LED_BUILTIN, HIGH);

//Опрашиваем сенсоры и WiFi 
temp1=sensors.getTempC(sensor1)+15;
temp2=sensors.getTempC(sensor2)+12;
temp3=sensors.getTempC(sensor3);
temp4=sensors.getTempC(sensor4); 
Rx = (WiFi.RSSI());
ip = WiFi.localIP().toString();
server.send(200, "text/html", SendHTML(temp1,temp2));

//Трижды мигаем светодиодом, показывая, что мы дошли до 
digitalWrite(LED_BUILTIN, LOW); delay(300);digitalWrite(LED_BUILTIN, HIGH);delay(300);digitalWrite(LED_BUILTIN, LOW);delay(300);digitalWrite(LED_BUILTIN, HIGH);delay(300);digitalWrite(LED_BUILTIN, LOW);digitalWrite(LED_BUILTIN, HIGH);delay(300);digitalWrite(LED_BUILTIN, LOW);
 }
void handle_NotFound() {server.send(404, "text/plain", "Not found");}

String SendHTML(float temp1,float temp2){
 
String ptr = "<!DOCTYPE html> <html>\n";
ptr +="<head>                                                                                            \n";

ptr +=" <title>Thermex</title>                                                                                                                                                                       \n";
 ptr +=" <style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}                                                                                          \n";
 ptr +=" body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;}                                                                                                                          \n";
 ptr +=" p {font-size: 24px;color: #444444;margin-bottom: 10px;}                                                                                                                                      \n";
ptr +="</style> \n";

ptr +="<script src=\"https://cdn.anychart.com/releases/v8/js/anychart-base.min.js\"></script>\n";
ptr +="<script src=\"https://cdn.anychart.com/releases/v8/js/anychart-ui.min.js\"></script>                                                                                                          \n";
ptr +="<script src=\"https://cdn.anychart.com/releases/v8/js/anychart-exports.min.js\"></script>                                                                                                     \n";
ptr +="<script src=\"https://cdn.anychart.com/releases/v8/themes/dark_glamour.min.js\"></script>                                                                                                     \n";
ptr +="<link href=\"https://cdn.anychart.com/releases/v8/css/anychart-ui.min.css\" type=\"text/css\" rel=\"stylesheet\">                                                                                 \n";
ptr +="<link href=\"https://cdn.anychart.com/releases/v8/fonts/css/anychart-font.min.css\" type=\"text/css\" rel=\"stylesheet\">                                                                         \n";

ptr +="<style type=\"text/css\">                                                                                                                                                                     \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="    html,                                                                                                                                                                                     \n";
ptr +="    body,                                                                                                                                                                                     \n";
ptr +="    #container {                                                                                                                                                                              \n";
ptr +="      width: 100%;                                                                                                                                                                            \n";
ptr +="      height: 100%;                                                                                                                                                                           \n";
ptr +="      margin: 0;                                                                                                                                                                              \n";
ptr +="      padding: 0;                                                                                                                                                                             \n";
ptr +="    }                                                                                                                                                                                         \n";
ptr +="</style>                                                                                                                                                                                      \n";

ptr +="</head>                                                                                                                                                                                       \n";


ptr +="<body>                                                                                                                                                                                        \n";

ptr +=" <h1>THERMEX</h1>                                                                                                                                                                             \n";
ptr +="<p>HOT="; ptr +=temp1;ptr +="    COLD=";ptr +=temp2; ptr +="</p>";
ptr +="<p>INDOOR="; ptr +=temp3; ptr +="   OUTDOOR="; ptr +=temp4; ptr +="</p>";


//Serial.print("IP address: "); Serial.println(WiFi.localIP());Serial.print("MAC=");Serial.println(WiFi.macAddress());Serial.print("Rx Level=");Serial.print(WiFi.RSSI());Serial.print("dBm");

ptr +="<div id=\"container\"></div>                                                                                                                                                                    \n";
ptr +="  <script>                                                                                                                                                                                    \n";
ptr +="  anychart.onDocumentReady(function () {                                                                                                                                                      \n";

ptr +="  anychart.theme('darkGlamour');                                                                                                                                                              \n";
ptr +="      // create data set on our data                                                                                                                                                          \n";
ptr +="      var dataSet = anychart.data.set(getData());                                                                                                                                             \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // map data for the first series, take x from the zero column and value from the first column of data set                                                                               \n";
ptr +="      var firstSeriesData = dataSet.mapAs({ x: 0, value: 1 });                                                                                                                                \n";
ptr +="      var secondSeriesData = dataSet.mapAs({ x: 0, value: 2 });                                                                                                                               \n";
ptr +="      var thirdSeriesData = dataSet.mapAs({ x: 0, value: 3 });                                                                                                                                \n";
ptr +="      var fourdSeriesData = dataSet.mapAs({ x: 0, value: 4 });                                                                                                                                \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // create line chart                                                                                                                                                                    \n";
ptr +="      var chart = anychart.line();                                                                                                                                                            \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // adding dollar symbols to yAxis labels                                                                                                                                                \n";
ptr +="      chart.yAxis().labels().format('{%Value} c');                                                                                                                                             \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // turn on chart animation                                                                                                                                                              \n";
ptr +="      chart.animation(true);                                                                                                                                                                  \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // turn on the crosshair                                                                                                                                                                \n";
ptr +="      chart                                                                                                                                                                                   \n";
ptr +="        .crosshair()                                                                                                                                                                          \n";
ptr +="        .enabled(true)                                                                                                                                                                        \n";
ptr +="        .yLabel({ enabled: false })                                                                                                                                                           \n";
ptr +="        .yStroke(null)                                                                                                                                                                        \n";
ptr +="        .xStroke('#cecece')                                                                                                                                                                   \n";
ptr +="        .zIndex(99);                                                                                                                                                                          \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      chart                                                                                                                                                                                   \n";
ptr +="        .yAxis()                                                                                                                                                                              \n";
ptr +="        .title('Thermex temperature')                                                                                                                                                             \n";
ptr +="        .labels({ padding: [5, 5, 0, 5] });                                                                                                                                                   \n";
ptr +="      chart.xAxis().title('Time');                                                                                                                                                       \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // set chart title text settings                                                                                                                                                        \n";
ptr +="      chart.title(                                                                                                                                                                            \n";
ptr +="        'daily data'                                                                                                                                                                  \n";
ptr +="      );                                                                                                                                                                                      \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // create first series with mapped data                                                                                                                                                 \n";
ptr +="      var firstSeries = chart.spline(firstSeriesData); firstSeries.name('HOT'); firstSeries.markers().zIndex(100); firstSeries.hovered().markers().enabled(true).type('circle').size(4);      \n";
ptr +="      var secondSeries = chart.spline(secondSeriesData); secondSeries.name('COLD'); secondSeries.markers().zIndex(100); secondSeries.hovered().markers().enabled(true).type('circle').size(4);\n";
ptr +="      var thirdSeries = chart.spline(thirdSeriesData); thirdSeries.name('INDOOR'); thirdSeries.markers().zIndex(100); thirdSeries.hovered().markers().enabled(true).type('circle').size(4);   \n";
ptr +="      var fourdSeries = chart.spline(fourdSeriesData); fourdSeries.name('OUTDOOR'); fourdSeries.markers().zIndex(100); fourdSeries.hovered().markers().enabled(true).type('circle').size(4);  \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // turn the legend on                                                                                                                                                                   \n";
ptr +="      chart.legend().enabled(true).fontSize(13).padding([0, 0, 20, 0]);                                                                                                                       \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // set container id for the chart                                                                                                                                                       \n";
ptr +="      chart.container('container');                                                                                                                                                           \n";
ptr +="                                                                                                                                                                                              \n";
ptr +="      // initiate chart drawing                                                                                                                                                               \n";
ptr +="      chart.draw();                                                                                                                                                                           \n";
ptr +="    });                                                                                                                                                                                       \n";
ptr +="                                                                                                                                                                                              \n";

ptr +="function getData() {return [['"; ptr +=formattedDate[1]; ptr +="',"; ptr +=temp11[1]; ptr +=","; ptr +=temp22[1];ptr +=","; ptr +=temp33[1]; ptr +=","; ptr +=temp44[1];ptr +=","; ptr +="],";
//ptr +="['"; ptr +=formattedDate[144]; ptr +="',"; ptr +=temp11[144]; ptr +=","; ptr +=temp22[144]; ptr +=",";  ptr +=temp33[144]; ptr +=","; ptr +=temp44[144];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[2];   ptr +="',"; ptr +=temp11[2  ]; ptr +=","; ptr +=temp22[2  ]; ptr +=",";  ptr +=temp33[2  ]; ptr +=","; ptr +=temp44[2  ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[3];   ptr +="',"; ptr +=temp11[3  ]; ptr +=","; ptr +=temp22[3  ]; ptr +=",";  ptr +=temp33[3  ]; ptr +=","; ptr +=temp44[3  ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[4];   ptr +="',"; ptr +=temp11[4  ]; ptr +=","; ptr +=temp22[4  ]; ptr +=",";  ptr +=temp33[4  ]; ptr +=","; ptr +=temp44[4  ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[5];   ptr +="',"; ptr +=temp11[5  ]; ptr +=","; ptr +=temp22[5  ]; ptr +=",";  ptr +=temp33[5  ]; ptr +=","; ptr +=temp44[5  ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[6];   ptr +="',"; ptr +=temp11[6  ]; ptr +=","; ptr +=temp22[6  ]; ptr +=",";  ptr +=temp33[6  ]; ptr +=","; ptr +=temp44[6  ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[7];   ptr +="',"; ptr +=temp11[7  ]; ptr +=","; ptr +=temp22[7  ]; ptr +=",";  ptr +=temp33[7  ]; ptr +=","; ptr +=temp44[7  ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[8];   ptr +="',"; ptr +=temp11[8  ]; ptr +=","; ptr +=temp22[8  ]; ptr +=",";  ptr +=temp33[8  ]; ptr +=","; ptr +=temp44[8  ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[9];   ptr +="',"; ptr +=temp11[9  ]; ptr +=","; ptr +=temp22[9  ]; ptr +=",";  ptr +=temp33[9  ]; ptr +=","; ptr +=temp44[9  ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[10];  ptr +="',"; ptr +=temp11[10 ]; ptr +=","; ptr +=temp22[10 ]; ptr +=",";  ptr +=temp33[10 ]; ptr +=","; ptr +=temp44[10 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[11];  ptr +="',"; ptr +=temp11[11 ]; ptr +=","; ptr +=temp22[11 ]; ptr +=",";  ptr +=temp33[11 ]; ptr +=","; ptr +=temp44[11 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[12];  ptr +="',"; ptr +=temp11[12 ]; ptr +=","; ptr +=temp22[12 ]; ptr +=",";  ptr +=temp33[12 ]; ptr +=","; ptr +=temp44[12 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[13];  ptr +="',"; ptr +=temp11[13 ]; ptr +=","; ptr +=temp22[13 ]; ptr +=",";  ptr +=temp33[13 ]; ptr +=","; ptr +=temp44[13 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[14];  ptr +="',"; ptr +=temp11[14 ]; ptr +=","; ptr +=temp22[14 ]; ptr +=",";  ptr +=temp33[14 ]; ptr +=","; ptr +=temp44[14 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[15];  ptr +="',"; ptr +=temp11[15 ]; ptr +=","; ptr +=temp22[15 ]; ptr +=",";  ptr +=temp33[15 ]; ptr +=","; ptr +=temp44[15 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[16];  ptr +="',"; ptr +=temp11[16 ]; ptr +=","; ptr +=temp22[16 ]; ptr +=",";  ptr +=temp33[16 ]; ptr +=","; ptr +=temp44[16 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[17];  ptr +="',"; ptr +=temp11[17 ]; ptr +=","; ptr +=temp22[17 ]; ptr +=",";  ptr +=temp33[17 ]; ptr +=","; ptr +=temp44[17 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[18];  ptr +="',"; ptr +=temp11[18 ]; ptr +=","; ptr +=temp22[18 ]; ptr +=",";  ptr +=temp33[18 ]; ptr +=","; ptr +=temp44[18 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[19];  ptr +="',"; ptr +=temp11[19 ]; ptr +=","; ptr +=temp22[19 ]; ptr +=",";  ptr +=temp33[19 ]; ptr +=","; ptr +=temp44[19 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[20];  ptr +="',"; ptr +=temp11[20 ]; ptr +=","; ptr +=temp22[20 ]; ptr +=",";  ptr +=temp33[20 ]; ptr +=","; ptr +=temp44[20 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[21];  ptr +="',"; ptr +=temp11[21 ]; ptr +=","; ptr +=temp22[21 ]; ptr +=",";  ptr +=temp33[21 ]; ptr +=","; ptr +=temp44[21 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[22];  ptr +="',"; ptr +=temp11[22 ]; ptr +=","; ptr +=temp22[22 ]; ptr +=",";  ptr +=temp33[22 ]; ptr +=","; ptr +=temp44[22 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[23];  ptr +="',"; ptr +=temp11[23 ]; ptr +=","; ptr +=temp22[23 ]; ptr +=",";  ptr +=temp33[23 ]; ptr +=","; ptr +=temp44[23 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[24];  ptr +="',"; ptr +=temp11[24 ]; ptr +=","; ptr +=temp22[24 ]; ptr +=",";  ptr +=temp33[24 ]; ptr +=","; ptr +=temp44[24 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[25];  ptr +="',"; ptr +=temp11[25 ]; ptr +=","; ptr +=temp22[25 ]; ptr +=",";  ptr +=temp33[25 ]; ptr +=","; ptr +=temp44[25 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[26];  ptr +="',"; ptr +=temp11[26 ]; ptr +=","; ptr +=temp22[26 ]; ptr +=",";  ptr +=temp33[26 ]; ptr +=","; ptr +=temp44[26 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[27];  ptr +="',"; ptr +=temp11[27 ]; ptr +=","; ptr +=temp22[27 ]; ptr +=",";  ptr +=temp33[27 ]; ptr +=","; ptr +=temp44[27 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[28];  ptr +="',"; ptr +=temp11[28 ]; ptr +=","; ptr +=temp22[28 ]; ptr +=",";  ptr +=temp33[28 ]; ptr +=","; ptr +=temp44[28 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[29];  ptr +="',"; ptr +=temp11[29 ]; ptr +=","; ptr +=temp22[29 ]; ptr +=",";  ptr +=temp33[29 ]; ptr +=","; ptr +=temp44[29 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[30];  ptr +="',"; ptr +=temp11[30 ]; ptr +=","; ptr +=temp22[30 ]; ptr +=",";  ptr +=temp33[30 ]; ptr +=","; ptr +=temp44[30 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[31];  ptr +="',"; ptr +=temp11[31 ]; ptr +=","; ptr +=temp22[31 ]; ptr +=",";  ptr +=temp33[31 ]; ptr +=","; ptr +=temp44[31 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[32];  ptr +="',"; ptr +=temp11[32 ]; ptr +=","; ptr +=temp22[32 ]; ptr +=",";  ptr +=temp33[32 ]; ptr +=","; ptr +=temp44[32 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[33];  ptr +="',"; ptr +=temp11[33 ]; ptr +=","; ptr +=temp22[33 ]; ptr +=",";  ptr +=temp33[33 ]; ptr +=","; ptr +=temp44[33 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[34];  ptr +="',"; ptr +=temp11[34 ]; ptr +=","; ptr +=temp22[34 ]; ptr +=",";  ptr +=temp33[34 ]; ptr +=","; ptr +=temp44[34 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[35];  ptr +="',"; ptr +=temp11[35 ]; ptr +=","; ptr +=temp22[35 ]; ptr +=",";  ptr +=temp33[35 ]; ptr +=","; ptr +=temp44[35 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[36];  ptr +="',"; ptr +=temp11[36 ]; ptr +=","; ptr +=temp22[36 ]; ptr +=",";  ptr +=temp33[36 ]; ptr +=","; ptr +=temp44[36 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[37];  ptr +="',"; ptr +=temp11[37 ]; ptr +=","; ptr +=temp22[37 ]; ptr +=",";  ptr +=temp33[37 ]; ptr +=","; ptr +=temp44[37 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[38];  ptr +="',"; ptr +=temp11[38 ]; ptr +=","; ptr +=temp22[38 ]; ptr +=",";  ptr +=temp33[38 ]; ptr +=","; ptr +=temp44[38 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[39];  ptr +="',"; ptr +=temp11[39 ]; ptr +=","; ptr +=temp22[39 ]; ptr +=",";  ptr +=temp33[39 ]; ptr +=","; ptr +=temp44[39 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[40];  ptr +="',"; ptr +=temp11[40 ]; ptr +=","; ptr +=temp22[40 ]; ptr +=",";  ptr +=temp33[40 ]; ptr +=","; ptr +=temp44[40 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[41];  ptr +="',"; ptr +=temp11[41 ]; ptr +=","; ptr +=temp22[41 ]; ptr +=",";  ptr +=temp33[41 ]; ptr +=","; ptr +=temp44[41 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[42];  ptr +="',"; ptr +=temp11[42 ]; ptr +=","; ptr +=temp22[42 ]; ptr +=",";  ptr +=temp33[42 ]; ptr +=","; ptr +=temp44[42 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[43];  ptr +="',"; ptr +=temp11[43 ]; ptr +=","; ptr +=temp22[43 ]; ptr +=",";  ptr +=temp33[43 ]; ptr +=","; ptr +=temp44[43 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[44];  ptr +="',"; ptr +=temp11[44 ]; ptr +=","; ptr +=temp22[44 ]; ptr +=",";  ptr +=temp33[44 ]; ptr +=","; ptr +=temp44[44 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[45];  ptr +="',"; ptr +=temp11[45 ]; ptr +=","; ptr +=temp22[45 ]; ptr +=",";  ptr +=temp33[45 ]; ptr +=","; ptr +=temp44[45 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[46];  ptr +="',"; ptr +=temp11[46 ]; ptr +=","; ptr +=temp22[46 ]; ptr +=",";  ptr +=temp33[46 ]; ptr +=","; ptr +=temp44[46 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[47];  ptr +="',"; ptr +=temp11[47 ]; ptr +=","; ptr +=temp22[47 ]; ptr +=",";  ptr +=temp33[47 ]; ptr +=","; ptr +=temp44[47 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[48];  ptr +="',"; ptr +=temp11[48 ]; ptr +=","; ptr +=temp22[48 ]; ptr +=",";  ptr +=temp33[48 ]; ptr +=","; ptr +=temp44[48 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[49];  ptr +="',"; ptr +=temp11[49 ]; ptr +=","; ptr +=temp22[49 ]; ptr +=",";  ptr +=temp33[49 ]; ptr +=","; ptr +=temp44[49 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[50];  ptr +="',"; ptr +=temp11[50 ]; ptr +=","; ptr +=temp22[50 ]; ptr +=",";  ptr +=temp33[50 ]; ptr +=","; ptr +=temp44[50 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[51];  ptr +="',"; ptr +=temp11[51 ]; ptr +=","; ptr +=temp22[51 ]; ptr +=",";  ptr +=temp33[51 ]; ptr +=","; ptr +=temp44[51 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[52];  ptr +="',"; ptr +=temp11[52 ]; ptr +=","; ptr +=temp22[52 ]; ptr +=",";  ptr +=temp33[52 ]; ptr +=","; ptr +=temp44[52 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[53];  ptr +="',"; ptr +=temp11[53 ]; ptr +=","; ptr +=temp22[53 ]; ptr +=",";  ptr +=temp33[53 ]; ptr +=","; ptr +=temp44[53 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[54];  ptr +="',"; ptr +=temp11[54 ]; ptr +=","; ptr +=temp22[54 ]; ptr +=",";  ptr +=temp33[54 ]; ptr +=","; ptr +=temp44[54 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[55];  ptr +="',"; ptr +=temp11[55 ]; ptr +=","; ptr +=temp22[55 ]; ptr +=",";  ptr +=temp33[55 ]; ptr +=","; ptr +=temp44[55 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[56];  ptr +="',"; ptr +=temp11[56 ]; ptr +=","; ptr +=temp22[56 ]; ptr +=",";  ptr +=temp33[56 ]; ptr +=","; ptr +=temp44[56 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[57];  ptr +="',"; ptr +=temp11[57 ]; ptr +=","; ptr +=temp22[57 ]; ptr +=",";  ptr +=temp33[57 ]; ptr +=","; ptr +=temp44[57 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[58];  ptr +="',"; ptr +=temp11[58 ]; ptr +=","; ptr +=temp22[58 ]; ptr +=",";  ptr +=temp33[58 ]; ptr +=","; ptr +=temp44[58 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[59];  ptr +="',"; ptr +=temp11[59 ]; ptr +=","; ptr +=temp22[59 ]; ptr +=",";  ptr +=temp33[59 ]; ptr +=","; ptr +=temp44[59 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[60];  ptr +="',"; ptr +=temp11[60 ]; ptr +=","; ptr +=temp22[60 ]; ptr +=",";  ptr +=temp33[60 ]; ptr +=","; ptr +=temp44[60 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[61];  ptr +="',"; ptr +=temp11[61 ]; ptr +=","; ptr +=temp22[61 ]; ptr +=",";  ptr +=temp33[61 ]; ptr +=","; ptr +=temp44[61 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[62];  ptr +="',"; ptr +=temp11[62 ]; ptr +=","; ptr +=temp22[62 ]; ptr +=",";  ptr +=temp33[62 ]; ptr +=","; ptr +=temp44[62 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[63];  ptr +="',"; ptr +=temp11[63 ]; ptr +=","; ptr +=temp22[63 ]; ptr +=",";  ptr +=temp33[63 ]; ptr +=","; ptr +=temp44[63 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[64];  ptr +="',"; ptr +=temp11[64 ]; ptr +=","; ptr +=temp22[64 ]; ptr +=",";  ptr +=temp33[64 ]; ptr +=","; ptr +=temp44[64 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[65];  ptr +="',"; ptr +=temp11[65 ]; ptr +=","; ptr +=temp22[65 ]; ptr +=",";  ptr +=temp33[65 ]; ptr +=","; ptr +=temp44[65 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[66];  ptr +="',"; ptr +=temp11[66 ]; ptr +=","; ptr +=temp22[66 ]; ptr +=",";  ptr +=temp33[66 ]; ptr +=","; ptr +=temp44[66 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[67];  ptr +="',"; ptr +=temp11[67 ]; ptr +=","; ptr +=temp22[67 ]; ptr +=",";  ptr +=temp33[67 ]; ptr +=","; ptr +=temp44[67 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[68];  ptr +="',"; ptr +=temp11[68 ]; ptr +=","; ptr +=temp22[68 ]; ptr +=",";  ptr +=temp33[68 ]; ptr +=","; ptr +=temp44[68 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[69];  ptr +="',"; ptr +=temp11[69 ]; ptr +=","; ptr +=temp22[69 ]; ptr +=",";  ptr +=temp33[69 ]; ptr +=","; ptr +=temp44[69 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[70];  ptr +="',"; ptr +=temp11[70 ]; ptr +=","; ptr +=temp22[70 ]; ptr +=",";  ptr +=temp33[70 ]; ptr +=","; ptr +=temp44[70 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[71];  ptr +="',"; ptr +=temp11[71 ]; ptr +=","; ptr +=temp22[71 ]; ptr +=",";  ptr +=temp33[71 ]; ptr +=","; ptr +=temp44[71 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[72];  ptr +="',"; ptr +=temp11[72 ]; ptr +=","; ptr +=temp22[72 ]; ptr +=",";  ptr +=temp33[72 ]; ptr +=","; ptr +=temp44[72 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[73];  ptr +="',"; ptr +=temp11[73 ]; ptr +=","; ptr +=temp22[73 ]; ptr +=",";  ptr +=temp33[73 ]; ptr +=","; ptr +=temp44[73 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[74];  ptr +="',"; ptr +=temp11[74 ]; ptr +=","; ptr +=temp22[74 ]; ptr +=",";  ptr +=temp33[74 ]; ptr +=","; ptr +=temp44[74 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[75];  ptr +="',"; ptr +=temp11[75 ]; ptr +=","; ptr +=temp22[75 ]; ptr +=",";  ptr +=temp33[75 ]; ptr +=","; ptr +=temp44[75 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[76];  ptr +="',"; ptr +=temp11[76 ]; ptr +=","; ptr +=temp22[76 ]; ptr +=",";  ptr +=temp33[76 ]; ptr +=","; ptr +=temp44[76 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[77];  ptr +="',"; ptr +=temp11[77 ]; ptr +=","; ptr +=temp22[77 ]; ptr +=",";  ptr +=temp33[77 ]; ptr +=","; ptr +=temp44[77 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[78];  ptr +="',"; ptr +=temp11[78 ]; ptr +=","; ptr +=temp22[78 ]; ptr +=",";  ptr +=temp33[78 ]; ptr +=","; ptr +=temp44[78 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[79];  ptr +="',"; ptr +=temp11[79 ]; ptr +=","; ptr +=temp22[79 ]; ptr +=",";  ptr +=temp33[79 ]; ptr +=","; ptr +=temp44[79 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[80];  ptr +="',"; ptr +=temp11[80 ]; ptr +=","; ptr +=temp22[80 ]; ptr +=",";  ptr +=temp33[80 ]; ptr +=","; ptr +=temp44[80 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[81];  ptr +="',"; ptr +=temp11[81 ]; ptr +=","; ptr +=temp22[81 ]; ptr +=",";  ptr +=temp33[81 ]; ptr +=","; ptr +=temp44[81 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[82];  ptr +="',"; ptr +=temp11[82 ]; ptr +=","; ptr +=temp22[82 ]; ptr +=",";  ptr +=temp33[82 ]; ptr +=","; ptr +=temp44[82 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[83];  ptr +="',"; ptr +=temp11[83 ]; ptr +=","; ptr +=temp22[83 ]; ptr +=",";  ptr +=temp33[83 ]; ptr +=","; ptr +=temp44[83 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[84];  ptr +="',"; ptr +=temp11[84 ]; ptr +=","; ptr +=temp22[84 ]; ptr +=",";  ptr +=temp33[84 ]; ptr +=","; ptr +=temp44[84 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[85];  ptr +="',"; ptr +=temp11[85 ]; ptr +=","; ptr +=temp22[85 ]; ptr +=",";  ptr +=temp33[85 ]; ptr +=","; ptr +=temp44[85 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[86];  ptr +="',"; ptr +=temp11[86 ]; ptr +=","; ptr +=temp22[86 ]; ptr +=",";  ptr +=temp33[86 ]; ptr +=","; ptr +=temp44[86 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[87];  ptr +="',"; ptr +=temp11[87 ]; ptr +=","; ptr +=temp22[87 ]; ptr +=",";  ptr +=temp33[87 ]; ptr +=","; ptr +=temp44[87 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[88];  ptr +="',"; ptr +=temp11[88 ]; ptr +=","; ptr +=temp22[88 ]; ptr +=",";  ptr +=temp33[88 ]; ptr +=","; ptr +=temp44[88 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[89];  ptr +="',"; ptr +=temp11[89 ]; ptr +=","; ptr +=temp22[89 ]; ptr +=",";  ptr +=temp33[89 ]; ptr +=","; ptr +=temp44[89 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[90];  ptr +="',"; ptr +=temp11[90 ]; ptr +=","; ptr +=temp22[90 ]; ptr +=",";  ptr +=temp33[90 ]; ptr +=","; ptr +=temp44[90 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[91];  ptr +="',"; ptr +=temp11[91 ]; ptr +=","; ptr +=temp22[91 ]; ptr +=",";  ptr +=temp33[91 ]; ptr +=","; ptr +=temp44[91 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[92];  ptr +="',"; ptr +=temp11[92 ]; ptr +=","; ptr +=temp22[92 ]; ptr +=",";  ptr +=temp33[92 ]; ptr +=","; ptr +=temp44[92 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[93];  ptr +="',"; ptr +=temp11[93 ]; ptr +=","; ptr +=temp22[93 ]; ptr +=",";  ptr +=temp33[93 ]; ptr +=","; ptr +=temp44[93 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[94];  ptr +="',"; ptr +=temp11[94 ]; ptr +=","; ptr +=temp22[94 ]; ptr +=",";  ptr +=temp33[94 ]; ptr +=","; ptr +=temp44[94 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[95];  ptr +="',"; ptr +=temp11[95 ]; ptr +=","; ptr +=temp22[95 ]; ptr +=",";  ptr +=temp33[95 ]; ptr +=","; ptr +=temp44[95 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[96];  ptr +="',"; ptr +=temp11[96 ]; ptr +=","; ptr +=temp22[96 ]; ptr +=",";  ptr +=temp33[96 ]; ptr +=","; ptr +=temp44[96 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[97];  ptr +="',"; ptr +=temp11[97 ]; ptr +=","; ptr +=temp22[97 ]; ptr +=",";  ptr +=temp33[97 ]; ptr +=","; ptr +=temp44[97 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[98];  ptr +="',"; ptr +=temp11[98 ]; ptr +=","; ptr +=temp22[98 ]; ptr +=",";  ptr +=temp33[98 ]; ptr +=","; ptr +=temp44[98 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[99];  ptr +="',"; ptr +=temp11[99 ]; ptr +=","; ptr +=temp22[99 ]; ptr +=",";  ptr +=temp33[99 ]; ptr +=","; ptr +=temp44[99 ];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[100]; ptr +="',"; ptr +=temp11[100]; ptr +=","; ptr +=temp22[100]; ptr +=",";  ptr +=temp33[100]; ptr +=","; ptr +=temp44[100];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[101]; ptr +="',"; ptr +=temp11[101]; ptr +=","; ptr +=temp22[101]; ptr +=",";  ptr +=temp33[101]; ptr +=","; ptr +=temp44[101];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[102]; ptr +="',"; ptr +=temp11[102]; ptr +=","; ptr +=temp22[102]; ptr +=",";  ptr +=temp33[102]; ptr +=","; ptr +=temp44[102];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[103]; ptr +="',"; ptr +=temp11[103]; ptr +=","; ptr +=temp22[103]; ptr +=",";  ptr +=temp33[103]; ptr +=","; ptr +=temp44[103];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[104]; ptr +="',"; ptr +=temp11[104]; ptr +=","; ptr +=temp22[104]; ptr +=",";  ptr +=temp33[104]; ptr +=","; ptr +=temp44[104];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[105]; ptr +="',"; ptr +=temp11[105]; ptr +=","; ptr +=temp22[105]; ptr +=",";  ptr +=temp33[105]; ptr +=","; ptr +=temp44[105];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[106]; ptr +="',"; ptr +=temp11[106]; ptr +=","; ptr +=temp22[106]; ptr +=",";  ptr +=temp33[106]; ptr +=","; ptr +=temp44[106];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[107]; ptr +="',"; ptr +=temp11[107]; ptr +=","; ptr +=temp22[107]; ptr +=",";  ptr +=temp33[107]; ptr +=","; ptr +=temp44[107];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[108]; ptr +="',"; ptr +=temp11[108]; ptr +=","; ptr +=temp22[108]; ptr +=",";  ptr +=temp33[108]; ptr +=","; ptr +=temp44[108];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[109]; ptr +="',"; ptr +=temp11[109]; ptr +=","; ptr +=temp22[109]; ptr +=",";  ptr +=temp33[109]; ptr +=","; ptr +=temp44[109];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[110]; ptr +="',"; ptr +=temp11[110]; ptr +=","; ptr +=temp22[110]; ptr +=",";  ptr +=temp33[110]; ptr +=","; ptr +=temp44[110];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[111]; ptr +="',"; ptr +=temp11[111]; ptr +=","; ptr +=temp22[111]; ptr +=",";  ptr +=temp33[111]; ptr +=","; ptr +=temp44[111];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[112]; ptr +="',"; ptr +=temp11[112]; ptr +=","; ptr +=temp22[112]; ptr +=",";  ptr +=temp33[112]; ptr +=","; ptr +=temp44[112];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[113]; ptr +="',"; ptr +=temp11[113]; ptr +=","; ptr +=temp22[113]; ptr +=",";  ptr +=temp33[113]; ptr +=","; ptr +=temp44[113];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[114]; ptr +="',"; ptr +=temp11[114]; ptr +=","; ptr +=temp22[114]; ptr +=",";  ptr +=temp33[114]; ptr +=","; ptr +=temp44[114];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[115]; ptr +="',"; ptr +=temp11[115]; ptr +=","; ptr +=temp22[115]; ptr +=",";  ptr +=temp33[115]; ptr +=","; ptr +=temp44[115];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[116]; ptr +="',"; ptr +=temp11[116]; ptr +=","; ptr +=temp22[116]; ptr +=",";  ptr +=temp33[116]; ptr +=","; ptr +=temp44[116];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[117]; ptr +="',"; ptr +=temp11[117]; ptr +=","; ptr +=temp22[117]; ptr +=",";  ptr +=temp33[117]; ptr +=","; ptr +=temp44[117];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[118]; ptr +="',"; ptr +=temp11[118]; ptr +=","; ptr +=temp22[118]; ptr +=",";  ptr +=temp33[118]; ptr +=","; ptr +=temp44[118];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[119]; ptr +="',"; ptr +=temp11[119]; ptr +=","; ptr +=temp22[119]; ptr +=",";  ptr +=temp33[119]; ptr +=","; ptr +=temp44[119];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[120]; ptr +="',"; ptr +=temp11[120]; ptr +=","; ptr +=temp22[120]; ptr +=",";  ptr +=temp33[120]; ptr +=","; ptr +=temp44[120];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[121]; ptr +="',"; ptr +=temp11[121]; ptr +=","; ptr +=temp22[121]; ptr +=",";  ptr +=temp33[121]; ptr +=","; ptr +=temp44[121];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[122]; ptr +="',"; ptr +=temp11[122]; ptr +=","; ptr +=temp22[122]; ptr +=",";  ptr +=temp33[122]; ptr +=","; ptr +=temp44[122];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[123]; ptr +="',"; ptr +=temp11[123]; ptr +=","; ptr +=temp22[123]; ptr +=",";  ptr +=temp33[123]; ptr +=","; ptr +=temp44[123];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[124]; ptr +="',"; ptr +=temp11[124]; ptr +=","; ptr +=temp22[124]; ptr +=",";  ptr +=temp33[124]; ptr +=","; ptr +=temp44[124];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[125]; ptr +="',"; ptr +=temp11[125]; ptr +=","; ptr +=temp22[125]; ptr +=",";  ptr +=temp33[125]; ptr +=","; ptr +=temp44[125];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[126]; ptr +="',"; ptr +=temp11[126]; ptr +=","; ptr +=temp22[126]; ptr +=",";  ptr +=temp33[126]; ptr +=","; ptr +=temp44[126];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[127]; ptr +="',"; ptr +=temp11[127]; ptr +=","; ptr +=temp22[127]; ptr +=",";  ptr +=temp33[127]; ptr +=","; ptr +=temp44[127];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[128]; ptr +="',"; ptr +=temp11[128]; ptr +=","; ptr +=temp22[128]; ptr +=",";  ptr +=temp33[128]; ptr +=","; ptr +=temp44[128];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[129]; ptr +="',"; ptr +=temp11[129]; ptr +=","; ptr +=temp22[129]; ptr +=",";  ptr +=temp33[129]; ptr +=","; ptr +=temp44[129];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[130]; ptr +="',"; ptr +=temp11[130]; ptr +=","; ptr +=temp22[130]; ptr +=",";  ptr +=temp33[130]; ptr +=","; ptr +=temp44[130];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[131]; ptr +="',"; ptr +=temp11[131]; ptr +=","; ptr +=temp22[131]; ptr +=",";  ptr +=temp33[131]; ptr +=","; ptr +=temp44[131];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[132]; ptr +="',"; ptr +=temp11[132]; ptr +=","; ptr +=temp22[132]; ptr +=",";  ptr +=temp33[132]; ptr +=","; ptr +=temp44[132];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[133]; ptr +="',"; ptr +=temp11[133]; ptr +=","; ptr +=temp22[133]; ptr +=",";  ptr +=temp33[133]; ptr +=","; ptr +=temp44[133];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[134]; ptr +="',"; ptr +=temp11[134]; ptr +=","; ptr +=temp22[134]; ptr +=",";  ptr +=temp33[134]; ptr +=","; ptr +=temp44[134];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[135]; ptr +="',"; ptr +=temp11[135]; ptr +=","; ptr +=temp22[135]; ptr +=",";  ptr +=temp33[135]; ptr +=","; ptr +=temp44[135];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[136]; ptr +="',"; ptr +=temp11[136]; ptr +=","; ptr +=temp22[136]; ptr +=",";  ptr +=temp33[136]; ptr +=","; ptr +=temp44[136];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[137]; ptr +="',"; ptr +=temp11[137]; ptr +=","; ptr +=temp22[137]; ptr +=",";  ptr +=temp33[137]; ptr +=","; ptr +=temp44[137];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[138]; ptr +="',"; ptr +=temp11[138]; ptr +=","; ptr +=temp22[138]; ptr +=",";  ptr +=temp33[138]; ptr +=","; ptr +=temp44[138];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[139]; ptr +="',"; ptr +=temp11[139]; ptr +=","; ptr +=temp22[139]; ptr +=",";  ptr +=temp33[139]; ptr +=","; ptr +=temp44[139];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[140]; ptr +="',"; ptr +=temp11[140]; ptr +=","; ptr +=temp22[140]; ptr +=",";  ptr +=temp33[140]; ptr +=","; ptr +=temp44[140];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[141]; ptr +="',"; ptr +=temp11[141]; ptr +=","; ptr +=temp22[141]; ptr +=",";  ptr +=temp33[141]; ptr +=","; ptr +=temp44[141];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[142]; ptr +="',"; ptr +=temp11[142]; ptr +=","; ptr +=temp22[142]; ptr +=",";  ptr +=temp33[142]; ptr +=","; ptr +=temp44[142];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[143]; ptr +="',"; ptr +=temp11[143]; ptr +=","; ptr +=temp22[143]; ptr +=",";  ptr +=temp33[143]; ptr +=","; ptr +=temp44[143];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[144]; ptr +="',"; ptr +=temp11[144]; ptr +=","; ptr +=temp22[144]; ptr +=",";  ptr +=temp33[144]; ptr +=","; ptr +=temp44[144];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[145]; ptr +="',"; ptr +=temp11[145]; ptr +=","; ptr +=temp22[145]; ptr +=",";  ptr +=temp33[145]; ptr +=","; ptr +=temp44[145];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[146]; ptr +="',"; ptr +=temp11[146]; ptr +=","; ptr +=temp22[146]; ptr +=",";  ptr +=temp33[146]; ptr +=","; ptr +=temp44[146];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[147]; ptr +="',"; ptr +=temp11[147]; ptr +=","; ptr +=temp22[147]; ptr +=",";  ptr +=temp33[147]; ptr +=","; ptr +=temp44[147];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[148]; ptr +="',"; ptr +=temp11[148]; ptr +=","; ptr +=temp22[148]; ptr +=",";  ptr +=temp33[148]; ptr +=","; ptr +=temp44[148];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[149]; ptr +="',"; ptr +=temp11[149]; ptr +=","; ptr +=temp22[149]; ptr +=",";  ptr +=temp33[149]; ptr +=","; ptr +=temp44[149];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[150]; ptr +="',"; ptr +=temp11[150]; ptr +=","; ptr +=temp22[150]; ptr +=",";  ptr +=temp33[150]; ptr +=","; ptr +=temp44[150];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[151]; ptr +="',"; ptr +=temp11[151]; ptr +=","; ptr +=temp22[151]; ptr +=",";  ptr +=temp33[151]; ptr +=","; ptr +=temp44[151];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[152]; ptr +="',"; ptr +=temp11[152]; ptr +=","; ptr +=temp22[152]; ptr +=",";  ptr +=temp33[152]; ptr +=","; ptr +=temp44[152];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[153]; ptr +="',"; ptr +=temp11[153]; ptr +=","; ptr +=temp22[153]; ptr +=",";  ptr +=temp33[153]; ptr +=","; ptr +=temp44[153];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[154]; ptr +="',"; ptr +=temp11[154]; ptr +=","; ptr +=temp22[154]; ptr +=",";  ptr +=temp33[154]; ptr +=","; ptr +=temp44[154];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[155]; ptr +="',"; ptr +=temp11[155]; ptr +=","; ptr +=temp22[155]; ptr +=",";  ptr +=temp33[155]; ptr +=","; ptr +=temp44[155];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[156]; ptr +="',"; ptr +=temp11[156]; ptr +=","; ptr +=temp22[156]; ptr +=",";  ptr +=temp33[156]; ptr +=","; ptr +=temp44[156];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[157]; ptr +="',"; ptr +=temp11[157]; ptr +=","; ptr +=temp22[157]; ptr +=",";  ptr +=temp33[157]; ptr +=","; ptr +=temp44[157];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[158]; ptr +="',"; ptr +=temp11[158]; ptr +=","; ptr +=temp22[158]; ptr +=",";  ptr +=temp33[158]; ptr +=","; ptr +=temp44[158];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[159]; ptr +="',"; ptr +=temp11[159]; ptr +=","; ptr +=temp22[159]; ptr +=",";  ptr +=temp33[159]; ptr +=","; ptr +=temp44[159];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[160]; ptr +="',"; ptr +=temp11[160]; ptr +=","; ptr +=temp22[160]; ptr +=",";  ptr +=temp33[160]; ptr +=","; ptr +=temp44[160];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[161]; ptr +="',"; ptr +=temp11[161]; ptr +=","; ptr +=temp22[161]; ptr +=",";  ptr +=temp33[161]; ptr +=","; ptr +=temp44[161];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[162]; ptr +="',"; ptr +=temp11[162]; ptr +=","; ptr +=temp22[162]; ptr +=",";  ptr +=temp33[162]; ptr +=","; ptr +=temp44[162];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[163]; ptr +="',"; ptr +=temp11[163]; ptr +=","; ptr +=temp22[163]; ptr +=",";  ptr +=temp33[163]; ptr +=","; ptr +=temp44[163];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[164]; ptr +="',"; ptr +=temp11[164]; ptr +=","; ptr +=temp22[164]; ptr +=",";  ptr +=temp33[164]; ptr +=","; ptr +=temp44[164];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[165]; ptr +="',"; ptr +=temp11[165]; ptr +=","; ptr +=temp22[165]; ptr +=",";  ptr +=temp33[165]; ptr +=","; ptr +=temp44[165];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[166]; ptr +="',"; ptr +=temp11[166]; ptr +=","; ptr +=temp22[166]; ptr +=",";  ptr +=temp33[166]; ptr +=","; ptr +=temp44[166];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[167]; ptr +="',"; ptr +=temp11[167]; ptr +=","; ptr +=temp22[167]; ptr +=",";  ptr +=temp33[167]; ptr +=","; ptr +=temp44[167];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[168]; ptr +="',"; ptr +=temp11[168]; ptr +=","; ptr +=temp22[168]; ptr +=",";  ptr +=temp33[168]; ptr +=","; ptr +=temp44[168];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[169]; ptr +="',"; ptr +=temp11[169]; ptr +=","; ptr +=temp22[169]; ptr +=",";  ptr +=temp33[169]; ptr +=","; ptr +=temp44[169];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[170]; ptr +="',"; ptr +=temp11[170]; ptr +=","; ptr +=temp22[170]; ptr +=",";  ptr +=temp33[170]; ptr +=","; ptr +=temp44[170];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[171]; ptr +="',"; ptr +=temp11[171]; ptr +=","; ptr +=temp22[171]; ptr +=",";  ptr +=temp33[171]; ptr +=","; ptr +=temp44[171];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[172]; ptr +="',"; ptr +=temp11[172]; ptr +=","; ptr +=temp22[172]; ptr +=",";  ptr +=temp33[172]; ptr +=","; ptr +=temp44[172];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[173]; ptr +="',"; ptr +=temp11[173]; ptr +=","; ptr +=temp22[173]; ptr +=",";  ptr +=temp33[173]; ptr +=","; ptr +=temp44[173];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[174]; ptr +="',"; ptr +=temp11[174]; ptr +=","; ptr +=temp22[174]; ptr +=",";  ptr +=temp33[174]; ptr +=","; ptr +=temp44[174];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[175]; ptr +="',"; ptr +=temp11[175]; ptr +=","; ptr +=temp22[175]; ptr +=",";  ptr +=temp33[175]; ptr +=","; ptr +=temp44[175];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[176]; ptr +="',"; ptr +=temp11[176]; ptr +=","; ptr +=temp22[176]; ptr +=",";  ptr +=temp33[176]; ptr +=","; ptr +=temp44[176];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[177]; ptr +="',"; ptr +=temp11[177]; ptr +=","; ptr +=temp22[177]; ptr +=",";  ptr +=temp33[177]; ptr +=","; ptr +=temp44[177];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[178]; ptr +="',"; ptr +=temp11[178]; ptr +=","; ptr +=temp22[178]; ptr +=",";  ptr +=temp33[178]; ptr +=","; ptr +=temp44[178];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[179]; ptr +="',"; ptr +=temp11[179]; ptr +=","; ptr +=temp22[179]; ptr +=",";  ptr +=temp33[179]; ptr +=","; ptr +=temp44[179];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[180]; ptr +="',"; ptr +=temp11[180]; ptr +=","; ptr +=temp22[180]; ptr +=",";  ptr +=temp33[180]; ptr +=","; ptr +=temp44[180];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[181]; ptr +="',"; ptr +=temp11[181]; ptr +=","; ptr +=temp22[181]; ptr +=",";  ptr +=temp33[181]; ptr +=","; ptr +=temp44[181];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[182]; ptr +="',"; ptr +=temp11[182]; ptr +=","; ptr +=temp22[182]; ptr +=",";  ptr +=temp33[182]; ptr +=","; ptr +=temp44[182];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[183]; ptr +="',"; ptr +=temp11[183]; ptr +=","; ptr +=temp22[183]; ptr +=",";  ptr +=temp33[183]; ptr +=","; ptr +=temp44[183];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[184]; ptr +="',"; ptr +=temp11[184]; ptr +=","; ptr +=temp22[184]; ptr +=",";  ptr +=temp33[184]; ptr +=","; ptr +=temp44[184];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[185]; ptr +="',"; ptr +=temp11[185]; ptr +=","; ptr +=temp22[185]; ptr +=",";  ptr +=temp33[185]; ptr +=","; ptr +=temp44[185];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[186]; ptr +="',"; ptr +=temp11[186]; ptr +=","; ptr +=temp22[186]; ptr +=",";  ptr +=temp33[186]; ptr +=","; ptr +=temp44[186];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[187]; ptr +="',"; ptr +=temp11[187]; ptr +=","; ptr +=temp22[187]; ptr +=",";  ptr +=temp33[187]; ptr +=","; ptr +=temp44[187];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[188]; ptr +="',"; ptr +=temp11[188]; ptr +=","; ptr +=temp22[188]; ptr +=",";  ptr +=temp33[188]; ptr +=","; ptr +=temp44[188];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[189]; ptr +="',"; ptr +=temp11[189]; ptr +=","; ptr +=temp22[189]; ptr +=",";  ptr +=temp33[189]; ptr +=","; ptr +=temp44[189];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[190]; ptr +="',"; ptr +=temp11[190]; ptr +=","; ptr +=temp22[190]; ptr +=",";  ptr +=temp33[190]; ptr +=","; ptr +=temp44[190];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[191]; ptr +="',"; ptr +=temp11[191]; ptr +=","; ptr +=temp22[191]; ptr +=",";  ptr +=temp33[191]; ptr +=","; ptr +=temp44[191];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[192]; ptr +="',"; ptr +=temp11[192]; ptr +=","; ptr +=temp22[192]; ptr +=",";  ptr +=temp33[192]; ptr +=","; ptr +=temp44[192];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[193]; ptr +="',"; ptr +=temp11[193]; ptr +=","; ptr +=temp22[193]; ptr +=",";  ptr +=temp33[193]; ptr +=","; ptr +=temp44[193];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[194]; ptr +="',"; ptr +=temp11[194]; ptr +=","; ptr +=temp22[194]; ptr +=",";  ptr +=temp33[194]; ptr +=","; ptr +=temp44[194];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[195]; ptr +="',"; ptr +=temp11[195]; ptr +=","; ptr +=temp22[195]; ptr +=",";  ptr +=temp33[195]; ptr +=","; ptr +=temp44[195];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[196]; ptr +="',"; ptr +=temp11[196]; ptr +=","; ptr +=temp22[196]; ptr +=",";  ptr +=temp33[196]; ptr +=","; ptr +=temp44[196];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[197]; ptr +="',"; ptr +=temp11[197]; ptr +=","; ptr +=temp22[197]; ptr +=",";  ptr +=temp33[197]; ptr +=","; ptr +=temp44[197];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[198]; ptr +="',"; ptr +=temp11[198]; ptr +=","; ptr +=temp22[198]; ptr +=",";  ptr +=temp33[198]; ptr +=","; ptr +=temp44[198];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[199]; ptr +="',"; ptr +=temp11[199]; ptr +=","; ptr +=temp22[199]; ptr +=",";  ptr +=temp33[199]; ptr +=","; ptr +=temp44[199];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[200]; ptr +="',"; ptr +=temp11[200]; ptr +=","; ptr +=temp22[200]; ptr +=",";  ptr +=temp33[200]; ptr +=","; ptr +=temp44[200];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[201]; ptr +="',"; ptr +=temp11[201]; ptr +=","; ptr +=temp22[201]; ptr +=",";  ptr +=temp33[201]; ptr +=","; ptr +=temp44[201];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[202]; ptr +="',"; ptr +=temp11[202]; ptr +=","; ptr +=temp22[202]; ptr +=",";  ptr +=temp33[202]; ptr +=","; ptr +=temp44[202];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[203]; ptr +="',"; ptr +=temp11[203]; ptr +=","; ptr +=temp22[203]; ptr +=",";  ptr +=temp33[203]; ptr +=","; ptr +=temp44[203];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[204]; ptr +="',"; ptr +=temp11[204]; ptr +=","; ptr +=temp22[204]; ptr +=",";  ptr +=temp33[204]; ptr +=","; ptr +=temp44[204];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[205]; ptr +="',"; ptr +=temp11[205]; ptr +=","; ptr +=temp22[205]; ptr +=",";  ptr +=temp33[205]; ptr +=","; ptr +=temp44[205];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[206]; ptr +="',"; ptr +=temp11[206]; ptr +=","; ptr +=temp22[206]; ptr +=",";  ptr +=temp33[206]; ptr +=","; ptr +=temp44[206];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[207]; ptr +="',"; ptr +=temp11[207]; ptr +=","; ptr +=temp22[207]; ptr +=",";  ptr +=temp33[207]; ptr +=","; ptr +=temp44[207];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[208]; ptr +="',"; ptr +=temp11[208]; ptr +=","; ptr +=temp22[208]; ptr +=",";  ptr +=temp33[208]; ptr +=","; ptr +=temp44[208];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[209]; ptr +="',"; ptr +=temp11[209]; ptr +=","; ptr +=temp22[209]; ptr +=",";  ptr +=temp33[209]; ptr +=","; ptr +=temp44[209];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[210]; ptr +="',"; ptr +=temp11[210]; ptr +=","; ptr +=temp22[210]; ptr +=",";  ptr +=temp33[210]; ptr +=","; ptr +=temp44[210];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[211]; ptr +="',"; ptr +=temp11[211]; ptr +=","; ptr +=temp22[211]; ptr +=",";  ptr +=temp33[211]; ptr +=","; ptr +=temp44[211];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[212]; ptr +="',"; ptr +=temp11[212]; ptr +=","; ptr +=temp22[212]; ptr +=",";  ptr +=temp33[212]; ptr +=","; ptr +=temp44[212];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[213]; ptr +="',"; ptr +=temp11[213]; ptr +=","; ptr +=temp22[213]; ptr +=",";  ptr +=temp33[213]; ptr +=","; ptr +=temp44[213];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[214]; ptr +="',"; ptr +=temp11[214]; ptr +=","; ptr +=temp22[214]; ptr +=",";  ptr +=temp33[214]; ptr +=","; ptr +=temp44[214];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[215]; ptr +="',"; ptr +=temp11[215]; ptr +=","; ptr +=temp22[215]; ptr +=",";  ptr +=temp33[215]; ptr +=","; ptr +=temp44[215];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[216]; ptr +="',"; ptr +=temp11[216]; ptr +=","; ptr +=temp22[216]; ptr +=",";  ptr +=temp33[216]; ptr +=","; ptr +=temp44[216];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[217]; ptr +="',"; ptr +=temp11[217]; ptr +=","; ptr +=temp22[217]; ptr +=",";  ptr +=temp33[217]; ptr +=","; ptr +=temp44[217];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[218]; ptr +="',"; ptr +=temp11[218]; ptr +=","; ptr +=temp22[218]; ptr +=",";  ptr +=temp33[218]; ptr +=","; ptr +=temp44[218];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[219]; ptr +="',"; ptr +=temp11[219]; ptr +=","; ptr +=temp22[219]; ptr +=",";  ptr +=temp33[219]; ptr +=","; ptr +=temp44[219];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[220]; ptr +="',"; ptr +=temp11[220]; ptr +=","; ptr +=temp22[220]; ptr +=",";  ptr +=temp33[220]; ptr +=","; ptr +=temp44[220];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[221]; ptr +="',"; ptr +=temp11[221]; ptr +=","; ptr +=temp22[221]; ptr +=",";  ptr +=temp33[221]; ptr +=","; ptr +=temp44[221];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[222]; ptr +="',"; ptr +=temp11[222]; ptr +=","; ptr +=temp22[222]; ptr +=",";  ptr +=temp33[222]; ptr +=","; ptr +=temp44[222];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[223]; ptr +="',"; ptr +=temp11[223]; ptr +=","; ptr +=temp22[223]; ptr +=",";  ptr +=temp33[223]; ptr +=","; ptr +=temp44[223];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[224]; ptr +="',"; ptr +=temp11[224]; ptr +=","; ptr +=temp22[224]; ptr +=",";  ptr +=temp33[224]; ptr +=","; ptr +=temp44[224];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[225]; ptr +="',"; ptr +=temp11[225]; ptr +=","; ptr +=temp22[225]; ptr +=",";  ptr +=temp33[225]; ptr +=","; ptr +=temp44[225];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[226]; ptr +="',"; ptr +=temp11[226]; ptr +=","; ptr +=temp22[226]; ptr +=",";  ptr +=temp33[226]; ptr +=","; ptr +=temp44[226];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[227]; ptr +="',"; ptr +=temp11[227]; ptr +=","; ptr +=temp22[227]; ptr +=",";  ptr +=temp33[227]; ptr +=","; ptr +=temp44[227];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[228]; ptr +="',"; ptr +=temp11[228]; ptr +=","; ptr +=temp22[228]; ptr +=",";  ptr +=temp33[228]; ptr +=","; ptr +=temp44[228];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[229]; ptr +="',"; ptr +=temp11[229]; ptr +=","; ptr +=temp22[229]; ptr +=",";  ptr +=temp33[229]; ptr +=","; ptr +=temp44[229];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[230]; ptr +="',"; ptr +=temp11[230]; ptr +=","; ptr +=temp22[230]; ptr +=",";  ptr +=temp33[230]; ptr +=","; ptr +=temp44[230];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[231]; ptr +="',"; ptr +=temp11[231]; ptr +=","; ptr +=temp22[231]; ptr +=",";  ptr +=temp33[231]; ptr +=","; ptr +=temp44[231];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[232]; ptr +="',"; ptr +=temp11[232]; ptr +=","; ptr +=temp22[232]; ptr +=",";  ptr +=temp33[232]; ptr +=","; ptr +=temp44[232];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[233]; ptr +="',"; ptr +=temp11[233]; ptr +=","; ptr +=temp22[233]; ptr +=",";  ptr +=temp33[233]; ptr +=","; ptr +=temp44[233];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[234]; ptr +="',"; ptr +=temp11[234]; ptr +=","; ptr +=temp22[234]; ptr +=",";  ptr +=temp33[234]; ptr +=","; ptr +=temp44[234];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[235]; ptr +="',"; ptr +=temp11[235]; ptr +=","; ptr +=temp22[235]; ptr +=",";  ptr +=temp33[235]; ptr +=","; ptr +=temp44[235];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[236]; ptr +="',"; ptr +=temp11[236]; ptr +=","; ptr +=temp22[236]; ptr +=",";  ptr +=temp33[236]; ptr +=","; ptr +=temp44[236];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[237]; ptr +="',"; ptr +=temp11[237]; ptr +=","; ptr +=temp22[237]; ptr +=",";  ptr +=temp33[237]; ptr +=","; ptr +=temp44[237];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[238]; ptr +="',"; ptr +=temp11[238]; ptr +=","; ptr +=temp22[238]; ptr +=",";  ptr +=temp33[238]; ptr +=","; ptr +=temp44[238];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[239]; ptr +="',"; ptr +=temp11[239]; ptr +=","; ptr +=temp22[239]; ptr +=",";  ptr +=temp33[239]; ptr +=","; ptr +=temp44[239];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[240]; ptr +="',"; ptr +=temp11[240]; ptr +=","; ptr +=temp22[240]; ptr +=",";  ptr +=temp33[240]; ptr +=","; ptr +=temp44[240];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[241]; ptr +="',"; ptr +=temp11[241]; ptr +=","; ptr +=temp22[241]; ptr +=",";  ptr +=temp33[241]; ptr +=","; ptr +=temp44[241];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[242]; ptr +="',"; ptr +=temp11[242]; ptr +=","; ptr +=temp22[242]; ptr +=",";  ptr +=temp33[242]; ptr +=","; ptr +=temp44[242];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[243]; ptr +="',"; ptr +=temp11[243]; ptr +=","; ptr +=temp22[243]; ptr +=",";  ptr +=temp33[243]; ptr +=","; ptr +=temp44[243];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[244]; ptr +="',"; ptr +=temp11[244]; ptr +=","; ptr +=temp22[244]; ptr +=",";  ptr +=temp33[244]; ptr +=","; ptr +=temp44[244];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[245]; ptr +="',"; ptr +=temp11[245]; ptr +=","; ptr +=temp22[245]; ptr +=",";  ptr +=temp33[245]; ptr +=","; ptr +=temp44[245];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[246]; ptr +="',"; ptr +=temp11[246]; ptr +=","; ptr +=temp22[246]; ptr +=",";  ptr +=temp33[246]; ptr +=","; ptr +=temp44[246];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[247]; ptr +="',"; ptr +=temp11[247]; ptr +=","; ptr +=temp22[247]; ptr +=",";  ptr +=temp33[247]; ptr +=","; ptr +=temp44[247];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[248]; ptr +="',"; ptr +=temp11[248]; ptr +=","; ptr +=temp22[248]; ptr +=",";  ptr +=temp33[248]; ptr +=","; ptr +=temp44[248];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[249]; ptr +="',"; ptr +=temp11[249]; ptr +=","; ptr +=temp22[249]; ptr +=",";  ptr +=temp33[249]; ptr +=","; ptr +=temp44[249];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[250]; ptr +="',"; ptr +=temp11[250]; ptr +=","; ptr +=temp22[250]; ptr +=",";  ptr +=temp33[250]; ptr +=","; ptr +=temp44[250];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[251]; ptr +="',"; ptr +=temp11[251]; ptr +=","; ptr +=temp22[251]; ptr +=",";  ptr +=temp33[251]; ptr +=","; ptr +=temp44[251];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[252]; ptr +="',"; ptr +=temp11[252]; ptr +=","; ptr +=temp22[252]; ptr +=",";  ptr +=temp33[252]; ptr +=","; ptr +=temp44[252];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[253]; ptr +="',"; ptr +=temp11[253]; ptr +=","; ptr +=temp22[253]; ptr +=",";  ptr +=temp33[253]; ptr +=","; ptr +=temp44[253];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[254]; ptr +="',"; ptr +=temp11[254]; ptr +=","; ptr +=temp22[254]; ptr +=",";  ptr +=temp33[254]; ptr +=","; ptr +=temp44[254];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[255]; ptr +="',"; ptr +=temp11[255]; ptr +=","; ptr +=temp22[255]; ptr +=",";  ptr +=temp33[255]; ptr +=","; ptr +=temp44[255];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[256]; ptr +="',"; ptr +=temp11[256]; ptr +=","; ptr +=temp22[256]; ptr +=",";  ptr +=temp33[256]; ptr +=","; ptr +=temp44[256];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[257]; ptr +="',"; ptr +=temp11[257]; ptr +=","; ptr +=temp22[257]; ptr +=",";  ptr +=temp33[257]; ptr +=","; ptr +=temp44[257];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[258]; ptr +="',"; ptr +=temp11[258]; ptr +=","; ptr +=temp22[258]; ptr +=",";  ptr +=temp33[258]; ptr +=","; ptr +=temp44[258];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[259]; ptr +="',"; ptr +=temp11[259]; ptr +=","; ptr +=temp22[259]; ptr +=",";  ptr +=temp33[259]; ptr +=","; ptr +=temp44[259];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[260]; ptr +="',"; ptr +=temp11[260]; ptr +=","; ptr +=temp22[260]; ptr +=",";  ptr +=temp33[260]; ptr +=","; ptr +=temp44[260];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[261]; ptr +="',"; ptr +=temp11[261]; ptr +=","; ptr +=temp22[261]; ptr +=",";  ptr +=temp33[261]; ptr +=","; ptr +=temp44[261];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[262]; ptr +="',"; ptr +=temp11[262]; ptr +=","; ptr +=temp22[262]; ptr +=",";  ptr +=temp33[262]; ptr +=","; ptr +=temp44[262];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[263]; ptr +="',"; ptr +=temp11[263]; ptr +=","; ptr +=temp22[263]; ptr +=",";  ptr +=temp33[263]; ptr +=","; ptr +=temp44[263];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[264]; ptr +="',"; ptr +=temp11[264]; ptr +=","; ptr +=temp22[264]; ptr +=",";  ptr +=temp33[264]; ptr +=","; ptr +=temp44[264];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[265]; ptr +="',"; ptr +=temp11[265]; ptr +=","; ptr +=temp22[265]; ptr +=",";  ptr +=temp33[265]; ptr +=","; ptr +=temp44[265];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[266]; ptr +="',"; ptr +=temp11[266]; ptr +=","; ptr +=temp22[266]; ptr +=",";  ptr +=temp33[266]; ptr +=","; ptr +=temp44[266];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[267]; ptr +="',"; ptr +=temp11[267]; ptr +=","; ptr +=temp22[267]; ptr +=",";  ptr +=temp33[267]; ptr +=","; ptr +=temp44[267];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[268]; ptr +="',"; ptr +=temp11[268]; ptr +=","; ptr +=temp22[268]; ptr +=",";  ptr +=temp33[268]; ptr +=","; ptr +=temp44[268];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[269]; ptr +="',"; ptr +=temp11[269]; ptr +=","; ptr +=temp22[269]; ptr +=",";  ptr +=temp33[269]; ptr +=","; ptr +=temp44[269];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[270]; ptr +="',"; ptr +=temp11[270]; ptr +=","; ptr +=temp22[270]; ptr +=",";  ptr +=temp33[270]; ptr +=","; ptr +=temp44[270];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[271]; ptr +="',"; ptr +=temp11[271]; ptr +=","; ptr +=temp22[271]; ptr +=",";  ptr +=temp33[271]; ptr +=","; ptr +=temp44[271];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[272]; ptr +="',"; ptr +=temp11[272]; ptr +=","; ptr +=temp22[272]; ptr +=",";  ptr +=temp33[272]; ptr +=","; ptr +=temp44[272];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[273]; ptr +="',"; ptr +=temp11[273]; ptr +=","; ptr +=temp22[273]; ptr +=",";  ptr +=temp33[273]; ptr +=","; ptr +=temp44[273];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[274]; ptr +="',"; ptr +=temp11[274]; ptr +=","; ptr +=temp22[274]; ptr +=",";  ptr +=temp33[274]; ptr +=","; ptr +=temp44[274];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[275]; ptr +="',"; ptr +=temp11[275]; ptr +=","; ptr +=temp22[275]; ptr +=",";  ptr +=temp33[275]; ptr +=","; ptr +=temp44[275];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[276]; ptr +="',"; ptr +=temp11[276]; ptr +=","; ptr +=temp22[276]; ptr +=",";  ptr +=temp33[276]; ptr +=","; ptr +=temp44[276];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[277]; ptr +="',"; ptr +=temp11[277]; ptr +=","; ptr +=temp22[277]; ptr +=",";  ptr +=temp33[277]; ptr +=","; ptr +=temp44[277];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[278]; ptr +="',"; ptr +=temp11[278]; ptr +=","; ptr +=temp22[278]; ptr +=",";  ptr +=temp33[278]; ptr +=","; ptr +=temp44[278];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[279]; ptr +="',"; ptr +=temp11[279]; ptr +=","; ptr +=temp22[279]; ptr +=",";  ptr +=temp33[279]; ptr +=","; ptr +=temp44[279];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[280]; ptr +="',"; ptr +=temp11[280]; ptr +=","; ptr +=temp22[280]; ptr +=",";  ptr +=temp33[280]; ptr +=","; ptr +=temp44[280];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[281]; ptr +="',"; ptr +=temp11[281]; ptr +=","; ptr +=temp22[281]; ptr +=",";  ptr +=temp33[281]; ptr +=","; ptr +=temp44[281];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[282]; ptr +="',"; ptr +=temp11[282]; ptr +=","; ptr +=temp22[282]; ptr +=",";  ptr +=temp33[282]; ptr +=","; ptr +=temp44[282];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[283]; ptr +="',"; ptr +=temp11[283]; ptr +=","; ptr +=temp22[283]; ptr +=",";  ptr +=temp33[283]; ptr +=","; ptr +=temp44[283];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[284]; ptr +="',"; ptr +=temp11[284]; ptr +=","; ptr +=temp22[284]; ptr +=",";  ptr +=temp33[284]; ptr +=","; ptr +=temp44[284];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[285]; ptr +="',"; ptr +=temp11[285]; ptr +=","; ptr +=temp22[285]; ptr +=",";  ptr +=temp33[285]; ptr +=","; ptr +=temp44[285];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[286]; ptr +="',"; ptr +=temp11[286]; ptr +=","; ptr +=temp22[286]; ptr +=",";  ptr +=temp33[286]; ptr +=","; ptr +=temp44[286];  ptr +=","; ptr +="],";
ptr +="['"; ptr +=formattedDate[287]; ptr +="',"; ptr +=temp11[287]; ptr +=","; ptr +=temp22[287]; ptr +=",";  ptr +=temp33[287]; ptr +=","; ptr +=temp44[287];  ptr +=","; ptr +="],";

ptr +="['"; ptr +=formattedDate[288]; ptr +="',"; ptr +=temp11[288]; ptr +=","; ptr +=temp22[288]; ptr +=",";  ptr +=temp33[288]; ptr +=","; ptr +=temp44[288];  ptr +="]];\n";

ptr +="}\n";
ptr +="</script>                                                                                                                                                                                     \n";

ptr +="<p>WiFi Rx="; ptr +=Rx; ptr +="   dBm"; ptr +="</p>";
ptr +="<p>IP "; ptr +=ip; ptr +="   address"; ptr +="</p>";

ptr +="</body>                                                                                                                                                                                       \n";
ptr +="</html>                                                                                                                                                                                       \n";



return ptr;
 

}












