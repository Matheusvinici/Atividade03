#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/i2c.h"
#include "hardware/gpio.h"
#include "hardware/irq.h"
#include "inc/ssd1306.h"
#include "inc/font.h"
#include <stdio.h>
#include <ctype.h>
#include "ws2812.h"  // Inclua a biblioteca WS2812
#include "ws2812.pio.h"


#define I2C_PORT i2c1
#define I2C_SDA 14
#define I2C_SCL 15
#define ENDERECO 0x3C
#define WIDTH 128
#define HEIGHT 64

#define BUTTON_A_PIN 5
#define BUTTON_B_PIN 6
#define LED_RGB_RED_PIN 11
#define LED_RGB_GREEN_PIN 12
#define LED_RGB_BLUE_PIN 13
#define WS2812_PIN 7

// Variáveis globais para o estado dos LEDs
bool led_green_state = false;
bool led_blue_state = false;

// Função para exibir o nome "Marcos" e outras animações
void exibir_animacao(ssd1306_t *ssd) {
    static bool cor = true; // Flag para alternar a cor
    cor = !cor;

    // Atualiza o conteúdo do display com animações
    ssd1306_fill(ssd, !cor); // Limpa o display
    ssd1306_rect(ssd, 3, 3, 122, 58, cor, !cor); // Desenha um retângulo
    ssd1306_draw_string(ssd, "    MATHEUS", 8, 10); // Desenha a string "MATHEUS"
    ssd1306_draw_string(ssd, "VINICIUS", 20, 30); // Desenha a string "VINICIUS"
    ssd1306_draw_string(ssd, "   Vidal", 15, 48); // Desenha a string "Vidal"
    ssd1306_send_data(ssd); // Atualiza o display
}

// Função para debouncing
void debounce() {
    sleep_ms(50);
}

// Função de interrupção para o botão A
void button_a_isr(uint gpio, uint32_t events) {
  printf("Interrupção do botão A acionada\n");
  debounce();
  led_green_state = !led_green_state;
  gpio_put(LED_RGB_GREEN_PIN, led_green_state);
  printf("Botão A pressionado: LED Verde %s\n", led_green_state ? "Ligado" : "Desligado");
}

// Função de interrupção para o botão B
void button_b_isr(uint gpio, uint32_t events) {
  printf("Interrupção do botão B acionada\n");
  debounce();
  led_blue_state = !led_blue_state;
  gpio_put(LED_RGB_BLUE_PIN, led_blue_state);
  printf("Botão B pressionado: LED Azul %s\n", led_blue_state ? "Ligado" : "Desligado");
}
// Função para exibir um número na matriz de LEDs WS2812
void exibir_numero_ws2812(uint8_t numero) {
    static uint32_t numeros[10] = {
        0x3F3F3F,  // 0
        0x0F0F0F,  // 1
        0x3F0F3F,  // 2
        0x3F3F0F,  // 3
        0x0F3F0F,  // 4
        0x0F3F3F,  // 5
        0x3F3F3F,  // 6
        0x0F0F3F,  // 7
        0x3F3F3F,  // 8
        0x0F3F3F   // 9
    };

    // Envia o valor de cor para o LED
    ws2812_put_pixel(numeros[numero]);
}

int main() {


    // Inicialização do I2C. Usando a taxa de 400kHz.
    i2c_init(I2C_PORT, 400 * 1000);

    gpio_set_function(I2C_SDA, GPIO_FUNC_I2C); // Configura o pino SDA para I2C
    gpio_set_function(I2C_SCL, GPIO_FUNC_I2C); // Configura o pino SCL para I2C
    gpio_pull_up(I2C_SDA); // Habilita o pull-up no pino SDA
    gpio_pull_up(I2C_SCL); // Habilita o pull-up no pino SCL

    ssd1306_t ssd; // Inicializa a estrutura do display
    ssd1306_init(&ssd, WIDTH, HEIGHT, false, ENDERECO, I2C_PORT); // Inicializa o display
    ssd1306_config(&ssd); // Configura o display
    ssd1306_send_data(&ssd); // Envia os dados para o display

    // Limpa o display. O display começa com todos os pixels apagados.
    ssd1306_fill(&ssd, false);
    ssd1306_send_data(&ssd);

    // Inicializa a comunicação serial
    stdio_init_all();  // Inicializa a comunicação serial
    sleep_ms(100);     // Aguarda um pouco para garantir a inicialização

    // Configura os botões
    gpio_init(BUTTON_A_PIN);
    gpio_init(BUTTON_B_PIN);
    gpio_set_dir(BUTTON_A_PIN, GPIO_IN);
    gpio_set_dir(BUTTON_B_PIN, GPIO_IN);
    gpio_pull_up(BUTTON_A_PIN);
    gpio_pull_up(BUTTON_B_PIN);

    // Configura as interrupções para os botões
    gpio_set_irq_enabled_with_callback(BUTTON_A_PIN, GPIO_IRQ_EDGE_FALL, true, &button_a_isr);
    gpio_set_irq_enabled_with_callback(BUTTON_B_PIN, GPIO_IRQ_EDGE_FALL, true, &button_b_isr);

    // Configura os LEDs RGB
    gpio_init(LED_RGB_RED_PIN);
    gpio_init(LED_RGB_GREEN_PIN);
    gpio_init(LED_RGB_BLUE_PIN);
    gpio_set_dir(LED_RGB_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_RGB_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_RGB_BLUE_PIN, GPIO_OUT);

    // Inicializa a matriz de LEDs WS2812
    PIO pio = pio0;
    uint offset = pio_add_program(pio, &ws2812_program);
    uint sm = pio_claim_unused_sm(pio, true);
    ws2812_program_init(pio, sm, offset, WS2812_PIN);

    while (true) {
        // Exibe a animação do nome "Marcos" e outras strings
        exibir_animacao(&ssd);

        char c = '\0';
        bool char_received = false;

        // Espera até que um caractere numérico seja digitado
        while (!char_received) {
            // Verifica se há caracteres disponíveis no Serial Monitor
            if (stdio_usb_connected()) {
                // Tenta ler um caractere
                c = getchar_timeout_us(0); 
                if (c != PICO_ERROR_TIMEOUT) { // Se um caractere foi recebido
                    printf("Caractere recebido (hex): 0x%X\n", c);  // Exibe o valor hexadecimal

                    // Verifica se o caractere é um número de '0' a '9'
                    if (isdigit(c)) {
                        printf("Caractere numérico recebido: %c\n", c);  // Exibe o caractere numérico
                        char_received = true;  // Marca que o caractere foi recebido
                        exibir_numero_ws2812(c - '0');  // Exibe o número na matriz de LEDs WS2812
                    } else {
                        // Não exibe mensagem para caracteres não numéricos
                        // printf("Caractere não numérico ignorado: %c\n", c); // Se quiser logar a ignorância, pode ativar
                    }
                }
            }

            sleep_ms(100); // Espera 100ms antes de tentar ler novamente
        }

        // Limpa o display e exibe o caractere recebido
        ssd1306_fill(&ssd, false);
        ssd1306_draw_char(&ssd, c, 60, 28); // Exibe o caractere no centro do display
        ssd1306_send_data(&ssd);

        // Aguarda 2 segundos antes de retornar à animação
        sleep_ms(2000); // 2000ms = 2 segundos
    }

    return 0;
}