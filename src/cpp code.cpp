#include <Wire.h>//
#include <ThreeWire.h>
#include <LiquidCrystal_I2C.h>
#include <Adafruit_Sensor.h>
#include <TimeLib.h>
#include <RtcDS1302.h>
#include <EEPROM.h>
#include <Arduino.h>

#define PUMP_PIN 9
#define DHT11_PIN 11
#define LDR_PIN A0
#define BUTTON_MENU_PIN 2
#define BUTTON_UP_PIN 7
#define BUTTON_DOWN_PIN 8
#define BUTTON_SELECT_PIN 3
#define BUTTON_BACK_PIN 10
#define LED_PIN 12
#define moisture_pin A1
#define RST_PIN 6
#define DAT_PIN 5
#define CLK_PIN 4
#define MIN_INTERVAL 2000  // מינימום זמן בין קריאות (במילישניות)

LiquidCrystal_I2C lcd(0x27, 16, 2);

unsigned long lastReadTime = 0;

ThreeWire myWire(DAT_PIN, CLK_PIN, RST_PIN);
RtcDS1302<ThreeWire> rtc(myWire);

// משתנה גלובלי לסימון לחיצה על כפתור התפריט
volatile bool menuButtonPressed = false;
volatile bool selectButtonPressed = false;
volatile bool PeriodPumpWork = false;
volatile bool checkSensor = false;
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
byte smiley[8] = {
  0b00000,
  0b01010,
  0b01010,
  0b00000,
  0b10001,
  0b01110,
  0b00000,
  0b00000
};
byte halfMoon[8] = {
  B00111,
  B01110,
  B11100,
  B11000,
  B11000,
  B11100,
  B01110,
  B00111
};
byte sun[8] = {
  B00100,
  B01010,
  B00100,
  B11111,
  B11111,
  B00100,
  B01010,
  B00100
};
const float GAMMA = 0.7;
const float RL10 = 50;
boolean temp = true;
int moistureValue =0;
int minuteS=0;

int startHour = 7;
int startMinute = 0;
int endHour = 8;
int endMinute = 0;
int plantType = 0;
int setTemp = 34;
int setHum = 80;
int setMoisture=0;


