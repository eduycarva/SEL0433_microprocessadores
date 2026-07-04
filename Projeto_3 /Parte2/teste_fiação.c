#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

//ESSE CÓDIGO FOI UTILIZADO PARA VERIFICAR SE OS FIOS DO WOKWI ESTAVAM CONFIGURADOS CORRETAMENTE

#define PINO_STEP 19
#define PINO_DIR  25
void app_main(void) {
    // Configura os pinos como saída de energia
    gpio_reset_pin(PINO_STEP);
    gpio_set_direction(PINO_STEP, GPIO_MODE_OUTPUT);

    gpio_reset_pin(PINO_DIR);
    gpio_set_direction(PINO_DIR, GPIO_MODE_OUTPUT);
    // Define a direção de giro (1 = Horário, 0 = Anti-horário)
    gpio_set_level(PINO_DIR, 1);
    printf("Iniciando teste bruto do Motor de Passo na porta 19...\n");
    while (1) {
        // Manda energia (Sobe o pulso)
        gpio_set_level(PINO_STEP, 1);
        vTaskDelay(pdMS_TO_TICKS(10)); // Espera 10 milissegundos
        // Tira a energia (Desce o pulso, completando 1 passo)
        gpio_set_level(PINO_STEP, 0);
        vTaskDelay(pdMS_TO_TICKS(10)); // Espera 10 milissegundos
    }
}
