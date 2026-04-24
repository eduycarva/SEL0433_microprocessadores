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

## Inicio do código  
O programa começa definindo a variável do contador na memória RAM na posição 30h, que serve para guardar o numero de voltas do motor, indo de 0 a 9 voltas. Após isso, o programa pula para a rotina INICIO.

```CONTADOR EQU 30H ```  
```ORG 0000h```  
```SJMP INICIO```  

Na rotina INICIO, configura-se a pilha ```SP, #40h```, com ```F0``` a flag que armazena e indica o sentido de rotação do motor. Zera-se o contador de voltas com ```MOV CONTADOR, #0``` e com ```ACALL ATUALIZA_MOTOR``` garante o primeiro sentido de giro do motor.

## Configuração do Timer 1
Nesta parte o codigo configura o Timer1 no modo 2 com ```MOV TMOD, #60h``` onde expressa um contador de 8 bits com auto-reload, carregando ```0FFh``` em ```TH1``` e ```Tl1```

```MOV TMOD, #60h```  
```MOV TH1, #0FFh```  
```MOV TL1, #0FFh```  

De modo geral, a cada pulso que o motor gera, o ```TL1``` incrementa. Como ele começa do ```0FFh```, o primeiro pulso faz o contador "estourar" e voltar para o ```00h```, gerando uma interrupção, que é usada usada para contar uma volta completa.
As interrupções são habilitadas com ```SETB ET1``` e ```SETB EA```. Por fim, ```SETB TR1``` liga o Timer1.

## Rotina de interrupção do timer1 (ISR)
Quando há o estouro do timer1, o programa solta o endereço que é o vetor de interrupção. Assim com o ```PUSH ACC``` e ```PUSH PSW``` é salvo o estado atual do acumulador ```ACC``` do ```PSW``` na pilha. Com o ```INC CONTADOR``` é incrementado a variável de voltas. 
Após isso, com o ```MOV A, CONTADOR``` e ```CJNE A, #10, FIM_ISR```, é limitado para o contador ir de 0 a 9 e reiniciando após o próximo pulso. O valor do ```CONTADOR``` é movido para ```A``` e comparado com ```#10```. Se não for 10, ele vai para o ```FIM_ISR```. Se for 10, ele chama a call ```REINICIA_TIMER``` para zerar a contagem.
Com o ```POP PSW``` e ```POP ACC```, restaura o ```PSW``` e o ```ACC``` para o estado anterior e com o ```RETI``` retorna da interrupção.

## Loop principal
