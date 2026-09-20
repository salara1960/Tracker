/********************************** (C) COPYRIGHT *******************************
 * File Name          : main.c
 * Author             : Alarm
 * Version            : V1.0.0
 * Date               : 2026/09/08
 * Description        : Main program body.
 *********************************************************************************/
// ls -la | grep "${BuildArtifactFileBaseName}.*"
//------------------------------------------------------------------------------

#include "hdr.h"

//------------------------------------------------------------------------------

// const char *ver = "GPS app ver.02";//09.09.2026
// const char *ver = "GPS app ver.03";//10.09.2026
// const char *ver = "GPS app ver.04";//11.09.2026 - §æ§å§ß§Ü§è§Ú§ñ DateTimeToEpoch() §â§Ñ§Ò§à§ä§Ñ§Ö§ä §ß§Ö §Ó§Ö§â§ß§à !!!(§Õ§Ö§ß§î §ß§Ñ 1 §Ò§à§Ý§î§ê§Ö))
// const char *ver = "GPS app ver.05";//12.09.2026 - §Õ§Ý§ñ §æ§å§ß§Ü§è§Ú§Ú DateTimeToEpoch() §ã§Õ§Ö§Ý§Ñ§ß §Ü§à§ã§ä§í§Ý§î !
// const char *ver = "GPS app ver.06";//14.09.2026 - add key - PA0 in interrupt mode
//const char *ver = "GPS app ver.07";  // 16.09.2026
const char *ver = "GPS app ver.08";  // 20.09.2026 - add ADC_chan4 (PA4) for get voltage power


const char *eol = "\n";
const char *uname = "RISC-V CH32X035";
uint8_t RxBuff[64] = {0};
volatile uint8_t evt = noneEvt;
volatile uint8_t ind = 0;
volatile uint32_t epoch = 1789919499;//1789560799;
// 1789391099;//1789220099;//1789136188;//1789035299;//1788942099;//1788867190;
volatile uint32_t seconda = 0;
bool set_time = true;
bool set_new_time = false;
uint16_t arr = 20 - 1;  // 40 - 1;
uint16_t psc = 48000 - 1;
static uint8_t led = 0;
GPIO_TypeDef *LedPort = GPIOB;
uint16_t LedPin = GPIO_Pin_12;  // led pin
char tmp[64] = {0};
bool inv = false;
uint16_t devError = 0;

s_recq_t queEvt;
bool queFlag = false;
bool key_enable = false;
uint8_t key_val = 0;
bool sleep_mode = false;
uint32_t key_tmr = 0;

bool oledOnOff = true;

#ifdef SET_GPS
uint8_t pps_val = 0;
bool pps_enable = false;
char RxGps[128] = {0};
volatile uint8_t ind_gps = 0;
// const char *gps_mask = "RMC";
#endif

//----------------------------------------------------------------------------------------

uint32_t get_sec (uint32_t t);
void Leds();

//----------------------------------------------------------------------------------------
#ifdef SET_FLOAT_PART
void floatPart (float val, s_float_t *part) {
    part->cel = (uint32_t)val;
    part->dro = (val - part->cel) * 1000000;
}
#endif
//----------------------------------------------------------------------------------------
inline uint32_t DateTimeToEpoch (const DateTime_t *dt) {
    uint32_t y = dt->year;
    uint32_t m = dt->mon;
    uint32_t d = dt->day;

    // §®§Ñ§ß§Ú§á§å§Ý§ñ§è§Ú§ñ §ã §Þ§Ö§ã§ñ§è§Ñ§Þ§Ú §Õ§Ý§ñ §å§á§â§à§ë§Ö§ß§Ú§ñ §å§é§Ö§ä§Ñ §Ó§Ú§ã§à§Ü§à§ã§ß§í§ç §Ý§Ö§ä
    // (§Á§ß§Ó§Ñ§â§î §Ú §æ§Ö§Ó§â§Ñ§Ý§î §á§Ö§â§Ö§ß§à§ã§ñ§ä§ã§ñ §Ó §Ü§à§ß§Ö§è §á§â§Ö§Õ§í§Õ§å§ë§Ö§Ô§à §Ô§à§Õ§Ñ)
    if (m <= 2) {
        m += 12;
        y -= 1;
    }

    // §£§í§é§Ú§ã§Ý§ñ§Ö§Þ §Ü§à§Ý§Ú§é§Ö§ã§ä§Ó§à §Õ§ß§Ö§Û §ã §ß§Ñ§é§Ñ§Ý§Ñ §ï§á§à§ç§Ú Unix (1 §ñ§ß§Ó§Ñ§â§ñ 1970 §Ô§à§Õ§Ñ)
    // §£§ã§Ö §Ó§í§é§Ú§ã§Ý§Ö§ß§Ú§ñ §ã§ä§â§à§Ô§à §Ó uint32_t §Ò§Ö§Ù §á§Ö§â§Ö§á§à§Ý§ß§Ö§ß§Ú§Û §Õ§Ý§ñ §Õ§Ñ§ß§ß§à§Ô§à §Õ§Ú§Ñ§á§Ñ§Ù§à§ß§Ñ
    uint32_t days = d + (153UL * m - 457UL) / 5UL + 365UL * y + y / 4UL - y / 100UL + y / 400UL - 719468UL;
    days--;
    // §±§Ö§â§Ö§Ó§à§Õ§Ú§Þ §Õ§ß§Ú §Ú §ä§Ö§Ü§å§ë§Ö§Ö §Ó§â§Ö§Þ§ñ §Ó §Ú§ä§à§Ô§à§Ó§í§Ö §ã§Ö§Ü§å§ß§Õ§í (Unix-time)
    uint32_t ep = days * 86400UL + (uint32_t)dt->hour * 3600UL + (uint32_t)dt->min * 60UL + dt->sec;

    return ep;
}

