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
  temperatureAve = ((double)vAve - 500)/10;
  double tmpMedian = (double)highest - (double)lowest;
  tmpMedian /= 2;
  temperatureMed = ((lowest+tmpMedian) - 500)/10;
  printf("Median Temp: %.2f*C\n", temperatureMed);
  printf("Average Temp: %.2f*C\n", temperatureAve);
  //Save Avg to NVS

  save_data((uint32_t)(temperatureAve*100));

  // xQueueSendFromISR(data_queue, &temperatureAve, NULL);

}

// void save_data(void* arg)
void save_data(uint32_t Average)
{
  int err;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) 
  {
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else 
  {
    uint32_t currAddr = 0;
    uint32_t baseAddr = 0;
    char baseStr[32];
    // convert address space to string
    itoa(baseAddr, baseStr, 10);
    err = nvs_get_u32(my_handle,baseStr, &currAddr);

    uint32_t tempMem = currAddr;
    
    int sector = tempMem & 0b11111111;
    tempMem = currAddr;
    int block = (tempMem & 0b1111111100000000) >> 8 ;
    // printf("block: %d\n", block);
    // printf("sector: %d\n", sector);

    if(sector < MAX_SECTOR_NUM)
    {
      sector++;
    }
    else
    {
      sector = 0;
      block++;
    }
    if ((sector == MAX_SECTOR_NUM) && (block == MAX_BLOCK_NUM))
    {
      block = 0;
      sector = 0;
    }
    currAddr = (block << 8) + sector;
    // printf("\n");
    // printf("block: %d\n", block);
    // printf("sector: %d\n", sector);
    // printf("tempMem: %d\n", currAddr);
    // tempMem = currAddr;

    char memStr[32];
    itoa(currAddr, memStr, 10);

    err = nvs_set_u32(my_handle,memStr, Average);
    printf((err != ESP_OK) ? "Data Failed!\n" : "Data Set!\n");

    err = nvs_set_u32(my_handle,baseStr, currAddr);
    printf((err != ESP_OK) ? "Memory Location Failed!\n" : "Memory Location Set!\n");

    // printf("Location of Memory: %d\n", currAddr);
    printf("Data: %d\n", Average);
    printf("Committing updates in NVS ... ");
    err = nvs_commit(my_handle);
    printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
    // Close
    nvs_close(my_handle);
  }
}

void display_data()
{
  int err;
  err = nvs_open("storage", NVS_READONLY, &my_handle);
  if (err != ESP_OK)
    {
      printf("Error Opening Handle\n");
    }
  else
  {
    // get the maxiumum address value for the print out of the data
    uint32_t baseAddr = 0;
    char baseStr[32];
    itoa(baseAddr, baseStr, 10);

    uint32_t maxAddress;
    nvs_get_u32(my_handle,baseStr,&maxAddress);
    uint32_t tempAddr = maxAddress >> 8;
    maxAddress = tempAddr + (maxAddress * 0b11111111);
    printf("Max Address: %d\n",maxAddress);
    uint32_t tempOut;
    uint32_t currAddr = 1;
    char currAddrStr[32];

    for (uint32_t i = 0; i < maxAddress;i++)
    {
      itoa(currAddr, currAddrStr, 10);
      err = nvs_get_u32(my_handle,currAddrStr,&tempOut);
      if (err != ESP_OK)
      {
        i = maxAddress;
      }
      else
      {
        // Find the next Address to print
        uint32_t tempMem = currAddr;

        int sector = tempMem & 0b11111111;
        tempMem = currAddr;
        int block = (tempMem & 0b1111111100000000) >> 8 ;
        // printf("block: %d\n", block);
        // printf("sector: %d\n", sector);
        if(sector < MAX_SECTOR_NUM)
        {
          sector++;
        }
        else
        {
          sector = 0;
          block++;
        }
        if ((sector == MAX_SECTOR_NUM) && (block == MAX_BLOCK_NUM))
        {
          i = maxAddress;
        }
        currAddr = (block << 8) + sector;
      
        printf("Minute %d: %d\n",i,tempOut);
      }
    }
    nvs_close(my_handle);
  }
}

