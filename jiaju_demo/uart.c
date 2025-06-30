#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include "iot_gpio_ex.h"
#include "ohos_init.h"
#include "cmsis_os2.h"
#include "iot_gpio.h"
#include "iot_uart.h"
#include "hi_uart.h"
#include "iot_watchdog.h"
#include "iot_errno.h"
#include <hi_pwm.h>
#include <hi_gpio.h>
#include <hi_io.h>
#include <hi_adc.h>
#include <hi_watchdog.h>
#include "hi_time.h"
#include "hi_adc.h"
#include "hi_i2c.h"



#define GPIO_reset      (0)
#define GPIO_set        (1)

#define U_SLEEP_TIME   100000

unsigned char uartWriteBuff[] = "uart_led ok";
unsigned char uartReadBuff[2048] = {0};

#define  COUNT   10
#define  FREQ_TIME    20000


void SetAngle(unsigned int duty)
{
    unsigned int time = FREQ_TIME;

    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_2, IOT_GPIO_VALUE1);
    hi_udelay(duty);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_2, IOT_GPIO_VALUE0);
    hi_udelay(time - duty);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE1);
    hi_udelay(duty);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, IOT_GPIO_VALUE0);
    hi_udelay(time - duty);
}

void RegressMiddle(void)
{
    unsigned int angle = 1500;
    for (int i = 0; i < COUNT; i++) {
        SetAngle(angle);
    }
}
void EngineTurnRight(void)
{
    unsigned int angle = 2500;
    for (int i = 0; i < COUNT; i++) {
        SetAngle(angle);
    }
}

void EngineTurnLeft(void)
{
    unsigned int angle = 2500;
    for (int i = 0; i < COUNT; i++) {
        SetAngle(angle);
    }
}
void S92RInit(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_2);
    IoSetFunc(IOT_IO_NAME_GPIO_2, IOT_IO_FUNC_GPIO_2_GPIO);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_2, IOT_GPIO_DIR_OUT);
    IoTGpioInit(IOT_IO_NAME_GPIO_10);
    IoSetFunc(IOT_IO_NAME_GPIO_10, IOT_IO_FUNC_GPIO_10_GPIO);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_10, IOT_GPIO_DIR_OUT);
}

void Uart1GpioInit(void)
{
    IoTGpioInit(IOT_IO_NAME_GPIO_0);
    IoTGpioSetDir(IOT_IO_NAME_GPIO_0,IOT_GPIO_DIR_OUT);
    IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_0, GPIO_reset);
    IoTGpioInit(IOT_IO_NAME_GPIO_11);
    IoSetFunc(IOT_IO_NAME_GPIO_11, IOT_IO_FUNC_GPIO_11_UART2_TXD);
    IoTGpioInit(IOT_IO_NAME_GPIO_12);
    IoSetFunc(IOT_IO_NAME_GPIO_12, IOT_IO_FUNC_GPIO_12_UART2_RXD);
}

void Uart1Config(void)
{
    uint32_t sat;
    IotUartAttribute uart_attr = {
        .baudRate = 115200,
        .dataBits = 8,
        .stopBits = 1,
        .parity = 0,
    };
    sat = IoTUartInit(HI_UART_IDX_2, &uart_attr);
    if (sat != 0) {
    }
     return sat;
}
static void UartTask(void *argument)
{
    unsigned int time = 200;
    static int count = 1000;
    S92RInit();
    Uart1GpioInit();
    Uart1Config();
    int ret =IoTUartWrite(HI_UART_IDX_2, (unsigned char*)uartWriteBuff, sizeof(uartWriteBuff));
    printf("Uart Write data: ret = %d\r\n", ret);
    while (count--) {
        unsigned int len = IoTUartRead(2, uartReadBuff, sizeof(uartReadBuff));
        if (len > 0) {
            uartReadBuff[len] = '\0';
            printf("Uart read data: %s\r\n", uartReadBuff);
            if (uartReadBuff[0] == '0') {
                IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_2, 1);
                                IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, 1);
            }else if (uartReadBuff[0] == '1') {
                IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_2, 0);
                                IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_10, 0);
            }
           if (uartReadBuff[0] == '2') {
                IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_5, 1);
                EngineTurnLeft();
            }else if (uartReadBuff[0] == '3') {
                IoTGpioSetOutputVal(IOT_IO_NAME_GPIO_5, 0);
                RegressMiddle();
            }
            }
        }
        usleep(U_SLEEP_TIME);
        
    }

void UartExampleEntry(void)
{
    osThreadAttr_t attr;
    IoTWatchDogDisable();

    attr.name = "UartTask";
    attr.attr_bits = 0U;
    attr.cb_mem = NULL;
    attr.cb_size = 0U;
    attr.stack_mem = NULL;
    attr.stack_size = 5 * 1024; 
    attr.priority = osPriorityNormal;

    if (osThreadNew((osThreadFunc_t)UartTask, NULL, &attr) == NULL) {
        printf("[UartTask] Failed to create UartTask!\n");
    }
    }

APP_FEATURE_INIT(UartExampleEntry);