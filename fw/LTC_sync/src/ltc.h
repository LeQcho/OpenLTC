#pragma once
#include <Arduino.h>
#include "config.h"

// State LTC (accessible depuis ISR et sync)
extern volatile uint8_t hh, mm, ss, ff;
extern volatile uint8_t ltcFrame[10];
extern volatile uint8_t currentBit;
extern volatile uint8_t halfBitCount;
extern volatile uint8_t sampleCount;
extern volatile bool    dacState;

void buildLTCFrame();
void advanceTimecode();
uint8_t getCurrentBitValue();
void onTimer();
void ltcStart();
