#include "radius_custom.h"
#include "radius.h"

// ...existing code...

void radius_process_response(RADIUS_PACKET *packet) {
    // ...existing code...
    handle_radius_response(packet); // Chama a função customizada para manipular a resposta
    // ...existing code...
}