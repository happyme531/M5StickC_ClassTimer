#include "noisedetection.h"
#include <driver/i2s.h>

const i2s_port_t I2S_PORT = I2S_NUM_0;
const int BLOCK_SIZE = 128;

void enableNoiseDetection()
{
    enableMic();
    esp_err_t err;

    // The I2S config as per the example
    i2s_config_t i2s_config = {
        .mode = (i2s_mode_t)(I2S_MODE_MASTER | I2S_MODE_RX | I2S_MODE_PDM),
        .sample_rate = 16000,
        .bits_per_sample = I2S_BITS_PER_SAMPLE_16BIT, // is fixed at 12bit, stereo, MSB
        .channel_format = I2S_CHANNEL_FMT_ALL_RIGHT,
        .communication_format = I2S_COMM_FORMAT_I2S,
        .intr_alloc_flags = ESP_INTR_FLAG_LEVEL1,
        .dma_buf_count = 4,
        .dma_buf_len = 128,
    };

    // The pin config as per the setup
    const i2s_pin_config_t pin_config = {
        .bck_io_num = -1, // BCKL(未使用)
        .ws_io_num = 0, // LRCL
        .data_out_num = -1, // not used (only for speakers)
        .data_in_num = 34 // DOUT
    };

    // Configuring the I2S driver and pins.
    // This function must be called before any I2S driver read/write operations.
    err = i2s_driver_install(I2S_PORT, &i2s_config, 0, NULL);
    err = i2s_set_pin(I2S_PORT, &pin_config);
    i2s_set_clk(I2S_NUM_0, 44160, I2S_BITS_PER_SAMPLE_16BIT, I2S_CHANNEL_MONO);
}

double getNoiseVal()
{
    int32_t samples[BLOCK_SIZE];
    int num_bytes_read = i2s_read_bytes(I2S_PORT,
        (char*)samples,
        BLOCK_SIZE, // the doc says bytes, but its elements.
        portMAX_DELAY); // no timeout
    int samples_read = num_bytes_read / 8;
    if (samples_read > 0) {

        float mean = 0;
        for (int i = 0; i < samples_read; ++i) {
            mean += (samples[i] >> 14);
        }
        mean /= samples_read;

        float maxsample = -1e8, minsample = 1e8;
        for (int i = 0; i < samples_read; ++i) {
            minsample = min(minsample, samples[i] - mean);
            maxsample = max(maxsample, samples[i] - mean);
        }
        return (maxsample - minsample);
    }
    return 0;
}

void disableNoisedetection()
{
    disableMic();
    i2s_driver_uninstall(I2S_PORT);
};