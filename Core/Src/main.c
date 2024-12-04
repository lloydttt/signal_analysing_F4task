/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2024 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dac.h"
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"
#include "fsmc.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "math.h"
#include "stdio.h"

#include "arm_math.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define SINE_SAMPLE_COUNT 1024 // 正弦波采样点�???????????????????????????????
#define SINE_AMPLITUDE     2048 // 正弦波幅�??????????????????????????????? (12位分辨率: 0-4095)
#define SINE_OFFSET        2047 // 信号偏移�??????????????????????????????? (为了输出正�??)

#define KEY0 HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_4)//读取PE4电平
#define KEY1 HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_3)//读取PE3电平
#define KEY2 HAL_GPIO_ReadPin(GPIOE,GPIO_PIN_2)//读取PE2电平
#define WK_UP HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_0)//读取PA0电平
#define KEY0_Press 1//KEY0被按下标志位
#define KEY1_Press 2//KEY1被按下标志位
#define KEY2_Press 3//KEY2被按下标志位
#define WK_UP_Press 4//WK_UP被按下标志位
uint16_t signal_type_flag = 0;

#define FFT_X_START 20
#define FFT_Y_START 350
#define FFT_WIDTH 300
#define FFT_HEIGHT 120
#define FFT_LENGTH 1024
//#define Fs 168674

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

uint16_t sin_wave_buffer[SINE_SAMPLE_COUNT];
uint16_t recv_data[SINE_SAMPLE_COUNT];
__IO uint8_t AdcConvEnd = 0;
int ad_flag = 0;

arm_cfft_radix4_instance_f32 scfft;
arm_rfft_fast_instance_f32 fft_instance;
float FFT_inArray[FFT_LENGTH];
float FFT_outArray[FFT_LENGTH];
uint8_t key = 0;
uint32_t maxIndex;
float32_t max_value = 0.0f;
float32_t base_frequency = 1000;
float Fs = 168674;
float omega = 50;
int flag_dma = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void Generate_SineWave(void) {
    if(signal_type_flag == 0){
        for (int i = 0; i < SINE_SAMPLE_COUNT; i++) {
            // 生成正弦
            sin_wave_buffer[i] = (uint16_t)(SINE_AMPLITUDE * sin(omega * PI * i / SINE_SAMPLE_COUNT) + SINE_OFFSET);
        }
    }else if(signal_type_flag == 1){
        const int wave_periods = 5; // 方波周期数（频率的�?�数�???
        for (int i = 0; i < SINE_SAMPLE_COUNT; i++) {
            // 计算当前点对应的周期位置
            if (((i * wave_periods) % SINE_SAMPLE_COUNT) < (SINE_SAMPLE_COUNT / (2 * wave_periods))) {
                // 高电�???
                sin_wave_buffer[i] = SINE_OFFSET + SINE_AMPLITUDE;
            } else {
                // 低电�???
                sin_wave_buffer[i] = SINE_OFFSET - SINE_AMPLITUDE;
            }
        }
    }
}
uint8_t KEY_Scan(uint8_t mode)
{
    static uint8_t key_up=1;//按键松开标志
    if(mode)key_up=1; //支持连按
    if(key_up&&(KEY0==0||KEY1==0||KEY2==0||WK_UP==1))
    {
        HAL_Delay(10);
        key_up=0;
        if(KEY0==0)return KEY0_Press;
        else if(KEY1==0)return KEY1_Press;
        else if(KEY2==0)return KEY2_Press;
        else if(WK_UP==1)return WK_UP_Press;
    }else if(KEY0==1&&KEY1==1&&KEY2==1&&WK_UP==0)key_up=1;
    return 0;//无按键按�??
}
/*----------------------------------------------------------------------------------------*/
void Start_DAC_With_DMA(void) {

    Generate_SineWave();

    // 启动 DAC  DMA
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t*)sin_wave_buffer, SINE_SAMPLE_COUNT, DAC_ALIGN_12B_R);

    // 启动定时器，作为 DAC 的触发源
//    HAL_TIM_Base_Start(&htim4);
}
/*----------------------------------------------------------------------------------------*/
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef* hadc)
{
//    printf("%.3f \n",recv_data[0]*3.3/4096);
}

