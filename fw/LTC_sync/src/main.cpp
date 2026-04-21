#include <Arduino.h>
#include "config.h"
#include "ltc.h"
#include "sync.h"
#include <WiFi.h>        

#if !defined(ROLE_MASTER) && !defined(ROLE_SLAVE)
  #error "Définis ROLE_MASTER ou ROLE_SLAVE dans config.h"
#endif
#if defined(ROLE_MASTER) && defined(ROLE_SLAVE)
  #error "Choisis UN SEUL rôle dans config.h"
#endif

void setup() {
    Serial.begin(115200);
    delay(500);

#ifdef ROLE_MASTER
    Serial.println("\n=== LTC MASTER ===");
#else
    Serial.println("\n=== LTC SLAVE ===");
#endif
    Serial.print("MAC: ");
    Serial.println(WiFi.macAddress());

    initESPNow();
    ltcStart();

    Serial.printf("LTC started @ %d fps\n", FPS);
}

void loop() {
#ifdef ROLE_MASTER
    masterSendSync();
#endif

    static unsigned long lastPrint = 0;
    if (millis() - lastPrint >= 1000) {
        lastPrint = millis();
        Serial.printf("TC: %02d:%02d:%02d:%02d\n", hh, mm, ss, ff);
    }
}
