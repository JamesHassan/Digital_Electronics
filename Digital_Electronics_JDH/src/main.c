/*
   @brief the main module for the PAV project.
   @author James Hassan 11991559
   @date 2019-06-04
*/

#include <stdio.h>
#include "freertos/FreeRTOS.h"  //FreeRTOS library
#include "freertos/task.h"      // "freertos/FreeRTOS.h" MUST apear before this!
#include "freertos/queue.h"

#include "esp_adc_cal.h"
#include "esp_system.h"
#include "esp_spi_flash.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_partition.h"

#include "driver/rtc_cntl.h"
#include "soc/rtc_cntl_reg.h"
#include "soc/soc.h"
#include "driver/rtc_io.h"

#include "esp_timer.h"

#include "rfc_int.h"
#include "driver/adc.h"
#include "driver/gpio.h"
#include "Timer.h"
#include "TempSense.h"

#include "locale.h"

// 60 second timer
timers_t timer00 = 
{
  .timer_group = TIMER_GROUP_0,
  .timer_num = TIMER_0,
  .timer_config.alarm_en = TIMER_ALARM_EN,      /*!< Timer alarm enable */
  .timer_config.counter_en = TIMER_PAUSE,    /*!< Counter enable */
  .timer_config.intr_type = TIMER_INTR_LEVEL, /*!< Interrupt mode */
  // .timer_config.counter_dir = TIMER_COUNT_DOWN, /*!< Counter direction  */
  .timer_config.counter_dir = TIMER_COUNT_UP, /*!< Counter direction  */
  .timer_config.auto_reload = TIMER_AUTORELOAD_EN,   /*!< Timer auto-reload */
  .timer_config.divider = TIMER_DIVDER,
  .period = 10, // 60s timer
};


static void setup()
{
  timer_queue = xQueueCreate(10, sizeof(timers_t));
  Timer_Init(timer00);
  setlocale(LC_ALL, "");


}

void app_main()
{
  setup();

  // xTaskCreate(adc_read_task, "ADC read task", 2048, NULL, 5, NULL);    
  xTaskCreate(timer0_evt,"timer0_evt", 2048, NULL, 5, NULL);

  // for(;;)
  // {
  //   vTaskDelay(200 / portTICK_RATE_MS);
  //   printf("Hello World\n");
  //   vTaskDelay(200 / portTICK_RATE_MS);
  //   printf("This is me!\n");

  // }
}