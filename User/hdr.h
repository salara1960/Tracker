/*
 * hrd.h
 *
 *  Created on: Dec 29, 2024
 *      Author: alarm
 */

#ifndef USER_HDR_H_
#define USER_HDR_H_


#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
#include <math.h>
#include <time.h>
#include "string.h"
#include <ch32x035.h>
#include "debug.h"


#define SET_SSD1306_SPI
#define SET_GPS
#define SET_FLOAT_PART



#if defined(SET_SSD1306_SPI)
	#define OLED_128x32

	#include "ssd1306.h"
	
	#define DC_Pin GPIO_Pin_3  //PB3
	#define CS_Pin GPIO_Pin_11 //PB11
	#define RS_Pin GPIO_Pin_6  //PA6
	//PA7-MOSI, PA5-SCK

	//#define SET_SCROLL_MODE
	#ifdef SET_SCROLL_MODE
		extern bool scrollFlag;

		extern void OLED_StartScroll(uint8_t start, uint8_t stop, bool left);
		extern void OLED_StopScroll(void);
		/*
		extern bool OLDE_SetScrollArea(uint8_t start, uint8_t stop);
		extern bool OLED_Scroll(uint8_t start, uint8_t stop, uint8_t on_off);//0x2e - deactivate, 0x2f - activate
		*/
	#endif
#endif

#ifdef SET_GPS
	#include "gps.h"

	#define PPS_PORT GPIOA
	#define PPS_PIN  GPIO_Pin_1
	#define MAX_GPS_BUF   128

	#define KEY_PORT GPIOA
	#define KEY_PIN  GPIO_Pin_0

	extern gps_t GPS;
#endif

#ifdef SET_FLOAT_PART
	typedef struct {
		uint32_t cel;
		uint32_t dro;
	} s_float_t;

	extern void floatPart(float val, s_float_t *part);
#endif

#define MAX_SQREC 32

#pragma pack(push,1)
typedef struct q_rec_t {
	int8_t evt;
	void *data;
} q_rec_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct s_recq_t {
	volatile bool lock;
	uint8_t cnt;
	uint8_t put;
	uint8_t get;
	q_rec_t rec[MAX_SQREC];
} s_recq_t;
#pragma pack(pop)

extern s_recq_t queEvt;
extern bool queFlag;


#define MAX_UART_BUF 1024
#define FMCLK	 25000000
#define LOOP_FOREVER() while(1) { Delay_Ms(1); }


enum {
	noneEvt = 0,
	rstEvt,
	secEvt,
	timEvt,
	syncEvt,
	rmcEvt,
	ggaEvt,
	getEvt,
	gpsEvt,
	ppsEvt,
	keyEvt,
	slpEvt,
	wupEvt,
	errEvt
};

enum {
	mFreq = 0,
	mForm,
	mStep
};

enum {
	devOK = 0,
	devTIK = 1,
	devUART = 2,
	devMEM = 4,
	devRTC = 8,
	devQue = 0x10,
	devGPS = 0x20,
	devSPI = 0x40,
	devCRC = 0x80,
};

extern uint16_t devError;
extern volatile uint32_t epoch;
extern bool set_time;
extern bool set_new_time;

extern bool oledOnOff;

extern int calcTime(uint32_t sec, char *st, bool all);
extern uint32_t DateTimeToEpoch(const DateTime_t *dt);
extern void EpochToDateTime(uint32_t ep, DateTime_t *dt);
extern void Report(const char *tag, bool addTime, const char *fmt, ...);

#endif /* USER_HDR_H_ */
