#include <stdio.h>                  // Funções de entrada/saída padrão (printf, snprintf)
#include <string.h>                 // Manipulação de strings (não usado diretamente aqui, mas comum em projetos C)
#include "freertos/FreeRTOS.h"      // Núcleo do FreeRTOS: tipos, ticks, macros de tempo
#include "freertos/task.h"          // Gerenciamento de tasks: vTaskDelay, xTaskGetTickCountFromISR
#include "driver/adc.h"             // Driver do conversor analógico-digital (leitura do potenciômetro)
#include "driver/gpio.h"            // Driver de GPIO (configuração de pinos digitais e da ISR do botão)
#include "driver/i2c.h"             // Driver do barramento I2C (comunicação com o display OLED)
#include "driver/ledc.h"            // Modulador PWM nativo da ESP32 (Substituto do MCPWM)

// ============================================================================
// Definições de Pinos e Constantes I2C
// ============================================================================
#define PINO_POTENCIOMETRO ADC1_CHANNEL_6 // Canal ADC1 ligado ao potenciômetro (GPIO 34)
#define PINO_STEP          19             // Pino que gera o pulso PWM enviado ao driver A4988 (STEP)
#define PINO_DIR           25             // Pino digital que define o sentido de rotação do motor (DIR)
#define PINO_BOTAO         12             // Pino do botão de trava de emergência (com pull-up interno)
#define PINO_SDA           21             // Linha de dados do barramento I2C (para o display OLED)
#define PINO_SCL           22             // Linha de clock do barramento I2C (para o display OLED)

#define ENDERECO_OLED      0x3C           // Endereço I2C padrão do controlador SSD1306
#define BARRAMENTO_I2C     I2C_NUM_0      // Usa o periférico I2C número 0 da ESP32

// ============================================================================
// Variáveis Globais e Interrupção (ISR) do Botão de Trava de Emergência
// ============================================================================
volatile bool sistema_ativo = true;      // Estado do motor (true = girando, false = travado). 'volatile' porque é alterada dentro da ISR.
volatile uint32_t ultimo_tempo_isr = 0;  // Guarda o tick do último acionamento válido do botão, usado para o debounce.

// Rotina de interrupção (ISR) executada em resposta à borda de descida (NEGEDGE) no pino do botão.
// IRAM_ATTR garante que o código fique na RAM interna, exigido para funções de ISR na ESP32.
static void IRAM_ATTR botao_isr_handler(void* arg) {
    uint32_t tempo_atual = xTaskGetTickCountFromISR(); // Lê o tick atual do FreeRTOS (versão segura para uso em ISR)

    // Debounce por software: só aceita um novo acionamento se já se passaram mais de 250ms desde o último
    if (tempo_atual - ultimo_tempo_isr > pdMS_TO_TICKS(250)) {
        sistema_ativo = !sistema_ativo;   // Alterna o estado do sistema (liga <-> trava)
        ultimo_tempo_isr = tempo_atual;   // Atualiza a referência de tempo para o próximo debounce
    }
}

// ============================================================================
// Mapeamento de Matriz (OLED Sem Bibliotecas Externas)
// ============================================================================
// Matriz de fonte 5x8: cada linha representa um caractere, e cada byte representa uma coluna de 8 pixels verticais.
// Índice 0 é o espaço em branco; os demais seguem a ordem: dígitos 0-9, letras A-Z, e os símbolos ':', '%', '-'.
const uint8_t fonte[][5] = {
    {0x00, 0x00, 0x00, 0x00, 0x00}, // Espaço
    {0x3e, 0x51, 0x49, 0x45, 0x3e}, // 0
    {0x00, 0x42, 0x7f, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x21, 0x41, 0x45, 0x4b, 0x31}, // 3
    {0x18, 0x14, 0x12, 0x7f, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3c, 0x4a, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1e}, // 9
    {0x7e, 0x11, 0x11, 0x11, 0x7e}, // A
    {0x7f, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3e, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7f, 0x41, 0x41, 0x22, 0x1c}, // D
    {0x7f, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7f, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3e, 0x41, 0x49, 0x49, 0x7a}, // G
    {0x7f, 0x08, 0x08, 0x08, 0x7f}, // H
    {0x00, 0x41, 0x7f, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3f, 0x01}, // J
    {0x7f, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7f, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7f, 0x02, 0x0c, 0x02, 0x7f}, // M
    {0x7f, 0x04, 0x08, 0x10, 0x7f}, // N
    {0x3e, 0x41, 0x41, 0x41, 0x3e}, // O
    {0x7f, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3e, 0x41, 0x51, 0x21, 0x5e}, // Q
    {0x7f, 0x09, 0x19, 0x29, 0x46}, // R
    {0x46, 0x49, 0x49, 0x49, 0x31}, // S
    {0x01, 0x01, 0x7f, 0x01, 0x01}, // T
    {0x3f, 0x40, 0x40, 0x40, 0x3f}, // U
    {0x1f, 0x20, 0x40, 0x20, 0x1f}, // V
    {0x3f, 0x40, 0x38, 0x40, 0x3f}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x00, 0x36, 0x36, 0x00, 0x00}, // :
    {0x23, 0x13, 0x08, 0x64, 0x62}, // %
    {0x08, 0x08, 0x08, 0x08, 0x08}  // -
};

