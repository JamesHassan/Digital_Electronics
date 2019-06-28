/*
  @brief Reads the ADC, calculates temperatures, saves data to Non-Volitile Storage, and prints out all data  @author James Hassan 11991559
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

#include <string.h>

#define I2S_ADC_UNIT ADC_UNIT_1
#define I2S_ADC_CHANNEL ADC1_CHANNEL_0

xQueueHandle data_queue;
 
/* @brief Reads the ADC to get the sample, then calls the 
sava_data function to save the calculated temperature.
@param void* arg is a pointer to any arguments passed in.*/
void adc_read_task(void* arg);

/* @brief Takes the Temperature average and saves it to Non-Volitile Storage (NVS),
then saves the reading and the time stamp into the lowest address space of the stack.
@param uint32_t Average is the temperature reading from the ADC.*/
void save_data(uint32_t Average);

/* @brief prints out all the data on the correct string being accepted.*/
void display_data();

#endif