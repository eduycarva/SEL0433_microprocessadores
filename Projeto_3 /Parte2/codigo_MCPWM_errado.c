#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/adc.h"
#include "driver/mcpwm.h" 
#include "driver/gpio.h"
#include "driver/i2c.h"

// Definições de Pinos
#define PINO_POTENCIOMETRO ADC1_CHANNEL_6 // Pino 34
#define PINO_STEP          19             // MCPWM (Controla a velocidade, sem JTAG)
#define PINO_DIR           25             // GPIO (Controla a direção)
#define PINO_BOTAO         12             // Botão Trava de emergência
#define PINO_SDA           21
#define PINO_SCL           22

#define ENDERECO_OLED      0x3C          
#define BARRAMENTO_I2C     I2C_NUM_0

volatile bool sistema_ativo = true;
volatile uint32_t ultimo_tempo_isr = 0;

static void IRAM_ATTR botao_isr_handler(void* arg) {
    uint32_t tempo_atual = xTaskGetTickCountFromISR();
    if (tempo_atual - ultimo_tempo_isr > pdMS_TO_TICKS(250)) { 
        sistema_ativo = !sistema_ativo;
        ultimo_tempo_isr = tempo_atual;
    }
}

// Fonte customizada (Números e Letras A-Z)
const uint8_t fonte[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // 0: Espaço
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 1: '0'
    {0x00, 0x42, 0x7f, 0x40, 0x00}, // 2: '1'
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 3: '2'
    {0x21, 0x41, 0x45, 0x4b, 0x31}, // 4: '3'
    {0x18, 0x14, 0x12, 0x7f, 0x10}, // 5: '4'
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 6: '5'
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 7: '6'
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 8: '7'
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 9: '8'
    {0x06, 0x49, 0x49, 0x29, 0x1e}, // 10: '9'
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, // 11: A
    {0x7f, 0x49, 0x49, 0x49, 0x36}, // 12: B
    {0x3e, 0x41, 0x41, 0x41, 0x22}, // 13: C
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, // 14: D
    {0x7f, 0x49, 0x49, 0x49, 0x41}, // 15: E
    {0x7f, 0x09, 0x09, 0x09, 0x01}, // 16: F
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, // 17: G
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, // 18: H
    {0x00, 0x41, 0x7f, 0x41, 0x00}, // 19: I
    {0x20, 0x40, 0x41, 0x3f, 0x01}, // 20: J
    {0x7f, 0x08, 0x14, 0x22, 0x41}, // 21: K
    {0x7f, 0x40, 0x40, 0x40, 0x40}, // 22: L
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // 23: M
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, // 24: N
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, // 25: O
    {0x7f, 0x09, 0x09, 0x09, 0x06}, // 26: P
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, // 27: Q
    {0x7f, 0x09, 0x19, 0x29, 0x46}, // 28: R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // 29: S
    {0x01, 0x01, 0x7f, 0x01, 0x01}, // 30: T
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, // 31: U
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, // 32: V
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, // 33: W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // 34: X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // 35: Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // 36: Z
    {0x00, 0x36, 0x36, 0x00, 0x00}, // 37: ':'
    {0x23, 0x13, 0x08, 0x64, 0x62}, // 38: '%'
    {0x08, 0x08, 0x08, 0x08, 0x08}  // 39: '-'
};

int obter_indice(char c) {
    if (c >= '0' && c <= '9') return c - '0' + 1;
    if (c >= 'A' && c <= 'Z') return c - 'A' + 11;
    if (c == ':') return 37;
    if (c == '%') return 38;
    if (c == '-') return 39;
    return 0; 
}

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
    uint8_t config[] = { 0xAE, 0x20, 0x10, 0xB0, 0x00, 0x10, 0x40, 0x81, 0x7F, 0xA0, 0xC0, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3, 0x00, 0xD5, 0x80, 0xD9, 0x22, 0xDA, 0x12, 0xDB, 0x20, 0x8D, 0x14, 0xAF };
    for (int i = 0; i < sizeof(config); i++) enviar_comando_oled(config[i]);
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
    int indice = obter_indice(caractere);
    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(link, 0x40, true);
    for (int i = 0; i < 5; i++) i2c_master_write_byte(link, fonte[indice][i], true);
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
    char texto_modo[32];
    char texto_vel[32];


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
    adc1_config_channel_atten(PINO_POTENCIOMETRO, ADC_ATTEN_DB_12); 

 
    gpio_config_t config_botao = {
        .pin_bit_mask = (1ULL << PINO_BOTAO),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_NEGEDGE
    };
    gpio_config(&config_botao);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PINO_BOTAO, botao_isr_handler, NULL);

    // Configura o pino de Direção do Driver
    gpio_reset_pin(PINO_DIR);
    gpio_set_direction(PINO_DIR, GPIO_MODE_OUTPUT);
    gpio_set_level(PINO_DIR, 1); 

    // 4. Configuração MCPWM 
    mcpwm_gpio_init(MCPWM_UNIT_0, MCPWM0A, PINO_STEP);
    mcpwm_config_t config_motor = {
        .frequency = 500,                   
        .cmpr_a = 0,                   
        .counter_mode = MCPWM_UP_COUNTER,   
        .duty_mode = MCPWM_DUTY_MODE_0      
    };
    mcpwm_init(MCPWM_UNIT_0, MCPWM_TIMER_0, &config_motor);
    mcpwm_start(MCPWM_UNIT_0, MCPWM_TIMER_0);

    // Variável para guardar a última frequência enviada ao motor
    static uint32_t ultima_frequencia = 0;

    while (1) {
        int leitura_adc = adc1_get_raw(PINO_POTENCIOMETRO);
        
        // Mapeia ADC para Frequência 
        uint32_t frequencia_desejada = 10 + ((leitura_adc * 990) / 4095);

        if (!sistema_ativo) {
            // TRAVA ATIVADA
            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 0.0);
            ultima_frequencia = 0; // Força uma atualização quando destravar
        } else {
            // MOTOR ATIVO
            mcpwm_set_duty(MCPWM_UNIT_0, MCPWM_TIMER_0, MCPWM_OPR_A, 50.0);
            
            // Só muda a frequência se a diferença for maior que 20Hz
            int diferenca = frequencia_desejada - ultima_frequencia;
            if (abs(diferenca) > 20 || ultima_frequencia == 0) {
                mcpwm_set_frequency(MCPWM_UNIT_0, MCPWM_TIMER_0, frequencia_desejada);
                ultima_frequencia = frequencia_desejada;
            }
        }

        // Imprime no Serial
        printf("MODO: %s | VELOCIDADE: %lu Hz\n", sistema_ativo ? "ATIVO " : "TRAVA ", ultima_frequencia);

        // Imprime no OLED
        snprintf(texto_modo, sizeof(texto_modo), "MODO: %s  ", sistema_ativo ? "ATIVO " : "TRAVA ");
        
        if(!sistema_ativo) {
            snprintf(texto_vel, sizeof(texto_vel), "VEL: 0 HZ      ");
        } else {
            snprintf(texto_vel, sizeof(texto_vel), "VEL: %lu HZ    ", ultima_frequencia);
        }

        escrever_linha_oled(texto_modo, 2);
        escrever_linha_oled(texto_vel, 5);

        vTaskDelay(pdMS_TO_TICKS(150));
    }
}
