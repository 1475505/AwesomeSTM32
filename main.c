#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "stm32f10x_usart.h"
#include "delay.h"
#include "xprintf.h"
#include <stdio.h>

#define LED_PIN1	GPIO_Pin_7	//PA.7
#define LED_PIN2	GPIO_Pin_8	//PA.8
#define LED_PIN3	GPIO_Pin_9	//PA.9

// 987 -> 7-0

#define BUTTON_PIN1 GPIO_Pin_3	//PC.3.temp
#define BUTTON_PIN2 GPIO_Pin_2	//PC.2.humid
#define BUTTON_PIN3 GPIO_Pin_1	//PC.1.light
#define CTRLKEY GPIO_Pin_0	


// 定义调试串口
#define DEBUG_UART          USART1

// 是否输出调试信息
#define DEBUG_PRINTF

#ifdef DEBUG_PRINTF
#define debug(FORMAT, ...)  printf(FORMAT, ##__VA_ARGS__)
#else
#define debug(FORMAT, ...)
#endif

int fputc(int ch, FILE *f)//重定向，让printf输出到串口
{
    USART_SendData(DEBUG_UART, (uint8_t) ch);

    while (USART_GetFlagStatus(DEBUG_UART, USART_FLAG_TXE) == RESET);
    return ch;
}

void USART1_Init(void)
{//串口初始化
  USART_InitTypeDef USART_InitStructure;
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA |RCC_APB2Periph_USART1|RCC_APB2Periph_AFIO , ENABLE);

	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
  GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
  
  // Configure the USART1_Rx as input floating
  GPIO_InitStructure.GPIO_Mode =GPIO_Mode_IN_FLOATING ;
  GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
  GPIO_Init(GPIOA, &GPIO_InitStructure);
	
  USART_InitStructure.USART_BaudRate = 115200;
  USART_InitStructure.USART_WordLength = USART_WordLength_8b;
  USART_InitStructure.USART_StopBits = USART_StopBits_1;
  USART_InitStructure.USART_Parity = USART_Parity_No ;
  USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
  USART_InitStructure.USART_Mode = USART_Mode_Tx;
  
  /* Configure the USARTx */ 
  USART_Init(USART1, &USART_InitStructure);
  /* Enable the USARTx */
  USART_Cmd(USART1, ENABLE);
}

void TIM2_INT_Init(void);

volatile uint32_t msTicks; // counts 1ms timeTicks

void Delay (uint32_t dlyTicks) {// no interrupt
  uint32_t curTicks;
  curTicks = msTicks;
  while ((msTicks - curTicks) < dlyTicks) { __NOP(); }
}

void TIM2_IRQHandler()
{
	// Checks whether the TIM2 interrupt has occurred or not
	if (TIM_GetITStatus(TIM2, TIM_IT_Update))
	{
		// Toggle LED on PB12
		GPIOB->ODR ^= GPIO_Pin_12;

		// Clears the TIM2 interrupt pending bit
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);
	}
}

GPIO_InitTypeDef GPIO_InitStruct;

void Button_Init(void) {
	/* Enable GPIOC clock            */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);                
	
	/* Configure Button (PC.3) pins as input */
	GPIO_InitStruct.GPIO_Pin = BUTTON_PIN1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = BUTTON_PIN2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = BUTTON_PIN3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
  GPIO_Init(GPIOC, &GPIO_InitStruct);
}

void LED_Init(void) {
	/* Enable GPIOA clock            */
  RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   
	
	/* Configure LED (PA.7) pins as output */
	GPIO_InitStruct.GPIO_Pin = LED_PIN1;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
	
  GPIO_InitStruct.GPIO_Pin = LED_PIN2;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
	
  GPIO_InitStruct.GPIO_Pin = LED_PIN3;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
  GPIO_Init(GPIOA, &GPIO_InitStruct);
}


