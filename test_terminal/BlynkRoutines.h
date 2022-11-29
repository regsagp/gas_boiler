#pragma once

#ifdef USE_BLYNK

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

#ifdef ESP32
#include <BlynkSimpleEsp32.h>
#ifndef BlynkSimpleEsp32_h_My
#error Used nom-edited blynk. Its doesn't reconnect when loose internet connection
#endif
#else
#include <BlynkSimpleEsp8266.h>
#endif

char auth[] = BLYNK_AUTH_TOKEN;

void CheckConnectService() 
{
    if(Blynk.connected() /*&& _blynkWifiClient.connected()*/) {
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

void SendTemperatureToChart(float temp, int relay_status)
{
    Blynk.virtualWrite(V2, temp);
    if(relay_status >= 0)
        Blynk.virtualWrite(V3, relay_status);
}

BlynkTimer Timer1;

// Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V1);

void terminal_print(const char* str) {
    terminal.print(str);
}
void terminal_println(const char* str) {
    terminal.println(str);
}
void terminal_print(int n) {
    terminal.print(n);
}
void terminal_println(int n) {
    terminal.print(n);
}
void terminal_println() {
    terminal.println();
}
void terminal_flush() {
    terminal.flush();
}


#endif