void setup() {
  lcd.init();
  lcd.backlight();
  lcd.createChar(1,degree);
  lcd.createChar(2, smiley);
  lcd.createChar(3,halfMoon);
  lcd.createChar(4, sun);
  lcd.setCursor(4, 0);
  lcd.print("Welcome");
  lcd.setCursor(0, 1);
  lcd.print("To Smart Garden!"); 
  delay(2000);
  lcd.clear();

  Serial.begin(9600);

  rtc.Begin();
  if (!rtc.IsDateTimeValid()) {
    if (rtc.GetIsWriteProtected()) {
      rtc.SetIsWriteProtected(false);
    }
  }
   
  
  // הגדרת הפסיקה לכפתור התפריט
  attachInterrupt(digitalPinToInterrupt(BUTTON_MENU_PIN), menuButtonISR, RISING);
  attachInterrupt(digitalPinToInterrupt(BUTTON_SELECT_PIN), selectButtonISR, RISING);

  pinMode(PUMP_PIN, OUTPUT);
  pinMode(LDR_PIN, INPUT);
  pinMode(BUTTON_MENU_PIN, INPUT_PULLUP);
  pinMode(BUTTON_UP_PIN, INPUT);
  pinMode(BUTTON_DOWN_PIN, INPUT);
  pinMode(BUTTON_SELECT_PIN, INPUT);
  pinMode(BUTTON_BACK_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(moisture_pin, INPUT);
   pinMode(DHT11_PIN, INPUT_PULLUP);

  //EEPROM.write(4, 0);//איפוס זיכרון
  // בדיקה אם יש צורך לכתוב את הערכים ההתחלתיים ל-EEPROM
  if (EEPROM.read(4) != 1) {
    EEPROM.write(0, startHour);
    EEPROM.write(1, startMinute);
    EEPROM.write(2, endHour);
    EEPROM.write(3, endMinute);
    EEPROM.write(4, 1); // סימון שהערכים ההתחלתיים נכתבו
    EEPROM.write(5, setTemp);
    EEPROM.write(6, setHum);
    EEPROM.write(7, setMoisture);

     // הגדרת זמן השעון לזמן הקומפילציה הנוכחי
    RtcDateTime compiled = RtcDateTime(__DATE__, __TIME__);
    rtc.SetDateTime(compiled);
  }

   // קריאה של הערכים מ-EEPROM
  startHour = EEPROM.read(0);
  startMinute = EEPROM.read(1);
  endHour = EEPROM.read(2);
  endMinute = EEPROM.read(3);
  setTemp = EEPROM.read(5);
  setHum = EEPROM.read(6);
  setMoisture=EEPROM.read(7);

}

void loop() {
  displaySensors();
}

// פונקציית הטיפול בפסיקה
void menuButtonISR() {
  menuButtonPressed = true;
}
void selectButtonISR() {
  selectButtonPressed = true;
}

void CheckMenu(int hour, int minute) {
  if (menuButtonPressed) {
    menuButtonPressed = false;
    showMenu(hour,minute);
  }
  return;
}

void displaySensors() {

  lcd.clear();
  RtcDateTime now = rtc.GetDateTime();
  
  int hour = now.Hour();
  int minute = now.Minute();
  
 float temperature, humidity;
  unsigned long currentTime = millis();
  if (currentTime - lastReadTime >= MIN_INTERVAL) {
    if (dht11_read(&temperature, &humidity)) {
       printHT(humidity,temperature);
    } else {
     lcd.setCursor(0, 0);
     lcd.print("Failed to read from");
     lcd.setCursor(0, 1);
     lcd.print("DHT11 sensor!");
    }
    lastReadTime = currentTime;
  }

  lcd.clear();
  CheckMenu(hour,minute);

  float lux=getLux();
   printLux(lux);

   lcd.clear();
   CheckMenu(hour,minute);

    moistureValue = analogRead(moisture_pin);
    printMoisture(moistureValue);

   printSerialMonitor( temperature, humidity, lux );

    lcd.clear();
  CheckMenu(hour,minute);
   for(int i=1; i<=6;i++){
   printClock();
   CheckMenu(hour,minute);
   delay(990);
   }
  lcd.clear();
 CheckMenu(hour,minute);
 /////////////////////////// דרישות שהמשאבה תעבוד

 if ((((hour > startHour || (hour == startHour && minute > startMinute)) && 
     (hour < endHour || (hour == endHour && minute < endMinute))) || 
    (startHour > endHour && 
     ((hour > startHour || (hour == startHour && minute > startMinute)) ||
      (hour < endHour || (hour == endHour && minute < endMinute))))) &&
      moistureValue <= 500) {
  
  digitalWrite(PUMP_PIN, HIGH);
  digitalWrite(LED_PIN, HIGH);
  lcd.setCursor(0, 0);
  lcd.print("The pump is");
  lcd.setCursor(0, 1);
  lcd.print("working   ");
  lcd.write(2);
  delay(2000);
  PeriodPumpWork = true;
  lcd.clear();
  }
 else {
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(LED_PIN, LOW);
  PeriodPumpWork = false;
 }

   lcd.clear();
   CheckMenu(hour,minute);

   //הפעלת משאבה באמצעות חיישנים
  if(minuteS == minute){
    checkSensor=false;
  }

  if ((humidity >= setHum || temperature >= setTemp || moistureValue <= setMoisture) && !PeriodPumpWork  && !checkSensor) {
    digitalWrite(PUMP_PIN, HIGH);
    digitalWrite(LED_PIN, HIGH);
    checkSensor=true;
    RtcDateTime getsensor = rtc.GetDateTime();
      minuteS = getsensor.Minute()-1;
      lcd.setCursor(0, 0);
    lcd.print("The pump is");
    lcd.setCursor(0, 1);
    lcd.print("working   ");
    lcd.write(2);
    delay(10000); // המשאבה פועלת למשך 10 שניות
    digitalWrite(PUMP_PIN, LOW);
    digitalWrite(LED_PIN, LOW);
     lcd.clear();
  }

}

void printSerialMonitor(float temperature,float humidity,float lux ){
  Serial.print("Temperature: ");
  Serial.print(temperature);
  Serial.print(" C, Humidity: ");
  Serial.print(humidity);
  Serial.print(" %, Soil Moisture: ");
  Serial.print(moistureValue);
  Serial.print("  , Light Intensity: ");
  Serial.print(lux);
  Serial.println(" lux");
  Serial.print("Max Temp:");
  Serial.print(setTemp);
  Serial.print("        Max Hum:");
  Serial.print(setHum);
  Serial.print("       Max moisture:");
  Serial.println(setMoisture);
  Serial.print("startHour:");
  Serial.print(startHour);
  Serial.print(" endHour:");
  Serial.println(endHour);
  Serial.print(" minute End:");
  Serial.println(minuteS);
  Serial.println("         ");
}
void printHT(float h,float t){
  lcd.setCursor(0, 0);
  lcd.print("Humidity  Temp");
  lcd.setCursor(0, 1);
  lcd.print(h);
  lcd.print("%   ");
  lcd.print(t);
  lcd.write(1);
  lcd.print("C");
  delay(2000);
 
}

void printMoisture(int moisture){
   if (moisture==-1) {
    lcd.setCursor(0, 0);
    lcd.print("Failed to read from");
     lcd.setCursor(0, 1);
     lcd.print("Moist sensor!");
  }
 else{
  if(moisture <= setMoisture){
  lcd.setCursor(0, 0);
  lcd.print(" Soil moisture: ");
  lcd.setCursor(0, 1);
  lcd.print("Ground dry(");
      if (moisture < 10) {
    lcd.print("00");
   } 
    else if (moisture < 100) {
    lcd.print("0");
     } 
  lcd.print(moisture);
   lcd.print(")");
  delay(2000);
  }
  else{
  lcd.setCursor(0, 0);
  lcd.print(" Soil moisture: ");
  lcd.setCursor(0, 1);
  lcd.print("Ground OK(");
        if (moisture < 10) {
    lcd.print("00");
   } 
    else if (moisture < 100) {
    lcd.print("0");
     } 
  lcd.print(moisture);
   lcd.print(")");
  delay(2000);
  }
 }
}

void printLux(float lux){
  lcd.setCursor(0, 0);
   if (lux == -1) {
    lcd.print("Failed to read from");
     lcd.setCursor(0, 1);
     lcd.print("lux sensor!");
  }
 else{
  if (lux > 50) {
    lcd.print("Day time!  ");
    lcd.write(4);
  } else {
    lcd.print("Night time! ");
    lcd.write(3);
  }
  lcd.setCursor(0, 1);
  lcd.print("Lux: ");
  lcd.print(lux);
  delay(2000);
  }
}

float getLux(){
 int analogValue = analogRead(LDR_PIN);
  float voltage = analogValue / 1024.0 * 5;
  float resistance = 2000 * voltage / (1 - voltage / 5);
  float lux = pow(RL10 * 1e3 * pow(10, GAMMA) / resistance, (1 / GAMMA));
  return lux;
}

void printClock(){
  RtcDateTime now = rtc.GetDateTime();
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Time: ");
  lcd.print(now.Hour() < 10 ? "0" : ""); lcd.print(now.Hour());
  lcd.print(":");
  lcd.print(now.Minute() < 10 ? "0" : ""); lcd.print(now.Minute());
  lcd.print(":");
  lcd.print(now.Second() < 10 ? "0" : ""); lcd.print(now.Second());
  lcd.setCursor(0, 1);
  lcd.print("Date: ");
  lcd.print(now.Day() < 10 ? "0" : ""); lcd.print(now.Day());
  lcd.print("/");
  lcd.print(now.Month() < 10 ? "0" : ""); lcd.print(now.Month());
  lcd.print("/");
  lcd.print(now.Year());
}

void showMenu(int hour, int minute) {
  int menuOption = 0;
   lcd.clear();
  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Menu:");
    lcd.setCursor(0, 1);
    if (menuOption == 0) lcd.print("> Set Watering");
    else if (menuOption == 1) lcd.print("> Set Clock    ");
    else if (menuOption == 2) lcd.print("> Set Plant     ");
    else if (menuOption == 3) lcd.print("> Set sensors    ");
    else lcd.print("               ");
   
    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
      menuOption = (menuOption + 3) % 4;
      delay(50);
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
      menuOption = (menuOption + 1) % 4;
      delay(50);
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      lcd.clear();
      return;
    }
    delay(50);

    if (selectButtonPressed) {
      selectButtonPressed = false;
       delay(300);
      if (menuOption == 0) setWateringTime();
      else if (menuOption == 1) setClock(hour,minute);
      else if (menuOption == 2) setPlant();
      else if (menuOption == 3) SettingUpSensors();   
      lcd.clear(); 
        return;
    }

    delay(100);
  }
}