//---------------------------------------------------------------------------------------
inline void EpochToDateTime (uint32_t ep, DateTime_t *dt) {
    // 1. §¢§í§ã§ä§â§à§Ö §Ú§Ù§Ó§Ý§Ö§é§Ö§ß§Ú§Ö §Ó§â§Ö§Þ§Ö§ß§Ú §é§Ö§â§Ö§Ù §Ñ§á§á§Ñ§â§Ñ§ä§ß§à§Ö 32-§Ò§Ú§ä§ß§à§Ö §Õ§Ö§Ý§Ö§ß§Ú§Ö
    uint32_t seconds_in_day = ep % 86400UL;
    uint32_t days = ep / 86400UL;

    dt->hour = (uint8_t)(seconds_in_day / 3600UL);
    uint32_t res_seconds = seconds_in_day % 3600UL;
    dt->min = (uint8_t)(res_seconds / 60UL);
    dt->sec = (uint8_t)(res_seconds % 60UL);

    // 2. §²§Ñ§ã§é§Ö§ä §Õ§Ñ§ä§í (§Ñ§Ý§Ô§à§â§Ú§ä§Þ §Ò§Ö§Ù §Ú§ã§á§à§Ý§î§Ù§à§Ó§Ñ§ß§Ú§ñ 64-§Ò§Ú§ä§ß§í§ç §ä§Ú§á§à§Ó)
    uint32_t era_days = days + 719468UL;
    uint32_t era = era_days / 146097UL;
    uint32_t day_of_era = era_days - era * 146097UL;

    uint32_t year_of_era = (day_of_era - day_of_era / 1460UL + day_of_era / 36524UL - day_of_era / 146096UL) / 365UL;
    uint32_t y = year_of_era + era * 400UL;
    uint32_t day_of_year = day_of_era - (365UL * year_of_era + year_of_era / 4UL - year_of_era / 100UL);

    uint32_t m = (5UL * day_of_year + 2UL) / 153UL;
    dt->day = (uint8_t)(day_of_year - (153UL * m + 2UL) / 5UL + 1UL);
    dt->mon = (uint8_t)(m < 10 ? m + 3 : m - 9);
    dt->year = (uint16_t)(dt->mon <= 2 ? y + 1 : y);
}

//----------------------------------------------------------------------------------------
bool initRECQ (s_recq_t *q) {
    q->put = q->get = 0;
    for (uint8_t i = 0; i < MAX_SQREC; i++) {
        q->rec[i].evt = noneEvt;
        q->rec[i].data = NULL;
    }
    q->cnt = 0;

    return true;
}

//----------------------------------------------------------------------------------------
int8_t putRECQ (q_rec_t *rc, /* int8_t evt, void *data,*/ s_recq_t *q) {
    int8_t ret = noneEvt;

    while (q->lock) { }
    q->lock = true;

    if (q->rec[q->put].evt == noneEvt) {
        q->rec[q->put].evt = rc->evt;
        q->rec[q->put].data = rc->data;
        q->put++;
        if (q->put >= MAX_SQREC)
            q->put = 0;
        ret = rc->evt;
        q->cnt++;
    }
    q->lock = false;

    return ret;
}

