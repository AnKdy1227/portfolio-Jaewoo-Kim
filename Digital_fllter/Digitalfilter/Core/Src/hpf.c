#include "HPF.h"

/* MATLAB 결과 (fc=100Hz, fs=10kHz) */
static double b[2] = {0.9695313, -0.9695313};
static double a[2] = {1.0, -0.9390625};
static double x[2] = {0, 0};
static double y[2] = {0, 0};

void HPF_Init(void)
{
    x[0] = x[1] = 0;
    y[0] = y[1] = 0;
}

double HPF_Update(double input)
{
    x[1] = x[0];
    x[0] = input;
    y[1] = y[0];
    y[0] = b[0]*x[0] + b[1]*x[1] - a[1]*y[1];
    return y[0];
}