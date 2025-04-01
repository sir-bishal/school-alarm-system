/*
Project: School Bell System
Developer: Bishal Paul
Project Started: 5th March 2025
Completed: 9th March 2025
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RTClib.h>

LiquidCrystal_I2C lcd(0x27, 16, 2);
RTC_DS3231 rtc;

#define RELAY_PIN 23
#define BTN_MODE 32
#define BTN_UP 33
#define BTN_DOWN 25
#define BTN_SET 26
#define BTN_MANUAL 27
#define BUZZER_PIN 4

const char* bellLabels[] = {"Prayer Bell", "1st Period", "2nd Period", "3rd Period", "4th Period", "Tiffin Bell", "6th Period", "7th Period", "8th Period", "9th Period", "End Bell"};
int bellTimes[][2] = {{10, 45}, {10, 55}, {11, 35}, {12, 15}, {12, 55}, {13, 35}, {14, 15}, {14, 50}, {15, 25}, {16, 00}, {16, 25}};
const int bellDurations[] = {15, 10, 10, 10, 10, 10, 10, 10, 10, 10, 15};

bool modifying = false;
bool settingTime = false;
bool settingSystemTime = false;
int selectedBell = 0;
int setHour, setMinute;
unsigned long lastPressTime = 0;

void setup() {
    Serial.begin(115200);
    Wire.begin();
    lcd.init();
    lcd.backlight();
    pinMode(RELAY_PIN, OUTPUT);
    digitalWrite(RELAY_PIN, HIGH);
    pinMode(BTN_MODE, INPUT_PULLUP);
    pinMode(BTN_UP, INPUT_PULLUP);
    pinMode(BTN_DOWN, INPUT_PULLUP);
    pinMode(BTN_SET, INPUT_PULLUP);
    pinMode(BTN_MANUAL, INPUT_PULLUP);
    pinMode(BUZZER_PIN, OUTPUT);
    digitalWrite(BUZZER_PIN, LOW);
    if (!rtc.begin()) {
        while (1);
    }
}

void loop() {
    DateTime now = rtc.now();
    static int lastMinute = -1;
    if (now.minute() != lastMinute || modifying || settingTime || settingSystemTime) {
        displayTime(now);
        lastMinute = now.minute();
    }
    if (!modifying && !settingTime && !settingSystemTime) {
        checkBell(now);
    }
    checkButtons();
    delay(100);
}

void displayTime(DateTime now) {
    lcd.setCursor(0, 0);
    if (modifying) {
        lcd.print(bellLabels[selectedBell]);
        lcd.setCursor(0, 1);
        lcd.printf("%02d:%02d  ", bellTimes[selectedBell][0], bellTimes[selectedBell][1]);
    } else if (settingSystemTime) {
        lcd.print("Set System Time  ");
        lcd.setCursor(0, 1);
        lcd.printf("%02d:%02d  ", setHour, setMinute);
    } else {
        lcd.print("System Time    ");
        lcd.setCursor(0, 1);
        lcd.printf("%02d:%02d  ", now.hour(), now.minute());
    }
}

void checkBell(DateTime now) {
    for (int i = 0; i < sizeof(bellTimes) / sizeof(bellTimes[0]); i++) {
        if (now.hour() == bellTimes[i][0] && now.minute() == bellTimes[i][1] && now.second() == 0) {
            activateBell(bellDurations[i]);
        }
    }
}

void activateBell(int duration) {
    digitalWrite(RELAY_PIN, LOW);
    delay(duration * 1000);
    digitalWrite(RELAY_PIN, HIGH);
}

void beep() {
    digitalWrite(BUZZER_PIN, HIGH);
    delay(50);
    digitalWrite(BUZZER_PIN, LOW);
}

void checkButtons() {
    if (digitalRead(BTN_MODE) == LOW) {
        beep();
        delay(200);
        if (!modifying && !settingTime && !settingSystemTime) {
            modifying = true;
            selectedBell = 0;
        } else if (modifying) {
            selectedBell++;
            if (selectedBell >= sizeof(bellTimes) / sizeof(bellTimes[0])) {
                modifying = false;
                settingSystemTime = true;
                DateTime now = rtc.now();
                setHour = now.hour();
                setMinute = now.minute();
            }
        } else if (settingSystemTime) {
            settingSystemTime = false;
        }
    }
    if (digitalRead(BTN_UP) == LOW) {
        beep();
        delay(200);
        if (modifying) {
            bellTimes[selectedBell][1] += 5;
            if (bellTimes[selectedBell][1] >= 60) {
                bellTimes[selectedBell][1] = 0;
            }
        } else if (settingSystemTime) {
            setMinute++;
            if (setMinute >= 60) {
                setMinute = 0;
            }
        }
    }
    if (digitalRead(BTN_DOWN) == LOW) {
        beep();
        delay(200);
        if (modifying) {
            bellTimes[selectedBell][0]++;
            if (bellTimes[selectedBell][0] >= 24) {
                bellTimes[selectedBell][0] = 0;
            }
        } else if (settingSystemTime) {
            setHour++;
            if (setHour >= 24) {
                setHour = 0;
            }
        }
    }
    if (digitalRead(BTN_SET) == LOW) {
        beep();
        delay(200);
        if (modifying || settingSystemTime) {
            modifying = false;
            settingSystemTime = false;
            lcd.clear();
            lcd.print("Changes Saved");
            delay(3000);
        }
    }
}