# SEL0433_microprocessadores
Este repositório é referente à matéria SEL0433 - Aplicação de microprocessadores - Professor Pedro Oliveira C. Junior.

# Projeto 3: Controle PWM de um LED RGB (Parte 1) e Controle PWM de motores (Parte 2)
Eduardo Yumoto Carvalheira - 15636150; Thayson Pereira Alves - 14681087

## Sobre o Projeto
O projeto 3 foi dividido em 2 partes:
- Parte 1: Controle PWM de LED RGB: Fou solicitado a implementação de 3 canais PWM indeoendentes com a biblioteca LEDC, um para cada intensidade de cor do led RGB, representando o espectro de cores Red, Green e Blue. O sistema varia automaticamaticamente com incrementos pré definidos e com monitoramento via a interfarce UART.
- Parte 2: Controle PWM aplicado a servomotores: Com a utilizaação de servomotores e as bibliotecas ESP32Servo e MCPWM, será realizado o controle avançado de motores por acionamento PWM.

Todas essas atividades foram testadas em simulação utilizando o Wokwi, com a montagem virtual dos circuitos, testes e análise dos resultados, utilizando a ESP32, para demonstrar a aplicação prática dos conceitos de sistemas embarcados, controle de dispositivos via PWM e o monitoramento via UART para uC de 32 bits.

## Parte 01 :Controle PWM de um LED RGB
A parte 01 contempla o o controle de um LED RGB com os seguintes requeisitos:
- Controle PWM de um LED RGB de catodo comum conectado da ESP32 por meio de resistores de 220 $\Omega$.
- Cada terminal corresponde às cores primárias do led RGB (no inglês, Vermelho, Verde e Azul).
- Resolução mínima de 8 bits e frequência do clock de 5kHz.
- O PWM de cada cor do led deve ser incrementado por valores pré definidos.
- Utilização da interface serial UART com informações do incremento e o duty cycles de cada canal.

# Hardware no Wokwi
A montagem do circuito é uma simples conexão dos terminais do led com o ESP32, passando por resistores de 220 $\Omega$ para limitar a corrente e o pino de catodo comum do LED RGB no terminal GND do ESP32. O esquemático da montagem é vista na figura abaixo.
<img width="816" height="552" alt="image" src="https://github.com/user-attachments/assets/ae228ce4-7c5a-42aa-a167-ee6b4af84c37" />

# Controle do LED RGB via PWM
O código começa com a inclução das bibliotecas básicas:  

```<stdio.h>``` Comando básicos e necessários para códigos em C.  
```<freertos/FreeRTOS.h>``` Sistema operacional do ESP32.  
```freertos/task.h``` Controle das funções do ESP32.  
```driver/ledc.h``` Controle de LEDs e sinais PWM no ESP32.  

Na sequência, variáveis das cores do led RGB são definidas uma pra cada pino do uC:  
``` #define R 23```  
``` #define G 22```  
``` #define B 21```  

Dentro do loop principal ```app_main(void) ``` temos a parte principal dos PWM, começando pela configuração do temporizador:

```ledc_timer_config_t timer = {```  
    ```.speed_mode = LEDC_LOW_SPEED_MODE,``` Define o modo de operação em baixa frequência  
    ```.duty_resolution = LEDC_TIMER_8_BIT,``` Resolução de 8 bits (0 a 255)  
    ```.timer_num = LEDC_TIMER_0,``` Temporizador da ESP32 em 0  
    ```.freq_hz = 5000,``` Frequencia  
    ```.clk_cfg = LEDC_AUTO_CLK```}; Fonte do relógio  
    
```ledc_timer_config(&timer);``` ativação no ESP32  

```ledc_channel_config_t canal = {```  
    ```.speed_mode = LEDC_LOW_SPEED_MODE,```  
    ```.timer_sel = LEDC_TIMER_0,``` Timer 0  
    ```.duty = 0,``` Estado inicial  
    ```.hpoint = 0};```  

Em seguida, acontece a configuração das cores RGB:

```canal.channel = LEDC_CHANNEL_0;``` Canal 0  
```canal.gpio_num = R;``` Pino 23  
```ledc_channel_config(&canal);```   

```canal.channel = LEDC_CHANNEL_1;``` Canal 1  
```canal.gpio_num = G;``` Pino 22  
```ledc_channel_config(&canal);```  

