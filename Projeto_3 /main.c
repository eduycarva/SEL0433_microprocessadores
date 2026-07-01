#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

#define R 23
#define G 22
#define B 21

void app_main(void)
{
    ledc_timer_config_t timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .timer_num = LEDC_TIMER_0,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ledc_timer_config(&timer);

    ledc_channel_config_t canal = {
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
        .duty = 0,
        .hpoint = 0
    };

    canal.channel = LEDC_CHANNEL_0;
    canal.gpio_num = R;
    ledc_channel_config(&canal);

    canal.channel = LEDC_CHANNEL_1;
    canal.gpio_num = G;
    ledc_channel_config(&canal);

    canal.channel = LEDC_CHANNEL_2;
    canal.gpio_num = B;
    ledc_channel_config(&canal);

    int vermelho = 0;
    int verde = 0;
    int azul = 0;

    while (1)
    {
        vermelho += 15;
        verde += 5;
        azul += 10;

        if (vermelho > 255)
            vermelho = 0;

        if (verde > 255)
            verde = 0;

        if (azul > 255)
            azul = 0;

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, vermelho);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, verde);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1);

        ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, azul);
        ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);

        printf("Incrementos: R=15 G=5 B=10\n");
        printf("Duty -> R:%d%% G:%d%% B:%d%%\n\n", (vermelho * 100) / 255, (verde * 100) / 255, (azul * 100) / 255);

        vTaskDelay(pdMS_TO_TICKS(200));
    }
}
