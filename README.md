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

## Lógica de implementação do código
Esta seção contém a explicação do código. Na programa, há pesquenos comentários feitos para entendimento geral.

##Inicio do código
O programa começa definindo a variável do contador na memória RAM na posição 30h, que serve para guardar o numero de voltas do motor, indo de 0 a 9 voltas. Após isso, o programa pula para a rotina INICIO.

```CONTADOR EQU 30H ORG 0000h SJMP INICIO```