```canal.channel = LEDC_CHANNEL_2;``` Canal 2
```canal.gpio_num = B;```  Pino 21
```ledc_channel_config(&canal);``` 

Após a configuração dos PWMs, é feito a definição e o incremento:  

Estado inicial das cores:
```int vermelho = 0;```
```int verde = 0;```
```int azul = 0;```

Dentro do loop ```while (1)```, é definido o incremento de cada cor a cada ciclo:

```vermelho += 15;```  
    ```verde += 5;```   
    ```azul += 10;```   

Em seguida, é feito o teto do incremento para 8 bits (0 a 255). A lógica é que se a cor ultrapassa o valor de 155, ela volta ao estado inicial:  

```if (vermelho > 255)  vermelho = 0; ```   
  ```  if (verde > 255)     verde = 0; ```   
  ```  if (azul > 255)      azul = 0;  ```  

  Em seguida, é feito a atualização dos estados para os pinos de saída do PWM para cada cor:    
        ```ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, vermelho);```  
       ``` ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);```  
        ```  ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1, verde); ```  
       ``` ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_1); ```  
       ```   ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2, azul);```  
       ```   ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_2);```    
       
Por fim, é feito a leitura para a porta seriaç (UART) como uma forma de telemetria, mostrando os valores de incremento nos valores das cores e a intensidade dos leds, vairando de 0 a 100 %, após isso, no final, um delay de 200 ms é aplicado para dar passos à transição das cores.  

```printf("Incrementos: R=15 G=5 B=10\n");```  
```printf("Duty -> R:%d%% G:%d%% B:%d%%\n\n", (vermelho * 100) / 255, (verde * 100) / 255, (azul * 100) / 255);```  
```vTaskDelay(pdMS_TO_TICKS(200));```  

Deste modo, foi implementado o controle PWM de um LED RGB utilizando a ESP32 com frequência de 5 kHz e resolução de 8 bits. Cada cor foi direcionada porum PWM independente, permitindo o controle individual do brilho. Os duty cycles variam continuamente entre 0 e 255, utilizando incrementos diferentes para cada cor. AS imagens abaixo mostra a operação do LED quando apenas um canal de cor é acionado em 100% (ficando vermelho, verde ou azul na intensidade máxima) e também na operação de misturar as cores de acordo com uma interação qualquer:

<img width="852" height="725" alt="image" src="https://github.com/user-attachments/assets/16d308c8-7ff7-4b5b-981e-cfc52c8ae795" />
<img width="817" height="685" alt="image" src="https://github.com/user-attachments/assets/5708bd03-fe91-4546-bdbf-fac64ad5ee3d" />
<img width="833" height="713" alt="image" src="https://github.com/user-attachments/assets/fa4beebf-34ee-4c51-95dc-51c3c131abd0" />

<img width="838" height="726" alt="image" src="https://github.com/user-attachments/assets/940498c8-e418-45d8-a31f-a25ed8501b52" />

## Parte 02 :Controle PWM de motores
A parte 02 contempla o o controle do servo motor com dois exercícios:
- Controle PWM do servomotor com o incremento de forma manual por um potenciômetro.
- Aplicação própria de controle PWM com a biblioteca MCPWM.

# Controle PWM de um servo motor
Para essa estapa do projeto, a montagem do hardware é mostrado na figura abaixo
<img width="695" height="406" alt="image" src="https://github.com/user-attachments/assets/7945d2ee-5537-4138-9c91-ccee778895ed" />

O potenciômetro é ligado ao pino 35, utilizado como um ADC para converter o sinal analógico em digital a ser interpretado pelo uC. Assim, o pino de controle via PWM é determinado pelo pino 22, sendo levado ao servomotor para ser realizado o controle de posição. Todos esses elementos ( o potenciômetro e o servomotor) também possuem os pinos de VCC e GND alimentados.

# Controle do servomotor via PWM

O código começa com a inicialização das bibliotecas básicas e a definição do pino ADC ligado ao potenciòmetro e a saída PWM ao servomotor:    

