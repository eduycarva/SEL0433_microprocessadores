; SEL0433 - Aplicacao de microsprocessadores
; Thayson Pereira Alves - 14681087
; Eduardo Yumoto Carvalheira - 15636150
; PROJETO FINAL

CONTADOR EQU 30H    ; Variavel de processo na RAM para armazenar as voltas (0 a 9)

ORG 0000h
    SJMP INICIO     ; Pula para o inicio do programa principal


; VETOR DE INTERRUPCAO DO TIMER 1

ORG 001BH           ; Endereco da interrupcao do timer 1
    PUSH ACC        ; Salva o Acumulador e PSW na pilha 
    PUSH PSW
    
    INC CONTADOR    ; Incrementa a variavel de processo 
    MOV A, CONTADOR
    CJNE A, #10, FIM_ISR ; Compara com 10. Se nao for 10, vai para o fim da ISR
    
    ; Se atingiu 10 voltas:
    ACALL REINICIA_TIMER ; Chama rotina dedicada para zerar o processo
    
FIM_ISR:
    POP PSW         ; Restaura o contexto
    POP ACC
    RETI            ; Retorna da interrupcao

; INICIO DO PROGRAMA PRINCIPAL

ORG 0030h
INICIO:
    MOV SP, #40H        ; Configura a pilha para uma area segura na RAM

    ; Inicializacao de variaveis e motor
    CLR F0              ; F0 armazena o estado atual do giro 
    MOV CONTADOR, #0    ; Zera a contagem de voltas inicial
    ACALL ATUALIZA_MOTOR; Garante que os pinos iniciem no estado correto

; Configuracao do Timer 1 (Modo 2 - Contador de 8 bits com Auto-reload)
; TMOD é um registrador de 8 bits:
; bits 7 ao 4 configuram o timer 1
; bits 3 ao 0 configuram o timer 0
    MOV TMOD, #60h 
    
; Carrega 0FFh. assim, a cada pulso do motor, ocorre o overflow e a interrupcao
; 0FFh = 255. Como o contador vai até 256, ao somar após o clock, TL1 estoura, e vai para 00h.
    MOV TH1, #0FFh      
    MOV TL1, #0FFh

    ; Habilita as interrupcoes e inicia o Timer
    SETB ET1            ; Habilita interrupção do Timer 1
    SETB EA             ; Habilita chave geral de interrupções
    SETB TR1            ; Liga o Timer 1

LOOP_PRINCIPAL:
    ACALL VERIFICA_CHAVE   ; Verifica se o operador mudou a chave SW
    ACALL ATUALIZA_DISPLAY ; Atualiza o display
    SJMP LOOP_PRINCIPAL    ; Fica neste loop eternamente


;chave acionada -> chave 0
;chave não acionada -> chave1
VERIFICA_CHAVE:
    JB P2.0, CHAVE_EM_1 
    
CHAVE_EM_0:
;F0 indica sentido de rotação
    JNB F0, FIM_VERIFICA  ; Se F0 ja é 0, mantém
    CLR F0                ; Se era 1, atualiza para 0
;invertemos o sentido do giro

    ACALL MUDANCA_DIRECAO ; Aciona a troca
    SJMP FIM_VERIFICA

CHAVE_EM_1:
    JB F0, FIM_VERIFICA   ; Se F0 ja e 1, mantem
    SETB F0               ; Se era 0, atualiza para 1
;invertemos a informação do sentido do giro
    ACALL MUDANCA_DIRECAO ; Aciona a troca

FIM_VERIFICA:
    RET

; 2. Acoes a tomar quando a direcao muda
MUDANCA_DIRECAO:
    ACALL ATUALIZA_MOTOR  ; Inverte os pinos do atuador
    
    CLR EA                ; 
    ACALL REINICIA_TIMER  ; Sempre que mudar sentido, reinicia contagem
    SETB EA               ; Reabilita interrupcoes
    RET

; 3. Atualiza pinos do Motor em P3 (Baseado no CP02)
ATUALIZA_MOTOR:
    JB F0, SENTIDO_1 ; Se F0 = 1, entao gira no sentido1
;Se F0=0, o sentido1 não é executado.

;Lembrando que a lógica do giro é 
;a partir da tabela da verdade fornecido
;pelo software.
 
SENTIDO_0:             
    SETB P3.0          
    CLR P3.1         
    RET                
SENTIDO_1:              
    CLR P3.0           
    SETB P3.1          
    RET  

; 4. Rotina dedicada de reinicializacao do Timer
REINICIA_TIMER:
    CLR TR1             ; Para o timer temporariamente
    MOV CONTADOR, #0    ; Zera o limite de voltas
    MOV TH1, #0FFH      ; Recarrega os registradores
    MOV TL1, #0FFH
    SETB TR1            ; Religa o timer
    RET

; 5. Exibicao no Display 7 Segmentos (Baseado no CP01/CP03)
ATUALIZA_DISPLAY:
    MOV A, CONTADOR     ; Pega a volta atual
    MOV DPTR, #TAB7SEG
    MOVC A, @A+DPTR     ; Busca o binario do numero na tabela

; sinalização visual do ponto decimal
    JNB F0, APAGA_PONTO 

;lembrando que F0 é onde guardamos a informação do sentido de rotação 

;lembrando que P1.7 é o local que acende ou desliga o ponto decimal

ACENDE_PONTO:
    CLR ACC.7           ; Zera o bit 7 
    SJMP ENVIA_DISPLAY
APAGA_PONTO:
    SETB ACC.7          ; Seta o bit 7 

;a lógica abaixo é a mesma dos checkpoints (melhor explicado lá)
ENVIA_DISPLAY:
    MOV P1, A           ; Manda para a porta P1
    RET

; TABELA DE DADOS NA MEMORIA DE PROGRAMA
ORG 0200h
TAB7SEG:
    DB 0C0h ; 0
    DB 0F9h ; 1
    DB 0A4h ; 2
    DB 0B0h ; 3
    DB 099h ; 4
    DB 092h ; 5
    DB 082h ; 6
    DB 0F8h ; 7
    DB 080h ; 8
    DB 090h ; 9

END
