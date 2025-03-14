#ifndef LED_FUNCTIONS_H
#define LED_FUNCTIONS_H

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pico/stdlib.h"
#include "main.pio.h"
#include "init_GPIO.h"
#include "pico/bootrom.h"
#include "hardware/pio.h"
#include "letters.h"

#define NUM_LEDS 25                      // Number of LEDs in the matrix
#define MAX_TEXT_LENGTH 100              // Max number of characters in the message
#define MAX_ROWS (5 * MAX_TEXT_LENGTH)   // Max number of columns in the concatenated message

typedef struct {
    double r; // Red (0.0 to 1.0)
    double g; // Green (0.0 to 1.0)
    double b; // Blue (0.0 to 1.0)
} RGBColor;

typedef enum {
    CHAR_A, CHAR_B, CHAR_C, CHAR_D, CHAR_E, CHAR_F, CHAR_G, CHAR_H, CHAR_I, CHAR_J,
    CHAR_K, CHAR_L, CHAR_M, CHAR_N, CHAR_O, CHAR_P, CHAR_Q, CHAR_R, CHAR_S, CHAR_T,
    CHAR_U, CHAR_V, CHAR_W, CHAR_X, CHAR_Y, CHAR_Z, CHAR_SPACE, CHAR_EXCLAMATION, CHAR_DOT
} Char;

uint32_t rgb_matrix(double b, double r, double g);
void normalize_color(RGBColor *color);
int map_index_to_position(int index);
void set_led(int index, RGBColor color, PIO pio, uint sm);
double **create_text(const char *text);
void display_frame(double *frame, RGBColor color, PIO pio, uint sm, double intensity);
void concatenate_text(double *text[], int text_length, double full_text[5][MAX_ROWS]);
void show_message(const char *text, RGBColor color, PIO pio, uint sm, double intensity, int speed);
void show_demo1(PIO pio, uint sm, int speed);

uint32_t rgb_matrix(double b, double r, double g) {
    unsigned char R = r * 255;
    unsigned char G = g * 255;
    unsigned char B = b * 255;
    return (G << 24) | (R << 16) | (B << 8); // Encodes as G | R | B
}

void normalize_color(RGBColor *color) {
    if (color->r > 255) color->r = 255;
    if (color->g > 255) color->g = 255;
    if (color->b > 255) color->b = 255;

    if (color->r < 0) color->r = 0;
    if (color->g < 0) color->g = 0;
    if (color->b < 0) color->b = 0;

    color->r /= 255.0;
    color->g /= 255.0;
    color->b /= 255.0;
}

int map_index_to_position(int index) {
    int row = 4 - (index / 5);
    int column = 4 - (index % 5);

    if (row == 1 || row == 3) {
        column = 4 - column;
    }

    return (row * 5) + column;
}

void set_led(int index, RGBColor color, PIO pio, uint sm) {
    uint32_t led_value = rgb_matrix(color.b, color.r, color.g);
    pio_sm_put_blocking(pio, sm, led_value);
}

double **create_text(const char *text) {
    int max_chars = strlen(text);
    double **frames = (double **)malloc((max_chars + 1) * sizeof(double *));
    double **font = letras_5x5;
    double *space = full;

    for (int i = 0; i < max_chars; i++) {
        char c = toupper(text[i]);

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
            default:  frames[i] = font[CHAR_SPACE]; break;
        }
    }

    frames[max_chars] = font[CHAR_SPACE];
    return frames;
}

void display_frame(double *frame, RGBColor color, PIO pio, uint sm, double intensity) {
    if (intensity < 0.0) intensity = 0.0;
    if (intensity > 1.0) intensity = 1.0;

    normalize_color(&color);

    for (int i = 0; i < NUM_LEDS; i++) {
        int physical_index = map_index_to_position(i);

        RGBColor led_color = {
            color.r * frame[physical_index] * intensity,
            color.g * frame[physical_index] * intensity,
            color.b * frame[physical_index] * intensity
        };

        set_led(i, led_color, pio, sm);
    }
}

void concatenate_text(double *text[], int text_length, double full_text[5][MAX_ROWS]) {
    for (int i = 0; i < text_length; i++) {
        for (int row = 0; row < 5; row++) {
            for (int col = 0; col < 5; col++) {
                full_text[row][(i * 5) + col] = text[i][row * 5 + col];
            }
        }
    }
}

void show_message(const char *text, RGBColor color, PIO pio, uint sm, double intensity, int speed) {
    if (!text) return;

    double **frames = create_text(text);
    if (!frames) return;

    int length = strlen(text) + 1;
    int rows_per_letter = 5;
    int columns_per_letter = 5;
    int spacing = 1; // uma row em branco entre letras

    int messa_height = length * (rows_per_letter + spacing) - spacing;

    if (messa_height > MAX_ROWS * 10) { // limite arbitrário de segurança
        free(frames);
        return;
    }

    double full_text[messa_height][5];
    memset(full_text, 0, sizeof(full_text));

    for (int i = 0; i < length; i++) {
        int base_row = i * (rows_per_letter + spacing);
        for (int row = 0; row < rows_per_letter; row++) {
            for (int column = 0; column < columns_per_letter; column++) {
                full_text[base_row + row][column] = frames[i][row * columns_per_letter + column];
            }
        }
    }

    for (int row_base = -4; row_base < messa_height; row_base++) {
        double frame[5][5] = {0};

        for (int row = 0; row < 5; row++) {
            int row_frames = row_base + row;
            for (int column = 0; column < 5; column++) {
                if (row_frames >= 0 && row_frames < messa_height) {
                    frame[row][column] = full_text[row_frames][column];
                } else {
                    frame[row][column] = 0;
                }
            }
        }

        display_frame(&frame[0][0], color, pio, sm, intensity);

        if (!gpio_get(JSTICK)) {
            reset_usb_boot(0, 0);
        }

        sleep_ms(speed);
    }

    free(frames);
}

void show_demo1(PIO pio, uint sm, int speed) {
    uint16_t red[] = {255, 255, 255, 0, 0, 0, 255, 0, 255, 255};
    uint16_t green[] = {0, 165, 255, 255, 255, 0, 0, 255, 255, 128};
    uint16_t blue[] = {0, 0, 0, 0, 255, 75, 255, 255, 255, 128};

    uint16_t cont = 0;

    RGBColor color = {red[cont], green[cont], blue[cont]};

    while (1) {
        for (int i = 0; i < NUM_LEDS; i++) {
            set_led(i, color, pio, sm);
        }

        sleep_ms(speed);

        cont += 1;

        color.r = red[cont];
        color.g = green[cont];
        color.b = blue[cont];
        
        if (cont >= 10) {
            break;
        }
    }

    color.r = 0;
    color.g = 0;
    color.b = 0;

    for (int i = 0; i < NUM_LEDS; i++) {
        set_led(i, color, pio, sm);
    }
    
}



#endif
