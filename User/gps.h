#ifndef __GPS_H__
#define __GPS_H__

#include "hdr.h"


#ifdef SET_GPS

#define MAX_NMEA_MSG   2//3//4
#define NMEA_TYPE_LEN  6
#define MIN_NMEA_LEN  14
#define MAX_LIST_ITEM 24
#define MAX_ITEM_LEN  20


enum {
    ixNone = -1,
	ixGNGGA,
	ixGNRMC//,
	//ixGNGLL,
	//ixGNVTG
};

//  Структура с переменными для запоминания данных геолокации
#pragma pack(push,1)
typedef struct {
    uint16_t year;   // Год (например, 2026)
    uint8_t  mon;  // Месяц (1 - 12)
    uint8_t  day;    // День (1 - 31)
    uint8_t  hour;   // Час (0 - 23)
    uint8_t  min; // Минута (0 - 59)
    uint8_t  sec; // Секунда (0 - 59)
} DateTime_t;
#pragma pack(pop)

#pragma pack(push,1)
typedef struct {
    // calculated values
    float dec_longitude;
    float dec_latitude;
    float altitude_ft;

    // GGA - Global Positioning System Fixed Data
    float nmea_longitude;
    float nmea_latitude;
    float utc_time;
    char ns, ew;
    //int lock;
    int satelites;
    //float hdop;
    float msl_altitude;
    char msl_units;
    float geoid;
    // RMC - Recommended Minimmum Specific GNS Data
    char rmc_status;
    float speed_k;
    float course_d;
    bool valid;
    //
    DateTime_t dt;
    uint32_t epoch;
    uint8_t mlsec;
} gps_t;
#pragma pack(pop)

/*
struct tm {
    int tm_sec;    // Seconds (0-60)
    int tm_min;    // Minutes (0-59)
    int tm_hour;   // Hours (0-23)
    int tm_mday;   // Day of the month (1-31)
    int tm_mon;    // Month (0-11)
    int tm_year;   // Year - 1900
    int tm_wday;   // Day of the week (0-6, Sunday = 0)
    int tm_yday;   // Day in the year (0-365, 1 Jan = 0)
    int tm_isdst;  // Daylight saving time 
};
*/

//

//
extern bool rmc;
extern bool gga;
extern bool gps_valid;
extern gps_t GPS;

int gpsValidate(const char *str);
float gpsToDec(float deg, char nsew);
bool gpsParse(char *str);
char *gpsPrint(uint32_t sec, char *str);
uint8_t hexToBin(char *sc);
uint32_t DateTimeToEpoch(const DateTime_t *dt);
void EpochToDateTime(uint32_t ep, DateTime_t *dt);



#endif

#endif
