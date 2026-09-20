#include "hdr.h"

#include "volt.h"


#ifdef SET_ADC_VOLT
//-----------------------------------------------------------------------------
float volt = 0.0;
const float volt_porog = 3.0;
//-----------------------------------------------------------------------------
void voltPinInit()
{
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOC;

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = VOLT_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init (VOLT_PORT, &GPIO_InitStructure);

    GPIO_WriteBit(VOLT_PORT, VOLT_PIN, VOLT_LED_OFF);
    
}
//-----------------------------------------------------------------------------
void ledVolt(uint8_t cv)
{
    GPIO_WriteBit(VOLT_PORT, VOLT_PIN, cv);
}
//-----------------------------------------------------------------------------
void ADC_Function_Init(void)
{
ADC_InitTypeDef ADC_InitStructure = {0};
GPIO_InitTypeDef GPIO_InitStructure = {0};

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    
    ADC_CLKConfig(ADC1, ADC_CLK_Div4);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    /* 3. §¬§à§ß§æ§Ú§Ô§å§â§Ñ§è§Ú§ñ §á§Ñ§â§Ñ§Þ§Ö§ä§â§à§Ó §¡§¸§± */
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent;               /* §¯§Ö§Ù§Ñ§Ó§Ú§ã§Ú§Þ§í§Û §â§Ö§Ø§Ú§Þ */
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;                    /* §³§Ü§Ñ§ß§Ú§â§à§Ó§Ñ§ß§Ú§Ö §Ó§í§Ü§Ý§ð§é§Ö§ß§à (§à§Õ§Ú§ß §Ü§Ñ§ß§Ñ§Ý) */
    ADC_InitStructure.ADC_ContinuousConvMode = ENABLE;//DISABLE;              /* §¯§Ö§á§â§Ö§â§í§Ó§ß§í§Û §â§Ö§Ø§Ú§Þ §Ó§í§Ü§Ý§ð§é§Ö§ß (§à§Õ§ß§à §Ú§Ù§Þ§Ö§â§Ö§ß§Ú§Ö) */
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; /* §©§Ñ§á§å§ã§Ü §á§â§à§Ô§â§Ñ§Þ§Þ§ß§í§Þ §á§å§ä§Ö§Þ */
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right;           /* §£§í§â§Ñ§Ó§ß§Ú§Ó§Ñ§ß§Ú§Ö §Õ§Ñ§ß§ß§í§ç §á§à §á§â§Ñ§Ó§à§Þ§å §Ü§â§Ñ§ð */
    ADC_InitStructure.ADC_NbrOfChannel = 1;                          /* §¬§à§Ý§Ú§é§Ö§ã§ä§Ó§à §Ü§Ñ§ß§Ñ§Ý§à§Ó §Ó §á§à§ã§Ý§Ö§Õ§à§Ó§Ñ§ä§Ö§Ý§î§ß§à§ã§ä§Ú = 1 */
    ADC_Init(ADC1, &ADC_InitStructure);

    /* 4. §£§Ü§Ý§ð§é§Ö§ß§Ú§Ö §¡§¸§± */
    ADC_Cmd(ADC1, ENABLE);
    
    /* 5. §¬§Ñ§Ý§Ú§Ò§â§à§Ó§Ü§Ñ §¡§¸§±
    ADC_ResetCalibration(ADC1);
    while(ADC_GetResetCalibrationStatus(ADC1));
    
    ADC_StartCalibration(ADC1);
    while(ADC_GetCalibrationStatus(ADC1)); */
}
//-----------------------------------------------------------------------------
/* §¶§å§ß§Ü§è§Ú§ñ §é§ä§Ö§ß§Ú§ñ §Ù§ß§Ñ§é§Ö§ß§Ú§ñ §ã §å§Ü§Ñ§Ù§Ñ§ß§ß§à§Ô§à §Ü§Ñ§ß§Ñ§Ý§Ñ §¡§¸§± */
uint16_t Get_ADC_Val(uint8_t ch)
{
    /* §¯§Ñ§ã§ä§â§à§Û§Ü§Ñ §â§Ö§Ô§å§Ý§ñ§â§ß§à§Ô§à §Ü§Ñ§ß§Ñ§Ý§Ñ: §ß§à§Þ§Ö§â §Ü§Ñ§ß§Ñ§Ý§Ñ, §â§Ñ§ß§Ô §Ó §á§à§ã§Ý§Ö§Õ§à§Ó§Ñ§ä§Ö§Ý§î§ß§à§ã§ä§Ú, §Ó§â§Ö§Þ§ñ §Ó§í§Ò§à§â§Ü§Ú */
    ADC_RegularChannelConfig(ADC1, ch, 1, ADC_SampleTime_11Cycles);//ADC_SampleTime_55Cycles5);
    
    /* §©§Ñ§á§å§ã§Ü §á§â§Ö§à§Ò§â§Ñ§Ù§à§Ó§Ñ§ß§Ú§ñÈí¼þ */
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    
    /* §°§Ø§Ú§Õ§Ñ§ß§Ú§Ö §à§Ü§à§ß§é§Ñ§ß§Ú§ñ §á§â§Ö§à§Ò§â§Ñ§Ù§à§Ó§Ñ§ß§Ú§ñ */
    while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    
    /* §¹§ä§Ö§ß§Ú§Ö §â§Ö§Ù§å§Ý§î§ä§Ñ§ä§Ñ */
    return  ADC_GetConversionValue(ADC1);
}
//-----------------------------------------------------------------------------
#endif

