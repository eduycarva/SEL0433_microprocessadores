; SEL0433 - Aplicacao de microsprocessadores
; Thayson Pereira Alves - 14681087
; Eduardo Yumoto Carvalheira - 15636150
; PROJETO FINAL

CONTADOR EQU 30H    ; variavel de processo na RAM para armazenar as voltas (0 a 9, depois reinicia)

ORG 0000h
    SJMP INICIO     ; pula para o inicio do programa principal


; INTERRUPCAO DO TIMER 1

ORG 001BH           ; endereco da interrupção do timer 1
    PUSH ACC        ; salva o Acumulador e PSW na pilha 
    PUSH PSW
    
    INC CONTADOR    ; incrementa a variável de processo 
    MOV A, CONTADOR
    CJNE A, #10, FIM_ISR ; compara com 10. Se não for 10, vai para o fim da ISR
    
    ; Se atingiu 10 voltas:
    ACALL REINICIA_TIMER ; chama rotina dedicada para zerar o processo
    
FIM_ISR:
    POP PSW         ; restaura o contexto
    POP ACC
    RETI            ; retorna da interrupção

; PROGRAMA PRINCIPAL

ORG 0030h
INICIO:
    MOV SP, #40H        ; configura a pilha para uma area segura na RAM

    ; inicializacao de variaveis e motor
    CLR F0               ; F0 armazena o estado atual do giro 
    MOV CONTADOR, #0     ; zera a contagem de voltas inicial
    ACALL ATUALIZA_MOTOR ; garante que os pinos iniciem no estado correto

; Configuração do Timer 1
; TMOD é um registrador de 8 bits:
; bits 7 ao 4 configuram o timer 1
; bits 3 ao 0 configuram o timer 0
    MOV TMOD, #60h 
    
; carrega 0FFh. assim, a cada pulso do motor, ocorre o overflow e a interrupção
; 0FFh = 255. Como o contador vai até 256, ao somar após o clock, TL1 estoura, e vai para 00h.
    MOV TH1, #0FFh      
    MOV TL1, #0FFh

; habilita as interrupções e inicia o Timer
    SETB ET1            ; habilita interrupção do Timer 1
    SETB EA             ; habilita chave geral de interrupções
    SETB TR1            ; liga o Timer 1

LOOP_PRINCIPAL:
    ACALL VERIFICA_CHAVE   ; verifica se o operador mudou a chave SW
    ACALL ATUALIZA_DISPLAY ; atualiza o display
    SJMP LOOP_PRINCIPAL    ; fica neste loop eternamente

; 1. verificação da mudança de sentido
;chave acionada -> chave 0
;chave não acionada -> chave1
VERIFICA_CHAVE:
    JB P2.0, CHAVE_EM_1 
    
CHAVE_EM_0:
;F0 indica sentido de rotação
    JNB F0, FIM_VERIFICA  ; se F0 ja é 0, mantém
    CLR F0                ; se era 1, atualiza para 0
;invertemos o sentido do giro

    ACALL MUDANCA_DIRECAO ; aciona a troca
    SJMP FIM_VERIFICA

CHAVE_EM_1:
    JB F0, FIM_VERIFICA   ; se F0 ja e 1, mantem
    SETB F0               ; se era 0, atualiza para 1
;invertemos a informação do sentido do giro
    ACALL MUDANCA_DIRECAO ; aciona a troca

FIM_VERIFICA:
    RET

; 2. ações quando ocorre mudança de sentido
MUDANCA_DIRECAO:
    ACALL ATUALIZA_MOTOR  ; inverte os pinos do atuador
    ACALL REINICIA_TIMER  ; sempre que mudar sentido, reinicia contagem
    SETB EA               ; reabilita interrupcoes
    RET

; 3. atualiza pinos do Motor em P3
ATUALIZA_MOTOR:
    JB F0, SENTIDO_1 ; se F0 = 1, entao gira no sentido1
                     ; se F0=0, o sentido1 não é executado.
; tabela da verdade do sentido do giro 
SENTIDO_0:             
    SETB P3.0          
    CLR P3.1         
    RET                
SENTIDO_1:              
    CLR P3.0           
    SETB P3.1          
    RET  

; 4. rotina de reinicializacao do Timer
REINICIA_TIMER:
    CLR TR1             ; para o timer temporariamente
    MOV CONTADOR, #0    ; zera o limite de voltas
    MOV TH1, #0FFH      ; recarrega os registradores
    MOV TL1, #0FFH      ; molde para TH1
    SETB TR1            ; seta o timer
    RET

; 5. exibição no display
ATUALIZA_DISPLAY:
    MOV A, CONTADOR     ; pega a volta atual
    MOV DPTR, #TAB7SEG  
    MOVC A, @A+DPTR     ; busca o binário do numero na tabela

; acende ou apaga ponto decimal
    JNB F0, APAGA_PONTO 

ACENDE_PONTO:
    CLR ACC.7           ; zera o bit 7 
    SJMP ENVIA_DISPLAY
APAGA_PONTO:
    SETB ACC.7          ; seta o bit 7 


ENVIA_DISPLAY:
    MOV P1, A           ; manda para a porta P1
    RET


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
