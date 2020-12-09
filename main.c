/* Standard includes. */
#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

/* FreeRTOS kernel includes. */
#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"
#include "timers.h"

/* Local includes. */
#include "console.h"

/* Priorities at which the tasks are created. */
#define mainQUEUE_RECEIVE_TASK_PRIORITY (tskIDLE_PRIORITY + 2)
#define mainQUEUE_SEND_TASK_PRIORITY (tskIDLE_PRIORITY + 1)

/* The rate at which data is sent to the queue.  The times are converted from
milliseconds to ticks using the pdMS_TO_TICKS() macro. */
#define mainTASK_SEND_FREQUENCY_MS pdMS_TO_TICKS(500UL)
#define mainTIMER_SEND_FREQUENCY_MS pdMS_TO_TICKS(1000UL)

/* The number of items the queue can hold at once. */
#define mainQUEUE_LENGTH (2)

/* The values sent to the queue receive task from the queue send task and the
queue send software timer respectively. */
#define mainVALUE_SENT_FROM_TASK (100UL)
#define mainVALUE_SENT_FROM_TIMER (200UL)

/*
 * Prototypes for the standard FreeRTOS application hook (callback) functions
 * implemented within this file.  See http://www.freertos.org/a00016.html .
 */
void vApplicationMallocFailedHook(void);
void vApplicationIdleHook(void);
void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName);
void vApplicationTickHook(void);
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize);
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize);

void traceOnEnter();

/* When configSUPPORT_STATIC_ALLOCATION is set to 1 the application writer can
use a callback function to optionally provide the memory required by the idle
and timer tasks.  This is the stack that will be used by the timer task.  It is
declared here, as a global, so it can be checked by a test that is implemented
in a different file. */
StackType_t uxTimerTaskStack[configTIMER_TASK_STACK_DEPTH];
/*
 * The tasks as described in the comments at the top of this file.
 */
static void prvQueueReceiveTask(void *pvParameters);
static void prvQueueSendTask(void *pvParameters);

/*
 * The callback function executed when the software timer expires.
 */
static void prvQueueSendTimerCallback(TimerHandle_t xTimerHandle);

/* The queue used by both tasks. */
static QueueHandle_t xQueue = NULL;

/* A software timer that is started from the tick hook. */
static TimerHandle_t xTimer = NULL;

int main(void) {
    console_init();

    console_print("Starting echo blinky demo\n");

    const TickType_t xTimerPeriod = mainTIMER_SEND_FREQUENCY_MS;

    /* Create the queue. */
    xQueue = xQueueCreate(mainQUEUE_LENGTH, sizeof(uint32_t));

    if (xQueue != NULL) {
        /* Start the two tasks as described in the comments at the top of this
        file. */
        xTaskCreate(
            prvQueueReceiveTask, /* The function that implements the task. */
            "Rx", /* The text name assigned to the task - for debug only as it
                     is not used by the kernel. */
            configMINIMAL_STACK_SIZE, /* The size of the stack to allocate to
                                         the task. */
            NULL, /* The parameter passed to the task - not used in this simple
                     case. */
            mainQUEUE_RECEIVE_TASK_PRIORITY, /* The priority assigned to the
                                                task. */
            NULL); /* The task handle is not required, so NULL is passed. */

        xTaskCreate(prvQueueSendTask, "TX", configMINIMAL_STACK_SIZE, NULL,
                    mainQUEUE_SEND_TASK_PRIORITY, NULL);

        /* Create the software timer, but don't start it yet. */
        xTimer = xTimerCreate(
            "Timer", /* The text name assigned to the software timer - for debug
                        only as it is not used by the kernel. */
            xTimerPeriod, /* The period of the software timer in ticks. */
            1,            /* xAutoReload is set to pdTRUE. */
            NULL,         /* The timer's ID is not used. */
            prvQueueSendTimerCallback); /* The function executed when the timer
                                           expires. */

        if (xTimer != NULL) {
            xTimerStart(xTimer, 0);
        }

        /* Start the tasks and timer running. */
        vTaskStartScheduler();
    }

    for (;;)
        ;

    return 0;
}
/*-----------------------------------------------------------*/

