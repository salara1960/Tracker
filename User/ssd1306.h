#ifndef __SSD1306_H_
#define __SSD1306_H_


#include "hdr.h"

#if defined(SET_SSD1306_SPI)


#ifndef M_PI
	#define M_PI (3.14159265358979323846)
#endif

#define OLED_CONTROL_BYTE_CMD_SINGLE    0x80
#define OLED_CONTROL_BYTE_CMD_STREAM    0x00
#define OLED_CONTROL_BYTE_DATA_STREAM   0x40
#define OLED_CONTROL_DISPLAY_OFF        0xAE
#define OLED_CONTROL_DISPLAY_ON         0xAF
//

// Fundamental commands (pg.28)
#define OLED_CMD_SET_CONTRAST           0x81    // follow with 0x7F
#define OLED_CMD_DISPLAY_RAM            0xA4
#define OLED_CMD_DISPLAY_ALLON          0xA5
#define OLED_CMD_DISPLAY_NORMAL         0xA6
#define OLED_CMD_DISPLAY_INVERTED       0xA7
#define OLED_CMD_DISPLAY_OFF            0xAE
#define OLED_CMD_DISPLAY_ON             0xAF

// Addressing Command Table (pg.30)
#define OLED_CMD_SET_MEMORY_ADDR_MODE   0x20    // follow with 0x00 = HORZ mode = Behave like a KS108 graphic LCD
#define OLED_CMD_SET_COLUMN_RANGE       0x21    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x7F = COL127
#define OLED_CMD_SET_PAGE_RANGE         0x22    // can be used only in HORZ/VERT mode - follow with 0x00 and 0x07 = PAGE7

#define OLED_CMD_SET_DEACTIVATE_SCROLL  0x2E
#define OLED_CMD_SET_ACTIVATE_SCROLL    0x2F
#define OLED_CMD_SHIFT_STOP             0x2e    // deactivate shift
#define OLED_CMD_SHIFT_START            0x2f    // activate shift
#define OLED_VERTICAL_AND_RIGHT_HORIZONTAL_SCROLL 0x29 ///< Init diag scroll
#define OLED_VERTICAL_AND_LEFT_HORIZONTAL_SCROLL 0x2A ///< Init diag scroll
#define OLED_RIGHT_HORIZONTAL_SCROLL    0x26
#define OLED_LEFT_HORIZONTAL_SCROLL     0x27
#define OLED_SET_VERTICAL_SCROLL_AREA   0xA3 ///< Set scroll range

// Hardware Config (pg.31)
#define OLED_CMD_SET_DISPLAY_START_LINE 0x40
#define OLED_CMD_SET_SEGMENT_REMAP      0xA1
#define OLED_CMD_SET_MUX_RATIO          0xA8    // follow with 0x3F = 64 MUX or 1F = 32 MUX
#define OLED_CMD_SET_COM_SCAN_MODE      0xC8
#define OLED_CMD_SET_DISPLAY_OFFSET     0xD3    // follow with 0x00
#define OLED_CMD_SET_COM_PIN_MAP        0xDA    // follow with 0x12
#define OLED_CMD_NOP                    0xE3    // NOP

// Timing and Driving Scheme (pg.32)
#define OLED_CMD_SET_DISPLAY_CLK_DIV    0xD5    // follow with 0x80
#define OLED_CMD_SET_PRECHARGE          0xD9    // follow with 0xF1
#define OLED_CMD_SET_VCOMH_DESELCT      0xDB    // follow with 0x30

// Charge Pump (pg.62)
#define OLED_CMD_SET_CHARGE_PUMP        0x8D    // follow with 0x14

#define OLED_DUMMY_BYTE                 0x00
#define OLED_TIME_INTERVAL              0x00    // 0 -   5 frame
												// 1 -  64 frame
                                                // 2 - 128 frame
                                                // 3 - 256 frame
                                                // 4 -   3 frame
                                                // 5 -   4 frame
                                                // 6 -  25 frame
                                                // 7 -   2 frame

// Change if needed
#define OLED_WIDTH       128
#define MAX_CHAR_IN_LINE  16 
#ifdef OLED_128x32
	#define OLED_HEIGHT 32
	#define LAST_LINE    4
	#define GPS_LINE     2
#else
	#define OLED_HEIGHT 64
	#define LAST_LINE    8
	#define GPS_LINE     3
#endif


// Display Powered with External Supply (EXTERNAL_VCC)
#ifdef EXTERNAL_VCC
	#define BOARD_POWER 0x01
#else
	// Display Powered with Board's Supply (SWITCH_APVCC)
	#define BOARD_POWER 0x02
#endif


#define SSD1306_SELECT() (GPIO_ResetBits(GPIOB, CS_Pin))
#define SSD1306_UNSELECT() (GPIO_SetBits(GPIOB, CS_Pin))
#define SSD1306_DC_CMD() (GPIO_ResetBits(GPIOB, DC_Pin))
#define SSD1306_DC_DATA() (GPIO_SetBits(GPIOB, DC_Pin))
#define SSD1306_RST_ON() (GPIO_ResetBits(GPIOA, RS_Pin))
#define SSD1306_RST_OFF() (GPIO_SetBits(GPIOA, RS_Pin))


enum {
	iDat = 0,
	iCmd
};


int OLED_Init(void);
void OLED_on(bool flag);
void OLED_Clear(void);
void OLED_clear_line(uint8_t cy, bool inv);
void OLED_clear_lines(uint8_t start, uint8_t end, bool inv);
uint8_t OLED_calcx(int len);
void OLED_text_xy(const char *stroka, uint8_t cx, uint8_t cy, bool inv);
#ifdef SET_SCROLL_MODE
	bool scrollFlag;

	void OLED_StartScroll(uint8_t start, uint8_t stop, bool left);
	void OLED_StopScroll(void);
	/*
	bool OLDE_SetScrollArea(uint8_t start, uint8_t stop);
	bool OLED_Scroll(uint8_t start, uint8_t stop, uint8_t on_off);//0x2e - deactivate, 0x2f - activate
	*/
#endif


#endif

#endif
