#ifndef _MICROPHONE_FFT_H
#define _MICROPHONE_FFT_H

#include "hardware.h"
#include <M5StickC.h>
#include <cinttypes>
#include <cmath>
#include <driver/i2s.h>

extern "C" {
#include "fft.h"
};

SemaphoreHandle_t xSemaphore = NULL;
SemaphoreHandle_t start_dis = NULL;
SemaphoreHandle_t start_fft = NULL;
TaskHandle_t xhandle_display = NULL;
TaskHandle_t xhandle_fft = NULL;
int8_t i2s_readraw_buff[1024];
uint8_t fft_dis_buff[161][80] = {0};
uint16_t posData = 160;

#define PIN_CLK 0
#define PIN_DATA 34


 constexpr unsigned char ImageData[768] = { 
 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x00 , 0x01 , 0x00 , 0x00 , 0x04 , 0x00 , 
 0x01 , 0x07 , 0x00 , 0x01 , 0x09 , 0x00 , 0x01 , 0x0D , 0x00 , 0x02 , 0x10 , 0x00 , 0x02 , 0x14 , 0x00 , 0x01 , 
 0x17 , 0x00 , 0x02 , 0x1C , 0x00 , 0x02 , 0x20 , 0x00 , 0x02 , 0x24 , 0x00 , 0x03 , 0x28 , 0x00 , 0x03 , 0x2D , 
 0x00 , 0x03 , 0x32 , 0x00 , 0x04 , 0x37 , 0x00 , 0x05 , 0x3C , 0x00 , 0x04 , 0x42 , 0x00 , 0x05 , 0x46 , 0x00 , 
 0x05 , 0x4D , 0x00 , 0x06 , 0x51 , 0x00 , 0x06 , 0x57 , 0x00 , 0x06 , 0x5D , 0x00 , 0x07 , 0x62 , 0x00 , 0x07 , 
 0x68 , 0x00 , 0x07 , 0x6E , 0x00 , 0x09 , 0x74 , 0x00 , 0x08 , 0x7A , 0x00 , 0x09 , 0x7F , 0x00 , 0x09 , 0x86 , 
 0x00 , 0x0A , 0x8B , 0x00 , 0x0A , 0x91 , 0x00 , 0x0B , 0x97 , 0x00 , 0x0B , 0x9D , 0x00 , 0x0C , 0xA2 , 0x00 , 
 0x0C , 0xA8 , 0x00 , 0x0C , 0xAD , 0x00 , 0x0D , 0xB2 , 0x00 , 0x0D , 0xB7 , 0x00 , 0x0E , 0xBC , 0x00 , 0x0E , 
 0xC2 , 0x00 , 0x0E , 0xC7 , 0x00 , 0x0E , 0xCB , 0x00 , 0x0F , 0xD0 , 0x00 , 0x0F , 0xD5 , 0x00 , 0x10 , 0xD9 , 
 0x00 , 0x0F , 0xDD , 0x00 , 0x10 , 0xE2 , 0x00 , 0x11 , 0xE5 , 0x00 , 0x11 , 0xE8 , 0x00 , 0x11 , 0xEC , 0x00 , 
 0x11 , 0xEF , 0x00 , 0x11 , 0xF1 , 0x00 , 0x11 , 0xF5 , 0x00 , 0x11 , 0xF6 , 0x00 , 0x12 , 0xF9 , 0x00 , 0x11 , 
 0xFA , 0x00 , 0x11 , 0xFC , 0x00 , 0x12 , 0xFD , 0x00 , 0x12 , 0xFE , 0x00 , 0x12 , 0xFF , 0x00 , 0x12 , 0xFF , 
 0x01 , 0x12 , 0xFF , 0x04 , 0x12 , 0xFE , 0x06 , 0x12 , 0xFE , 0x09 , 0x11 , 0xFD , 0x0B , 0x11 , 0xFB , 0x0E , 
 0x11 , 0xFB , 0x11 , 0x11 , 0xF8 , 0x14 , 0x10 , 0xF7 , 0x17 , 0x0F , 0xF5 , 0x1B , 0x0F , 0xF2 , 0x1E , 0x0E , 
 0xEF , 0x22 , 0x0E , 0xED , 0x26 , 0x0D , 0xE9 , 0x29 , 0x0C , 0xE7 , 0x2D , 0x0B , 0xE3 , 0x32 , 0x0A , 0xE0 , 
 0x36 , 0x09 , 0xDC , 0x3A , 0x08 , 0xD7 , 0x3F , 0x07 , 0xD4 , 0x44 , 0x07 , 0xCF , 0x48 , 0x06 , 0xCB , 0x4C , 
 0x04 , 0xC6 , 0x51 , 0x04 , 0xC2 , 0x55 , 0x02 , 0xBD , 0x5A , 0x02 , 0xB8 , 0x5F , 0x01 , 0xB4 , 0x63 , 0x00 , 
 0xAF , 0x68 , 0x00 , 0xAA , 0x6D , 0x00 , 0xA5 , 0x73 , 0x00 , 0xA0 , 0x78 , 0x00 , 0x9A , 0x7C , 0x00 , 0x95 , 
 0x81 , 0x00 , 0x90 , 0x86 , 0x00 , 0x8A , 0x8B , 0x00 , 0x85 , 0x90 , 0x00 , 0x7E , 0x96 , 0x00 , 0x78 , 0x9B , 
 0x00 , 0x73 , 0xA0 , 0x00 , 0x6E , 0xA5 , 0x00 , 0x68 , 0xA9 , 0x00 , 0x63 , 0xAF , 0x00 , 0x5D , 0xB3 , 0x00 , 
 0x57 , 0xB8 , 0x00 , 0x53 , 0xBC , 0x00 , 0x4D , 0xC1 , 0x00 , 0x48 , 0xC5 , 0x00 , 0x43 , 0xCA , 0x00 , 0x3D , 
 0xCE , 0x00 , 0x38 , 0xD3 , 0x00 , 0x33 , 0xD6 , 0x00 , 0x2F , 0xDA , 0x00 , 0x2B , 0xDE , 0x00 , 0x26 , 0xE2 , 
 0x00 , 0x22 , 0xE6 , 0x00 , 0x1D , 0xE8 , 0x00 , 0x1A , 0xEC , 0x00 , 0x16 , 0xEF , 0x00 , 0x12 , 0xF2 , 0x00 , 
 0x0E , 0xF5 , 0x00 , 0x0B , 0xF7 , 0x00 , 0x09 , 0xF9 , 0x00 , 0x06 , 0xFC , 0x00 , 0x04 , 0xFE , 0x00 , 0x01 , 
 0xFF , 0x01 , 0x00 , 0xFF , 0x03 , 0x00 , 0xFF , 0x05 , 0x00 , 0xFF , 0x07 , 0x00 , 0xFF , 0x0A , 0x00 , 0xFF , 
 0x0D , 0x00 , 0xFF , 0x10 , 0x00 , 0xFF , 0x13 , 0x00 , 0xFF , 0x16 , 0x00 , 0xFF , 0x19 , 0x00 , 0xFF , 0x1C , 
 0x00 , 0xFF , 0x21 , 0x00 , 0xFF , 0x24 , 0x00 , 0xFF , 0x28 , 0x00 , 0xFF , 0x2C , 0x00 , 0xFF , 0x31 , 0x00 , 
 0xFF , 0x35 , 0x00 , 0xFF , 0x38 , 0x00 , 0xFF , 0x3D , 0x00 , 0xFF , 0x41 , 0x00 , 0xFF , 0x46 , 0x00 , 0xFF , 
 0x4B , 0x00 , 0xFF , 0x50 , 0x00 , 0xFF , 0x54 , 0x00 , 0xFF , 0x59 , 0x00 , 0xFF , 0x5D , 0x00 , 0xFF , 0x63 , 
 0x00 , 0xFF , 0x67 , 0x00 , 0xFF , 0x6C , 0x00 , 0xFF , 0x71 , 0x00 , 0xFF , 0x76 , 0x00 , 0xFF , 0x7B , 0x00 , 
 0xFF , 0x81 , 0x00 , 0xFF , 0x85 , 0x00 , 0xFD , 0x8A , 0x00 , 0xFC , 0x8F , 0x00 , 0xFB , 0x95 , 0x00 , 0xFA , 
 0x9A , 0x00 , 0xF8 , 0x9E , 0x00 , 0xF8 , 0xA3 , 0x00 , 0xF6 , 0xA7 , 0x00 , 0xF5 , 0xAD , 0x00 , 0xF4 , 0xB1 , 
 0x00 , 0xF3 , 0xB6 , 0x00 , 0xF1 , 0xBA , 0x00 , 0xF0 , 0xBF , 0x00 , 0xF0 , 0xC4 , 0x00 , 0xEE , 0xC8 , 0x00 , 
 0xED , 0xCD , 0x00 , 0xEC , 0xD0 , 0x00 , 0xEB , 0xD4 , 0x00 , 0xEB , 0xD8 , 0x00 , 0xE9 , 0xDD , 0x00 , 0xE8 , 
 0xE0 , 0x00 , 0xE8 , 0xE4 , 0x00 , 0xE7 , 0xE7 , 0x00 , 0xE7 , 0xEB , 0x00 , 0xE6 , 0xED , 0x00 , 0xE6 , 0xF0 , 
 0x00 , 0xE5 , 0xF4 , 0x00 , 0xE4 , 0xF7 , 0x00 , 0xE4 , 0xF9 , 0x00 , 0xE4 , 0xFB , 0x00 , 0xE4 , 0xFE , 0x00 , 
 0xE4 , 0xFF , 0x01 , 0xE4 , 0xFF , 0x02 , 0xE5 , 0xFF , 0x05 , 0xE4 , 0xFF , 0x07 , 0xE5 , 0xFF , 0x0B , 0xE4 , 
 0xFF , 0x0D , 0xE4 , 0xFF , 0x10 , 0xE5 , 0xFF , 0x13 , 0xE5 , 0xFF , 0x16 , 0xE6 , 0xFF , 0x1A , 0xE5 , 0xFF , 
 0x1D , 0xE5 , 0xFF , 0x21 , 0xE6 , 0xFF , 0x24 , 0xE6 , 0xFF , 0x29 , 0xE7 , 0xFF , 0x2C , 0xE7 , 0xFF , 0x30 , 
 0xE8 , 0xFF , 0x34 , 0xE8 , 0xFF , 0x39 , 0xE9 , 0xFF , 0x3D , 0xE9 , 0xFF , 0x41 , 0xE9 , 0xFF , 0x46 , 0xEA , 
 0xFF , 0x4A , 0xEB , 0xFF , 0x50 , 0xEB , 0xFF , 0x54 , 0xEC , 0xFF , 0x59 , 0xEC , 0xFF , 0x5E , 0xED , 0xFF , 
 0x62 , 0xED , 0xFF , 0x67 , 0xEE , 0xFF , 0x6C , 0xEF , 0xFF , 0x71 , 0xEF , 0xFF , 0x76 , 0xF0 , 0xFF , 0x7B , 
 0xF0 , 0xFF , 0x80 , 0xF0 , 0xFF , 0x85 , 0xF1 , 0xFF , 0x8A , 0xF2 , 0xFF , 0x8F , 0xF2 , 0xFF , 0x94 , 0xF3 , 
 0xFF , 0x99 , 0xF3 , 0xFF , 0x9D , 0xF4 , 0xFF , 0xA3 , 0xF5 , 0xFF , 0xA7 , 0xF5 , 0xFF , 0xAC , 0xF6 , 0xFF , 
 0xB1 , 0xF6 , 0xFF , 0xB5 , 0xF6 , 0xFF , 0xBA , 0xF7 , 0xFF , 0xBE , 0xF8 , 0xFF , 0xC3 , 0xF8 , 0xFF , 0xC7 , 
 0xF9 , 0xFF , 0xCB , 0xF9 , 0xFF , 0xD0 , 0xFA , 0xFF , 0xD4 , 0xFB , 0xFF , 0xD8 , 0xFB , 0xFF , 0xDC , 0xFB , 
 0xFF , 0xDF , 0xFC , 0xFF , 0xE2 , 0xFC , 0xFF , 0xE6 , 0xFC , 0xFF , 0xEA , 0xFD , 0xFF , 0xEC , 0xFD , 0xFF , 
 0xF0 , 0xFD , 0xFF , 0xF3 , 0xFE , 0xFF , 0xF6 , 0xFE , 0xFF , 0xF8 , 0xFF , 0xFF , 0xFB , 0xFF , 0xFF , 0xFD , 
 };