static void prvQueueSendTask(void *pvParameters)
{
    TickType_t xNextWakeTime;
    const TickType_t xBlockTime = mainTASK_SEND_FREQUENCY_MS;
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TASK;

    /* Prevent the compiler warning about the unused parameter. */
    (void)pvParameters;

    /* Initialise xNextWakeTime - this only needs to be done once. */
    xNextWakeTime = xTaskGetTickCount();

    for (;;) {
        /* Place this task in the blocked state until it is time to run again.
        The block time is specified in ticks, pdMS_TO_TICKS() was used to
        convert a time specified in milliseconds into a time specified in ticks.
        While in the Blocked state this task will not consume any CPU time. */
        vTaskDelayUntil(&xNextWakeTime, xBlockTime);

        /* Send to the queue - causing the queue receive task to unblock and
        write to the console.  0 is used as the block time so the send operation
        will not block - it shouldn't need to block as the queue should always
        have at least one space at this point in the code. */
        xQueueSend(xQueue, &ulValueToSend, 0U);
    }
}
/*-----------------------------------------------------------*/

static void prvQueueSendTimerCallback(TimerHandle_t xTimerHandle)
{
    const uint32_t ulValueToSend = mainVALUE_SENT_FROM_TIMER;

    /* This is the software timer callback function.  The software timer has a
    period of two seconds and is reset each time a key is pressed.  This
    callback function will execute if the timer expires, which will only happen
    if a key is not pressed for two seconds. */

    /* Avoid compiler warnings resulting from the unused parameter. */
    (void)xTimerHandle;

    /* Send to the queue - causing the queue receive task to unblock and
    write out a message.  This function is called from the timer/daemon task, so
    must not block.  Hence the block time is set to 0. */
    xQueueSend(xQueue, &ulValueToSend, 0U);
}
/*-----------------------------------------------------------*/

static void prvQueueReceiveTask(void *pvParameters)
{
    uint32_t ulReceivedValue;

    /* Prevent the compiler warning about the unused parameter. */
    (void)pvParameters;

    for (;;) {
        /* Wait until something arrives in the queue - this task will block
        indefinitely provided INCLUDE_vTaskSuspend is set to 1 in
        FreeRTOSConfig.h.  It will not use any CPU time while it is in the
        Blocked state. */
        xQueueReceive(xQueue, &ulReceivedValue, portMAX_DELAY);

        /* To get here something must have been received from the queue, but
        is it an expected value?  Normally calling printf() from a task is not
        a good idea.  Here there is lots of stack space and only one task is
        using console IO so it is ok.  However, note the comments at the top of
        this file about the risks of making Linux system calls (such as
        console output) from a FreeRTOS task. */
        if (ulReceivedValue == mainVALUE_SENT_FROM_TASK) {
            console_print("Message received from task\n");
        } else if (ulReceivedValue == mainVALUE_SENT_FROM_TIMER) {
            console_print("Message received from software timer\n");
        } else {
            console_print("Unexpected message\n");
        }
    }
}
/*-----------------------------------------------------------*/

