#include "sync.h"
#include "ltc.h"
#include <WiFi.h>
#include <esp_now.h>
#include <esp_wifi.h>

static const uint8_t broadcastAddr[] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

// ═══════════════════════════════════════
// SLAVE PENDING SYNC
// ═══════════════════════════════════════
volatile bool    syncPending = false;
volatile uint8_t pendingHH = 0, pendingMM = 0, pendingSS = 0, pendingFF = 0;

// ═══════════════════════════════════════
// PACKET
// ═══════════════════════════════════════
typedef struct {
    uint8_t magic;
    uint8_t hh, mm, ss, ff;
} __attribute__((packed)) sync_packet_t;

// ═══════════════════════════════════════
// CALLBACKS
// ═══════════════════════════════════════
#ifdef ROLE_MASTER
static void onSendDone(const uint8_t *mac, esp_now_send_status_t status) {}
#endif

#ifdef ROLE_SLAVE
static void onReceive(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len != sizeof(sync_packet_t)) return;
    const sync_packet_t *pkt = (const sync_packet_t *)data;
    if (pkt->magic != SYNC_MAGIC) return;

    // +1 frame pour compenser latence
    uint8_t nff = pkt->ff + 1;
    uint8_t nss = pkt->ss, nmm = pkt->mm, nhh = pkt->hh;
    if (nff >= FPS) { nff = 0; nss++;
    if (nss >= 60)  { nss = 0; nmm++;
    if (nmm >= 60)  { nmm = 0; nhh++;
    if (nhh >= 24)  { nhh = 0; }}}}

    pendingHH = nhh; pendingMM = nmm;
    pendingSS = nss; pendingFF = nff;
    syncPending = true;
}
#endif

// ═══════════════════════════════════════
// INIT
// ═══════════════════════════════════════
void initESPNow() {
    WiFi.mode(WIFI_STA);
    WiFi.disconnect();
    esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);

    if (esp_now_init() != ESP_OK) {
        Serial.println("ESP-NOW init FAILED");
        while (1) delay(1000);
    }

#ifdef ROLE_MASTER
    esp_now_register_send_cb(onSendDone);
    esp_now_peer_info_t peer = {};
    memcpy(peer.peer_addr, broadcastAddr, 6);
    peer.channel = 1;
    peer.encrypt = false;
    esp_now_add_peer(&peer);
    Serial.println("ESP-NOW Master ready");
#endif

#ifdef ROLE_SLAVE
    esp_now_register_recv_cb(onReceive);
    Serial.println("ESP-NOW Slave ready");
#endif
}

// ═══════════════════════════════════════
// MASTER SEND
// ═══════════════════════════════════════
#ifdef ROLE_MASTER
void masterSendSync() {
    static unsigned long lastSync = 0;
    if (millis() - lastSync < SYNC_INTERVAL_MS) return;
    lastSync = millis();

    sync_packet_t pkt;
    pkt.magic = SYNC_MAGIC;
    pkt.hh = hh; pkt.mm = mm;
    pkt.ss = ss; pkt.ff = ff;

    esp_now_send(broadcastAddr, (uint8_t *)&pkt, sizeof(pkt));
    Serial.printf("SYNC sent: %02d:%02d:%02d:%02d\n", hh, mm, ss, ff);
}
#endif