```#include <stdio.h>```: Biblioteca básica em C   
```#include "freertos/FreeRTOS.h"``` Sistema operacional do ESP32  
```#include "freertos/task.h"``` Gerencia o tempo e funções  
```#include "driver/ledc.h"``` Controle e configuração do PWM  
```#include "driver/adc.h"``` Serve para leitura das tensões analógicas  

```#define SERVO_PIN 22```  Pino de saída para o servomotor  
```#define POT_PIN   ADC1_CHANNEL_7```  Canal analógico para o adc

Em seguida, dentro do loop  ```app_main``` segue a configuração dos temporizadores utilizados pelo PWM  

```ledc_timer_config_t timer_servo = {```  
       ``` .speed_mode = LEDC_LOW_SPEED_MODE,``` Temporizador  
      ```  .duty_resolution = LEDC_TIMER_10_BIT, ``` Resolucao de 10 bits  
       ``` .timer_num = LEDC_TIMER_0,``` Timer 0  
      ```  .freq_hz = 50,``` 50 Hz                  
      ```  .clk_cfg = LEDC_AUTO_CLK };``` Clock interno  
    ```ledc_timer_config(&timer_servo);``` Configuração dos registradores  
    ```ledc_channel_config_t canal_servo = {```  
        ```.speed_mode = LEDC_LOW_SPEED_MODE,```Alinha o timer em baixa frequência  
        ```.timer_sel = LEDC_TIMER_0,``` Timer 0  
        ```.channel = LEDC_CHANNEL_0,``` Canal 0  
        ```.gpio_num = SERVO_PIN,``` Saída do PWM no GPIO 22  
        ```.duty = 0,``` Estado inicial  
        ```.hpoint = 0};```  
    ```ledc_channel_config(&canal_servo);``` Aplica as configurações do servomotor definidas a cima  
    ```adc1_config_width(ADC_WIDTH_BIT_10);``` Valores do potenciômetro de 0 a 1023 (10 bits)  
    ```adc1_config_channel_atten(POT_PIN, ADC_ATTEN_DB_12);```    
  
 No  último loop definido pelo ```while (1)```, o código faz os seguintes passos:     
       ```leitura = adc1_get_raw(POT_PIN);``` Leitura do potenciômetro em binário (0 a 1023)   
       ``` duty_servo = 25 + ((leitura * (125 - 25)) / 1023);``` Converte a leitura do pot em largura de pulso do servo   
        ```ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_servo);``` Grava no canal 0 o dc do servomotor   
       ``` ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);``` Aplica o novo valor de dc   
       ``` int angulo = (leitura * 180) / 1023;``` Converte o valor em binario do pot em angulo   
       ``` printf("Pot: %d e Angulo (graus): %d\n", leitura, angulo);``` Leituras UART   
       ``` vTaskDelay(100 / portTICK_PERIOD_MS);``` delay de 100 ms    

Dessa forma, a posição do servomotor varia de acordo com o potencômetro, como mostra os dois exemplos nas figuras abaixo: 
<img width="860" height="715" alt="image" src="https://github.com/user-attachments/assets/afb7e407-068b-484b-99d0-89bca4c5d30d" />  

<img width="881" height="720" alt="image" src="https://github.com/user-attachments/assets/d350ff7d-2d8f-4b2e-80f7-b29be90541a3" />  

# Controle Avançado de Motor de Passo com ESP32 e FreeRTOS

Este repositório contém o código e a documentação referentes ao **Exercício 2 da Parte 2** do Projeto 3 (SEL0433 - Aplicação de Microprocessadores): um sistema de controle de Motor de Passo Bipolar utilizando um driver A4988 e um microcontrolador ESP32. O projeto integra leitura analógica (ADC), interrupção externa por hardware, comunicação I2C com display OLED e modulação de PWM em tempo real, desenvolvido diretamente sobre o ESP-IDF.

Conforme solicitado no enunciado, o projeto foi desenvolvido com base na biblioteca nativa MCPWM. Durante os testes de simulação, no entanto, essa biblioteca apresentou uns bugs com o Wokwi, detalhado a seguir. Por esse motivo a geração de PWM foi migrada para a biblioteca LEDC, mantendo-se o restante da arquitetura sem alterações. Os dois códigos estão documentados neste repositório para registro.

