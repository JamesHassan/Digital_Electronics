/*
  @brief Reads the ADC
  @author James Hassan 11991559
  @date 2019-06-20
*/

// #define I2S_ADC_UNIT ADC_UNIT_1
// #define I2S_ADC_CHANNEL ADC1_CHANNEL_0
#include "TempSense.h"

void adc_read_task(void* arg)
{
	adc1_config_width(ADC_WIDTH_12Bit);
	adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_11db);
	esp_adc_cal_characteristics_t characteristics;
	esp_adc_cal_characterize(I2S_ADC_UNIT, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 3300, &characteristics);

  uint32_t voltage;
  double temperatureAve;
  double temperatureMed;
  uint32_t vAve = 0;
  int samples = 20;
  uint32_t lowest = 3300;
  uint32_t highest = 0;

  for(int i=0; i<samples; i++)
  {  
    esp_adc_cal_get_voltage(I2S_ADC_CHANNEL, &characteristics, &voltage);
    // printf("Raw: %dmV\n", voltage);
    if (voltage < lowest)
    {
      lowest = voltage;
      // printf("Lowest: %dmV\n", lowest);

    }
    if (voltage > highest)
    {
      highest = voltage;
      // printf("Highest: %dmV\n", highest);
    }
    vAve += voltage;
  }

  vAve /= samples;
  temperatureAve = ((double)vAve - 500)/10;
  double tmpMedian = (double)highest - (double)lowest;
  tmpMedian /= 2;
  temperatureMed = ((lowest+tmpMedian) - 500)/10;
  printf("Median Temp: %.2f*C\n", temperatureMed);
  printf("Average Temp: %.2f*C\n", temperatureAve);
}
