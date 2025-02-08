#ifndef WS2812_H
#define WS2812_H

#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

// Função para inicializar o programa PIO
void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin);

// Função para enviar um valor de cor para o LED
void ws2812_put_pixel(uint32_t pixel_grb);

#endif // WS2812_H