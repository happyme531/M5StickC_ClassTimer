#include "camerareceive.h"

#include "FS.h"

#include "HTTPClient.h"
#include "M5StickC.h"
#include "WiFi.h"
#include <cinttypes>
#include "rom/tjpgd.h"

#define jpgColor(c) (((uint16_t)(((uint8_t*)(c))[0] & 0xF8) << 8) | ((uint16_t)(((uint8_t*)(c))[1] & 0xFC) << 3) | ((((uint8_t*)(c))[2] & 0xF8) >> 3))

#if ARDUHAL_LOG_LEVEL >= ARDUHAL_LOG_LEVEL_ERROR
const char* jd_errors[] = {
    "Succeeded",
    "Interrupted by output function",
    "Device error or wrong termination of input stream",
    "Insufficient memory pool for the image",
    "Insufficient stream input buffer",
    "Parameter error",
    "Data format error",
    "Right format but not supported",
    "Not supported JPEG standard"
};
#endif

// Struct to pass parameters to jpeg decoder
typedef struct {
    uint16_t x;
    uint16_t y;
    uint16_t maxWidth;
    uint16_t maxHeight;
    uint16_t offX;
    uint16_t offY;
    jpeg_div_t scale;
    const void* src;
    size_t len;
    size_t index;
    TFT_eSPI* tft;
    uint16_t outWidth;
    uint16_t outHeight;
} jpg_file_decoder_t;

/**************************************************************************/
//
//    JPEG decoder support function prototypes
//
/**************************************************************************/
static uint32_t jpgReadFile(JDEC* decoder, uint8_t* buf, uint32_t len);
static uint32_t jpgRead(JDEC* decoder, uint8_t* buf, uint32_t len);
static uint32_t jpgWrite(JDEC* decoder, void* bitmap, JRECT* rect);
static bool jpgDecode(jpg_file_decoder_t* jpeg, uint32_t (*reader)(JDEC*, uint8_t*, uint32_t));

/**************************************************************************/
/*!
    @brief  Decode an array stored FLASH in memory (ESP32 only)
    @param    Array name
    @param    Size of array, use sizeof(array_name)
    @param    Display x coord to draw at
    @param    Display y coord to draw at
    @param    Optional: Unscaled jpeg maximum width in pixels
    @param    Optional: Unscaled jpeg maximum height in pixels
    @param    Optional: Unscaled jpeg start x coordinate offset in jpeg
    @param    Optional: Unscaled jpeg start y coordinate offset in jpeg
    @param    Optional: Scale factor 0-4 (type jpeg_div_t)
    @return   true if decoded, else false
*/
/**************************************************************************/
// e.g tft.drawJpg(EagleEye, sizeof(EagleEye), 0, 10);
// Where EagleEye is an array of bytes in PROGMEM:
// const uint8_t EagleEye[] PROGMEM = {...};
bool drawJpg(const uint8_t* jpg_data, size_t jpg_len, uint16_t x, uint16_t y, uint16_t maxWidth, uint16_t maxHeight, uint16_t offX, uint16_t offY, jpeg_div_t scale)
{

    maxWidth = maxWidth >> (uint8_t)scale;
    maxHeight = maxHeight >> (uint8_t)scale;

    jpg_file_decoder_t jpeg;
    jpeg.src = jpg_data;
    jpeg.len = jpg_len;
    jpeg.index = 0;
    jpeg.x = x;
    jpeg.y = y;
    jpeg.maxWidth = maxWidth;
    jpeg.maxHeight = maxHeight;
    jpeg.offX = offX >> (uint8_t)scale;
    jpeg.offY = offY >> (uint8_t)scale;
    jpeg.scale = scale;
    jpeg.tft = &M5.Lcd;

    return jpgDecode(&jpeg, jpgRead);
}

/**************************************************************************/
//
//    JPEG decoder support functions
//
/**************************************************************************/

static uint32_t jpgRead(JDEC* decoder, uint8_t* buf, uint32_t len)
{
    jpg_file_decoder_t* jpeg = (jpg_file_decoder_t*)decoder->device;
    if (buf) {
        memcpy(buf, (const uint8_t*)jpeg->src + jpeg->index, len);
    }
    jpeg->index += len;
    return len;
}

