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
#include "LPS22HB.h"
void FreeRtosSetup();
void FreeRtosStart();

static SemaphoreHandle_t xLPSBinarySemaphor;

CY_ISR(temp){
    LED_1_Write(~LED_1_Read());
    BaseType_t xHigherPriorityTaskWoken;
    
    xHigherPriorityTaskWoken = pdFALSE;
    
    xSemaphoreGiveFromISR(xLPSBinarySemaphor, &xHigherPriorityTaskWoken);
    
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}

void UartEchoBackTask(){ /* Test */
    char buf;    
    
    for(;;){     
        LED_0_Write(~LED_0_Read());
        USBUARTGetString(&buf,1);
        USBUARTPutString(&buf,1);
    }
}

void test(){
    char buf[64];
    uint32_t time = 0;
    int num=0;
    LPS22HB_DATA_CONTEINER_t lps22hb_data;
    //LPS22HB_CONFIG_t lps22hb_config;
    LPS22HBStart();
    LPS22HBInitializeConfig();
    
    LPS22HBWhoAmI();

    for(;;){
        xSemaphoreTake(xLPSBinarySemaphor,portMAX_DELAY);
        time = 4294967296-Timer_ReadCounter();
        
        LED_0_Write(~LED_0_Read());
        for(num = 0;num < 32;num++){
            lps22hb_data = LPS22HBUpdateData();
            
            sprintf(buf,"%07d,%d,%f,%f\r\n",time,num,LPS22HBGetPress(lps22hb_data),LPS22HBGetTemp(lps22hb_data));
            USBUARTPutString(buf,strlen(buf));
            //vTaskDelay(135);
        }
    }
}

int main(void)
{
    CyGlobalIntEnable; /* Enable global interrupts. */

    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    FreeRtosSetup();
    USBUARTStart();
    SPIM_LPS_Start();
    
    Timer_Start();
    xLPSBinarySemaphor = xSemaphoreCreateBinary();
    //xSemaphoreGive(xLPSBinarySemaphor);
    isr_1_StartEx(temp);
    FreeRtosStart();
    for(;;)
    {
        /* Place your application code here. */

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
    xTaskCreate(test,"EchoBack",1000,NULL,3,NULL);//Echo back test task
    
    vTaskStartScheduler();
}
/* [] END OF FILE */
