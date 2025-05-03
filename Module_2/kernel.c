#include "io.h"

void main() {
    uart_init();
    uart_writeText("Hello from kernel!\n");
    while (1);
}