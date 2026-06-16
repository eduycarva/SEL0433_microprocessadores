//==============================================================================
// ENTREGA FINAL - SEL0433
// Thayson Pereira Alves - 14681087
// Eduardo Yumoto Carvalheira - 15636150
//==============================================================================
// Definições de pinos do LCD (Kit EasyPIC v7)
sbit LCD_RS at LATD0_bit;
sbit LCD_EN at LATD1_bit;
sbit LCD_D4 at LATD2_bit;
sbit LCD_D5 at LATD3_bit;
sbit LCD_D6 at LATD4_bit;
sbit LCD_D7 at LATD5_bit;
sbit LCD_RS_Direction at TRISD0_bit;
sbit LCD_EN_Direction at TRISD1_bit;
sbit LCD_D4_Direction at TRISD2_bit;
sbit LCD_D5_Direction at TRISD3_bit;
sbit LCD_D6_Direction at TRISD4_bit;
sbit LCD_D7_Direction at TRISD5_bit;
// Definição do pino do LED (Resistência do Forno)
sbit LED_Resistencia at LATC0_bit;
sbit LED_Resistencia_Direction at TRISC0_bit;
// Variáveis globais de controle de tempo
volatile short tempo_longo = -1;
volatile short tempo_curto = -1;
volatile unsigned short sub_t1 = 0;
volatile unsigned short cont_250ms = 0;
volatile bit em_execucao;
// Flags auxiliares para o tratamento de Bouncing
volatile bit flag_btn0;
volatile bit flag_btn1;
unsigned char modo_tempo = 0;       // 0 = Curta (10s), 1 = Longa (60s)
unsigned int adc_val = 0;
unsigned int temp_ponto_fixo = 0;
// Rotina de Interrupção ultra-rápida (Sem Delays!)
void interrupt() {
    // Interrupção do Botão 1
    if (INT0IF_bit) {
        flag_btn0 = 1;
        INT0IF_bit = 0;
    }
    // Interrupção do Botão 2
    if (INT1IF_bit) {
        flag_btn1 = 1;
        INT1IF_bit = 0;
    }
    // Interrupção do TMR0 (Base de 1s para contagem de 60s)
    if (TMR0IF_bit) {
        TMR0H = 0x0B;
        TMR0L = 0xDC;
        if (tempo_longo > 0) {
            tempo_longo--;
        } else if (tempo_longo == 0) {
            TMR0ON_bit = 0;
            tempo_longo = -1;   // CORREÇÃO: resetar para -1 ao finalizar
            em_execucao = 0;
        }
        TMR0IF_bit = 0;
    }
    // Interrupção do TMR1 (Base de 50ms para contagem de 10s)
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
                    tempo_curto = -1;   // CORREÇÃO: resetar para -1 ao finalizar
                    em_execucao = 0;
                }
            }
        }
        TMR1IF_bit = 0;
    }
}
// ===========================================================================
// Formata a temperatura para o LCD.
// Garante sempre 10 caracteres visíveis (sem depender de buffers antigos).
// Formato: "XX.X°C    " ou "100.0°C   "
// ===========================================================================
void FormatarTemperatura(unsigned int valor, char *saida) {
    if (valor >= 1000) {
        // "100.0°C   " — 10 chars
        saida[0] = '1'; saida[1] = '0'; saida[2] = '0'; saida[3] = '.';
        saida[4] = '0'; saida[5] = 223; saida[6] = 'C';
        saida[7] = ' '; saida[8] = ' '; saida[9] = ' '; saida[10] = '\0';
    } else {
        // "XX.X°C    " — 10 chars
        saida[0] = (valor / 100) + '0';
        saida[1] = ((valor % 100) / 10) + '0';
        saida[2] = '.';
        saida[3] = (valor % 10) + '0';
        saida[4] = 223; // Símbolo de grau (°)
        saida[5] = 'C';
        saida[6] = ' '; saida[7] = ' '; saida[8] = ' '; saida[9] = ' '; saida[10] = '\0';
    }
}
// ===========================================================================
// Monta a string da linha 2 completa (sempre 16 chars) com o tempo restante.
// CORREÇÃO: uma única Lcd_Out por linha evita cursor fora de posição.
// Formato: "Restam:  XXs    " ou "Restam:   Xs    "
// ===========================================================================
void MontarLinhaRestam(short tempo, char *saida) {
    // "Restam:" fixo nos primeiros 7 chars
    saida[0] = 'R'; saida[1] = 'e'; saida[2] = 's'; saida[3] = 't';
    saida[4] = 'a'; saida[5] = 'm'; saida[6] = ':';
    if (tempo < 0) tempo = 0;
    if (tempo >= 10) {
        // dois dígitos: "  XXs   "
        saida[7] = ' ';
        saida[8] = ' ';
        saida[9]  = (tempo / 10) + '0';
        saida[10] = (tempo % 10) + '0';
    } else {
        // um dígito: "   Xs   "
        saida[7] = ' ';
        saida[8] = ' ';
        saida[9] = ' ';
        saida[10] = tempo + '0';
    }
    saida[11] = 's';
    // Preenche até 16 chars com espaços
    saida[12] = ' '; saida[13] = ' '; saida[14] = ' '; saida[15] = ' ';
    saida[16] = '\0';
}
void main() {
    char txt_temp[12];
    char linha2[17];
    // Configurações Iniciais
    CMCON = 0x07;
    TRISB.B0 = 1;
    TRISB.B1 = 1;
    TRISA.B0 = 1;       // Pino AN0 para o LM35
    LED_Resistencia_Direction = 0;
    LED_Resistencia = 0;
    Lcd_Init();
    Lcd_Cmd(_LCD_CLEAR);
    Lcd_Cmd(_LCD_CURSOR_OFF);
    // Vref Externa (RA3 e RA2)
    ADC_Init();
    ADCON1 = 0x3A;
    // Interrupções
    INTEDG0_bit = 0;
    INTEDG1_bit = 0;
    INT0IE_bit = 1;
    INT1IE_bit = 1;
    TMR0IE_bit = 1;
    TMR1IE_bit = 1;
    PEIE_bit = 1;
    GIE_bit = 1;
    em_execucao = 0;
    flag_btn0 = 0;
    flag_btn1 = 0;
    while(1) {
        // --- TRATAMENTO DOS BOTÕES ---
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
                    tempo_longo = -1;
                    sub_t1 = 0;
                    cont_250ms = 0;
                    T1CON = 0x10; TMR1H = 0x3C; TMR1L = 0xB0;
                    TMR1ON_bit = 1; TMR0ON_bit = 0;
                } else {
                    tempo_longo = 60;
                    tempo_curto = -1;
                    T0CON = 0x84; TMR0H = 0x0B; TMR0L = 0xDC;
                    TMR0ON_bit = 1; TMR1ON_bit = 0;
                }
            }
            flag_btn1 = 0;
        }
        // --- LEITURA DO SENSOR E CONTROLE DO FORNO ---
        adc_val = ADC_Get_Sample(0);
        temp_ponto_fixo = ((unsigned long)adc_val * 1000) / 1023;
        if (temp_ponto_fixo > 500) {
            LED_Resistencia = 1; // Acende > 50 graus
        } else {
            LED_Resistencia = 0; // Apaga <= 50 graus
        }
        // -------------------------------------------------------------------
        // ATUALIZAÇÃO DO DISPLAY LCD
        //
        // CORREÇÃO PRINCIPAL: cada linha é montada como uma string completa
        // de 16 caracteres e escrita com UMA ÚNICA chamada Lcd_Out().
        // Múltiplas chamadas parciais deslocam o cursor interno do HD44780
        // e causam os caracteres "fantasmas" vistos na imagem.
        // -------------------------------------------------------------------
        // LINHA 1: "Temp: XX.X°C    "  (sempre 16 chars)
        // Escrita completa a cada iteração para proteger contra corrupção.
        FormatarTemperatura(temp_ponto_fixo, txt_temp);
        // txt_temp começa no caractere 7; "Temp: " ocupa cols 1-6.
        // Escrevemos "Temp: " + temperatura em UMA única string implícita
        // aproveitando que Lcd_Out escreve até o '\0'.
        Lcd_Out(1, 1, "Temp: ");
        Lcd_Out(1, 7, txt_temp);
        // LINHA 2: monta string completa antes de escrever
        if (em_execucao) {
            short tempo_mostrar;
            if (tempo_longo >= 0) {
                tempo_mostrar = tempo_longo;
            } else if (tempo_curto >= 0) {
                tempo_mostrar = tempo_curto;
            } else {
                tempo_mostrar = 0;
            }
            MontarLinhaRestam(tempo_mostrar, linha2);
            Lcd_Out(2, 1, linha2); // UMA escrita: sem risco de cursor perdido
        } else {
            if (modo_tempo == 0) {
                Lcd_Out(2, 1, "Modo: 10s       "); // 16 chars exatos
            } else {
                Lcd_Out(2, 1, "Modo: 60s       "); // 16 chars exatos
            }
        }
        Delay_ms(100);
    }
}
