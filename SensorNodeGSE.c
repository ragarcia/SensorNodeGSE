/******************************************************************************
* CC430 RF Code Example - TX and RX (fixed packet length =< FIFO size)
*
* Simple RF Link to Toggle Receiver's LED by pressing Transmitter's Button
* Warning: This RF code example is setup to operate at either 868 or 915 MHz,
* which might be out of allowable range of operation in certain countries.
* The frequency of operation is selectable as an active build configuration
* in the project menu.
*
* Please refer to the appropriate legal sources before performing tests with
* this code example.
*
* This code example can be loaded to 2 CC430 devices. Each device will transmit
* a small packet, less than the FIFO size, upon a button pressed. Each device will also toggle its LED
* upon receiving the packet.
*
* The RF packet engine settings specify fixed-length-mode with CRC check
* enabled. The RX packet also appends 2 status bytes regarding CRC check, RSSI
* and LQI info. For specific register settings please refer to the comments for
* each register in RfRegSettings.c, the CC430x614x User's Guide, and/or
* SmartRF Studio.
*
* G. Larmore
* Texas Instruments Inc.
* June 2012
* Built with IAR v5.40.1 and CCS v5.2
******************************************************************************/


#include "SensorNodeGSE.h"
#include <stdio.h>

#define  PACKET_LEN_RX      (0x20)			// PACKET_LEN <= 61
#define  PACKET_LEN_TX      (0x0A)			// PACKET_LEN <= 61
#define  RSSI_IDX           (PACKET_LEN_RX)    // Index of appended RSSI
#define  CRC_LQI_IDX        (PACKET_LEN_RX+1)  // Index of appended LQI, checksum
#define  CRC_OK             (BIT7)          // CRC_OK bit
#define  PATABLE_VAL        (0x51)          // 0 dBm output


//extern RF_SETTINGS rfSettings;
RF_SETTINGS rfSettings_plus = {
    0x08,   // FSCTRL1   Frequency synthesizer control.
    0x00,   // FSCTRL0   Frequency synthesizer control.
    0x21,   // FREQ2     Frequency control word, high byte.
    0x62,   // FREQ1     Frequency control word, middle byte.
    0x76,   // FREQ0     Frequency control word, low byte.
    0xCA,   // MDMCFG4   Modem configuration.
    0x83,   // MDMCFG3   Modem configuration.
    0x93,   // MDMCFG2   Modem configuration.
    0x22,   // MDMCFG1   Modem configuration.
    0xF8,   // MDMCFG0   Modem configuration.
    0x00,   // CHANNR    Channel number.
    0x34,   // DEVIATN   Modem deviation setting (when FSK modulation is enabled).
    0x56,   // FREND1    Front end RX configuration.
    0x10,   // FREND0    Front end TX configuration.
    0x18,   // MCSM0     Main Radio Control State Machine configuration.
    0x16,   // FOCCFG    Frequency Offset Compensation Configuration.
    0x6C,   // BSCFG     Bit synchronization Configuration.
    0x43,   // AGCCTRL2  AGC control.
    0x40,   // AGCCTRL1  AGC control.
    0x91,   // AGCCTRL0  AGC control.
    0xE9,   // FSCAL3    Frequency synthesizer calibration.
    0x2A,   // FSCAL2    Frequency synthesizer calibration.
    0x00,   // FSCAL1    Frequency synthesizer calibration.
    0x1F,   // FSCAL0    Frequency synthesizer calibration.
    0x59,   // FSTEST    Frequency synthesizer calibration.
    0x81,   // TEST2     Various test settings.
    0x35,   // TEST1     Various test settings.
    0x09,   // TEST0     Various test settings.
    0x47,   // FIFOTHR   RXFIFO and TXFIFO thresholds.
    0x29,   // IOCFG2    GDO2 output pin configuration.
    0x06,   // IOCFG0    GDO0 output pin configuration. Refer to SmartRF® Studio User Manual for detailed pseudo register explanation.
    0x04,   // PKTCTRL1  Packet automation control.
    0x04,   // PKTCTRL0  Packet automation control.
    0x00,   // ADDR      Device address.
    PACKET_LEN_RX    // PKTLEN    Packet length.
};

unsigned char packetReceived;
unsigned char packetTransmit;

unsigned char RxBuffer[PACKET_LEN_RX+2];
unsigned char RxBufferLength = 0;
const unsigned char TxBuffer[PACKET_LEN_TX]= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};	// RSSI, PKT_CNT
unsigned char buttonPressed = 0;
unsigned int i = 0;
unsigned int toggle = 0;

unsigned char transmitting = 0;
unsigned char receiving = 0;