void vApplicationMallocFailedHook(void) {
    /* vApplicationMallocFailedHook() will only be called if
    configUSE_MALLOC_FAILED_HOOK is set to 1 in FreeRTOSConfig.h.  It is a hook
    function that will get called if a call to pvPortMalloc() fails.
    pvPortMalloc() is called internally by the kernel whenever a task, queue,
    timer or semaphore is created.  It is also called by various parts of the
    demo application.  If heap_1.c, heap_2.c or heap_4.c is being used, then the
    size of the    heap available to pvPortMalloc() is defined by
    configTOTAL_HEAP_SIZE in FreeRTOSConfig.h, and the xPortGetFreeHeapSize()
    API function can be used to query the size of free heap space that remains
    (although it does not provide information on how the remaining heap might be
    fragmented).  See http://www.freertos.org/a00111.html for more
    information. */
    vAssertCalled(__FILE__, __LINE__);
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook(void) {
    /* vApplicationIdleHook() will only be called if configUSE_IDLE_HOOK is set
    to 1 in FreeRTOSConfig.h.  It will be called on each iteration of the idle
    task.  It is essential that code added to this hook function never attempts
    to block in any way (for example, call xQueueReceive() with a block time
    specified, or call vTaskDelay()).  If application tasks make use of the
    vTaskDelete() API function to delete themselves then it is also important
    that vApplicationIdleHook() is permitted to return to its calling function,
    because it is the responsibility of the idle task to clean up memory
    allocated by the kernel to any task that has since deleted itself. */

    usleep(15000);
    traceOnEnter();
}
/*-----------------------------------------------------------*/

void vApplicationStackOverflowHook(TaskHandle_t pxTask, char *pcTaskName) {
    (void)pcTaskName;
    (void)pxTask;

    /* Run time stack overflow checking is performed if
    configCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
    function is called if a stack overflow is detected.  This function is
    provided as an example only as stack overflow checking does not function
    when running the FreeRTOS POSIX port. */
    vAssertCalled(__FILE__, __LINE__);
}
/*-----------------------------------------------------------*/

void vApplicationTickHook(void) {
    /* This function will be called by each tick interrupt if
    configUSE_TICK_HOOK is set to 1 in FreeRTOSConfig.h.  User code can be
    added here, but the tick hook is called from an interrupt context, so
    code must not attempt to block, and only the interrupt safe FreeRTOS API
    functions can be used (those that end in FromISR()). */
}

void traceOnEnter() {
    int ret;
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    ret = select(1, &fds, NULL, NULL, &tv);
    if (ret > 0) {
        /* clear the buffer */
        char buffer[200];
        read(1, &buffer, 200);
    }
}

void vLoggingPrintf(const char *pcFormat, ...) {
    va_list arg;

    va_start(arg, pcFormat);
    vprintf(pcFormat, arg);
    va_end(arg);
}
/*-----------------------------------------------------------*/

void vApplicationDaemonTaskStartupHook(void) {
    /* This function will be called once only, when the daemon task starts to
    execute    (sometimes called the timer task).  This is useful if the
    application includes initialisation code that would benefit from executing
    after the scheduler has been started. */
}
/*-----------------------------------------------------------*/

void vAssertCalled(const char *const pcFileName, unsigned long ulLine) {
    static BaseType_t xPrinted = pdFALSE;
    volatile uint32_t ulSetToNonZeroInDebuggerToContinue = 0;

    /* Called if an assertion passed to configASSERT() fails.  See
    http://www.freertos.org/a00110.html#configASSERT for more information. */

    /* Parameters are not used. */
    (void)ulLine;
    (void)pcFileName;

    taskENTER_CRITICAL();
    {
        /* Stop the trace recording. */
        if (xPrinted == pdFALSE) {
            xPrinted = pdTRUE;
        }

        /* You can step out of this function to debug the assertion by using
        the debugger to set ulSetToNonZeroInDebuggerToContinue to a non-zero
        value. */
        while (ulSetToNonZeroInDebuggerToContinue == 0) {
            __asm volatile("NOP");
            __asm volatile("NOP");
        }
    }
    taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION is set to 1, so the application must provide an
implementation of vApplicationGetIdleTaskMemory() to provide the memory that is
used by the Idle task. */
void vApplicationGetIdleTaskMemory(StaticTask_t **ppxIdleTaskTCBBuffer,
                                   StackType_t **ppxIdleTaskStackBuffer,
                                   uint32_t *pulIdleTaskStackSize) {
    /* If the buffers to be provided to the Idle task are declared inside this
    function then they must be declared static - otherwise they will be
    allocated on the stack and so not exists after this function exits. */
    static StaticTask_t xIdleTaskTCB;
    static StackType_t uxIdleTaskStack[configMINIMAL_STACK_SIZE];

    /* Pass out a pointer to the StaticTask_t structure in which the Idle task's
    state will be stored. */
    *ppxIdleTaskTCBBuffer = &xIdleTaskTCB;

    /* Pass out the array that will be used as the Idle task's stack. */
    *ppxIdleTaskStackBuffer = uxIdleTaskStack;

    /* Pass out the size of the array pointed to by *ppxIdleTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulIdleTaskStackSize = configMINIMAL_STACK_SIZE;
}
/*-----------------------------------------------------------*/

/* configUSE_STATIC_ALLOCATION and configUSE_TIMERS are both set to 1, so the
application must provide an implementation of vApplicationGetTimerTaskMemory()
to provide the memory that is used by the Timer service task. */
void vApplicationGetTimerTaskMemory(StaticTask_t **ppxTimerTaskTCBBuffer,
                                    StackType_t **ppxTimerTaskStackBuffer,
                                    uint32_t *pulTimerTaskStackSize) {
    /* If the buffers to be provided to the Timer task are declared inside this
    function then they must be declared static - otherwise they will be
    allocated on the stack and so not exists after this function exits. */
    static StaticTask_t xTimerTaskTCB;

    /* Pass out a pointer to the StaticTask_t structure in which the Timer
    task's state will be stored. */
    *ppxTimerTaskTCBBuffer = &xTimerTaskTCB;

    /* Pass out the array that will be used as the Timer task's stack. */
    *ppxTimerTaskStackBuffer = uxTimerTaskStack;

    /* Pass out the size of the array pointed to by *ppxTimerTaskStackBuffer.
    Note that, as the array is necessarily of type StackType_t,
    configMINIMAL_STACK_SIZE is specified in words, not bytes. */
    *pulTimerTaskStackSize = configTIMER_TASK_STACK_DEPTH;
}
