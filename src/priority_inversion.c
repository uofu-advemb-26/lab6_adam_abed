#include "pico/stdlib.h"
#include "pico/stdio_usb.h"
#include <stdio.h>
#include "pico/stdlib.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"

#define LOW_PRIORITY        1
#define MEDIUM_PRIORITY     2
#define HIGH_PRIORITY       3
#define SUPERVISOR_PRIORITY 4

#define TASK_STACK_SIZE     512

static SemaphoreHandle_t sharedSemaphore;

static volatile bool lowHasSemaphore = false;
static volatile bool highIsWaiting = false;
static volatile bool mediumFinished = false;
static volatile bool lowFinished = false;
static volatile bool highFinished = false;

static void busy_work(uint32_t duration_ms)
{
    TickType_t start = xTaskGetTickCount();
    TickType_t duration = pdMS_TO_TICKS(duration_ms);

    while ((xTaskGetTickCount() - start) < duration) {
        __asm volatile ("nop");
    }
}

static void low_task(void *params)
{
    (void)params;

    xSemaphoreTake(sharedSemaphore, portMAX_DELAY);
    lowHasSemaphore = true;

    printf("LOW: Acquired semaphore\n");

    busy_work(1000);

    printf("LOW: Releasing semaphore\n");
    xSemaphoreGive(sharedSemaphore);

    lowFinished = true;
    vTaskDelete(NULL);
}

static void medium_task(void *params)
{
    (void)params;

    while (!lowHasSemaphore) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelay(pdMS_TO_TICKS(100));

    printf("MEDIUM: Starting CPU-intensive work\n");
    busy_work(2000);

    printf("MEDIUM: Finished\n");
    mediumFinished = true;

    vTaskDelete(NULL);
}

static void high_task(void *params)
{
    (void)params;

    while (!lowHasSemaphore) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    vTaskDelay(pdMS_TO_TICKS(200));

    printf("HIGH: Requesting semaphore\n");
    highIsWaiting = true;

    TickType_t start = xTaskGetTickCount();

    xSemaphoreTake(sharedSemaphore, portMAX_DELAY);

    TickType_t elapsed = xTaskGetTickCount() - start;

    printf("HIGH: Acquired semaphore after %lu ms\n",
           (unsigned long)(elapsed * portTICK_PERIOD_MS));



    highFinished = true;
    vTaskDelete(NULL);
}

static void supervisor_task(void *params)
{
    (void)params;

    printf("SUPERVISOR: Starting Activity 0\n");

    sharedSemaphore = xSemaphoreCreateMutex();
    configASSERT(sharedSemaphore != NULL);

    // A binary semaphore starts empty.
    xSemaphoreGive(sharedSemaphore);

    // Start the low-priority task first.
    xTaskCreate(low_task, "Low", TASK_STACK_SIZE,
                NULL, LOW_PRIORITY, NULL);

    // Wait until the low-priority task holds the semaphore.
    while (!lowHasSemaphore) {
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    xTaskCreate(medium_task, "Medium", TASK_STACK_SIZE,
                NULL, MEDIUM_PRIORITY, NULL);

    xTaskCreate(high_task, "High", TASK_STACK_SIZE,
                NULL, HIGH_PRIORITY, NULL);

    while (!highFinished || !mediumFinished || !lowFinished) {
        vTaskDelay(pdMS_TO_TICKS(10));
    }

    printf("SUPERVISOR: Activity 0 complete\n");

    vSemaphoreDelete(sharedSemaphore);
    vTaskDelete(NULL);
}

int main(void)
{
    stdio_init_all();

    // Wait for the USB serial monitor to connect.
while (!stdio_usb_connected()) {
    sleep_ms(100);
}

// Give the connection time to settle.
sleep_ms(500);

    printf("\n=================================\n");
    printf("ECE 5785 - Lab 6 - Activity 0\n");
    printf("FreeRTOS Priority Inversion Test\n");
    printf("=================================\n");

    xTaskCreate(supervisor_task, "Supervisor",
                TASK_STACK_SIZE, NULL,
                SUPERVISOR_PRIORITY, NULL);

    vTaskStartScheduler();

    while (true) {
        tight_loop_contents();
    }
}