// === INCLUSÃO DE BIBLIOTECAS ===
#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"         // Funções básicas da Raspberry Pi Pico
#include "hardware/clocks.h"     // Controle de clock
#include "pico/bootrom.h"        // Funções de boot
#include "main.pio.h"            // Programa PIO para controle de LEDs/matriz
#include "frames.h"              // Animações ou quadros predefinidos
#include "letters.h"             // Letras para a rolagem de texto
#include "led_functions.h"       // Funções de controle de LED

// === CONFIGURAÇÕES DO SISTEMA ===
#define SYS_CLOCK_KHZ 128000     // Clock do sistema definido para 128 MHz
#define INTENSITY 0.1            // Intensidade dos LEDs (0.0 a 1.0)
#define SPEED 150                // Velocidade da rolagem de texto em milissegundos
#define DEBOUNCE_TIME_MS 400     // Tempo de espera para evitar múltiplos cliques no botão
#define PHRASE "VIRTUS CC"       // Frase que será exibida na matriz de LEDs
#define COLOR_LED_R 100          // Valor do canal vermelho
#define COLOR_LED_G 156          // Valor do canal verde
#define COLOR_LED_B 255          // Valor do canal azul
#define BUTTONA_PIN 5            // GPIO do botão A
#define BUTTONB_PIN 6            // GPIO do botão B
#define OUT_PIN 7                // GPIO de saída para o PIO

// === FUNÇÃO DE INICIALIZAÇÃO DA MATRIZ COM PIO ===
bool matrix_init(PIO *pio, uint *sm, uint *offset)
{
    // Ajusta o clock do sistema
    if (!set_sys_clock_khz(SYS_CLOCK_KHZ, false))
    {
        return false;
    }

    // Seleciona o PIO0
    *pio = pio0;

    // Carrega o programa PIO na memória e obtém o offset
    *offset = pio_add_program(*pio, &main_program);

    // Reivindica um state machine disponível
    *sm = pio_claim_unused_sm(*pio, true);

    // Verifica se houve erro na alocação
    if (*offset == -1 || *sm == -1)
    {
        return false;
    }

    // Inicializa o programa PIO com os parâmetros definidos
    main_program_init(*pio, *sm, *offset, OUT_PIN);

    return true;
}

// === VARIÁVEIS DE CONTROLE GLOBAL ===
volatile bool demo_active = false;     // Ativa o modo demo
volatile bool message_active = false;  // Ativa o modo de mensagem
PIO pio;       // PIO selecionado
uint sm, offset; // State machine e offset do programa PIO

// === CALLBACK DO BOTÃO (INTERRUPÇÃO) ===
void button_callback(uint gpio, uint32_t events)
{
    // Controle de debounce usando timestamp
    static absolute_time_t last_time = {0};
    absolute_time_t now = get_absolute_time();

    // Ignora se o tempo entre cliques for menor que o limite
    if (absolute_time_diff_us(last_time, now) / 1000 < DEBOUNCE_TIME_MS)
    {
        return;
    }

    last_time = now;

    // Botão B ativa o modo de mensagem
    if ((gpio == BUTTONB_PIN) && (events & GPIO_IRQ_EDGE_FALL))
    {
        demo_active = false;
        message_active = true;
    }

    // Botão A ativa o modo demo
    if ((gpio == BUTTONA_PIN) && (events & GPIO_IRQ_EDGE_FALL))
    {
        message_active = false;
        demo_active = true;
    }
}

// === MODO DEMO: MOSTRA UMA ANIMAÇÃO PRÉ-DEFINIDA ===
void demo_test()
{
    printf("VOCÊ ENTROU NO MODO DE DEMO\n");
    printf("VALOR DO pio: %p\n", (void *)pio);
    printf("VALOR DO sm: %d\n", sm);
    printf("VALOR DA INTENSIDADE: 1.0\n\n");
    sleep_ms(1000);

    // Função que mostra a animação (implementada em frames.h/.c)
    show_demo1(pio, sm, 500);
}

// === MODO MENSAGEM: MOSTRA TEXTO EM ROLAGEM NA MATRIZ ===
void message_test(RGBColor message_color)
{
    printf("VOCÊ ENTROU NO MODO DE MENSAGEM EM ROLAGEM\n");
    printf("FRASE ESCOLHIDA: %s\n", PHRASE);
    printf("CORES DA MENSAGEM R:%d G:%d B:%d\n", COLOR_LED_R, COLOR_LED_G, COLOR_LED_B);
    printf("VALOR DO pio: %p\n", (void *)pio);
    printf("VALOR DO sm: %d\n", sm);
    printf("VALOR DA INTENSIDADE: %.1f\n", INTENSITY);
    printf("VELOCIDADE DA MENSAGEM: %d ms\n\n", SPEED);
    sleep_ms(1000);

    // Mostra a mensagem configurada rolando na matriz de LEDs
    show_message(PHRASE, message_color, pio, sm, INTENSITY, SPEED);
}

// === FUNÇÃO PRINCIPAL ===
int main()
{
    stdio_init_all(); // Inicializa USB serial (debug)

    // === Configuração do botão A ===
    gpio_init(BUTTONA_PIN);
    gpio_set_dir(BUTTONA_PIN, GPIO_IN);
    gpio_pull_up(BUTTONA_PIN);
    gpio_set_irq_enabled_with_callback(BUTTONA_PIN, GPIO_IRQ_EDGE_FALL, true, &button_callback);

    // === Configuração do botão B ===
    gpio_init(BUTTONB_PIN);
    gpio_set_dir(BUTTONB_PIN, GPIO_IN);
    gpio_pull_up(BUTTONB_PIN);
    gpio_set_irq_enabled(BUTTONB_PIN, GPIO_IRQ_EDGE_FALL, true);

    // Inicializa a matriz com PIO
    if (!matrix_init(&pio, &sm, &offset))
    {
        return 1; // Se falhar, encerra o programa
    }

    // Define a cor da mensagem
    RGBColor message_color = {COLOR_LED_R, COLOR_LED_G, COLOR_LED_B};

    printf("INICIO DOS TESTES\n\n");

    // Mostra a animação de demo e a frase uma vez no boot
    demo_test();
    message_test(message_color);

    printf("TESTES FINALIZADOS\n\n");

    sleep_ms(10); // Delay leve

    // === LOOP PRINCIPAL ===
    while (1)
    {
        // Adiciona LEDs nos cantos da matriz com cores diferentes
        add_led(0, (RGBColor){255, 0, 0}, pio, sm, 0.1);     // LED vermelho no canto
        add_led(4, (RGBColor){0, 255, 0}, pio, sm, 0.1);     // LED verde
        add_led(20, (RGBColor){0, 0, 255}, pio, sm, 0.1);    // LED azul
        add_led(24, (RGBColor){255, 255, 0}, pio, sm, 0.1);  // LED amarelo

        sleep_ms(2000); // Delay para não congestionar a CPU
    }
}