FILE* MYF;
char mybuf[20];

void main( void )
{
	// Stop watchdog timer to prevent time out reset
	  WDTCTL = WDTPW + WDTHOLD;

	// UART stuff
	PMAPPWD = 0x02D52;                        // Get write-access to port mapping regs
	P1MAP5 = PM_UCA0RXD;                      // Map UCA0RXD output to P1.5
	P1MAP6 = PM_UCA0TXD;                      // Map UCA0TXD output to P1.6
	PMAPPWD = 0;                              // Lock port mapping registers

	P1DIR = BIT0+BIT6+BIT4+BIT7;                  // Set P1.6 as TX output, P1.4 as VCC, P1.7 as GND
	P1SEL |= BIT5 + BIT6;                     // Select P1.5 & P1.6 to UART function
	P1OUT &= ~BIT7;							  // set GND pin low
	P1OUT |= BIT4;							  // set VCC pin high

	UCA0CTL1 |= UCSWRST;                      // **Put state machine in reset**
	UCA0CTL1 |= UCSSEL_2;                     // SMCLK
	UCA0BR0 = 9;                              // 1MHz 115200 (see User's Guide)
	UCA0BR1 = 0;                              // 1MHz 115200
	//UCA0CTL0 |= UCMSB;					  // MSB first
	//UCA0CTL0 |= UCPEN;					  // parity enable (odd by default)
	UCA0MCTL |= UCBRS_1 + UCBRF_0;            // Modulation UCBRSx=1, UCBRFx=0
	UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
	UCA0IE |= UCRXIE;                         // Enable USCI_A0 RX interrupt
	__bis_SR_register(GIE);                   // interrupts enabled

	char hdr_str[17] = "packet contents: ";
	char init_str[10] = "starting\r\n";

	for (i = 0; i < (sizeof init_str); i++) {
		while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
		UCA0TXBUF = init_str[i];
	}

//  // FILE I/O initialization
//  printf("starting\n");
//  FILE* MYF = fopen("loggy.txt","w");

  // Increase PMMCOREV level to 2 for proper radio operation
  SetVCore(2);

  ResetRadioCore();
  InitRadio();
  InitButtonLeds();

  ReceiveOn();
  receiving = 1;

  while (1)
  {
    __bis_SR_register( LPM3_bits + GIE );
    __no_operation();
//	fprintf(MYF,"packet contents: ");
//		for(i = 0; i < PACKET_LEN;i++){
//			//fprintf(MYF,"%x ", RxBuffer[i]);
//			fprintf(MYF,"%x%x ", RxBuffer[i]>>4, RxBuffer[i]%16);
//		}
//		fprintf(MYF,"\n");
//	fflush(MYF);

	for (i = 0; i < (sizeof hdr_str); i++) {
		while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
		UCA0TXBUF = hdr_str[i];
	}
	for (i = 0; i < PACKET_LEN_RX+1; i++) {
		while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
		if ((RxBuffer[i]>>4) < 10) {
		  UCA0TXBUF = (RxBuffer[i]>>4)+0x30;
		} else {
		  UCA0TXBUF = (RxBuffer[i]>>4)+0x37;
		}
		while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
		if ((RxBuffer[i]%16) < 10) {
		  UCA0TXBUF = (RxBuffer[i]%16)+0x30;
		} else {
		  UCA0TXBUF = (RxBuffer[i]%16)+0x37;
		}
		while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
		UCA0TXBUF = 0x20;
	}
	while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
	UCA0TXBUF = 0x0D;
	while (!(UCA0IFG&UCTXIFG));             // USCI_A0 TX buffer ready?
	UCA0TXBUF = 0x0A;

    if (buttonPressed)                      // Process a button press->transmit
    {
      P3OUT |= BIT6;                        // Pulse LED during Transmit
      buttonPressed = 0;
      P1IFG = 0;

      ReceiveOff();
      receiving = 0;
      Transmit( (unsigned char*)TxBuffer, sizeof TxBuffer);
      transmitting = 1;

      P1IE |= BIT7;                         // Re-enable button press
    }
    else if(!transmitting)
    {
    	ReceiveOn();
    	receiving = 1;
    }
  }
}

void InitButtonLeds(void)
{
  // Set up the button as interruptible
  P1DIR &= ~BIT7;
  P1REN |= BIT7;
  P1IES &= BIT7;
  P1IFG = 0;
  P1OUT |= BIT7;
  P1IE  |= BIT7;

  // Initialize Port J
  PJOUT = 0x00;
  PJDIR = 0xFF;

  // Set up LEDs
  P1OUT &= ~BIT0;
  P1DIR |= BIT0;
  P3OUT &= ~BIT6;
  P3DIR |= BIT6;
}