//----------------------------------------------------------------------------------------
int8_t getRECQ (q_rec_t *rc, s_recq_t *q) {
    int8_t ret = noneEvt;

    if (q->lock)
        return ret;

    if (q->rec[q->get].evt != noneEvt) {
        ret = q->rec[q->get].evt;
        memcpy ((uint8_t *)rc, &q->rec[q->get].evt, sizeof (q_rec_t));
        if (q->rec[q->get].data) {
            free (q->rec[q->get].data);
            q->rec[q->get].data = NULL;
        }
        q->rec[q->get].evt = noneEvt;
    }

    if (ret > 0) {
        q->get++;
        if (q->get >= MAX_SQREC)
            q->get = 0;
    }
    if (q->cnt)
        q->cnt--;

    return ret;
}

//------------------------------------------------------------------------------------------
int8_t totalRECQ (s_recq_t *q) {
    return q->cnt;
}

//------------------------------------------------------------------------------------------
#ifdef SET_GPS
void PPS_INIT (void) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    EXTI_InitTypeDef EXTI_InitStructure = {0};
    NVIC_InitTypeDef NVIC_InitStructure = {0};

    RCC_APB2PeriphClockCmd (RCC_APB2Periph_AFIO | RCC_APB2Periph_GPIOA, ENABLE);

    /* PPS: GPIOA PA1 ----> EXTI_Line1 */
    GPIO_InitStructure.GPIO_Pin = PPS_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init (PPS_PORT, &GPIO_InitStructure);
    //
    GPIO_EXTILineConfig (GPIO_PortSourceGPIOA, GPIO_PinSource1);
    EXTI_InitStructure.EXTI_Line = EXTI_Line1;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init (&EXTI_InitStructure);

    /* KEY: GPIOA PA0 ----> EXTI_Line0 */
    GPIO_InitStructure.GPIO_Pin = KEY_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPD;
    GPIO_Init (KEY_PORT, &GPIO_InitStructure);
    //
    GPIO_EXTILineConfig (GPIO_PortSourceGPIOA, GPIO_PinSource0);
    EXTI_InitStructure.EXTI_Line = EXTI_Line0;
    EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising;
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init (&EXTI_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = EXTI7_0_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);
}

//
void EXTI7_0_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void EXTI7_0_IRQHandler (void) {
    if (EXTI_GetITStatus (EXTI_Line1) != RESET) {
        if (!sleep_mode) {
            pps_val = GPIO_ReadInputDataBit (PPS_PORT, PPS_PIN);
            if (queFlag && pps_enable) {
                q_rec_t rc = {ppsEvt, NULL};
                uint8_t *pval = calloc (1, sizeof (uint8_t));
                if (pval) {
                    *pval = pps_val;
                    rc.data = pval;
                }
                if (putRECQ (&rc, &queEvt) == noneEvt)
                    devError |= devQue;
            }
        }
        EXTI_ClearITPendingBit (EXTI_Line1);
    } else if (EXTI_GetITStatus (EXTI_Line0) != RESET) {
        key_val = GPIO_ReadInputDataBit (KEY_PORT, KEY_PIN);
        if (key_enable && key_val) {
            if (sleep_mode) {  // exit from sleep_mode
                SystemInit();
                if (queFlag) {
                    q_rec_t rc = {wupEvt, NULL};
                    if (putRECQ (&rc, &queEvt) == noneEvt)
                        devError |= devQue;
                }
                sleep_mode = false;
                key_enable = false;
            } else {  // go to sleep mode
                if (queFlag) {
                    q_rec_t rc = {slpEvt, NULL};
                    if (putRECQ (&rc, &queEvt) == noneEvt)
                        devError |= devQue;
                }
                // sleep_mode = true;
                key_enable = false;
            }
            key_tmr = get_sec (2);
        }
        EXTI_ClearITPendingBit (EXTI_Line0);
    }
}

//
void InitUSART2 (uint32_t speed) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA;
    RCC->APB1PCENR |= RCC_APB1Periph_USART2;

    // USART4 TX-->A.2   RX-->A.3
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOA, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = speed;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init (USART2, &USART_InitStructure);

    USART_ITConfig (USART2, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ (USART2_IRQn);

    USART_Cmd (USART2, ENABLE);
}

