//﻿#if ndef CONTROLLER_H
//#define CONTROLLER_H

bool bDHTstarted;       // flag to indicate we started acquisition


#define TstatTimerMax 120 //минимальная пауза между включениями горелки, сек
unsigned int TstatTimer = 20; //таймер паузы между включениями/выключениями, начальная установка 20 сек для устаканивания системы после сброса
float TstatTemp = 8; //температура термостатирования, может изменяться настройками
float TemperatureCorr = 0; //коррекция температуры, может изменяться настройками
const int MIN_TEMP = 8;
const int MAX_TEMP = 35;
float DS18B20Temperature = 0; //сырая температура от датчика
float Temperature = 0; //вычисленная температура с коррекцией
float DS18B20TempTmp; //времянка
byte DS18B20iteration = 0; //счётчик измерений температуры для усреднения

float Hysteresis = 0.5; // гистерезис термостата, может изменяться настройками
boolean blink500ms = false; // мигающий бит, инвертируется каждые 500мс
boolean plus1sec = false; // ежесекундно взводится
byte MenuTimeoutTimer;

float t, h, d;
int acquireresult;
int acquirestatus;

#define REPORT_INTERVAL 5000 // in msec must > 2000
#define PRINT_INTERVAL 15*60*1000 // print interval 

// to check dht
unsigned long startMills, startMillsPrint, startMillsPrintNotConnected, controlMillis;

void SendTemperatureToChart(float temp, int relay_status);

int printDHTResult(int result)
{
    switch (result) {
    case DHTLIB_OK:
        Serial.println("OK");
        break;
    case DHTLIB_ERROR_CHECKSUM:
        Serial.println("Error\n\r\tChecksum error");
        break;
    case DHTLIB_ERROR_ISR_TIMEOUT:
        Serial.println("Error\n\r\tISR time out error");
        break;
    case DHTLIB_ERROR_RESPONSE_TIMEOUT:
        Serial.println("Error\n\r\tResponse time out error");
        break;
    case DHTLIB_ERROR_DATA_TIMEOUT:
        Serial.println("Error\n\r\tData time out error");
        break;
    case DHTLIB_ERROR_ACQUIRING:
        Serial.println("Error\n\r\tAcquiring");
        break;
    case DHTLIB_ERROR_DELTA:
        Serial.println("Error\n\r\tDelta time to small");
        break;
    case DHTLIB_ERROR_NOTSTARTED:
        Serial.println("Error\n\r\tNot started");
        break;
    default:
        Serial.println("Unknown error");
        break;
    }
}

void InitDHT() {
    // delay 2 sec before start acquire
    delay(2000);
    //startMills = millis();
    //while ( (millis() - startMills) < 2000 ) {
    //  yield();
    //}
    // blocking method
    acquirestatus = 0;
    acquireresult = DHT.acquireAndWait(1000);
    if (acquireresult == 0) {
        Serial.println("dht acquired");
        Temperature = t = DHT.getCelsius();
        h = DHT.getHumidity();
        d = DHT.getDewPoint();
    }
    else {
        printDHTResult(acquireresult);
        //Serial.print("dht acquired failed, err="); Serial.println(acquireresult);
        t = h = d = 0;
    }
}

void FetchTemp()
{
    if (bDHTstarted) {
        acquirestatus = DHT.acquiring();
        if (!acquirestatus) {
            acquireresult = DHT.getStatus();
            if (acquireresult == 0) {
                t = DHT.getCelsius();
                h = DHT.getHumidity();
                d = DHT.getDewPoint();

                DS18B20Temperature += t;
                DS18B20iteration++;
                if (DS18B20iteration == 10)
                {
                    DS18B20iteration = 0;
                    Temperature = (DS18B20Temperature / 10) + TemperatureCorr; //усреднённая + коррекция
                    DS18B20Temperature = 0;
                }
            }
            bDHTstarted = false;
        }
    }
}

void CheckDHT() {
    // to remove lock
    if (acquirestatus == 1) {
        //Serial.println("DHT.reset()");
        DHT.reset();
    }

    if (!bDHTstarted) {
        //Serial.println("DHT.acquire()");
        // non blocking method
        DHT.acquire();
        bDHTstarted = true;
    }
}

// returns: 
// 0 - relay is off
// 1 - relay is on
// -1 - undefined
int checkRelay() {
    // термостатирование
    if (TstatTimer == 0)
    {
        Serial.print("checkRelay: "); Serial.print(Temperature); Serial.print(", TstatTemp:"); Serial.println(TstatTemp + Hysteresis);
        if (Temperature > (TstatTemp + Hysteresis)) // гистерезис
        {
            if (digitalRead(Relay) == RelayOn) // если горелка включена -
            {
                Serial.println("RELAY OFF");
                digitalWrite(Relay, !RelayOn); // выключить горелку
                TstatTimer = TstatTimerMax; // горелку держать выключённой не менее заданного в TstatTimerMax времени
                SendTemperatureToChart(Temperature, 0);
            }
            return 0;
        }
        if (Temperature < TstatTemp)
        {
            if (digitalRead(Relay) == !RelayOn) // если горелка выключена -
            {
                Serial.println("RELAY ON");
                digitalWrite(Relay, RelayOn); // включить горелку
                TstatTimer = TstatTimerMax; // горелку держать включённой не менее заданного в TstatTimerMax времени
                SendTemperatureToChart(Temperature, 1);
            }
            return 1;
        }
    }
    return -1;
}

const int pinX = 34;
#ifdef ESP32
const int ADC_RES = 4096;
#else
const int ADC_RES = 1024;
#endif

const int joystickMax = ADC_RES;
const int joystickZero = joystickMax / 2;

bool joystickReady = false;

int lastX = -1;
void checkControl() {

    int X = analogRead(pinX); // СЃС‡РёС‚С‹РІР°РµРј Р°РЅР°Р»РѕРіРѕРІРѕРµ Р·РЅР°С‡РµРЅРёРµ РѕСЃРё РҐ

    const int jNoise = joystickZero / 5; //10
    if (fabs(lastX - X) > jNoise / 2) {
        lastX = X;
        Serial.print("MenuTimeoutTimer="); Serial.print(MenuTimeoutTimer); Serial.print(", jX = ");  Serial.println(X);
    }

    if (!joystickReady) {
        if (abs(X - joystickZero) > jNoise) // joystick not plugged
            return;

        Serial.println("Joystick is ready");
        joystickReady = true;
    }

    if (abs(X - joystickZero) < jNoise) // skip noise
        return;

    if (MenuTimeoutTimer != 0) { // in menu mode
        if (X > joystickZero * 3 / 2)
            TstatTemp += 0.5;
        else if (X < joystickZero / 2)
            TstatTemp -= 0.5;
        else
            return;
    }

    MenuTimeoutTimer = 10; //таймер таймаута, секунд

    TstatTemp = constrain(TstatTemp, MIN_TEMP, MAX_TEMP);
    Serial.print("Set TEMP = ");
    Serial.println(TstatTemp);
    display.showNumberDec(TstatTemp);

    Serial.print("TstatTimer=");
    Serial.println(TstatTimer);

    //delay(500);
    //startMills = millis();
    //while ( (millis() - startMills) < 500 ) {
    //  yield();
    //}
}
//#endif
