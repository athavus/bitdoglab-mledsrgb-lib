#include <stdio.h>
#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/clocks.h"
#include "pico/bootrom.h"
#include "main.pio.h"
#include "frames.h"
#include "letters.h"
#include "led_functions.h"

/**
 * Converte valores RGB normalizados (0.0-1.0) para formato de 32 bits
 * usado pela matriz de LEDs
 * @param b Blue (0.0-1.0)
 * @param r Red (0.0-1.0) 
 * @param g Green (0.0-1.0)
 * @return Valor de 32 bits no formato G|R|B para envio aos LEDs
 */
uint32_t rgb_matrix(double b, double r, double g) {
    // Converte valores normalizados para 0-255
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    
    // Empacota as cores no formato esperado pelo hardware
    // Green nos bits 31-24, Red nos bits 23-16, Blue nos bits 15-8
    return (G << 24) | (R << 16) | (B << 8);
}

/**
 * Normaliza valores de cor RGB para o intervalo 0.0-1.0
 * Aplica clamping para garantir que os valores estejam dentro dos limites
 * @param color Ponteiro para estrutura RGBColor a ser normalizada
 */
void normalize_color(RGBColor *color) {
    // Clamp superior - limita valores máximos a 255
    if (color->r > 255) color->r = 255;
    if (color->g > 255) color->g = 255;
    if (color->b > 255) color->b = 255;

    // Clamp inferior - limita valores mínimos a 0
    if (color->r < 0) color->r = 0;
    if (color->g < 0) color->g = 0;
    if (color->b < 0) color->b = 0;

    // Normaliza para 0.0-1.0 dividindo por 255
    color->r /= 255.0;
    color->g /= 255.0;
    color->b /= 255.0;
}

/**
 * Mapeia índice lógico para posição física na matriz LED
 * Considera o padrão serpentina (zigzag) comum em matrizes de LED
 * @param index Índice lógico (0-24 para matriz 5x5)
 * @return Posição física correspondente
 */
int map_index_to_position(int index) {
    // Calcula linha e coluna baseado no índice
    int row = 4 - (index / 5);      // Linha invertida (4 para 0)
    int column = 4 - (index % 5);   // Coluna invertida (4 para 0)

    // Inverte direção nas linhas ímpares para padrão serpentina
    // Linhas 1 e 3 têm direção invertida
    if (row == 1 || row == 3) {
        column = 4 - column;
    }

    return (row * 5) + column;
}

/**
 * Define a cor de um LED específico
 * @param index Índice do LED (0-24)
 * @param color Cor RGB a ser aplicada
 * @param pio Instância PIO para comunicação
 * @param sm State machine do PIO
 */
void set_led(int index, RGBColor color, PIO pio, uint sm) {
    // Converte cor para formato da matriz
    uint32_t led_value = rgb_matrix(color.b, color.r, color.g);
    
    // Envia dados para o LED via PIO (protocolo WS2812 provavelmente)
    pio_sm_put_blocking(pio, sm, led_value);
}

/**
 * Cria array de frames para exibir texto
 * Cada caractere é convertido para sua representação em matriz 5x5
 * @param text String de texto a ser convertida
 * @return Array de ponteiros para frames de cada caractere
 */
