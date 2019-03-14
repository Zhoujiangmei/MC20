#ifndef __METER_H__
#define __METER_H__

extern u8 taxiBuf[50] ;
#define   Test_METER
#define EDPDATASEND  0
#define EDPDATASAVE  1

void MeterSerialInit(void );
void TaxiMeterLoop(void);
void twobytes2onehex(u8* bytebuf,u8* hexbuf);



#endif
