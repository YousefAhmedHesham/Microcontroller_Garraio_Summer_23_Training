#include "inc/tm4c123gh6pm.h"
#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>
#include "inc/hw_memmap.h"
#include "inc/hw_types.h"
#include "driverlib/sysctl.h"
#include "driverlib/gpio.h"
#include "driverlib/pin_map.h"
#include "driverlib/uart.h"
#include "utils/uartstdio.h"

  int num1 = 0 ;
  int num2 = 0 ;
  int result = 0 ;
  int counter ;
  char operator ;
  char operators[4]= { '+' , '-', '*', '/' } ;
  char numbers[10] = { '0' , '1' , '2' , '3' , '4', '5' , '6' , '7' , '8' , '9' } ;

  
//Keypad connected to PORT E, Pins 0,1,2,3 //OUTPUT from tiva    // OUTPUT ROW , left of keypad
//connected to PORT F, Pins 1,2,3,4 // INPUT to tiva             // INPUT COLUMN  , Right of keypad
void uart_print (void)
{
  // Set the system clock to 50MHz
     SysCtlClockSet(SYSCTL_SYSDIV_4 | SYSCTL_USE_PLL | SYSCTL_OSC_MAIN | SYSCTL_XTAL_16MHZ);

     // Enable UART0 and GPIOA
     SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);
     SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);

     // Configure GPIO Pins for UART mode
     GPIOPinConfigure(GPIO_PA0_U0RX);
     GPIOPinConfigure(GPIO_PA1_U0TX);
     GPIOPinTypeUART(GPIO_PORTA_BASE, GPIO_PIN_0 | GPIO_PIN_1);

     // Initialize UART
     UARTStdioConfig(0, 115200, SysCtlClockGet());
}
void delayMs(int n)
{
    int32_t i, j;
    for(i = 0 ; i < n; i++)
        for(j = 0; j < 3180; j++)
            {}  /* do nothing for 1 ms */
}

void PORTE_Init(void)
{
        SYSCTL_RCGCGPIO_R |= 0x10 ; //activate clock for Port E
        while((SYSCTL_PRGPIO_R&0x00000010) == 0){}; //we next will wait for its status bit in the PRGPIO to be true
        GPIO_PORTE_LOCK_R = 0x4C4F434B ; //Unlock the PORTE lock register
        GPIO_PORTE_CR_R = 0x0FF ; //Allow changes for 8 pins // Allow executing functions
        GPIO_PORTE_DIR_R = 0xFF  ; //  0000 1111   specifies bit for bit whether the corresponding pins are input(0) or output(1)
        GPIO_PORTE_PDR_R |= 0xFF ; //Set bits in the PDR register for the switches inputs to have an internal pull-down resistor
        GPIO_PORTE_DEN_R |= 0xFF ; //Digital Enable for my Digital pins

        GPIO_PORTE_DATA_R=0xF;                        // sends 1s to pins PE 0,1,2,3 TO DETECT NEXT INTERRUPT

}