static uint32_t jpgWrite(JDEC* decoder, void* bitmap, JRECT* rect)
{
    jpg_file_decoder_t* jpeg = (jpg_file_decoder_t*)decoder->device;
    uint16_t x = rect->left;
    uint16_t y = rect->top;
    uint16_t w = rect->right + 1 - x;
    uint16_t h = rect->bottom + 1 - y;
    uint16_t oL = 0, oR = 0;
    uint8_t* data = (uint8_t*)bitmap;

    if (rect->right < jpeg->offX) {
        return 1;
    }
    if (rect->left >= (jpeg->offX + jpeg->outWidth)) {
        return 1;
    }
    if (rect->bottom < jpeg->offY) {
        return 1;
    }
    if (rect->top >= (jpeg->offY + jpeg->outHeight)) {
        return 1;
    }
    if (rect->top < jpeg->offY) {
        uint16_t linesToSkip = jpeg->offY - rect->top;
        data += linesToSkip * w * 3;
        h -= linesToSkip;
        y += linesToSkip;
    }
    if (rect->bottom >= (jpeg->offY + jpeg->outHeight)) {
        uint16_t linesToSkip = (rect->bottom + 1) - (jpeg->offY + jpeg->outHeight);
        h -= linesToSkip;
    }
    if (rect->left < jpeg->offX) {
        oL = jpeg->offX - rect->left;
    }
    if (rect->right >= (jpeg->offX + jpeg->outWidth)) {
        oR = (rect->right + 1) - (jpeg->offX + jpeg->outWidth);
    }

    uint16_t pixBuf[32];
    uint8_t pixIndex = 0;
    uint16_t line;

    jpeg->tft->startWrite();
    jpeg->tft->setAddrWindow(x - jpeg->offX + jpeg->x + oL, y - jpeg->offY + jpeg->y, w - (oL + oR), h);

    while (h--) {
        data += 3 * oL;
        line = w - (oL + oR);
        while (line--) {
            pixBuf[pixIndex++] = jpgColor(data);
            data += 3;
            if (pixIndex == 32) {
                jpeg->tft->pushColors(pixBuf, 32);
                pixIndex = 0;
            }
        }
        data += 3 * oR;
    }
    if (pixIndex) {
        jpeg->tft->pushColors(pixBuf, pixIndex);
    }
    jpeg->tft->endWrite();
    return 1;
}

static bool jpgDecode(jpg_file_decoder_t* jpeg, uint32_t (*reader)(JDEC*, uint8_t*, uint32_t))
{
    static uint8_t work[3100];
    JDEC decoder;

    JRESULT jres = jd_prepare(&decoder, reader, work, 3100, jpeg);
    if (jres != JDR_OK) {
        log_e("jd_prepare failed! %s", jd_errors[jres]);
        return false;
    }

    uint16_t jpgWidth = decoder.width / (1 << (uint8_t)(jpeg->scale));
    uint16_t jpgHeight = decoder.height / (1 << (uint8_t)(jpeg->scale));

    if (jpeg->offX >= jpgWidth || jpeg->offY >= jpgHeight) {
        log_e("Offset Outside of JPEG size");
        return false;
    }

    size_t jpgMaxWidth = jpgWidth - jpeg->offX;
    size_t jpgMaxHeight = jpgHeight - jpeg->offY;

    jpeg->outWidth = (jpgMaxWidth > jpeg->maxWidth) ? jpeg->maxWidth : jpgMaxWidth;
    jpeg->outHeight = (jpgMaxHeight > jpeg->maxHeight) ? jpeg->maxHeight : jpgMaxHeight;

    jres = jd_decomp(&decoder, jpgWrite, (uint8_t)jpeg->scale);
    if (jres != JDR_OK) {
        log_e("jd_decomp failed! %s", jd_errors[jres]);
        return false;
    }

    return true;
}

String url = "http://192.168.4.1/capture";

HTTPClient http;

typedef struct Frame {
    uint8_t* buff;
    uint32_t len;
} xFrame;

xFrame* fr;

bool updateFrame(WiFiClient &client)
{

    if (WiFi.status() != WL_CONNECTED)
        return 0;
    http.begin(client,url);
    http.setTimeout(250);
    
    //http.setReuse(false);
    int httpCode = http.GET();
    // HTTP header has been send and Server response header has been handled
    if (httpCode <= 0) {
        Serial.printf("[HTTP] GET... failed, error: %s\n", http.errorToString(httpCode).c_str());
    } else {
        if (httpCode != HTTP_CODE_OK) {
            Serial.printf("[HTTP] GET... code: %d\n", httpCode);
        } else {
            xFrame* fr = (xFrame*)malloc(sizeof(xFrame));
            // get lenght of document (is -1 when Server sends no Content-Length header)
            fr->len = http.getSize();
            if (fr->len <= 0) {
                Serial.printf("[HTTP] Unknow content size: %d\n", fr->len);
            } else {
                // get tcp stream
                WiFiClient* stream = http.getStreamPtr();
                //Allocate buffer for reading
                fr->buff = (uint8_t*)malloc(fr->len * sizeof(uint8_t));
                int chunks = fr->len;
                // read all data from server
                while (http.connected() && (chunks > 0 || fr->len == -1)) {
                    // get available data size
                    size_t size = stream->available();

                    if (size) {
                        int chunk_size = ((size > ((fr->len) * sizeof(uint8_t))) ? ((fr->len) * (sizeof(uint8_t))) : size);
                        int indexer = stream->readBytes(fr->buff, chunk_size);
                        fr->buff += indexer;

                        if (chunks > 0) {
                            chunks -= indexer;
                        }
                    }
                }
                fr->buff -= fr->len;
                drawJpg(fr->buff, fr->len, 0, 0, 160, 80, 0, 0, JPEG_DIV_NONE);
                free(fr->buff);
                free(fr);
            }
        }
    }
    http.end();
    return 1;
}
