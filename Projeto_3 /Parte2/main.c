#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"
#include "driver/adc.h"


#define SERVO_PIN 22
#define POT_PIN   ADC1_CHANNEL_7 // GPIO 35 para ADC1

void app_main(void)
{

    ledc_timer_config_t timer_servo = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_10_BIT, 
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 50,                      
        .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&timer_servo);

    
    ledc_channel_config_t canal_servo = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .channel = LEDC_CHANNEL_0,
        .gpio_num = SERVO_PIN,
        .duty = 0,
        .hpoint = 0
    };
    ledc_channel_config(&canal_servo);

    // Potenciometro
    adc1_config_width(ADC_WIDTH_BIT_10); 
    adc1_config_channel_atten(POT_PIN, ADC_ATTEN_DB_12);

    int leitura = 0;
    int duty_servo = 0;

    while (1)
    {
        
        leitura = adc1_get_raw(POT_PIN);

        duty_servo = 25 + ((leitura * (125 - 25)) / 1023);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_servo);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        int angulo = (leitura * 180) / 1023;
        printf("Pot: %d e Angulo (graus): %d\n", leitura, angulo);

        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}
