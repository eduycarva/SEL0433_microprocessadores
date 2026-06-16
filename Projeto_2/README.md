# SEL0433_microprocessadores
Este repositório é referente à matéria SEL0433 - Aplicação de microprocessadores - Professor Pedro Oliveira C. Junior.

# Projeto 2: Aferidor de temperatura de forno industrial
Eduardo Yumoto Carvalheira - 15636150; Thayson Pereira Alves - 14681087

## Sobre o Projeto
Este projeto consiste no desenvolvimento de um aferidor de temperatura para forno industrial, utilizando o microcontrolador PIC18F4550 e implementado em linguagem C com o compilador MikroC PRO for PIC. O sistema realiza a leitura analógica de temperatura por meio de um sensor LM35 (simulado por potenciômetro no SimulIDE), exibe continuamente a temperatura em um display LCD e permite ao usuário selecionar entre dois modos de contagem regressiva de tempo (curta duração: 10 s; longa duração: 60 s), iniciada por botão. Um LED sinaliza quando a temperatura ultrapassa 50 °C.

## Requisitos implementados:
O código desenvolvido atende às seguintes necessidades do projeto:
- Exibição da contagem regressiva e da temperatura em display LCD em formatação de XX.X°C
- Leitura analógica da temperatura na faixa de 0 a 100 graus Celsius
- Configuração de um conversor analógico digital utilizando uma tensão externa de 1V
- Contagem de longa (60 segundos) e curta (10 segundos) duração acionada por botão
- Botão de acionamento geral para iniciar simultaneamente a contagem regressiva e a leitura de temperatura
- Formatação de valores sem o uso de dados do tipo float, para economizar memória
- Tratamento do efeitou bouncing (debounce) nos botões
- Inclusão de um LED representando a resistência do forno, configurado para acender em temperaturas maiores que 50°C

## Lógica de implementação do código

## Definição dos pinos do LCD e LED
O código começa mapeando os pinos do display LCD conforme o pinout do Kit EasyPIC v7: os sinais de controle (`RS` e `EN`) e os quatro bits de dados (`D4`–`D7`) são conectados ao PORTD, e as direções dos pinos são configuradas pelos registradores `TRISx` correspondentes.

```c
sbit LCD_RS at LATD0_bit;
sbit LCD_EN at LATD1_bit;
sbit LCD_D4 at LATD2_bit;
// ...
sbit LCD_RS_Direction at TRISD0_bit;
// ...
```
 
O LED que representa a resistência do forno é mapeado em `LATC0`, com direção configurada por `TRISC0`.
 
```c
sbit LED_Resistencia at LATC0_bit;
sbit LED_Resistencia_Direction at TRISC0_bit;
```

## Variáveis globais
 
As variáveis de controle de tempo são declaradas como `volatile` para garantir que o compilador não as otimize, visto que são modificadas dentro de rotinas de interrupção. `tempo_longo` e `tempo_curto` armazenam os segundos restantes em cada modo (-1 indica inatividade). `sub_t1` e `cont_250ms` são contadores auxiliares que acumulam os disparos do TMR1 até completar 1 segundo.
 
```c
volatile short tempo_longo = -1;
volatile short tempo_curto = -1;
volatile unsigned short sub_t1 = 0;
volatile unsigned short cont_250ms = 0;
volatile bit em_execucao;
```
 
As flags `flag_btn0` e `flag_btn1` são setadas na ISR e tratadas no loop principal, evitando a execução de lógica demorada (como `Delay_ms`) dentro da interrupção. A variável `modo_tempo` indica se o modo selecionado é curto (0) ou longo (1).

## Rotina de Interrupção (ISR)
 
O PIC18F4550 possui um vetor de interrupção único, portanto a função `interrupt()` trata todas as fontes de interrupção verificando os respectivos flags de interrupção pendente.
 
**Interrupções dos botões (INT0 e INT1):** Ao detectar a borda de descida nos pinos `RB0` e `RB1`, as flags `flag_btn0` e `flag_btn1` são setadas e os flags de interrupção (`INT0IF_bit` e `INT1IF_bit`) são limpos manualmente.
 
```c
if (INT0IF_bit) {
    flag_btn0 = 1;
    INT0IF_bit = 0;
}
```
 
**Interrupção do TMR0 (modo longo, base de 1 s):** O TMR0 é recarregado com `0x0BDC` (valor calculado para gerar estouro a cada 1 s com clock de 8 MHz e prescaler de 256). A cada disparo, `tempo_longo` é decrementado. Quando chega a zero, o timer é desligado, `tempo_longo` é resetado para -1 e a flag `em_execucao` é zerada, indicando fim da contagem.
 
