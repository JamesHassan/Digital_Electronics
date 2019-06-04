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

#include "driver/adc.h"

void app_main()
{
  for(;;)
  {
    vTaskDelay(200 / portTICK_RATE_MS);
    printf("Hello World\n");
    vTaskDelay(200 / portTICK_RATE_MS);
    printf("This is me!\n");

  }


}