/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : FSMC.h
  * Description        : This file provides code for the configuration
  *                      of the FSMC peripheral.
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
/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FSMC_H
#define __FSMC_H
#ifdef __cplusplus
 extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

extern SRAM_HandleTypeDef hsram4;

/* USER CODE BEGIN Private defines */
//
// Created by Lyt on 2024/11/26.
//

#ifndef A_TASK_TFTLCD_H
#define A_TASK_TFTLCD_H

#include "main.h"
#include "stdlib.h"
//
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK STM32������
//2.8��/3.5��/4.3��/7�� TFTҺ������
//֧������IC�ͺŰ���:ILI9341/NT35310/NT35510/SSD1963��
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/4/8
//�汾��V4.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//*******************************************************************************
//�޸�˵��
//V4.0 20211111
//1��ȥ���󲿷ֲ���������IC��֧��
//2��������ST7789����IC��֧��
//3���Ż�����ṹ������Դ�룩
//

//LCD��Ҫ������
typedef struct
{
    uint16_t width;											//LCD ���
    uint16_t height;									 	//LCD �߶�
    uint16_t id;											//LCD ID
    uint8_t  dir;											//���������������ƣ�0��������1��������
    uint16_t wramcmd;										//��ʼдgramָ��
    uint16_t setxcmd;										//����x����ָ��
    uint16_t setycmd;										//����y����ָ��
}_lcd_dev;

//LCD����
extern _lcd_dev lcddev;							//����LCD��Ҫ����
//LCD�Ļ�����ɫ�ͱ���ɫ
extern uint32_t POINT_COLOR;					//Ĭ�Ϻ�ɫ
extern uint32_t BACK_COLOR; 					//������ɫ.Ĭ��Ϊ��ɫ

//
//-----------------MCU�� LCD�˿ڶ���----------------
//LCD��ַ�ṹ��
typedef struct
{
    volatile uint16_t LCD_REG;
    volatile uint16_t LCD_RAM;
} LCD_TypeDef;

#define LCD_BASE 	((uint32_t)(0x6C000000 | 0x0000007E))
#define LCD 			((LCD_TypeDef *) LCD_BASE)
//
//LCD�ֱ�������
#define SSD_HOR_RESOLUTION		480			//LCDˮƽ�ֱ���
#define SSD_VER_RESOLUTION		320			//LCD��ֱ�ֱ���

//ɨ�跽����
#define L2R_U2D  0 							//������,���ϵ���
#define L2R_D2U  1 							//������,���µ���
#define R2L_U2D  2 							//���ҵ���,���ϵ���
#define R2L_D2U  3 							//���ҵ���,���µ���

#define U2D_L2R  4 							//���ϵ���,������
#define U2D_R2L  5 							//���ϵ���,���ҵ���
#define D2U_L2R  6 							//���µ���,������
#define D2U_R2L  7							//���µ���,���ҵ���

#define DFT_SCAN_DIR  L2R_U2D  				//Ĭ�ϵ�ɨ�跽��

//������ɫ
#define WHITE				0xFFFF
#define BLACK				0x0000
#define BLUE				0x001F
#define BRED				0XF81F
#define GRED				0XFFE0
#define GBLUE				0X07FF
#define RED					0xF800
#define MAGENTA			    0xF81F
#define GREEN				0x07E0
#define CYAN				0x7FFF
#define YELLOW			    0xFFE0
#define BROWN				0XBC40 						//��ɫ
#define BRRED				0XFC07 						//�غ�ɫ
#define GRAY				0X8430 						//��ɫ

//GUI��ɫ
#define DARKBLUE		    0X01CF						//����ɫ
#define LIGHTBLUE		    0X7D7C						//ǳ��ɫ
#define GRAYBLUE		    0X5458 						//����ɫ
//������ɫΪPANEL����ɫ

#define LIGHTGREEN	        0X841F 						//ǳ��ɫ
#define LGRAY				0XC618 						//ǳ��ɫ(PANNEL),���屳��ɫ