## Arquitetura 
- GPIO 34: Potenciômetro para controle dinâmico da velocidade.
- GPIO 21: Comunicação I2C para o Display OLED.
- GPIO 19: Pino `STEP` conectado ao driver, responsável por enviar os pulsos.
- GPIO 25 (OUTPUT): Pino `DIR` que define o sentido de rotação.
  
## Bug do MCPWM

## Tentativa inicial: biblioteca `driver/mcpwm.h`
Seguindo a recomendação do enunciado, a primeira versão do projeto foi implementada com a biblioteca nativa `driver/mcpwm.h`, que é o módulo de PWM da ESP32 voltado especificamente para controle de motores (arquivo `mcpwm_versao_original.c`). A fiação, o driver A4988, o ADC, a ISR do botão e o display OLED eram exatamente os mesmos da versão final - a única diferença estava no bloco responsável por gerar o sinal de `STEP`.

**Como isolamos o problema:**
Para confirmar que o problema estava na biblioteca MCPWM dentro do simulador, foi feito um teste bruto, no qual o pino `STEP` é alternado manualmente em nível alto/baixo via `gpio_set_level()`, sem uso de nenhuma biblioteca de PWM. Esse teste funcionou perfeitamente no Wokwi: o motor girou de forma consistente, confirmando que:
- A fiação entre ESP32, driver A4988 e motor de passo estava correta;
- O pino `DIR` e a lógica de sentido de rotação estavam corretos;
- O problema estava isolado especificamente na geração de PWM via `mcpwm_set_frequency()` dentro do ambiente de simulação, e não no restante do software do projeto.

**A Solução:**
Com o problema devidamente isolado, a aplicação foi refatorada substituindo a arquitetura de modulação para o **LEDC** (`driver/ledc.h`). 

## Dissecando o Código Principal (Solução Final com LEDC)

Abaixo está o código definitivo do projeto, perfeitamente operacional, explicado bloco a bloco para total compreensão da arquitetura construída.

### Bloco 1: Importações e Definições de Hardware
Importamos os módulos do FreeRTOS (sistema operacional de tempo real da ESP32) e as bibliotecas nativas de todos os periféricos necessários. Também mapeamos os pinos físicos em macros para facilitar a manutenção do código.

```c
#include <stdio.h>             
#include <string.h>          
#include "freertos/FreeRTOS.h"  
#include "freertos/task.h"    
#include "driver/adc.h"         // Driver do conversor AD
#include "driver/gpio.h"           // Driver de GPIO 
#include "driver/i2c.h"           // Driver do barramento I2C 
#include "driver/ledc.h"     // Modulador PWM nativo da ESP32 
 
// Definições de Pinos e Constantes I2C
#define PINO_POTENCIOMETRO ADC1_CHANNEL_6 // Canal ADC1 ligado ao potenciômetro
#define PINO_STEP      19        // Pino que gera o pulso PWM enviado ao driver A4988 
#define PINO_DIR       25           // Pino digital que define o sentido de rotação do motor 
#define PINO_BOTAO     12           // Pino do botão de trava de emergência 
#define PINO_SDA       21       // Linha de dados do barramento I2C 
#define PINO_SCL       22         // Linha de clock do barramento I2C 
 
#define ENDERECO_OLED      0x3C          // Endereço I2C padrão do controlador OLED
#define BARRAMENTO_I2C     I2C_NUM_0    // Usa o periférico I2C número 0 da ESP32
```

### Bloco 2: Variáveis Globais e Interrupção (ISR)
Aqui declaramos variáveis como `volatile` pois elas serão alteradas dentro de uma interrupção de Hardware. A função `botao_isr_handler` é acionada quando o botão é pressionado, independentemente do que o processador esteja fazendo. Para evitar o efeito "bouncing" (onde o contato metálico do botão gera múltiplos acionamentos em milissegundos), usamos um cálculo temporal baseado nos FreeRTOS, ignorando acionamentos duplos em menos de 250ms.

```c
volatile bool sistema_ativo = true;      // Estado do motor (true = girando, false = travado).
volatile uint32_t ultimo_tempo_isr = 0;  // Guarda o tick do último acionamento válido do botão, usado para o debounce.
 

static void IRAM_ATTR botao_isr_handler(void* arg) {
    uint32_t tempo_atual = xTaskGetTickCountFromISR(); // Lê o tick atual do FreeRTOS 
 
    // Debounce por software: só aceita um novo acionamento se já se passaram mais de 250ms desde o último
    if (tempo_atual - ultimo_tempo_isr > pdMS_TO_TICKS(250)) { 
        sistema_ativo = !sistema_ativo;   // Alterna o estado do sistema 
        ultimo_tempo_isr = tempo_atual;   // Atualiza a referência de tempo para o próximo debounce
    }
}
```