void setWateringTime() {
  lcd.clear();
  boolean settingStart = true;
  while (true) {
    lcd.setCursor(0, 0);
    if (settingStart) {
      lcd.print("Set Start Time:");
      lcd.setCursor(0, 1);
      lcd.print(startHour < 10 ? "0" : ""); lcd.print(startHour);
      lcd.print(":");
      lcd.print(startMinute < 10 ? "0" : ""); lcd.print(startMinute);
      delay(300);
      lcd.setCursor(0, 1);
      lcd.print("  ");
      lcd.print(":");
      lcd.print(startMinute < 10 ? "0" : ""); lcd.print(startMinute);
    } else {
      lcd.print("Set End Time:");
      lcd.setCursor(0, 1);
      lcd.print(endHour < 10 ? "0" : ""); lcd.print(endHour);
      lcd.print(":");
      lcd.print(endMinute < 10 ? "0" : ""); lcd.print(endMinute);
     delay(300);
    lcd.setCursor(0, 1);
       lcd.print("  ");
      lcd.print(":");
      lcd.print(endMinute < 10 ? "0" : ""); lcd.print(endMinute);
    }

    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
      if (settingStart) {
        startHour++;
        if (startHour > 23) startHour = 0;
      } else {
        endHour++;
        if (endHour > 23) endHour = 0;
      }
      delay(50);
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
      if (settingStart) {
        startHour--;
        if (startHour < 0) startHour = 23;
      } else {
        endHour--;
        if (endHour < 0) endHour = 23;
      }
      delay(50);
    }

    delay(200);

    if (selectButtonPressed) {
      delay(400);
      selectButtonPressed = false;
      if (settingStart) {
        settingStart = false;
        lcd.clear();
      } else {
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("start pump:");
      lcd.print(startHour);
      lcd.print("[H]");
      lcd.setCursor(0, 1);
      lcd.print("end pump:  ");
      lcd.print(endHour);
      lcd.print("[H]");
      delay(2000);
      lcd.clear();
        // כתיבת הערכים ל-EEPROM
        EEPROM.write(0, startHour);
        EEPROM.write(1, startMinute);
        EEPROM.write(2, endHour);
        EEPROM.write(3, endMinute);
        return;
      }
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      return;
    }
  }
}

