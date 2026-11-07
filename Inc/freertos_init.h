#ifndef FREERTOS_INIT_H_
#define FREERTOS_INIT_H_

/*
 * Creates the initial FreeRTOS task and starts the scheduler.
 *
 * This is intentionally minimal: it only proves that the kernel, port and
 * heap are configured correctly. The parking application logic is not
 * migrated to tasks yet.
 *
 * Does not return when the scheduler starts successfully.
 */
void FreeRTOS_Init(void);

#endif /* FREERTOS_INIT_H_ */