//
void USART2_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void USART2_IRQHandler (void) {
    if (USART_GetFlagStatus (USART2, USART_FLAG_RXNE) != RESET) {
        uint8_t ch = USART_ReceiveData (USART2);
        RxGps[ind_gps++] = ch;
        if (ch == 0x0a) {  // && (ind_gps > 2)) {
            ind_gps -= 2;
            RxGps[ind_gps] = '\0';
            if (!sleep_mode) {
                int dl = strlen (RxGps);
                if (queFlag && pps_enable && (dl > MIN_NMEA_LEN)) {
                    if (gpsValidate (RxGps)) {  // §¶§å§ß§Ü§è§Ú§ñ §á§â§à§Ó§Ö§â§ñ§Ö§ä §ß§Ñ §Ó§Ñ§Ý§Ú§Õ§ß§ã§ä§î §Õ§Ñ§ß§ß§í§Ö §à§ä GPS §Þ§à§Õ§å§Ý§ñ
                        q_rec_t rc = {gpsEvt, NULL};
                        char *msg = calloc (1, dl + 1);
                        if (msg) {
                            memcpy (msg, RxGps, dl + 1);
                            rc.data = msg;
                        }
                        if (putRECQ (&rc, &queEvt) == noneEvt)
                            devError |= devQue;
                    }
                }
            }
            ind_gps = 0;
            memset (RxGps, 0, sizeof (RxGps));
        }
    }
}

//------------------------------------------------------------------------------------------
#endif
//------------------------------------------------------------------------------------------
void InitUSART4 (uint32_t speed) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};
    USART_InitTypeDef USART_InitStructure = {0};

    RCC->APB2PCENR |= RCC_APB2Periph_GPIOB;
    RCC->APB1PCENR |= RCC_APB1Periph_USART4;
    AFIO->PCFR1 |= AFIO_PCFR1_USART4_REMAP;

    // USART4 TX-->B.0   RX-->B.1
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init (GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_1;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  // GPIO_Mode_IN_FLOATING;
    GPIO_Init (GPIOB, &GPIO_InitStructure);

    USART_InitStructure.USART_BaudRate = speed;  // 115200;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_Init (USART4, &USART_InitStructure);

    USART_ITConfig (USART4, USART_IT_RXNE, ENABLE);
    NVIC_EnableIRQ (USART4_IRQn);

    USART_Cmd (USART4, ENABLE);
}

//------------------------------------------------------------------------------
void USART4_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void USART4_IRQHandler (void) {
    if (USART_GetFlagStatus (USART4, USART_FLAG_RXNE) != RESET) {
        uint8_t ch = USART_ReceiveData (USART4);
        RxBuff[ind] = ch;
        USART_SendData (USART4, RxBuff[ind]);
        ind++;
        int8_t evts = noneEvt;
        if ((ch == 0x0a) || (ch == 0x0d)) {
            RxBuff[++ind] = 0;
            char *uk = NULL;
            if (strstr ((char *)RxBuff, "volt")) {         // show voltage
                evts = voltEvt;
            } else if (strstr ((char *)RxBuff, "rst")) {         // restart system
                evts = rstEvt;
            } else if (strstr ((char *)RxBuff, "rmc")) {  // on/off print RMC messages
                evts = rmcEvt;
                if (!rmc)
                    rmc = true;
                else
                    rmc = false;
            } else if (strstr ((char *)RxBuff, "gga")) {  // on/off print GGA messages
                evts = ggaEvt;
                if (!gga)
                    gga = true;
                else
                    gga = false;
            } else if (strstr ((char *)RxBuff, "sync")) {  // set time from gps data
                evts = syncEvt;
            } else if (strstr ((char *)RxBuff, "get")) {
                evts = getEvt;
            } else if ((uk = strstr ((char *)RxBuff, "epoch"))) {
                if ((uk = strchr ((char *)RxBuff, '='))) {  // set epoch time
                    epoch = atol (uk + 1);
                    set_time = true;
                } else {  // get epoch time
                    evts = timEvt;
                }
            } else
                evts = errEvt;
            ind = 0;
            memset (RxBuff, 0, sizeof (RxBuff));
        }

        if (!sleep_mode && queFlag && (evts != noneEvt)) {
            q_rec_t rc = {evts, NULL};
            if (putRECQ (&rc, &queEvt) == noneEvt)
                devError |= devQue;
        }
    }
}

