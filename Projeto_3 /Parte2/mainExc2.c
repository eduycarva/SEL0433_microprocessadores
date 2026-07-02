#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/mcpwm.h" 
#include "driver/gpio.h"
#include "driver/i2c.h"

#define CANAL_POTENCIOMETRO ADC1_CHANNEL_7 
#define PINO_MOTOR          21             


#define PINO_SCL            22            
#define PINO_SDA            23             
#define ENDERECO_OLED       0x3C          
#define BARRAMENTO_I2C      I2C_NUM_0

const uint8_t texto_font[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, 
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, 
    {0x00, 0x42, 0x7f, 0x40, 0x00}, 
    {0x42, 0x61, 0x51, 0x49, 0x46}, 
    {0x21, 0x41, 0x45, 0x4b, 0x31}, 
    {0x18, 0x14, 0x12, 0x7f, 0x10}, 
    {0x27, 0x45, 0x45, 0x45, 0x39}, 
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, 
    {0x01, 0x71, 0x09, 0x05, 0x03}, 
    {0x36, 0x49, 0x49, 0x49, 0x36}, 
    {0x06, 0x49, 0x49, 0x29, 0x1e}, 
    {0x00, 0x36, 0x36, 0x00, 0x00}, 
    {0x23, 0x13, 0x08, 0x64, 0x62},
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, 
    {0x3e, 0x41, 0x41, 0x41, 0x22}, 
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, 
    {0x7f, 0x49, 0x49, 0x49, 0x41}, 
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, 
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, 
    {0x7f, 0x09, 0x19, 0x29, 0x46}, 
    {0x7f, 0x01, 0x01, 0x01, 0x01}, 
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, 
    {0x08, 0x08, 0x08, 0x08, 0x08}  
};


void enviar_comando_oled(uint8_t comando) {
    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(link, 0x00, true); 
    i2c_master_write_byte(link, comando, true);
    i2c_master_stop(link);
    i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(link);
}

void inicializar_tela_oled() {
    uint8_t comandos_config[] = {
        0xAE, 0x20, 0x10, 0xB0, 0x00, 0x10, 0x40, 0x81, 
        0x7F, 0xA0, 0xC0, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3, 
        0x00, 0xD5, 0x80, 0xD9, 0x22, 0xDA, 0x12, 0xDB, 
        0x20, 0x8D, 0x14, 0xAF
    };
    for (int i = 0; i < sizeof(comandos_config); i++) {
        enviar_comando_oled(comandos_config[i]);
    }
}

void limpar_tela_oled() {
    for (uint8_t pagina = 0; pagina < 8; pagina++) {
        enviar_comando_oled(0xB0 + pagina);
        enviar_comando_oled(0x00);
        enviar_comando_oled(0x10);
        i2c_cmd_handle_t link = i2c_cmd_link_create();
        i2c_master_start(link);
        i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(link, 0x40, true);
        for (int i = 0; i < 128; i++) i2c_master_write_byte(link, 0x00, true);
        i2c_master_stop(link);
        i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(link);
    }
}

void desenhar_caractere(char caractere) {
    int indice = 0; 
    if (caractere >= '0' && caractere <= '9') indice = caractere - '0' + 1;
    else if (caractere == ':') indice = 11;
    else if (caractere == '%') indice = 12;
    else if (caractere == 'A') indice = 13;
    else if (caractere == 'C') indice = 14;
    else if (caractere == 'D') indice = 15;
    else if (caractere == 'E') indice = 16;
    else if (caractere == 'M') indice = 17;
    else if (caractere == 'O') indice = 18;
    else if (caractere == 'R') indice = 19;
    else if (caractere == 'T') indice = 20;
    else if (caractere == 'W') indice = 21;
    else if (caractere == '-') indice = 22;

    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(link, 0x40, true);
    for (int i = 0; i < 5; i++) {
        i2c_master_write_byte(link, texto_font[indice][i], true);
    }
    i2c_master_write_byte(link, 0x00, true); 
    i2c_master_stop(link);
    i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(link);
}

void escrever_linha_oled(const char* texto, uint8_t pagina) {
    enviar_comando_oled(0xB0 + pagina);
    enviar_comando_oled(0x00);
    enviar_comando_oled(0x10);
    while (*texto) {
        desenhar_caractere(*texto);
        texto++;
    }
}

void app_main(void)
{
    int leitura_potenciometro = 0;
    float velocidade_motor = 0.0;
    
    char string_linha_adc[20];
    char string_linha_pwm[20];


    i2c_config_t config_i2c = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = PINO_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = PINO_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 400000, 
    };
    i2c_param_config(BARRAMENTO_I2C, &config_i2c);
    i2c_driver_install(BARRAMENTO_I2C, config_i2c.mode, 0, 0, 0);

    inicializar_tela_oled();
    limpar_tela_oled();

  
    adc1_config_width(ADC_WIDTH_BIT_12); 
    adc1_config_channel_atten(CANAL_POTENCIOMETRO, ADC_ATTEN_DB_11);

   
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PINO_MOTOR);

    mcpwm_config_t config_motor = {
        .frequency = 1000,                
        .cmpr_a = 0,                      
        .counter_mode = MCPWM_UP_COUNTER,   
        .duty_mode = MCPWM_DUTY_MODE_0      
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &config_motor);
    

    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);

   
    while (1) {
        leitura_potenciometro = adc1_get_raw(CANAL_POTENCIOMETRO);

        velocidade_motor = (leitura_potenciometro * 100.0) / 4095.0;

      
        mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, velocidade_motor);
        mcpwm_set_duty_type(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, MCPWM_DUTY_MODE_0);

      
        printf("MONITORAMENTO -> ADC: %d | VELOCIDADE: %.1f%%\n", leitura_potenciometro, velocidade_motor);

        snprintf(string_linha_adc, sizeof(string_linha_adc), "ADC: %d     ", leitura_potenciometro);
        snprintf(string_linha_pwm, sizeof(string_linha_pwm), "MOTOR: %d%%    ", (int)velocidade_motor);

        escrever_linha_oled(string_linha_adc, 4);
        escrever_linha_oled(string_linha_pwm, 6);

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}
