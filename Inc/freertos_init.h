#ifndef FREERTOS_INIT_H_
#define FREERTOS_INIT_H_

/*
 * Creates the parking application tasks and starts the scheduler.
 *
 * The parking logic and each peripheral owner are separate FreeRTOS tasks,
 * communicating through the shared state protected by a mutex in tasks.c.
 *
 * Does not return when the scheduler starts successfully.
 */
void FreeRTOS_Init(void);

#endif /* FREERTOS_INIT_H_ */