//------------------------------------------------------------------------------
void TIM1_UP_IRQHandler (void) __attribute__ ((interrupt ("WCH-Interrupt-fast")));

void TIM1_UP_IRQHandler (void) {
    if (TIM_GetITStatus (TIM1, TIM_IT_Update) == SET) {
        Leds();
        if (set_time) {
            seconda = epoch + 1;
            set_time = false;
        }
        seconda++;

        if (queFlag && !sleep_mode) {
            q_rec_t rc = {secEvt, NULL};
            if (putRECQ (&rc, &queEvt) == noneEvt)
                devError |= devQue;
        }

        if (!oledOnOff && sleep_mode) {
            if (queFlag) {
                q_rec_t rc = {wupEvt, NULL};
                if (putRECQ (&rc, &queEvt) == noneEvt)
                    devError |= devQue;
            }
        }
    }
    TIM_ClearITPendingBit (TIM1, TIM_IT_Update);
}

//
void TIM1_Init (void)  // u16 arr, u16 psc)
{
    NVIC_InitTypeDef NVIC_InitStructure = {0};
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStructure = {0};

    RCC->APB2PCENR |= RCC_APB2Periph_TIM1;

    TIM_TimeBaseInitStructure.TIM_Period = arr;
    TIM_TimeBaseInitStructure.TIM_Prescaler = psc;
    TIM_TimeBaseInitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStructure.TIM_RepetitionCounter = 50;
    TIM_TimeBaseInit (TIM1, &TIM_TimeBaseInitStructure);

    TIM1->INTFR = (uint16_t)~TIM_IT_Update;  // TIM_ClearITPendingBit(TIM1, TIM_IT_Update);

    NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init (&NVIC_InitStructure);

    TIM1->DMAINTENR |= TIM_IT_Update;  // IM_ITConfig(TIM1, TIM_IT_Update, ENABLE);
    TIM1->CTLR1 |= TIM_CEN;            // TIM_Cmd(TIM1, ENABLE);
}

//------------------------------------------------------------------------------
void GPIO_ConfigOut (GPIO_TypeDef *port, uint16_t pin) {
    GPIO_InitTypeDef GPIO_InitStructure = {0};  // structure variable used for the GPIO configuration
    uint32_t bus = RCC_APB2Periph_GPIOA;

    if (port == GPIOB)
        bus = RCC_APB2Periph_GPIOB;
    else if (port == GPIOC)
        bus = RCC_APB2Periph_GPIOC;
    RCC->APB2PCENR |= bus;

    GPIO_InitStructure.GPIO_Pin = pin;                 // Defines which Pin to configure
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;   // Defines Output Type
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // Defines speed
    GPIO_Init (port, &GPIO_InitStructure);
}

//------------------------------------------------------------------------------
void Leds() {
    if (led)
        GPIO_ResetBits (LedPort, LedPin);
    else
        GPIO_SetBits (LedPort, LedPin);
    led = ~led;
}

//------------------------------------------------------------------------------
uint32_t get_sec (uint32_t t) {
    return (seconda + t);
}

//
int check_sec (uint32_t t) {
    return (get_sec (0) >= t) ? 1 : 0;
}

//------------------------------------------------------------------------------
int calcTime (uint32_t sec, char *st, bool all) {
    int ret = 0;

    DateTime_t dt;
    EpochToDateTime (sec, &dt);

    if (all)
        ret = sprintf (st, "%02u.%02u.%02u %02u:%02u:%02u", dt.day, dt.mon, dt.year - 2000, dt.hour, dt.min, dt.sec);
    else
        ret = sprintf (st, "%02u.%02u %02u:%02u:%02u", dt.day, dt.mon, dt.hour, dt.min, dt.sec);

    return ret;
}