#define LGRAYBLUE		    0XA651 						//ǳ����ɫ(�м����ɫ)
#define LBBLUE			    0X2B12 						//ǳ����ɫ(ѡ����Ŀ�ķ�ɫ)

/*Ӧ�ú���*/
//��ʼ��
void LCD_Init(void);
//����ʾ
void LCD_DisplayOn(void);
//����ʾ
void LCD_DisplayOff(void);
//����
void LCD_Clear(uint32_t Color);
//���ù��
void LCD_SetCursor(uint16_t Xpos, uint16_t Ypos);
//����
void LCD_DrawPoint(uint16_t x,uint16_t y);
//���ٻ���
void LCD_Fast_DrawPoint(uint16_t x,uint16_t y,uint32_t color);
//����
uint32_t LCD_ReadPoint(uint16_t x,uint16_t y);
//��Բ
void LCD_Draw_Circle(uint16_t x0,uint16_t y0,uint8_t r);
//����
void LCD_DrawLine(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
//������
void LCD_DrawRectangle(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2);
//��䵥ɫ
void LCD_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint32_t color);
//���ָ����ɫ
void LCD_Color_Fill(uint16_t sx,uint16_t sy,uint16_t ex,uint16_t ey,uint16_t *color);
//��ʾһ���ַ�
void LCD_ShowChar(uint16_t x,uint16_t y,uint8_t num,uint8_t size,uint8_t mode);
//��ʾһ������
void LCD_ShowNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size);
//��ʾ ����
void LCD_ShowxNum(uint16_t x,uint16_t y,uint32_t num,uint8_t len,uint8_t size,uint8_t mode);
//��ʾһ���ַ���,12/16����
void LCD_ShowString(uint16_t x,uint16_t y,uint16_t width,uint16_t height,uint8_t size,uint8_t *p);

/*�ײ㺯��*/
void LCD_WriteReg(uint16_t LCD_Reg, uint16_t LCD_RegValue);
uint16_t LCD_ReadReg(uint16_t LCD_Reg);
void LCD_WriteRAM_Prepare(void);
void LCD_WriteRAM(uint16_t RGB_Code);
//SSD1963 �������
void LCD_SSD_BackLightSet(uint8_t pwm);
//������ɨ�跽��
void LCD_Scan_Dir(uint8_t dir);
//������Ļ��ʾ����
void LCD_Display_Dir(uint8_t dir);
//���ô���
void LCD_Set_Window(uint16_t sx,uint16_t sy,uint16_t width,uint16_t height);

//LCD������������
#define SSD_HOR_PULSE_WIDTH		1			//ˮƽ����
#define SSD_HOR_BACK_PORCH		46			//ˮƽǰ��
#define SSD_HOR_FRONT_PORCH		210			//ˮƽ����

#define SSD_VER_PULSE_WIDTH		1			//��ֱ����
#define SSD_VER_BACK_PORCH		23			//��ֱǰ��
#define SSD_VER_FRONT_PORCH		22			//��ֱǰ��

//���¼����������Զ�����
#define SSD_HT	(SSD_HOR_RESOLUTION+SSD_HOR_BACK_PORCH+SSD_HOR_FRONT_PORCH)
#define SSD_HPS	(SSD_HOR_BACK_PORCH)
#define SSD_VT 	(SSD_VER_RESOLUTION+SSD_VER_BACK_PORCH+SSD_VER_FRONT_PORCH)
#define SSD_VPS (SSD_VER_BACK_PORCH)


#endif //A_TASK_TFTLCD_H


//----------------------------------------------------------------------------------------------------//


/* USER CODE END Private defines */

void MX_FSMC_Init(void);
void HAL_SRAM_MspInit(SRAM_HandleTypeDef* hsram);
void HAL_SRAM_MspDeInit(SRAM_HandleTypeDef* hsram);

/* USER CODE BEGIN Prototypes */

/* USER CODE END Prototypes */

#ifdef __cplusplus
}
#endif
#endif /*__FSMC_H */

/**
  * @}
  */

/**
  * @}
  */
