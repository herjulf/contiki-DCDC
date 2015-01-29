/*
 * adc.h
 *
 *  Created on: 21/02/2012
 *      Author: Jorge Querol
 */

#ifndef ADC_H_
#define ADC_H_

//#define ADCChannels 4		//for 2 more current sensor, change this to 8
#define ADCChannels 8		//for 2 more current sensor, change this to 8
#define PREFILTER
#define GlitchBuffer 2
#define GlitchHyst 1000

void ADCInit(void);
void ADCRead(unsigned char ADC);
int ADCValues(int Channel);

#endif /* __ADC_H__ */