//------------------------------------------------------------------------------
#if defined(SET_SSD1306_SPI)
void spi_init (void) {
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOA | RCC_APB2Periph_SPI1;
    RCC->APB2PCENR |= RCC_APB2Periph_GPIOB;

    GPIO_InitTypeDef GPIO_InitStructure = {0};
    GPIO_InitStructure.GPIO_Pin = CS_Pin | DC_Pin | RS_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // Defines speed
    GPIO_Init (GPIOB, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = RS_Pin;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // Defines speed
    GPIO_Init (GPIOA, &GPIO_InitStructure);

    // GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6;// Defines which Pin to configure - MISO
    // GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    // GPIO_Init(GPIOA, &GPIO_InitStructure);
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_7;          // Defines which Pin to configure : PA7-MOSI, PA5-SCK
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    // Defines Output Type
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // Defines speed
    GPIO_Init (GPIOA, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;          // Defines which Pin to configure - SCK
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;    // Defines Output Type
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;  // Defines speed
    GPIO_Init (GPIOA, &GPIO_InitStructure);

#ifdef SET_HARD_CS
    SPI_SSOutputCmd (SPI1, ENABLE);  // for hard NSS
#else
    SSD1306_UNSELECT();
    SSD1306_DC_CMD();
    SSD1306_RST_OFF();
#endif

    SPI_InitTypeDef spi_def = {0};
    spi_def.SPI_Direction = SPI_Direction_1Line_Tx;  // SPI_Direction_2Lines_FullDuplex;
    spi_def.SPI_Mode = SPI_Mode_Master;
    spi_def.SPI_DataSize = SPI_DataSize_8b;
#ifdef SET_HARD_CS
    spi_def.SPI_NSS = SPI_NSS_Hard;
#else
    spi_def.SPI_NSS = SPI_NSS_Soft;
#endif
    spi_def.SPI_CPOL = SPI_CPOL_High;
    spi_def.SPI_CPHA = SPI_CPHA_2Edge;                        // SPI_CPHA_1Edge;
    spi_def.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_2;  //_4;//_8
    spi_def.SPI_FirstBit = SPI_FirstBit_MSB;
    spi_def.SPI_CRCPolynomial = 0;
    SPI_Init (SPI1, &spi_def);
#ifdef SET_WITH_DMA
    SPI1->CTLR2 |= SPI_I2S_DMAReq_Tx;  // SPI_I2S_DMACmd(SPI1, ENABLE);
#endif

    // enable SPI port
    SPI1->CTLR1 |= CTLR1_SPE_Set;  // SPI_Cmd(SPI1, ENABLE);
}
#endif
//----------------------------------------------------------------------------------------
void Report (const char *tag, bool addTime, const char *fmt, ...) {
    va_list args;
    size_t len = MAX_UART_BUF;
    int dl = 0;

    char *buff = (char *)calloc (1, len);
    if (buff) {
        if (addTime) {
            dl = calcTime (get_sec (0), buff, false);  // date_to_str(buff);
            strcat (buff, " | ");
            dl += 3;
        }

        if (tag)
            dl += sprintf (buff + strlen (buff), "[%s] ", tag);

        va_start (args, fmt);
        vsnprintf (buff + dl, len - dl, fmt, args);
        printf ("%s", buff);
        va_end (args);

        free (buff);
    }
}
//------------------------------------------------------------------------------
const char *chipName()
{
    uint32_t cid = DBGMCU_GetCHIPID() & 0xffffff0f;
    switch (cid) {
        case 0x03500601:
            return "CH32X035R8T6";
        case 0x03510601:
            return "CH32X035C8T6";
        case 0x035E0601:
            return "CH32X035F8U6";
        case 0x03560601:
            return "CH32X035G8U6";
        case 0x035B0601:
            return "CH32X035G8R6";
        case 0x03570601:
            return "CH32X035F7P6";
        case 0x035A0601:
            return "CH32X033F8P6";
    }
    return "Unknown";
}
//------------------------------------------------------------------------------
void prnInfo() {
    Report (NULL, true, "%s RISC-V:%s SystemClk:%uHz Epoch:%u%s",
            ver, chipName(), SystemCoreClock, get_sec (0), eol);
}
//------------------------------------------------------------------------------
int main (void) {


    NVIC_PriorityGroupConfig (NVIC_PriorityGroup_1);
    SystemCoreClockUpdate();
    GPIO_ConfigOut (LedPort, LedPin);
    Delay_Init();

    queFlag = initRECQ (&queEvt);

    TIM1_Init();
    InitUSART4 (115200);
#ifdef SET_GPS
    PPS_INIT();
    InitUSART2 (9600);
#endif

#ifdef SET_ADC_VOLT
    voltPinInit();
    ADC_Function_Init();
    //ADC_SoftwareStartConvCmd(ADC1, ENABLE);
#endif

    Delay_Ms (1500);

    prnInfo();

#if defined(SET_SSD1306_SPI)
    spi_init();
    Delay_Ms (10);

    OLED_Init();
    Delay_Ms (50);

    OLED_text_xy (tmp, OLED_calcx (sprintf (tmp, "%s", uname)), LAST_LINE, inv);

#ifdef SET_SCROLL_MODE
    uint8_t shift_start = 3, shift_stop = shift_start + 0;
#endif

#endif

#ifdef SET_GPS
    char gBuf[192] = {0};
    char scr[MAX_GPS_BUF] = {0};
    uint8_t sch = 0;
    PPS_INIT();
    Delay_Ms (10);
    pps_enable = true;
#endif
    key_enable = true;
    sleep_mode = false;

    bool no_vld = true;
    q_rec_t rec = {0};
    // int8_t cnt = 0, cnt_last = 0;

    while (1) {
        if (queFlag) {
            evt = getRECQ (&rec, &queEvt);
            /*if (evt != noneEvt) {
                cnt = totalRECQ(&queEvt);
                if (cnt != cnt_last) {
                    cnt_last = cnt;
#ifdef SET_SSD1306_SPI
                    OLED_clear_line(2, inv);
                            OLED_text_xy(tmp, OLED_calcx(sprintf(tmp, "QUE:%d", cnt)), 2, inv);
#endif
                }
            }*/
            switch (evt) {
                case errEvt:
                    Report (NULL, true, "Error command !%s", eol);
                break;
                case rstEvt:
#ifdef SET_SSD1306_SPI
                    OLED_Clear();
#endif
                    Report (NULL, true, "Restart...%s%s", eol, eol);
                    Delay_Ms (100);
                    NVIC_SystemReset();
                break;
                case keyEvt:
                {
                    q_rec_t rec = {rstEvt, NULL};
                    if (putRECQ (&rec, &queEvt) == noneEvt) devError |= devQue;
                } 
                break;
                case slpEvt:
                    oledOnOff = false;
                    OLED_on (oledOnOff);
                    sleep_mode = true;
                    RCC_APB1PeriphClockCmd (RCC_APB1Periph_PWR, ENABLE);
                    printf ("Enter to sleep mode (sleep mode:%s)\r\n", sleep_mode ? "true" : "false");
                    PWR_EnterSTOPMode (PWR_STOPEntry_WFI);
                break;
                case wupEvt:
                    oledOnOff = true;
                    OLED_on (oledOnOff);
                    Delay_Ms (100);
                    sleep_mode = false;
                    printf ("Exit from sleep mode (sleep mode:%s)\n", sleep_mode ? "true" : "false");
                break;
                case secEvt: 
                {
#ifdef SET_SSD1306_SPI

                    char *st = &tmp[1];
                    tmp[0] = ' ';
                    int dl = calcTime (get_sec (0), st, false);
                    if (dl < MAX_CHAR_IN_LINE) {
                        strcat (st, " ");
                        dl++;
                    }
                    OLED_text_xy (tmp, OLED_calcx (dl), 1, inv);
#ifdef SET_SCROLL_MODE
                    if (!gps_valid) OLED_StartScroll (shift_start, shift_stop, true);
#endif
#endif
#ifdef SET_ADC_VOLT
                    uint16_t val = Get_ADC_Val(ADC_Channel_4);
                    //ADC_SoftwareStartConvCmd(ADC1, DISABLE);
                    volt = (float)val * 3.3f / 4095.0f;
                    if (volt < volt_porog)
                        ledVolt(VOLT_LED_ON);
                    else
                        ledVolt(VOLT_LED_OFF);
                    if (no_vld && !gps_valid) {
                        s_float_t flo = {0, 0};
                        floatPart(volt, &flo);
    #ifdef SET_SSD1306_SPI
                        OLED_clear_line(LAST_LINE, inv);
                        OLED_text_xy(scr, OLED_calcx(sprintf(scr, "volt:%u.%02u\n", flo.cel, flo.dro / 1000)), LAST_LINE, inv);
    #endif             
                    }
                    //ADC_SoftwareStartConvCmd(ADC1, ENABLE);
#endif
                } 
                break;
                case voltEvt:
                {
#ifdef SET_ADC_VOLT
                    s_float_t flo = {0, 0};
                    floatPart(volt, &flo);
                    Report(NULL, true, "volt:%u.%06u\n", flo.cel, flo.dro);
#endif                    
                }
                break;
                case timEvt:
                    Report (NULL, true, "Epoch=%u%s", get_sec (0), eol);
                break;
                case syncEvt:
                    set_new_time = true;
                break;
                case getEvt:
                    prnInfo();
#ifdef SET_SSD1306_SPI
                    OLED_text_xy (tmp, OLED_calcx (sprintf (tmp, "%s", uname)), LAST_LINE, inv);
#endif
                break;
                case ppsEvt:
#ifdef SET_GPS
                {
                    int8_t pv = -1;
                    if (rec.data) {
                        pv = *(uint8_t *)rec.data;
                        free (rec.data);
                    }
#ifdef SET_SSD1306_SPI
#ifndef OLED_128x32
                    OLED_clear_line (LAST_LINE, inv);
                    OLED_text_xy (tmp, OLED_calcx (sprintf (tmp, "PPS:%d", pv)), LAST_LINE, inv);
#endif
#endif
                }
#endif
                break;
                case gpsEvt:
                    if (!sleep_mode) {
#ifdef SET_GPS
                        if (rec.data) {
                            strcpy (gBuf, (char *)rec.data);
                            free (rec.data);
                            bool rt = gpsParse (gBuf);
                            if (rt) {
                                sch++;
                                if (sch == MAX_NMEA_MSG) {
                                    sch = 0;
                                    // §£§í§Ó§à§Õ §Õ§Ñ§ß§ß§í§ç §ß§Ñ OLED §Õ§Ú§ã§á§Ý§Ö§Û
                                    if (gps_valid) {
                                        no_vld = true;
#ifdef SET_SCROLL_MODE
                                        // if (scrollFlag)
                                        OLED_StopScroll();
                                        // OLED_clear_line(LAST_LINE, inv);
                                        // OLED_Scroll(shift_start, shift_stop, 0x2e);//0x2e - deactivate, 0x2f - activate
#endif
                                        //
                                        Report (NULL, true, "%s%s", gpsPrint (get_sec (0), gBuf), eol);
                                        //
                                        s_float_t flo = {0, 0};
                                        floatPart (GPS.dec_latitude, &flo);
                                        sprintf (scr, "  lat:%02u.%04u\n", flo.cel, flo.dro);                 // / 100);
                                        floatPart (GPS.dec_longitude, &flo);
                                        sprintf (scr + strlen (scr), " long:%02u.%04u\n", flo.cel, flo.dro);  // / 100);
                                        floatPart (GPS.msl_altitude, &flo);
                                        sprintf (scr + strlen (scr), " sat:%d alt:%u", GPS.satelites, flo.cel);
#ifdef SET_SSD1306_SPI
                                        OLED_clear_lines(GPS_LINE, GPS_LINE + 2, inv);
                                        OLED_text_xy(scr, 1, GPS_LINE, inv);
#endif
                                    } else {
#ifdef SET_SSD1306_SPI
                                        if (no_vld) {
                                            no_vld = false;
    #ifndef SET_ADC_VOLT
                                            OLED_clear_lines (GPS_LINE, GPS_LINE + 2, inv); 
                                            OLED_text_xy (tmp, OLED_calcx (sprintf (tmp, "%s", uname)), LAST_LINE, inv);
    #endif                                    
#ifdef SET_SCROLL_MODE
                                            if (!scrollFlag) OLED_StartScroll (shift_start, shift_stop, true);
                                            //OLED_Scroll(shift_start, shift_stop, 0x2f);//0x2e - deactivate, 0x2f - activate
#endif
                                        }
#endif
                                    }
                                    //
                                }
                            }  // else Report(NULL, true, "[%s]%s\n", rt ? "true" : "false", gBuf);
                        } else {
                            OLED_clear_line(LAST_LINE - 1, inv);
                            OLED_text_xy(tmp, OLED_calcx (sprintf(tmp, "gpsEvt")), LAST_LINE - 1, inv);
                        }
#endif
                    }
                    break;
            }
        }
        //
        if (key_tmr) {
            key_tmr = 0;
            key_enable = true;
        }
        //
        if (devError) {
            Report (NULL, true, "!!! devError=%u !!!%s", devError, eol);
#ifdef SET_SSD1306_SPI
            OLED_clear_line (LAST_LINE, inv);
            OLED_text_xy (tmp, OLED_calcx (sprintf (tmp, "devError:%u", devError)), LAST_LINE, inv);
#endif
            devError = 0;
        }
        //
        Delay_Ms (1);
        //
    }
}