void InitRadio(void)
{
  // Set the High-Power Mode Request Enable bit so LPM3 can be entered
  // with active radio enabled
  PMMCTL0_H = 0xA5;
  PMMCTL0_L |= PMMHPMRE_L;
  PMMCTL0_H = 0x00;

  WriteRfSettings(&rfSettings_plus);

  WriteSinglePATable(PATABLE_VAL);
}

#pragma vector=PORT1_VECTOR
__interrupt void PORT1_ISR(void)
{
  switch(__even_in_range(P1IV, 16))
  {
    case  0: break;
    case  2: break;                         // P1.0 IFG
    case  4: break;                         // P1.1 IFG
    case  6: break;                         // P1.2 IFG
    case  8: break;                         // P1.3 IFG
    case 10: break;                         // P1.4 IFG
    case 12: break;                         // P1.5 IFG
    case 14: break;                         // P1.6 IFG
    case 16:                                // P1.7 IFG
      P1IE = 0;                             // Debounce by disabling buttons
      buttonPressed = 1;
      __bic_SR_register_on_exit(LPM3_bits); // Exit active
      break;
  }
}

void Transmit(unsigned char *buffer, unsigned char length)
{
  RF1AIES |= BIT9;
  RF1AIFG &= ~BIT9;                         // Clear pending interrupts
  RF1AIE |= BIT9;                           // Enable TX end-of-packet interrupt

  WriteBurstReg(RF_TXFIFOWR, buffer, length);

  Strobe( RF_STX );                         // Strobe STX
}

void ReceiveOn(void)
{
  RF1AIES |= BIT9;                          // Falling edge of RFIFG9
  RF1AIFG &= ~BIT9;                         // Clear a pending interrupt
  RF1AIE  |= BIT9;                          // Enable the interrupt

  // Radio is in IDLE following a TX, so strobe SRX to enter Receive Mode
  Strobe( RF_SRX );
}

void ReceiveOff(void)
{
  RF1AIE &= ~BIT9;                          // Disable RX interrupts
  RF1AIFG &= ~BIT9;                         // Clear pending IFG

  // It is possible that ReceiveOff is called while radio is receiving a packet.
  // Therefore, it is necessary to flush the RX FIFO after issuing IDLE strobe
  // such that the RXFIFO is empty prior to receiving a packet.
  Strobe( RF_SIDLE );
  Strobe( RF_SFRX  );
}

#pragma vector=CC1101_VECTOR
__interrupt void CC1101_ISR(void)
{
  switch(__even_in_range(RF1AIV,32))        // Prioritizing Radio Core Interrupt
  {
    case  0: break;                         // No RF core interrupt pending
    case  2: break;                         // RFIFG0
    case  4: break;                         // RFIFG1
    case  6: break;                         // RFIFG2
    case  8: break;                         // RFIFG3
    case 10: break;                         // RFIFG4
    case 12: break;                         // RFIFG5
    case 14: break;                         // RFIFG6
    case 16: break;                         // RFIFG7
    case 18: break;                         // RFIFG8
    case 20:                                // RFIFG9
      if(receiving)			    // RX end of packet
      {
        // Read the length byte from the FIFO
        RxBufferLength = ReadSingleReg( RXBYTES );
        ReadBurstReg(RF_RXFIFORD, RxBuffer, RxBufferLength);

        // Stop here to see contents of RxBuffer
        __no_operation();

        // Check the CRC results
        if(RxBuffer[CRC_LQI_IDX] & CRC_OK)  {
          P1OUT ^= BIT0;                    // Toggle LED1
        }
      }
      else if(transmitting)		    // TX end of packet
      {
        RF1AIE &= ~BIT9;                    // Disable TX end-of-packet interrupt
        P3OUT &= ~BIT6;                     // Turn off LED after Transmit
        transmitting = 0;
      }
      else while(1); 			    // trap
      break;
    case 22: break;                         // RFIFG10
    case 24: break;                         // RFIFG11
    case 26: break;                         // RFIFG12
    case 28: break;                         // RFIFG13
    case 30: break;                         // RFIFG14
    case 32: break;                         // RFIFG15
  }
  __bic_SR_register_on_exit(LPM3_bits);
}

// do nothing if UART character received
#pragma vector=USCI_A0_VECTOR
__interrupt void USCI_A0_ISR(void)
{
  switch(__even_in_range(UCA0IV,4))
  {
  case 0:break;                             // Vector 0 - no interrupt
  case 2:                                   // Vector 2 - RXIFG
    // do nothing
    break;
  case 4:break;                             // Vector 4 - TXIFG
  default: break;
  }
}
