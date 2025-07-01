#include "uart.h"

/*
void UARTInit( int baudRate )
{
	U2BRG				=	baudRate;
	U2MODE				=	0x8000;
	U2STA				=	0x8400;
	IFS1bits.U2RXIF		=	0;
}

void UARTPutChar( char c )
{
	while( U2STAbits.UTXBF == 1 );
	U1TXREG				=	c;
}

char UARTGetChar( void )
{
	char temp;
	while( IFS1bits.U2RXIF == 0 );
	temp				=	U2RXREG;
	IFS1bits.U2RXIF		=	0;
	return temp;
}
*/

/** EOF *************************************************/

