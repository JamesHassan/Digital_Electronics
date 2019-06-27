/*
  @brief Reads the ADC
  @author James Hassan 11991559
  @date 2019-06-20
*/

// #define I2S_ADC_UNIT ADC_UNIT_1
// #define I2S_ADC_CHANNEL ADC1_CHANNEL_0
#include "TempSense.h"

#define AddressStart 0x3F400001
#define AddressEnd 0x3F7FFFFF
//Address location: 8388608
static uint32_t WriteBuff[SPI_FLASH_SEC_SIZE / 4];
static uint32_t ReadBuff[SPI_FLASH_SEC_SIZE / 4];

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
  save_data((uint32_t)temperatureAve);
}

static void save_data(uint32_t Average)
{
  /*
  // uint32_t AddressCurrent = AddressStart;
  uint32_t AddressCurrent[1024];
  AddressCurrent[0] = AddressStart;
  // int AddressSize = sizeof(AddressCurrent);
  uint32_t srcAddr = 0x3F400000;
  // The location of the address of the data is stored in 0x3F80_000 = 8388608
  //ESP_ERROR_CHECK(spi_flash_read(srcAddr, AddressCurrent,sizeof(AddressCurrent)));
  spi_flash_read(srcAddr, AddressCurrent,sizeof(AddressCurrent));
  // Advance the memory location pointer
  if (AddressCurrent[0] != AddressEnd)
    AddressCurrent[0] += 1;
  else
    AddressCurrent[0] = AddressStart;

    printf("Adress: %d\n",AddressCurrent[0]);
  // ESP_ERROR_CHECK(spi_flash_write(AddressCurrent,&Average,sizeof(Average)));
}
*/
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
    int32_t restart_counter = 0; // value will default to 0, if not set yet in NVS
    err = nvs_get_i32(my_handle, "restart_counter", &restart_counter);
    switch (err) 
    {
      case ESP_OK:
        printf("Done\n");
        printf("Restart counter = %d\n", restart_counter);
        break;
      case ESP_ERR_NVS_NOT_FOUND:
        printf("The value is not initialized yet!\n");
        break;
      default :
        printf("Error (%s) reading!\n", esp_err_to_name(err));
    }

    // Write
    printf("Updating restart counter in NVS ... ");
    restart_counter++;
    err = nvs_set_i32(my_handle, "restart_counter", restart_counter);
    printf((err != ESP_OK) ? "Failed!\n" : "Done\n");

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