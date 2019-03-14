#include "ql_trace.h"
#include "ql_system.h"
#include "ql_adc.h"
#include "ql_uart.h"
#include "ql_stdlib.h"
#include "ql_error.h"
#include "ql_type.h"
#include "ql_gpio.h"

#include "debug.h"
#include "Digitron.h"

static u32 ADC_CustomParam = 1;
 float Power_voltage = 0.00;

  Enum_PinName adcPin = PIN_ADC0;

static void CallBack_UART_Hdlr(Enum_SerialPort port, Enum_UARTEventType msg, bool level, void* customizedPara)
{
     
}

static void Callback_OnADCSampling(Enum_ADCPin adcPin, u32 adcValue, void *customParam)
{
    //APP_DEBUG("<-- Callback_OnADCSampling: sampling voltage(mV)=%d  times=%d -->\r\n", adcValue, *((s32*)customParam))
    *((s32*)customParam) += 1;
	Power_voltage = (float )adcValue / 1000;
	APP_DEBUG("sampling voltage=%f\r\n",Power_voltage);
	DIG_DISP(Power_voltage);
	
    //Ql_ADC_Sampling(adcPin, FALSE);
}




 void ADC_Program(void)
{
    //Enum_PinName adcPin = PIN_ADC0;

    // Register callback foR ADC
    APP_DEBUG("<-- Register callback for ADC -->\r\n")
    Ql_ADC_Register(adcPin, Callback_OnADCSampling, (void *)&ADC_CustomParam);

    // Initialize ADC (sampling count, sampling interval)
    APP_DEBUG("<-- Initialize ADC (sampling count=5, sampling interval=200ms) -->\r\n")
    Ql_ADC_Init(adcPin, 5, 400);

    // Start ADC sampling
    APP_DEBUG("<-- Start ADC sampling -->\r\n")
    Ql_ADC_Sampling(adcPin, TRUE);

    // Stop  sampling ADC
    //Ql_ADC_Sampling(adcPin, FALSE);
}


 void ADC_Start(void)
 {
         // Start ADC sampling
    APP_DEBUG("<-- Start ADC sampling -->\r\n")
    Ql_ADC_Sampling(adcPin, TRUE);
 }