double **create_text(const char *text) {
    int max_chars = strlen(text);
    
    // Aloca memória para array de ponteiros (+1 para espaço final)
    double **frames = (double **)malloc((max_chars + 1) * sizeof(double *));
    double **font = letras_5x5;  // Fonte 5x5 definida externamente
    double *space = full;        // Padrão de espaço

    // Mapeia cada caractere para seu frame correspondente
    for (int i = 0; i < max_chars; i++) {
        char c = toupper(text[i]);  // Converte para maiúscula

        // Switch extenso para mapear caracteres para índices da fonte
        switch (c) {
            case 'A': frames[i] = font[CHAR_A]; break;
            case 'B': frames[i] = font[CHAR_B]; break;
            case 'C': frames[i] = font[CHAR_C]; break;
            case 'D': frames[i] = font[CHAR_D]; break;
            case 'E': frames[i] = font[CHAR_E]; break;
            case 'F': frames[i] = font[CHAR_F]; break;
            case 'G': frames[i] = font[CHAR_G]; break;
            case 'H': frames[i] = font[CHAR_H]; break;
            case 'I': frames[i] = font[CHAR_I]; break;
            case 'J': frames[i] = font[CHAR_J]; break;
            case 'K': frames[i] = font[CHAR_K]; break;
            case 'L': frames[i] = font[CHAR_L]; break;
            case 'M': frames[i] = font[CHAR_M]; break;
            case 'N': frames[i] = font[CHAR_N]; break;
            case 'O': frames[i] = font[CHAR_O]; break;
            case 'P': frames[i] = font[CHAR_P]; break;
            case 'Q': frames[i] = font[CHAR_Q]; break;
            case 'R': frames[i] = font[CHAR_R]; break;
            case 'S': frames[i] = font[CHAR_S]; break;
            case 'T': frames[i] = font[CHAR_T]; break;
            case 'U': frames[i] = font[CHAR_U]; break;
            case 'V': frames[i] = font[CHAR_V]; break;
            case 'W': frames[i] = font[CHAR_W]; break;
            case 'X': frames[i] = font[CHAR_X]; break;
            case 'Y': frames[i] = font[CHAR_Y]; break;
            case 'Z': frames[i] = font[CHAR_Z]; break;
            case ' ': frames[i] = font[CHAR_SPACE]; break;
            case '!': frames[i] = font[CHAR_EXCLAMATION]; break;
            case '.': frames[i] = font[CHAR_DOT]; break;
            default:  frames[i] = font[CHAR_SPACE]; break;  // Espaço para caracteres não suportados
        }
    }

    // Adiciona espaço final
    frames[max_chars] = font[CHAR_SPACE];
    return frames;
}

/**
 * Exibe um frame (matriz 5x5) na matriz de LEDs
 * @param frame Array com valores de intensidade para cada LED
 * @param color Cor base para o frame
 * @param pio Instância PIO
 * @param sm State machine PIO
 * @param intensity Intensidade geral (0.0-1.0)
 */
void display_frame(double *frame, RGBColor color, PIO pio, uint sm, double intensity) {
    // Clamp da intensidade
    if (intensity < 0.0) intensity = 0.0;
    if (intensity > 1.0) intensity = 1.0;

    // Normaliza a cor base
    normalize_color(&color);

    // Itera por todos os LEDs da matriz
    for (int i = 0; i < NUM_LEDS; i++) {
        // Mapeia índice lógico para posição física
        int physical_index = map_index_to_position(i);

        // Calcula cor final considerando intensidade do frame e intensidade geral
        RGBColor led_color = {
            color.r * frame[physical_index] * intensity,
            color.g * frame[physical_index] * intensity,
            color.b * frame[physical_index] * intensity
        };

        // Define a cor do LED
        set_led(i, led_color, pio, sm);
    }
}

/**
 * Concatena múltiplos frames de texto em uma matriz maior
 * NOTA: Esta função parece não estar sendo usada no código atual
 * @param text Array de frames de texto
 * @param text_length Número de frames
 * @param full_text Matriz de destino para texto concatenado
 */
void concatenate_text(double *text[], int text_length, double full_text[5][MAX_ROWS]) {
    for (int i = 0; i < text_length; i++) {
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                full_text[row][(i * 5) + col] = text[i][row * 5 + col];
            }
        }
    }
}

/**
 * Acende um LED específico com cor e intensidade definidas
 * @param index Índice do LED (0-24)
 * @param color Cor RGB
 * @param pio Instância PIO
 * @param sm State machine PIO
 * @param intensity Intensidade (0.0-1.0)
 */
void add_led(int index, RGBColor color, PIO pio, uint sm, double intensity) {
    // Verifica se índice está dentro dos limites
    if (index < 0 || index >= NUM_LEDS) return;

    // Clamp da intensidade
    if (intensity < 0.0) intensity = 0.0;
    if (intensity > 1.0) intensity = 1.0;

    // Normaliza cor
    normalize_color(&color);

    // Aplica intensidade à cor
    RGBColor adjusted = {
        color.r * intensity,
        color.g * intensity,
        color.b * intensity
    };

    // Define LED
    set_led(index, adjusted, pio, sm);
}