void setClock(int hourNow,int minuteNow) {
  lcd.clear();
  int hour = hourNow;
  int minute = minuteNow;
  boolean settingHour = true;

  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Set Time:");
    lcd.setCursor(0, 1);
    if(settingHour){
    lcd.print(hour < 10 ? "0" : ""); lcd.print(hour);
    lcd.print(":");
    lcd.print(minute < 10 ? "0" : ""); lcd.print(minute);
    delay(200);
    lcd.setCursor(0, 1);
    lcd.print("  ");
    lcd.print(":");
    lcd.print(minute < 10 ? "0" : ""); lcd.print(minute);
    }
    else{
    lcd.print(hour < 10 ? "0" : ""); lcd.print(hour);
    lcd.print(":");
    lcd.print(minute < 10 ? "0" : ""); lcd.print(minute);
    delay(200);
    lcd.setCursor(0, 1);
    lcd.print(hour < 10 ? "0" : ""); lcd.print(hour);
    lcd.print(":");
    lcd.print("  ");
    }

    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
      if (settingHour) {
        hour++;
        if (hour > 23) hour = 0;
      } else {
        minute++;
        if (minute > 59) minute = 0;
      }
      
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
      if (settingHour) {
        hour--;
        if (hour < 0) hour = 23;
      } else {
        minute--;
        if (minute < 0) minute = 59;
      }
      
    }

    delay(200);

    if (selectButtonPressed) {
      selectButtonPressed = false;
      if (settingHour) {
        settingHour = false;
      } else {
        lcd.clear();
        setDate(hour, minute);
        return;
      }
      delay(500);
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      return;
    }
  }
}

