// === APF.c ===
#include "APF.h"

static float a = 0.9391f;   // MATLAB에서 계산된 계수
static float x_prev = 0.0f;
static float y_prev = 0.0f;

void APF_Init(void) {
    x_prev = 0.0f;
    y_prev = 0.0f;
}

float APF_Update(float x) {
    float y = -a * x + x_prev + a * y_prev;
    x_prev = x;
    y_prev = y;
    return y;
}
