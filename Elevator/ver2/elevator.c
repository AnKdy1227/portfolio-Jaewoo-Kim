#include "elevator.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"
#include <string.h>
#include "cmsis_os2.h"

// ===== RTOS objects =====
static QueueHandle_t     gElevatorReqQ = NULL;
static SemaphoreHandle_t gStateMutex   = NULL;

// ===== Shared state =====
static ElevatorState gState;



static void set_direction_from_target(void)
{
    if (gState.targetFloor > gState.currentFloor) {
        gState.dir = ELEV_DIR_UP;
        gState.moving = true;
    } else if (gState.targetFloor < gState.currentFloor) {
        gState.dir = ELEV_DIR_DOWN;
        gState.moving = true;
    } else {
        gState.dir = ELEV_DIR_IDLE;
        gState.moving = false;
    }
}

void Elevator_Init(void)
{
	//gEmgEvt = osEventFlagsNew(NULL); // 이머전시 이벤트
    memset(&gState, 0, sizeof(gState));
    gState.currentFloor = 1;
    gState.targetFloor  = 1;
    gState.dir  = ELEV_DIR_IDLE;
    gState.door = DOOR_CLOSED;
    gState.moving = false;

    // FreeRTOS 객체 생성 (스케줄러 시작 전에도 OK)
    gElevatorReqQ = xQueueCreate(4, sizeof(ElevatorRequest));
    gStateMutex   = xSemaphoreCreateMutex();


}

void Elevator_SendFloorRequest(uint8_t floor)
{
    if (floor < 1 || floor > 8) return;
    if (gElevatorReqQ == NULL) return;

    ElevatorRequest req = { .targetFloor = floor };
    (void)xQueueSend(gElevatorReqQ, &req, 0);
}

void Elevator_GetState(ElevatorState* outState)
{
    if (!outState) return;

    if (gStateMutex) xSemaphoreTake(gStateMutex, portMAX_DELAY);
    *outState = gState;
    if (gStateMutex) xSemaphoreGive(gStateMutex);
}

void ElevatorControlTask(void *argument)
{
    (void)argument;

    ElevatorRequest req;
    uint32_t doorTimerMs = 0;
    const uint32_t tickMs = 100;
    uint32_t moveAccumMs = 0;

    for (;;)
    {


        // 요청 받기 (논블록)
        if (gElevatorReqQ && xQueueReceive(gElevatorReqQ, &req, 0) == pdTRUE)
        {
            if (gStateMutex) xSemaphoreTake(gStateMutex, portMAX_DELAY);

            gState.targetFloor = req.targetFloor;

            if (gState.door == DOOR_OPEN || gState.door == DOOR_OPENING) {
                gState.door = DOOR_CLOSING;
                doorTimerMs = 500;
            } else if (gState.door == DOOR_CLOSED) {
                set_direction_from_target();
            }

            if (gStateMutex) xSemaphoreGive(gStateMutex);
        }

        // 상태머신 진행
        if (gStateMutex) xSemaphoreTake(gStateMutex, portMAX_DELAY);

        switch (gState.door)
        {
        case DOOR_CLOSING:
            if (doorTimerMs > tickMs) doorTimerMs -= tickMs;
            else { doorTimerMs = 0; gState.door = DOOR_CLOSED; set_direction_from_target(); }
            break;

        case DOOR_OPENING:
            if (doorTimerMs > tickMs) doorTimerMs -= tickMs;
            else { doorTimerMs = 0; gState.door = DOOR_OPEN; doorTimerMs = 1500; }
            break;

        case DOOR_OPEN:
            if (doorTimerMs > tickMs) doorTimerMs -= tickMs;
            else { doorTimerMs = 0; gState.door = DOOR_CLOSING; doorTimerMs = 500; }
            break;

        case DOOR_CLOSED:
        default:
            if (gState.moving && gState.dir != ELEV_DIR_IDLE) {
                moveAccumMs += tickMs;
                if (moveAccumMs >= 1000) {
                    moveAccumMs = 0;

                    if (gState.dir == ELEV_DIR_UP && gState.currentFloor < 8) gState.currentFloor++;
                    else if (gState.dir == ELEV_DIR_DOWN && gState.currentFloor > 1) gState.currentFloor--;

                    if (gState.currentFloor == gState.targetFloor) {
                        gState.dir = ELEV_DIR_IDLE;
                        gState.moving = false;
                        gState.door = DOOR_OPENING;
                        doorTimerMs = 500;
                    }
                }
            } else {
                moveAccumMs = 0;
            }
            break;
        }

        if (gStateMutex) xSemaphoreGive(gStateMutex);

        vTaskDelay(pdMS_TO_TICKS(tickMs));
    }
}