int Button_Read(void) {
	return GPIO_ReadInputDataBit(GPIOC, BUTTON_PIN1) * 4 + GPIO_ReadInputDataBit(GPIOC, BUTTON_PIN2) * 2 + GPIO_ReadInputDataBit(GPIOC, BUTTON_PIN3);
}

int ReadKeyDown(void)
{
  
  if(GPIO_ReadInputDataBit(GPIOC, CTRLKEY))
  {
    return 1; 
  }	
  else 
  {
    return -1;
  }							      
}

void LED_Write(int state) {
	  switch(state){
			case 0:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
				xprintf(0);
			printf("0\n");
				break;
			case 1:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
				//xprintf(1);
			printf("1\n");
				break;
			case 2:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
			  //xprintf(2);
			printf("2\n");
				break;
			case 3:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
			  //xprintf(3);
			printf("3\n");
				break;
			case 4:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
			  //xprintf(4);
			printf("4\n");
			  break;
			case 5:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
			  //xprintf(5);
			printf("5\n");
				break;
			case 6:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
			  //xprintf(6);
			printf("6\n");
				break;
			default:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
			  //xprintf(7);
			printf("7\n");
			  break;
		}
}
int InitLED(int state);
int getnum(int a, int b);
// 7 segment font (0-9)
// D7=DP, D6=A, D5=B, D4=C, D3=D, D2=E, D1=F, D0=G
const uint8_t font[10] =
{
	0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x7B
};
uint8_t buffer[4];

int main(void)
{
	int last = 0;
	SystemInit ();
	DelayInit();
	USART1_Init();
	// Initialize timer interrupt
	TIM2_INT_Init();
	
	// Initialize PB12 as push-pull output for LED
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	// Initialize PC13 as push-pull output for LED
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	// above is for interrupt control. May delete.
	
	
	Button_Init();
	LED_Init();
	//TODO: while 不按某个按键不执行后面的流程
	while (ReadKeyDown() != 1);//这种形式做到了实现，但是有bug，虽然可以运行
	last = InitLED(Button_Read());
	LED_Write(last);
	//TODO: while 不按某个按键不执行后面的流程
	while(1) {
		//TODO: show 七段显示译码器结果输出在串口
		last = getnum(Button_Read(), last);
		LED_Write(last);
		DelayMs(500);//interval about 0.6s
	}
	
}

void TIM2_INT_Init()
{
	// Init struct
	TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
	NVIC_InitTypeDef NVIC_InitStruct;
	
	// Enable clock for TIM2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	
	// TIM2 initialization for overflow every 500ms
	// Update Event (Hz) = timer_clock / ((TIM_Prescaler + 1) * (TIM_Period + 1))
	// Update Event (Hz) = 72MHz / ((3599 + 1) * (9999 + 1)) = 2Hz (0.5s)
	TIM_TimeBaseInitStruct.TIM_Prescaler = 3599;
	TIM_TimeBaseInitStruct.TIM_Period = 9999;
	TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
	
	// Enable TIM2 interrupt
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	// Start TIM2
	TIM_Cmd(TIM2, ENABLE);
	
	// Nested vectored interrupt settings
	NVIC_InitStruct.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0x00;
	NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStruct);
}

int InitLED(int state){
	  switch(state){
			case 0:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
				return 3;
			case 1:
			case 2:
			case 4:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
			  return 4;
			case 5:
			case 3:
			case 6:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
				return 5;
			case 7:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
				return 6;
			default:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
			  return 7;
		}
}

int getnum(int a, int b){
	int ret = b;
	switch(a){
	    case 0:
				ret-=2;
				break;
			case 1:
			case 2:
			case 4:
				ret--;
			  return 4;
			case 5:
			case 3:
			case 6:
				ret++;
				break;
			case 7:
				ret+=2;
			  //printf("So exciting!");
				break;
			default:
				return b;
	}
	if (ret > 7) return 7;
	if (ret < 0) return 0;
  return ret;	
}