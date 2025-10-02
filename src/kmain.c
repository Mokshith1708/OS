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
#include "drivers/driver.h"
#include "shell.h"
#include "ff.h"
#include "axi_ps2.h"
#include "keyboard.h"

#define HSize 1920
#define VSize 1080
#define FrameSize HSize *VSize * 3
#define GPIO_RESET_PIN 54
#define imgHSize 512
#define imgVSize 512

static XScuGic Intc;

static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 VdmaIntrId, axi_ps2 *Ps2Ptr, u16 Ps2IntrId);
void PS2_InterruptHandler(void *CallBackRef);

volatile unsigned char Buffer[FrameSize];
__attribute__((section(".frame_buffer"))) int drawImage(u32 displayHSize, u32 displayVSize, u32 imageHSize, u32 imageVSize, u32 hOffset, u32 vOffset, char *imagePointer);

XGpioPs Gpio;
FATFS fatfs;
FRESULT res;
static axi_ps2 Ps2Inst;

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
    console_init(Buffer); // Initialize framebuffer for console

    // Initialize SD card

    XSdPs SdInstance;
    XSdPs_Config *sd_config;
    sd_config = XSdPs_LookupConfig(XPAR_XSDPS_0_DEVICE_ID);
    if (!sd_config)
    {
        xil_printf("SD lookup config failed\n");
//        return XST_FAILURE;
    }
    status = XSdPs_CfgInitialize(&SdInstance, sd_config, sd_config->BaseAddress);
    if (status != XST_SUCCESS)
    {
        xil_printf("SD initialization failed\n");
//        return status;
    }

    // Optionally power on SD card and setup clocks if needed

    // Mount the file system on the SD card
    res = f_mount(&fatfs, "0:", 1);
    if (res != FR_OK)
    {
        xil_printf("Failed to mount filesystem. FatFs error = %d\n", res);
//        return XST_FAILURE;
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

    //Initialize PS2
    axi_ps2_Config *ConfigPtr;

    ConfigPtr = axi_ps2_LookupConfig(XPAR_AXI_PS2_0_DEVICE_ID);
    if (ConfigPtr == NULL) {
        return XST_FAILURE;
    }
    axi_ps2_CfgInitialize(&Ps2Inst, ConfigPtr, ConfigPtr->BaseAddress);

    SetupIntrSystem(&myVDMA, XPAR_FABRIC_AXI_VDMA_0_MM2S_INTROUT_INTR,&Ps2Inst,XPAR_FABRIC_AXI_PS2_0_PS2_INTERRUPT_INTR);

    Xil_DCacheFlush();

    status = XAxiVdma_DmaStart(&myVDMA, XAXIVDMA_READ);
    if (status != XST_SUCCESS)
    {
        if (status == XST_VDMA_MISMATCH_ERROR)
            xil_printf("DMA Mismatch Error\r\n");
        return XST_FAILURE;
    }

    axi_ps2_SetHandler(&Ps2Inst, PS2_InterruptHandler, &Ps2Inst);

    axi_ps2_IntrEnable(&Ps2Inst, axi_ps2_IPIXR_RX_ALL);  // Enable receive interrupts

    axi_ps2_IntrGlobalEnable(&Ps2Inst); // Enable global interrupt in the IP


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
//    xil_printf("Read Callback\r\n");
	Xil_DCacheFlushRange((INTPTR)Buffer, FrameSize);
}

static void ReadErrorCallBack(void *CallbackRef, u32 Mask)
{
    /* User can add his code in this call back function */
    xil_printf("Read Call back Error function is called\r\n");
}

static int SetupIntrSystem(XAxiVdma *AxiVdmaPtr, u16 VdmaIntrId, axi_ps2 *Ps2Ptr, u16 Ps2IntrId)
{
    int Status;

    /* Initialize the interrupt controller and connect the ISRs */
    XScuGic_Config *IntcConfig;
    IntcConfig = XScuGic_LookupConfig(XPAR_PS7_SCUGIC_0_DEVICE_ID);
    Status = XScuGic_CfgInitialize(&Intc, IntcConfig, IntcConfig->CpuBaseAddress);
    if (Status != XST_SUCCESS)
    {
        xil_printf("Interrupt controller initialization failed..");
        return -1;
    }

    // Connect VDMA interrupt
    Status = XScuGic_Connect(&Intc, VdmaIntrId, (Xil_InterruptHandler)XAxiVdma_ReadIntrHandler, (void *)AxiVdmaPtr);
    if (Status != XST_SUCCESS)
    {
        xil_printf("Failed read channel connect intc %d\r\n", Status);
        return XST_FAILURE;
    }
    XScuGic_Enable(&Intc, VdmaIntrId);

    // Connect PS2 interrupt
    Status = XScuGic_Connect(&Intc, Ps2IntrId, (Xil_InterruptHandler)axi_ps2_IntrHandler, (void *)Ps2Ptr);
    if (Status != XST_SUCCESS)
    {
        xil_printf("PS2 interrupt connect failed %d\r\n", Status);
        return XST_FAILURE;
    }
    XScuGic_Enable(&Intc, Ps2IntrId);

    /* Register call-back functions for VDMA */
    XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_GENERAL, ReadCallBack, (void *)AxiVdmaPtr, XAXIVDMA_READ);
    XAxiVdma_SetCallBack(AxiVdmaPtr, XAXIVDMA_HANDLER_ERROR, ReadErrorCallBack, (void *)AxiVdmaPtr, XAXIVDMA_READ);

    /* Initialize exception handling */
    Xil_ExceptionInit();
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT, (Xil_ExceptionHandler)XScuGic_InterruptHandler, &Intc);
    Xil_ExceptionEnable();

    return XST_SUCCESS;
}


void PS2_InterruptHandler(void *CallBackRef)
{
    axi_ps2 *InstancePtr = (axi_ps2 *)CallBackRef;
    u32 Data;

    axi_ps2_Recv(InstancePtr, &Data, 1);

    PS2_ScanCodeHandler(Data);
}

