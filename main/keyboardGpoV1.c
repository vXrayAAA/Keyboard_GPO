#include <stdio.h>								// Acesso as operacoes de entradas/saidas.


#include "lcdv2r6.h"

#define ___keyCK	GPIO_NUM_4							                        // Seleciona o pino de 'clock' para o registrador.
#define ___keyWR	GPIO_NUM_16							                        // Seleciona o pino de 'data out' para o registrador.
#define ___keyLD	GPIO_NUM_2							                        // Seleciona o pino de 'load' para o registrador.
#define ___keyRD	GPIO_NUM_15							                        // Seleciona o pino de 'data in' para o registrador.


/* Saida */
#define ___gpoCK	GPIO_NUM_12                       			                // Seleciona o pino de 'clock' para o registrador.
#define ___gpoDT	GPIO_NUM_27                       			                // Seleciona o pino de 'data out' para o registrador.
#define ___gpoLD	GPIO_NUM_14                       			                // Seleciona o pino de 'load' para o registrador.

unsigned char ___charVlr=0;														// Variavel global para recerregar o valor da saida.
unsigned char tecTecNew,tecTecOld;				                                // Var. global para rotina anti-repeticao.

static const char *TAG = "n1K";


void __regClear(void)															// Limpa o registrador.
{
	/* Limpa o registrador */
	unsigned char __tmp010;									                    // Var local temporaria.					
	for(__tmp010=0;__tmp010<8;__tmp010++)                                       // Laco para zerar o registrador.
	{
		gpio_set_level(___gpoCK,1);									            // Gera um pulso de clock no registrador.
		gpio_set_level(___gpoCK,0);                                             // 
	}
}


