#define MY_IP                                                                  \
  { 192, 168, 100, 2 }
#define MQTT_SN_BROKER                                                         \
  { 192, 168, 100, 1 }
#define PREDEFINED_TOPIC_ID 1

// Maximal length of the MQTT-SN payload
#ifndef MQTTSN_DATA_MAX
#define MQTTSN_DATA_MAX 64
#endif

#ifdef PACKET_SERIAL_USB
// Cannot use both serial and usb serial :(
//#define DBG_SER Serial
#else
#define DBG_SER Serial2
#endif
