/*
 * adc.c
 *
 *  Created on: 21/02/2012
 *      Author: Jorge Querol
 */

#include "lpc17xx.h"
#include "adc.h"
#include "control.h"

int ADC[MAX_ADC];
int UpdateChannel;

int pos[MAX_ADC];
int glitches[MAX_ADC];
int Buffer[2][GlitchBuffer][MAX_ADC];

extern int Vout, Vin, Iref, Il;

void ADCvaluesInit(void);

void ADCInit(void)
{
	LPC_PINCON->PINSEL1  |= 0x00154000;	// Select ADC function for pins (ADC0-3)
	LPC_PINCON->PINMODE1 |= 0x002A8000;	// Neither pull-up nor pull-down resistor

	LPC_SC->PCONP |= (1 << 12);				// Set up bit PCADC
	LPC_SC->PCLKSEL0 |= (01 << 24);			// PCLK = CCLK (102 MHz)
	/* 7-0 SEL
	 * 15-8 CLKDIV = 7 (12.25 MHz)
	 * 15-8 CLKDIV = 3 (25.5 MHz)
	 * 16 BURST software
	 * 21 PDN on (ADC on)
	 */
	LPC_ADC->ADCR |= 0x00200300;
	LPC_ADC->ADINTEN = 0x00000100;			// ADC Interrupt Enabled
	NVIC_EnableIRQ(ADC_IRQn);
	NVIC_SetPriority(ADC_IRQn, 2); // ** 0
	ADCvaluesInit();						// Init ADC glitch filter
	UpdateChannel = -1;
	ADCRead(0);								// Start first conversion
}

void ADCRead(unsigned char ADC)
{
	LPC_ADC->ADCR &= ~(0x000000FF);			// Remove ADC selected
	LPC_ADC->ADCR |= (1 << ADC);			// Select ADC
	LPC_ADC->ADCR |= (1 << 24);				// Start conversion
}

void ADC_IRQHandler(void)
{
	int Channel;
	Channel = (LPC_ADC->ADGDR >> 24) & 0x00000007;
	ADC[Channel] = (LPC_ADC->ADGDR >> 4) & 0x00000FFF;
	Channel++;
	if(Channel < MAX_ADC)
		ADCRead(Channel);
	else
	{
		/*
		 * Bangbang control in hardware interrupt context
		 */
#ifdef BOARD_2013
LPC_GPIO1->FIOPIN ^= (1 << 26); //new = 26
#else
LPC_GPIO1->FIOPIN ^= (1 << 29); //old =29
#endif
		ADCRead(0);
		Vout = ADCValues(0);
		Vin = ADCValues(1);
		Iref = ADCValues(2);
		Il = ADCValues(3) - Iref;
		MeanValues();

  //  		  VSC_Calc(i_out);
		BangBang();

#ifdef BOARD_2013
LPC_GPIO1->FIOPIN ^= (1 << 26);  //new
#else
LPC_GPIO1->FIOPIN ^= (1 << 29); //old
#endif
		/**/
	}
}

void ADCvaluesInit(void)
{
	int i, j, k;
	for(i=0; i<2; i++)
	{
		for(j=0; j<GlitchBuffer; j++)
		{
			for(k=0; k<MAX_ADC; k++)
			{
				Buffer[i][j][k]=0;
			}
		}
	}
	for(i=0; i<MAX_ADC; i++)
	{
		pos[i] = 0;
		glitches[i] = 0;
	}
}

int ADCValues(int Channel)
{
#ifdef PREFILTER
	static int i, match;
#endif

	static int sample;
	sample = ADC[Channel];

#ifdef PREFILTER
	match = 0;

	while(((sample - Buffer[0][pos[Channel]][Channel]) > (GlitchHyst))||((Buffer[0][pos[Channel]][Channel] - sample) > (GlitchHyst)))
	{
		pos[Channel]++;
		if(pos[Channel] >= GlitchBuffer)
			pos[Channel] = 0;
		match++;
		if(match >= GlitchBuffer)
			break;
	}

	if(match >= GlitchBuffer)					// Possible glitch detected
	{
		glitches[Channel]++;
		if(glitches[Channel] > GlitchBuffer)	// Previous samples maybe were not glitches
		{
			for(i=0; i<GlitchBuffer; i++)
				Buffer[0][i][Channel] = Buffer[1][i][Channel];
			glitches[Channel] = 0;
		}
		else									// Store suspicious samples
			Buffer[1][glitches[Channel]-1][Channel] = sample;
		sample = 0;
		for(i=0; i<GlitchBuffer; i++)
			sample += Buffer[0][i][Channel];
		sample /= GlitchBuffer;
	}
	else
	{
		glitches[Channel] = 0;
		pos[Channel]--;
		if(pos[Channel] < 0)
			pos[Channel] = GlitchBuffer - 1;
		Buffer[0][pos[Channel]][Channel] = sample;
	}
#endif

	return(sample);
}
