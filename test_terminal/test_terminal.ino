// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID           "TMPLrV30J4jZ"
#define BLYNK_DEVICE_NAME           "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "6udSeBvnyEMFv6xlBKz7cBJeV1H6uKkQ"
#define USE_DISPLAY
#define USE_JOYSTICK

// version history:
// 1.0 - add superchart
#define APP_VERSION "ver 1.0"

#include <PietteTech_DHT.h>  // Uncommend if building using CLI
//#include <../libraries/PietteTech_DHT-8266-master/PietteTech_DHT.h>  // Uncommend if building using CLI

#ifdef USE_DISPLAY
#ifdef ESP32
#define D5 17
#define D6 16
#endif
#include <TM1637Display.h>
const int CLK = D6; //Set the CLK pin connection to the display
const int DIO = D5; //Set the DIO pin connection to the display
TM1637Display display(CLK, DIO); //set up the 4-Digit Display.
#endif


#define DHTTYPE  AM2302       // Sensor type DHT11/21/22/AM2301/AM2302
#define DHTPIN   4           // Digital pin for communications

  //declaration
void IRAM_ATTR dht_wrapper(); // must be declared before the lib initialization

// Lib instantiate
PietteTech_DHT DHT(DHTPIN, DHTTYPE, dht_wrapper);
int n;      // counter

void setupPrintDHT()
{
    //Serial.begin(115200);
    //while (!Serial.available()) {
    //    Serial.println("Press any key to start.");
    //    delay(1000);
    //}
    Serial.println("DHT Example program using DHT.acquireAndWait");
    Serial.print("LIB version: ");
    Serial.println(DHTLIB_VERSION);
    Serial.println("---------------");
}

// This wrapper is in charge of calling
// must be defined like this for the lib work
void IRAM_ATTR dht_wrapper() {
    DHT.isrCallback();
}

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#ifdef ESP32
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#define  Relay  23 //LED_BUILTIN//D12 // нога, к которой подключено реле
#else
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#define  Relay  D8 //LED_BUILTIN//D12 // нога, к которой подключено реле
#endif


void loopPrintDHT()
{

    Serial.print("\n");
    Serial.print(n);
    Serial.print(": Retrieving information from sensor: ");
    Serial.print("Read sensor: ");
    //delay(100);

    int result = DHT.acquireAndWait(0);

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
    Serial.print("Humidity (%): ");
    Serial.println(DHT.getHumidity(), 2);

    float temp = DHT.getCelsius();
    Serial.print("Temperature (oC): ");
    Serial.println(temp, 2);

    Blynk.virtualWrite(V2, temp);

    //Serial.print("Temperature (oF): ");
    //Serial.println(DHT.getFahrenheit(), 2);

    //Serial.print("Temperature (K): ");
    //Serial.println(DHT.getKelvin(), 2);

    //Serial.print("Dew Point (oC): ");
    //Serial.println(DHT.getDewPoint());

    //Serial.print("Dew Point Slow (oC): ");
    //Serial.println(DHT.getDewPointSlow());

    n++;
    //delay(2500);
}

//  1 = 61 2 = MegaFonMR100 3=HUAWEI_P30 4=IPHONE
#define WIFI_  2


BlynkTimer Timer1;

#define RelayOn LOW // полярность сигнала включения реде (HIGH/LOW)
#include "Controller.h"

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
#if WIFI_ == 1 //61
char ssid[] = "Mgts kv 61";
char pass[] = "92008310";
#elif WIFI_ == 2
char ssid[] = "MegaFonMR100-3-76B0";
char pass[] = "D8EA3J3N";
#elif WIFI_ == 3
char ssid[] = "HUAWEI P30";
char pass[] = "21a62778cbba";
#elif WIFI_ == 4
char ssid[] = "iPhone a.pervov";
char pass[] = "6xaipxkke89fp";
#endif

#include "Connect.h"

// Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V1);
bool setTempMode = false;

unsigned long last_loop_time, relay_on_time, relay_off_time;
int relay_is_on_state = 0;
#define RESET_RELAY_STATS_TIME 24*60*60*1000ul

