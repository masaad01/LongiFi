#ifndef LONGIFI_PIN_UTIL_H
#define LONGIFI_PIN_UTIL_H

// #define LILYGO
// #define HELTEC


#define UNUSE_PIN                   (0)

#if defined(LILYGO)
#define I2C_SDA                    21
#define I2C_SCL                    22
#define OLED_RST                   UNUSE_PIN

#define LORA_SCLK_PIN              5
#define LORA_MISO_PIN              19
#define LORA_MOSI_PIN              27
#define LORA_CS_PIN                18
#define LORA_DIO0_PIN              26
#define LORA_RST_PIN               23
#define LORA_DIO1_PIN              33
#define LORA_BUSY_PIN              32

#define SDCARD_MOSI                15
#define SDCARD_MISO                2
#define SDCARD_SCLK                14
#define SDCARD_CS                  13

#define BOARD_LED                  25
#define LED_BUILTIN                BOARD_LED
#define LED_ON                     HIGH

#define ADC_PIN                    35

#define HAS_SDCARD
#define HAS_DISPLAY
#elif defined(HELTEC)
#define I2C_SDA                    4
#define I2C_SCL                    15
#define OLED_RST                   16

#define LORA_SCLK_PIN              5
#define LORA_MISO_PIN              19
#define LORA_MOSI_PIN              27
#define LORA_CS_PIN                18
#define LORA_DIO0_PIN              26
#define LORA_RST_PIN               14
#define LORA_DIO1_PIN              35
#define LORA_BUSY_PIN              32

#define SDCARD_MOSI                UNUSE_PIN
#define SDCARD_MISO                UNUSE_PIN
#define SDCARD_SCLK                UNUSE_PIN
#define SDCARD_CS                  UNUSE_PIN

#define BOARD_LED                  25
#define LED_BUILTIN                BOARD_LED
#define LED_ON                     HIGH

#define ADC_PIN                    36

// #define HAS_SDCARD
#define HAS_DISPLAY

#else
#error "Please define the board version and model as either: HELTEC or LILYGO."
#endif



#endif