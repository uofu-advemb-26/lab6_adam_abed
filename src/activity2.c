#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/stdio_usb.h"

#include "FreeRTOS.h"
#include "task.h"

#define TASK_STACK_SIZE 512

static volatile uint32_t busyBusyCount = 0;
static volatile uint32_t busyYieldCount = 0;
static void supervisor_task(void *params);


static void busy_busy(void *params)
{
    (void)params;

    while (true) {
        busyBusyCount++;
        __asm volatile ("nop");
    }
}

static void busy_yield(void *params)
{
    (void)params;

    while (true) {
        busyYieldCount++;
        __asm volatile ("nop");
        taskYIELD();
    }
}


int main(void)
{
    stdio_init_all();

    while (!stdio_usb_connected()) {
        sleep_ms(100);
    }

    sleep_ms(500);

    printf("\n=================================\n");
    printf("ECE 5785 - Lab 6 - Activity 2\n");
    printf("FreeRTOS Task Scheduling Test\n");
    printf("=================================\n");


    // Experiment 1: Both tasks have equal priority.
BaseType_t result1 = xTaskCreate(
    busy_busy,
    "BusyBusy",
    TASK_STACK_SIZE,
    NULL,
    2,
    NULL
);

BaseType_t result2 = xTaskCreate(
    busy_yield,
    "BusyYield",
    TASK_STACK_SIZE,
    NULL,
    2,
    NULL
);

configASSERT(result1 == pdPASS);
configASSERT(result2 == pdPASS);

BaseType_t result3 = xTaskCreate(
    supervisor_task,
    "Supervisor",
    TASK_STACK_SIZE,
    NULL,
    3,
    NULL
);

configASSERT(result3 == pdPASS);

printf("Starting two equal-priority tasks...\n");

vTaskStartScheduler();

while (true) {
    tight_loop_contents();
}

}


static void supervisor_task(void *params)
{
    (void)params;

    printf("SUPERVISOR: Starting 5-second measurement\n");

    vTaskDelay(pdMS_TO_TICKS(5000));

    uint32_t busyCount = busyBusyCount;
    uint32_t yieldCount = busyYieldCount;

    printf("\n===== ACTIVITY 2 RESULTS =====\n");
    printf("busy_busy iterations: %lu\n",
           (unsigned long)busyCount);
    printf("busy_yield iterations: %lu\n",
           (unsigned long)yieldCount);

    printf("SUPERVISOR: Measurement complete\n");

    vTaskDelete(NULL);
}