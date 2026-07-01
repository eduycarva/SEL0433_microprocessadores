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
A montagem do circuito é uma simples conexão dos terminais do led com o ESP32, passando por resistores de 220 $Omega$ para limitar a corrente. O esquemático da montagem é vista na figura abaixo.
<img width="816" height="552" alt="image" src="https://github.com/user-attachments/assets/ae228ce4-7c5a-42aa-a167-ee6b4af84c37" />