void collect_relay_stats()
{
    unsigned long cur_tm = millis();
    unsigned long loop_tm = cur_tm - last_loop_time;
    if (relay_is_on_state == 0)
        relay_off_time += loop_tm;
    else if (relay_is_on_state == 1)
        relay_on_time += loop_tm;
    last_loop_time = cur_tm;

    if (relay_on_time + relay_off_time > RESET_RELAY_STATS_TIME)
    {
        relay_off_time = relay_off_time = 0;
    }
}

void printRelayStats()
{
    unsigned long total_time_ms = relay_on_time + relay_off_time;
    if (total_time_ms > 0) {
        int relay_on_percent = 100ul * relay_on_time / total_time_ms;
        int total_time_min = total_time_ms / (1000 * 60);
        terminal.print("relay: "); terminal.print(relay_on_percent); terminal.print("% on, ");
        terminal.print(total_time_min); terminal.print("min, relay_is_on_state="); terminal.println(relay_is_on_state);
    }
    else
        terminal.println("relay stat: n/a");
}

void printTStat() {
    bool relay = digitalRead(Relay) == RelayOn;
    terminal.print("relay temp:");
    char s[64];
    sprintf(s, "%.1f", TstatTemp);
    terminal.write(s, strlen(s));
    terminal.write(relay ? " [ON]" : " [OFF]");
    terminal.println();
}

// You can send commands from Terminal to your hardware. Just use
// the same Virtual Pin as your Terminal Widget
BLYNK_WRITE(V1)
{
    if (setTempMode)
    {
        Serial.println("terminal setTempMode");

        t = param.asInt();
        if (t >= MIN_TEMP && t <= MAX_TEMP)
            TstatTemp = t;
        else
        {
            Serial.print("incorrect temperature"); Serial.println(t);
            terminal.print("incorrect temperature:"); terminal.println(t);
        }
        setTempMode = false;
        printTStat();
    }
    else {
        // if you type "Marco" into Terminal Widget - it will respond: "Polo:"
        if (String("settemp") == param.asStr()) {
            Serial.println("terminal settemp");
            terminal.print("enter temp:");
            setTempMode = true;
        }
        else if (String("i") == param.asStr()) {
            terminal.print("temp: raw:"); terminal.print(t); terminal.print(", work:"); terminal.println(Temperature);
            terminal.print("hum (%):"); terminal.println(DHT.getHumidity());
            if (acquirestatus != 0) {
                terminal.print("acquirestatus: "); terminal.println(acquirestatus);
                terminal.print("acquireresult: "); terminal.println(acquireresult);
            }
            printTStat();
            printRelayStats();
            terminal.println(APP_VERSION);
        }
        else if (String("heat") == param.asStr()) {
            Serial.println("terminal heat");
            TstatTemp = 22;
            printTStat();
        }
        else if (String("low") == param.asStr()) {
            Serial.println("terminal low");
            TstatTemp = 10;
            printTStat();
        }
        else if (String("sw") == param.asStr()) {
            Serial.println("terminal switch");

            if (digitalRead(Relay) == RelayOn) // если горелка включена -
            {
                Serial.println("RELAY OFF");
                digitalWrite(Relay, !RelayOn); // выключить горелку
                terminal.println("turn off");
            }
            else // если горелка выключена -
            {
                Serial.println("RELAY ON");
                digitalWrite(Relay, RelayOn); // включить горелку
                terminal.println("turn on");
            }
        }
        else {
            Serial.println("terminal back");
            terminal.println("Usage:");
            terminal.println(" settemp");
            terminal.println(" info/i");
            terminal.println(" heat");
            terminal.println(" low");
            terminal.println("echo:");
            terminal.write(param.getBuffer(), param.getLength());
            terminal.println();
        }
    }

    // Ensure everything is sent
    terminal.flush();
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    reconnectBlynk();
    //Blynk.begin(auth, ssid, pass);
    setupPrintDHT();

    Serial.println("Hello2");
    pinMode(Relay, OUTPUT);
    //digitalWrite(Relay, HIGH);
    pinMode(pinX, INPUT);

#ifdef USE_DISPLAY
    display.setBrightness(1); 
#endif
    InitDHT();

    // Setup a function to be called every second
    Timer1.setInterval(500L, timerIsr);

    // You can also specify server:
    //Blynk.begin(auth, ssid, pass, "blynk.cloud", 80);
    //Blynk.begin(auth, ssid, pass, IPAddress(192,168,1,100), 8080);

    // Clear the terminal content
    terminal.clear();

    // This will print Blynk Software version to the Terminal Widget when
    // your hardware gets connected to Blynk Server
    terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
    terminal.println(F("-------------"));
    terminal.println(F("Type 'Marco' and get a reply, or type"));
    terminal.println(F("anything else and get it printed back."));
    terminal.flush();
}