/**
 * Exibe mensagem de texto com efeito de scroll vertical
 * @param text String de texto a ser exibida
 * @param color Cor do texto
 * @param pio Instância PIO
 * @param sm State machine PIO
 * @param intensity Intensidade do brilho
 * @param speed Velocidade do scroll (delay em ms)
 */
void show_message(const char *text, RGBColor color, PIO pio, uint sm, double intensity, int speed) {
    if (!text) return;  // Proteção contra ponteiro nulo

    // Converte texto para frames
    double **frames = create_text(text);
    if (!frames) return;

    int length = strlen(text) + 1;  // +1 para espaço final
    int rows_per_letter = 5;        // Altura de cada letra
    int columns_per_letter = 5;     // Largura de cada letra
    int spacing = 1;                // Espaçamento entre letras

    // Calcula altura total da mensagem
    int messa_height = length * (rows_per_letter + spacing) - spacing;

    // Proteção contra mensagens muito longas
    if (messa_height > MAX_ROWS * 10) {
        free(frames);
        return;
    }

    // Cria matriz para toda a mensagem
    double full_text[messa_height][5];
    memset(full_text, 0, sizeof(full_text));

    // Monta a mensagem completa concatenando os frames
    for (int i = 0; i < length; i++) {
        int base_row = i * (rows_per_letter + spacing);
        for (int row = 0; row < rows_per_letter; row++) {
            for (int column = 0; column < columns_per_letter; column++) {
                full_text[base_row + row][column] = frames[i][row * columns_per_letter + column];
            }
        }
    }

    // Efeito de scroll: move a "janela" de visualização pela mensagem
    for (int row_base = -4; row_base < messa_height; row_base++) {
        double frame[5][5] = {0};  // Frame atual (5x5)

        // Extrai porção da mensagem para o frame atual
        for (int row = 0; row < 5; row++) {
            int row_frames = row_base + row;
            for (int column = 0; column < 5; column++) {
                if (row_frames >= 0 && row_frames < messa_height) {
                    frame[row][column] = full_text[row_frames][column];
                } else {
                    frame[row][column] = 0;  // Área vazia (fora da mensagem)
                }
            }
        }

        // Exibe frame atual
        display_frame(&frame[0][0], color, pio, sm, intensity);

        // Aguarda antes do próximo frame
        sleep_ms(speed);
    }

    // Libera memória
    free(frames);
}

/**
 * Função de demonstração que executa sequência de cores nos LEDs
 * @param pio Instância PIO
 * @param sm State machine PIO
 * @param speed Velocidade da transição (delay em ms)
 */
void show_demo1(PIO pio, uint sm, int speed) {
    // Arrays com sequência de cores predefinidas
    uint16_t red[]   = {255, 255, 255, 0, 0, 0, 255, 0, 255, 255};
    uint16_t green[] = {0, 165, 255, 255, 255, 0, 0, 255, 255, 128};
    uint16_t blue[]  = {0, 0, 0, 0, 255, 75, 255, 255, 255, 128};

    uint16_t cont = 0;  // Contador para indexar as cores

    RGBColor color = {red[cont], green[cont], blue[cont]};

    // Loop principal da demonstração
    while (1) {
        // Acende todos os LEDs com a cor atual
        for (int i = 0; i < NUM_LEDS; i++) {
            set_led(i, color, pio, sm);
        }

        sleep_ms(speed);  // Pausa

        cont += 1;  // Próxima cor

        // Atualiza cor
        color.r = red[cont];
        color.g = green[cont];
        color.b = blue[cont];
        
        // Sai do loop após 10 iterações
        if (cont >= 10) {
            break;
        }
    }

    // Apaga todos os LEDs no final
    color.r = 0;
    color.g = 0;
    color.b = 0;

    for (int i = 0; i < NUM_LEDS; i++) {
        set_led(i, color, pio, sm);
    }
}