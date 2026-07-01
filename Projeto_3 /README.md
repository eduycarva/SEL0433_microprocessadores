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



