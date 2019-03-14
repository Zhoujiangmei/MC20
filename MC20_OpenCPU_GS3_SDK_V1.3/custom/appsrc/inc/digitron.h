#ifndef __DIGITRON_H__
#define __DIGITRON_H__


#define DIGBITDELAY 510//Լ52uS


void DIG_INIT(void);
void DIG_DISP(float val);
void DIG_DISP_CLR(void);
void DIG_Brightness_Control(unsigned int level);

void DIG_Brightness_Control_Test(void);
void testTime(void );

void DIGSDAH(void );
void DIGSDAL(void );
#endif
