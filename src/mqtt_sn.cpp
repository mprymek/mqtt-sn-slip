#include <PacketSerial.h>

#include "app_config.h"
#include "mqtt_sn.h"

#define CKSUM_IP 0
#define CKSUM_UDP 1
#define CKSUM_TCP 2

static uint16_t checksum(uint8_t *buf, uint16_t len, uint8_t type);

static SLIPPacketSerial pktser;

int mqttsn_init(mqttsn_on_pkt_rx_t on_pkt_rx) {
#ifdef PACKET_SERIAL_USB
  SerialUSB.begin();
  pktser.setStream(&SerialUSB);
#else
  Serial1.begin(115200);
  pktser.setStream(&Serial1);
#endif

  if (on_pkt_rx != NULL) {
    pktser.setPacketHandler(on_pkt_rx);
  }

  return 0;
}

void mqttsn_update() { pktser.update(); }

int mqttsn_quicksend(const ip_addr_t src, const ip_addr_t dst,
                     uint16_t topic_id, const uint8_t *data, uint8_t data_len) {
  if (data_len > MQTTSN_DATA_MAX) {
    return -1;
  }

  const uint8_t mqttsn_hdr_len = 7;
  const uint8_t udp_hdr_len = 8;
  const uint8_t ip_hdr_len = 20;

  uint8_t buf[MQTTSN_DATA_MAX + 7 + 8 + 20] = {
      // IP header
      0x45, 0, 0, 0, 0, 0, 0x40, 0, 0x40, 0x11, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
      // UDP header
      0, 0, 0, 0, 0, 0, 0, 0,
      // MQTT-SN header
      0, 0x0C, 0x61, 0, 0, 0, 0};
  uint8_t *ip = buf;
  uint8_t *udp = &buf[ip_hdr_len];
  uint8_t *mqtt = &buf[ip_hdr_len + udp_hdr_len];

  uint8_t mqtt_len = mqttsn_hdr_len + data_len;
  uint16_t udp_len = udp_hdr_len + mqtt_len;
  uint16_t pkt_len = ip_hdr_len + udp_len;

  uint16_t src_port = 35000 /*+ millis() % 1024*/;
  uint16_t dst_port = 1883;
  static uint16_t msg_id = 0;

  msg_id++;
  uint16_t ip_msg_id = msg_id;

  //// IP header
  // src IP
  ip[12] = src[0];
  ip[13] = src[1];
  ip[14] = src[2];
  ip[15] = src[3];
  // dst IP
  ip[16] = dst[0];
  ip[17] = dst[1];
  ip[18] = dst[2];
  ip[19] = dst[3];

  // pkt len
  ip[2] = pkt_len >> 8;
  ip[3] = pkt_len & 0xFF;
  // identification
  // should be non-repeating...
  ip[4] = ip_msg_id >> 8;
  ip[5] = ip_msg_id & 0xFF;
  // cksum
  ip[10] = buf[11] = 0;
  uint16_t cksum = checksum(buf, 20, CKSUM_IP);
  ip[10] = cksum >> 8;
  ip[11] = cksum & 0xFF;

  //// UDP header
  // src port
  udp[0] = src_port >> 8;
  udp[1] = src_port & 0xFF;
  // dst port
  udp[2] = dst_port >> 8;
  udp[3] = dst_port & 0xFF;
  // length
  udp[4] = udp_len >> 8;
  udp[5] = udp_len & 0xFF;
  // cksum
  // The checksum is optional in IPv4.
  // ...and I don't know how to use checksum() for UDP correctly :(
  // cksum = checksum(udp, udp_len+8, CKSUM_UDP);
  // udp[6] = cksum >> 8;
  // udp[7] = cksum & 0xFF;

  //// MQTT-SN header
  mqtt[0] = mqtt_len;
  // topic id
  mqtt[3] = topic_id >> 8;
  mqtt[4] = topic_id & 0xFF;
  // msg id
  mqtt[5] = msg_id >> 8;
  mqtt[6] = msg_id & 0xFF;

  //// data
  uint8_t *data2 = &buf[ip_hdr_len + udp_hdr_len + mqttsn_hdr_len];
  memcpy(data2, data, data_len);

#ifdef DBG_SER
  DBG_SER.println("Sending packet.");
#endif

  pktser.send(buf, pkt_len);

  return 0;
}

// Taken from https://github.com/renatoaloi/EtherEncLib/blob/master/checksum.c

// The Ip checksum is calculated over the ip header only starting
// with the header length field and a total length of 20 bytes
// unitl ip.dst
// You must set the IP checksum field to zero before you start
// the calculation.
// len for ip is 20.
//
// For UDP/TCP we do not make up the required pseudo header. Instead we
// use the ip.src and ip.dst fields of the real packet:
// The udp checksum calculation starts with the ip.src field
// Ip.src=4bytes,Ip.dst=4 bytes,Udp header=8bytes + data length=16+len
// In other words the len here is 8 + length over which you actually
// want to calculate the checksum.
// You must set the checksum field to zero before you start
// the calculation.
// len for udp is: 8 + 8 + data length
// len for tcp is: 4+4 + 20 + option len + data length
//
// For more information on how this algorithm works see:
// http://www.netfor2.com/checksum.html
// http://www.msc.uky.edu/ken/cs471/notes/chap3.htm
// The RFC has also a C code example: http://www.faqs.org/rfcs/rfc1071.html
#define IP_PROTO_TCP_V 0x06
#define IP_PROTO_UDP_V 0x11

static uint16_t checksum(uint8_t *buf, uint16_t len, uint8_t type) {
  // type 0=ip
  //      1=udp
  //      2=tcp
  uint32_t sum = 0;

  // if(type==0){
  //        // do not add anything
  //}
  if (type == CKSUM_UDP) {
    sum += IP_PROTO_UDP_V; // protocol udp
    // the length here is the length of udp (data+header len)
    // =length given to this function - (IP.scr+IP.dst length)
    sum += len - 8; // = real tcp len
  }
  if (type == CKSUM_TCP) {
    sum += IP_PROTO_TCP_V;
    // the length here is the length of tcp (data+header len)
    // =length given to this function - (IP.scr+IP.dst length)
    sum += len - 8; // = real tcp len
  }
  // build the sum of 16bit words
  while (len > 1) {
    sum += 0xFFFF & (*buf << 8 | *(buf + 1));
    buf += 2;
    len -= 2;
  }
  // if there is a byte left then add it (padded with zero)
  if (len) {
    //--- made by SKA ---                sum += (0xFF & *buf)<<8;
    sum += 0xFFFF & (*buf << 8 | 0x00);
  }
  // now calculate the sum over the bytes in the sum
  // until the result is only 16bit long
  while (sum >> 16) {
    sum = (sum & 0xFFFF) + (sum >> 16);
  }
  // build 1's complement:
  return ((uint16_t)sum ^ 0xFFFF);
}