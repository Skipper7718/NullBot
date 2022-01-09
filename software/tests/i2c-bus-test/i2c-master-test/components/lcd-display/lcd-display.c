#include <string.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "driver/spi_master.h"
#include "driver/gpio.h"
#include "font.h"
#include "driver/sdspi_host.h"
#include "driver/spi_common.h"
#include "esp_err.h"
#include "esp_log.h"
#include <sys/unistd.h>
#include <sys/stat.h>
#include "driver/adc.h"
#include <sys/time.h>
#include "esp_sleep.h"
#include "lcd-display.h"

uint16_t colstart = 52;
uint16_t rowstart = 40;
uint16_t _init_height = 240;
uint16_t _init_width = 135;
uint16_t _width = 135;
uint16_t _height = 240;

static spi_device_handle_t spi;
static RTC_DATA_ATTR struct timeval sleep_enter_time;

#define DEFAULT_VERF                            1100


void lcd_cmd( const uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));                  
    t.length = 8;                               
    t.tx_buffer = &cmd;                        
    t.user = (void *)0;                       
    ret = spi_device_polling_transmit(spi, &t); 
    assert(ret == ESP_OK);                
}

void lcd_data( const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t;
    if (len == 0) return;                  
    memset(&t, 0, sizeof(t));             
    t.length = len * 8;                     
    t.tx_buffer = data;                     
    t.user = (void *)1;                       
    ret = spi_device_polling_transmit(spi, &t);
    assert(ret == ESP_OK);
}

void lcd_spi_pre_transfer_callback(spi_transaction_t *t)
{
    int dc = (int)t->user;
    gpio_set_level(PIN_NUM_DC, dc);
}

void lcd_write_u8( const uint8_t data)
{
    lcd_data( &data, 1);
}

//Initialize
void lcd_init(spi_device_handle_t spi)
{
    //init non-SPI GPIOs
    gpio_set_direction(PIN_NUM_DC, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_RST, GPIO_MODE_OUTPUT);
    gpio_set_direction(PIN_NUM_BCKL, GPIO_MODE_OUTPUT);

    //Reset
    gpio_set_level(PIN_NUM_RST, 0);
    vTaskDelay(100 / portTICK_RATE_MS);
    gpio_set_level(PIN_NUM_RST, 1);
    vTaskDelay(100 / portTICK_RATE_MS);


    lcd_cmd( ST7789_SLPOUT);    
    vTaskDelay(120 / portTICK_RATE_MS);

    lcd_cmd( ST7789_NORON); 

    //------------------------------display and color format setting--------------------------------//
    lcd_cmd( ST7789_MADCTL);
    lcd_write_u8( TFT_MAD_RGB);

    // JLX240 display datasheet
    lcd_cmd( 0xB6);
    lcd_write_u8( 0x0A);
    lcd_write_u8( 0x82);

    lcd_cmd( ST7789_COLMOD);
    lcd_write_u8( 0x55);
    vTaskDelay(10 / portTICK_RATE_MS);

    //--------------------------------ST7789V Frame rate setting----------------------------------//
    lcd_cmd( ST7789_PORCTRL);
    lcd_write_u8( 0x0c);
    lcd_write_u8( 0x0c);
    lcd_write_u8( 0x00);
    lcd_write_u8( 0x33);
    lcd_write_u8( 0x33);

    lcd_cmd( ST7789_GCTRL);                 // Voltages: VGH / VGL
    lcd_write_u8( 0x35);

    //---------------------------------ST7789V Power setting--------------------------------------//
    lcd_cmd( ST7789_VCOMS);
    lcd_write_u8( 0x28);                    // JLX240 display datasheet

    lcd_cmd( ST7789_LCMCTRL);
    lcd_write_u8( 0x0C);

    lcd_cmd( ST7789_VDVVRHEN);
    lcd_write_u8( 0x01);
    lcd_write_u8( 0xFF);

    lcd_cmd( ST7789_VRHS);                  // voltage VRHS
    lcd_write_u8( 0x10);

    lcd_cmd( ST7789_VDVSET);
    lcd_write_u8( 0x20);

    lcd_cmd( ST7789_FRCTR2);
    lcd_write_u8( 0x0f);

    lcd_cmd( ST7789_PWCTRL1);
    lcd_write_u8( 0xa4);
    lcd_write_u8( 0xa1);

    //--------------------------------ST7789V gamma setting---------------------------------------//
    lcd_cmd( ST7789_PVGAMCTRL);
    lcd_write_u8( 0xd0);
    lcd_write_u8( 0x00);
    lcd_write_u8( 0x02);
    lcd_write_u8( 0x07);
    lcd_write_u8( 0x0a);
    lcd_write_u8( 0x28);
    lcd_write_u8( 0x32);
    lcd_write_u8( 0x44);
    lcd_write_u8( 0x42);
    lcd_write_u8( 0x06);
    lcd_write_u8( 0x0e);
    lcd_write_u8( 0x12);
    lcd_write_u8( 0x14);
    lcd_write_u8( 0x17);

    lcd_cmd( ST7789_NVGAMCTRL);
    lcd_write_u8( 0xd0);
    lcd_write_u8( 0x00);
    lcd_write_u8( 0x02);
    lcd_write_u8( 0x07);
    lcd_write_u8( 0x0a);
    lcd_write_u8( 0x28);
    lcd_write_u8( 0x31);
    lcd_write_u8( 0x54);
    lcd_write_u8( 0x47);
    lcd_write_u8( 0x0e);
    lcd_write_u8( 0x1c);
    lcd_write_u8( 0x17);
    lcd_write_u8( 0x1b);
    lcd_write_u8( 0x1e);

    lcd_cmd(ST7789_INVON);

    lcd_cmd( ST7789_DISPON);   //Display on
    vTaskDelay(120 / portTICK_RATE_MS);

    ///Enable backlight
    gpio_set_level(PIN_NUM_BCKL, 1);
}