bool online = true;
bool wifilost_flag = false;
int wifilost_timer_start;
int wifilost_timer_max = 60; // 60 sec timeout for reset if WiFi connection lost
int uptime;


bool connectBlynk() {
    _blynkWifiClient.stop();
    return _blynkWifiClient.connect(BLYNK_DEFAULT_DOMAIN, BLYNK_DEFAULT_PORT, 5000);
}


bool old_connect = false;

void checkConnect()
{
    uptime = millis() / 1000;
    if (WiFi.status() == WL_CONNECTED) {
        online = true;
        //Serial.print(Blynk.connected());
      //wifilost_flag = false;

      //if (blynk_token[0] != '\0')
        {

            if (Blynk.connected() /*&& _blynkWifiClient.connected()*/) {
                Blynk.run();
            }
            else {
                Blynk.begin(auth, ssid, pass);

                Serial.print("\n\rReconnecting to blynk.. "); Serial.println(Blynk.connected());
                //if (!_blynkWifiClient.connected()) {
                //    connectBlynk();
                //    return;
                //}

                //FIXME: add exit after n-unsuccesfull tries.
                //Blynk.connect(4000);
                Serial.print("..."); Serial.println(Blynk.connected());
            }
        }
    }
    else {
        online = false;
        if ((millis() - startMillsPrintNotConnected) > 5000) {
            startMillsPrintNotConnected = millis();
            Serial.println("wifi not connected");
        }
    }
        
    if (WiFi.status() != WL_CONNECTED && online) {
        if (!wifilost_flag) {
            wifilost_timer_start = uptime;
            wifilost_flag = true;
        }
        if (((uptime - wifilost_timer_start) > wifilost_timer_max) && wifilost_flag) {
            Serial.print("\n\rWiFi connection lost, restarting..");
            wifilost_flag = false;
            //ESP.restart();
            reconnectBlynk();
        }
    }
}

void loop()
{
    //Blynk.run();
    checkConnect();

    if ((millis() - startMills) > REPORT_INTERVAL * 2) {
        loopPrintDHT();
    }

    Timer1.run();
    FetchTemp();

    if ((millis() - controlMillis) > 500) {
        //Serial.println("checkControl");
        checkControl();
        controlMillis = millis();
    }

    if ((millis() - startMills) > REPORT_INTERVAL) {
        if (acquireresult == 0) {

            if ((millis() - startMillsPrint) > PRINT_INTERVAL) {
                Serial.println("");

                Serial.print("Humidity (%): ");
                Serial.println(h);

                Serial.print("Temperature (oC): ");
                Serial.println(t);

                Serial.print("Dew Point (oC): ");
                Serial.println(d);
                startMillsPrint = millis();

                Blynk.virtualWrite(V2, Temperature);

            }
#ifdef USE_JOYSTICK
            //Serial.print("joystickReady: ");  Serial.println(joystickReady);
#endif

#ifdef USE_DISPLAY
            if (MenuTimeoutTimer == 0)
                display.showNumberDec(Temperature * 10);
#endif

            int relay_state = checkRelay();
            if (relay_state >= 0)
                relay_is_on_state = relay_state;
        }
        else {
            Serial.println("Is dht22 connected");
        }
        startMills = millis();

        CheckDHT();
    }

    collect_relay_stats();
}

// ============================ Timer0 interrupt =============================
// run every 500ms
void timerIsr()
{
    //checkControl();
    blink500ms = !blink500ms; // инверсия мерцающего бита
    if (blink500ms) {
        plus1sec = true; // ежесекундно взводится
        if (TstatTimer != 0) {
            TstatTimer--; // ежесекундный декремент этого таймера
        }
        if (MenuTimeoutTimer != 0) {
            MenuTimeoutTimer--; // ежесекундный декремент этого таймера
        }
    }

    //Serial.print("RELAY: "); Serial.println(blink500ms);
    //digitalWrite(Relay, blink500ms); // выключить горелку
}
