#ifndef LCDV2R6_H
#define LCDV2R6_H

#include "geralv2r1.h"															// Configuracao basica de elementos internos.

/* LCD */
#define ___lcdCK	GPIO_NUM_17                       			                // Seleciona o pino de 'clock' para o registrador.
#define ___lcdDT	GPIO_NUM_18                       			                // Seleciona o pino de 'data' para o registrador.
#define ___lcdLD	GPIO_NUM_5                       			                // Seleciona o pino de 'load' para o registrador.
#define ___RS		GPIO_NUM_2                       			                // Bit do registrador.
#define ___EN		GPIO_NUM_3                       			                // Bit do registrador.
#define ___tempo	10															// Tempo (ms) para pulso do pino EN.

/**
*	@brief Limpa os registradores do hardware (74HC595). Impede acionamentos indevidos na inicializacao.
*/
void __lcdCls(void)										                        // Limpa o registrador.
{
	unsigned char __tmp0;									                    // Var local temporaria.
	gpio_set_level(___lcdDT,0);								                    // Desliga o bit.						
	for(__tmp0=0;__tmp0<8;__tmp0++)                                             // Laco para zerar o registrador.
	{
		gpio_set_level(___lcdCK,1);									            // Gera um pulso de clock no registrador.
		gpio_set_level(___lcdCK,0);                                             // ...
	}
}

/**
*	@brief Funcao interna: Converte valor paralelo em serial para o 74HC595.
*	@param __vlrL1 Dado para conversao.
*/
void __lcdSerial(unsigned char __vlrL1)					                        // Serializa o dado.
{
	unsigned char __tmp1;														// Var local temporaria.
	for(__tmp1=0;__tmp1<8;__tmp1++)												// Laco para serializar.
	{
		if(bitX(__vlrL1,(7-__tmp1)))gpio_set_level(___lcdDT,1);					// Verifica o valor do bit, se 1...
		else gpio_set_level(___lcdDT,0);										// ... e se for 0.				
		gpio_set_level(___lcdCK,1);									            // Gera um pulso de clock no registrador.
		gpio_set_level(___lcdCK,0);                                             // ...
	}							
	gpio_set_level(___lcdLD,1); 							                    // Gera um pulso para carregar o dado.
	gpio_set_level(___lcdLD,0);                                                 // ...
}

/**
*	@brief Funcao interna: Converte valor 8bits em 4+4bits + controle do bit RS.
*	@param valor Dado em 8bits para conversao.
*	@param pinoRs Valor do pino RS do LCD, valor: 0(comando) ou 1(dado).
*/
void __lcd1Bit(unsigned char valor, unsigned char pinoRs)                       // Rotina de acesso ao LCD por registrador.
{
	unsigned char __tmp0;														// Var local temporaria.
	__lcdCls();																	// Limpa o registrador.
	__tmp0= valor & 0xF0;														// Separa a unidade.
	bit1(__tmp0,___EN);															// Acrescenta o bit '___EN'.
	if(pinoRs)	bit1(__tmp0,___RS);												// Se dado ___RS=1...
	else		bit0(__tmp0,___RS);												// ... senao ___RS=0-.
	__lcdSerial(__tmp0);														// Serializa o dado.
	vTaskDelay(pdMS_TO_TICKS(___tempo));										// Aguarda...
	bit0(__tmp0,___EN);															// Remove o bit '___EN' (gerando a borda de descida).
	if(pinoRs)	bit1(__tmp0,___RS);												// Se dado ___RS=1
	else		bit0(__tmp0,___RS);												// Senao ___RS=0
	__lcdSerial(__tmp0);														// Serializa o dado.
	__tmp0=(valor & 0x0F)<<4;													// Separa a dezena e posiciona.
	bit1(__tmp0,___EN);															// Acrescenta o bit '___EN'.
	if(pinoRs)	bit1(__tmp0,___RS);												// Se dado ___RS=1
	else		bit0(__tmp0,___RS);												// Senao ___RS=0
	__lcdSerial(__tmp0);														// Serializa o dado.
	vTaskDelay(pdMS_TO_TICKS(___tempo));										// Aguarda...
	bit0(__tmp0,___EN);															// Remove o bit '___EN' (gerando a borda de descida).
	if(pinoRs)	bit1(__tmp0,___RS);												// Se dado ___RS=1...
	else		bit0(__tmp0,___RS);												// ... senao ___RS=0.
	__lcdSerial(__tmp0);														// Serializa o dado.
}

