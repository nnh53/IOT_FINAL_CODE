//time
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <NTPClient.h>
#include <TimeLib.h>
//get http
#include <ESP8266HTTPClient.h>

#include <WiFiClient.h>
#include <Arduino_JSON.h>
#include <SoftwareSerial.h>

#include <WiFiClientSecureBearSSL.h>
// WIFI
const char* ssid = "Pixel";
const char* password = "ahihi123";
String openWeatherMapApiKey = "";
String city = "Ho Chi Minh";
String countryCode = "VN";
WiFiUDP ntpUDP;
// TIME
NTPClient timeClient(ntpUDP, "asia.pool.ntp.org", 25200, 60000);
unsigned long unix_epoch;  // Get Unix epoch time from the NTP server
#define RX_PIN 4           //D2
#define TX_PIN 5           //D1
SoftwareSerial espSerial(RX_PIN, TX_PIN);
// OTHER

String jsonBuffer;
String dataWeather;
String dataSettingAlarm;
String DataToSend;
String alarmSetting;
String httpGETRequest(const char* serverName);
String dateStringAlarm;

void setup() {
  Serial.begin(9600);
  espSerial.begin(9600);
  WiFi.begin(ssid, password);
  Serial.println("Connecting");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to WiFi network with IP Address: ");
  Serial.println(WiFi.localIP());
  timeClient.begin();  // setup lấy datetime
}


String notiRequestFromUNO = "";
unsigned long timerDelay = 60000;  //delay time between each request 60s  120 000 =  2min
long lastTime = -(timerDelay);
long lastTimeCalibrateTimeClient = -1000;
long lastTimeAlarm = -30000;

void loop() {
  if (WiFi.status() == WL_CONNECTED) {

    ////IF 1
    if ((millis() - lastTimeCalibrateTimeClient) > 1000) {
      //1.SET AND STORE DATA ON ESP
      //1.1 TIME AND DATE
      timeClient.update();
      unix_epoch = timeClient.getEpochTime();  // Get Unix epoch time from the NTP server
      Serial.println("unix_epoch");
      Serial.println(unix_epoch);
      lastTimeCalibrateTimeClient = millis();
    }

    if ((millis() - lastTimeAlarm) > 30000) {
      ////////////// ALARM SETTING
      alarmSetting = sendHTTPS_Request("https://AAAAAAAAAAAAAAA.ondigitalocean.app/get_setting");
      // Find the position of the "date" key
      int startIndex = alarmSetting.indexOf("\"date\":\"") + 8;
      int endIndex = alarmSetting.indexOf("\"", startIndex);
      // Extract the date string
      dateStringAlarm = alarmSetting.substring(startIndex, endIndex);
      Serial.println(dateStringAlarm);

      lastTimeAlarm = millis();
    }

    ////IF 2
    if ((millis() - lastTime) > timerDelay) {
      // Check WiFi connection status

      // 1.2 WEATHER DATA
      String serverPath = "http://api.weatherapi.com/v1/current.json?key=AAAAAAAAAAAAAAA&q=ho-chi-minh-city&aqi=no";
      jsonBuffer = httpGETRequest(serverPath.c_str());
      Serial.println(jsonBuffer);
      JSONVar myObject = JSON.parse(jsonBuffer);
      // JSON.typeof(jsonVar) can be used to get the  type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      Serial.print("JSON object = ");
      Serial.println(myObject);
      // 1.2.1 PARSE DATA WEATHER GET VỀ
      String temp = JSON.stringify(myObject["current"]["temp_c"]);
      String humid = JSON.stringify(myObject["current"]["humidity"]);

      Serial.print("data ne: ");
      Serial.print(myObject);
      // Serial.println(temp);
      // Serial.print("Humidity: ");
      // Serial.println(humid);
      dataWeather = temp + ";" + humid;
      Serial.print("dataWeather: ");
      Serial.println(dataWeather);

      lastTime = millis();
    }

  } else {
    Serial.println("WiFi Disconnected");
  }
  //LISTEN TO UNO REQUEST
  if (espSerial.available()) {
    notiRequestFromUNO = espSerial.readString();
    // Serial.println("toi day ahihi");
    Serial.println(notiRequestFromUNO);
  }

  // Serial.println(notiRequestFromUNO);
  //RESPONE TO REQUEST FROM UNO
  if (notiRequestFromUNO == "req") {
    Serial.println("request_calibrate");
    DataToSend = (String)unix_epoch + ";" + dataWeather + ";" + dateStringAlarm;
    Serial.print("Data sent to Arduino is : ");
    Serial.println(DataToSend);
    espSerial.print(DataToSend);
    notiRequestFromUNO = "";
  }
}


/* ----------------------------------------------------------- */
/*                            lib function                     */
/* ----------------------------------------------------------- */

String sendHTTPS_Request(String url) {
  std::unique_ptr<BearSSL::WiFiClientSecure> client(new BearSSL::WiFiClientSecure);
  client->setInsecure();
  HTTPClient https;
  //Initializing an HTTPS communication using the secure client
  Serial.print("[HTTPS] begin...\n");
  if (https.begin(*client, url)) {  // HTTPS
    Serial.print("[HTTPS] GET...\n");
    // start connection and send HTTP header
    int httpCode = https.GET();
    // httpCode will be negative on error
    if (httpCode > 0) {
      // HTTP header has been send and Server response header has been handled
      Serial.printf("[HTTPS] GET... code: %d\n", httpCode);
      // file found at server
      if (httpCode == HTTP_CODE_OK || httpCode == HTTP_CODE_MOVED_PERMANENTLY) {
        String payload = https.getString();
        Serial.println(payload);
        return payload;
      }
    } else {
      Serial.printf("[HTTPS] GET... failed, error: %s\n", https.errorToString(httpCode).c_str());
    }
    https.end();
  } else {
    Serial.printf("[HTTPS] Unable to connect\n");
  }
  return "";
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;

  Serial.println("serverName");
  Serial.println(serverName);

  // Your IP address with path or Domain name with URL path
  http.begin(client, serverName);

  // Send HTTP POST request
  int httpResponseCode = http.GET();

  String payload = "{}";

  if (httpResponseCode > 0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  } else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();
  return payload;
}
