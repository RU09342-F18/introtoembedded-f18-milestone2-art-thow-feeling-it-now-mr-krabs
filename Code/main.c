#include <msp430.h> 


/**
 * main.c
 */
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	// Timer A0 Setup
    TA0CTL |= TASSEL_2 + MC_1;                      // Setup Timer A0 in UP mode w/ SMCLK
    TA0CCR0 = 100;                                  // Set TA0CCR0 to 255

    TA0CCTL1 |= OUTMOD_7;                           // Enable Output
    TA0CCR1 = 50;                                 // Set Duty Cycle to 0%

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
    UCA1CTL1 &= ~UCSWRST;                       // **Initialize USCI state machine**
    UCA1IE |= UCRXIE;                           // Enable Interrupt on RX
    UCA1IFG &= ~UCRXIFG;                        // Clear Interrupt Flag

    ADC12CTL0 = ADC12SHT02 + ADC12ON;         // Sampling time, ADC12 on
      ADC12CTL1 = ADC12SHP;                     // Use sampling timer
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
	return 0;
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
    if (ADC12MEM0 >= 0x7ff)                 // ADC12MEM = A0 > 0.5AVcc?
      P1OUT |= BIT0;                        // P1.0 = 1
    else
      P1OUT &= ~BIT0;                       // P1.0 = 0

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

/*
#pragma vector=USCI_A1_VECTOR
__interrupt void USCI_A1_ISR(void)
{
  switch(byte)
  {
  case 0:
      size = UCA1RXBUF;                    // Save Packet Size
      break;                                    // Vector 0 - no interrupt
  case 1:
      TA0CCR1 = UCA1RXBUF;                      // Sets Red PWM
      break;
  case 2:
      TA0CCR2 = UCA1RXBUF;                      // Sets Green PWM
      break;
  case 3:
      TA0CCR3 = UCA1RXBUF;                      // Sets Blue PWM
      while(!(UCA1IFG & UCTXIFG));
          UCA1TXBUF = size - 3;
      break;
  default:
      if(byte > size)
      {
          byte = -1;
          size = 0;
      }
      else
      {
          while(!(UCA1IFG & UCTXIFG));
              UCA1TXBUF = UCA1RXBUF;
      }
      break;
  }
  byte++;
}
*/