/*----------------------------------------------------------------------------------------*/
/*----------------------------------------------------------------------------------------*/
void AdcDataDrawing()
{
    uint16_t x1, y1, x2, y2;
    x1 = 0;
    y1 = 0;
    // 计算缩放比例
    float x_scale = (float)(320 + 10) / SINE_SAMPLE_COUNT;
    float y_scale = (float)(480 / 5) / 4095;


    for (uint16_t i = 1; i < SINE_SAMPLE_COUNT; i++)
    {
        x2 = (uint16_t)(i * x_scale) ;
        y2 = (uint16_t)(recv_data[i] * y_scale)+60;
////
////        // 绘制线段
        LCD_DrawLine(x1, y1, x2, y2);

        x1 = x2;
        y1 = y2;
//        LCD_DrawPoint(x2, y2);
    }
}
/*----------------------------------------------------------------------------------------*/
/* Draw FFT spectrum */
void DrawFFT(void) {
    float x, y;
    float x_scale = (float) FFT_WIDTH / (FFT_LENGTH / 2); // X轴缩放比例
    float y_scale;


    // 寻找幅值最大值，用于Y轴缩放
    for (int i = 0; i < FFT_LENGTH / 2; i++) {
        if (FFT_outArray[i] > max_value) {
            max_value = FFT_outArray[i];
        }
    }

    // 防止除以零
    if (max_value == 0) {
        max_value = 1.0f;
    }

    y_scale = (float) FFT_HEIGHT / max_value;

    // 清除绘图区
    // LCD_ClearArea(FFT_X_START, FFT_Y_START, FFT_WIDTH, FFT_HEIGHT);

    // 绘制频谱
    for (int i = 0; i < FFT_LENGTH / 2; i++) {
        x = FFT_X_START + i * x_scale;
        y = FFT_Y_START - FFT_outArray[i] * y_scale;

        // 将浮点坐标转换为整数坐标进行绘制
        LCD_DrawLine((uint16_t) x, FFT_Y_START, (uint16_t) x, (uint16_t) y);
    }
//    LCD_ShowString(10,380,20,10,16,"该信号");

    LCD_ShowString(15,380,300,16,16,"frequency of the fundamental wave is");

    LCD_ShowNum(135,410,base_frequency,4,16);

}
/*----------------------------------------------------------------------------------------*/
void axis_drawing(){
    LCD_ShowString(15,10,300,16,16,"*************  WELCOME  *************");
    LCD_DrawLine(0,195,320,195);
    LCD_ShowNum(20,200,0,4,12);
//    LCD_ShowNum(64,200,20,4,12);
//    LCD_ShowNum(108,200,30,4,12);
    LCD_ShowNum(152,200,500,4,12);
//    LCD_ShowNum(196,200,50,4,12);
//    LCD_ShowNum(240,200,60,4,12);
    LCD_ShowNum(280,200,1000,4,12);
    LCD_DrawLine(160,195,160,40);
    LCD_DrawLine(160,40,155,50);
    LCD_DrawLine(160,40,165,50);
    LCD_ShowString(45,240,300,16,16,"-------  Spectrogram  -------");
//    LCD_ShowNum(170,00,90,2,12);
//    LCD_ShowNum(170,20,80,2,12);
//    LCD_ShowNum(170,40,70,2,12);
//    LCD_ShowNum(170,60,60,2,12);
    LCD_ShowFloat(170,60,12,3.31,1,2);
//    LCD_ShowNum(170,80,50,2,12);
//    LCD_ShowNum(170,100,40,2,12);
    LCD_ShowFloat(170,100,12,1.66,1,2);
//    LCD_ShowNum(170,120,30,2,12);
//    LCD_ShowNum(170,140,20,2,12);
    LCD_ShowNum(170,140,0,2,12);
//    LCD_ShowNum(170,180,0,2,12);
    LCD_ShowString(20,450,300,16,16,"************  QEA LYT  ************");

}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_ADC1_Init();
  MX_DAC_Init();
  MX_USART1_UART_Init();
  MX_TIM6_Init();
  MX_FSMC_Init();
  /* USER CODE BEGIN 2 */
    HAL_ADC_Start_IT(&hadc1);
    HAL_ADC_Start_DMA(&hadc1,(uint32_t*)recv_data,SINE_SAMPLE_COUNT);
    HAL_TIM_Base_Start_IT(&htim6);
    Start_DAC_With_DMA();

//    HAL_TIM_Base_Start(&htim3);
    HAL_Delay(50);
    LCD_Init();
    LCD_Clear(WHITE);
    POINT_COLOR=BLACK;
    arm_cfft_radix4_init_f32(&scfft, FFT_LENGTH, 0, 1);
    arm_rfft_fast_init_f32(&fft_instance, FFT_LENGTH);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
      LCD_Clear(WHITE);

//      if(flag_dma == 1){
          AdcDataDrawing();

          for (int i = 0; i < FFT_LENGTH; i++)
          {

              FFT_inArray[i] = (float)recv_data[i];
          }

          /* 执行 FFT 计算 */
          arm_rfft_fast_f32(&fft_instance, FFT_inArray, FFT_outArray, 0);
//      // Normalize and limit FFT values for display

///* 计算幅值谱 */
          FFT_outArray[0] = fabsf(FFT_outArray[0]); // 直流分量，实数
          for (int i = 1; i < FFT_LENGTH / 2; i++)
          {
              float real = FFT_outArray[2 * i];                   // 实部
              float imag = FFT_outArray[2 * i + 1];               // 虚部
              FFT_outArray[i] = sqrtf(real * real + imag * imag); // 计算幅值
          }

          arm_max_f32(FFT_outArray+1, SINE_SAMPLE_COUNT-1, &max_value, &maxIndex);
//      printf("Dominant index: %.2lu Hz\n", maxIndex);
          // 计算对应的频率
          base_frequency = (float32_t)maxIndex * Fs / FFT_LENGTH ;

          // 输出结果
//      printf("Dominant base_frequency: %.2f Hz\n", base_frequency);
          DrawFFT();


          axis_drawing();




          key = KEY_Scan(0);
          if(key == KEY0_Press){
              signal_type_flag = 1;
//          base_frequency -= 100;
          } else if (key == KEY1_Press){
              signal_type_flag = 0;
          }else if(key == KEY2_Press){
//              sin_wave_signal = 1;
                omega -= 5;
          }
//          flag_dma = 0;

          HAL_ADC_Start_DMA(&hadc1,(uint32_t*)recv_data,SINE_SAMPLE_COUNT);
//      }

//      printf("%d\n\n\n\n\n\n\n\n",signal_type_flag);
      HAL_Delay(50);
      Generate_SineWave();

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }

  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