### Bloco 3: Mapeamento de Matriz

```c
// Matriz de fonte 5x8: cada linha representa um caractere, e cada byte representa uma coluna de 8 pixels verticais.
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
 
int obter_indice(char c) {
    if (c >= '0' && c <= '9') return c - '0' + 1;   // Dígitos '0'-'9' mapeiam para os índices 1 a 10
    if (c >= 'A' && c <= 'Z') return c - 'A' + 11;  // Letras 'A'-'Z' mapeiam para os índices 11 a 36
    if (c == ':') return 37;                       // Índice fixo para o símbolo ':'
    if (c == '%') return 38;                  // Índice fixo para o símbolo '%'
    if (c == '-') return 39;              // Índice fixo para o símbolo '-'
    return 0; 
}
```

### Bloco 4: Funções Controladoras do Barramento I2C
Este conjunto de funções atua no nível de hardware do protocolo I2C. São construídos lincs de dados preenchidos com as instruções necessárias para ativar e escrever no controlador OLED.

```c
// Envia um único byte de comandopara o controlador SSD1306 via I2C.
void enviar_comando_oled(uint8_t comando) {
    i2c_cmd_handle_t link = i2c_cmd_link_create();                          
    i2c_master_start(link);                                 
    i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);     // Endereço do display + bit de escrita
    i2c_master_write_byte(link, 0x00, true);                        
    i2c_master_write_byte(link, comando, true);                     // O comando propriamente dito
    i2c_master_stop(link);                        // Insere a condição de STOP
    i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));         // Executa a transação com timeout de 10ms
    i2c_cmd_link_delete(link);                        // Libera a memória do link
}
 
// Envia a sequência de inicialização padrão do SSD1306 (liga o display, define endereçamento, contraste etc.)
void inicializar_tela_oled() {
    uint8_t config[] = { 0xAE, 0x20, 0x10, 0xB0, 0x00, 0x10, 0x40, 0x81, 0x7F, 0xA0, 0xC0, 0xA6, 0xA8, 0x3F, 0xA4, 0xD3, 0x00, 0xD5, 0x80, 0xD9, 0x22, 0xDA, 0x12, 0xDB, 0x20, 0x8D, 0x14, 0xAF };
    for (int i = 0; i < sizeof(config); i++) enviar_comando_oled(config[i]); 
}
 
// Apaga todo o conteúdo do display, escrevendo 0x00 em todas as 8 páginas de 128 colunas.
void limpar_tela_oled() {
    for (uint8_t pagina = 0; pagina < 8; pagina++) {
        enviar_comando_oled(0xB0 + pagina); // Seleciona a página  atual (0xB0 a 0xB7)
        enviar_comando_oled(0x00)     // Define o nibble baixo da coluna inicial 
        enviar_comando_oled(0x10);         // Define o nibble alto da coluna inicial
 
        i2c_cmd_handle_t link = i2c_cmd_link_create();
        i2c_master_start(link);
        i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);
        i2c_master_write_byte(link, 0x40, true);                  
        for (int i = 0; i < 128; i++) i2c_master_write_byte(link, 0x00, true); // Escreve 128 colunas zeradas 
        i2c_master_stop(link);
        i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));
        i2c_cmd_link_delete(link);
    }
}
 
// Desenha um único caractere na posição de coluna atual do display, usando os 5 bytes da matriz 
void desenhar_caractere(char caractere) {
    int indice = obter_indice(caractere); // Traduz o caractere no índice da matriz de fonte
    i2c_cmd_handle_t link = i2c_cmd_link_create();
    i2c_master_start(link);
    i2c_master_write_byte(link, (ENDERECO_OLED << 1) | I2C_MASTER_WRITE, true);
    i2c_master_write_byte(link, 0x40, true);                                   // Indica que virão dados de pixel
    for (int i = 0; i < 5; i++) i2c_master_write_byte(link, fonte[indice][i], true); // Envia as 5 colunas do caractere
    i2c_master_write_byte(link, 0x00, true);                                 // Coluna extra em branco 
    i2c_master_stop(link);
    i2c_master_cmd_begin(BARRAMENTO_I2C, link, pdMS_TO_TICKS(10));
    i2c_cmd_link_delete(link);
}
 
// Posiciona o cursor na página (linha) desejada e escreve uma string inteira, caractere por caractere.
void escrever_linha_oled(const char* texto, uint8_t pagina) {
    enviar_comando_oled(0xB0 + pagina); // Seleciona a linha
    enviar_comando_oled(0x00);         // Reinicia a coluna para o início 
    enviar_comando_oled(0x10);        // Reinicia a coluna para o início 
    while (*texto) {           // Percorre a string até encontrar o terminador nulo
        desenhar_caractere(*texto);
        texto++;
    }
}
```

