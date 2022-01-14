#include <M5Stack.h>
#include "Adafruit_Si7021.h"
#include "FS.h"
#include <SD.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

int split_time;
int t1_int;
int t1_adj;
String t1;
String status1;
String date1;
String t1_check;
String t_total;
String t1_nosec;
char c_temp[10];
char c_humid[8];
String weekDays[7] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
String months[12] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};

File file1; 
Adafruit_Si7021 sensor = Adafruit_Si7021();  // Adafruit_Si7021
const int chipSelect = 4;
String message;
const char *DATA;
WiFiUDP udp;
char ssid[] = "wifi \"heavy breathing\"";
char password[] = "RobustAgenda430";
NTPClient timeClient(udp,"1.north-america.pool.ntp.org", -14400);

class DateTime{
  public:
    String d, t;
};

void writeFile(fs::FS &fs, const char * path, const char * message){
  //M5.Lcd.setFont(&FreeSans9pt7b);
  File file = fs.open(path, FILE_WRITE);
  //M5.Lcd.fillRect(119,0,320,30,0x439);
  if(!file){
    //M5.Lcd.setCursor(120,18);
   // M5.Lcd.print("File error");
    return;
  }
  if(file.print(message)){
   // M5.Lcd.setCursor(120,18);
   // M5.Lcd.print("File ok");
//  }else{
   // M5.Lcd.setCursor(120,18);
    //M5.Lcd.print("File error");
  }
  file.close();
//  Serial.println("writeFile");
}

DateTime updatetime(){
 DateTime output;
 if(WiFi.status() != WL_CONNECTED){
   
 }else{
  timeClient.update();
  unsigned long epochTime = timeClient.getEpochTime();
  String weekDay = weekDays[timeClient.getDay()];
  Serial.println(weekDay);
  struct tm *ptm = gmtime ((time_t *)&epochTime);
  int monthDay = ptm->tm_mday;
  Serial.println(monthDay);
  int currentMonth = ptm->tm_mon+1;
  Serial.println(currentMonth);
  String currentMonthName = months[currentMonth-1];
  Serial.println(currentMonthName);
  int currentYear = ptm->tm_year+1900;
  Serial.println(currentYear);
  String currentDate = String(currentYear) + "-" + String(currentMonth) + "-" + String(monthDay);
  Serial.println(currentDate);
  output.d = currentDate;
  t1 = timeClient.getFormattedTime();
  split_time = t1.indexOf(":");
  Serial.println(split_time);
  t1_check = t1.substring(0,split_time);
  t1_nosec = t1.substring(0,split_time+3);
  Serial.println(t1_check);
  t1_int = t1_check.toInt();
  Serial.println(t1_int);

  if(t1_int > 12){
     t1_adj = (t1_int - 1) % 12;
     if(t1_adj == 0){
       t1_adj = 12;
     }
      t_total = t1_adj + t1.substring(split_time,split_time+3) + "pm"; 
    }
  else{
    t_total = t1_nosec + "am";
  }
  //M5.Lcd.setFont(&FreeSans9pt7b);
  //M5.Lcd.fillRect(245,0,320,25,0x439);
  //M5.Lcd.setCursor(250, 20);
  //M5.Lcd.print(t_total);
//  Serial.println("UpdateTime");
 }
 Serial.println(t_total);
 
 output.t = t_total;
 
 return output;
}

void postDataToServer() {
 
  Serial.println("Posting JSON data to server...");
  // Block until we are able to connect to the WiFi access point
  if (WiFi.status() == WL_CONNECTED) {
     
    HTTPClient http;   
     
    http.begin("http://10.100.10.64:1337/m5/ambient");  
    http.addHeader("Content-Type", "application/json");         
     
    StaticJsonDocument<200> doc;
    // Add values in the document
    //
    doc["deviceName"] = "M5Ambient";
    doc["type"] = "M5";
    doc["ambientTemp"] = 10;
    doc["humidity"] = 20;
    doc["status"] = "on";
   
    // Add an array.
    //
    // JsonArray data = doc.createNestedArray("data");
    // data.add(48.756080);
    // data.add(2.302038);
     
    String requestBody;
    serializeJson(doc, requestBody);
     
    int httpResponseCode = http.POST(requestBody);
 
    if(httpResponseCode>0){
       
      String response = http.getString();                       
       
      Serial.println(httpResponseCode);   
      Serial.println(response);
     
    }
    else {
     
      Serial.printf("Oops");
       
    }
     
  }
}

void setup(){
    M5.begin();  // M5Stack
    Serial.begin(9600);
    if (!sensor.begin()) {  // Si7021
        Serial.println("Did not find Si7021 sensor!");
        while (true) {
            delay(0);
        }
    }

    M5.Lcd.setTextSize(3);
    SD.begin(chipSelect);
    file1 = SD.open("/data.txt");
    if(!file1){
    writeFile(SD, "/data.txt", "Date, Time, AMB Temp, Humidity, \r\n");
    }
    file1.close();
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid,password);
    while(WiFi.status() !=WL_CONNECTED){
      Serial.println("Wifi not connect");
      delay(1000);
    }
}

void appendFile(fs::FS &fs, const char * path, const char * message){
  File file = fs.open(path, FILE_APPEND);
  //M5.Lcd.setFont(&FreeSans9pt7b);
  //M5.Lcd.fillRect(119,0,320,30,0x439);
  if(!file){
    //M5.Lcd.setCursor(120,18);
    //M5.Lcd.print("File Error");
    return;
  }
  if(file.print(message)){
    //M5.Lcd.setCursor(120,18);
    //M5.Lcd.print("File ok");
  //}else{
    //M5.Lcd.setCursor(120,18);
    //M5.Lcd.print("File Error");
  }
  file.close();
//  Serial.println("appendFile");
}

void loop(){
    float temp = sensor.readTemperature();   // Si7021
    float humid = sensor.readHumidity();     // Si7021
    M5.Lcd.setCursor(40, 80);
    M5.Lcd.print("Temp: "); 
    M5.Lcd.print(temp, 1);                   // LCD
    M5.Lcd.print("'C");
    M5.Lcd.setCursor(40, 140);
    M5.Lcd.print("Humid: ");
    M5.Lcd.print(humid, 1);                  // LCD
    M5.Lcd.print("%");
    DateTime Time;
    Time = updatetime();
    message= String(Time.d)+"," + String(Time.t) + "," + String(temp)+ "," + String(humid) + "\r\n";
    DATA= message.c_str();
    appendFile(SD,"/data.txt",DATA);
    postDataToServer();
    delay(1000);
}
