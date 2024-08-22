/******************************************************************************
* Copyright (C) 2023 Advanced Micro Devices, Inc. All Rights Reserved.
* SPDX-License-Identifier: MIT
******************************************************************************/
/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include <xparameters.h>
#include <xscugic.h>
#include <xil_exception.h>
#include "xhls_pp.h"
//#include "xgpio.h"

#define INTC_DEVICE_ID XPAR_PS7_SCUGIC_0_DEVICE_ID
#define XHLS_DEVICE_ID XPAR_HLS_PP_0_DEVICE_ID
#define XHLS_INTERRUPT_ID XPAR_FABRIC_HLS_PP_0_INTERRUPT_INTR

#define VECTOR_SIZE 5
#define TRIAL 5
#define BRAMS 2
#define BUFFER_SIZE 3

XScuGic intc;
XHls_pp ip;

volatile int Delay;
int Status;
volatile int ip_status;


void getVector(float vec1[VECTOR_SIZE], float vec2[VECTOR_SIZE]);
int TxDataSend(XHls_pp *InstancePtr, float data1[VECTOR_SIZE], float data2[VECTOR_SIZE]);
void IPHandler(void *InstPtr);
int IntcInitFunction(u16 DeviceId);

void (*XHLSWritexFunc[])() = {XHls_pp_Write_x_0_Words,
                            XHls_pp_Write_x_1_Words
};

void (*XHLSWriteyFunc[])() = {XHls_pp_Write_y_0_Words,
                            XHls_pp_Write_y_1_Words
};

enum IP_ready
{
	IP_Ready,
	IP_Busy
};

void getVector(float vec1[VECTOR_SIZE], float vec2[VECTOR_SIZE]){ //esta PARTE TIENE DIRECTA RELACION CON EL PYTHON****
	for (int i = 0; i < VECTOR_SIZE; i++){
		scanf("%f", &vec1[i]);
	}
    for (int i = 0; i < VECTOR_SIZE; i++){
		scanf("%f", &vec2[i]);
	}
}

u32 TxData1[BUFFER_SIZE];
u32 TxData2[BUFFER_SIZE];

int TxDataSend(XHls_pp *InstancePtr, float data1[VECTOR_SIZE], float data2[VECTOR_SIZE]){
	int status = XST_SUCCESS;
	for (int br = 0; br < BRAMS; br++){
		for (int i = 0; i < BUFFER_SIZE; i++){
			TxData1[i] = (i*BRAMS +br)< VECTOR_SIZE ? *((u32*) &data1[(i*BRAMS) + br]) : 0; //esta god la condicion, me resuelve la vida
            TxData2[i] = (i*BRAMS +br)< VECTOR_SIZE ? *((u32*) &data2[(i*BRAMS) + br]) : 0;
		}
		XHLSWritexFunc[br](InstancePtr, 0, TxData1, BUFFER_SIZE);
        XHLSWriteyFunc[br](InstancePtr, 0, TxData2, BUFFER_SIZE);
	}
	return status;
}

u32 RxData;
void IPHandler(void *InstPtr)
{
	float result;
	XHls_pp_InterruptDisable(&ip,1);/////////////////////////////INT

	//XGpio_DiscreteWrite(&extp, 1, 0x00);
	RxData = XHls_pp_Get_pp(&ip);
	result = *((float*) &(RxData));
	printf("Resultado: %.3f\n", result);

	ip_status = IP_Ready;

	XHls_pp_InterruptClear(&ip,1);////////////////////////////INT
	XHls_pp_InterruptEnable(&ip,1);////////////////////////////INT
}

int IntcInitFunction(u16 DeviceId){
	//GIC CONFIG
    XScuGic_Config *IntcConfig;
    //Xil_ExceptionInit();//Por lo que vi esta funcion ya no hace nada

    //INITIALIZE THE GIC
    IntcConfig = XScuGic_LookupConfig(DeviceId);
    XScuGic_CfgInitialize(&intc,IntcConfig, IntcConfig->CpuBaseAddress);

    //connect to the hardware //la verdad no se para que es esto pero si no lo uso no funciona.
    Xil_ExceptionRegisterHandler(XIL_EXCEPTION_ID_INT,(Xil_ExceptionHandler)XScuGic_InterruptHandler,&intc);
    Xil_ExceptionEnable();

    //Connect IP interrupt to handler
    XScuGic_Connect(&intc, XHLS_INTERRUPT_ID,(Xil_ExceptionHandler)IPHandler,(void *)&ip);

    XHls_pp_InterruptEnable(&ip, 1);
	XHls_pp_InterruptGlobalEnable(&ip);

    // Enable ip interrupts in the controller
	XScuGic_Enable(&intc, XHLS_INTERRUPT_ID);

	return XST_SUCCESS;
}

int main()
{
	Status = XST_SUCCESS;
    init_platform();

    float txbuffer1[VECTOR_SIZE];
    float txbuffer2[VECTOR_SIZE];

	/* HLS IP init */
	Status+=XHls_pp_Initialize(&ip, XHLS_DEVICE_ID);
		if (Status != XST_SUCCESS) {
    	xil_printf("IP Initialization Failed\r\n");
		return XST_FAILURE;
	}

	/* interrupt controller init*/
	Status+=IntcInitFunction(INTC_DEVICE_ID);
	if (Status != XST_SUCCESS) {
    	xil_printf("INTC Initialization Failed\r\n");
		return XST_FAILURE;
	}

    ip_status = IP_Ready;

    for (int trial = 0; trial < TRIAL; trial++ ){
		while (ip_status == IP_Busy) {};
		getVector(txbuffer1, txbuffer2);
		TxDataSend(&ip, txbuffer1, txbuffer2);
		ip_status = IP_Busy;
        XHls_pp_Start(&ip);
		//XGpio_DiscreteWrite(&extp, 1, 0xff);//UTIL, falta agregar el gpio extra
	}

    cleanup_platform();
	while(1);
    return 0;
}
