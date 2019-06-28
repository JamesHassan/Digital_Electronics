/*
  @brief Reads the ADC
  @author James Hassan 11991559
  @date 2019-06-20
*/

#include "TempSense.h"

#define AddressStart 0xEF401000
#define AddressEnd 0xEF7FFFFF

#define MAX_SECTOR_NUM 7
#define MAX_BLOCK_NUM 127

#define MAX_ADDRESS 720 // FOR 12 HOUR WRAP AROUND

nvs_handle my_handle;

spi_flash_mmap_memory_t memory;
spi_flash_mmap_handle_t mmapHandle;

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
  temperatureAve = ((double)vAve - 500)/10; // Removed an extra 50mV due to noise of the lines
  double tmpMedian = (double)highest - (double)lowest;
  tmpMedian /= 2;
  temperatureMed = ((lowest+tmpMedian) - 500)/10;
  // printf("Median Temp: %.2f*C\n", temperatureMed);
  // printf("Average Temp: %.2f*C\n", temperatureAve);
  //Save Avg to NVS
  save_data((uint32_t)(temperatureAve*100));
}

static inline bool error_check(int err, char messFail[80], char messSuccess[80])
{
  printf((err != ESP_OK) ? messFail : messSuccess);

  if (err != ESP_OK)
  {
    // printf(messFail);
    if (err == ESP_ERR_NVS_NOT_ENOUGH_SPACE)
      nvs_erase_all(my_handle);
    // ESP_ERROR_CHECK(err);
    return false;
  }
  else
    return true;
}

// void save_data(void* arg)
void save_data(uint32_t Average)
{
  int err;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if(error_check(err, "Error (%s) opening NVS handle!\n",""))
  {
    uint32_t currAddr = 1;
    uint32_t baseAddr = 0;
    char baseStr[32];
    // convert address space to string
    itoa(baseAddr, baseStr, 10);
    err = nvs_get_u32(my_handle,baseStr, &currAddr);

    // printf("Base Address: %d\n",baseAddr);
    // printf("Current Address: %d\n",currAddr);
    error_check(err, "Current Address Retrival Failed!\n","");

    if (currAddr < MAX_ADDRESS)
      currAddr++;
    else
      currAddr = 1;

    char memStr[32];
    itoa(currAddr, memStr, 10);
    // printf("Current Address: %d\n",currAddr);

    err = nvs_set_u32(my_handle,baseStr, currAddr);
    error_check(err, "Memory Location Failed!\n","");

    uint32_t timeCode = currAddr << 16;
    uint32_t data = timeCode + Average;

    err = nvs_set_u32(my_handle,memStr, data);
    error_check(err, "Data Failed!\n","");

    // uint32_t temp = data;
    // uint32_t hour = (temp >> 16)/60;//(tempAddr/60) << 24;
    // temp = data;
    // uint32_t minute = (temp >> 16)%60;
    // uint32_t Average = temp & 0b1111111111111111;
    // printf("Hour: %d, Minute: %d, Average Temperature: %d\n", hour, minute, Average);
    // printf("Location of Memory: %d\n", currAddr);

    // printf("Data: %d\n", data);
    // printf("Committing updates in NVS ... ");
    err = nvs_commit(my_handle);
    printf((err != ESP_OK) ? "Failed!\n" : "");
    // Close
    nvs_close(my_handle);
    // display_data();
  }
}

void display_data()
{
  while(1)
  {
  char str [80] = "";
  char desired [9] = "Download";
  printf("To see some data, please enter: 'Download': ");
  scanf("%s",str); //If you'd like to see the last 12 hours of data, 
  printf("%s\n",str); 
  vTaskDelay(10000 / portTICK_RATE_MS);
  printf("%s\n",str);
  if ((strcmp(str,desired) == 0) || (strcmp(str,"Download\n") == 0) || (strcmp(str,"Download\r\n") == 0))
  {
    printf("SUccess!!!\n");
    int err;
    err = nvs_open("storage", NVS_READWRITE, &my_handle);
    if(error_check(err, "Error (%s) opening NVS handle!\n","Download Sucess!\n"))
    {
      uint32_t tempOut;
      // get the maxiumum address value for the print out of the data
      uint32_t baseAddr = 0;
      char baseStr[32];
      uint32_t latestData;
      itoa(baseAddr, baseStr, 10);
      err = nvs_get_u32(my_handle,baseStr,&latestData);
      if(error_check(err, "Data Retrieval Failed!\n",""))
      {}
      // uint32_t currAddr = 1;
      char currAddrStr[32];
      uint32_t hour;
      uint32_t minute;
      uint32_t Average;

      for (uint32_t i = 1; i <= latestData;i++)
      {
        itoa(i, currAddrStr, 10);
        err = nvs_get_u32(my_handle,currAddrStr,&tempOut);
        uint32_t temp = tempOut;
        hour = (temp >> 16)/60;//(tempAddr/60) << 24;
        temp = tempOut;
        minute = (temp >> 16)%60;
        printf("Hour: %d, Minute: %d, ", hour, minute);

        if(error_check(err, "",""))
        {
          Average = tempOut & 0b1111111111111111;
          double avg = ((double)Average)/100;
          printf("Average Temperature: %.2f*C\n",avg);
        }
        else
        {
          printf("No data or Failed\n");
        }
      }
      nvs_close(my_handle);
    }
  }
  else if (strcmp(str,"") == 0){}
  else
    printf("Incorrect! Please enter the correct string: Download\n");
}
}