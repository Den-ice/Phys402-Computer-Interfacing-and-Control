/*
 * Lab4-Part2 
 * Created: 9/19/2019 3:16:32 PM
 * Author : Sean and Denice Hickethier
 */
#include <avr/io.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <math.h>

#include <AVRXlib/AVRXClocks.h>
#include <AVRXlib/AVRXSerial.h>


/* fun_prototypes.h */
//prototypes for peripheral initialization functions
void sys_clock();
void setup_timer();
void setup_spi();
void setup_usart();
void setup_peripherals();
void setup_avrx_usart();
void setup_gpio(); 
/*end of fun */
void DMA_Setup( DMA_CH_t *, const void * , void * , int16_t , uint8_t , uint8_t);

void setup_adcA(); //new for lab4
void setup_adcB(); //new for lab4
void parse_it();
volatile int count=0;
volatile int tx_ready=0;
volatile XUSARTst Ser;
 char *wordcount[]= {"one","two","three","four","five","six","seven","eight","nine","ten","eleven","twelve","thirteen","fourteen","fifteen"};
 char *word;

unsigned long sClk, pClk; //sysclock and peripheral clock
 char rx_buf[1];
volatile int sweep=0;
volatile uint16_t adc_sample[3];
char *sample_word;
volatile int wait_for_it=0; //sephamore /flag
volatile int enable_count=0;
volatile int enable_light_adc=0;
volatile int enable_accel=0;
volatile int adccount=0;
ISR (TCC0_OVF_vect){


	if (enable_count==1){
		word=wordcount[count%15];
		count++;	
		USART_send(&Ser, word);
	}
	if (enable_light_adc==1){

		ADCA_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
		ADCA_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?
		ADCA_CTRLA |= ADC_CH0START_bm ; //
	}
	
	if (enable_accel==1){
		if (adccount==0){
			sprintf(sample_word,"%d,%d,%d\n",adc_sample[0],adc_sample[1],adc_sample[2]);
			USART_send(&Ser,sample_word );
			ADCB_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;//ADC_CH_MUXPOS_PIN4_gc|ADC_CH_MUXPOS_PIN5_gc;
			}
			else if (adccount==1)
			ADCB_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;//ADC_CH_MUXPOS_PIN4_gc|ADC_CH_MUXPOS_PIN5_gc;
			else if (adccount==2)
			ADCB_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;//ADC_CH_MUXPOS_PIN4_gc|ADC_CH_MUXPOS_PIN5_gc;

			ADCB_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
			ADCB_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?
			ADCB_CTRLA |= ADC_CH0START_bm ; //
			
		adccount=(adccount+1)%3;
	}
		
	
	
	}
	
	
	ISR(TCC0_CCA_vect){  //start pin 0 adc  x axis
			ADCB_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN0_gc;//ADC_CH_MUXPOS_PIN4_gc|ADC_CH_MUXPOS_PIN5_gc;
			ADCB_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
			ADCB_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?
			ADCB_CTRLA |= ADC_CH0START_bm ; //
	}
	
	
	ISR(TCC0_CCB_vect){ //start pin 1 adc y axis
			ADCB_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN1_gc;//ADC_CH_MUXPOS_PIN4_gc|ADC_CH_MUXPOS_PIN5_gc;
			ADCB_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
			ADCB_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?
			ADCB_CTRLA |= ADC_CH0START_bm ; //
	}

	ISR(TCC0_CCC_vect){
			ADCB_CH0_MUXCTRL = ADC_CH_MUXPOS_PIN2_gc;//ADC_CH_MUXPOS_PIN4_gc|ADC_CH_MUXPOS_PIN5_gc;
			ADCB_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
			ADCB_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?
			ADCB_CTRLA |= ADC_CH0START_bm ; //
	}

/* Interrupt Service Handlers */
ISR(USARTC0_RXC_vect){
	Rx_Handler(&Ser);
	wait_for_it=1;
}

ISR(USARTC0_TXC_vect){
	Tx_Handler(&Ser);
}

ISR(USARTC0_DRE_vect){
}

ISR(ADCA_CH0_vect){
	free(sample_word);
	sample_word=malloc(50);
	adc_sample[0]=ADCA.CH0.RES;
	sprintf(sample_word,"The adc value is : %d \n",adc_sample[0]);
	USART_send(&Ser,sample_word );
	}

ISR(ADCB_CH0_vect){
//we simply store the adc into an array for display later
	adc_sample[adccount]=ADCB.CH0.RES;
}

/* //non avrx library usart tx function
void USART_TX(volatile int *flag){
	if (!(wordcount[(count%26)]=='\0')&&(flag!=1))
	USARTC0_DATA=wordcount[(count++)%26]; //write next char
	else flag=1;	
	return;
}*/


