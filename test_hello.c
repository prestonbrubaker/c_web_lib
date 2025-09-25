// test_hello.c
#include "hello.h"

int main(void) {
    tcp_server(8081);  // Start server on port 12345
    return 0;  // Never reached (server loops forever)
}
