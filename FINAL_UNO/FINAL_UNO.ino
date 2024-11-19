#include <RTClib.h>
//#include <DS1307RTC.h>
#include <TimeLib.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>                // Thư viện cho cổng nối tiếp mềm
#define RX_PIN 11                          // Chân RX của cổng nối tiếp mềm
#define TX_PIN 12                          // Chân TX của cổng nối tiếp mềm
#define BUZZER_PIN 8                       // Chân kết nối với led
const int buttonPin = 7;                   // Chân nối nút nhấn đổi lcd
const int offBuzzerButtonPin = 4;          // Chân nút tắt buzzer
int temperatureAnalogInPin = A0;           // Chân analog để đọc nhiệt độ
LiquidCrystal_I2C lcd(0x27, 16, 2);        // Địa chỉ I2C và kích thước LCD (16x2)
SoftwareSerial unoSerial(RX_PIN, TX_PIN);  // Tạo một đối tượng cổng nối tiếp mềm
int buttonPushCounter = 0;                 // Biến đếm số lần nhấn nút
int buttonState = 0;                       // Trạng thái hiện tại của nút nhấn
int lastButtonState = 0;                   // Trạng thái trước đó của nút nhấn
String getValue(String data, char separator, int index);
unsigned long convertStringToLong(String str);
int alarmMinute = 0;
tmElements_t tm;
DateTime nowDateTime;
bool offBuzzerButtonState = false;
bool enableBuzz = false;
bool flag = 0;
unsigned long valueDateTimeEpoch;
String valueTime;
String valueHCMTemp;
String valueHumid;
String data;
int valueSetting_year;
int valueSetting_month;
int valueSetting_day;
int valueSetting_hour;
int valueSetting_min;
int valueSetting_sec;

// OTHER
// Ký hiệu độ cho LCD
byte degree[8] = {
  0b00011,
  0b00011,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000
};

RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {
  "Sun",
  "Mon",
  "Tue",
  "Wed",
  "Thu",
  "Fri",
  "Sat"
};

void Button_Push(void) {
  // Đọc trạng thái nút nhấn
  buttonState = digitalRead(buttonPin);
  // Serial.println("buttonbuttonState");
  // Serial.println(buttonState);
  if (buttonState == HIGH) {
    delay(20);
    buttonState = digitalRead(buttonPin);
    if (buttonState == HIGH) {
      // Tăng biến đếm số lần nhấn
      buttonPushCounter++;
      Serial.println("buttonPushCounter");
      Serial.println(buttonPushCounter);
      if (buttonPushCounter > 2) {
        buttonPushCounter = 0;
      }
      do {
        Serial.println("treoooo");
      } while (digitalRead(buttonPin));
    }
  }
}

