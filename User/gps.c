
#include "hdr.h"

#include "gps.h"


#ifdef SET_GPS
//-----------------------------------------------------------------------------
bool rmc = false;
bool gga = false;
bool gps_valid = false;;
gps_t GPS = {0};// структура для данных геолокации
static char item[MAX_LIST_ITEM][MAX_ITEM_LEN];

const char *nmea[MAX_NMEA_MSG] = {// cимвольные маркеры NMEA сообщений, которые будут анализироваться
		"$GNGGA",
		"$GNRMC"
		//"$GNGLL",
		//"$GNVTG"
};

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
//  Функция проверяет на валидность строку на соответствие NMEA формату
//
int gpsValidate(const char *str)
{
int i = 0;

	if (strlen(str) < MIN_NMEA_LEN) return 0;

    if (str[i] != '$') return 0; else i++;

    int8_t j = -1;
    for (int8_t k = 0; k < MAX_NMEA_MSG; k++) {
    	if (strstr(str, nmea[k])) {
    		j = k;
    		break;
    	}
    }
    if (j == -1) return 0; else return 1;
}
//-----------------------------------------------------------------------------
//            Пересчет данных геолокации в градусы
//
float gpsToDec(float deg, char nsew)
{
    int degree = (int)(deg / 100);
    float minutes = deg - degree * 100;
    float dec_deg = minutes / 60;
    float decimal = degree + dec_deg;
    if (nsew == 'S' || nsew == 'W') decimal *= -1;

    return decimal;
}
//-----------------------------------------------------------------------------
void prnMsgList(int it)
{
char *tmp = (char *)calloc(1, 32 + (MAX_ITEM_LEN + 8) * MAX_LIST_ITEM);//1024);

    if (!tmp) return;

    sprintf(tmp, "items=%d  ", it);

    for (int i = 0; i < MAX_LIST_ITEM; i++)
        if (strlen(item[i])) sprintf(tmp+strlen(tmp), ", %02d:'%s'", i, item[i]);
    Report(NULL, false, "\t%s\n", tmp);

    free(tmp);
}
//-----------------------------------------------------------------------------
// Делает из строки список из n-элементов (не более 24 элементов)
//
int splitMsg(const char *str)
{ 
int ret = -1, dl = 0;

    char *us = strchr(str, ',');
    if (!us) return ret;
    us++;

    char *ue = us;
    char *end = strchr(str, '*');
    if (!end) return ret;

    memset((uint8_t *)&item[0], 0, MAX_ITEM_LEN * MAX_LIST_ITEM);

    bool loop = true;

    while (loop) {
        ue = strchr(us, ',');
        if (!ue) { ue = end; loop = false; }
        if (ue) {
            ret++;
            dl = ue - us;
            if (dl > 0) {
                if (dl > MAX_ITEM_LEN) dl = MAX_ITEM_LEN - 1; 
                memcpy(&item[ret][0], us, dl);
            }
            us = ue + 1;
        } else loop = false;
    } 

    ret++;

    //prnMsgList(ret);

    return ret;
}
//-----------------------------------------------------------------------------
//      Преобразует два символа строки из hex-формата в двоичный
//
uint8_t hexToBin(char *sc)
{
char st = 0, ml = 0;

	if ((sc[0] >= '0') && (sc[0] <= '9')) st = (sc[0] - 0x30);
	else
	if ((sc[0] >= 'A') && (sc[0] <= 'F')) st = (sc[0] - 0x37);
	else
	if ((sc[0] >= 'a') && (sc[0] <= 'f')) st = (sc[0] - 0x57);

	if ((sc[1] >= '0') && (sc[1] <= '9')) ml = (sc[1] - 0x30);
	else
	if ((sc[1] >= 'A') && (sc[1] <= 'F')) ml = (sc[1] - 0x37);
	else
	if ((sc[1] >= 'a') && (sc[1] <= 'f')) ml = (sc[1] - 0x57);

	return ((st << 4) | (ml & 0x0f));

}
//-----------------------------------------------------------------------------
//   Парсер валидных NMEA сообщений и заполнение структуры данными геолокации
//
bool gpsParse(char *str)
{
bool ret = false;
int8_t idx = -1;

	for (int8_t i = 0; i < MAX_NMEA_MSG; i++) {
		if (!strncmp(str, nmea[i], NMEA_TYPE_LEN)) {
			idx = i;
			break;
		}
	}
	if (idx == -1) return ret;

	//  Подсчет контрольной суммы NMEA сообщения
	char sc[2] = {0};
	uint8_t crc_in = 255, crc_calc = 0;
	char *uk = strchr(str, '*');
	if (uk) {
		memcpy(sc, uk + 1, 2);
		crc_in = hexToBin(sc);
		char *us = strchr(str, '$');
		if (us) {
			us++;
			if (uk > us) {
				while(us < uk) crc_calc ^= *us++;
			}
		}
	}
	//  Проверка контрольной суммы
	if (crc_in != crc_calc) {
		devError |= devCRC;
		return ret;
	} else {
		if (devError & devCRC) devError &= ~devCRC;
	}

	int res = splitMsg(str);
    if (res <= 0) {
        devError |= devGPS;
        return ret;
    } else {
		ret = true;
		char tmp[16] = {0};
		switch (idx) {
            case ixGNGGA://$GNGGA,081549.00,5550.6198987,N,03732.2783241,E,1,20,1.03,173.2604,M,15.0999,M,,*5C
                         //$GNGGA,115349.00,5550.602007,N,03732.263725,E,1,16,2.3,00146.429,M,0014.442,M,,*6E
                if (gga) Report(NULL, true, "%s\n", str);
                
                if (strlen(item[6])) GPS.satelites      = atoi(item[6]);
                if (strlen(item[8])) GPS.msl_altitude       = atof(item[8]);
                if (strlen(item[9])) GPS.msl_units = item[9][0];
                if (strlen(item[10])) GPS.geoid         = atof(item[10]);
            break;
            case ixGNRMC://$GNRMC,081549.00,A,5550.6198987,N,03732.2783241,E,0.0232,003.400,090224,12.054,W,A*3A
                         //$GNRMC,115348.90,A,5550.602007,N,03732.263725,E,000.00000,179.3,190624,,,A*5D
                if (rmc) Report(NULL, true, "%s\n", str);
                if (strlen(item[0])) GPS.utc_time   = atof(item[0]);
                if (strlen(item[1])) GPS.rmc_status = item[1][0];
                if (GPS.rmc_status == 'A') {
                    GPS.valid = gps_valid = true;
                } else {
                    GPS.valid = gps_valid = false; 
                }
                if (strlen(item[2])) GPS.nmea_latitude  = atof(item[2]);
                if (strlen(item[3])) GPS.ns             = item[3][0];
                if (strlen(item[4])) GPS.nmea_longitude = atof(item[4]);
                if (strlen(item[5])) GPS.ew             = item[5][0];
                if (strlen(item[6])) GPS.speed_k        = atof(item[6]);
                if (strlen(item[7])) GPS.course_d       = atof(item[7]);
                if (strlen(item[8]) == 6) {
                    memcpy(tmp, &item[8][0], 2); GPS.dt.day  = atol(tmp);
                    memcpy(tmp, &item[8][2], 2); GPS.dt.mon  = atol(tmp);
                    memcpy(tmp, &item[8][4], 2); GPS.dt.year = atol(tmp) + 2000; 
                }
                if (strlen(item[0]) >= 6) {
                    memcpy(tmp, &item[0][0], 2); GPS.dt.hour = atol(tmp); 
                    memcpy(tmp, &item[0][2], 2); GPS.dt.min  = atol(tmp);
                    memcpy(tmp, &item[0][4], 2); GPS.dt.sec  = atol(tmp);
                    char *ukz = strchr(item[0], '.');
                    if (ukz) GPS.mlsec = atol(ukz + 1);
					GPS.epoch = DateTimeToEpoch(&GPS.dt);
                    if (gps_valid) {
                        if (!set_new_time) {
                            set_new_time = true;
                            epoch = GPS.epoch;
                            set_time = true;
                        }
                    }
                }
                GPS.dec_latitude  = gpsToDec(GPS.nmea_latitude,  GPS.ns);
                GPS.dec_longitude = gpsToDec(GPS.nmea_longitude, GPS.ew);
            break;
				default : ret = false;
        }
	}

    return ret;
}
//-----------------------------------------------------------------------------
//        Функция формирует символьную строку с данными геолокации
// с использованием функции разделения целой и дробной частей числа типа float
//
char *gpsPrint(uint32_t sec, char *str)
{
	if (str) {
#ifdef SET_FLOAT_PART
		s_float_t flo = {0,0};
		sprintf(str, "epoch:%u date:%02u.%02u.%02u time:%02u:%02u:%02u",
                     GPS.epoch,
					 GPS.dt.day, GPS.dt.mon, GPS.dt.year - 2000,
					 GPS.dt.hour, GPS.dt.min, GPS.dt.sec); 
		floatPart(GPS.dec_latitude, &flo); 	sprintf(str+strlen(str), " lat:%u.%u", flo.cel, flo.dro);
		floatPart(GPS.dec_longitude, &flo); sprintf(str+strlen(str), " lon:%u.%u sat:%d", flo.cel, flo.dro, GPS.satelites);
		floatPart(GPS.msl_altitude, &flo);  sprintf(str+strlen(str), " alt:%u.%01u", flo.cel, flo.dro/100000);
		floatPart(GPS.speed_k, &flo);       sprintf(str+strlen(str), " spd:%u.%02u", flo.cel, flo.dro/10000);
		floatPart(GPS.course_d, &flo);      sprintf(str+strlen(str), " dir:%u.%02u", flo.cel, flo.dro/10000);
#else

		sprintf(str, "time:%f date:%d lat:%.4f long:%.4f sat:%d alt:%.2f speed:%.2f dir:%.2f",
				GPS.utc_time, GPS.date, GPS.dec_latitude, GPS.dec_longitude,
				GPS.satelites, GPS.msl_altitude, GPS.speed_km, GPS.course_m);
#endif
	}

	return str;
}
//-----------------------------------------------------------------------------
#endif

