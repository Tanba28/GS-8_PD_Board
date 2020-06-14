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

#include "sd_api.h"
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
    FATFS fatfs;
    FRESULT	fres;
    FIL file;
    UINT byte;
    
    LPS22HB_DATA_CONTEINER_t lps22hb_data;
    //LPS22HB_CONFIG_t lps22hb_config;
    
    if((fres = f_mount(&fatfs,"",1)) != FR_OK){
        sprintf(buf,"SD Card Mount Failed.\r\n");
        USBUARTPutString(buf,strlen(buf));
        
        switch(fres){
            case FR_INVALID_DRIVE:
                sprintf(buf,"FR_INVALID_DRIVE\r\n");
                break;
            case FR_DISK_ERR:
                sprintf(buf,"FR_DISK_ERR\r\n");
                break;
            case FR_NOT_READY:
                sprintf(buf,"FR_NOT_READY\r\n");
                break;
            case FR_NO_FILESYSTEM:
                sprintf(buf,"FR_NO_FILESYSTEM\r\n");
                break;
            default:
                sprintf(buf,"UNKNOWN ERROR\r\n");
                break;
        }
        USBUARTPutString(buf,strlen(buf));
        LED_1_Write(0);
        for(;;);
    }  
    
    else{
        sprintf(buf,"SD Card Mounted\r\n");
        USBUARTPutString(buf,strlen(buf));
        
        switch(fatfs.fs_type){
        case FS_FAT12:
            sprintf(buf,"FS_FAT12\r\n");
            break;
        case FS_FAT16:
            sprintf(buf,"FS_FAT16\r\n");
            break;
        case FS_FAT32:
            sprintf(buf,"FS_FAT32\r\n");
            break;
        default:
            sprintf(buf,"?\r\n");
            break;
        }
        USBUARTPutString(buf,strlen(buf));
    }    
    fres = f_open(&file,"test.csv",FA_CREATE_ALWAYS|FA_WRITE);
    
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
            f_puts(buf,&file);
            //vTaskDelay(135);
        }
        f_sync(&file);
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