void PORTF_Init(void)
{
  SYSCTL_RCGCGPIO_R |= 0x20;                     // Enable clock for port f
  while((SYSCTL_PRGPIO_R&0x00000020) == 0){};    // Wait for clock to be enabled
  GPIO_PORTF_LOCK_R = 0x4C4F434B ;               // Unlock the PORTF lock register
  GPIO_PORTF_DIR_R = 0x00;                       // make PF1,2,3,4 inputs
  GPIO_PORTF_PDR_R |= 0x1F ;                     // PULL Down pins PF1,2,3,4
  GPIO_PORTF_AFSEL_R = 0x1F;                     // disable alt. funct on PF1,2,3,4
  GPIO_PORTF_DEN_R |= 0x1F;                      // enable digital I/O on PF1,2,3,4
  GPIO_PORTF_PCTL_R &= ~0xfffff;                 // configure PORT F, PF1,2,3,4 as GPIO
  GPIO_PORTF_AMSEL_R &= ~0xFF;                   // disable analog functionality on PORT F

  GPIO_PORTF_IS_R |= 0x1f;                       // PF1,2,3,4 are NOT edge-sensitive , level sensitive
  GPIO_PORTF_IBE_R &= ~0x1f;                     // are not both edges to detect both rising and falling edges
  GPIO_PORTF_IEV_R |= 0x1f;                      // PF1,2,3,4 HIGH LEVEL event

  //NOTE : This keypad uses HIGH level event to make it work like a real keyboard;
  //in case of a long press, the character is printed MANY TIMES.

  GPIO_PORTF_ICR_R = 0x1f;                       // clear flags PF1,2,3,4
  GPIO_PORTF_IM_R |= 0x1f;                       // arm interrupt on PF1,2,3,4
  NVIC_PRI7_R = (NVIC_PRI7_R&0xFF00FFFF)|0x00A00000;   // priority 5
  NVIC_EN0_R = 0x40000000;                       // enable interrupt 30 in NVIC
}
  void calculater (char n)
  {
     delayMs (200);
     int k ;
     for (k = 0 ; k < 10 ; k++ )
     {
      if (n==numbers[k] && counter == 0 )
          {
            num1 = n - '0' ;    UARTprintf("%c  ", n); counter++ ;  break ;
          }
          else if (n==numbers[k] && counter == 1 )
          {
              num2 = n - '0' ;    UARTprintf("%c  ", n) ; counter = 0 ;  break ;
          }
      }
          int j ;
          for (j = 0 ; j < 4 ; j++)
              {
               if ( n == operators[j] )
                  {
                   operator =  operators[j] ;
                   UARTprintf("%c  ", n);
                   break ;
                  }
              }
         switch(operator)
                  {
                   case '+' :
                   result = num1 + num2 ; break ;
                   case '-' :
                   result = num1 - num2 ; break ;
                   case '/' :
                   result = num1 / num2 ; break ;
                   case '*' :
                   result = num1 * num2 ; break ;
                   }
         if (n == '=')
            {
             UARTprintf("%c  ", n);
             UARTprintf("%d \n ", result);
             num1 = 0 ;
             num2 = 0 ;
             operator = '0' ;
             }
}

void GPIOPortF_Handler(void)
{
  GPIO_PORTF_ICR_R = 0x1E;     // acknowledge flags 4,3,2,1
  GPIO_PORTF_DATA_R=0x00;      //Erasing Data in PORT , to allow NEW BUTTON TO BE PRESSED
  GPIO_PORTE_DATA_R = 0x00;

  //JUST ONE SCAN PER INTERRUPT TO KNOW PRESSED BUTTON !!
  char Numbers[4][4] = { {'1', '2', '3', '+'},
                         {'4', '5', '6', '-'},
                         {'7', '8', '9', '*'},
                         {'*', '0', '=', '/'} };


 int i ;
 for ( i=3; i>=0; i--)           //sends ONES to the 4 Columns; PINS 0, 1, 2, 3 //SPECIFIES the COLUMN in which a  BUTTON is PRESSED
     {
       GPIO_PORTE_DATA_R = (1U<<i);
       int y ;
       for ( y=4; y>=1; y--)     //PINS 4, 5, 6, 7    //SPECIFIES the ROW in which a BUTTON is PRESSED
       {

         if( (GPIO_PORTF_DATA_R & (1U<<(y))) )   // a BUTTON IS PRESSED in a ROW
          {
             char n;
             n = (Numbers[4-y][3-i]);

             calculater(n) ;
          }
       }
     }
   GPIO_PORTE_DATA_R=0x0F;  //REsends 1s to pins PE 0,1,2,3 TO DETECT NEXT INTERRUPT
   delayMs (100);   //DEBOUNCING
}


int main(void)
{
   PORTF_Init();
   PORTE_Init();
   uart_print ();

   while(1)
   {
     // Processor enters Sleep Mode to save Power instead of Looping
     // Once Interrupt happens, Processor goes directly into Run mode
   }
}