void setup() {
  Serial.begin(9600);
  unoSerial.begin(9600);  // Khởi tạo cổng nối tiếp mềm với tốc độ baud là 9600

  if (!rtc.begin()) {
    Serial.println("Couldn't find RTC");
  }
  // Serial.println("1730976837 diiiiiii");
  // Serial.println(convertStringToLong("1730976837"));
  // sets the RTC to the date & time on PC this sketch was compiled
  rtc.adjust(DateTime(__DATE__, __TIME__));
  // Serial.println("DATE");
  // Serial.println(__DATE__);
  // Serial.println("Time");
  // Serial.println(__TIME__);
  // sets the RTC with an explicit date & time, for example to set
  // January 21, 2021 at 3am you would call:
  // Serial.println("adjust to 1730976837");
  // rtc.adjust(DateTime(1 730 976 837));1 731 018 365  | 1 731 018 365

  pinMode(buttonPin, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  lcd.init();                 // Khởi tạo LCD I2C
  lcd.backlight();            // Bật đèn nền LCD
  lcd.createChar(1, degree);  // Tạo ký tự độ (°)
  // attachInterrupt(digitalPinToInterrupt(buttonPin), Button_Push, RISING);
  // Hiển thị lời chào ban đầu
  lcd.setCursor(5, 0);
  lcd.print("GROUP4");
  lcd.setCursor(4, 1);
  lcd.print("Xin chao");
  delay(3000);
  lcd.clear();
}


long timerDelayRequestCalibrate = 5000;                  //delay time between each request 60s
long lastTimeCalibrate = -(timerDelayRequestCalibrate);  // để ngay lập tức gọi 1 lần khi vừa start chương trình
long lastTimeCalibrateRTC = -1000;
void loop() {
  Button_Push();
  // printTime(nowDateTime);
  if ((millis() - lastTimeCalibrateRTC) > 1000) {
    nowDateTime = rtc.now();
    lastTimeCalibrateRTC = millis();
  }

  //REQUEST CALIBRATE
  if ((millis() - lastTimeCalibrate) > timerDelayRequestCalibrate) {
    //REQUEST TO ESP
    Serial.println("request_calibrate now");
    unoSerial.print("req");
    delay(30);
    // tone(BUZZER_PIN, 1000, 500);  // send request buzz
    lastTimeCalibrate = millis();
  }

  //LISTEN TO REQUEST
  if (unoSerial.available()) {      // Nếu có dữ liệu từ cổng nối tiếp mềm
    data = unoSerial.readString();  // Đọc chuỗi dữ liệu
    Serial.println("data ne");
    Serial.println(data);

    String valueDateTimeEpochString = getValue(data, ';', 0);         // In ra chuỗi dữ liệu
    long tmpCompare = convertStringToLong(valueDateTimeEpochString);  // Tác,h phần tử thứ 0 theo ký tự phân cách ';'

    // Serial.println("valueDateTimeEpoch");
    // Serial.println(valueDateTimeEpoch);
    // Serial.println("tmpCompare");
    // Serial.println(tmpCompare);
    if (valueDateTimeEpoch < tmpCompare && tmpCompare > 10) {
      valueDateTimeEpoch = tmpCompare;
    }
    valueHCMTemp = getValue(data, ';', 1);  // Tách phần tử thứ 1 theo ký tự phân cách ';'
    valueHumid = getValue(data, ';', 2);    // Tách phần tử thứ 1 theo ký tự phân cách ';'

    valueSetting_year = getValue(data, ';', 3).toInt();
    valueSetting_month = getValue(data, ';', 4).toInt();
    valueSetting_day = getValue(data, ';', 5).toInt();
    valueSetting_hour = getValue(data, ';', 6).toInt();
    valueSetting_min = getValue(data, ';', 7).toInt();
    valueSetting_sec = getValue(data, ';', 8).toInt();

    // Serial.println("adjust epoch:");
    // Serial.println(valueDateTimeEpoch);
    rtc.adjust(DateTime(valueDateTimeEpoch));
    nowDateTime = rtc.now();
    Serial.println("HCM Temperature:" + valueHCMTemp);  // In ra phần tử thứ 1
    Serial.println("Humidity:" + valueHumid);           // In ra phần tử thứ 1
  }

  // Cập nhật nội dung hiển thị theo số lần nhấn nút
  if (buttonPushCounter == 0) {
    showDate(nowDateTime);
  } else if (buttonPushCounter == 1) {
    showHumidTemp(valueHCMTemp, valueHumid);
  } else if (buttonPushCounter == 2) {
    showTemperature();
  }


  // Serial.println(flag);
  if (nowDateTime.hour() != valueSetting_hour && nowDateTime.minute() != valueSetting_min) {
    // Serial.println("nowDateTime.hour()");
    // Serial.println(nowDateTime.hour());
    flag = 1;
    // Serial.println(flag);
    // Serial.println(nowDateTime.hour());
  }

  // && nowDateTime.minute() = valueSetting_min && valueSetting_hour != -1 && valueSetting_min != -1
  if (nowDateTime.hour() == valueSetting_hour && nowDateTime.minute() == valueSetting_min && flag == 1) {
    Serial.println("toi alarm ");
    Serial.println(flag);
    flag = 0;
    enableBuzz = true;
    // valueSetting_hour = -1;
    // valueSetting_min = -1;
  }

  // Serial.println(enableBuzz);

  if (enableBuzz && (nowDateTime.minute() == valueSetting_min)) {
    tone(BUZZER_PIN, 1000, 500);
  } else {
    noTone(BUZZER_PIN);
  }

  // Điều khiển bật/tắt buzzer bằng công tắc
  offBuzzerButtonState = digitalRead(offBuzzerButtonPin);

  if (offBuzzerButtonState == HIGH) {
    delay(70);
    offBuzzerButtonState = digitalRead(offBuzzerButtonPin);
    if (offBuzzerButtonState == HIGH) {
      Serial.println("da bi high");
      enableBuzz = false;
    }
  }
}


unsigned long convertStringToLong(String str) {
  long tong = 0;
  long so1 = str.substring(0, 4).toInt();
  // Serial.println("so 1");
  // Serial.println(so1);
  long so2 = str.substring(4, 8).toInt();
  // Serial.println("so 2");
  // Serial.println(so2);
  long so3 = str.substring(8, 10).toInt();
  // Serial.println("so 3");
  // Serial.println(so3);
  return so1 * 1000000 + so2 * 100 + so3;
}

long lastTimeLCDRefresh = 0;
int delayTimeLCDRefresh = 700;

// lastTimeLCDRefresh = 0;
// if ((millis() - lastTimeLCDRefresh) > delayTimeLCDRefresh) {

//   lastTimeLCDRefresh = millis();
// }

// lib function
void showHumidTemp(String temp, String humid) {
  // lastTimeLCDRefresh = 0;
  if ((millis() - lastTimeLCDRefresh) > delayTimeLCDRefresh) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("HCM TEMP: ");
    lcd.print(temp);
    lcd.write(223);  // Hiển thị ký tự độ C
    lcd.print("C");
    lcd.setCursor(0, 1);
    lcd.print("HUMIDITY: ");
    lcd.print(humid);
    lcd.print("%");
    lastTimeLCDRefresh = millis();
  }
}