```c
if (TMR0IF_bit) {
    TMR0H = 0x0B;
    TMR0L = 0xDC;
    if (tempo_longo > 0) {
        tempo_longo--;
    } else if (tempo_longo == 0) {
        TMR0ON_bit = 0;
        tempo_longo = -1;
        em_execucao = 0;
    }
    TMR0IF_bit = 0;
}
```
 
**Interrupção do TMR1 (modo curto, base de 250 ms):** O TMR1 é configurado para estourar a cada 50 ms (valor de recarga `0x3CB0`). O contador auxiliar `sub_t1` acumula 5 disparos (5 × 50 ms = 250 ms); `cont_250ms` acumula 4 períodos de 250 ms (4 × 250 ms = 1 s), momento em que `tempo_curto` é decrementado. Isso evita recargas longas e mantém a base de tempo precisa sem uso de prescaler elevado.
 
```c
if (TMR1IF_bit) {
    TMR1H = 0x3C;
    TMR1L = 0xB0;
    sub_t1++;
    if (sub_t1 >= 5) {
        sub_t1 = 0;
        cont_250ms++;
        if (cont_250ms >= 4) {
            cont_250ms = 0;
            if (tempo_curto > 0) {
                tempo_curto--;
            } else if (tempo_curto == 0) {
                TMR1ON_bit = 0;
                tempo_curto = -1;
                em_execucao = 0;
            }
        }
    }
    TMR1IF_bit = 0;
}
```
## Formatação da temperatura (`FormatarTemperatura`)
 
Para cumprir o requisito de exibir a temperatura sem uso de `float`, a função `FormatarTemperatura` recebe um valor em ponto fixo com uma casa decimal (ex.: `temp_ponto_fixo = 253` representa 25,3 °C) e monta manualmente uma string de 10 caracteres. Para valores abaixo de 100 °C, extrai dezenas, unidades e décimos por divisão e módulo inteiro. Para 100 °C exato, utiliza a string literal `"100.0°C   "`. O caractere de grau é inserido como código ASCII 223 (compatível com o controlador HD44780).
 
```c
void FormatarTemperatura(unsigned int valor, char *saida) {
    if (valor >= 1000) {
        saida[0]='1'; saida[1]='0'; saida[2]='0'; saida[3]='.';
        saida[4]='0'; saida[5]=223; saida[6]='C';
        // ...
    } else {
        saida[0] = (valor / 100) + '0';
        saida[1] = ((valor % 100) / 10) + '0';
        saida[2] = '.';
        saida[3] = (valor % 10) + '0';
        saida[4] = 223; saida[5] = 'C';
        // ...
    }
}
```

## Montagem da linha 2 do LCD (`MontarLinhaRestam`)
 
A função `MontarLinhaRestam` monta uma string fixa de 16 caracteres (largura do LCD) com o tempo restante. Os primeiros 7 caracteres sempre formam `"Restam:"`. Para valores de dois dígitos, os caracteres 9 e 10 recebem dezenas e unidades; para um dígito, o campo é deslocado à direita com espaço extra. O campo é encerrado com `'s'` e preenchido com espaços até a posição 16 para apagar resíduos de escritas anteriores.
 
```c
void MontarLinhaRestam(short tempo, char *saida) {
    saida[0]='R'; saida[1]='e'; /* ... */ saida[6]=':';
    if (tempo >= 10) {
        saida[9]  = (tempo / 10) + '0';
        saida[10] = (tempo % 10) + '0';
    } else {
        saida[9] = ' ';
        saida[10] = tempo + '0';
    }
    saida[11] = 's';
    saida[12]=' '; saida[13]=' '; saida[14]=' '; saida[15]=' '; saida[16]='\0';
}
```

## Função principal (`main`)
 
**Configurações iniciais:** O registrador `CMCON = 0x07` desabilita os comparadores analógicos do PORTB, liberando os pinos `RB0` e `RB1` para uso como entradas digitais. `TRISB.B0` e `TRISB.B1` são configurados como entradas (botões) e `TRISA.B0` como entrada analógica (canal AN0 do LM35/potenciômetro). O LED tem sua direção configurada como saída e começa apagado.
 
```c
CMCON = 0x07;
TRISB.B0 = 1;
TRISB.B1 = 1;
TRISA.B0 = 1;
LED_Resistencia_Direction = 0;
LED_Resistencia = 0;
```
 