### Bloco 5: Main Setup
O corpo inicial do `app_main` é responsável por instalar os serviços de periféricos. São configurados os atenuadores e a largura de bits do ADC, registrada a ISR do pino do botão e levantada a porta de controle I2C.

```c
void app_main(void)
{
    char texto_modo[32];  
    char texto_vel[32];   
 
    // Configuração I2C (OLED)
    i2c_config_t config_i2c = {
        .mode = I2C_MODE_MASTER,            // ESP32 atua como mestre do barramento
        .sda_io_num = PINO_SDA,          // Pino físico usado como linha SDA
        .sda_pullup_en = GPIO_PULLUP_ENABLE,  // Habilita pull-up interno em SDA
        .scl_io_num = PINO_SCL,            // Pino físico usado como linha SCL
        .scl_pullup_en = GPIO_PULLUP_ENABLE,  // Habilita pull-up interno em SCL
        .master.clk_speed = 400000,         // Velocidade do barramento: 400 kHz
    };
    i2c_param_config(BARRAMENTO_I2C, &config_i2c);         // Aplica a configuração à porta I2C escolhida
    i2c_driver_install(BARRAMENTO_I2C, config_i2c.mode, 0, 0, 0); // Instala o driver I2C 
    inicializar_tela_oled();  // Envia a sequência de inicialização do oled
    limpar_tela_oled();       
 
    // Configuração ADC (Potenciômetro)
    adc1_config_width(ADC_WIDTH_BIT_12);         // Define resolução de 12 bits
    adc1_config_channel_atten(PINO_POTENCIOMETRO, ADC_ATTEN_DB_12);   // Define atenuação para ler toda a faixa de 0 a 3.3v
 
    // Configuração do Botão (Interrupção)
    gpio_config_t config_botao = {
        .pin_bit_mask = (1ULL << PINO_BOTAO),   
        .mode = GPIO_MODE_INPUT,                // Pino configurado como entrada
        .pull_up_en = GPIO_PULLUP_ENABLE,       // Pull-up interno habilitado 
        .pull_down_en = GPIO_PULLDOWN_DISABLE,  
        .intr_type = GPIO_INTR_NEGEDGE          // Interrupção disparada na borda de descida
    };
    gpio_config(&config_botao);                              // Aplica a configuração ao pino do botão
    gpio_install_isr_service(0);                             // Inicializa o serviço de interrupções de GPIO da ESP32
    gpio_isr_handler_add(PINO_BOTAO, botao_isr_handler, NULL); // Associa a função de callback à interrupção deste pino
 
    gpio_reset_pin(PINO_DIR);                       // Reseta o pino para o estado padrão 
    gpio_set_direction(PINO_DIR, GPIO_MODE_OUTPUT); // Define o pino como saída digital
    gpio_set_level(PINO_DIR, 1);                    // Fixa o sentido de rotação 
```

### Bloco 6: PWM paar Motor
Aqui mora a solução do problema de hardware virtual descrito acima. O `LEDC` divide o PWM em duas partes: o Timer e o Canal.