void setDate(int hour, int minute) {
  lcd.clear();
  int month = 1;
  int day = 1;
  boolean settingDay = true;

  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Set Date:");
    lcd.setCursor(0, 1);
    if(settingDay){
    lcd.print(day < 10 ? "0" : ""); lcd.print(day);
    lcd.print("/");
    lcd.print(month < 10 ? "0" : ""); lcd.print(month);
    delay(200);
    lcd.setCursor(0, 1);
     lcd.print("  ");
    lcd.print("/");
    lcd.print(month < 10 ? "0" : ""); lcd.print(month);
    }
    else{
    lcd.print(day < 10 ? "0" : ""); lcd.print(day);
    lcd.print("/");
    lcd.print(month < 10 ? "0" : ""); lcd.print(month);
    delay(200);
    lcd.setCursor(0, 1);
    lcd.print(day < 10 ? "0" : ""); lcd.print(day);
    lcd.print("/");
    lcd.print("  ");
    }
    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
      if (settingDay) {
        day++;
        if (day > 31) day = 1;
      } else {
        month++;
        if (month > 12) month = 1;
      }
    
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
      if (settingDay) {
        day--;
        if (day < 1) day = 31;
      } else {
        month--;
        if (month < 1) month = 12;
      }
     
    }

    delay(200);

    if (selectButtonPressed) {
      selectButtonPressed = false;
      if (settingDay) {
        settingDay = false;
      } else {
        RtcDateTime newTime = RtcDateTime(2024, month, day, hour, minute, 0);
        rtc.SetDateTime(newTime);
        return;
      }
      delay(400);
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      delay(200);
      return;
    }
  }
}

void SettingUpSensors(){
  int sensorOption = 0;
  lcd.clear();
  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Set sensor:");
    lcd.setCursor(0, 1);
    if (sensorOption == 0) lcd.print("> Set Temperature");
    else if (sensorOption == 1) lcd.print("> Set Humidity     ");
    else if (sensorOption == 2) lcd.print("> Set moisture     ");
    else lcd.print("               ");

    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
      sensorOption = (sensorOption + 2) % 3;
      delay(50);
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
      sensorOption = (sensorOption + 1) % 3;
      delay(50);
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      delay(100);
      return;
    }
    delay(200);

    if (selectButtonPressed) {
      delay(300);   
      selectButtonPressed = false;
      if (sensorOption == 0) setTemperature();
      else if (sensorOption == 1) setHumidity();
      else if (sensorOption == 2) setMois();    
      delay(400);   
      lcd.clear();  
    }

  }

}

void setMois(){
 lcd.clear();
  while (true) {
    lcd.setCursor(0, 0);
      lcd.print("Set moisture:");
      lcd.setCursor(0, 1);
      if (setMoisture < 10) {
    lcd.print("00");
   } 
    else if (setMoisture < 100) {
    lcd.print("0");
     } 
      lcd.print(setMoisture);
      
    
    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
        setMoisture=setMoisture+2;
        if (setMoisture > 650) setMoisture = 0;
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
        setMoisture=setMoisture-2;
        if (setMoisture < 0) setMoisture = 650;
    }

    delay(300);

    if (selectButtonPressed) {
      selectButtonPressed = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("maximum ");
      lcd.setCursor(0, 1);
      lcd.print("Moisture: ");
      lcd.print(setMoisture);
      delay(2000);
        // כתיבת הערכים ל-EEPROM
        EEPROM.write(7, setMoisture);
        return;
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      delay(200);
      return;
    }
  }
}

void setHumidity(){
 lcd.clear();
  while (true) {
    lcd.setCursor(0, 0);
      lcd.print("Set Humidity:");
      lcd.setCursor(0, 1);
      lcd.print(setHum < 10 ? "0" : ""); lcd.print(setHum);
      lcd.print("%");
      
    
    
    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
        setHum++;
        if (setHum > 90) setHum = 30;
      
      delay(50);
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
        setHum--;
        if (setHum < 30) setHum = 90;
      delay(50);
    }

    delay(200);

    if (selectButtonPressed) {
     selectButtonPressed = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("maximum");
       lcd.setCursor(0, 1);
      lcd.print("Humidity: ");
      lcd.print(setHum);
      lcd.print("%");
      delay(2000);
        // כתיבת הערכים ל-EEPROM
        EEPROM.write(6, setHum);
      return;
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      delay(200);
      return;
    }
  }
}

