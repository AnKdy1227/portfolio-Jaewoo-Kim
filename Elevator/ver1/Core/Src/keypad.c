/*
 * keypad.c
 *
 *  Created on: Sep 23, 2025
 *      Author: andykim02
 */
#include "keypad.h"


// --- 키패드 핀 설정 (사용자 환경에 맞게 수정) ---
// 열(Column) 핀 - 출력으로 설정
#define C1_PORT GPIOB
#define C1_PIN  GPIO_PIN_4
#define C2_PORT GPIOB
#define C2_PIN  GPIO_PIN_5
#define C3_PORT GPIOB
#define C3_PIN  GPIO_PIN_6
#define C4_PORT GPIOB
#define C4_PIN  GPIO_PIN_7

// 행(Row) 핀 - 입력(Pull-up)으로 설정  이게 GPIO_INPUT
#define R1_PORT GPIOB
#define R1_PIN  GPIO_PIN_0
#define R2_PORT GPIOB
#define R2_PIN  GPIO_PIN_1
#define R3_PORT GPIOB
#define R3_PIN  GPIO_PIN_2
#define R4_PORT GPIOB
#define R4_PIN  GPIO_PIN_3

/**
  * @brief  4x4 매트릭스 키패드를 스캔하여 눌린 키 값을 반환합니다.
  * @retval 눌린 키의 문자 값 ('0'-'9', '*', '#'). 눌리지 않았으면 '\0' 반환.
  */
char read_keypad(void)
{
    // 1열(C1)을 LOW로 설정
    HAL_GPIO_WritePin(C1_PORT, C1_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(C2_PORT, C2_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C3_PORT, C3_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C4_PORT, C4_PIN, GPIO_PIN_SET);

    if (HAL_GPIO_ReadPin(R1_PORT, R1_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R1_PORT, R1_PIN) == GPIO_PIN_RESET); return '1'; }
    if (HAL_GPIO_ReadPin(R2_PORT, R2_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R2_PORT, R2_PIN) == GPIO_PIN_RESET); return '4'; }
    if (HAL_GPIO_ReadPin(R3_PORT, R3_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R3_PORT, R3_PIN) == GPIO_PIN_RESET); return '7'; }
    if (HAL_GPIO_ReadPin(R4_PORT, R4_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R4_PORT, R4_PIN) == GPIO_PIN_RESET); return '*'; }

    // 2열(C2)을 LOW로 설정
    HAL_GPIO_WritePin(C1_PORT, C1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C2_PORT, C2_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(C3_PORT, C3_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C4_PORT, C4_PIN, GPIO_PIN_SET);

    if (HAL_GPIO_ReadPin(R1_PORT, R1_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R1_PORT, R1_PIN) == GPIO_PIN_RESET); return '2'; }
    if (HAL_GPIO_ReadPin(R2_PORT, R2_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R2_PORT, R2_PIN) == GPIO_PIN_RESET); return '5'; }
    if (HAL_GPIO_ReadPin(R3_PORT, R3_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R3_PORT, R3_PIN) == GPIO_PIN_RESET); return '8'; }
    if (HAL_GPIO_ReadPin(R4_PORT, R4_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R4_PORT, R4_PIN) == GPIO_PIN_RESET); return '0'; }

    // 3열(C3)을 LOW로 설정
    HAL_GPIO_WritePin(C1_PORT, C1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C2_PORT, C2_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C3_PORT, C3_PIN, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(C4_PORT, C4_PIN, GPIO_PIN_SET);

    if (HAL_GPIO_ReadPin(R1_PORT, R1_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R1_PORT, R1_PIN) == GPIO_PIN_RESET); return '3'; }
    if (HAL_GPIO_ReadPin(R2_PORT, R2_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R2_PORT, R2_PIN) == GPIO_PIN_RESET); return '6'; }
    if (HAL_GPIO_ReadPin(R3_PORT, R3_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R3_PORT, R3_PIN) == GPIO_PIN_RESET); return '9'; }
    if (HAL_GPIO_ReadPin(R4_PORT, R4_PIN) == GPIO_PIN_RESET) { HAL_Delay(50); while(HAL_GPIO_ReadPin(R4_PORT, R4_PIN) == GPIO_PIN_RESET); return '#'; }

    // 4열(C4)을 LOW로 설정 (4x4 키패드의 경우)
    HAL_GPIO_WritePin(C1_PORT, C1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C2_PORT, C2_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C3_PORT, C3_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(C4_PORT, C4_PIN, GPIO_PIN_RESET);

    // if (HAL_GPIO_ReadPin(R1_PORT, R1_PIN) == GPIO_PIN_RESET) { HAL_Delay(20); while(HAL_GPIO_ReadPin(R1_PORT, R1_PIN) == GPIO_PIN_RESET); return 'A'; }
    //... (필요시 A, B, C, D 키 추가)

    return '\0'; // 아무 키도 눌리지 않음
}