void gpoIniciar(void)															// Inicializa o hardware da saida.
{
    gpio_reset_pin(___gpoCK);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___gpoDT);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___gpoLD);													// Limpa configuracoes anteriores.
    gpio_set_direction(___gpoCK, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(___gpoDT, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(___gpoLD, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
	gpio_set_level(___gpoCK,0);													// Limpa o pino.
	gpio_set_level(___gpoDT,0);													// Limpa o pino.
	gpio_set_level(___gpoLD,0);													// Limpa o pino.
	__regClear();																// Limpa conteudo do registrador.
}


void gpoDado(char vlrSaida)														// Envia um dado para o GPO (saida).
{
	unsigned char __tmp001;														// Var local temporaria.
	___charVlr=vlrSaida;														// Salva uma copia do valor da saida.  
	for(__tmp001=0;__tmp001<8;__tmp001++)										// Laco para serializar.
	{
		if(bitX(vlrSaida,(7-__tmp001)))gpio_set_level(___gpoDT,1);				// Verifica o valor do bit, se 1...
		else gpio_set_level(___gpoDT,0);										// ... e se for 0.				
		gpio_set_level(___gpoCK,1);									            // Gera um pulso de clock no registrador.
		gpio_set_level(___gpoCK,0);                                             // ...
	}							
	gpio_set_level(___gpoCK,0);													// Limpa o pino.
	gpio_set_level(___gpoDT,0);													// Limpa o pino.
	gpio_set_level(___gpoLD,1); 							                    // Gera um pulso para carregar o dado.
	gpio_set_level(___gpoLD,0);                                                 // ...
}


unsigned char __keyScan(void)					                                // Le as linhas.
{
	unsigned char entrada=0x00;				                                    // Var local temporaria para a entrada.
	unsigned char tst=0x00;				                                        // Var local temporaria para o laco.

	gpio_set_level(___keyLD,1);					                                // Ativa o pino da carga do dado.
	for(tst=0;tst<8;tst++)							                            // Laco para varrer os bits.
	{
		if(gpio_get_level(___keyRD)==1) bit1(entrada,(7-tst));		            // Se o pino de entrada estiver ativado...
		else			                bit0(entrada,(7-tst));		            // ... senao...
        gpio_set_level(___keyCK,1);					                            // Gera um pulso de clock no registrador.
        gpio_set_level(___keyCK,0);					                            //
	}
	gpio_set_level(___keyLD,0);						                            // Desativa o pino da carga do dado.
	return (entrada);								                            // Retorna com o valor
}

void __keySerial(unsigned char __vlrK1)				                            // Envia as colunas para os registradores.
{
	unsigned char __tmp1;
	for(__tmp1=0;__tmp1<8;__tmp1++)					                            // Laco para serializar.
	{
        if(bitX(__vlrK1,(7-__tmp1)))gpio_set_level(___keyWR,1);		            // Verifica o valor do bit, se 1...
        else                        gpio_set_level(___keyWR,0);		            // ... e se for 0.
        gpio_set_level(___keyCK,1);					                            // Gera um pulso de clock no registrador.
        gpio_set_level(___keyCK,0);					                            //
	}
    gpio_set_level(___keyLD,1);						                            // Gera um pulso para carregar o dado.
    gpio_set_level(___keyLD,0);						                            //
}

const unsigned char ___teclaMatriz[4][4]=			                            // Definicao do valor das teclas.
{
	// {'1','2','3','A'},
	// {'4','5','6','B'},
	// {'7','8','9','C'},
	// {'F','0','E','D'}
    {'1','4','7','F'},
	{'2','5','8','0'},
	{'3','6','9','E'},
	{'A','B','C','D'}
};

void __keyTest(unsigned char keyTmp, unsigned char matPos)	                    // Verifica e recupera valor na matriz.
{
	if(keyTmp==1) tecTecNew=___teclaMatriz[0][matPos];	                        // Se for a 1ª linha...
	if(keyTmp==2) tecTecNew=___teclaMatriz[1][matPos];	                        // Se for a 2ª linha...
	if(keyTmp==4) tecTecNew=___teclaMatriz[2][matPos];	                        // Se for a 3ª linha...
	if(keyTmp==8) tecTecNew=___teclaMatriz[3][matPos];	                        // Se for a 4ª linha...
    // if(keyTmp==1) tecTecNew=___teclaMatriz[matPos][0];	                        // Se for a 1ª linha...
	// if(keyTmp==2) tecTecNew=___teclaMatriz[matPos][1];	                        // Se for a 2ª linha...
	// if(keyTmp==4) tecTecNew=___teclaMatriz[matPos][2];	                        // Se for a 3ª linha...
	// if(keyTmp==8) tecTecNew=___teclaMatriz[matPos][3];	                        // Se for a 4ª linha...
}

char tecla(void)							                            // Rotina de varredura e verificacao do teclado.
{
	unsigned char ___tmpKey=0x00;						                        // Var local temporaria com o valor da tecla.
	unsigned char ___tmpCol=0x01;						                        // Var local temporaria com o valor da coluna.
    // unsigned char ___tmpLin=0x01;						                    // Var local temporaria com o valor da linha.
	unsigned char ___tmpPos=0x00;						                        // Var local temporaria com o valor da posicao.

	for(___tmpPos=0;___tmpPos<4;___tmpPos++)			                        // Laco de varredura e verificacao.
	{
		__keySerial(___tmpCol);							                        // Seleciona a coluna.
        // __keySerial(___tmpLin);							                    // Seleciona a linha.
		// vTaskDelay(1);									                        // Aguarda estabilizar.
		___tmpKey=__keyScan();							                        // Faz varredura do teclado.
		__keyTest(___tmpKey,___tmpPos);					                        // Verifica o valor se a tecla for acionada.
		___tmpCol=___tmpCol<<1;							                        // Proxima coluna.
        // ___tmpLin=___tmpLin<<1;							                    // Proxima linha.
	}
	__keySerial(0x00);									                        // Limpa o registrador de deslocamento.

	/* Codigo anti-repeticao */
	if(tecTecNew!=tecTecOld)	tecTecOld=tecTecNew;	                        // Se o valor atual eh diferente da anterior, salva atual e ... 
	else	tecTecNew=0x00;								                        // ...se nao... Salva como nao acionada ou nao liberada.						
	return (tecTecNew);									                        // ...retorna com o valor.
}

void tecladoIniciar(void)							                            // Rotina para iniciar o hardware.
{
	gpio_reset_pin(___keyCK);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___keyWR);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___keyLD);													// Limpa configuracoes anteriores.
	gpio_reset_pin(___keyRD);													// Limpa configuracoes anteriores.

    gpio_set_direction(___keyCK, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(___keyWR, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(___keyLD, GPIO_MODE_OUTPUT);								// Configura o pino como saida.
    gpio_set_direction(___keyRD, GPIO_MODE_INPUT);								// Configura o pino como entrada.

	gpio_set_level(___keyCK,0);													// Limpa o pino.
	gpio_set_level(___keyWR,0);													// Limpa o pino.
	gpio_set_level(___keyLD,0);													// Limpa o pino.
	gpio_set_level(___keyRD,0);													// Limpa o pino.
}

void TecladoGpo (void)
{
    
    unsigned char key = 0;
    key = tecla();
    switch (key)
    {
    case '0' :
        gpoDado(0x00);
        lcdTexto("SAIDA: 0X00",2,1);
        ESP_LOGI(TAG,"sua tecla pressionada: 0");
        break;
       case '1':
            gpoDado(0x01);
            lcdTexto("SAIDA: 0X01",2,1);
            ESP_LOGI(TAG,"sua tecla pressionada: 1 ");
            break;
        case '2':
            gpoDado(0x03);
            lcdTexto("SAIDA: 0X03",2,1);
            ESP_LOGI(TAG,"sua tecla pressionada: 2");
            break;
        case '3':
            gpoDado(0x07);
            lcdTexto("SAIDA: 0X07",2,1);
            ESP_LOGI(TAG,"sua tecla pressionada: 3");
            break;
        case '4':
            gpoDado(0x0F);
            lcdTexto("SAIDA: 0X0F",2,1);
            ESP_LOGI(TAG,"sua tecla pressionada: 4");
            break;
    }    


}

void app_main(void)
{

    gpoIniciar();
    tecladoIniciar();
    lcdIniciar();
   
    
    lcdTexto("n1K...",1,1);
    


    while (1)
    {
        TecladoGpo();
        vTaskDelay(1);
    }
    

}