**Inicialização do LCD e ADC:** O LCD é iniciado e o cursor é desligado. O ADC é inicializado pela função de biblioteca `ADC_Init()` e, logo em seguida, `ADCON1 = 0x3A` é configurado **após** a chamada de `ADC_Init` para corrigir um comportamento conhecido do MikroC: a biblioteca força os bits 4 e 5 de `ADCON1` para 0, o que selecionaria VDD/VSS como referência; o valor `0x3A` configura corretamente `AN2` e `AN3` como tensões de referência externas (Vref+ = 1 V), adequando-se à sensibilidade do LM35.
 
```c
ADC_Init();
ADCON1 = 0x3A;
```
 
**Habilitação das interrupções:** As bordas de descida dos botões são selecionadas com `INTEDG0_bit = 0` e `INTEDG1_bit = 0`. As interrupções individuais de INT0, INT1, TMR0 e TMR1 são habilitadas, assim como o bit de habilitação de periféricos (`PEIE_bit`) e o bit global de interrupções (`GIE_bit`).
 
```c
INTEDG0_bit = 0; INTEDG1_bit = 0;
INT0IE_bit = 1;  INT1IE_bit = 1;
TMR0IE_bit = 1;  TMR1IE_bit = 1;
PEIE_bit = 1;    GIE_bit = 1;
```
 
**Loop principal — tratamento dos botões:** A cada iteração do loop infinito, as flags de botão são verificadas. Se `flag_btn0` estiver setada, aplica-se um debounce de 20 ms e confirma-se o nível lógico do pino. Se o botão ainda estiver pressionado e nenhuma contagem estiver em curso (`!em_execucao`), o modo é alternado entre curto e longo. O processo é análogo para `flag_btn1`: ao confirmar o pressionamento, `em_execucao` é setado, os contadores auxiliares são zerados e o timer correspondente ao modo selecionado é configurado e ligado, enquanto o outro é desligado.
 
```c
if (flag_btn0) {
    Delay_ms(20);
    if (PORTB.B0 == 0 && !em_execucao) {
        modo_tempo = !modo_tempo;
    }
    flag_btn0 = 0;
}
if (flag_btn1) {
    Delay_ms(20);
    if (PORTB.B1 == 0 && !em_execucao) {
        em_execucao = 1;
        if (modo_tempo == 0) {
            tempo_curto = 10;
            // configura e liga TMR1, desliga TMR0
        } else {
            tempo_longo = 60;
            // configura e liga TMR0, desliga TMR1
        }
    }
    flag_btn1 = 0;
}
```
 
**Leitura do sensor e controle do LED:** O valor ADC de 10 bits é lido com `ADC_Get_Sample(0)` (canal AN0). A conversão para temperatura em ponto fixo com uma casa decimal é feita pela expressão `(adc_val * 1000) / 1023`, onde o resultado representa décimos de grau Celsius considerando Vref = 1 V e a sensibilidade do LM35 de 10 mV/°C. Se a temperatura superar 500 (50,0 °C), o LED da resistência é aceso; caso contrário, é apagado.
 
```c
adc_val = ADC_Get_Sample(0);
temp_ponto_fixo = ((unsigned long)adc_val * 1000) / 1023;
if (temp_ponto_fixo > 500) {
    LED_Resistencia = 1;
} else {
    LED_Resistencia = 0;
}
```
 
**Atualização do display LCD:** Para evitar o deslocamento do cursor interno do controlador HD44780 causado por múltiplas chamadas parciais de `Lcd_Out`, cada linha do display é composta como uma string completa e escrita de uma única vez. A linha 1 exibe sempre `"Temp: XX.X°C    "`. A linha 2 exibe o tempo restante com `MontarLinhaRestam` quando `em_execucao` está ativo, ou o modo selecionado (`"Modo: 10s       "` ou `"Modo: 60s       "`) quando em standby. Ao final do loop, um `Delay_ms(100)` limita a taxa de atualização do display.
 
```c
Lcd_Out(1, 1, "Temp: ");
Lcd_Out(1, 7, txt_temp);
 
if (em_execucao) {
    MontarLinhaRestam(tempo_mostrar, linha2);
    Lcd_Out(2, 1, linha2);
} else {
    if (modo_tempo == 0)
        Lcd_Out(2, 1, "Modo: 10s       ");
    else
        Lcd_Out(2, 1, "Modo: 60s       ");
}
Delay_ms(100);
```