// Converte um caractere ASCII no índice correspondente dentro da matriz 'fonte'.
// Isso permite montar qualquer string (letras, números e símbolos) pixel a pixel no display.
int obter_indice(char c) {
    if (c >= '0' && c <= '9') return c - '0' + 1;   // Dígitos '0'-'9' mapeiam para os índices 1 a 10
    if (c >= 'A' && c <= 'Z') return c - 'A' + 11;  // Letras 'A'-'Z' mapeiam para os índices 11 a 36
    if (c == ':') return 37;                        // Índice fixo para o símbolo ':'
    if (c == '%') return 38;                        // Índice fixo para o símbolo '%'
    if (c == '-') return 39;                        // Índice fixo para o símbolo '-'
    return 0;  // Qualquer caractere não mapeado (ex: espaço) retorna o índice do espaço em branco
}

// ============================================================================
// Funções Controladoras do Barramento I2C (Display OLED SSD1306)
// ============================================================================

// Envia um único byte de comando (não de dados) para o controlador SSD1306 via I2C.
void enviar_comando_oled(uint8_t comando) {
    i2c_cmd_handle_t link = i2c_cmd_link_create();                                      // Cria um novo "link" (fila de instruções I2C)
    i2c_master_start(link);                                                             // Insere a condição de START no barramento
    i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);         // Endereço do display + bit de escrita
    i2c_master_write_byte(link, 0x00, true);                                            // Byte de controle 0x00 = "os próximos bytes são comandos"
    i2c_master_write_byte(link, comando, true);                                         // O comando propriamente dito
    i2c_master_stop(link);                                                              // Insere a condição de STOP
    i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));                      // Executa a transação com timeout de 10ms
    i2c_cmd_link_delete(link);                                                          // Libera a memória do link
}

// Envia a sequência de inicialização padrão do SSD1306 (liga o display, define endereçamento, contraste etc.)
void inicializar_tela_oled() {
    uint8_t config[] = { 0xAE, 0x20, 0x10, 0xB0, 0x00, 0x10, 0x40, 0x81, 0x7F, 0xA0, 0xC0, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3, 0x00, 0xD5, 0x80, 0xD9, 0x22, 0xDA, 0x12, 0xDB, 0x20, 0x8D, 0x14, 0xAF };
    for (int i = 0; i < sizeof(config); i++) enviar_comando_oled(config[i]); // Envia cada byte da sequência, um a um
}

// Apaga todo o conteúdo do display, escrevendo 0x00 (pixels apagados) em todas as 8 páginas de 128 colunas.
void limpar_tela_oled() {
    for (uint8_t pagina = 0; pagina < 8; pagina++) {
        enviar_comando_oled(0xB0 + pagina); // Seleciona a página (linha) atual (0xB0 a 0xB7)
        enviar_comando_oled(0x00);          // Define o nibble baixo da coluna inicial (coluna 0)
        enviar_comando_oled(0x10);          // Define o nibble alto da coluna inicial (coluna 0)

        i2c_cmd_handle_t link = i2c_cmd_link_create();
        i2c_master_start(link);
        i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(link, 0x40, true);                       // Byte de controle 0x40 = "os próximos bytes são dados de pixel"
        for (int i = 0; i < 128; i++) i2c_master_write_byte(link, 0x00, true); // Escreve 128 colunas zeradas (apagadas)
        i2c_master_stop(link);
        i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(link);
    }
}

