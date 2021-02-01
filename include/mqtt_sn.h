#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t ip_addr_t[4];
typedef void (*mqttsn_on_pkt_rx_t)(const uint8_t *buffer, size_t size);

int mqttsn_init(mqttsn_on_pkt_rx_t on_pkt_rx);
void mqttsn_update();
int mqttsn_quicksend(const ip_addr_t src, const ip_addr_t dst,
                     uint16_t topic_id, const uint8_t *data, uint8_t data_len);

#ifdef __cplusplus
}
#endif
