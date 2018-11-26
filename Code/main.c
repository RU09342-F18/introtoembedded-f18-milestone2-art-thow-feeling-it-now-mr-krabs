#include <msp430.h> 


/**
 * main.c
 */

unsigned int setTemp = 20;                          // Initial Set Temp (20 Celcius)
unsigned int currentTemp = 0;                       // Current Temp
float v_in = 0.0;
float tempTemp;

unsigned char a;
unsigned char b;
unsigned char c;
unsigned int firstDigit;
unsigned int secondDigit;
unsigned int thirdDigit;



int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// Timer A0 Setup
    TA0CTL |= TASSEL_2 + MC_1;                      // Setup Timer A0 in UP mode w/ SMCLK
    TA0CCR0 = 1000;                                  // Set TA0CCR0 to 255

    TA0CCTL1 |= OUTMOD_7;                           // Enable Output
    TA0CCR1 = 500;                                 // Set Duty Cycle to 0%

    P1OUT |= BIT2;                              // Set Pin 1.2 to High
    P1DIR |= BIT2;                              // Set Pin 1.2 to Output
    P1SEL |= BIT2;                              // Enable TA0.1 Output

    // UART Setup

    P4SEL |= BIT4+BIT5;
    // TX
    P3SEL |= BIT3;                              // Enable TX on pin 3.3

    // RX
    P3SEL |= BIT4;                              // Enable RX on pin 3.4

    // UART Initialization
    UCA1CTL1 |= UCSWRST;                        // **Put state machine in reset**
    UCA1CTL1 |= UCSSEL_2;
    UCA1BR0 = 104;
    UCA1BR1 = 0;
    UCA1MCTL |= UCBRS_1 + UCBRF_0;
    UCA1CTL1 &= ~UCSWRST;                               // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                                   // Enable Interrupt on RX
    UCA1IFG &= ~UCRXIFG;                                // Clear Interrupt Flag

    // Voltage Reference
    ADC12CTL0 = ADC12SHT02 + ADC12ON;         // Sampling time, ADC12 on
    ADC12CTL1 = ADC12SHP + ADC12DIV_7+ ADC12SSEL_1;                     // Use sampling timer
    ADC12IE = 0x01;                           // Enable interrupt
    ADC12CTL0 |= ADC12ENC;
    P6SEL |= 0x01;                            // P6.0 ADC option select
    P1DIR |= 0x01;                            // P1.0 output

      while (1)
      {
        ADC12CTL0 |= ADC12SC;                   // Start sampling/conversion


        __bis_SR_register(LPM0_bits + GIE);     // LPM0, ADC12_ISR will force exit
        __no_operation();                       // For debugger
      }
}


#pragma vector = ADC12_VECTOR
__interrupt void ADC12_ISR(void)
{
  switch(__even_in_range(ADC12IV,34))
  {
  case  0: break;                           // Vector  0:  No interrupt
  case  2: break;                           // Vector  2:  ADC overflow
  case  4: break;                           // Vector  4:  ADC timing overflow
  case  6:                                  // Vector  6:  ADC12IFG0

      // Temperature Calculations
    v_in = (ADC12MEM0 * 3.3 )/ 4095;
    tempTemp = (v_in - 0.424) * 160;

    currentTemp = (int)tempTemp;

    // TX operations
    firstDigit = currentTemp % 100;

    a = (currentTemp - firstDigit) / 100;

    secondDigit = firstDigit % 10;

    b = (firstDigit - secondDigit) / 10;

    c = secondDigit;

    if(a != 0)
    {
        while(!(UCA1IFG & UCTXIFG));
                UCA1TXBUF = a + '0';
            __delay_cycles(1000);
    }
    while(!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = b + '0';
    __delay_cycles(1000);
    while(!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = c + '0';
    __delay_cycles(2000);
    while(!(UCA1IFG & UCTXIFG));
        UCA1TXBUF = ' ';
    __delay_cycles(1000);

    if((currentTemp > (setTemp + 2)) && (TA0CCR1 < 1000))
    {
        TA0CCR1 = TA0CCR1 + 10;
        if(TA0CCR1 > 1000)
            TA0CCR1 = 1000;
    }
    else if(currentTemp < (setTemp - 2))
    {
        TA0CCR1 = TA0CCR1 - 10;
        if(TA0CCR1 < 0)
            TA0CCR1 = 0;
    }

    __bic_SR_register_on_exit(LPM0_bits);   // Exit active CPU
  case  8: break;                           // Vector  8:  ADC12IFG1
  case 10: break;                           // Vector 10:  ADC12IFG2
  case 12: break;                           // Vector 12:  ADC12IFG3
  case 14: break;                           // Vector 14:  ADC12IFG4
  case 16: break;                           // Vector 16:  ADC12IFG5
  case 18: break;                           // Vector 18:  ADC12IFG6
  case 20: break;                           // Vector 20:  ADC12IFG7
  case 22: break;                           // Vector 22:  ADC12IFG8
  case 24: break;                           // Vector 24:  ADC12IFG9
  case 26: break;                           // Vector 26:  ADC12IFG10
  case 28: break;                           // Vector 28:  ADC12IFG11
  case 30: break;                           // Vector 30:  ADC12IFG12
  case 32: break;                           // Vector 32:  ADC12IFG13
  case 34: break;                           // Vector 34:  ADC12IFG14
  default: break;
  }
}


#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
    setTemp = UCA1RXBUF;                        // Might change depending on value sent through uart
}

