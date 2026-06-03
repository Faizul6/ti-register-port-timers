

#include "inc/tm4c1294ncpdt.h"
#include <stdint.h>
#include <stdio.h>


#define TRIG_PIN    0x01    // PD0
#define ECHO_PIN    0x02    // PD1

volatile uint32_t elapsed_ticks =0;
volatile float distance_cm = 0;
//volatile int trigCount = 0;  
//volatile int echoCount = 0;  

void PortD_Init(void);
void GPIOPortD_Handler(void);
void time0A_Init(void);
void time1A_Init(void);
void Timer0A_Handler(void);


int main(void) {

    PortD_Init();
    time0A_Init();
    time1A_Init();

    while(1){
        printf("Distance (cm)= %f\n",distance_cm);
       // printf("trigger (cm)= %d\n",trigCount);
        //printf("Echo (cm)= %d\n",echoCount);
        for(int j=0; j<500000; j++);  // slow down printing

    }
}

void Timer0A_
Handler(void){
    TIMER0_ICR_R = 0x1;
    //trigCount++;
    GPIO_PORTD_AHB_DATA_R |= TRIG_PIN;

    volatile int i;
    for(i = 0; i< 1000; i++);
    GPIO_PORTD_AHB_DATA_R &= ~TRIG_PIN;
}

void GPIOPortD_Handler(void){
    //echoCount++;   
    if(GPIO_PORTD_AHB_DATA_R & ECHO_PIN){
        TIMER1_CTL_R &= ~0x1;
        TIMER1_TAV_R = 0x0;
        TIMER1_CTL_R |= 0x1;
    } else {
        TIMER1_CTL_R &= ~0x1;
        elapsed_ticks = TIMER1_TAR_R;
        distance_cm = (elapsed_ticks / 16.0) / 58.0;
    }

    GPIO_PORTD_AHB_ICR_R = (1<<1);
    
}

void PortD_Init(void){
    SYSCTL_RCGCGPIO_R |= (1<<3);
    while(!(SYSCTL_PRGPIO_R & (1<<3)));
    GPIO_PORTD_AHB_DEN_R = TRIG_PIN | ECHO_PIN;
    GPIO_PORTD_AHB_DIR_R |= TRIG_PIN;
    GPIO_PORTD_AHB_DIR_R &= ~ECHO_PIN;
    GPIO_PORTD_AHB_DATA_R &= ~TRIG_PIN; 

    //interrupt on PD1
    GPIO_PORTD_AHB_IM_R &= ~ECHO_PIN; //masked
    GPIO_PORTD_AHB_IS_R &= ~ECHO_PIN; //edge sense
    GPIO_PORTD_AHB_IBE_R |= ECHO_PIN; //both edge
    // GPIO_PORTD_AHB_IEV_R  (IBE enough since its corresponds to both edges
    GPIO_PORTD_AHB_ICR_R |= ECHO_PIN; //clear flags
    GPIO_PORTD_AHB_IM_R |= ECHO_PIN; //unmased
    NVIC_EN0_R |= (1<<3);
}

void time0A_Init(void){
    SYSCTL_RCGCTIMER_R |= (1<<0);
    while(!(SYSCTL_PRTIMER_R & (1<<0)));

    //will come back to it down or up
    TIMER0_CTL_R &= ~0x1;  //disable
    TIMER0_CFG_R = 0x0;     //32 bit mode
    TIMER0_TAMR_R = 0x2;    //periodic, count down
    TIMER0_TAPR_R     = 0;      //prescaler = 1
    TIMER0_TAILR_R    =  8000000 - 1 ;  // ILR = ceil(16M*0.5) 
    TIMER0_IMR_R       = 0x1;   //timeout interrpt only set with bit 0 TATOIM
    TIMER0_ICR_R        = 0x1;      //clear falg
    TIMER0_CTL_R |= 0x1;        //enable
    NVIC_EN0_R |= (1<<19);         //Timer 0A IRQ 19
}

void time1A_Init(void){
    SYSCTL_RCGCTIMER_R |= (1<<1);
    while(!(SYSCTL_PRTIMER_R & (1<<1)));

    TIMER1_CTL_R &= ~0x01; //disable
    TIMER1_CFG_R = 0x0; //32 bit mode
    TIMER1_TAMR_R =   0x12; // count up,periodic
    TIMER1_TAILR_R = 0xFFFFFFFF; //max count
}



