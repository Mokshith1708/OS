#include "xparameters.h"
#include "xaxivdma.h"
#include "xscugic.h"
#include "sleep.h"
#include <stdlib.h>
#include "xil_cache.h"
#include "xil_cache.h"
#include "xgpiops.h"
#include "xsdps.h"
#include "hal/hal_console.h"
#include "shell.h"
#include "ff.h"

#define HSize 1920
#define VSize 1080
#define FrameSize HSize *VSize * 3
#define GPIO_RESET_PIN 54
#define imgHSize 512
#define imgVSize 512

static XScuGic Intc;

static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId);
unsigned char Buffer[FrameSize];
__attribute__((section(".frame_buffer"))) int drawImage(u32 displayHSize, u32 displayVSize, u32 imageHSize, u32 imageVSize, u32 hOffset, u32 vOffset, char *imagePointer);

XGpioPs Gpio;
FATFS fatfs;
FRESULT res;

int main()
{
    int status;
    // Hold Softcore in Reset
    XGpioPs_Config *GpioConfig = XGpioPs_LookupConfig(XPAR_XGPIOPS_0_DEVICE_ID);
    if (GpioConfig == NULL)
    {
        xil_printf("GPIO config lookup failed\n");
        return XST_FAILURE;
    }

    status = XGpioPs_CfgInitialize(&Gpio, GpioConfig, GpioConfig->BaseAddr);
    if (status != XST_SUCCESS)
    {
        xil_printf("GPIO initialization failed\n");
        return XST_FAILURE;
    }

    XGpioPs_SetDirectionPin(&Gpio, GPIO_RESET_PIN, 1);
    XGpioPs_SetOutputEnablePin(&Gpio, GPIO_RESET_PIN, 1);

    XGpioPs_WritePin(&Gpio, GPIO_RESET_PIN, 1);

    hal_console_init(); // Initialize UART

    // Initialize SD card

    XSdPs SdInstance;
    XSdPs_Config *sd_config;
    sd_config = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
    if (!sd_config)
    {
        xil_printf("SD lookup config failed\n");
        return XST_FAILURE;
    }
    status = XSdPs_CfgInitialize(&SdInstance, sd_config, sd_config->BaseAddress);
    if (status != XST_SUCCESS)
    {
        xil_printf("SD initialization failed\n");
        return status;
    }

    // Optionally power on SD card and setup clocks if needed

    // Mount the file system on the SD card
    res = f_mount(&fatfs, "0:", 1);
    if (res != FR_OK)
    {
        xil_printf("Failed to mount filesystem. FatFs error = %d\n", res);
        return XST_FAILURE;
    }

    xil_printf("SD card mounted successfully\n");

    // Initialize VDMA
    int Index;
    u32 Addr;
    XAxiVdma myVDMA;
    XAxiVdma_Config *config = XAxiVdma_LookupConfig(XPAR_AXI_VDMA_0_DEVICE_ID);
    XAxiVdma_DmaSetup ReadCfg;
    status = XAxiVdma_CfgInitialize(&myVDMA, config, config->BaseAddress);
    if (status != XST_SUCCESS)
    {
        xil_printf("DMA Initialization failed");
    }
    ReadCfg.VertSizeInput = VSize;
    ReadCfg.HoriSizeInput = HSize * 3;
    ReadCfg.Stride = HSize * 3;
    ReadCfg.FrameDelay = 0;
    ReadCfg.EnableCircularBuf = 1;
    ReadCfg.EnableSync = 1;
    ReadCfg.PointNum = 0;
    ReadCfg.EnableFrameCounter = 0;
    ReadCfg.FixedFrameStoreAddr = 0;
    status = XAxiVdma_DmaConfig(&myVDMA, XAXIVDMA_READ, &ReadCfg);
    if (status != XST_SUCCESS)
    {
        xil_printf("Write channel config failed %d\r\n", status);
        return status;
    }

    Addr = (u32) & (Buffer[0]);

    xil_printf("Frame Buffer Address: %x", Addr);

    for (Index = 0; Index < myVDMA.MaxNumFrames; Index++)
    {
        ReadCfg.FrameStoreStartAddr[Index] = Addr;
        Addr += FrameSize;
    }

    status = XAxiVdma_DmaSetBufferAddr(&myVDMA, XAXIVDMA_READ, ReadCfg.FrameStoreStartAddr);
    if (status != XST_SUCCESS)
    {
        xil_printf("Read channel set buffer address failed %d\r\n", status);
        return XST_FAILURE;
    }

    XAxiVdma_IntrEnable(&myVDMA, XAXIVDMA_IXR_COMPLETION_MASK, XAXIVDMA_READ);

    SetupIntrSystem(&myVDMA, XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR);
    Xil_DCacheFlush();

    status = XAxiVdma_DmaStart(&myVDMA, XAXIVDMA_READ);
    if (status != XST_SUCCESS)
    {
        if (status == XST_VDMA_MISMATCH_ERROR)
            xil_printf("DMA Mismatch Error\r\n");
        return XST_FAILURE;
    }

    shell_run(); //  never return
    while (1)
    {
    }
}

