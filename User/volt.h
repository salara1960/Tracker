#ifndef __VOLT_H__
#define __VOLT_H__

#include "hdr.h"

#ifdef SET_ADC_VOLT

//----------------------------------------------------- 

#define VOLT_LED_ON  1
#define VOLT_LED_OFF 0
#define VOLT_PORT GPIOC
#define VOLT_PIN  GPIO_Pin_14

//-----------------------------------------------------

extern float volt;
extern const float volt_porog;

//-----------------------------------------------------

void voltPinInit();
void ledVolt(uint8_t cv);
void ADC_Function_Init(void);
uint16_t Get_ADC_Val(uint8_t ch);

//-----------------------------------------------------

#endif



#endif
