/*
  @brief Reads the ADC
  @author James Hassan 11991559
  @date 2019-06-20
*/

// #define I2S_ADC_UNIT ADC_UNIT_1
// #define I2S_ADC_CHANNEL ADC1_CHANNEL_0
#include "TempSense.h"

#define AddressStart 0xEF401000
#define AddressEnd 0xEF7FFFFF

#define MAX_SECTOR_NUM 8
#define MAX_BLOCK_NUM 128

spi_flash_mmap_memory_t memory;
spi_flash_mmap_handle_t mmapHandle;

//Address location: 8388608
// static uint32_t WriteBuff[SPI_FLASH_SEC_SIZE / 4];
// static uint32_t ReadBuff[SPI_FLASH_SEC_SIZE / 4];

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
      printf("Lowest: %dmV\n", lowest);

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
/*
  // if(xQueueReceive(data_queue, &arg, portMAX_DELAY))
  // {
    // uint32_t Average = (uint32_t)&arg; 
    uint32_t Average = arg; 

    // uint32_t AddressCurrent = AddressStart;
    // uint32_t AddressCurrent[1024];
    // AddressCurrent[0] = AddressStart;
    // uint32_t base_addr = 0x200000;

    uint32_t curAddr = AddressStart;
    uint32_t srcAddr = 0xEF400000;
    spi_flash_op_block_func;
    esp_ipc_call;
  
    uint32_t free_mem = spi_flash_mmap_get_free_pages(memory);
    printf("Free Pages: %d\n", free_mem);
    ESP_ERROR_CHECK(spi_flash_mmap(srcAddr, sizeof(srcAddr), memory,curAddr, &mmapHandle));

    // The location of the address of the data is stored in 0x3F80_000 = 8388608
    // ESP_ERROR_CHECK(spi_flash_read(srcAddr, &curAddr,sizeof(curAddr)));
    // ESP_ERROR_CHECK(spi_flash_read(srcAddr, AddressCurrent,sizeof(AddressCurrent)));
    //spi_flash_read(base_addr, AddressCurrent,sizeof(AddressCurrent));
    // Advance the memory location pointer
    if (curAddr != AddressEnd)  //(AddressCurrent[0] != AddressEnd) || (base_addr != 0x300000))
    {
      curAddr += SPI_FLASH_SEC_SIZE;
    }
    else
    {
      curAddr = 0x3F400000 + SPI_FLASH_SEC_SIZE;
    }
    // printf("Adress: %d\n",AddressCurrent[0]);
    printf("Base Adress: %d\n",curAddr);

    // ESP_ERROR_CHECK(spi_flash_write(curAddr,&Average,sizeof(Average)));
    // ESP_ERROR_CHECK(spi_flash_read(srcAddr, &curAddr,sizeof(curAddr)));

}*/
// }


  int err;
  nvs_handle my_handle;
  err = nvs_open("storage", NVS_READWRITE, &my_handle);
  if (err != ESP_OK) 
  {
    printf("Error (%s) opening NVS handle!\n", esp_err_to_name(err));
  }
  else 
  {
    printf("Done\n");

    // Read
    printf("Reading restart counter from NVS ... ");
    uint32_t currAddr = 0;

    // sector

    uint32_t baseAddr = 0;
    char baseStr[32];
    // convert address space to string
    itoa(baseAddr, baseStr, 10);

    // uint32_t memoryOut = atoi(baseStr);
    
    err = nvs_set_u32(my_handle,baseStr, &currAddr);


    err = nvs_get_u32(my_handle,baseStr, &currAddr);
    uint32_t memoryLoc = currAddr;
    uint32_t tempMem = currAddr;
    
    int sector = tempMem & 0b11111111;
    int block = (memoryLoc & 0b1111111100000000) >> 8 ;
    printf("block: %d\n", block);
    printf("sector: %d\n", sector);

    if(sector < MAX_SECTOR_NUM)
    {
      sector++;
    }
    else if (sector == MAX_SECTOR_NUM)
    {
      sector = 0;
      block++;
    }
    if (block == MAX_BLOCK_NUM)
      block = 0;

    memoryLoc = (block << 8) + sector;
    printf("\n");
    printf("block: %d\n", block);
    printf("sector: %d\n", sector);
    printf("memoryLoc: %d\n", memoryLoc);

    char memStr[32];
    itoa(memoryLoc, memStr, 10);

    err = nvs_set_u32(my_handle,memStr, &Average);
    printf((err != ESP_OK) ? "Data Failed!\n" : "Done\n");

    err = nvs_set_u32(my_handle,baseStr, &currAddr);
    printf((err != ESP_OK) ? "Memory Location Failed!\n" : "Done\n");

    printf("Location of Memory: %d\n", memoryLoc);
    printf("Data: %d\n", Average);


/* 
    switch (err) 
    {
      case ESP_OK:
        printf("Done\n");
        printf("Restart counter = %d\n", currAddr);
        break;
      case ESP_ERR_NVS_NOT_FOUND:
        printf("The value is not initialized yet!\n");
        break;
      default :
        printf("Error (%s) reading!\n", esp_err_to_name(err));
    }


    // Write
    printf("Updating restart counter in NVS ... ");
    // currAddr++;
    err = nvs_set_u32(my_handle, baseStr, currAddr);

    printf((err != ESP_OK) ? "Failed!\n" : "Done\n");
*/

    // Commit written value.
    // After setting any values, nvs_commit() must be called to ensure changes are written
    // to flash storage. Implementations may write to storage at other times,
    // but this is not guaranteed.
    printf("Committing updates in NVS ... ");
    err = nvs_commit(my_handle);
    printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

    // Close
    nvs_close(my_handle);
  }

}