/**
*	@brief Funcao interna: Envia comando de posicao do cursor para o LCD.
*	@param linha Linha onde o cursor sera posicionado, valor: 1 ou 2.
*	@param col Coluna onde o cursor sera posicionado, valor: 1 ate 16.
*/
void __lcdPos(unsigned char linha, unsigned char coluna)						// Posiciona o cursor.
{
	if(coluna>16){coluna=16;}													// Limita valor maximo.
	if(linha==1)__lcd1Bit((127+coluna),0);					                    // Se for a 1a. linha...
	if(linha==2)__lcd1Bit((191+coluna),0);					                    // Se for a 2a. linha...
}

/**
*	@brief Inicializa os pinos do hardware e o LCD. Obrigatorio antes de qualquer funcao.
*/
void lcdIniciar(void)									                        // Inicializa o LCD.
{
	gpio_reset_pin(___lcdCK);													// Reinicia o pino.
	gpio_set_direction(___lcdCK, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
	gpio_reset_pin(___lcdDT);													// Reinicia o pino.
	gpio_set_direction(___lcdDT, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
	gpio_reset_pin(___lcdLD);													// Reinicia o pino.
	gpio_set_direction(___lcdLD, GPIO_MODE_OUTPUT);								// Configura o pino como saida.

	__lcd1Bit(0x02,0);															// Habilita o uso em 4 bits.
	__lcd1Bit(0x28,0);															// Habilita  duas linhas, 5x7 e cursor simples.
//	__lcd1Bit(0x0E,0);															// Liga o display e o cursor.
	__lcd1Bit(0x0C,0);															// Liga somente o display.
	__lcd1Bit(0x01,0);															// Limpa memoria do LCD e posiciona em HOME.
}

/**
*	@brief Envia texto para o LCD.
*	@param letras Texto a ser enviado. Exemplo: "Teste".
*	@param linha Linha onde o texto sera posicionado, valor: 1 ou 2.
*	@param col Coluna onde o texto sera posicionado, valor: 1 ate 16.
*/
void lcdTexto(char *letras, unsigned char linha, unsigned char coluna)	        // Envia um texto para o LCD.
{
	__lcdPos(linha,coluna);														// Posiciona o cursor.
	while(*letras)											                    // Enquanto houver caracteres validos...
	{
		__lcd1Bit(*letras,1);								                    // ...envia o caracter e...
		letras++;											                    // ...avanca para o proximo caracter.
	}
}

/**
*	@brief Envia caracter unico para o LCD.
*	@param letra Caracter a ser enviado. Exemplo: 'A' ou 0x30.
*	@param linha Linha onde o caracter sera posicionado, valor: 1 ou 2.
*	@param col Coluna onde o caracter sera posicionado, valor: 1 ate 16.
*/
void lcdCaracter(char letra, unsigned char linha, unsigned char coluna)	        	// Envia um caracter para o LCD.
{
	__lcdPos(linha,coluna);															// Posiciona o cursor.
	__lcd1Bit(letra,1);								                    			// ...envia o caracter.
}

/**
*	@brief Carrega caracter customizado.
*	@param ender Endereco com valor de 0 a 7.
*	@param nome Nome da variavel Matriz com os dados.
*/
void lcdCustom(unsigned char ender, unsigned char *nome)
{
	unsigned char ___tmp01=0;				// Variavel temporaria para o laco.
	if(ender>0x07){ender=0x07;}				// Previne erro de digitacao.
	__lcd1Bit((0x40+ender),0);				// Envia o endereco da posicao do caracter.
	for (___tmp01=0;___tmp01<8;___tmp01++)	// Laco de carga dos dados.
	{
		__lcd1Bit(nome[___tmp01],1);		// Envia o caracter da matriz.
	}
	__lcd1Bit(0x01,0);						// Finaliza processo.
}

#endif
