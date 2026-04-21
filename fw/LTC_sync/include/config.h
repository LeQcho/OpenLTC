#pragma once

#include <driver/dac_oneshot.h>

// ══════════════════════════════════════
// RÔLE — décommenter UN SEUL
// ══════════════════════════════════════
//#define ROLE_MASTER
#define ROLE_SLAVE

// ══════════════════════════════════════
// LTC
// ══════════════════════════════════════
#define FPS                   25
#define SAMPLE_RATE           48000
#define DAC_CHANNEL           DAC_CHAN_0      // GPIO17 sur S2 Mini
#define BITS_PER_FRAME        80
#define SAMPLES_PER_BIT       (SAMPLE_RATE / (FPS * BITS_PER_FRAME))   // 24
#define SAMPLES_PER_HALF_BIT  (SAMPLES_PER_BIT / 2)                    // 12

// ══════════════════════════════════════
// DAC
// ══════════════════════════════════════
#define DAC_HIGH   218
#define DAC_LOW    38
#define DAC_MID    128

// ══════════════════════════════════════
// ESP-NOW
// ══════════════════════════════════════
#define SYNC_INTERVAL_MS  1000
#define SYNC_MAGIC        0xAA
