#pragma once

#include <Arduino.h>


typedef enum {
    ON,
    OFF,
    HEATING,
    WAITING,
} state_t;

typedef enum {
   SCREEN_STATUS,
   SCREEN_CONFIG
} screen_t;