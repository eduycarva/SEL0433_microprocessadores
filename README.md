# SEL0433_microprocessadores
Este repositório é referente à matéria SEL0433 - Aplicação de microprocessadores - Professor Pedro Oliveira C. Junior.
# Projeto 1: Sistema de dosagem rotativa

SEL0433 - Aplicação de Microprocessadores  
Eduardo Yumoto Carvalheira - 15636150;
Thayson Pereira Alves - 14681087

## Sobre o Projeto
Este projeto consiste no desenvolvimento de um módulo dosador rotativo acionado por motor DC, projetado para operar em uma linha de produção de uma fábrica de parafusos. O sistema foi desenvolvido em linguagem Assembly para a família de microcontroladores 8051.

## Requisitos Implementados (Entrega Final)
O código fornecido atende a todos os requisitos solicitados:
- Contagem e exibição em display de 7 segmentos de 0 a 9.
- Limitação do contador a um ciclo (0 a 9), reiniciando o Timer 1 automaticamente ao atingir 10 eventos.
- Uso de **interrupção** no Timer 1 para verificação do limite do contador e reinicialização.
- Controle de mudança de direção do motor via chave SW.
- Reset automático da contagem de voltas sempre que a direção do motor for invertida.
- Indicação visual do sentido de rotação utilizando o ponto decimal (bit P1.7) do display de 7 segmentos.

## Passo a passo da lógica do programa

O funcionamento do sistema é dividido em blocos:

### Setup
*Configuramos o ponteiro de pilha (stack pointer) em uma área segura
*Zeramos F0 (informação do sentido de rotação do motor) e CONTADOR (informação da quantidade de rotações feitas)
*Timer 1 é configurado no Modo 2, que nos permite utilizar um contador de 8 bits com auto-reload. 
Ao inserir o valor de 255 (0FFh) em TH1 e TL1, basta um pulso do motor para o contador estourar e gerar uma interrupção imediata no microcontrolador, que pula diretamente para 001Bh, o vetor de interrupção de memória. Dentro do bloco, o INC CONTADOR incrementa o valor de contador em 1
*Por fim, as interrupções são habilitadas e o Timer é ligado