void lcd_write_byte( const uint16_t data)
{
    uint8_t val;
    val = data >> 8 ;
    lcd_data( &val, 1);
    val = data;
    lcd_data( &val, 1);
}

void setAddress( uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    lcd_cmd( ST7789_CASET);
    lcd_write_byte( x1 + colstart);
    lcd_write_byte( x2 + colstart);
    lcd_cmd( ST7789_RASET);
    lcd_write_byte( y1 + rowstart);
    lcd_write_byte( y2 + rowstart);
    lcd_cmd( ST7789_RAMWR);
}

void setRotation( uint8_t m)
{
    uint8_t rotation = m % 4;
    lcd_cmd( ST7789_MADCTL);
    switch (rotation) {
    case 0:
        colstart = 52;
        rowstart = 40;
        _width  = _init_width;
        _height = _init_height;
        lcd_write_u8( TFT_MAD_COLOR_ORDER);
        break;

    case 1:
        colstart = 40;
        rowstart = 53;
        _width  = _init_height;
        _height = _init_width;
        lcd_write_u8( TFT_MAD_MX | TFT_MAD_MV | TFT_MAD_COLOR_ORDER);
        break;
    case 2:
        colstart = 53;
        rowstart = 40;
        _width  = _init_width;
        _height = _init_height;
        lcd_write_u8( TFT_MAD_MX | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
        break;
    case 3:
        colstart = 40;
        rowstart = 52;
        _width  = _init_height;
        _height = _init_width;
        lcd_write_u8( TFT_MAD_MV | TFT_MAD_MY | TFT_MAD_COLOR_ORDER);
        break;
    }
}



void lcd_send_uint16_r(const uint16_t data, int32_t repeats)
{
    uint32_t i;
    uint32_t word = data << 16 | data;
    uint32_t word_tmp[16];
    spi_transaction_t t;

    while (repeats > 0) {
        uint16_t bytes_to_transfer = MIN(repeats * sizeof(uint16_t), SPIFIFOSIZE * sizeof(uint32_t));
        for (i = 0; i < (bytes_to_transfer + 3) / 4; i++) {
            word_tmp[i] = word;
        }
        memset(&t, 0, sizeof(t));       
        t.length = bytes_to_transfer * 8; 
        t.tx_buffer = word_tmp;         
        t.user = (void *) 1; 
        spi_device_transmit(spi, &t);
        repeats -= bytes_to_transfer / 2;
    }
}


void fillRect( int32_t x, int32_t y, int32_t w, int32_t h, uint32_t color)
{
    // Clip
    if ((x >= _width) || (y >= _height)) return;

    if (x < 0) {
        w += x;
        x = 0;
    }
    if (y < 0) {
        h += y;
        y = 0;
    }

    if ((x + w) > _width)  w = _width  - x;
    if ((y + h) > _height) h = _height - y;

    if ((w < 1) || (h < 1)) return;


    setAddress( x, y, x + w - 1, y + h - 1);

    lcd_send_uint16_r(SWAPBYTES(color), h * w);
}

void fillScreen( uint32_t color)
{
    fillRect( 0, 0, _width, _height, color);
}

void drawPixel(int32_t x, int32_t y, uint32_t color)
{
    setAddress(x, y, x, y);
    lcd_write_byte(color);
}

void drawChar(uint16_t x, uint16_t y, uint8_t num, uint8_t mode, uint16_t color)
{
    uint8_t temp;
    uint8_t pos, t;
    uint16_t x0 = x;
    if (x > _width - 16 || y > _height - 16)return;
    num = num - ' ';
    setAddress(x, y, x + 8 - 1, y + 16 - 1);
    if (!mode) {
        for (pos = 0; pos < 16; pos++) {
            temp = asc2_1608[(uint16_t)num * 16 + pos];
            for (t = 0; t < 8; t++) {
                if (temp & 0x01)
                    lcd_write_byte(color);
                else
                    lcd_write_byte(TFT_BLACK);
                temp >>= 1;
                x++;
            }
            x = x0;
            y++;
        }
    } else {
        for (pos = 0; pos < 16; pos++) {
            temp = asc2_1608[(uint16_t)num * 16 + pos];
            for (t = 0; t < 8; t++) {
                if (temp & 0x01)
                    drawPixel(x + t, y + pos, color);
                temp >>= 1;
            }
        }
    }
}

void drawString(uint16_t x, uint16_t y, const char *p, uint16_t color)
{
    while (*p != '\0') {
        if (x > _width - 16) {
            x = 0;
            y += 16;
        }
        if (y > _height - 16) {
            y = x = 0;
            fillScreen(TFT_RED);
        }
        drawChar(x, y, *p, 0, color);
        x += 8;
        p++;
    }
}


uint32_t read_adc_raw()
{
    uint32_t adc_reading = 0;
    for (int i = 0; i < NO_OF_SAMPLES; i++) {
        adc_reading += adc1_get_raw((adc1_channel_t)ADC_CHANNEL_8);
    }
    adc_reading /= NO_OF_SAMPLES;
    return adc_reading;
}

void init_display(){
    adc1_config_width(ADC_WIDTH_BIT_13);

    adc1_config_channel_atten(ADC_CHANNEL_8, ADC_ATTEN_DB_11);


    gpio_pad_select_gpio(POWER_PIN);
    gpio_set_direction(POWER_PIN, GPIO_MODE_OUTPUT);

    gpio_set_level(POWER_PIN, 1);

    esp_err_t ret;
    spi_bus_config_t buscfg = {
        .miso_io_num = PIN_NUM_MISO,
        .mosi_io_num = PIN_NUM_MOSI,
        .sclk_io_num = PIN_NUM_CLK,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = SPIFIFOSIZE * 240 * 2 + 8
    };
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = 26 * 1000 * 1000,
        .mode = 0,
        .spics_io_num = PIN_NUM_CS,
        .queue_size = 7,
        .pre_cb = lcd_spi_pre_transfer_callback,
    };
    //Initialize the SPI bus
    ret = spi_bus_initialize(LCD_HOST, &buscfg, DMA_CHAN);
    ESP_ERROR_CHECK(ret);
    //Attach the LCD to the SPI bus
    ret = spi_bus_add_device(LCD_HOST, &devcfg, &spi);
    ESP_ERROR_CHECK(ret);
    //Initialize the LCD
    lcd_init(spi);

    setAddress( 0, 0,  _width - 1, _height - 1);
    setRotation(1);
}

void write_rect(uint32_t border, char *value){
    fillScreen(border);
    fillRect(20, 20, 160,95,TFT_BLACK);
    drawString(30,55, value, TFT_WHITE);
}