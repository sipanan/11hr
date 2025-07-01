/** INCLUDES *******************************************************/
#include <stdio.h>
#include <string.h>
#include "USB/USB.h"
#include "HardwareProfile.h"
#include "MDD File System\SD-SPI.h"
#include "MDD File System/FSIO.h"
#include "./USB/usb_function_msd.h"
#include "i2c.h"
#include "display.h"
#include "math.h"
#include <libpic30.h>
#include "globalVars.h"
#include "calc.h"

void PCRequest( void )
{
	if( pcString[0] == 'I' )
	{
		if( pcString[1] == 0x31 )
		{
			if( pcString[2] == 0x30 )
			{
				if( pcString[3] == 0x30 )
				{
				}
				else if( pcString[3] == 0x31 )
				{
					DisplayRequest( '4', '0', '1' );
				}
				else if( pcString[3] == 0x32 )
				{
					DisplayRequest( '4', '0', '2' );
				}
				else if( pcString[3] == 0x33 )
				{
					DisplayRequest( '4', '0', '3' );
				}
				else if( pcString[3] == 0x34 ) {
					DisplayRequest( '8', '0', '1' );
				}
				else if( pcString[3] == 0x35 ) {
					DisplayRequest( '8', '0', '2' );
				}			
			}	
		}	
	}	
}

void PCSendPeak( int start, int mitte, int ende )
{
}	

//EOF