bool InitI2SMicroPhone()
{
    esp_err_t err = ESP_OK;
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = 44100,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
        .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 2,
        .dma_buf_len = 128,
    };

    i2s_pin_config_t pin_config;
    pin_config.bck_io_num = I2S_PIN_NO_CHANGE;
    pin_config.ws_io_num = PIN_CLK;
    pin_config.data_out_num = I2S_PIN_NO_CHANGE;
    pin_config.data_in_num = PIN_DATA;
    

    err += i2s_driver_install(I2S_NUM_0, &i2s_config, 0, NULL);
    err += i2s_set_pin(I2S_NUM_0, &pin_config);
    err += i2s_set_clk(I2S_NUM_0, 44100, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
    //i2s_set_clk(0)

    if (err != ESP_OK)
    {
        return false;
    }
    else
    {
        return true;
    }
}

void MicRecordfft(void *arg)
{
    int16_t *buffptr;
    size_t bytesread;
    uint16_t count_n = 0;
    float adc_data;
    double data = 0;
    uint16_t ydata;

    while (1)
    {
        xSemaphoreTake(start_fft, portMAX_DELAY);
        xSemaphoreGive(start_fft);
        fft_config_t *real_fft_plan = fft_init(512, FFT_REAL, FFT_FORWARD, NULL, NULL);
        i2s_read(I2S_NUM_0, (char *)i2s_readraw_buff, 1024, &bytesread, (100 / portTICK_RATE_MS));
        buffptr = (int16_t *)i2s_readraw_buff;

        for (count_n = 0; count_n < real_fft_plan->size; count_n++)
        {
            adc_data = (float)map(buffptr[count_n], INT16_MIN, INT16_MAX, -1000, 1000);
            real_fft_plan->input[count_n] = adc_data;
        }
        fft_execute(real_fft_plan);

        xSemaphoreTake(xSemaphore, 100 / portTICK_RATE_MS);
        for (count_n = 1; count_n < real_fft_plan->size / 4; count_n++)
        {
            data = sqrt(real_fft_plan->output[2 * count_n] * real_fft_plan->output[2 * count_n] + real_fft_plan->output[2 * count_n + 1] * real_fft_plan->output[2 * count_n + 1]);
            if ((count_n - 1) < 80)
            {
                ydata = map(data, 0, 2000, 0, 256);
                fft_dis_buff[posData][80 - count_n] = ydata;
            }
        }
        posData++;
        if (posData >= 161)
        {
            posData = 0;
        }
        xSemaphoreGive(xSemaphore);
        fft_destroy(real_fft_plan);
    }
}

