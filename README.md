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
O programa entra no ```LOOP_PRINCIPAL``` onde fica verificando a chave com a ```ACALL VERIFICA_CHAVE``` e atualiza o numero de voltas no display com ```ACALL ATUALIZA_DISPLAY```.

Com o ```VERIFICA_CHAVE```, o programa lê o pino ```P2.0``` e se estiver em ```0```, ele verifica a flag ```F0```. Caso essa flag já for zero, não realiza nada, mas se for ```1```, ele zera ```F0``` e chama a call ```MUDANCA_DIRECAO``` para inverter o sentido de giro do motor. O processo é semelhante quando ```P2.0``` já estiver em 1.

```VERIFICA_CHAVE:```  
    ```JB P2.0, CHAVE_EM_1```  
```CHAVE_EM_0:```  
    ```JNB F0, FIM_VERIFICA```  
    ```CLR F0```  
    ```ACALL MUDANCA_DIRECAO```   
    ```SJMP FIM_VERIFICA```  
```CHAVE_EM_1:```  
    ```JB F0, FIM_VERIFICA```   
    ```SETB F0```  
    ```ACALL MUDANCA_DIRECAO```  
```FIM_VERIFICA:```  
    ```RET```  
Quando a mudança de direção é acionada pela call ```MUDANCA_DIRECAO```, é chamada a call ```ATUALIZA_MOTOR``` para comutar os pinos de controle do motor. Além disso, é chamado também o ```ACALL REINICIA_TIMER``` para zerar o timer1 logo o contador, fazendo o reset automático com a inversão de sentido de giro do motor.

## Sentido de giro do motor

Com essa rotina é possível controlar o sentido de giro do motor com os pinos ```P3.0``` e ```P3.1```. Ele verifica a flag ```F0```, se ```F0``` for 0 (SENTIDO_0): ele seta o bit ```SETB P3.0``` e reseta o bit ```CLR P3.1``` assim configurando o motor para um sentido de rotação. Para o outro sentido de rotação, quando ```F0``` é 1 (SENTIDO_1), o processo inverso é feito, reseta o ```CLR P3.0``` e seta o ```SETB P3.1``` para levar o motor a girar no outro sentido.

```ATUALIZA_MOTOR:```  
    ```JB F0, SENTIDO_1```  
```SENTIDO_0:```  
    ```SETB P3.0```  
    ```CLR P3.1```   
    ```RET```  
```SENTIDO_1:```  
    ```CLR P3.0```  
    ```SETB P3.1```  
    ```RET```  

## Display de 7 segmentos
Por fim, a rotina ```ATUALIZA_DISPLAY``` pega o valor atual do ```CONTADOR``` e usa a tabela (TAB7SEG) para encontrar o código em binário que é correspondente ao dígito para o display de 7seg e envia para a porta ```P1```.

```ATUALIZA_DISPLAY:```  
    ```MOV A, CONTADOR```  
    ```MOV DPTR, #TAB7SEG```  
    ```MOVC A, @A+DPTR```  
    ```MOV P1, A```  
    ```RET```  
Para o controle do ponto decimal que usa a mesma flag ```F0``` para decidir se o ponto decimal acende o apaga de acordo com o sentido de giro do motor. Se ```F0``` for ```0```, o ponto apaga com ```SETB ACC.7``` e se for ```1```, o ponto decimal acente com ```CLR ACC.7```.

Ao fim do código tem a tabela de 7 segmentos ```TAB7SEG``` que é uma tabela de dados na memória de programa ```ORG 0200h``` que armazena os códigos binários pra exibir de 0 a 9 no display de 7 segmentos.
