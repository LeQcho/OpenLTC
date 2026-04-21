#pragma once
#include <Arduino.h>
#include "config.h"

// Variables partagées avec ltc.cpp (ISR slave)
extern volatile bool    syncPending;
extern volatile uint8_t pendingHH, pendingMM, pendingSS, pendingFF;

void initESPNow();

#ifdef ROLE_MASTER
void masterSendSync();
#endif
