/*
  @brief Reads the ADC
  @author James Hassan 11991559
  @date 2019-06-20
*/

#ifndef TempSense_H
#define TempSense_H

#include <stdio.h>
#include "freertos/FreeRTOS.h"  //FreeRTOS library
#include "freertos/task.h"      // "freertos/FreeRTOS.h" MUST apear before this!
#include "freertos/queue.h"
#include "esp_err.h"
#include "esp_log.h"

#include "driver/adc.h"
#include "esp_adc_cal.h"
#include "driver/gpio.h"
#include "esp_spi_flash.h"
#include "rom/spi_flash.h"

#include "nvs_flash.h"
#include "nvs.h"

#define I2S_ADC_UNIT ADC_UNIT_1
#define I2S_ADC_CHANNEL ADC1_CHANNEL_0
// Comments
static void save_data(uint32_t Average);

//Comments
void adc_read_task(void* arg);

#endif