void Drawdisplay(void *arg)
{
    uint16_t count_x = 0, count_y = 0;
    uint16_t colorPos;
    while (1)
    {
        xSemaphoreTake(start_dis, portMAX_DELAY);
        xSemaphoreGive(start_dis);
        xSemaphoreTake(xSemaphore, 500 / portTICK_RATE_MS);
        for (count_y = 0; count_y < 80; count_y++)
        {
            for (count_x = 0; count_x < 160; count_x++)
            {
                if ((count_x + (posData % 160)) > 160)
                {
                    colorPos = fft_dis_buff[count_x + (posData % 160) - 160][count_y];
                }
                else
                {
                    colorPos = fft_dis_buff[count_x + (posData % 160)][count_y];
                }

                dispBuf.drawPixel(count_x, count_y, dispBuf.color565(ImageData[colorPos * 3 + 0], ImageData[colorPos * 3 + 1], ImageData[colorPos * 3 + 2]));
                /*
        dispBuf[ count_y * 160 + count_x ].r =  ImageData[ colorPos * 3 + 0 ];
        dispBuf[ count_y * 160 + count_x ].g =  ImageData[ colorPos * 3 + 1 ];
        dispBuf[ count_y * 160 + count_x ].b =  ImageData[ colorPos * 3 + 2 ];
        */
            }
        }
        xSemaphoreGive(xSemaphore);
        dispBuf.setTextColor(WHITE);
        dispBuf.setTextSize(1);
        dispBuf.fillRect(0,0,70,18,dispBuf.color565(20,20,20));
        dispBuf.drawString("MicroPhone",5,5,1);
        dispBuf.pushSprite(0, 0);
    }
}

void micFFTRun() {
  xSemaphoreGive(start_dis);
  xSemaphoreGive(start_fft);
  xSemaphoreTake(start_dis, portMAX_DELAY);
  xSemaphoreTake(start_fft, portMAX_DELAY);
};

bool micFFTInit() {
  InitI2SMicroPhone();
  dispBuf.fillRect(0, 0, 160, 80, dispBuf.color565(0, 0, 0));
  dispBuf.pushSprite(0, 0);
  xSemaphore = xSemaphoreCreateMutex();
  start_dis = xSemaphoreCreateMutex();
  start_fft = xSemaphoreCreateMutex();

  xSemaphoreTake(start_dis, portMAX_DELAY);
  xSemaphoreTake(start_fft, portMAX_DELAY);

  xTaskCreate(Drawdisplay, "Drawdisplay", 1024 * 2, (void *)0, 4, &xhandle_display);
  xTaskCreate(MicRecordfft, "MicRecordfft", 1024 * 2, (void *)0, 5, &xhandle_fft);
};

bool micFFTDeinit(){
  vTaskDelete(xhandle_display);
  vTaskDelete(xhandle_fft);
  i2s_driver_uninstall(I2S_NUM_0);
};

#endif // _MICROPHONE_FFT_H