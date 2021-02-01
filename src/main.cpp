#include <Arduino.h>

#include "app_config.h"
#include "mqtt_sn.h"

const ip_addr_t my_ip = MY_IP;
const ip_addr_t mqttsn_broker = MQTT_SN_BROKER;
// predefined topic id
const uint16_t topic_id = PREDEFINED_TOPIC_ID;

const uint8_t *data = (const uint8_t *)"Hello!";
const uint8_t data_len = 6;

void blinks(int n);
static void on_pkt_rx(const uint8_t *buffer, size_t size);

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  blinks(3);

#ifdef DBG_SER
  DBG_SER.begin(115200);
  DBG_SER.println("Starting.");
#endif

  mqttsn_init(on_pkt_rx);
}

void loop() {
  unsigned long now = millis();
  static unsigned long last_send = 0;
  if (now - last_send > 5000) {
    last_send = now;
    mqttsn_quicksend(my_ip, mqttsn_broker, topic_id, data, data_len);
  }

  mqttsn_update();
}

void blinks(int n) {
  for (int i = 0; i < n; i++) {
    digitalWrite(LED_BUILTIN, LOW);
    delay(25);
    digitalWrite(LED_BUILTIN, HIGH);
    if (i < n - 1)
      delay(150);
  }
}

static void on_pkt_rx(const uint8_t *buffer, size_t size) {
  if (size == 0) {
    return;
  }

  blinks(1);

#ifdef DBG_SER
  DBG_SER.print("\nReceived ");
  DBG_SER.print(size);
  DBG_SER.print("B:\n--------");
  for (size_t i = 0; i < size; i++) {
    if (i % 8 == 0)
      DBG_SER.print(" ");
    if (i % 16 == 0)
      DBG_SER.println();
    if (buffer[i] < 0x10)
      DBG_SER.print("0");
    DBG_SER.print(buffer[i], HEX);
    DBG_SER.print(" ");
  }
  DBG_SER.println("\n--------");
#endif
}
