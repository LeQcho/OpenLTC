#include "ltc.h"
#include <driver/dac_oneshot.h>

// ═══════════════════════════════════════
// DAC handle
// ═══════════════════════════════════════
static dac_oneshot_handle_t dac_handle = NULL;

// ═══════════════════════════════════════
// STATE
// ═══════════════════════════════════════
volatile uint8_t hh = 1, mm = 0, ss = 0, ff = 0;
volatile uint8_t ltcFrame[10];
volatile uint8_t currentBit   = 0;
volatile uint8_t halfBitCount = 0;
volatile uint8_t sampleCount  = 0;
volatile bool    dacState      = false;

static hw_timer_t *timer = NULL;

// ═══════════════════════════════════════
// BCD HELPERS
// ═══════════════════════════════════════
static uint8_t toBCD_lo(uint8_t val) { return val % 10; }
static uint8_t toBCD_hi(uint8_t val) { return val / 10; }

// ═══════════════════════════════════════
// BUILD FRAME
// ═══════════════════════════════════════
void buildLTCFrame() {
    uint8_t frame[80];
    memset(frame, 0, 80);

    uint8_t f_lo = toBCD_lo(ff), f_hi = toBCD_hi(ff);
    frame[0] = (f_lo >> 0) & 1; frame[1] = (f_lo >> 1) & 1;
    frame[2] = (f_lo >> 2) & 1; frame[3] = (f_lo >> 3) & 1;
    frame[8] = (f_hi >> 0) & 1; frame[9] = (f_hi >> 1) & 1;

    uint8_t s_lo = toBCD_lo(ss), s_hi = toBCD_hi(ss);
    frame[16] = (s_lo >> 0) & 1; frame[17] = (s_lo >> 1) & 1;
    frame[18] = (s_lo >> 2) & 1; frame[19] = (s_lo >> 3) & 1;
    frame[24] = (s_hi >> 0) & 1; frame[25] = (s_hi >> 1) & 1;
    frame[26] = (s_hi >> 2) & 1;

    uint8_t m_lo = toBCD_lo(mm), m_hi = toBCD_hi(mm);
    frame[32] = (m_lo >> 0) & 1; frame[33] = (m_lo >> 1) & 1;
    frame[34] = (m_lo >> 2) & 1; frame[35] = (m_lo >> 3) & 1;
    frame[40] = (m_hi >> 0) & 1; frame[41] = (m_hi >> 1) & 1;
    frame[42] = (m_hi >> 2) & 1;

    uint8_t h_lo = toBCD_lo(hh), h_hi = toBCD_hi(hh);
    frame[48] = (h_lo >> 0) & 1; frame[49] = (h_lo >> 1) & 1;
    frame[50] = (h_lo >> 2) & 1; frame[51] = (h_lo >> 3) & 1;
    frame[56] = (h_hi >> 0) & 1; frame[57] = (h_hi >> 1) & 1;

    uint16_t syncWord = 0xBFFC;
    for (int i = 0; i < 16; i++) {
        frame[64 + i] = (syncWord >> i) & 1;
    }

    for (int i = 0; i < 10; i++) {
        uint8_t byte = 0;
        for (int b = 0; b < 8; b++) {
            byte |= (frame[i * 8 + b] << b);
        }
        ltcFrame[i] = byte;
    }
}

// ═══════════════════════════════════════
// ADVANCE TIMECODE
// ═══════════════════════════════════════
void advanceTimecode() {
    ff++;
    if (ff >= FPS) { ff = 0; ss++;
    if (ss >= 60)  { ss = 0; mm++;
    if (mm >= 60)  { mm = 0; hh++;
    if (hh >= 24)  { hh = 0; }}}}
    buildLTCFrame();
}

// ═══════════════════════════════════════
// GET BIT
// ═══════════════════════════════════════
uint8_t getCurrentBitValue() {
    return (ltcFrame[currentBit / 8] >> (currentBit % 8)) & 1;
}

// ═══════════════════════════════════════
// ISR
// ═══════════════════════════════════════
extern volatile bool syncPending;
extern volatile uint8_t pendingHH, pendingMM, pendingSS, pendingFF;

void IRAM_ATTR onTimer() {
    sampleCount++;
    if (sampleCount < SAMPLES_PER_HALF_BIT) {
        dac_oneshot_output_voltage(dac_handle, dacState ? DAC_HIGH : DAC_LOW);
        return;
    }
    sampleCount = 0;
    halfBitCount++;

    uint8_t bitVal = getCurrentBitValue();

    if (halfBitCount % 2 == 1) {
        if (bitVal == 1) dacState = !dacState;
    } else {
        dacState = !dacState;
        currentBit++;
        if (currentBit >= BITS_PER_FRAME) {
            currentBit    = 0;
            halfBitCount  = 0;
#ifdef ROLE_SLAVE
            if (syncPending) {
                hh = pendingHH;
                mm = pendingMM;
                ss = pendingSS;
                ff = pendingFF;
                syncPending = false;
            }
#endif
            advanceTimecode();
        }
    }

    dac_oneshot_output_voltage(dac_handle, dacState ? DAC_HIGH : DAC_LOW);
}

// ═══════════════════════════════════════
// START
// ═══════════════════════════════════════
void ltcStart() {
    dac_oneshot_config_t dac_cfg = {
        .chan_id = DAC_CHANNEL
    };
    dac_oneshot_new_channel(&dac_cfg, &dac_handle);
    dac_oneshot_output_voltage(dac_handle, DAC_MID);

    buildLTCFrame();

    timer = timerBegin(SAMPLE_RATE);
    timerAttachInterrupt(timer, &onTimer);
    timerAlarm(timer, 1, true, 0);
}