/*****************************************************************************/
/* Call back function for read channel
 ******************************************************************************/
static void ReadCallBack(void *CallbackRef, u32 Mask)
{
    // Box size
    const int boxWidth = 80;
    const int boxHeight = 60;

    // Static variables to hold box position and velocity
    static int boxX = 0;
    static int boxY = 0;
    static int velX = 5; // pixels per frame
    static int velY = 5;
    static int prevBoxX = 0;
    static int prevBoxY = 0;

    // Move the box position
    boxX += velX;
    boxY += velY;

    // Bounce off edges
    if (boxX < 0)
    {
        boxX = 0;
        velX = -velX;
    }
    else if (boxX + boxWidth >= HSize)
    {
        boxX = HSize - boxWidth - 1;
        velX = -velX;
    }

    if (boxY < 0)
    {
        boxY = 0;
        velY = -velY;
    }
    else if (boxY + boxHeight >= VSize)
    {
        boxY = VSize - boxHeight - 1;
        velY = -velY;
    }

    // Clear entire screen black first

    // Draw gradient box at current position
    for (int y = prevBoxY; y < prevBoxY + boxHeight; y++)
    {
        for (int x = prevBoxX; x < prevBoxX + boxWidth; x++)
        {
            int idx = (y * HSize + x) * 3;
            Buffer[idx] = 0x00;
            Buffer[idx + 1] = 0x00;
            Buffer[idx + 2] = 0x00;
        }
    }

    // Draw new box
    for (int y = boxY; y < boxY + boxHeight; y++)
    {
        for (int x = boxX; x < boxX + boxWidth; x++)
        {
            int idx = (y * HSize + x) * 3;
            Buffer[idx] = (u8)(((x - boxX) * 32) / (boxWidth - 1));
            Buffer[idx + 1] = (u8)(((y - boxY) * 64) / (boxHeight - 1));
            Buffer[idx + 2] = 0x00;
        }
    }

    prevBoxX = boxX;
    prevBoxY = boxY;

    Xil_DCacheFlushRange((INTPTR)Buffer, FrameSize);
}

static void ReadErrorCallBack(void *CallbackRef, u32 Mask)
{
    /* User can add his code in this call back function */
    xil_printf("Read Call back Error function is called\r\n");
}

static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 ReadIntrId)
{
    int Status;
    XScuGic *IntcInstancePtr = &Intc;

    /* Initialize the interrupt controller and connect the ISRs */
    XScuGic_Config *IntcConfig;
    IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
    Status = XScuGic_CfgInitialize(IntcInstancePtr, IntcConfig, IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS)
    {
        xil_printf("Interrupt controller initialization failed..");
        return -1;
    }

    Status = XScuGic_Connect(IntcInstancePtr, ReadIntrId, (Xil_InterruptHandler)XAxiVdma_ReadIntrHandler, (void *)AxiVdmaPtr);
    if (Status != XST_SUCCESS)
    {
        xil_printf("Failed read channel connect intc %d\r\n", Status);
        return XST_FAILURE;
    }

    XScuGic_Enable(IntcInstancePtr, ReadIntrId);

    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, (void *)IntcInstancePtr);
    Xil_ExceptionEnable();

    /* Register call-back functions
     */
    XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_GENERAL, ReadCallBack, (void *)AxiVdmaPtr, XAXIVDMA_READ);

    XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_ERROR, ReadErrorCallBack, (void *)AxiVdmaPtr, XAXIVDMA_READ);

    return XST_SUCCESS;
}
