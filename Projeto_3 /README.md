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




  
    




