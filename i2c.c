#include "i2c.h"
#include "p24FJ256GB110.h"

/** I2C INIT SEQUENCE ***********************************************/
void I2CInit( int BRG )
{
	int temp;

	I2C3BRG					=	BRG;
	I2C3CONbits.I2CEN		=	0;		//disable I2C mode
	I2C3CONbits.DISSLW		=	1;		//disable slew rate control
//	I2C1CONbits.SCLREL		=	0;		//disable scl release
	IFS5bits.MI2C3IF		=	0;		//clear interrupt
	I2C3CONbits.I2CEN		=	1;		//enable I2C mode
	I2C3STATbits.IWCOL		=	0;
	I2C3STATbits.BCL		=	0;
//	I2C1STATbits.P			=	1;
//	I2C1STAT				|=	0x0001;
	temp					=	I2C1RCV;//read buffer to clear buffer
//	I2CResetBus();						//reset I2C bus to idle
}

/** I2C RESET BUS ***************************************************/
void I2CResetBus( void )
{
	I2C3CONbits.PEN 		=	1;		//initiate stop bit
	while( I2C3CONbits.PEN );			//wait for HW clear of stop bit
	I2C3CONbits.RCEN		=	0;
	IFS5bits.MI2C3IF		=	0;		//clear interrupt
	I2C3STATbits.IWCOL		=	0;
	I2C3STATbits.BCL		=	0;
}

/** I2C STOP **********************************************************/
void I2CStop( void )
{
	I2C3CONbits.PEN			=	1;
	while( I2C3CONbits.PEN );
}

/** I2C START CONDITION ***********************************************/
void I2CStart( void )
{
	I2C3CONbits.ACKDT		=	0;		//reset any previous ACK
	I2C3CONbits.SEN			=	1;		//initiate start condition
	Nop();
	while( I2C3CONbits.SEN );
}

/** I2C REPEATED START ************************************************/
void I2CRepStart( void )
{
	I2C3CONbits.RSEN		=	1;		//set repeated start bit
	while( I2C3CONbits.RSEN );
}

/** I2C SEND BYTE *****************************************************/
int I2CSendByte( char data )
{
	while( I2C3STATbits.TBF );			//wait for transmit buffer to be cleared
	IFS5bits.MI2C3IF		=	0;		//clear interrupt
	I2C3TRN					=	data;	//load outgoing data byte
	while( I2C3STATbits.TRSTAT );		//wait for 8bits + ACK
	if( I2C3STATbits.ACKSTAT == 1 )
		{ return 1; }					//return 1 if NACK
	else
		{ return 0; }					//return 0 if ACK
}

/** I2C WRITE 1 BYTE **************************************************/
void I2CWriteByte( char addr, char data )
{
	I2CStart();
	I2CSendByte( addr );
	I2CSendByte( data );
	I2CResetBus();
}

/** I2C WRITE N BYTES *************************************************/
void I2CWriteNBytes( char addr, char* data, int cnt )
{
	int i = 0;		

	I2CStart();
	I2CSendByte( addr );
	for( i = 0; i < cnt; i++ )
	{
		I2CSendByte( *data );
		data++;
	}
	I2CResetBus();
}

/** I2C READ BYTE ****************************************************/
char I2CReadByte( int n )
{
	char data				=	0;
	while( I2C3CON & 0x1F );				//wait for idle connection
	I2C3CONbits.RCEN		=	1;			//enable receive
	while( !I2C3STATbits.RBF );				//wait for byte
	data					=	I2C3RCV;	//read byte
	while( I2C3CON & 0x1F );				//wait for idle connection
	if( n == 0 )
		{ I2C3CONbits.ACKDT		=	0; }	//send ACK
	else
		{ I2C3CONbits.ACKDT		=	1; }	//send NACK
	I2C3CONbits.ACKEN		=	1;			//enable ACKbit transmission to slave
	while( I2C3CONbits.ACKEN );				//wait for completion
	return data;
}

/** I2C READ BYTE POINTER ****************************************************/
void I2CReadBytePointer( char* c, int i )
{
	while( I2C3CON & 0x1F );				//wait for idle connection
	I2C3CONbits.RCEN		=	1;			//enable receive
	while( !I2C3STATbits.RBF );				//wait for byte
	*c						=	I2C3RCV;	//read byte
	while( I2C3CON & 0x1F );				//wait for idle connection
	if( i == 1 )
		{ I2C3CONbits.ACKDT		=	1; }	//send NACK
	else
		{ I2C3CONbits.ACKDT		=	0; }	//send ACK
	I2C3CONbits.ACKEN		=	1;			//enable ACKbit transmission to slave
	while( I2C3CONbits.ACKEN );				//wait for completion
}

/** EOF i2c.c ***************************************************************/
