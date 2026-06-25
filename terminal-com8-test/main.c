//===========================================================
// Test program UART6 TX 8/N/1 @ 115200 bit via Port PP1 
// LTL 17.5.2020 / mod. V0.2 - V0.4 RMS  1.6.2023
//==========================================================
#include "inc/tm4c1294ncpdt.h" // Header of the controller type
#include <stdint.h>             // Header w. types for the register ..
#include <stdio.h>
#define IDLETIME 100000                  // waiting time between transmissions
#define MAXSIZE 50
int wt = 0;                            // auxillary variable
char buffer[MAXSIZE];
void config_port(void){
    // initialize Port P
    SYSCTL_RCGCGPIO_R |= 0x02000;   // switch on clock for Port P
    wt++;                           // delay for stable clock
    // initialize Port P
      GPIO_PORTP_DEN_R |= 0x2;   // enable digital pin function for PP1
      GPIO_PORTP_DIR_R |= 0x2;   // set PP1 to output
      GPIO_PORTP_AFSEL_R |= 0x2; // switch to alternate pin function PP1
      GPIO_PORTP_PCTL_R |= 0x10; // select alternate pin function PP1->U6Tx

    // PP0 = Rx
        GPIO_PORTP_DEN_R   |= (1<<0);
        GPIO_PORTP_AFSEL_R |= (1<<0);
        GPIO_PORTP_PCTL_R  |= (1<<0);
}
void config_leds(void){
    SYSCTL_RCGCGPIO_R |= (1<<11);
    while((SYSCTL_PRGPIO_R & (1<<11)) == 0);
    GPIO_PORTM_DEN_R |= 0x0F;
    GPIO_PORTM_DIR_R |= 0x0F;
}
void decode_command(void){
    if(buffer[0] == 'l' && buffer[1] == 'e' && buffer[2] == 'd'){
        if(buffer[3] == '+'){
            if(buffer[4] == '0')      GPIO_PORTM_DATA_R |= (1<<0);
            else if(buffer[4] == '1') GPIO_PORTM_DATA_R |= (1<<1);
            else if(buffer[4] == '2') GPIO_PORTM_DATA_R |= (1<<2);
            else if(buffer[4] == '3') GPIO_PORTM_DATA_R |= (1<<3);
            else return;
        }
        else if(buffer[3] == '-'){
            if(buffer[4] == '0')      GPIO_PORTM_DATA_R &= ~(1<<0);
            else if(buffer[4] == '1') GPIO_PORTM_DATA_R &= ~(1<<1);
            else if(buffer[4] == '2') GPIO_PORTM_DATA_R &= ~(1<<2);
            else if(buffer[4] == '3') GPIO_PORTM_DATA_R &= ~(1<<3);
            else return;
        }
        else return;
    }
    else return;
}
void config_uart(void){
    // initialize UART6
    SYSCTL_RCGCUART_R |= 0x40;  // switch on clock for UART6
    while((SYSCTL_PRUART_R & (1<<6)) == 0);                       // delay for stable clock
    UART6_CTL_R &= ~0x01;       // disable UART6 for config
    // initialize bitrate of 115200 bit per second
    UART6_IBRD_R = 8;         // set DIVINT of BRD floor(16 MHz/16*115200 bps)
    UART6_FBRD_R = 44;          // set DIVFRAC of BRD remaining fraction divider
    UART6_LCRH_R = 0x00000060;  // serial format 8N1
    UART6_CTL_R |= 0x0301;      // UART transmit on and UART enable
}
void send_char(char c){
    while((UART6_FR_R & (1<<5)) != 0);
    UART6_DR_R = c;
}
void send_prompt(void){
    send_char('\r');
    send_char('\n');
    send_char('>');
}
char read_char(void){
    while((UART6_FR_R & (1<<4)) != 0);
    return UART6_DR_R;
}
void idle()      {                     // simple wait for idle state
   int i;
   for (i=IDLETIME;i>0;i--);           // count down loop for waiting 
}
void main(void){
    config_port();
    config_uart();
    config_leds();
    GPIO_PORTM_DATA_R |= 0x0F;  // turn ALL PM0-PM3 ON immediately

    char r = '\r';

    while(1){
        send_prompt();

        int index = 0;
        while(1){
            char c = read_char();
            //send_char(c);

            if(c == '\r' || index == MAXSIZE - 1){
                buffer[index] = '\0';
                break;
            }
            buffer[index] = c;
            index++;
        }
        printf("%s\n", buffer);
        decode_command();
    }
}
