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

