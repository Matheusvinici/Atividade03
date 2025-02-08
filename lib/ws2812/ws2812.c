#include "ws2812.h"
#include "ws2812.pio.h"  // Gerado automaticamente pelo PIO

// Inicializa o programa PIO para controlar os LEDs WS2812
void ws2812_program_init(PIO pio, uint sm, uint offset, uint pin) {
    pio_sm_config c = ws2812_program_get_default_config(offset);

    // Configura o pino de saída
    sm_config_set_set_pins(&c, pin, 1);
    pio_gpio_init(pio, pin);
    pio_sm_set_consecutive_pindirs(pio, sm, pin, 1, true);

    // Configura o clock do PIO (8 MHz)
    float div = clock_get_hz(clk_sys) / 8000000.0;
    sm_config_set_clkdiv(&c, div);

    // Configura o FIFO e o deslocamento de dados
    sm_config_set_fifo_join(&c, PIO_FIFO_JOIN_TX);
    sm_config_set_out_shift(&c, false, true, 24);

    // Inicializa a máquina de estados do PIO
    pio_sm_init(pio, sm, offset, &c);
    pio_sm_set_enabled(pio, sm, true);
}

// Envia um valor de cor para o LED
void ws2812_put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8);
}