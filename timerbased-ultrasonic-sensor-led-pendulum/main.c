/** Default Project for TM4C1294ncpdt
 *
 * File:    main.c
 * Author:
 * Date:
 * Version:
 *
 * Blinks the onboard LED using a busy-wait loop as delay
 */

#include <stdint.h>
#include <stdio.h>
#include "inc/tm4c1294ncpdt.h"

#define SYSCLK      16000000UL

#define TRIG_PIN    0x01    // PD0
#define ECHO_PIN    0x02    // PD1

#define LR_PIN      0x01    // PL0

#define ALL_LEDS    0xFF    // PM0-PM7

void PortD_Init(void);
void PortM_Init(void);
void PortL_Init(void);
void Timer0_Init(void);
void Timer1_Init(void);

void sleep(int ms);
float measureDistance_cm(void);
void waitForLeftTurn(void);
void waitForRightTurn(void);
void showVerticalBar(void);
void showDistanceBar(float distance);

int main(void)
{
    float distance;

    PortD_Init();
    PortM_Init();
    PortL_Init();

    Timer0_Init();
    Timer1_Init();

    while(1)
    {
        waitForLeftTurn();

	showVerticalBar();

        distance = measureDistance_cm();
        printf("Distance = %.2f cm\n", distance);

        waitForRightTurn();

	showVerticalBar();

        showDistanceBar(distance);
    }
}

void waitForLeftTurn(void)
{
    while((GPIO_PORTL_DATA_R & LR_PIN) == 0);
}

void waitForRightTurn(void)
{
    while((GPIO_PORTL_DATA_R & LR_PIN) != 0);
}

void showVerticalBar(void)
{
    GPIO_PORTM_DATA_R = 0xFF;
    sleep(2);
    GPIO_PORTM_DATA_R = 0x00;
}

void showDistanceBar(float distance)
{
    uint8_t pattern;

    if(distance < 10)
        pattern = 0x01;
    else if(distance < 20)
        pattern = 0x03;
    else if(distance < 30)
        pattern = 0x07;
    else if(distance < 40)
        pattern = 0x0F;
    else if(distance < 50)
        pattern = 0x1F;
    else if(distance < 60)
        pattern = 0x3F;
    else if(distance < 70)
        pattern = 0x7F;
    else
        pattern = 0xFF;

    GPIO_PORTM_DATA_R = pattern;
    sleep(600);
    GPIO_PORTM_DATA_R = 0x00;
}

void PortD_Init(void)
{
    SYSCTL_RCGCGPIO_R |= (1 << 3);

    while((SYSCTL_PRGPIO_R & (1 << 3)) == 0);

    GPIO_PORTD_AHB_DIR_R |= TRIG_PIN;
    GPIO_PORTD_AHB_DIR_R &= ~ECHO_PIN;

    GPIO_PORTD_AHB_DEN_R |= (TRIG_PIN | ECHO_PIN);

    GPIO_PORTD_AHB_DATA_R |= TRIG_PIN;
}

void PortM_Init(void)
{
    SYSCTL_RCGCGPIO_R |= (1 << 11);

    while((SYSCTL_PRGPIO_R & (1 << 11)) == 0);

    GPIO_PORTM_DIR_R |= ALL_LEDS;
    GPIO_PORTM_DEN_R |= ALL_LEDS;

    GPIO_PORTM_DATA_R = 0x00;
}

void PortL_Init(void)
{
    SYSCTL_RCGCGPIO_R |= (1 << 10);

    while((SYSCTL_PRGPIO_R & (1 << 10)) == 0);

    GPIO_PORTL_DIR_R &= ~LR_PIN;
    GPIO_PORTL_DEN_R |= LR_PIN;
}

void Timer0_Init(void)
{
    SYSCTL_RCGCTIMER_R |= (1 << 0);

    while((SYSCTL_PRTIMER_R & (1 << 0)) == 0);

    TIMER0_CTL_R &= ~0x01;
}

void Timer1_Init(void)
{
    SYSCTL_RCGCTIMER_R |= (1 << 1);

    while((SYSCTL_PRTIMER_R & (1 << 1)) == 0);

    TIMER1_CTL_R &= ~0x01;
    TIMER1_CFG_R = 0x00;
    TIMER1_TAMR_R = 0x02;
    TIMER1_TAILR_R = 0xFFFFFFFF;
}

void sleep(int ms)
{
    TIMER0_CTL_R &= ~0x01;
    TIMER0_CFG_R = 0x00;
    TIMER0_TAMR_R = 0x01;

    TIMER0_TAILR_R = ((SYSCLK / 1000) * ms) - 1;

    TIMER0_ICR_R = 0x01;
    TIMER0_CTL_R |= 0x01;

    while((TIMER0_RIS_R & 0x01) == 0);

    TIMER0_ICR_R = 0x01;
}

float measureDistance_cm(void)
{
    uint32_t elapsed_ticks;
    float time_us;
    float distance_cm;

    GPIO_PORTD_AHB_DATA_R |= TRIG_PIN;
    sleep(500);

    GPIO_PORTD_AHB_DATA_R &= ~TRIG_PIN;
    //sleep(1);

    //GPIO_PORTD_AHB_DATA_R |= TRIG_PIN;

    while((GPIO_PORTD_AHB_DATA_R & ECHO_PIN) == 0);

    TIMER1_CTL_R &= ~0x01;
    TIMER1_TAV_R = 0xFFFFFFFF;
    TIMER1_ICR_R = 0x01;
    TIMER1_CTL_R |= 0x01;

    while((GPIO_PORTD_AHB_DATA_R & ECHO_PIN) != 0);

    TIMER1_CTL_R &= ~0x01;

    elapsed_ticks = 0xFFFFFFFF - TIMER1_TAR_R;

    time_us = elapsed_ticks / 16.0;

    distance_cm = time_us / 58.0;

    return distance_cm;
}


