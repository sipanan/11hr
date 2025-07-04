/********************************************************************
 FileName:     	HardwareProfile - PIC24FJ256GB110 PIM.h
 Dependencies:  See INCLUDES section
 Processor:     PIC24FJ256GB110
 Hardware:      PIC24FJ256GB110 PIM
 Compiler:      Microchip C30
 Company:       Microchip Technology, Inc.

 Software License Agreement:

 The software supplied herewith by Microchip Technology Incorporated
 (the �Company�) for its PIC� Microcontroller is intended and
 supplied to you, the Company�s customer, for use solely and
 exclusively on Microchip PIC Microcontroller products. The
 software is owned by the Company and/or its supplier, and is
 protected under applicable copyright laws. All rights are reserved.
 Any use in violation of the foregoing restrictions may subject the
 user to criminal sanctions under applicable laws, as well as to
 civil liability for the breach of the terms and conditions of this
 license.

 THIS SOFTWARE IS PROVIDED IN AN �AS IS� CONDITION. NO WARRANTIES,
 WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING, BUT NOT LIMITED
 TO, IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
 PARTICULAR PURPOSE APPLY TO THIS SOFTWARE. THE COMPANY SHALL NOT,
 IN ANY CIRCUMSTANCES, BE LIABLE FOR SPECIAL, INCIDENTAL OR
 CONSEQUENTIAL DAMAGES, FOR ANY REASON WHATSOEVER.

********************************************************************
 File Description:

 Change History:
  Rev   Date         Description
  1.0   11/19/2004   Initial release
  2.1   02/26/2007   Updated for simplicity and to use common
                     coding style
  2.3   09/15/2008   Broke out each hardware platform into its own
                     "HardwareProfile - xxx.h" file
********************************************************************/

#ifndef HARDWARE_PROFILE_PIC24FJ256GB110_PIM_H
#define HARDWARE_PROFILE_PIC24FJ256GB110_PIM_H

    #include "usb_config.h"

    /*******************************************************************/
    /******** USB stack hardware selection options *********************/
    /*******************************************************************/
    //This section is the set of definitions required by the MCHPFSUSB
    //  framework.  These definitions tell the firmware what mode it is
    //  running in, and where it can find the results to some information
    //  that the stack needs.
    //These definitions are required by every application developed with
    //  this revision of the MCHPFSUSB framework.  Please review each
    //  option carefully and determine which options are desired/required
    //  for your application.

    //#define USE_SELF_POWER_SENSE_IO
    #define tris_self_power     TRISAbits.TRISA2    // Input
    #define self_power          1

    //#define USE_USB_BUS_SENSE_IO
    #define tris_usb_bus_sense  TRISBbits.TRISB5    // Input
    #define USB_BUS_SENSE       1 
   
    //Uncomment this to make the output HEX of this project 
    //   to be able to be bootloaded using the HID bootloader
