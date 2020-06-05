/* ========================================
 *
 * Copyright (c) 2020 Takumi Niwa
 * This software is released under the MIT license.
 *
 * ========================================
*/
#include "project.h"

#include "FreeRTOS.h"
#include "task.h"

#include "USBUART_FreeRTOS.h"

void FreeRtosSetup();
void FreeRtosStart();

void UartEchoBackTask(){ /* Test */
    char buf;    
    
    for(;;){     
        LED_0_Write(~LED_0_Read());
        USBUARTGetString(&buf,1);
        USBUARTPutString(&buf,1);
    }
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    FreeRtosSetup();
    USBUARTStart();
    
    FreeRtosStart();
    for(;;)
    {
        /* Place your application code here. */
        LED_0_Write(~LED_0_Read());
        //LED_1_Write(~LED_1_Read());
        
        CyDelay(1000);
    }
}

void FreeRtosSetup(){
    extern void xPortPendSVHandler( void );
    extern void xPortSysTickHandler( void );
    extern void vPortSVCHandler( void );
    extern cyisraddress CyRamVectors[];

    CyRamVectors[ 11 ] = ( cyisraddress ) vPortSVCHandler;
    CyRamVectors[ 14 ] = ( cyisraddress ) xPortPendSVHandler;
    CyRamVectors[ 15 ] = ( cyisraddress ) xPortSysTickHandler;
}

void FreeRtosStart(){
    xTaskCreate(UartEchoBackTask,"EchoBack",1000,NULL,3,NULL);//Echo back test task
    
    vTaskStartScheduler();
}
/* [] END OF FILE */
