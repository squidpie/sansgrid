/*
* Author: Matthew Branstetter
* Date: 3/18/2013
* Description: MSP430 SPI Test Code
*/

#include "msp430g2553.h"

#define SCLK BIT5		// SPI Clock at P1.5
#define SOMI BIT6		// SPI SOMI (Slave Out, Master In) at P1.6
#define SIMO BIT7		// SPI SIMO (Slave In, Master Out) at P1.7
#define CS BIT4			// STE (Slave Reset) at P1.3
#define LED BIT0		// LED at 1.0

void spi_init(void);

unsigned char MST_Data, SLV_Data;			// Variables used to store an 8 bit character

void main(void)
{
  volatile unsigned int i;

  WDTCTL = WDTPW + WDTHOLD;                 // Stop watchdog timer
  
  spi_init();								// Initialize SPI Master
  
  IFG2 &= ~UCB0RXIE;
  IE2 |= UCB0RXIE;                          // Enable USCI0 RX interrupt
  
  
  P1OUT &= ~CS;                       		// Now with SPI signals initialized,
  P1OUT |= CS;                       		// reset slave

  __delay_cycles(75);                 		// Wait for slave to initialize

  MST_Data = 0x01;                          // Initialize data values
  SLV_Data = 0x00;

  UCB0TXBUF = MST_Data;                     // Transmit first character

  __bis_SR_register(LPM0_bits + GIE);       // CPU off, enable interrupts
}

// Test for valid RX and TX character
#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCIAB0RX_ISR(void)
{
  volatile unsigned int i;

  while (!(IFG2 & UCB0TXIFG));              // USCI_B0 TX buffer ready?

  if (UCB0RXBUF == SLV_Data)                // Test for correct character RX'd
    P1OUT |= LED;                          	// If correct, light LED
  else
    P1OUT &= ~LED;                         	// If incorrect, clear LED

  MST_Data++;                               // Increment master value
  SLV_Data++;                               // Increment expected slave value
  UCB0TXBUF = MST_Data;                     // Send next value

  __delay_cycles(50);                     	// Add time between transmissions to
}                                           // make sure slave can keep up

void spi_init(void)
{
  // Set UCSWRST 
  UCB0CTL1 = UCSEL_2 + UCSWRST;

  // Initialize all USCI registers with UCSWRST=1 (including UCxCTL1)
  P1OUT = 0x00;                             	
  P1DIR |= LED + CS;
  P1OUT |= LED + CS;
  P1SEL |= SOMI + SIMO + SCLK;
  P1SEL2 |= SOMI + SIMO + SCLK;

  // Configure ports, 3-pin, 8-bit SPI master
  UCB1CTL0 |= UCSYNC+                       // Synchronous mode enabled
              UCCKPL+                       // Clock Polarity High
              UCMSB+                        // MSB first select. 
              UCMST;                        // SPI master.
			  
  UCB0CTL1 |= UCSSEL_2; // SMCLK

  // Intialize UCSI State Machine
  UCB0CTL1 &= ~UCSWRST;
}