//    #define PROGRAMMABLE_WITH_USB_HID_BOOTLOADER	

    //If the application is going to be used with the HID bootloader
    //  then this will provide a function for the application to 
    //  enter the bootloader from the application (optional)
    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
        #define EnterBootloader() __asm__("goto 0x400")
    #endif   

    /*******************************************************************/
    /******** MDD File System selection options ************************/
    /*******************************************************************/
    #define USE_SD_INTERFACE_WITH_SPI
    #define USE_PIC24F
    #define USE_16BIT

    // Sample definitions for 16-bit processors (modify to fit your own project)
    #define SD_CS				PORTBbits.RB1
    #define SD_CS_TRIS			TRISBbits.TRISB1
    
    #define SD_CD				PORTFbits.RF0
    #define SD_CD_TRIS			TRISFbits.TRISF0
    
    #define SD_WE				PORTFbits.RF1
    #define SD_WE_TRIS			TRISFbits.TRISF1
    
    // Registers for the SPI module you want to use
    #define SPICON1				SPI1CON1
    #define SPISTAT				SPI1STAT
    #define SPIBUF				SPI1BUF
    #define SPISTAT_RBF			SPI1STATbits.SPIRBF
    #define SPICON1bits			SPI1CON1bits
    #define SPISTATbits			SPI1STATbits
    #define SPIENABLE           SPISTATbits.SPIEN

    // Tris pins for SCK/SDI/SDO lines
    #define SPICLOCK			TRISBbits.TRISB0
    #define SPIIN				TRISDbits.TRISD2
    #define SPIOUT			    TRISFbits.TRISF8

    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/
    /******** Application specific definitions *************************/
    /*******************************************************************/
    /*******************************************************************/
    /*******************************************************************/

    /** Board definition ***********************************************/
    //These defintions will tell the main() function which board is
    //  currently selected.  This will allow the application to add
    //  the correct configuration bits as wells use the correct
    //  initialization functions for the board.  These defitions are only
    //  required in the stack provided demos.  They are not required in
    //  final application design.
    #define DEMO_BOARD PIC24FJ256GB110_PIM
    #define EXPLORER_16
    #define PIC24FJ256GB110_PIM
	#define CLOCK_FREQ 32000000
    #define GetSystemClock() CLOCK_FREQ
    #define GetInstructionClock() GetSystemClock()
    
    /** LED ************************************************************/
	#define mInitAllLEDs()      LATA &= 0x00; TRISA &= 0x00;
    
    #define mLED_1              LATAbits.LATA0
    #define mLED_2              LATAbits.LATA1
    #define mLED_3              LATAbits.LATA2
    #define mLED_4              LATAbits.LATA3
	#define mLED_5				LATAbits.LATA4
	#define mLED_6				LATAbits.LATA5
	#define mLED_7				LATAbits.LATA6
	#define mLED_8				LATAbits.LATA7

    #define mGetLED_1()         mLED_1
    #define mGetLED_2()         mLED_2
    #define mGetLED_3()         mLED_3
    #define mGetLED_4()         mLED_4     
    
    #define mLED_1_On()         mLED_1 = 1;
    #define mLED_2_On()         mLED_2 = 1;
    #define mLED_3_On()         mLED_3 = 1;
    #define mLED_4_On()         mLED_4 = 1;
	#define mLED_5_On()			mLED_5 = 1;
	#define mLED_6_On()			mLED_6 = 1;
	#define mLED_7_On()			mLED_7 = 1;
	#define mLED_8_On()			mLED_8 = 1;
    
    #define mLED_1_Off()        mLED_1 = 0;
    #define mLED_2_Off()        mLED_2 = 0;
    #define mLED_3_Off()        mLED_3 = 0;
    #define mLED_4_Off()        mLED_4 = 0;
	#define mLED_5_Off()		mLED_5 = 0;
	#define mLED_8_Off()		mLED_8 = 0;
    
    #define mLED_1_Toggle()     mLED_1 = !mLED_1;
    #define mLED_2_Toggle()     mLED_2 = !mLED_2;
    #define mLED_3_Toggle()     mLED_3 = !mLED_3;
    #define mLED_4_Toggle()     mLED_4 = !mLED_4;
    
    /** SWITCH *********************************************************/
    #define mInitSwitch2()      TRISDbits.TRISD6=1;
    #define mInitSwitch3()      TRISDbits.TRISD7=1;
    #define mInitAllSwitches()  mInitSwitch2();mInitSwitch3();
    #define sw2                 PORTDbits.RD6
    #define sw3                 PORTDbits.RD7

   
    /** POT ************************************************************/
    #define mInitPOT()  {AD1PCFGLbits.PCFG5 = 0;    AD1CON2bits.VCFG = 0x0;    AD1CON3bits.ADCS = 0xFF;    AD1CON1bits.SSRC = 0x0;    AD1CON3bits.SAMC = 0b10000;    AD1CON1bits.FORM = 0b00;    AD1CON2bits.SMPI = 0x0;    AD1CON1bits.ADON = 1;}

    /** MDD File System error checking *********************************/
    // Will generate an error if the clock speed is too low to interface to the card
    #if (GetSystemClock() < 100000)
        #error Clock speed must exceed 100 kHz
    #endif 

    /** I/O pin definitions ********************************************/
    #define INPUT_PIN 1
    #define OUTPUT_PIN 0

#endif  //HARDWARE_PROFILE_PIC24FJ256GB110_PIM_H
