#include "BPF.h"

static float b0=0.0062f, b1=0.0f, b2=-0.0062f;
static float a1=-1.9836f, a2=0.9875f;
static float x1=0.0f,x2=0.0f,y1=0.0f,y2=0.0f;

void BPF_Init(void){ x1=x2=y1=y2=0.0f; }

float BPF_Update(float x){
    float y = b0*x + b1*x1 + b2*x2 - a1*y1 - a2*y2;
    x2=x1; x1=x; y2=y1; y1=y;
    return y;
}