// Desenha um único caractere na posição de coluna atual do display, usando os 5 bytes da matriz 'fonte'.
void desenhar_caractere(char caractere) {
    int indice = obter_indice(caractere); // Traduz o caractere no índice da matriz de fonte
    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(link, 0x40, true);                                    // Indica que virão dados de pixel
    for (int i = 0; i < 5; i++) i2c_master_write_byte(link, fonte[indice][i], true); // Envia as 5 colunas do caractere
    i2c_master_write_byte(link, 0x00, true);                                    // Coluna extra em branco (espaçamento entre caracteres)
    i2c_master_stop(link);
    i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(link);
}

// Posiciona o cursor na página (linha) desejada e escreve uma string inteira, caractere por caractere.
void escrever_linha_oled(const char* texto, uint8_t pagina) {
    enviar_comando_oled(0xB0 + pagina); // Seleciona a página (0 a 7) onde o texto será escrito
    enviar_comando_oled(0x00);          // Reinicia a coluna para o início (nibble baixo)
    enviar_comando_oled(0x10);          // Reinicia a coluna para o início (nibble alto)
    while (*texto) {           // Percorre a string até encontrar o terminador nulo
        desenhar_caractere(*texto);
        texto++;
    }
}

// ============================================================================
// Função Principal (app_main)
// ============================================================================
void app_main(void)
{
    char texto_modo[32];  // Buffer para a linha "MODO: ATIVO/TRAVA" exibida no OLED
    char texto_vel[32];   // Buffer para a linha "VEL: XXX HZ" exibida no OLED

    // ----- Configuração I2C (OLED) -----
    i2c_config_t config_i2c = {
        .mode = I2C_MODE_MASTER,              // ESP32 atua como mestre do barramento
        .sda_io_num = PINO_SDA,               // Pino físico usado como linha SDA
        .sda_pullup_en = GPIO_PULLUP_ENABLE,  // Habilita pull-up interno em SDA (padrão I2C)
        .scl_io_num = PINO_SCL,               // Pino físico usado como linha SCL
        .scl_pullup_en = GPIO_PULLUP_ENABLE,  // Habilita pull-up interno em SCL (padrão I2C)
        .master.clk_speed = 400000,           // Velocidade do barramento: 400 kHz (modo Fast I2C)
    };
    i2c_param_config(BARRAMENTO_I2C, &config_i2c);                 // Aplica a configuração à porta I2C escolhida
    i2c_driver_install(BARRAMENTO_I2C, config_i2c.mode, 0, 0, 0);  // Instala o driver I2C (sem buffers de RX/TX, pois é modo mestre)
    inicializar_tela_oled();  // Envia a sequência de inicialização do SSD1306
    limpar_tela_oled();       // Limpa qualquer conteúdo residual do display

    // ----- Configuração ADC (Potenciômetro) -----
    adc1_config_width(ADC_WIDTH_BIT_12);                             // Define resolução de 12 bits (leituras de 0 a 4095)
    adc1_config_channel_atten(PINO_POTENCIOMETRO, ADC_ATTEN_DB_12);  // Define atenuação para ler toda a faixa de 0 a ~3.3V

    // ----- Configuração do Botão (Interrupção) -----
    gpio_config_t config_botao = {
        .pin_bit_mask = (1ULL << PINO_BOTAO),   // Máscara de bits selecionando apenas o pino do botão
        .mode = GPIO_MODE_INPUT,                // Pino configurado como entrada
        .pull_up_en = GPIO_PULLUP_ENABLE,       // Pull-up interno habilitado (botão liga o pino ao GND quando pressionado)
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  // Pull-down desabilitado (não é necessário, já há pull-up)
        .intr_type = GPIO_INTR_NEGEDGE          // Interrupção disparada na borda de descida (nível alto -> baixo, ao pressionar)
    };
    gpio_config(&config_botao);                                 // Aplica a configuração ao pino do botão
    gpio_install_isr_service(0);                                // Inicializa o serviço de interrupções de GPIO da ESP32
    gpio_isr_handler_add(PINO_BOTAO, botao_isr_handler, NULL);  // Associa a função de callback à interrupção deste pino

    // ----- Configura Pino de Direção (DIR) do Motor -----
    gpio_reset_pin(PINO_DIR);                        // Reseta o pino para o estado padrão (remove configurações antigas)
    gpio_set_direction(PINO_DIR, GPIO_MODE_OUTPUT);  // Define o pino como saída digital
    gpio_set_level(PINO_DIR, 1);                     // Fixa o sentido de rotação (nível alto = sentido horário, por exemplo)

    // ----- A SOLUÇÃO: Configuração da biblioteca LEDC (PWM nativo) -----
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,     // Modo de baixa velocidade (adequado para PWM de propósito geral)
        .duty_resolution = LEDC_TIMER_10_BIT,  // Resolução do duty cycle: 10 bits (0 a 1023)
        .timer_num = LEDC_TIMER_0,             // Usa o timer de hardware nº 0 da LEDC
        .freq_hz = 500,                        // Frequência inicial do PWM: 500 Hz (será sobrescrita no loop)
        .clk_cfg = LEDC_AUTO_CLK               // Deixa a ESP-IDF escolher automaticamente a fonte de clock
    };
    ledc_timer_config(&timer_conf); // Aplica a configuração do timer

    ledc_channel_config_t ch_conf = {
        .gpio_num = PINO_STEP,             // Pino de saída do sinal PWM (conectado ao STEP do A4988)
        .speed_mode = LEDC_LOW_SPEED_MODE, // Deve corresponder ao speed_mode do timer associado
        .channel = LEDC_CHANNEL_0,         // Canal de PWM nº 0
        .timer_sel = LEDC_TIMER_0,         // Amarra este canal ao timer configurado acima
        .duty = 0,                         // Duty cycle inicial: 0% (motor começa parado)
        .hpoint = 0                        // Ponto de disparo do pulso dentro do período (0 = início do ciclo)
    };
    ledc_channel_config(&ch_conf); // Aplica a configuração do canal, ligando efetivamente a GPIO 19 ao timer

    // ----- Loop Infinito (FreeRTOS Task) -----
    while (1) {
        int leitura_adc = adc1_get_raw(PINO_POTENCIOMETRO); // Lê o valor bruto do potenciômetro (0 a 4095)

        // Mapeia ADC (0 a 4095) para Frequência (10Hz a 1000Hz)
        // Regra de três: proporção da leitura multiplicada pela faixa de 990Hz, somada ao piso de 10Hz
        uint32_t frequencia = 10 + ((leitura_adc * 990) / 4095);

        if (!sistema_ativo) {
            // TRAVA ATIVADA: Desliga o PWM
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);   // Define duty cycle 0% (sem pulso, motor parado)
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);   // Aplica a mudança de duty imediatamente
        } else {
            // MOTOR ATIVO: Duty Cycle em 50% e atualiza frequência em tempo real
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512); // 512/1024 = 50% de duty cycle (onda quadrada simétrica)
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);   // Aplica o novo duty
            ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, frequencia); // Atualiza a frequência do timer (velocidade do motor)
        }

        // Imprime no Console Serial (Debug)
        printf("MODO: %s | VELOCIDADE: %lu Hz\n", sistema_ativo ? "ATIVO " : "TRAVA ", frequencia);

        // Prepara e Imprime as strings na Tela OLED
        snprintf(texto_modo, sizeof(texto_modo), "MODO: %s  ", sistema_ativo ? "ATIVO " : "TRAVA ");

        if (!sistema_ativo) {
            snprintf(texto_vel, sizeof(texto_vel), "VEL: 0 HZ      "); // Exibe 0 Hz quando travado, independente da leitura do ADC
        } else {
            snprintf(texto_vel, sizeof(texto_vel), "VEL: %lu HZ    ", frequencia);
        }

        escrever_linha_oled(texto_modo, 2); // Escreve o status (ATIVO/TRAVA) na página 2 do display
        escrever_linha_oled(texto_vel, 5);  // Escreve a velocidade atual na página 5 do display

        // Libera a CPU para as outras tarefas do Sistema Operacional (FreeRTOS)
        vTaskDelay(pdMS_TO_TICKS(150)); // Aguarda 150ms antes da próxima leitura/atualização (também suaviza o refresh do OLED)
    }
}
