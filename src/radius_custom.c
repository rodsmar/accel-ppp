#include "radius.h"
#include "logger.h"

void handle_radius_response(RADIUS_PACKET *packet) {
    if (packet->code == PW_ACCESS_REJECT) {
        // Verifica se o pacote contém o atributo Mikrotik-Rate-Limit
        if (radius_get_attr(packet, PW_VENDOR_SPECIFIC, VENDOR_MICROTIK, PW_MICROTIK_RATE_LIMIT)) {
            log_info("Access-Reject with Mikrotik-Rate-Limit detected, treating as Access-Accept");
            packet->code = PW_ACCESS_ACCEPT;
        }
    }
}
