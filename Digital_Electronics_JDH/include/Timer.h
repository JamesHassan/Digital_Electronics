/*
  @brief to control the inbuilt timers to influence functions periodically
  @author James Hassan 11991559
  @date 2019-04-05
*/

#ifndef Timer_H
#define Timer_H

#include <stdio.h>
#include "stdint.h"
#include "stdlib.h"
#include "stdbool.h"
#include <types.h>

#include "esp_system.h"
#include "esp_types.h"
#include "esp_attr.h"

#include "driver/timer.h"
#include "soc/timer_group_reg.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "soc/timer_group_struct.h"
#include "driver/periph_ctrl.h"

#include "driver/gpio.h"

#include "TempSense.h"

typedef struct  {
  timer_group_t timer_group;
  timer_idx_t timer_num;
  timer_config_t timer_config;
  timer_isr_handle_t timer_intr;
  double period;
} timers_t;

// timer defines
#define MAX_NUM_TIMERS 4
#define TIMER_DIVDER 8000   // to convert to 100uS from an 80MHz clk
#define TIMER_SCALE (TIMER_BASE_CLK/TIMER_DIVDER) // 

xQueueHandle timer_queue;

/* @brief The ISR used for resetting the timer and signalling events.
@param void* arg is a pointer to any arguments passed in.*/
void IRAM_ATTR timer0_isr(void *arg);

/* @brief Sets up the required functions and modes for any Timer function.
@param timers_t timerGN is the number of the timer to be initialized 
*Note: this is only the timer number, not the group as I'm only using group 0 */
bool Timer_Init(timers_t timerGN);

/* @brief The event handler for do something when a timer's time has elasped.
@param void* arg is a pointer to any arguments passed in.*/
void timer0_evt(void *arg);

#endif