#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "main.pio.h"
#include "init_GPIO.h"
#include "frames.h"
#include "letters.h"
#include "led_functions.h"

// ==== CONFIGURAÇÕES DO SISTEMA ====
#define SYS_CLOCK_KHZ        128000  // Clock do sistema em kHz (128 MHz)
#define INTENSITY      0.1     // Intensidade dos LEDs (0.0 a 1.0)
#define SPEED    150     // Velocidade da rolagem em ms
#define DEBOUNCE_TIME_MS     400     // Tempo de debounce para o botão
#define PHRASE        "VIRTUS CC"  // Texto na matriz
#define COLOR_LED_R            100
#define COLOR_LED_G            156
#define COLOR_LED_B            255
#define BUTTONA_PIN 5 // Botão A conectado ao GPIO5
#define BUTTONB_PIN 6 // Botão B conectado ao GPIO6

bool matrix_init(PIO *pio, uint *sm, uint *offset) {
    if (!set_sys_clock_khz(SYS_CLOCK_KHZ, false)) {
        return false;
    }
    *pio = pio0;
    *offset = pio_add_program(*pio, &main_program);
    *sm = pio_claim_unused_sm(*pio, true);
    if (*offset == -1 || *sm == -1) {
        return false;
    }
    main_program_init(*pio, *sm, *offset, OUT_PIN);
    return true;
}

volatile bool demo_active = false;
volatile bool message_active = false;
PIO pio;
uint sm, offset;

void button_callback(uint gpio, uint32_t events) {
    // Debounce simples
    static absolute_time_t last_time = {0};
    absolute_time_t now = get_absolute_time();
    
    if (absolute_time_diff_us(last_time, now) / 1000 < DEBOUNCE_TIME_MS) {
        return;  // Tempo de debounce não passou
    }
    
    last_time = now;
    
    if ((gpio == BUTTONB_PIN) && (events & GPIO_IRQ_EDGE_FALL)) {
        demo_active = false;
        message_active = true;

    }
    if ((gpio == BUTTONA_PIN) && (events & GPIO_IRQ_EDGE_FALL)) {
        message_active = false;
        demo_active = true;
    }
}

int main() {
    stdio_init_all();
    init_GPIO();      
    
    // Configuração dos botões com pull-up interno
    gpio_init(BUTTONA_PIN);
    gpio_set_dir(BUTTONA_PIN, GPIO_IN);
    gpio_pull_up(BUTTONA_PIN);
    gpio_set_irq_enabled_with_callback(BUTTONA_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);
   
    gpio_init(BUTTONB_PIN);
    gpio_set_dir(BUTTONB_PIN, GPIO_IN);
    gpio_pull_up(BUTTONB_PIN);
    gpio_set_irq_enabled(BUTTONB_PIN, GPIO_IRQ_EDGE_FALL, true);
    
    if (!matrix_init(&pio, &sm, &offset)) {
        return 1;
    }
    
    // Cor em RGB
    RGBColor message_color = {COLOR_LED_R, COLOR_LED_G, COLOR_LED_B};
    
    // Inicialmente, não mostrar nada
    while (1) {
        if (!gpio_get(JSTICK)) {
            reset_usb_boot(0, 0); // Reinicia em modo bootloader
        }
        
        // Verifica se deve mostrar o demo
        if (demo_active) {
            printf("VOCÊ ENTROU NO MODO DE DEMO\n");
            printf("VALOR DO pio: %p\n", (void *)pio);
            printf("VALOR DO sm: %d\n", sm);
            printf("VALOR DA INTENSIDADE: 1.0\n\n");
            sleep_ms(1000);
            show_demo1(pio, sm, 500);
            demo_active = false;
        }
        
        // Verifica se deve mostrar a mensagem
        if (message_active) {
            printf("VOCÊ ENTROU NO MODO DE MENSAGEM EM ROLAGEM\n");
            printf("FRASE ESCOLHIDA: %s\n", PHRASE);
            printf("CORES DA MENSAGEM R:%d G:%d B:%d\n", COLOR_LED_R, COLOR_LED_G, COLOR_LED_B);
            printf("VALOR DO pio: %p\n", (void *)pio);
            printf("VALOR DO sm: %d\n", sm);
            printf("VALOR DA INTENSIDADE: %.1f\n", INTENSITY);
            printf("VELOCIDADE DA MENSAGEM: %d ms\n\n", SPEED);
            sleep_ms(1000);
            show_message(
                PHRASE,
                message_color,
                pio,
                sm,
                INTENSITY,
                SPEED
            );
            message_active = false;
        }
        
        // Pequena pausa para não sobrecarregar o CPU
        sleep_ms(10);
    }
}