void setTemperature(){
 lcd.clear();
  while (true) {
    lcd.setCursor(0, 0);
      lcd.print("Set Temperature:");
      lcd.setCursor(0, 1);
      lcd.print(setTemp < 10 ? "0" : ""); lcd.print(setTemp);
      lcd.write(1);
      lcd.print("C");
      
    
    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
        setTemp++;
        if (setTemp > 45) setTemp = 15;
      
      delay(50);
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
        setTemp--;
        if (setTemp < 15) setTemp = 45;
      delay(50);
    }

    delay(200);

    if (selectButtonPressed) {
      selectButtonPressed = false;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("maximum");
      lcd.setCursor(0, 1);
      lcd.print("temperature:");
      lcd.print(setTemp);
      lcd.write(1);
      lcd.print("C");
      delay(2000);
        // כתיבת הערכים ל-EEPROM
        EEPROM.write(5, setTemp);
        return;
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      delay(200);
       return;
    }
  }
}

void setPlant() {
  lcd.clear();
  int plantOption = 0;
  delay(200);
  while (true) {
    lcd.setCursor(0, 0);
    lcd.print("Select Plant:");
    lcd.setCursor(0, 1);
    if (plantOption == 0) lcd.print("> Tomato");
    else if (plantOption == 1) lcd.print("> Cucumber");
    else if (plantOption == 2) lcd.print("> Lettuce");
    else lcd.print("             ");

    if (digitalRead(BUTTON_UP_PIN) == HIGH) {
      plantOption = (plantOption + 2) % 3;
      delay(200);
    }

    if (digitalRead(BUTTON_DOWN_PIN) == HIGH) {
      plantOption = (plantOption + 1) % 3;
      delay(200);
    }

     delay(300);
    if (selectButtonPressed) {
      selectButtonPressed = false;
      lcd.clear();
      if (plantOption == 0) {
        startHour = 10;
        startMinute = 0;
        endHour = 11;
        endMinute = 0;
      } else if (plantOption == 1) {
        startHour = 8;
        startMinute = 0;
        endHour = 9;
        endMinute = 0;
      } else if (plantOption == 2) {
        startHour = 12;
        startMinute = 0;
        endHour = 13;
        endMinute = 0;
      }
      plantType = plantOption + 1;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("start pump:");
      lcd.print(startHour);
      lcd.print("[H]");
      lcd.setCursor(0, 1);
      lcd.print("end pump:  ");
      lcd.print(endHour);
      lcd.print("[H]");
      delay(2000);
      lcd.clear();
        // כתיבת הערכים ל-EEPROM
        EEPROM.write(0, startHour);
        EEPROM.write(1, startMinute);
        EEPROM.write(2, endHour);
        EEPROM.write(3, endMinute);
      return;
    }

    if (digitalRead(BUTTON_BACK_PIN) == HIGH) {
      lcd.clear();
      return;
    }

    delay(100);
  }
}

bool dht11_read(float* temperature, float* humidity) {
  uint8_t data[5] = {0, 0, 0, 0, 0};
  uint8_t counter = 0, j;

  // אתחול תקשורת
  pinMode(DHT11_PIN, OUTPUT);
  digitalWrite(DHT11_PIN, LOW);
  delay(18);
  digitalWrite(DHT11_PIN, HIGH);
  delayMicroseconds(40);
  pinMode(DHT11_PIN, INPUT_PULLUP);

  // בדיקת תגובת החיישן
  if (digitalRead(DHT11_PIN) == HIGH) {
    Serial.println("DHT11 not responding to pull down");
    return false;
  }
  delayMicroseconds(80);
  if (digitalRead(DHT11_PIN) == LOW) {
    Serial.println("DHT11 not responding to pull up");
    return false;
  }
  delayMicroseconds(80);

  // קריאת נתונים
  for (j = 0; j < 5; j++) {
    uint8_t result = 0;
    for (int i = 0; i < 8; i++) {
      while (digitalRead(DHT11_PIN) == LOW) {
        if (++counter > 100) {
          Serial.println("Timeout waiting for bit start");
          return false;
        }
      }
      delayMicroseconds(30);
      if (digitalRead(DHT11_PIN) == HIGH)
        result |= (1 << (7 - i));
      counter = 0;
      while (digitalRead(DHT11_PIN) == HIGH) {
        if (++counter > 100) {
          Serial.println("Timeout waiting for bit end");
          return false;
        }
      }
    }
    data[j] = result;
  }

  // בדיקת checksum
  if (data[4] != ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
    Serial.println("Checksum error");
    return false;
  }

  // חישוב טמפרטורה ולחות
  *humidity = data[0] + data[1] * 0.1;
  *temperature = data[2] + data[3] * 0.1;

  return true;
}