// call used peripheral setups
void setup_peripherals(){
	cli();
	sys_clock();
	setup_timer();
	setup_adcA();   //light sensor
	setup_adcB();	//accelerometer on adc0,1,2

	//setup_spi();
	//setup_usart();
	setup_avrx_usart();
	PMIC_CTRL = PMIC_HILVLEN_bm|PMIC_MEDLVLEN_bm|PMIC_LOLVLEX_bm;   // enable all priorities
	sei();
}


int main(void)
{	
	
	sample_word=malloc(50);
	setup_peripherals();
	
while (1)
    {
		if (wait_for_it==1)			
		//if ( (Ser.serStatus))//& _USART_RX_DONE) ==1)
		{
			wait_for_it=0;
			USART_read(&Ser,rx_buf);
			USART_RxFlush(&Ser);
			parse_it(rx_buf);	
		}
    }
}

void parse_it(char *text){	
	switch (text[0]){
		case '1': count=1;break;
		case '2': count=2;break;
		case '3': count=3;break;
		case '4': count=4;break;
		case '5': count=5;break;
		case '6': count=6;break;
		case '7': count=7;break;
		case '8': count=8;break;
		case '9': count=9;break;
		case 'a': count=10;break;
		case 'b': count=11;break;
		case 'c': count=12;break;
		case 'd': count=13;break;
		case 'e': count=14;break;
		case 'f': count=15;break;
		case 's': enable_count^=0x01;
			USART_send(&Ser,"toggle_count");
			break;
		case 'l': enable_light_adc^=0x01;
			USART_send(&Ser,"sample_light");
			ADCA_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
			ADCA_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?
			ADCA_CTRLA |= ADC_CH0START_bm ; //
			break;
		case 'x': 
			//USART_send(&Ser,"sample_pin0 ");
			ADCB_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
			ADCB_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?
			ADCB_CTRLA |= ADC_CH0START_bm ; //
			break;
		case 'q':
			enable_accel=1;
			break;
			
	}
	count =count-1;
	return;	
}

void setup_adcA(){
		ADCA_CTRLB |= ADC_RESOLUTION_12BIT_gc;
		ADCA_REFCTRL |= ADC_REFSEL_INTVCC2_gc; // use 3V3 to chip
		ADCA_PRESCALER |= ADC_PRESCALER_DIV512_gc; //peripheral clock/512
		ADCA_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
		
		ADCA_CH0_MUXCTRL = ADC_CH_MUXNEG_PIN0_gc;//ADC_CH_MUXPOS_PIN4_gc|ADC_CH_MUXPOS_PIN5_gc;
		ADCA_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?	
		ADCA_CH0_INTCTRL = ADC_CH_INTMODE_COMPLETE_gc| ADC_CH_INTLVL_HI_gc;	
		ADCA_CTRLA |= ADC_CH0START_bm ; //
		
}


void setup_adcB(){
	ADCB_CTRLB |= ADC_RESOLUTION_12BIT_gc;
	ADCB_REFCTRL |= ADC_REFSEL_INTVCC2_gc; // use 3V3 to chip
	ADCB_PRESCALER |= ADC_PRESCALER_DIV256_gc; //peripheral clock/512

	ADCB_CH0_CTRL =  ADC_CH_INPUTMODE_SINGLEENDED_gc|ADC_CH_GAIN_1X_gc;
	//ADCB_CH0_MUXCTRL = ADC_CH_MUXNEG_PIN0_gc|ADC_CH_MUXNEG_PIN1_gc|ADC_CH_MUXNEG_PIN2_gc;
	ADCB_CH0_MUXCTRL =ADC_CH_MUXPOS_PIN0_gc|ADC_CH_MUXPOS_PIN1_gc|ADC_CH_MUXPOS_PIN2_gc;//|ADC_CH_MUXPOS_PIN5_gc;
	ADCB_CTRLA |= ADC_ENABLE_bm; // enable and wait ~24clks?
	ADCB_CH0_INTCTRL = ADC_CH_INTMODE_COMPLETE_gc| ADC_CH_INTLVL_HI_gc;
	ADCB_CTRLA |= ADC_CH0START_bm ; //
	
}

void sys_clock(){
	//set mcu clock/frequency
	SetSystemClock(CLK_SCLKSEL_RC32M_gc, CLK_PSADIV_1_gc,CLK_PSBCDIV_1_1_gc);
	GetSystemClocks(&sClk, &pClk);
	}
	
void setup_timer(){
	//	1s= 2Mhz/1024= 0x7a1
	//  1s=32m/1024=0x7a12
	TCC0_PER=0X7a12;					//1s
	TCC0_INTCTRLA = PMIC_MEDLVLEN_bm; // medium level interrupt
	TCC0_CTRLA=0x7;
	//CTRLA /PRESCALER /1	/2	/4	/8	/64	/256	/1024
	//VALUE		      0X1	0X2	0X3	0X4	0X5	0X6		0X7	
}

