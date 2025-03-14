#ifndef INIT_GPIO_H
#define INIT_GPIO_H

#include "pico/stdlib.h"

#define OUT_PIN 7
#define JSTICK 22

void init_GPIO()
{
    gpio_init(JSTICK);
    gpio_set_dir(JSTICK, GPIO_IN);
    gpio_pull_up(JSTICK);
}

#endif