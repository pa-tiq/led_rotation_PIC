#include <stdio.h>
#include <string.h> //para usar funçoes de string deve se adicionar este header
#include <stdlib.h>

#define _XTAL_FREQ 4000000 // Oscilador interno de 4 MHz
#include <xc.h>

#pragma config FOSC = INTOSC_EC   // Oscilador externo
#pragma config WDT = OFF   // Watchdog Timer desligado
#pragma config MCLRE = OFF // Master Clear desabilitado
char caracter;
int flag_interrupcao = 0;
int var1, var2; 

void __interrupt() RS232(void) //interrupção
{
    caracter = RCREG;     // Lê caractere recebido do registrador
    flag_interrupcao = 1; // Habilita variável indicando que houve recepção
    RCIF = 0;             // Limpa flag de interrupção de recepção
}

void inicializa_RS232(long velocidade, int modo)
{
    RCSTA = 0X90; // Habilita porta serial, recepção de 8 bits em modo contínuo, assíncrono.
    int valor;
    if (modo == 1) {  // modo = 1, modo alta velocidade (BRGH = 1)
        TXSTA = 0X24; // modo assíncrono, transmissão 8 bits.
        valor =(int)(((_XTAL_FREQ/velocidade)-16)/16); // valor para gerar o baud rate
    }
    else {            //modo = 0 ,modo baixa velocidade (BRGH = 0)
        TXSTA = 0X20; //modo assincrono,trasmissao 8 bits.
        valor =(int)(((_XTAL_FREQ/velocidade)-64)/64);
        //calculo do valor do gerador de baud rate
    }
    SPBRG = valor; // esse registrador, carregado com o valor calculado, define o baud rate
    RCIE = 1;      //habilita interrupção de recepção
    TXIE = 0;      //deixa interrupção de transmissão desligado
                   //(pois corre-se o risco de ter uma interrupção escrita e leitura ao mesmo tempo)
}

void envia_serial(char valor)
{
    TXIF = 0;        // limpa flag que sinaliza envio completo.
    TXREG = valor;   // Envia caractere desejado à porta serial
    while(TXIF ==0); // espera caractere ser enviado
}
void envia_texto_serial(const char frase[])
{
    char indice = 0;               // índice da cadeia de caracteres
    char tamanho = strlen(frase); // tamanho total da cadeia a ser impressa
    
    while(indice < tamanho ) {        // verifica se todos foram impressos
        envia_serial (frase[indice]); // Chama rotina que escreve o caractere
        indice++;                     // incrementa índice
    }
}

void main(void)
{
    OSCCON = 0b01100000;      // Oscilador interno a 4 MHz (desnecessário ao usar oscilador externo)
    TRISC = 0b11000000;       // configura RC7 e RC6 como entrada
    inicializa_RS232(9600,1); // modo de alta velocidade
    GIE = 1;                  // GIE: Global Interrupt Enable bit
    PEIE = 1;                 // habilita interrupção de periféricos do pic
    TRISD = 0;                // Porta D é saída
    
    envia_texto_serial("(A) Rotacao ininterrupta a esquerda \n\r");
    envia_texto_serial("(B) Rotacao ininterrupta a direita \n\r");
    envia_texto_serial("(C) Alternancia entre leds amarelos e verdes \n\r");
    envia_texto_serial("(D) Efeito sanfona \n\r");
    envia_texto_serial("(E) Bate e volta \n\r");
    envia_texto_serial("(F) Parar qualquer rotacao \n\r ");
    
    while (1) {
        if(flag_interrupcao == 1) { //tem dados para ler
            if (caracter == 'A'){
                envia_texto_serial("\r(A) Rotacao ininterrupta a esquerda \n\r");
                LATD = 0b00000001;
                while(caracter != 'F'){
                    __delay_ms(150); 
                    LATD = LATD << 1;
                    if (LATD==0b10000000){
                        __delay_ms(150); 
                        LATD = 0b00000001;
                    }
                }
            }
            if (caracter == 'B'){
                envia_texto_serial("\r(B) Rotacao ininterrupta a direita \n\r");
                LATD = 0b10000000;
                while(caracter != 'F'){
                    __delay_ms(150); 
                    LATD = LATD >> 1;
                    if (LATD==0b00000001){
                        __delay_ms(150); 
                        LATD = 0b10000000;
                    }
                }
            }
            if (caracter == 'C'){
                envia_texto_serial("\r(C) Alternancia entre leds amarelos e verdes \n\r");
                LATD =0b10101010;
                while(caracter != 'F'){
                    __delay_ms(150); 
                    LATD = 0b10101010;
                    __delay_ms(150); 
                    LATD = 0b01010101;                   
                }
            }
            if (caracter == 'D'){
                envia_texto_serial("\r(D) Efeito sanfona \n\r");
                LATD =0b10000001;
                while(caracter != 'F'){
                    __delay_ms(150); 
                    LATD = 0b01000010;
                    __delay_ms(150); 
                    LATD = 0b00100100;
                    __delay_ms(150); 
                    LATD = 0b00011000;
                    __delay_ms(150); 
                    LATD = 0b00100100;
                    __delay_ms(150); 
                    LATD = 0b01000010;
                    __delay_ms(150); 
                    LATD = 0b10000001;
                }
            }
            if (caracter == 'E'){
                envia_texto_serial("\r(E) Bate e volta \n\r");
                LATD = 0b10000001; // Liga os Leds dos pino 0 e 7 da porta D
                __delay_ms(150);   // Atraso de 150 ms
                var1 = 0b00000001; // var1 = 0x01
                var2 = 0b10000000; // var2 = 0x80
                while(caracter != 'F'){
                    while(var1 != 0b10000000){ // Enquanto LATD ? 18H, fica no loop 
                        if (caracter == 'F') break;
                        var1 = var1 << 1;     // rotaciona var1 1 passo para a esquerda
                        var2 = var2 >> 1;     // rotaciona var2 1 passo para a direita
                        LATD = (var1 | var2); // Junta var1 com var2 (var1 OU var2)
                        __delay_ms(150);      // Atraso de 150 ms
                    }
                    while(var1 != 0b00000001){ // Enquanto LATD ? 81H, fica no loop     
                        if (caracter == 'F') break;
                        var1 = var1 >> 1;     // rotaciona var1 1 passo para a direita
                        var2 = var2 << 1;     // rotaciona var2 1 passo para a esquerda
                        LATD = (var1 | var2); // Junta var1 com var2 (var1 OU var2)
                        __delay_ms(150);      // Atraso de 150 ms
                    }
                }
            }
            if (caracter == 'F')
            {
                envia_texto_serial("\r(F) Parar qualquer rotacao \n\r");
                LATD = 0b00000000;
                flag_interrupcao = 0;
            }
        }        
    }
}