void setup_compare_timer(){
	TCC0_CTRLC=(1<<TC0_CMPA_bm)|(1<<TC0_CMPB_bm)|(1<<TC0_CMPC_bm);
	TCC0_CTRLB=(1<<TC0_CCAEN_bm)|(1<<TC0_CCBEN_bm)|(1<<TC0_CCCEN_bm);
	
	
	
}
//Configure SPI
void setup_spi(){
	PORTC.DIR|=PIN4_bm|PIN5_bm|PIN7_bm;;
	PORTC.PIN5CTRL=PORT_OPC_PULLUP_gc; 
	SPIC.CTRL=  SPI_ENABLE_bm|SPI_MASTER_bm|SPI_PRESCALER_DIV128_gc|SPI_MODE0_bm;  //| SPI_CLK2X_bm
	SPIC.DATA=0b11001010;  //something to transmit
}

//Configure USART
void setup_usart(){
	
	int nBScale =-5;
	unsigned long nBSel;
	long fCLK =2000000L ;  //cpu is at 32mhz, 2000000  works for 57.6k sync
	long fbaud= 57611 ;
	
	PORTC.DIRSET=PIN3_bm;		//set pin3 to output
	PORTC.PIN3CTRL=PORT_OPC_PULLUP_gc;	//pullup pin3
	nBSel=fCLK/(16*(pow(2,nBScale)*fbaud))-1; //calculate settings, tweaked to sync at ~57.6k
	
	USARTC0_BAUDCTRLA=(unsigned char)(nBSel &0x00FF);	
	USARTC0_BAUDCTRLB=(char)((nBScale &0x00F)<<4)|((nBSel & 0x0F00>>8));	
	USARTC0.CTRLA= USART_TXCINTLVL_LO_gc;
				//ASYNC MODE                //8 BIT CHARS        //PARITY OFF               //NO STOP BIT     //low interrupt
	USARTC0.CTRLC=USART_CMODE_ASYNCHRONOUS_gc | USART_CHSIZE_8BIT_gc | USART_PMODE_DISABLED_gc	|0x00	;
	USARTC0.CTRLB=USART_RXEN_bm|USART_TXEN_bm;  
	
}
void setup_avrx_usart(){
	
	USART_init(&Ser, 0xc0, pClk, (_USART_TXCIL_LO | _USART_RXCIL_LO), 576, -4,_USART_CHSZ_8BIT, _USART_PM_DISABLED, _USART_SM_1BIT);
	USART_buffer_init(&Ser, 160, 180);
	Ser.fInMode = _INPUT_CR ; //| _INPUT_ECHO | _INPUT_TTY;
	//Ser.fOutMode = _OUTPUT_CRLF;
	USART_enable(&Ser,(USART_TXEN_bm|USART_RXEN_bm));
	
}

void setup_gpio(){
	PORTR_DIR|=0x3; //enable leds
	PORTR_OUT|=0X02;//turn R0 led on

}

/*
//dma_setup provided by Professor Shaafsma
void DMA_Setup( DMA_CH_t * dmaChannel, const void * src, void * dest, int16_t blockSize, uint8_t repeatCount, uint8_t trigSrc )
{
	DMA_CTRL |= DMA_ENABLE_bm;

	dmaChannel->SRCADDR0 = (uint8_t)(( (uint32_t) src)) & 0xFF;
	dmaChannel->SRCADDR1 = (uint8_t)(( (uint32_t) src) >> 1*8 ) & 0xFF;
	dmaChannel->SRCADDR2 = (uint8_t)(( (uint32_t) src) >> 2*8 ) & 0xFF;

	dmaChannel->DESTADDR0 = (uint8_t)(( (uint32_t) dest)) & 0xFF;
	dmaChannel->DESTADDR1 = (uint8_t)(( (uint32_t) dest) >> 1*8 ) & 0xFF;
	dmaChannel->DESTADDR2 = (uint8_t)(( (uint32_t) dest) >> 2*8 ) & 0xFF;

	dmaChannel->ADDRCTRL = DMA_CH_SRCRELOAD_BLOCK_gc | DMA_CH_SRCDIR_INC_gc |
	DMA_CH_DESTRELOAD_BURST_gc | DMA_CH_DESTDIR_INC_gc;
	dmaChannel->TRFCNT = blockSize;
	dmaChannel->CTRLA = DMA_CH_BURSTLEN_2BYTE_gc | DMA_CH_REPEAT_bm;
	dmaChannel->REPCNT = repeatCount; 						// controls blocks
	//0x40 TCC0 Timer/counter C0 DMA triggers base value p 63
	dmaChannel->TRIGSRC = trigSrc;
	dmaChannel->CTRLA |= DMA_CH_SINGLE_bm;
	
	// start dac again
	DACdata_Init();
	DACB_CTRLB = DAC_CHSEL_SINGLE_gc;
	DACB_CTRLC = DAC_REFSEL_AVCC_gc;
	DACB_CTRLA = DAC_CH0EN_bm | DAC_ENABLE_bm;
	DACB_TIMCTRL = DAC_CONINTVAL_16CLK_gc;
	}
	*/
