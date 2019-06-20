/*
  @brief to control the inbuilt timers to influence functions periodically
  @author James Hassan 11991559
  @date 2019-04-05
*/

#include "Timer.h"
// #include "main.c"

void IRAM_ATTR timer0_isr(void *arg)
{
  int timer_num = (int)arg;
  timers_t evt;
  evt.timer_num = timer_num;
  evt.timer_group = 0;

  uint32_t intr_status = TIMERG0.int_st_timers.val;

  if((intr_status & BIT(timer_num)))
  {
    TIMERG0.hw_timer[timer_num].update = 1;
    if (timer_num == 0)
      TIMERG0.int_clr_timers.t0 = 1;
    else if (timer_num == 1)
      TIMERG0.int_clr_timers.t1 = 1;
    TIMERG0.hw_timer[timer_num].config.alarm_en = 1;
  }
  intr_status = TIMERG0.int_st_timers.val;
  xQueueSendFromISR(timer_queue, &evt, NULL);
}

//Init
bool Timer_Init(timers_t timerGN) //,void (*usrFn)(void*), void* usrArg, xQueueHandle timer_queue)
{
  //int err
  // Config
  ESP_ERROR_CHECK(timer_init(timerGN.timer_group, timerGN.timer_num, &timerGN.timer_config));

  printf("Timer_Val == %d\n",(int)(timerGN.period*TIMER_SCALE));
  // Timer Pause
  ESP_ERROR_CHECK(timer_pause(timerGN.timer_group, timerGN.timer_num));

  /****Counter DOWN*****/

  // err = timer_set_counter_value(timerGN.timer_group, timerGN.timer_num, (uint64_t)(timerGN.period*TIMER_SCALE));
  // printf("timer_set_counter_value == %d\n",err);

  // err = timer_set_alarm_value(timerGN.timer_group, timerGN.timer_num, 0);
  // printf("timer_set_alarm_value == %d\n",err);
  /*********************/
  /*****Counter UP******/
  // Set counter value
  ESP_ERROR_CHECK(timer_set_counter_value(timerGN.timer_group, timerGN.timer_num, 0));
  // Set Alarm Value
  ESP_ERROR_CHECK(timer_set_alarm_value(timerGN.timer_group, timerGN.timer_num, (uint64_t)(timerGN.period*TIMER_SCALE)));
  /*********************/
  // Enable interupt
  ESP_ERROR_CHECK(timer_enable_intr(timerGN.timer_group,timerGN.timer_num));
  // Set ISR
    ESP_ERROR_CHECK(timer_isr_register(timerGN.timer_group,timerGN.timer_num,timer0_isr,(void *) timerGN.timer_num,ESP_INTR_FLAG_IRAM, NULL));
  //Timer START!
  ESP_ERROR_CHECK(timer_start(timerGN.timer_group, timerGN.timer_num));

  return true;
}

void timer0_evt(void *arg)
{
  while (1) {
    timers_t tempTimer;

    if (xQueueReceive(timer_queue, &tempTimer, portMAX_DELAY))
    {
      // /* Print which timer and their values */
      // printf("---- Timer Event ----\n");
      // printf("Group[%d], timer[%d]\n", tempTimer.timer_group, tempTimer.timer_num);
      // uint64_t task_counter_value;
      // timer_get_counter_value(tempTimer.timer_group, tempTimer.timer_num, &task_counter_value);
      // printf("%d\n",(int)task_counter_value);

      if (tempTimer.timer_num == 0)
      {
        // Read ADC
        adc_read_task(NULL);
      }
      else if (tempTimer.timer_num == 1)
      {
        /*
        // For checking jitter
        int level = gpio_get_level(GPIO_NUM_16);
        if (level)
            gpio_set_level(GPIO_NUM_16, 0);
        else
            gpio_set_level(GPIO_NUM_16, 1);
        */
      }
    }
  }
}