int oldCurTemp = 0;
void showTemperature() {
  // lastTimeLCDRefresh = 0;
  if ((millis() - lastTimeLCDRefresh) > delayTimeLCDRefresh) {
    int tempCurrent = getCurrentTemperature(temperatureAnalogInPin);
    if (oldCurTemp != tempCurrent) {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("CURRENT TEMPERATURE");
      lcd.setCursor(0, 1);
      lcd.print(tempCurrent);
      lcd.write(223);  // Hiển thị ký tự độ C
      lcd.print("C");
      oldCurTemp = tempCurrent;
    }
    lastTimeLCDRefresh = millis();
  }
}

// Hàm tách chuỗi dữ liệu theo ký tự phân cách và trả về phần tử thứ index
String getValue(String data, char separator, int index) {
  int found = 0;
  int strIndex[] = { 0, -1 };
  int maxIndex = data.length() - 1;

  for (int i = 0; i <= maxIndex && found <= index; i++) {
    if (data.charAt(i) == separator || i == maxIndex) {
      found++;
      strIndex[0] = strIndex[1] + 1;
      strIndex[1] = (i == maxIndex) ? i + 1 : i;
    }
  }
  return found > index ? data.substring(strIndex[0], strIndex[1]) : "";
}

// Hàm đọc nhiệt độ từ cảm biến
int getCurrentTemperature(int pinInput) {
  int val = analogRead(pinInput);
  float mv = (val / 1023.0) * 5000;  // Chuyển đổi giá trị ADC sang mV
  float cel = mv / 10;               // Chuyển đổi mV sang độ C
  return (int)cel;                   // Trả về nhiệt độ dưới dạng số nguyên
}

char buffer[2];  // Buffer to hold the formatted string
void showDate(DateTime time) {
  // lastTimeLCDRefresh = 0;
  if ((millis() - lastTimeLCDRefresh) > delayTimeLCDRefresh) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("DATE: ");
    lcd.print(time.year());
    lcd.print('/');
    lcd.print(time.month());
    lcd.print('/');
    lcd.print(time.day());
    lcd.setCursor(0, 1);
    lcd.print(daysOfTheWeek[time.dayOfTheWeek()]);
    lcd.print("    ");

    // lcd.print((String)time.hour());
    // if (time.hour() < 10) {
    //   lcd.println("0" + (String)time.hour());
    // } else {
    //   lcd.print((String)time.hour());
    // }

    sprintf(buffer, "%02d", time.hour());
    Serial.println(buffer);
    lcd.printstr(buffer);

    sprintf(buffer, "%02d", time.minute());
    Serial.println(buffer);
    lcd.print(':');
    lcd.printstr(buffer);


    sprintf(buffer, "%02d", time.second());
    Serial.println(buffer);
    lcd.print(':');
    lcd.printstr(buffer);

    // lcd.print(':');
    // if (time.second() < 10) {
    //   lcd.println("0" + (String)time.second());
    // } else {
    //   lcd.println((String)time.second());
    // }

    lastTimeLCDRefresh = millis();
    printTime(time);
  }
}


void printTime(DateTime time) {
  Serial.print("TIME: ");
  Serial.print(time.year(), DEC);
  Serial.print('/');
  Serial.print(time.month(), DEC);
  Serial.print('/');
  Serial.print(time.day(), DEC);
  Serial.print(" (");
  Serial.print(daysOfTheWeek[time.dayOfTheWeek()]);
  Serial.print(") ");
  Serial.print(time.hour(), DEC);
  Serial.print(':');
  Serial.print(time.minute(), DEC);
  Serial.print(':');
  Serial.println(time.second(), DEC);
}
