#include "stm32f10x.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"
#include "delay.h"



#define LED_PIN1	GPIO_Pin_7	//PA.7
#define LED_PIN2	GPIO_Pin_8	//PA.8
#define LED_PIN3	GPIO_Pin_9	//PA.9

// 987 -> 7-0

#define BUTTON_PIN1 GPIO_Pin_3	//PC.3.temp
#define BUTTON_PIN2 GPIO_Pin_2	//PC.2.humid
#define BUTTON_PIN3 GPIO_Pin_1	//PC.1.light

void TIM2_INT_Init(void);


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

void LED_Write(int state) {

	  switch(state){
			case 0:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
				break;
			case 1:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
				break;
			case 2:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
				break;
			case 3:
				GPIO_ResetBits(GPIOA, LED_PIN1);		//0
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
				break;
			case 4:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
			  break;
			case 5:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN2);		//0
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
				break;
			case 6:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_ResetBits(GPIOA, LED_PIN3);		//0
				break;
			default:
				GPIO_SetBits(GPIOA, LED_PIN1);			//1
			  GPIO_SetBits(GPIOA, LED_PIN2);			//1
			  GPIO_SetBits(GPIOA, LED_PIN3);			//1
			  break;
		}
}

int main(void)
{
	int last = 0;
	
	DelayInit();
	
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
	
	
	
  SystemCoreClockUpdate();
	
	
	Button_Init();
	LED_Init();
	while(1) {
		last = Button_Read();
		LED_Write(last);
		DelayMs(1000);//interval about 0.6s
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