```c
    // Configuração da biblioteca LEDC
    ledc_timer_config_t timer_conf = {
        .speed_mode = LEDC_LOW_SPEED_MODE,   // Modo de baixa velocidade
        .duty_resolution = LEDC_TIMER_10_BIT,  // DC
        .timer_num = LEDC_TIMER_0,             // Timer
        .freq_hz = 500,                       // freq.
        .clk_cfg = LEDC_AUTO_CLK             // Fonte
    };
    ledc_timer_config(&timer_conf); // Aplica a configuração do timer
 
    ledc_channel_config_t ch_conf = {
        .gpio_num = PINO_STEP,        // Pino de saída do sinal pwm
        .speed_mode = LEDC_LOW_SPEED_MODE, // Deve corresponder ao speed_mode do timer associado
        .channel = LEDC_CHANNEL_0,      // Canal do pwm
        .timer_sel = LEDC_TIMER_0,    // Amarra este canal ao timer configuradoacima
        .duty = 0,                   // dc inicial
        .hpoint = 0           // Ponto de disparo do pulso dentro do período 
    };
    ledc_channel_config(&ch_conf); // Aplica a configuração do canal, ligando efetivamente a GPIO 19 ao timer
```

### Bloco 7: Loop 
O laço principal do microcontrolador se repete. É feita a leitura do potenciômetro e sua conversão para uma escala plausível para o motor de passo (10Hz a 1000Hz).
Se o motor estiver ativo, atualizamos a frequência e o duty. Se travado, cortamos o pulso para 0%. A execução é fechada com `vTaskDelay` para poupar a CPU, e o texto é formatado na tela OLED.

```c
    while (1) {
        int leitura_adc = adc1_get_raw(PINO_POTENCIOMETRO); // Lê o valor bruto do potenciômetro (0 a 4095)
       
        // Regra de 3
        uint32_t frequencia = 10 + ((leitura_adc * 990) / 4095);
 
        if (!sistema_ativo) {
            // Modo trava
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 0);   // Define duty cycle = 0
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);   // Aplica a mudança de duty
        } else {
            // modo ativo
            ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, 512); // 512/1024 = 50% de duty cycle
            ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);   // Aplica o novo duty
            ledc_set_freq(LEDC_LOW_SPEED_MODE, LEDC_TIMER_0, frequencia); // Atualiza a frequência do timer
        }
 
        // Imprime no Console Serial
        printf("MODO: %s | VELOCIDADE: %lu Hz\n", sistema_ativo ? "ATIVO " : "TRAVA ", frequencia);
 
        // Prepara e Imprime as strings na Tela OLED
        snprintf(texto_modo, sizeof(texto_modo), "MODO: %s  ", sistema_ativo ? "ATIVO " : "TRAVA ");
        
        if(!sistema_ativo) {
            snprintf(texto_vel, sizeof(texto_vel), "VEL: 0 HZ      "); // Exibe 0 Hz quando travado, independente da leitura do ADC
        } else {
            snprintf(texto_vel, sizeof(texto_vel), "VEL: %lu HZ    ", frequencia);
        }
 
        escrever_linha_oled(texto_modo, 2); // Escreve a página 2 do display
        escrever_linha_oled(texto_vel, 5);  // Escreve a velocidade atual na página 5 do display
 
        vTaskDelay(pdMS_TO_TICKS(150)); // Aguarda 150ms
    }
}
```
---

## Simulação em Funcionamento
 
Abaixo estão os registros da simulação do projeto em funcionamento no Wokwi, evidenciando o sistema completo operando: leitura do potenciômetro alterando a velocidade do motor de passo em tempo real, acionamento da trava de emergência via botão/interrupção, e atualização das informações no display OLED.
 
<img width="943" height="787" alt="ativo" src="https://github.com/user-attachments/assets/3775fc05-606b-4d4f-8013-7fafeed7231a" />
 
<img width="1000" height="768" alt="trava" src="https://github.com/user-attachments/assets/1d82b55b-b46a-45a8-9759-0692595f9a01" />

## Conclusão do Processo

O desenvolvimento deste exercício não foi linear, e justamente por isso ele ilustra bem um processo real de embarcados: implementar, testar, e adaptar a solução conforme as limitações do ambiente disponível. O caminho percorrido pode ser resumido em três etapas:
 
O resultado final é um sistema estável e integralmente funcional em simulação, capaz de controlar a velocidade do motor de passo em tempo real, sinalizar seu estado tanto no display OLED quanto no monitor serial, e reagir de forma imediata ao acionamento da trava de emergência.








 




