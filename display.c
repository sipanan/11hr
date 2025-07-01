#include 	"display.h"
#include 	"i2c.h"
#include 	"HardwareProfile - PIC24FJ256GB110 PIM.h"
#include 	"p24FJ256GB110.h"
#include 	"MDD File System\SD-SPI.h"
#include	"MDD File System/FSIO.h"
#include 	<string.h>
#include 	"GenericTypeDefs.h"
#include 	"math.h"
#include 	"globalVars.h"
#include	"data.h"
#include 	"definitions.h"

/** VARIABLEN ********************************************/
BOOL 						invertStatus		=	FALSE;
char 						data[50];
static unsigned short 		fluss				=	150;
static short 				fluss_plus			=	0;
static short 				fluss_minus			=	0;
static short 				eichgas_plus		=	0;
static short 				eichgas_minus		=	0;
static unsigned short int 	menge				=	500;
static short 				menge_plus			=	0;
static short 				menge_minus			=	0;
static unsigned short 		volumen				=	70;
static short				volumen_plus		=	0;
static short				volumen_minus		=	0;
static unsigned short 		druck				=	40;
static short 				druck_plus			=	0;
static short 				druck_minus			=	0;
static unsigned short 		messzyklus			=	20;
static unsigned short int 	eichzyklus			=	40;
static unsigned short 		spuelzeit			=	30;
static short 				spuelzeit_plus		=	0;
static short 				spuelzeit_minus		=	0;
static unsigned short 		splitzeit			=	100;
static short 				splitzeit_plus		=	0;
static short 				splitzeit_minus		=	0;
static short 				skalierung_plus		=	0;
static short 				skalierung_minus	=	0;
static unsigned short 		korrekturfaktor		=	100;
static short 				korrektur_plus		=	0;
static short 				korrektur_minus		=	0;
char 						passwort[9]			=	"00000000";
static short 				messortindex		=	7;
static short 				passwortindex		=	7;
char 						minutes;
unsigned short 				min1;
unsigned short 				min2;
char 						hours;
unsigned short 				hrs1;
unsigned short 				hrs2;
char 						day;
unsigned short 				day1;
unsigned short 				day2;
char 						month;
unsigned short 				month1;
unsigned short 				month2;
char 						year;
unsigned short 				year1;
unsigned short 				year2;
unsigned short 				datetimeindex		=	0;
unsigned short 				datecursor			=	0;
unsigned short 				timecursor			=	0;
unsigned short* 			messpunkt_start		=	&messpunkte[0];
unsigned short* 			messpunkt_ende		=	&messpunkte[239];
unsigned short* 			messpunkt_aktuell	=	&messpunkte[0];
unsigned short* 			messpunkt_index		=	&messpunkte[0];
unsigned short int 			detektor_istwert		=	0;
unsigned short int 			detektor_vorgabe		=	0;
unsigned short int 			fluss_vorgabe			=	0;
unsigned short 				vordruck_vorgabe		=	0;
char pwd[20];          // falls als Passwort-Zwischenspeicher genutzt
char pwd_master[20];   // Masterpasswort (z. B. für Vergleich)

//unsigned int defaultTemp = 250;         // Beispielwerte
//unsigned int defaultFlow = 500;
//unsigned int defaultSensibility = 100;
//unsigned int defaultPressure = 200;
//unsigned int defaultSplittime = 20;

unsigned short int			pressure				=	0;

/** SYSTEM PAUSE **************************************
 *
 *	Function waits until the analytic part releases I2C
 *
 ******************************************************/
void SystemPause( void )
{
	int i		=	0;
	for( i = 0; i < DELAY; i++ )
	{
		Nop();
	}	
}

/** SYSTEM SET MA2 **************************************
 *
 *	Function sends the parameter to the analytic part
 *	via I2C
 *
 ********************************************************/
void SystemSetMA2( unsigned int mA )
{
	unsigned short lByte;
	unsigned short hByte;
		
//	lByte		=	mA & 0x00FF;
//	hByte		=	mA >> 8;
//	hByte		=	hByte & 0x00FF;

//Neu Mai 2021 Anfang
	lByte		=	pressure & 0x00FF;
	hByte		=	pressure >> 8;
	hByte		=	hByte & 0x00FF;
//Neu Mai 2021 Ende

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{						SystemPause();
		I2CStop();			SystemPause();
		return;
	}						SystemPause();
	I2CSendByte( 0x07 );	SystemPause();
	I2CSendByte( lByte );	SystemPause();
	I2CSendByte( hByte );	SystemPause();
	I2CStop();				SystemPause();
}	

/** GRAPH INIT GRAPH *******************************************
 *
 *	Function sets the values in the graph array to 
 *	default (200)
 *
 ***************************************************************/
void GraphInitGraph( void )
{
	unsigned int i;

	for( i = 0; i <= 240; i++ )
	{
		messpunkte[i]	=	200;
	}
}

/** CLOCK GET DATESTRING *******************************
 *
 *	Function generates a string in the parameter
 *	that has the structure of the file system
 *		
 *	\Location\Year\Month\Day
 *	\LLLLLLLL\YYYY\MM\DD
 *
 *******************************************************/
void ClockGetDatestring( char *s )
{
	MemoryGetMessort();
	ClockReadYear();
	ClockReadMonth();
	ClockReadDay();
	s[0]		=	0x5C;
	s[1]		=	messort[0];
	s[2]		=	messort[1];
	s[3]		=	messort[2];
	s[4]		=	messort[3];
	s[5]		=	messort[4];
	s[6]		=	messort[5];
	s[7]		=	messort[6];
	s[8]		=	messort[7];
	s[9]		=	0x5C;
	s[10]		=	'2';
	s[11]		=	'0';
	itoa( ( int ) year1, buffer );
	s[12]		=	buffer[0];
	itoa( ( int ) year2, buffer );
	s[13]		=	buffer[0];
	s[14]		=	0x5C;
	itoa( ( int ) month1, buffer );
	s[15]		=	buffer[0];
	itoa( ( int ) month2, buffer );
	s[16]		=	buffer[0];
	s[17]		=	0x5C;
	itoa( ( int ) day1, buffer );
	s[18]		=	buffer[0];
	itoa( ( int ) day2, buffer );
	s[19]		=	buffer[0];
	s[20]		=	0x00;
}

/** GRAPH NEW VALUE ********************************************
 *
 *	Function gets a new value for the graph and adds it to
 *	the array
 *
 ***************************************************************/
void GraphNewValue( unsigned short val )
{
	if( ( val < 0 ) || ( val > 200 ) )
	{
		*messpunkt_aktuell		=	0;
	}
	else
	{
		*messpunkt_aktuell		=	val;
	}	
	messpunkt_aktuell++;
}

/** GRAPH DRAW LINE ********************************************
 *
 *	Function draws a vertical red line to mark the start
 *	and ending of the area that will be integrated
 *
 ***************************************************************/
void GraphDrawLine( int i )
{
	int j		=	0;
	char xPos;
	
//calc x-position
	if( meas6min == TRUE )
	{	j		=	floor( i * 0.67 );	}
	else
	{	j		=	floor( i * 0.33 );	}		
	xPos		=	j & 0x00FF;
	
//set color to red
	data[0]		=	0x1B;
	data[1]		=	'F';
	data[2]		=	'G';
	data[3]		=	0x03;
	data[4]		=	0x01;
	
//draw line
	data[5]		=	0x1B;
	data[6]		=	'G';
	data[7]		=	'D';
	data[8]		=	( xPos+24 );
	data[9]		=	0x00;
	data[10]	=	0x80;
	data[11]	=	0x00;
	data[12]	=	( xPos+24 );
	data[13]	=	0x00;
	data[14]	=	0x84;
	data[15]	=	0x00;
	
//set color to blue
	data[16]	=	0x1B;
	data[17]	=	'F';
	data[18]	=	'G';
	data[19]	=	0x07;
	data[20]	=	0x01;
	
	DisplaySendNBytes( data, 21 );
}

/** GRAPH SET COLOR BLUE *************************************
 *
 *	Function sets the graphs color to blue
 *
 *************************************************************/
void GraphSetColorBlue( void )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	'F';
	data[2]		=	'G';
	data[3]		=	0x01;
	data[4]		=	0x01;
	
	DisplaySendNBytes( data, 5 );
}	

/** GRAPH DRAW LINE INTEGRAL NEU NOVEMBER 2020 ***************
 *
 *	Function draws a line from the start of the peak to
 *	the end of the peak and displays the middle of the peak
 *	above the graph
 *
 ************************************************************/
void GraphDrawLineIntegral_neu( int start, int ende, int mitte, double p2y )
{
	int peakStart		=	0;
	int peakEnde		=	0;
	unsigned short y1	=	0;
	unsigned short y2	=	0;
	
	litoa( mitte, buffer );

//calc y-positions
	y1		=	( ( 200 - ( unsigned short ) ( ( p2y ) * 0.04395 ) ) + baselineConstant );
	y2		=	( ( 200 - ( unsigned short ) ( ( messdaten[ende] ) * 0.04395 ) ) + baselineConstant );
	
	if( y1 < 0 ) {
		y1	=	0;
	}	
	else if( y1 > 200 ) {
		y1	=	200;
	}
	if( y2 < 0 ) {
		y2	=	0;
	}
	else if ( y2 > 200 ) {
		y2	=	200;
	}	
	
//calc x-position
	if( meas6min == TRUE )
	{
		peakStart	=	floor( start * 0.67 );
		peakEnde	=	floor( ende * 0.67 );
		peakStart	=	peakStart + 24;
		peakEnde	=	peakEnde + 24;	
	}
	else
	{
		peakStart	=	floor( start * 0.33 );
		peakEnde	=	floor( ende * 0.33 );
		peakStart	=	peakStart + 24;
		peakEnde	=	peakEnde + 24;
	}		

//set color to red
	data[0]		=	0x1B;
	data[1]		=	'F';
	data[2]		=	'G';
	data[3]		=	0x03;	//red
	data[4]		=	0x01;

//draw line
	data[5]		=	0x1B;
	data[6]		=	'G';
	data[7]		=	'D';
	data[8]		=	( unsigned short ) peakStart;
	data[9]		=	0x00;
	data[10]	=	y1;
	data[11]	=	0x00;
	data[12]	=	( unsigned short ) peakEnde;
	data[13]	=	0x00;
	data[14]	=	y2;
	data[15]	=	0x00;

//set color to blue
	data[16]	=	0x1B;
	data[17]	=	'F';
	data[18]	=	'G';
	data[19]	=	0x07;	//blue
	data[20]	=	0x01;

//display middle of peak in seconds
	data[21]	=	0x1B;	//ESC
	data[22]	=	'Z';
	data[23]	=	'L';
	data[24]	=	peakStart;
	data[25]	=	0x00;
	data[26]	=	0x05;
	data[27]	=	0x00;
	if( mitte < 100 )
	{
		data[28]	=	' ';
		data[29]	=	' ';
		data[30]	=	buffer[0];
		data[31]	=	buffer[1];
	}	
	else if( mitte < 1000 )
	{
		data[28]	=	' ';
		data[29]	=	buffer[0];
		data[30]	=	buffer[1];
		data[31]	=	buffer[2];
	}	
	else
	{
		data[28]	=	buffer[0];
		data[29]	=	buffer[1];
		data[30]	=	buffer[2];
		data[31]	=	buffer[3];
	}	
	data[32]		=	0x00;

	DisplaySendNBytes( data, 33 );
}

/** GRAPH DRAW LINE INTEGRAL ********************************
 *
 *	Function draws a line from the start of the peak to
 *	the end of the peak and displays the middle of the peak
 *	above the graph
 *
 ************************************************************/
void GraphDrawLineIntegral( int start, int ende, int mitte )
{
	int peakStart		=	0;
	int peakEnde		=	0;
	unsigned short y1	=	0;
	unsigned short y2	=	0;
	
	litoa( mitte, buffer );

//calc y-positions
	y1		=	( ( 200 - ( unsigned short ) ( ( messdaten[start] ) * 0.04395 ) ) + baselineConstant );
	y2		=	( ( 200 - ( unsigned short ) ( ( messdaten[ende] ) * 0.04395 ) ) + baselineConstant );
	
	if( y1 < 0 ) {
		y1	=	0;
	}	
	else if( y1 > 200 ) {
		y1	=	200;
	}
	if( y2 < 0 ) {
		y2	=	0;
	}
	else if ( y2 > 200 ) {
		y2	=	200;
	}	
	
//calc x-position
	if( meas6min == TRUE )
	{
		peakStart	=	floor( start * 0.67 );
		peakEnde	=	floor( ende * 0.67 );
		peakStart	=	peakStart + 24;
		peakEnde	=	peakEnde + 24;	
	}
	else
	{
		peakStart	=	floor( start * 0.33 );
		peakEnde	=	floor( ende * 0.33 );
		peakStart	=	peakStart + 24;
		peakEnde	=	peakEnde + 24;
	}		

//set color to red
	data[0]		=	0x1B;
	data[1]		=	'F';
	data[2]		=	'G';
	data[3]		=	0x03;	//red
	data[4]		=	0x01;

//draw line
	data[5]		=	0x1B;
	data[6]		=	'G';
	data[7]		=	'D';
	data[8]		=	( unsigned short ) peakStart;
	data[9]		=	0x00;
	data[10]	=	y1;
	data[11]	=	0x00;
	data[12]	=	( unsigned short ) peakEnde;
	data[13]	=	0x00;
	data[14]	=	y2;
	data[15]	=	0x00;

//set color to blue
	data[16]	=	0x1B;
	data[17]	=	'F';
	data[18]	=	'G';
	data[19]	=	0x07;	//blue
	data[20]	=	0x01;

//display middle of peak in seconds
	data[21]	=	0x1B;	//ESC
	data[22]	=	'Z';
	data[23]	=	'L';
	data[24]	=	peakStart;
	data[25]	=	0x00;
	data[26]	=	0x05;
	data[27]	=	0x00;
	if( mitte < 100 )
	{
		data[28]	=	' ';
		data[29]	=	' ';
		data[30]	=	buffer[0];
		data[31]	=	buffer[1];
	}	
	else if( mitte < 1000 )
	{
		data[28]	=	' ';
		data[29]	=	buffer[0];
		data[30]	=	buffer[1];
		data[31]	=	buffer[2];
	}	
	else
	{
		data[28]	=	buffer[0];
		data[29]	=	buffer[1];
		data[30]	=	buffer[2];
		data[31]	=	buffer[3];
	}	
	data[32]		=	0x00;

	DisplaySendNBytes( data, 33 );
}

/** GRAPH SHOW PEAK *************************************
 *
 *	Function displays where the peak has been found
 *	in secondes
 *
 ********************************************************/
void GraphShowPeak( int start, int mitte )
{
	int peakStart	=	0;
	
	itoa( mitte, buffer );
	
//calc x-position
	if( meas6min == TRUE )
	{
		peakStart	=	floor( start * 0.67 );
		peakStart	=	peakStart + 24;
	}
	else
	{
		peakStart	=	floor( start * 0.33 );
		peakStart	=	peakStart + 24;
	}		
	
//display middle of peak in seconds
	data[0]		=	0x1B;	//ESC
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	peakStart;
	data[4]		=	0x00;
	data[5]		=	0x05;
	data[6]		=	0x00;
	if( mitte < 100 )
	{
		data[7]		=	' ';
		data[8]		=	' ';
		data[9]		=	buffer[0];
		data[10]	=	buffer[1];
	}	
	else if( mitte < 1000 )
	{
		data[7]		=	' ';
		data[8]		=	buffer[0];
		data[9]		=	buffer[1];
		data[10]	=	buffer[2];
	}	
	else
	{
		data[7]		=	buffer[0];
		data[8]		=	buffer[1];
		data[9]		=	buffer[2];
		data[10]	=	buffer[3];
	}	
	data[11]		=	0x00;
	
	DisplaySendNBytes( data, 12 );
}

/** GRAPH DELETE GRAPH **********************************
 *
 *	Function deletes the graph on the main screen
 *
 ********************************************************/
void GraphDeleteGraph( void )
{
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'L';
	data[3]		=	0x00;
	data[4]		=	0x00;
	data[5]		=	0x00;
	data[6]		=	0x00;
	data[7]		=	0x07;
	data[8]		=	0x01;
	data[9]		=	0xC8;
	data[10]	=	0x00;
	
	DisplaySendNBytes( data, 11 );
}	

/** GRAPH DRAW GRAPH *****************************************/
void GraphDrawGraph( void )
{
	int i				=	0;
	messpunkt_index		=	messpunkt_start;
	unsigned short x0	=	0x00;
	unsigned short x1	=	0x18;
	
	//FALL 1: KEINE MESSPUNKTE VORHANDEN
	if( messpunkt_aktuell == messpunkt_start )
	{
		return;
	}		
	
	//FALL 2: EIN ODER MEHRERE MESSPUNKTE VORHANDEN
	//Farbe
	data[0]		=	0x1B;	//ESC
	data[1]		=	'F';
	data[2]		=	'G';
	data[3]		=	0x07;
	data[4]		=	0x01;
	//erstes Geradenstück zeichnen
	data[5]		=	0x1B;	//ESC
	data[6]		=	0x47;	//G
	data[7]		=	0x44;	//D
	data[8]		=	x1;
	data[9]		=	x0;		//x1
	data[10]		=	*messpunkt_index;
	data[11]		=	0x00;	//y1
	data[12]		=	x1;
	data[13]		=	x0;		//x2
	data[14]		=	*messpunkt_index;
	data[15]	=	0x00;	//y2

	DisplaySendNBytes( data, 16 );
	
	messpunkt_index++;
	//NUR EIN MESSWERT VORHANDEN?
	if( messpunkt_index == messpunkt_aktuell )
	{
		return;
	}	

	//SONST: weiter zeichnen
	for( i = 0; i < 239; i++ )
	{
		x1++;

		data[0]		=	0x1B;	//ESC
		data[1]		=	0x47;	//G
		data[2]		=	0x57;	//W
		data[3]		=	x1;
		data[4]		=	x0;		//x1
		data[5]		=	*messpunkt_index;
		data[6]		=	0x00;	//y1

		if( x1 == 0xFF )
			{ x0 = 0x01; }

		DisplaySendNBytes( data, 7 );
		
		messpunkt_index++;
		//ABBRUCHKRITERIUM
		if( messpunkt_index == messpunkt_aktuell )
		{
			return;
		}	
	}
}

/** GRAPH REFRESH ************************************************/
void GraphRefresh( void )
{
	int arrayIndex		=	0;
	
	messpunkt_index		=	messpunkt_aktuell;
	messpunkt_index--;
	
	if( drawgraph == FALSE ) {  }
	else
	{
		data[0]		=	0x1B;
		data[1]		=	'F';
		data[2]		=	'G';
		data[3]		=	0x07;
		data[4]		=	0x01;
		
		DisplaySendNBytes( data, 5 );
		
		//FALL 1: ERSTER PUNKT DES GRAPHEN
		if( messpunkt_index == messpunkt_start )
		{
			data[0]		=	0x1B;	//ESC
			data[1]		=	0x47;	//G
			data[2]		=	0x44;	//D
			data[3]		=	0x18;
			data[4]		=	0x00;	//x1
			data[5]		=	*messpunkt_index;
			data[6]		=	0x00;	//y1
			data[7]		=	0x18;
			data[8]		=	0x00;	//x2
			data[9]		=	*messpunkt_index;
			data[10]	=	0x00;	//y2

			DisplaySendNBytes( data, 11 );
		}	
		//FALL 2: NEUER MESSPUNKT
		else
		{
			arrayIndex	=	messpunkt_index - messpunkt_start;
			
			data[0]		=	0x1B;
			data[1]		=	'G';
			data[2]		=	'W';
			if( arrayIndex + 24 < 256 )
			{
				data[3]	=	arrayIndex + 24;
			}	
			else
			{
				data[3]	=	arrayIndex - 256 + 24;
			}	
			if( arrayIndex + 24 < 256 )
			{
				data[4]	=	0x00;
			}	
			else
			{
				data[4]	=	0x01;
			}	
			data[5]		=	*messpunkt_index;
			data[6]		=	0x00;
			
			DisplaySendNBytes( data, 7 );
		}	
	}
}

/** DISPLAY: BUFFER INFO *****************************************************/
/*
 *	Function reads info from the display and returns the number of bytes
 *  that are in the displays send buffer
 */
char DisplayBufferInfo( void )
{
	unsigned char BytesInBuffer			=	0;

	unsigned char bcc		=	(0x12 + 0x01 + 0x49)%256;

	I2CStart();
	I2CSendByte( DISPLAY_ADDRW );			//displays write address
	I2CSendByte( 0x12 );					//<DC2>
	I2CSendByte( 0x01 );					//len
	I2CSendByte( 0x49 );					//'I'
	I2CSendByte( bcc );						//checksum

	I2CRepStart();
	I2CSendByte( DISPLAY_ADDRR );			//displays read address

	I2CReadByte( 0 );						//small protocol: ACK
	I2CReadByte( 0 );						//small protocol: Device Context
	I2CReadByte( 0 );						//small protocol: length
	BytesInBuffer	=	I2CReadByte( 0 );	//small protocol: bytes in send buffer
	I2CReadByte( 0 );						//small protocol: bytes free in receive buffer
	I2CReadByte( 1 );						//small protocol: bcc

	I2CStop();

	return BytesInBuffer;
}

/** DISPLAY GET BUFFER *******************************************/
/*
 *	Function collects all data from the displays send buffer
 *	and calls the DisplaysRequest()-Function if O-instructions
 *	are found (Oxxx - Code that display sends out; xxx = 3 numbers)
 */
void DisplayGetBuffer( void )
{
	char BytesInBuffer	=	DisplayBufferInfo();

	unsigned char displayBuffer[256];
	unsigned char c2, c3, c4;				//chars that store the request code
	unsigned short i = 0;

	unsigned char bcc		=	(0x12 + 0x01 + 0x53)%256;

	I2CStart();
	I2CSendByte( DISPLAY_ADDRW );			//address
	I2CSendByte( 0x12 );					//<DC2>
	I2CSendByte( 0x01 );					//len
	I2CSendByte( 0x53 );					//'S'
	I2CSendByte( bcc );						//checksum

	I2CRepStart();
	I2CSendByte( DISPLAY_ADDRR );			//address

	I2CReadByte( 0 );						//ACK
	I2CReadByte( 0 );						//DC1
	I2CReadByte( 0 );						//length = BytesInBuffer

	//Loop that collects all the data from the displays send buffer
	for( i = 0; i < BytesInBuffer; i++ )
	{
		displayBuffer[i]		=	I2CReadByte( 0 );
	}

	I2CReadByte( 1 );						//BCC

	I2CStop();

	//check if the first char is 'O'
	//if so, the following three chars are numbers
	//that indicate which request is called
	for( i = 0; i < ( ( int ) ( BytesInBuffer/4 ) ); i+=4 )
	{
		if( displayBuffer[i] == 0x4F ) 
		{ 
			c2					=	displayBuffer[i+1];
			c3					=	displayBuffer[i+2];
			c4					=	displayBuffer[i+3];

			DisplayRequest( c2, c3, c4 );
		}
	}
}

/** DISPLAY SEND N BYTES TO DISPLAY ******************************/
/*
 *	Function gets a char-array and an int and sends instructions
 *	to the display
 */
void DisplaySendNBytes( char* data, unsigned short length )
{
	char bcc			=	0;
	int i				=	0;

	I2CStart();
	I2CSendByte( DISPLAY_ADDRW );			//displays write address
	I2CSendByte( 0x11 );					//device context
	bcc					+=	0x11;
	I2CSendByte( ( char ) length );			//length
	bcc					+=	( char ) length;

	for( i = 0; i < length; i++ )
	{
		I2CSendByte( data[i] );
		bcc				+=	data[i];
	}

	I2CSendByte( bcc );

	I2CRepStart();
	I2CSendByte( DISPLAY_ADDRR );

	I2CReadByte( 1 );

	I2CStop();
}

/** DISPLAY LOCK SYSTEM **********************************************/
void DisplayLockSystem( void )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	'M';
	data[2]		=	'X';
	data[3]		=	0x80;
	
	DisplaySendNBytes( data, 4 );
}

/** DISPLAY SET LANGUAGE **********************************************/
void DisplaySetLanguage( char lang )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	'M';
	data[2]		=	'K';
	data[3]		=	lang;
	
	DisplaySendNBytes( data, 4 );
}

/** DISPLAY RESTART DISPLAY **********************************************/
void DisplayRestartDisplay( void )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	'M';
	data[2]		=	'N';
	data[3]		=	0x04;
	
	DisplaySendNBytes( data, 4 );
}	

/** DISPLAY REQUEST **********************************************/
void DisplayRequest( char c2, char c3, char c4 )
{
	if( c2 == 0x30 )
	{
		if( c3 == 0x30 )
		{	
			if( c4 == 0x30 )
			{
//REQUEST 000: Unlock System
				malfunction			=	FALSE;
				malfunctionCount	=	0;
				lockSystem			=	FALSE;
				MemoryGetAutomessung();
				if( autoMeasurement )
				{
					DisplayRequest( '4', '0', '1' );
				}	
			}	
			else if( c4 == 0x31 )
			{
//REQUEST 001: Anzeige Hauptbildschirm
				drawgraph		=	TRUE;

				DisplayShowPeakschranke();
				GraphDrawGraph();

				DisplayShowSekunden();
				DisplayShowSignal();
				DisplayShowAkku();
				DisplayShowDruck( 0 );
				DisplayShowTemperatur( 0 );
				DisplayShowFluss( 0 );
				DisplayShowStartzeit();
				DisplayShowMessergebnis();
			}
		}
	}
	else if( c2 == 0x31 )
	{
		if( c3 == 0x30 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 100: Messort, Messortcursor und Eichgasmenge anzeigen
				drawgraph		=	FALSE;

				MemoryGetMessort();
				DisplayShowMessort();
				DisplayShowEichgas( 0 );
				DisplayShowMessortCursor();
			}
			else if( c4 == 0x31 )
			{
//REQUEST 101: Nummer an Stelle erhöhen
				if( messort[messortindex] == 0x2D )
				{
					messort[messortindex]	=	0x30;
				}
				else if( messort[messortindex] == 0x39 )
				{
					messort[messortindex]	=	0x2D;
				}
				else
				{
					messort[messortindex]	+=	1;
				}
				DisplayShowMessort();
			}
			else if( c4 == 0x32 )
			{
//REQUEST 102: Nummer an Stelle verringern
				if( messort[messortindex] == 0x30 )
				{
					messort[messortindex]	=	0x2D;
				}
				else if( messort[messortindex] == 0x2D )
				{
					messort[messortindex]	=	0x39;
				}
				else
				{
					messort[messortindex]	-=	1;
				}
				DisplayShowMessort();
			}
			else if( c4 == 0x33 )
			{
//REQUEST 103: Cursor nach links
				if( messortindex > 0 )
				{
					messortindex--;
					DisplayShowMessortCursor();
				}
			}
			else if( c4 == 0x34 )
			{
//REQUEST 104: Cursor nach rechts
				if( messortindex < 7 )
				{
					messortindex++;
					DisplayShowMessortCursor();
				}
			}
			else if( c4 == 0x35 )
			{
//REQUEST 105: Messortdaten speichern
				MemorySetMessort();
				DisplayShowSpeicherung();
				DisplayShowMessort();
				DisplayShowEichgas( 0 );
				DisplayShowMessortCursor();
			}
			else if( c4 == 0x36 )
			{
//REQUEST 106: Messort und Eichgasmenge anzeigen
				eichgas_plus			=	0;
				eichgas_minus			=	0;
				
				MemoryGetMessort();
				DisplayShowMessort();
				DisplayShowEichgas( 0 );
			}	
			else if( c4 == 0x37 )
			{
//REQUEST 107: Eichgasmenge erhöhen
				eichgas_minus		=	0;
				if( eichgasmenge == 1000 ) {  }
				else if( ( eichgas_plus < 5 ) && ( eichgasmenge < 1000 ) )
				{
					eichgasmenge		+=	1;
					eichgas_plus		+=	1;
					DisplayShowEichgas( 0 );
				}
				else if( ( eichgas_plus < 10 ) && ( eichgasmenge < 996 ) )
				{
					eichgasmenge		+=	5;
					eichgas_plus		+=	1;
					DisplayShowEichgas( 0 );
				}
				else if( ( eichgas_plus < 10 ) && ( eichgasmenge > 995 ) )
				{
					eichgasmenge		=	1000;
					DisplayShowEichgas( 0 );
				}
				else if( ( eichgas_plus >= 10 ) && ( eichgasmenge < 991 ) )
				{
					eichgasmenge 		+=	10;
					DisplayShowEichgas( 0 );
				}
				else if( ( eichgas_plus >= 10 ) && ( eichgasmenge > 990 ) )
				{
					eichgasmenge		=	1000;
					DisplayShowEichgas( 0 );
				}
			}
			else if( c4 == 0x38 )
			{
//REQUEST 108: Eichgasmenge verringern
				eichgas_plus		=	0;
				if( eichgasmenge == 30 ) {  }
				else if( ( eichgas_minus < 5 ) && ( eichgasmenge > 30 ) )
				{
					eichgasmenge		-=	1;
					eichgas_minus		+=	1;
					DisplayShowEichgas( 0 );
				}
				else if( ( eichgas_minus < 10 ) && ( eichgasmenge > 34 ) )
				{
					eichgasmenge		-=	5;
					eichgas_minus		+=	1;
					DisplayShowEichgas( 0 );
				}
				else if( ( eichgas_minus < 10 ) && ( eichgasmenge < 35 ) )
				{
					eichgasmenge		=	30;
					DisplayShowEichgas( 0 );
				}
				else if( ( eichgas_minus >= 10 ) && ( eichgasmenge > 39 ) )
				{
					eichgasmenge		-=	10;
					DisplayShowEichgas( 0 );
				}
				else if( ( eichgas_minus >= 10 ) && ( eichgasmenge < 40 ) )
				{
					eichgasmenge		=	0;
					DisplayShowEichgas( 0 );
				}
			}
			else if( c4 == 0x39 )
			{
//REQUEST 109: Eichgasmenge speichern
				MemorySetEichgasmenge();
				DisplayShowMessort();
				DisplayShowEichgas( 0 );
				DisplayShowMessortCursor();
				DisplayShowSpeicherung();
			}			
		}
		else if( c3 == 0x31 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 110: Spülen
				SystemClearEndFlag();
				SystemStartSpuelen();
				displayWait		=	0;
				while( displayWait < 20 ) { Nop(); }
				SystemStatus();
				while( ( system_status & DEV_CALGAS ) > 0 )
				{
					displayWait		=	0;
					while( displayWait < 2 ) { Nop(); }
					SystemStatus();
				}	
				
				DisplayShowMain();
			}	
		}	
	}
	else if( c2 == 0x32 )
	{
		if( c3 == 0x30 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 200: Alle Betriebsparameter anzeigen
				drawgraph		=	FALSE;

				eichgas_plus			=	0;
				eichgas_minus			=	0;
				menge_plus				=	0;
				menge_minus				=	0;
				fluss_plus				=	0;
				fluss_minus				=	0;
				druck_plus				=	0;
				druck_minus				=	0;
				spuelzeit_plus			=	0;
				spuelzeit_minus			=	0;
				skalierung_plus			=	0;
				skalierung_minus		=	0;
				korrektur_plus			=	0;
				korrektur_minus			=	0;
				splitzeit_plus			=	0;
				splitzeit_minus			=	0;
				datecursor				=	0;
				timecursor				=	0;
				
				MemoryGetEichgas();
				DisplayShowEichgas( 1 );	
				MemoryGetMenge();
				DisplayShowMenge();
				MemoryGetVolumen();
				DisplayShowVolumen();
				DisplayShowDatum( 0 );
				DisplayShowUhrzeit( 1, 0 );
				MemoryGetTemperatur();
				DisplayShowTemperatur( 1 );
				saeule_vorgabe		=	temperatur;
				MemoryGetFluss();
				DisplayShowFluss( 1 );	
				fluss_vorgabe			=	fluss;
				MemoryGetUebertragungsart();
				DisplayShowUebertragungsart();	
				MemoryGetSkalierung();
				DisplayShowSkalierung();
				
				displayWait		=	0;
				while( displayWait < 2 );
				
				MemoryGetDruck();
				DisplayShowDruck( 1 );
				vordruck_vorgabe		=	druck;
				MemoryGetEmpfindlichkeit();
				DisplayShowEmpfindlichkeit();
				empfindlichkeit_vorgabe	=	empfindlichkeit;
				MemoryGetMesszyklus();
				DisplayShowMesszyklus();
				MemoryGetEichzyklus();
				DisplayShowEichzyklus();
				MemoryGetSpuelzeit();
				DisplayShowSpuelzeit();
				MemoryGetKorrekturfaktor();
				DisplayShowKorrekturfaktor();
				MemoryGetAutomessung();
				DisplayShowAutomessung();
				MemoryGetSplitzeit();
				DisplayShowSplitzeit();
				
				SystemVordruckVorgabe();
				SystemSaeuleVorgabe();
				saeule_vorgabe			/=	10;
				SystemEmpfindlichkeitVorgabe();
				SystemFlussVorgabe();
			}
			else if( c4 == 0x31 )
			{
//REQUEST 201: Eichgasmenge erhöhen
				eichgas_minus		=	0;
				if( eichgasmenge == 1000 ) {  }
				else if( ( eichgas_plus < 5 ) && ( eichgasmenge < 1000 ) )
				{
					eichgasmenge		+=	1;
					eichgas_plus		+=	1;
					DisplayShowEichgas( 1 );
				}
				else if( ( eichgas_plus < 10 ) && ( eichgasmenge < 996 ) )
				{
					eichgasmenge		+=	5;
					eichgas_plus		+=	1;
					DisplayShowEichgas( 1 );
				}
				else if( ( eichgas_plus < 10 ) && ( eichgasmenge > 995 ) )
				{
					eichgasmenge		=	1000;
					DisplayShowEichgas( 1 );
				}
				else if( ( eichgas_plus >= 10 ) && ( eichgasmenge < 991 ) )
				{
					eichgasmenge 		+=	10;
					DisplayShowEichgas( 1 );
				}
				else if( ( eichgas_plus >= 10 ) && ( eichgasmenge > 990 ) )
				{
					eichgasmenge		=	1000;
					DisplayShowEichgas( 1 );
				}
			}
			else if( c4 == 0x32 )
			{
//REQUEST 202: Eichgasmenge verringern
				eichgas_plus		=	0;
				if( eichgasmenge == 30 ) {  }
				else if( ( eichgas_minus < 5 ) && ( eichgasmenge > 30 ) )
				{
					eichgasmenge		-=	1;
					eichgas_minus		+=	1;
					DisplayShowEichgas( 1 );
				}
				else if( ( eichgas_minus < 10 ) && ( eichgasmenge > 34 ) )
				{
					eichgasmenge		-=	5;
					eichgas_minus		+=	1;
					DisplayShowEichgas( 1 );
				}
				else if( ( eichgas_minus < 10 ) && ( eichgasmenge < 35 ) )
				{
					eichgasmenge		=	30;
					DisplayShowEichgas( 1 );
				}
				else if( ( eichgas_minus >= 10 ) && ( eichgasmenge > 39 ) )
				{
					eichgasmenge		-=	10;
					DisplayShowEichgas( 1 );
				}
				else if( ( eichgas_minus >= 10 ) && ( eichgasmenge < 40 ) )
				{
					eichgasmenge		=	30;
					DisplayShowEichgas( 1 );
				}
			}
			else if( c4 == 0x33 )
			{
//REQUEST 203: Eichgasmenge speichern
				MemorySetEichgasmenge();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x36 )
			{
//REQUEST 206: Menge erhöhen
				menge_minus		=	0;
				if( menge == 1000 ) {  }
				else if( ( menge_plus < 5 ) && ( menge < 1000 ) )
				{
					menge			+=	1;
					menge_plus		+=	1;
					DisplayShowMenge();
				}
				else if( ( menge_plus < 10 ) && ( menge < 996 ) )
				{
					menge			+=	5;
					menge_plus		+=	1;
					DisplayShowMenge();
				}
				else if( ( menge_plus < 10 ) && ( menge > 995 ) )
				{
					menge			=	1000;
					DisplayShowMenge();
				}
				else if( ( menge_plus < 15 ) && ( menge < 991 ) )
				{
					menge	 		+=	10;
					menge_plus		+=	1;
					DisplayShowMenge();
				}
				else if( ( menge_plus < 15 ) && ( menge > 990 ) )
				{
					menge			=	1000;
					DisplayShowMenge();
				}
				else if( ( menge_plus >= 15 ) && ( menge < 951 ) )
				{
					menge			+=	50;
					menge_plus		+=	1;
					DisplayShowMenge();
				}
				else if( ( menge_plus >= 15 ) && ( menge > 950 ) )
				{
					menge			=	1000;
					DisplayShowMenge();
				}
			}
			else if( c4 == 0x37 )
			{
//REQUEST 207: Menge verringern
				menge_plus		=	0;
				if( menge == 100 ) {  }
				else if( ( menge_minus < 5 ) && ( menge > 100 ) )
				{
					menge			-=	1;
					menge_minus		+=	1;
					DisplayShowMenge();
				}
				else if( ( menge_minus < 10 ) && ( menge > 104 ) )
				{
					menge			-=	5;
					menge_minus		+=	1;
					DisplayShowMenge();
				}
				else if( ( menge_minus < 10 ) && ( menge < 105 ) )
				{
					menge			=	100;
					DisplayShowMenge();
				}
				else if( ( menge_minus < 15 ) && ( menge > 109 ) )
				{
					menge	 		-=	10;
					menge_minus		+=	1;
					DisplayShowMenge();
				}
				else if( ( menge_minus < 15 ) && ( menge < 110 ) )
				{
					menge			=	100;
					DisplayShowMenge();
				}
				else if( ( menge_minus >= 15 ) && ( menge > 149 ) )
				{
					menge			-=	50;
					menge_minus		+=	1;
					DisplayShowMenge();
				}
				else if( ( menge_minus >= 15 ) && ( menge < 150 ) )
				{
					menge			=	100;
					DisplayShowMenge();
				}
			}
			else if( c4 == 0x38 )
			{
//REQUEST 208: Menge speichern
				MemorySetMenge();
				DisplayShowSpeicherung();
			}
		}
		else if( c3 == 0x31 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 210: Volumen speichern
				MemorySetVolumen();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x31 )
			{
//REQUEST 211: Volumen erhöhen
				volumen_minus		=	0;
				if( volumen == 150 ) {  }
				else if( ( volumen_plus < 5 ) && ( volumen < 150 ) )
				{
					volumen			+=	1;
					volumen_plus	+=	1;
					DisplayShowVolumen();
				}
				else if( ( volumen_plus < 10 ) && ( volumen < 146 ) )
				{
					volumen			+=	5;
					volumen_plus	+=	1;
					DisplayShowVolumen();
				}
				else if( ( volumen_plus < 10 ) && ( volumen > 145 ) )
				{
					volumen			=	150;
					DisplayShowVolumen();
				}
				else if( ( volumen_plus >= 10 ) && ( volumen < 141 ) )
				{
					volumen	 		+=	10;
					volumen_plus	+=	1;
					DisplayShowVolumen();
				}
				else if( ( volumen_plus >= 10 ) && ( volumen > 140 ) )
				{
					volumen			=	150;
					DisplayShowVolumen();
				}
			}
			else if( c4 == 0x32 )
			{
//REQUEST 212: Volumen verringern
				volumen_plus		=	0;
				if( volumen <= 10 ) {  }
				else if( ( volumen_minus < 5 ) && ( volumen > 10 ) )
				{
					volumen			-=	1;
					volumen_minus	+=	1;
					DisplayShowVolumen();
				}
				else if( ( volumen_minus < 10 ) && ( volumen > 14 ) )
				{
					volumen			-=	5;
					volumen_minus	+=	1;
					DisplayShowVolumen();
				}
				else if( ( volumen_minus < 10 ) && ( volumen < 15 ) )
				{
					volumen			=	10;
					DisplayShowVolumen();
				}
				else if( ( volumen_minus >= 10 ) && ( volumen > 19 ) )
				{
					volumen	 		-=	10;
					volumen_minus	+=	1;
					DisplayShowVolumen();
				}
				else if( ( volumen_minus >= 10 ) && ( volumen < 20 ) )
				{
					volumen			=	10;
					DisplayShowVolumen();
				}
			}
			else if( c4 == 0x33 )
			{
//REQUEST 213: Datum erhöhen
				if( datecursor == 0 ) {  }
				else if( datecursor == 1 )
				{
					if( day1 != 3 && day2 < 9 )
					{
						day2++;
					}
					else if( day1 != 3 && day2 == 9 )
					{
						day1++;
						day2	=	0;
					}
					else if( day1 == 3 && day2 == 0 )
					{
						day2++;
					}
					else if( day1 == 3 && day2 == 1 )
					{
						day1	=	0;
						day2	=	1;
					}
					DisplayShowDatum( 1 );
                   

				}
				else if( datecursor == 2 )
				{
					if( month1 != 1	&&	month2 < 9 )
					{
						month2++;
					}
					else if( month1 != 1 && month2 == 9 )
					{
						month1++;
						month2	=	0;
					}
					else if( month1 == 1 && month2 < 2 )
					{
						month2++;
					}
					else if( month1 == 1 && month2 == 2 )
					{
						month1	=	0;
						month2	=	1;
					}
					DisplayShowDatum( 1 );
				}
				else if( datecursor == 3 )
				{
					if( year1 != 9 && year2 < 9 )
					{
						year2++;
					}
					else if( year1 != 9 && year2 == 9 )
					{
						year1++;
						year2	=	0;
					}
					else if( year1 == 9 && year2 < 9 )
					{
						year2++;
					}
					else if( year1 == 9 && year2 == 9 )
					{
						year1	=	0;
						year2	=	0;
					}
					DisplayShowDatum( 1 );
                    
				}
			}
			else if( c4 == 0x34 )
			{
//REQUEST 214: Datumsstelle ändern
				if( datecursor == 3 )
				{
					datecursor		=	1;
				}
				else
				{
					datecursor		+=	1;
				}
				DisplayShowDateCursor();
			}
			else if( c4 == 0x35 )
			{
//REQUEST 215: Datum speichern
				day		=	day1;
				day		=	day << 4;
				day		+=	day2;
				month	=	month1;
				month	=	month << 4;
				month	+=	month2;
				year	=	year1;
				year	=	year << 4;
				year	+=	year2;

				ClockInit();
				ClockReadHours();
				ClockReadMinutes();
				ClockSetClock();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x36 )
			{
//REQUEST 216: Temperatur erhöhen
				if( saeule_vorgabe == 60 ) {  }
				else if( saeule_vorgabe < 60 )
				{
					saeule_vorgabe++;
					DisplayShowTemperatur( 4 );
				}
			}
			else if( c4 == 0x37 )
			{
//REQUEST 217: Temperatur verringern
				if( saeule_vorgabe == 20 ) {  }
				else if( saeule_vorgabe > 20 )
				{
					saeule_vorgabe--;
					DisplayShowTemperatur( 4 );
				}
			}
			else if( c4 == 0x38 )
			{
//REQUEST 218: Uhrzeit erhöhen
				if( autoMeasurement )
				{
					DisplayShowUhrBlockiert();
				}
				else
				{
					if( timecursor == 0 ) {  }
					else if( timecursor == 1 )
					{
						if( hrs1 != 2 && hrs2 < 9 )
						{
							hrs2++;
						}
						else if( hrs1 != 2 && hrs2 == 9 )
						{
							hrs1++;
							hrs2	=	0;
						}
						else if( hrs1 == 2 && hrs2 < 3 )
						{
							hrs2++;
						}
						else if( hrs1 == 2 && hrs2 == 3 )
						{
							hrs1	=	0;
							hrs2	=	0;
						}
                        
                        DisplayShowDatum(0);
//qu						DisplayShowUhrzeit( 1, 0 );
                        DisplayShowUhrzeit( 1, 1 );
					}
					else if( timecursor == 2 )
					{
						if( min1 != 5	&&	min2 < 9 )
						{
							min2++;
						}
						else if( min1 != 5 && min2 == 9 )
						{
							min1++;
							min2	=	0;
						}
						else if( min1 == 5 && min2 < 9 )
						{
							min2++;
						}
						else if( min1 == 5 && min2 == 9 )
						{
							min1	=	0;
							min2	=	0;
						}
                      
					    DisplayShowDatum(0);
//qu                        DisplayShowUhrzeit( 1, 0 );
                        DisplayShowUhrzeit( 1, 1 );
					}
				}	
			}
			else if( c4 == 0x39 )
			{
//REQUEST 219: Uhrzeitstelle verändern
				if( timecursor == 2 )
				{
					timecursor		=	1;
				}
				else
				{
					timecursor		+=	1;
				}
                
				DisplayShowDatum(0);
//qu                DisplayShowUhrzeit( 1, 0 );
				DisplayShowUhrzeit( 1, 1 ); //qu
                DisplayShowTimeCursor();
			}
		}
		else if( c3 == 0x32 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 220: Uhrzeit speichern
				minutes		=	min1;
				minutes		=	minutes << 4;
				minutes		+=	min2;
				hours		=	hrs1;
				hours		=	hours << 4;
				hours		+=	hrs2;

				ClockInit();
				ClockReadDay();
				ClockReadMonth();
				ClockReadYear();
				ClockSetClock();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x31 )
			{
//REQUEST 221: Druck erhöhen
				druck_minus		=	0;
				if( vordruck_vorgabe == 70 ) {  }
				else if( ( druck_plus < 5 ) && ( vordruck_vorgabe < 70 ) )
				{
					vordruck_vorgabe		+=	1;
					druck_plus				+=	1;
					DisplayShowDruck( 2 );
				}
				else if( ( druck_plus >= 5 ) && ( vordruck_vorgabe < 66 ) )
				{
					vordruck_vorgabe		+=	5;
					druck_plus				+=	1;
					DisplayShowDruck( 2 );
				}
				else if( ( druck_plus >= 5 ) && ( vordruck_vorgabe > 65 ) )
				{
					vordruck_vorgabe		=	70;
					DisplayShowDruck( 2 );
				}
			}
			else if( c4 == 0x32 )
			{
//REQUEST 222: Druck verringern
				druck_plus		=	0;
				if( vordruck_vorgabe == 15 ) {  }
				else if( ( druck_minus < 5 ) && ( vordruck_vorgabe > 15 ) )
				{
					vordruck_vorgabe		-=	1;
					druck_minus				+=	1;
					DisplayShowDruck( 2 );
				}
				else if( ( druck_minus >= 5 ) && ( vordruck_vorgabe > 19 ) )
				{
					vordruck_vorgabe		-=	5;
					druck_minus				+=	1;
					DisplayShowDruck( 2 );
				}
				else if( ( druck_minus >= 5 ) && ( vordruck_vorgabe < 20 ) )
				{
					vordruck_vorgabe		=	15;
					DisplayShowDruck( 2 );
				}
			}
			else if( c4 == 0x33 )
			{
//REQUEST 223: Druck speichern
				MemorySetDruck();
				SystemSetVordruck();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x34 )
			{
//REQUEST 224: Temperatur speichern
				MemorySetTemperatur();
				SystemSetSaeule();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x36 )
			{
//REQUEST 226: Empfindlichkeit erhöhen
				if( empfindlichkeit_vorgabe == 9 ) {  }
				else if( empfindlichkeit_vorgabe < 9 )
				{
					empfindlichkeit_vorgabe++;
					DisplayShowEmpfindlichkeit();
				}
			}
			else if( c4 == 0x37 )
			{
//REQUEST 227: Empfindlichkeit verringern
				if( empfindlichkeit_vorgabe == 0 ) {  }
				else if( empfindlichkeit_vorgabe > 0 )
				{
					empfindlichkeit_vorgabe--;
					DisplayShowEmpfindlichkeit();
				}
			}
			else if( c4 == 0x38 )
			{
//REQUEST 228: Empfindlichkeit speichern
				MemorySetEmpfindlichkeit();
				SystemSetEmpfindlichkeit();
				DisplayShowSpeicherung();
			}
		}
		else if( c3 == 0x33 )
		{
			if( c4 == 0x31 )
			{
//REQUEST 231: Messzyklus vergrößern
				if( messzyklus == 20 )
				{
					messzyklus		=	40;
					DisplayShowMesszyklus();
					eichzyklus		=	1;
					DisplayShowEichzyklus();
				}
				else if( messzyklus == 40 )
				{
					messzyklus		=	1;
					DisplayShowMesszyklus();
					eichzyklus		=	messzyklus;
					DisplayShowEichzyklus();
				}
				else if( messzyklus >= 1 && messzyklus < 19 )
				{
					messzyklus		+=	1;
					DisplayShowMesszyklus();
					eichzyklus		=	messzyklus;
					DisplayShowEichzyklus();
				}
				else if( messzyklus == 19 )
				{
					messzyklus		=	21;
					DisplayShowMesszyklus();
					eichzyklus		=	messzyklus;
					DisplayShowEichzyklus();
				}
				else if( messzyklus >= 21 && messzyklus	<= 24 )
				{
					messzyklus		+=	1;
					DisplayShowMesszyklus();
					eichzyklus		=	messzyklus;
					DisplayShowEichzyklus();
				}	
			}
			else if( c4 == 0x32 )
			{
//REQUEST 232: Messzyklus verringern
				if( messzyklus == 40 )
				{
					messzyklus		=	20;
					DisplayShowMesszyklus();
					eichzyklus		=	1;
					DisplayShowEichzyklus();
				}
				else if( messzyklus == 1 )
				{
					messzyklus		=	40;
					DisplayShowMesszyklus();
					eichzyklus		=	1;
					DisplayShowEichzyklus();
				}
				else if( messzyklus >= 2 && messzyklus <= 19 )
				{
					messzyklus		-=	1;
					DisplayShowMesszyklus();
					eichzyklus		=	messzyklus;
					DisplayShowEichzyklus();
				}
				else if( messzyklus == 21 )
				{
					messzyklus		=	19;
					DisplayShowMesszyklus();
					eichzyklus		=	messzyklus;
					DisplayShowEichzyklus();
				}	
				else if( messzyklus >= 22 && messzyklus <= 25 )
				{
					messzyklus		-=	1;
					DisplayShowMesszyklus();
					eichzyklus		=	messzyklus;
					DisplayShowEichzyklus();
				}	
			}
			else if( c4 == 0x33 )
			{
//REQUEST 233: Messzyklus speichern
				MemorySetMesszyklus();
				
				displayWait		=	0;
				while( displayWait < 2 );
				
				MemorySetEichzyklus();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x36 )
			{
//REQUEST 236: Eichzyklus vergrößern
				if( eichzyklus == 20 )
				{
					eichzyklus		=	40;
					DisplayShowEichzyklus();
				}
				else if( eichzyklus == 40 )
				{
					eichzyklus		=	1;
					DisplayShowEichzyklus();
				}
				else if( eichzyklus >= 1 && eichzyklus < 19 )
				{
					eichzyklus		+=	1;
					DisplayShowEichzyklus();
				}
				else if( eichzyklus == 19 )
				{
					eichzyklus		=	21;
					DisplayShowEichzyklus();
				}
				else if( eichzyklus >= 21 && eichzyklus	<= 24 )
				{
					eichzyklus		+=	1;
					DisplayShowEichzyklus();
				}	
			}
			else if( c4 == 0x37 )
			{
//REQUEST 237: Eichzyklus verkleinern
				if(( messzyklus == 20 ) ||
				( messzyklus == 40 && eichzyklus >= 1 && eichzyklus != 20 && eichzyklus != 40 ) ||
				( eichzyklus > messzyklus ))
				{
					if( eichzyklus == 40 )
					{
						eichzyklus		=	20;
						DisplayShowEichzyklus();
					}
					else if( eichzyklus == 1 )
					{
						eichzyklus		=	40;
						DisplayShowEichzyklus();
					}
					else if( eichzyklus >= 2 && eichzyklus <= 19 )
					{
						eichzyklus		-=	1;
						DisplayShowEichzyklus();
					}
					else if( eichzyklus == 21 )
					{
						eichzyklus		=	19;
						DisplayShowEichzyklus();
					}	
					else if( eichzyklus >= 22 && eichzyklus <= 25 )
					{
						eichzyklus		-=	1;
						DisplayShowEichzyklus();
					}
				}	
			}
			else if( c4 == 0x38 )
			{
//REQUEST 238: Eichzyklus speichern
				MemorySetEichzyklus();
				DisplayShowSpeicherung();
			}
		}
		else if( c3 == 0x34 )
		{
			if( c4 == 0x31 )
			{
//REQUEST 241: Spuelzeit vergrößern
				spuelzeit_minus		=	0;
				if( spuelzeit == 60 ) {  }
				else if( ( spuelzeit_plus < 5 ) && ( spuelzeit < 60 ) )
				{
					spuelzeit		+=	1;
					spuelzeit_plus	+=	1;
					DisplayShowSpuelzeit();
				}
				else if( ( spuelzeit_plus >= 5 ) && ( spuelzeit < 56 ) )
				{
					spuelzeit		+=	5;
					spuelzeit_plus	+=	1;
					DisplayShowSpuelzeit();
				}
				else if( ( spuelzeit >= 5 ) && ( spuelzeit > 55 ) )
				{
					spuelzeit		=	60;
					DisplayShowSpuelzeit();
				}
			}
			else if( c4 == 0x32 )
			{
//REQUEST 242: Spuelzeit verringern
				spuelzeit_plus		=	0;
				if( spuelzeit == 15 ) {  }
				else if( ( spuelzeit_minus < 5 ) && ( spuelzeit > 15 ) )
				{
					spuelzeit		-=	1;
					spuelzeit_minus	+=	1;
					DisplayShowSpuelzeit();
				}
				else if( ( spuelzeit_minus >= 5 ) && ( spuelzeit > 19 ) )
				{
					spuelzeit		-=	5;
					spuelzeit_minus	+=	1;
					DisplayShowSpuelzeit();
				}
				else if( ( spuelzeit_minus >= 5 ) && ( spuelzeit < 20 ) )
				{
					spuelzeit		=	15;
					DisplayShowSpuelzeit();
				}
			}
			else if( c4 == 0x33 )
			{
//REQUEST 243:  Spuelzeit speichern
				MemorySetSpuelzeit();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x36 )
			{
//REQUEST 246: Korrekturfaktor erhöhen
				korrektur_minus			=	0;
				if( korrekturfaktor == 135 ) {  }
				else if( ( korrektur_plus < 5 ) && ( korrekturfaktor < 135 ) )
				{
					korrekturfaktor		+=	1;
					korrektur_plus		+=	1;
					DisplayShowKorrekturfaktor();
				}
				else if( ( korrektur_plus >= 5 ) && ( korrekturfaktor < 131 ) )
				{
					korrekturfaktor		+=	5;
					DisplayShowKorrekturfaktor();
				}
				else if( ( korrektur_plus >= 5 ) && ( korrekturfaktor > 130 ) )
				{
					korrekturfaktor		=	135;
					DisplayShowKorrekturfaktor();
				}
			}
			else if( c4 == 0x37 )
			{
//REQUEST 247: Korrekturfaktor verringern
				korrektur_plus			=	0;
				if( korrekturfaktor == 50 ) {  }
				else if( ( korrektur_minus < 5 ) && ( korrekturfaktor > 50 ) )
				{
					korrekturfaktor		-=	1;
					korrektur_minus		+=	1;
					DisplayShowKorrekturfaktor();
				}
				else if( ( korrektur_minus >= 5 ) && ( korrekturfaktor > 54 ) )
				{
					korrekturfaktor		-=	5;
					DisplayShowKorrekturfaktor();
				}
				else if( ( korrektur_minus >= 5 ) && ( korrekturfaktor < 55 ) )
				{
					korrekturfaktor		=	50;
					DisplayShowKorrekturfaktor();
				}
			}
			else if( c4 == 0x38 )
			{
//REQUEST 248: Korrekturfaktor speichern
				MemorySetKorrekturfaktor();
				DisplayShowSpeicherung();
			}
		}
		else if( c3 == 0x35 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 250: Fluss erhöhen
				fluss_minus			=	0;
				if( fluss_vorgabe == 200 ) {  }
				else if( ( fluss_plus < 5 ) && ( fluss_vorgabe < 200 ) )
				{
					fluss_vorgabe			+=	1;
					fluss_plus				+=	1;
					DisplayShowFluss( 1 );
				}
				else if( ( fluss_plus >= 5 ) && ( fluss_vorgabe <196 ) )
				{
					fluss_vorgabe			+=	5;
					DisplayShowFluss( 1 );
				}
				else if( ( fluss_plus >= 5 ) && ( fluss_vorgabe > 195 ) )
				{
					fluss_vorgabe			=	200;
					DisplayShowFluss( 1 );
				}
			}
			else if( c4 == 0x31 )
			{
//REQUEST 251: Fluss verringern
				fluss_plus			=	0;
				if( fluss_vorgabe == 10 ) {  }
				else if( ( fluss_minus < 5 ) && ( fluss_vorgabe > 10 ) )
				{
					fluss_vorgabe			-=	1;
					fluss_minus				+=	1;
					DisplayShowFluss( 1 );
				}
				else if( ( fluss_minus >= 5 ) && ( fluss_vorgabe > 14 ) )
				{
					fluss_vorgabe			-=	5;
					DisplayShowFluss( 1 );
				}
				else if( ( fluss_minus >= 5 ) && ( fluss_vorgabe < 15 ) )
				{
					fluss_vorgabe			=	10;
					DisplayShowFluss( 1 );
				}
			}
			else if( c4 == 0x32 )
			{
//REQUEST 252: Fluss speichern
				MemorySetFluss();
				SystemSetFluss();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x36 )
			{
//REQUEST 256: Automessung ändern
				if( autoMeasurement == TRUE )
					{ autoMeasurement = FALSE; }
				else
					{ autoMeasurement = TRUE; }
				DisplayShowAutomessung();
			}
			else if( c4 == 0x37 )
			{
//REQUEST 257: Automessung speichern
				if( autoMeasurement )
				{
					startAutoMeas		=	TRUE;
				}
				else
				{
					startAutoMeas		=	FALSE;
				}		
				MemorySetAutomessung();
				DisplayShowSpeicherung();
			}		
		}
		else if( c3 == 0x36 )
		{
			if( c4 == 0x31 )
			{
//REQUEST 261: Splitzeit erhöhen
				splitzeit_minus		=	0;
				if( splitzeit == 100 ) {  }
				else if( ( splitzeit_plus < 5 ) && ( splitzeit < 100 ) )
				{
					splitzeit		+=	1;
					splitzeit_plus	+=	1;
					DisplayShowSplitzeit();
				}
				else if( ( splitzeit_plus >= 5 ) && ( splitzeit < 96 ) )
				{
					splitzeit		+=	5;
					splitzeit_plus	+=	1;
					DisplayShowSplitzeit();
				}
				else if( ( splitzeit >= 5 ) && ( splitzeit > 95 ) )
				{
					splitzeit		=	100;
					DisplayShowSplitzeit();
				}
			}
			else if( c4 == 0x32 )
			{
//REQUEST 262: Splitzeit verringern
				splitzeit_plus		=	0;
				if( splitzeit == 1 ) {  }
				else if( ( splitzeit_minus < 5 ) && ( splitzeit > 1 ) )
				{
					splitzeit		-=	1;
					splitzeit_minus	+=	1;
					DisplayShowSplitzeit();
				}
				else if( ( splitzeit_minus >= 5 ) && ( splitzeit > 5 ) )
				{
					splitzeit		-=	5;
					splitzeit_minus	+=	1;
					DisplayShowSplitzeit();
				}
				else if( ( splitzeit_minus >= 5 ) && ( splitzeit < 6 ) )
				{
					splitzeit		=	1;
					DisplayShowSplitzeit();
				}
			}
			else if( c4 == 0x33 )
			{
//REQUEST 263: Splitzeit speichern
				MemorySetSplitzeit();
				DisplayShowSpeicherung();
				SystemSetSplitzeit();
			}			
		}	
	}
	else if( c2 == 0x33 )
	{
		if( c3 == 0x30 )
		{
			if( c4 == 0x31 )
			{
//REQUEST 301: Wechsel der Übertragungsart
				if( uebertragungsart == TRUE )
				{
					uebertragungsart = FALSE;
					MemorySetUebertragungsart();
					mA1			=	0;
					SystemSetMA1( mA1 );
					mA2			=	0;
					SystemSetMA2( mA2 );
				}
				else
				{
					uebertragungsart = TRUE;
					MemorySetUebertragungsart();
					mA1			=	830;
					SystemSetMA1( mA1 );
					mA2			=	830;
					SystemSetMA2( mA2 );
				}
				DisplayShowUebertragungsart();
			}
			else if( c4 == 0x32 )
			{
//REQUEST 302: Übertragungsart speichern
				MemorySetUebertragungsart();
				DisplayShowSpeicherung();
			}
			else if( c4 == 0x36 )
			{
//REQUEST 306: Skalierung erhöhen
				skalierung_minus	=	0;
				if( skalierung == 99 ) {  }
				else if( ( skalierung_plus < 5 ) && ( skalierung < 99 ) )
				{
					skalierung			+=	1;
					skalierung_plus		+=	1;
					DisplayShowSkalierung();
				}
				else if( ( skalierung_plus < 10 ) && ( skalierung < 95 ) )
				{
					skalierung			+=	5;
					skalierung_plus		+=	1;
					DisplayShowSkalierung();
				}
				else if( ( skalierung_plus < 10 ) && ( skalierung > 95 ) )
				{
					skalierung			=	99;
					DisplayShowSkalierung();
				}
				else if( ( skalierung_plus >= 10 ) && ( skalierung < 90 ) )
				{
					skalierung			+=	10;
					skalierung_plus		+=	1;
					DisplayShowSkalierung();
				}
				else if( ( skalierung_plus >= 10 ) && ( skalierung > 89 ) )
				{
					skalierung			=	99;
					DisplayShowSkalierung();
				}
			}
			else if( c4 == 0x37 )
			{
//REQUEST 307: Skalierung verringern
				skalierung_plus		=	0;
				if( skalierung == 10 ) {  }
				else if( ( skalierung_minus < 5 ) && ( skalierung > 10 ) )
				{
					skalierung			-=	1;
					skalierung_minus	+=	1;
					DisplayShowSkalierung();
				}
				else if( ( skalierung_minus < 10 ) && ( skalierung > 14 ) )
				{
					skalierung			-=	5;
					skalierung_minus	+=	1;
					DisplayShowSkalierung();
				}
				else if( ( skalierung_minus < 10 ) && ( skalierung < 15 ) )
				{
					skalierung			=	10;
					DisplayShowSkalierung();
				}
				else if( ( skalierung_minus >= 10 ) && ( skalierung > 19 ) )
				{
					skalierung			-=	10;
					skalierung_minus	+=	1;
					DisplayShowSkalierung();
				}
				else if( ( skalierung_minus >= 10 ) && ( skalierung < 20 ) )
				{
					skalierung			=	10;
					DisplayShowSkalierung();
				}	
			}
			else if( c4 == 0x38 )
			{
//REQUEST 308: Skalierung speichern
				MemorySetSkalierung();
				DisplayShowSpeicherung();
			}
		}
	}
	else if( c2 == 0x34 )
	{
		if( c3 == 0x30 )
		{
//REQUEST 401-403: Kalibrierung und/oder Messung starten
			
			//ANALYTIK RESTARTEN
			SystemStatus();
			if( ( system_status & DEV_RESET ) > 0 )
			{
				SystemClearResetFlag();
				SystemClearEndFlag();
				SystemClearActiveStarted();
				DisplayShowReset();
			}
			if( ( system_status & DEV_ACTIVE ) > 0 )
			{
				//MESSUNG LÄUFT --> KEINE NEUE MESSUNG STARTEN
			}
			else
			{
//aux
timecountaux = 0;
				//KANN GESTARTET WERDEN
				IsMASet = FALSE;				//um erkennen zu können, ob ein neuer MA-Wert angelegt wurde
				if( ( system_status & DEV_OK ) == 0 )
				{
					SystemSetOK( 1 );
				}
				if( ( system_status & DEV_ENDED ) > 0 )
				{
					SystemClearEndFlag();
				}
				
				checkFlussCount				=	0;
				
				//STARTUHRZEIT BESTIMMEN
				ClockReadHours();
				ClockReadMinutes();
	
				itoa( hrs1, buffer );
				uhrzeitLetzterLauf[0]		=	buffer[0];
				itoa( hrs2, buffer );
				uhrzeitLetzterLauf[1]		=	buffer[0];
				uhrzeitLetzterLauf[2]		=	0x3A;
	
				itoa( min1, buffer );
				uhrzeitLetzterLauf[3]		=	buffer[0];
				itoa( min2, buffer );
				uhrzeitLetzterLauf[4]		=	buffer[0];
				uhrzeitLetzterLauf[5]		=	0x00;
				
				//MESSDATEN MIT 0 INITIALISIEREN
				for( counter = 0; counter < 720; counter++ )
				{
					messdaten[counter]		=	0;
				}	
				
				//DATENPOINTER ZURÜCKSETZEN
				messdatenptr			=	&messdaten[0];
				
//				baselineConstant		=	floor( ( ( ( double ) signal ) * 0.9 ) * 0.04395 );
				baselineConstant		=	0;
								
				//KALIBRIERUNG UND MESSUNG STARTEN
				if( c4 == 0x31 )
				{
					measurementType		=	'b';
					SystemStartCalmeasurement();
					startAutoMeas		=	FALSE;
					calFlaeche			=	0;
					integral			=	0;
				}
				//MESSUNG STARTEN
				else if( c4 == 0x32 )
				{
					if( !syringe && startAutoMeas )
					{
						startAutoMeas		=	FALSE;
						DisplayRequest( '4', '0', '1' );
					}	
					else
					{
						if( calFlaeche >= calFlaecheMin )
						{
							measurementType		=	'm';
							if( !syringe ) {
								SystemStartMeasurement();
							}
							else {
								SystemStartSyringe();
							}	
							integral			=	0;
						}	
						else
						{
							measurementType		=	'b';
							if( !syringe ) {
								SystemStartCalmeasurement();
							}
							else {
								SystemStartSyringe();
							}	
							calFlaeche			=	0;
							integral			=	0;
						}	
					}	
				}
				//KALIBRIERUNG STARTEN
				else if( c4 == 0x33 )
				{
					if( !syringe && startAutoMeas )
					{
						startAutoMeas		=	FALSE;
						DisplayRequest( '4', '0', '1' );
					}
					else
					{
						measurementType		=	'c';
						if( !syringe ) {
							SystemStartCalibration();
						}
						else {
							SystemStartSyringe();
						}	
						calFlaeche			=	0;
					}	
				}	
				//GRAPH NEU INITIALISIEREN
				GraphInitGraph();
				GraphDeleteGraph();
				messpunkt_aktuell			=	messpunkt_start;
				DisplayShowPeakschranke();
				GraphDrawGraph();
			}	
		}	
	}
	else if( c2 == 0x35 )
	{
		if( c3 == 0x30 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 500: Passwort anzeigen
				drawgraph		=	FALSE;
				
				if( autoMeasurement )
				{
					DisplayShowErrorCopy();
				}	

				DisplayShowPasswort();
				DisplayShowPasswortCursor();
			}
			else if( c4 == 0x31 )
			{
//REQUEST 501: Passwortstelle erhöhen
				if( passwort[passwortindex] == 0x2D )
				{
					passwort[passwortindex]	=	0x30;
				}
				else if( passwort[passwortindex] == 0x39 )
				{
					passwort[passwortindex]	=	0x2D;
				}
				else
				{
					passwort[passwortindex]	+=	1;
				}
				DisplayShowPasswort();
			}
			else if( c4 == 0x32 )
			{
//REQUEST 502: Passwortstelle verringern
				if( passwort[passwortindex] == 0x30 )
				{
					passwort[passwortindex]	=	0x2D;
				}
				else if( passwort[passwortindex] == 0x2D )
				{
					passwort[passwortindex]	=	0x39;
				}
				else
				{
					passwort[passwortindex]	-=	1;
				}
				DisplayShowPasswort();
			}
			else if( c4 == 0x33 )
			{
//REQUEST 503: Passwortstelle rechts
				if( passwortindex > 0 )
				{
					passwortindex--;
					DisplayShowPasswortCursor();
				}
			}
			else if( c4 == 0x34 )
			{
//REQUEST 504: Passwortstelle links
				if( passwortindex < 7 )
				{
					passwortindex++;
					DisplayShowPasswortCursor();
				}	
			}
			else if( c4 == 0x35 )
			{
//REQUEST 505: Passwort vergleichen
				if( strcmp( passwort, pwd ) == 0 )
				{
					checkPwd		=	TRUE;
				}
				else
				{
					//PWD FALSCH!
					DisplaySaveDataPWFalsch();
				}
				passwort[0]			=	0x30;
				passwort[1]			=	0x30;
				passwort[2]			=	0x30;
				passwort[3]			=	0x30;
				passwort[4]			=	0x30;
				passwort[5]			=	0x30;
				passwort[6]			=	0x30;
				passwort[7]			=	0x30;
				passwort[8]			=	0x00;
			}
//REQUEST 506: RefreshMain ausschalten, da Bildschirm gewechselt
			else if( c4 == 0x36 )
			{
				drawgraph			=	FALSE;
			}
			else if( c4 == 0x37 )
			{
				copyAll				=	TRUE;
			}								
			else if( c4 == 0x38 )
			{
				copyAll				=	FALSE;
			}	
		}	
	}
	else if( c2 == 0x36 )
	{
		if( c3 == 0x30 )
		{
			if( c4 == 0x31 )
			{
//REQUEST 601: Anzeige Messwerte aktuelles Datum
				pageNo			=	0;
				dateiNo			=	0;
				recentFileNo	=	0;
				DataInitRead( current_date );
				DataShowDayResults( pageNo );
				if( dateiNo > 0 )
				{
					recentFileNo	=	1;
					DisplayShowDataCursor( recentFileNo );
				}	
			}
			else if( c4 == 0x32 )
			{
//REQUEST 602: Anzeige Messwerte vorhergehender Tag
				pageNo			=	0;
				dateiNo			=	0;
				recentFileNo	=	0;
				DataGetPreviousDay();
				DataShowDayResults( pageNo );
				if( dateiNo > 0 )
				{
					recentFileNo	=	1;
					DisplayShowDataCursor( recentFileNo );					
				}	
			}
			else if( c4 == 0x33 )
			{
//REQUEST 603: Anzeige Messwerte nächste Seite
				pageNo++;
				DataShowDayResults( pageNo );
				if( ( ( pageNo == 1 ) && ( dateiNo != 36 ) )
				|| ( ( pageNo == 2 ) && ( dateiNo != 72 ) ) )
				{
					recentFileNo	=	( pageNo * 36 ) + 1;
					DisplayShowDataCursor( recentFileNo );				
				}	
			}
			else if( c4 == 0x34 )
			{
//REQUEST 604: Auswahl vorheriger Messwert
				if( recentFileNo > ( pageNo * 36 ) + 1 )
				{
					recentFileNo--;
					DisplayClearDataCursor();
					DisplayShowDataCursor( recentFileNo );
				}
			}
			else if( c4 == 0x35 )
			{
//REQUEST 605: Auswahl nächster Messwert
				if( ( recentFileNo + 1 ) <= dateiNo )
				{
					recentFileNo++;
					DisplayClearDataCursor();
					DisplayShowDataCursor( recentFileNo );
				}	
			}
			else if( c4 == 0x36 )
			{
//REQUEST 606: Messdaten anzeigen
				DataShowMeasurement( recentFileNo );
			}
			else if( c4 == 0x37 )
			{
//REQUEST 607: Nächste Messkurve anzeigen
				recentFileNo++;
				DataShowMeasurement( recentFileNo );
			}							
		}
		if( c3 == 0x31 )
		{
			if( c4 == 0x31 )
			{
//REQUEST 611: 

			}
			else if( c4 == 0x32 )
			{
//REQUEST 612: Messwerte vorheriger Tag als Graph anzeigen
				DataGetPreviousDay();
				DisplayShowScaledGrid();
				DataShowGraphResults();
			}
			else if( c4 == 0x33 )
			{
//REQUEST 613: Auf richtigen Bildschirm für grafische Darstellung umschalten (Skalierung)
				MemoryGetSkalierung();
				DisplayShowScaledGrid();
				DataInitRead( current_date );
				DataShowGraphResults();
			}		
			else if( c4 == 0x34 )
			{
//REQUEST 614: Stoerung anzeigen
				MemoryGetErrorCount();
				DisplayGetErrorLog( errorcount );
			}		
		}		
	}
	else if( c2 == 0x37 )
	{
		if( c3 == 0x30 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 700: Master-Passwort checken
				if( strcmp( passwort, pwd_master ) == 0 )
				{
					checkMasterPwd		=	TRUE;
				}
				else
				{
					//PWD FALSCH!
					DisplaySaveDataPWFalsch();
				}
				passwort[0]			=	0x30;
				passwort[1]			=	0x30;
				passwort[2]			=	0x30;
				passwort[3]			=	0x30;
				passwort[4]			=	0x30;
				passwort[5]			=	0x30;
				passwort[6]			=	0x30;
				passwort[7]			=	0x30;
				passwort[8]			=	0x00;
			}
			else if( c4 == 0x31 )
			{
//REQUEST 701: Memory neu initalisieren
				ClockInit();
				ClockWriteEnable();
				ClockInitDateTime();

				MemoryInit();
				displayWait		=	0;
				while( displayWait < 2 ) { Nop(); }
				
				MemoryInit2();
				displayWait		=	0;
				while( displayWait < 2 ) { Nop(); }
				
				MemoryInit3();
				displayWait		=	0;
				while( displayWait < 2 ) { Nop(); }
				
				DisplayShowMemInitDone();
			}						
		}	
	}
	if( c2 == 0x38 )
	{
		if( c3 == 0x30 )
		{
			if( c4 == 0x30 )
			{
//REQUEST 800: Spritzenmessung?
				drawgraph		=	FALSE;
			}
			else if( c4 == 0x31 )
			{
//REQUEST 801: Spritzenkalibrierung starten
				//syringe		=	TRUE;
				
				//MemoryGetMenge();
				//MemoryGetVolumen();
				
				//eichgasmenge	=	(( double )menge / ( double )volumen ) * 100;
				//MemorySetEichgasmenge();
				//DisplayRequest( '4', '0', '3' );
				//DisplayRequest( '0', '0', '1' );
			}		
			else if( c4 == 0x32 )
			{
//REQUEST 802: Spritzenmessung starten
				syringe		=	TRUE;
				DisplayRequest( '4', '0', '2' );
				DisplayRequest( '0', '0', '1' );
			}	
		}	
	}
/*
//REQUEST 900: Messung von PC kommt und wird angezeigt und berechnet
	if(( c2 == 0x39 )
	{
		if( c3 == 0x30 )
		{
			if( c4 == 0x30 )
			{
				
			}
		}
	}
*/				
}

/** DISPLAY REFRESH MAIN ***********************************/
void DisplayRefreshMain( void )
{
	if( drawgraph == TRUE )
	{
		DisplayShowSekunden();
		DisplayShowSignal();
		DisplayShowAkku();
		DisplayShowDruck( 0 );
		DisplayShowTemperatur( 0 );
		DisplayShowFluss( 0 );
		DisplayShowStartzeit();
		DisplayShowCalflaeche();
		DisplayShowSystemStatus( ( char ) system_status );
		DisplayShowMessergebnis();
		DisplayShowSDOK();
		DisplayShowNetzbetrieb();
	}
}

/** DISPLAY SAVE DATA ***********************************/
void DisplaySaveData( void )
{
	data[0]		=	0x1B;
	data[1]		=	'Y';
	data[2]		=	'L';
	data[3]		=	0x01;
	data[4]		=	0x1B;
	data[5]		=	'M';
	data[6]		=	'X';
	data[7]		=	0x48;

	DisplaySendNBytes( data, 8 );
}

/** DISPLAY SEND BARGRAPH VALUE ***********************************/
void DisplaySendBargraphValue( unsigned short s )
{
	data[0]		=	0x1B;
	data[1]		=	'B';
	data[2]		=	'A';
	data[3]		=	0x01;
	data[4]		=	( char ) s;

	DisplaySendNBytes( data, 5 );
}

/** DISPLAY SEND BARGRAPH TOTAL NUMBER ***********************************/
void DisplaySendBargraphTotalNumber( unsigned int i )
{
	filesTotal		=	i;
	itoa( filesTotal, buffer );

	data[0]		=	0x1B;
	data[1]		=	'F';
	data[2]		=	'Z';
	data[3]		=	0x08;
	data[4]		=	0x00;
	data[5]		=	0x1B;
	data[6]		=	'Z';
	data[7]		=	'L';
	data[8]		=	0x9A;
	data[9]		=	0x01;	//x = 410
	data[10]	=	0xA0;
	data[11]	=	0x00;	//y = 160
	if( filesTotal < 10 )
	{
		data[12]	=	0x20;
		data[13]	=	0x20;
		data[14]	=	0x20;
		data[15]	=	0x20;
		data[16]	=	buffer[0];
	}
	else if( filesTotal < 100 )
	{
		data[12]	=	0x20;
		data[13]	=	0x20;
		data[14]	=	0x20;
		data[15]	=	buffer[0];
		data[16]	=	buffer[1];
	}
	else if( filesTotal < 1000 )
	{
		data[12]	=	0x20;
		data[13]	=	0x20;
		data[14]	=	buffer[0];
		data[15]	=	buffer[1];
		data[16]	=	buffer[2];
	}
	else if( filesTotal < 10000 )
	{
		data[12]	=	0x20;
		data[13]	=	buffer[0];
		data[14]	=	buffer[1];
		data[15]	=	buffer[2];
		data[16]	=	buffer[3];
	}
	else
	{
		data[12]	=	buffer[0];
		data[13]	=	buffer[1];
		data[14]	=	buffer[2];
		data[15]	=	buffer[3];
		data[16]	=	buffer[4];
	}
	data[17]		=	0x00;
	data[18]		=	0x1B;
	data[19]		=	'F';
	data[20]		=	'Z';
	data[21]		=	0x01;
	data[22]		=	0x00;

	DisplaySendNBytes( data, 23 );
}	

/** DISPLAY SEND BARGRAPH RECENT NUMBER ***********************************/
void DisplaySendBargraphRecentNumber( unsigned int i )
{
	filesCopied		=	i;
	itoa( filesCopied, buffer );

	data[0]		=	0x1B;
	data[1]		=	'F';
	data[2]		=	'Z';
	data[3]		=	0x08;
	data[4]		=	0x00;
	data[5]		=	0x1B;
	data[6]		=	'R';
	data[7]		=	'F';
	data[8]		=	0x90;
	data[9]		=	0x01;	//x1 = 400
	data[10]	=	0xB4;
	data[11]	=	0x00;	//y1 = 180
	data[12]	=	0xDF;
	data[13]	=	0x01;	//x2 = 479
	data[14]	=	0xC8;
	data[15]	=	0x00;	//y2 = 200
	data[16]	=	0x07;
	data[17]	=	0x1B;
	data[18]	=	'Z';
	data[19]	=	'L';
	data[20]	=	0x9A;
	data[21]	=	0x01;	//x = 410
	data[22]	=	0xB4;
	data[23]	=	0x00;	//y = 180
	if( filesCopied < 10 )
	{
		data[24]	=	0x20;
		data[25]	=	0x20;
		data[26]	=	0x20;
		data[27]	=	0x20;
		data[28]	=	buffer[0];
	}
	else if( filesCopied < 100 )
	{
		data[24]	=	0x20;
		data[25]	=	0x20;
		data[26]	=	0x20;
		data[27]	=	buffer[0];
		data[28]	=	buffer[1];
	}
	else if( filesCopied < 1000 )
	{
		data[24]	=	0x20;
		data[25]	=	0x20;
		data[26]	=	buffer[0];
		data[27]	=	buffer[1];
		data[28]	=	buffer[2];
	}
	else if( filesCopied < 10000 )
	{
		data[24]	=	0x20;
		data[25]	=	buffer[0];
		data[26]	=	buffer[1];
		data[27]	=	buffer[2];
		data[28]	=	buffer[3];
	}
	else
	{
		data[24]	=	buffer[0];
		data[25]	=	buffer[1];
		data[26]	=	buffer[2];
		data[27]	=	buffer[3];
		data[28]	=	buffer[4];
	}
	data[29]	=	0x00;
	data[30]	=	0x1B;
	data[31]	=	'F';
	data[32]	=	'Z';
	data[33]	=	0x01;
	data[34]	=	0x00;

	DisplaySendNBytes( data, 35 );
}

/** DISPLAY SAVE DATA COMPLETE ***********************************/
void DisplaySaveDataComplete( void )
{
	data[0]		=	0x1B;
	data[1]		=	'M';
	data[2]		=	'X';
	data[3]		=	0x49;

	DisplaySendNBytes( data, 4 );
}

/** DISPLAY SAVE DATA INSERT ***********************************/
void DisplaySaveDataInsert( void )
{
	data[0]		=	0x1B;
	data[1]		=	'M';	//M
	data[2]		=	'X';
	data[3]		=	0x59;

	DisplaySendNBytes( data, 4 );
}

/** DISPLAY SAVE DATA PW FALSCH ***********************************/
void DisplaySaveDataPWFalsch( void )
{
	data[0]		=	0x1B;
	data[1]		=	'M';
	data[2]		=	'X';
	data[3]		=	0x5A;

	DisplaySendNBytes( data, 4 );
}

/** DISPLAY SAVE DATA RETURN ***********************************/
void DisplaySaveDataReturn( void )
{
	data[0]		=	0x1B;
	data[1]		=	'M';
	data[2]		=	'N';
	data[3]		=	0x01;

	DisplaySendNBytes( data, 4 );
}

/** DISPLAY SHOW ERROR COPY *************************************/
void DisplayShowErrorCopy( void )
{
	data[0]		=	0x1B;
	data[1]		=	'F';
	data[2]		=	'Z';
	data[3]		=	0x03;
	data[4]		=	0x00;
	data[5]		=	0x1B;
	data[6]		=	'Z';
	data[7]		=	'L';
	data[8]		=	0x0A;
	data[9]		=	0x00;
	data[10]	=	0x0A;
	data[11]	=	0x00;
	data[12]	=	'E';
	data[13]	=	'R';
	data[14]	=	'R';
	data[15]	=	'O';
	data[16]	=	'R';
	data[17]	=	':';
	data[18]	=	' ';
	data[19]	=	'A';
	data[20]	=	'u';
	data[21]	=	't';
	data[22]	=	'o';
	data[23]	=	'm';
	data[24]	=	'e';
	#if defined( DEUTSCH )
	data[25]	=	's';
	data[26]	=	's';
	data[27]	=	'u';
	data[28]	=	'n';
	data[29]	=	'g';
	data[30]	=	' ';
	data[31]	=	'a';
	data[32]	=	'k';
	data[33]	=	't';
	data[34]	=	'i';
	data[35]	=	'v';
	data[36]	=	'!';
	data[37]	=	0x00;
	data[38]	=	0x1B;
	data[39]	=	'F';
	data[40]	=	'Z';
	data[41]	=	0x08;
	data[42]	=	0x00;
	
	DisplaySendNBytes( data, 43 );
	#endif
	
	#if defined( ENGLISCH )
	data[25]	=	'a';
	data[26]	=	's';
	data[27]	=	'u';
	data[28]	=	'r';
	data[29]	=	'e';
	data[30]	=	'm';
	data[31]	=	'e';
	data[32]	=	'n';
	data[33]	=	't';
	data[34]	=	' ';
	data[35]	=	'a';
	data[36]	=	'c';
	data[37]	=	't';
	data[38]	=	'i';
	data[39]	=	'v';
	data[40]	=	'e';
	data[41]	=	0x00;
	data[42]	=	0x1B;
	data[43]	=	'F';
	data[44]	=	'Z';
	data[45]	=	0x08;
	data[46]	=	0x00;
	
	DisplaySendNBytes( data, 47 );
	#endif
}	

/** DISPLAY SHOW SERIAL NUMBER *************************************/
void DisplayShowSerialNumber( int sn, char* SWVersion )
{
	itoa( sn, buffer );
	
	data[0]		=	0x1B;
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x50;
	data[4]		=	0x01;
	data[5]		=	0xF0;
	data[6]		=	0x00;
	data[7]		=	buffer[0];
	data[8]		=	buffer[1];
	data[9]		=	buffer[2];
	data[10]	=	buffer[3];
	data[11]	=	' ';
	data[12]	=	SWVersion[0];
	data[13]	=	SWVersion[1];
	data[14]	=	SWVersion[2];
	data[15]	=	SWVersion[3];
	data[16]	=	SWVersion[4];
	data[17]	=	SWVersion[5];
	data[18]	=	0x00;
	
	DisplaySendNBytes( data, 19 );
}	

/** DISPLAY SHOW MESSERGEBNIS ***********************************/
void DisplayShowMessergebnis( void )
{
	itoa( messergebnis, buffer );
	
	//Rechteck löschen von 300,185 nach 478,215
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x2C;
	data[4]		=	0x01;	//x1=300
	data[5]		=	0xB9;
	data[6]		=	0x00;	//y1=185
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0xD7;
	data[10]	=	0x00;	//y2=215
	//Schriftart auf groß stellen
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x46;	//F
	data[14]	=	0x06;	//6
	//letztes Messergebnis senden
	data[15]	=	0x1B;	//ESC
	data[16]	=	0x5A;	//Z
	data[17]	=	0x4C;	//L
	data[18]	=	0x2C;
	data[19]	=	0x01;	//x=300
	data[20]	=	0xB9;
	data[21]	=	0x00;	//y=185
	if( messergebnis < 10 )
	{
		data[22]	=	0x30;
		data[23]	=	0x30;
		data[24]	=	0x2E;
		data[25]	=	buffer[0];	//result
	}	
	else if( messergebnis < 100 )
	{
		data[22]	=	0x30;
		data[23]	=	buffer[0];
		data[24]	=	0x2E;
		data[25]	=	buffer[1];	//result
	}	
	else if( messergebnis < 1000 )
	{
		data[22]	=	buffer[0];
		data[23]	=	buffer[1];
		data[24]	=	0x2E;
		data[25]	=	buffer[2];	//result
	}	
	data[26]	=	0x20;	//SPACE
	data[27]	=	0x6D;	//m
	data[28]	=	'g';	//g	
	data[29]	=	0x20;	//SPACE
	data[30]	=	'T';
	data[31]	=	'H';
	data[32]	=	'T';
	data[33]	=	0x00;	//NUL
	//Schriftart auf normal stellen
	data[34]	=	0x1B;	//ESC
	data[35]	=	0x5A;	//Z
	data[36]	=	0x46;	//F
	data[37]	=	0x05;	//5

	DisplaySendNBytes( data, 38 );
}

/** DISPLAY SHOW RESET ***********************************/
void DisplayShowReset( void )
{
	data[0]		=	0x1B;
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x24;
	data[4]		=	0x01;
	data[5]		=	0xE6;
	data[6]		=	0x00;
	data[7]		=	'R';
	data[8]		=	'E';
	data[9]		=	'S';
	data[10]	=	'E';
	data[11]	=	'T';
	data[12]	=	0x00;
	
	DisplaySendNBytes( data, 13 );
}	

/** DISPLAY SHOW SIGNAL ***********************************/
void DisplayShowSignal( void )
{
	signal		=	floor( signal * 0.1221 );
	
	itoa( signal, buffer );

	//Rechteck löschen von 400,27 nach 478,42
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'L';
	data[3]		=	0x90;
	data[4]		=	0x01;	//x1=400
	data[5]		=	0x1B;
	data[6]		=	0x00;	//y1=27
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x2A;
	data[10]	=	0x00;	//y2=42
	//Signalwert
	data[11]	=	0x1B;
	data[12]	=	'Z';
	data[13]	=	'L';
	data[14]	=	0x90;
	data[15]	=	0x01;	//x=400
	data[16]	=	0x1B;
	data[17]	=	0x00;	//y=27
	if( signal < 10 )
	{
		data[18]	=	' ';
		data[19]	=	' ';
		data[20]	=	0x30;
		data[21]	=	'.';
		data[22]	=	buffer[0];
	}
	else if( signal < 100 )
	{
		data[18]	=	' ';
		data[19]	=	' ';
		data[20]	=	buffer[0];
		data[21]	=	'.';
		data[22]	=	buffer[1];
	}
	else if( signal < 1000 )
	{
		data[18]	=	' ';
		data[19]	=	buffer[0];
		data[20]	=	buffer[1];
		data[21]	=	'.';
		data[22]	=	buffer[2];
	}
	else
	{
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	buffer[2];
		data[21]	=	'.';
		data[22]	=	buffer[3];
	}	
	data[23]	=	' ';	//SPACE
	data[24]	=	'%';	//%
	data[25]	=	0x00;	//NUL

	DisplaySendNBytes( data, 26 );
}

/** DISPLAY SHOW CALFLAECHE ***********************************/
void DisplayShowCalflaeche( void )
{
	unsigned char buffer[8];
	
	litoa( calFlaeche, buffer );
	
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'L';
	data[3]		=	0x90;
	data[4]		=	0x01;
	data[5]		=	0x81;
	data[6]		=	0x00;
	data[7]		=	0xDE;
	data[8]		=	0x01;
	data[9]		=	0x90;
	data[10]	=	0x00;
	
	DisplaySendNBytes( data, 11 );
	
	data[0]		=	0x1B;
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x90;
	data[4]		=	0x01;
	data[5]		=	0x81;
	data[6]		=	0x00;
	
	if( calFlaeche < 10 )
	{
		data[7]		=	buffer[0];
		data[8]		=	0x00;
		
		DisplaySendNBytes( data, 9 );
	}
	else if( calFlaeche < 100 )
	{
		data[7]		=	buffer[0];
		data[8]		=	buffer[1];
		data[9]		=	0x00;
		
		DisplaySendNBytes( data, 10 );
	}
	else if( calFlaeche < 1000 )
	{
		data[7]		=	buffer[0];
		data[8]		=	buffer[1];
		data[9]		=	buffer[2];
		data[10]	=	0x00;
		
		DisplaySendNBytes( data, 11 );
	}
	else if( calFlaeche < 10000 )
	{
		data[7]		=	buffer[0];
		data[8]		=	buffer[1];
		data[9]		=	buffer[2];
		data[10]	=	buffer[3];
		data[11]	=	0x00;
		
		DisplaySendNBytes( data, 12 );
	}
	else if( calFlaeche < 100000 )
	{
		data[7]		=	buffer[0];
		data[8]		=	buffer[1];
		data[9]		=	buffer[2];
		data[10]	=	buffer[3];
		data[11]	=	buffer[4];
		data[12]	=	0x00;
		
		DisplaySendNBytes( data, 13 );
	}
	else if( calFlaeche < 1000000 )
	{
		data[7]		=	buffer[0];
		data[8]		=	buffer[1];
		data[9]		=	buffer[2];
		data[10]	=	buffer[3];
		data[11]	=	buffer[4];
		data[12]	=	buffer[5];
		data[13]	=	0x00;
		
		DisplaySendNBytes( data, 14 );
	}
	else
	{
		data[7]		=	buffer[0];
		data[8]		=	buffer[1];
		data[9]		=	buffer[2];
		data[10]	=	buffer[3];
		data[11]	=	buffer[4];
		data[12]	=	buffer[5];
		data[13]	=	buffer[6];
		data[14]	=	0x00;
		
		DisplaySendNBytes( data, 15 );
	}							
}	

/** DISPLAY SHOW AKKU ***********************************/
void DisplayShowAkku( void )
{
	akku		=	ReadAkku();
	
	Nop();
	Nop();
	Nop();

	itoa( akku, buffer );

	//Rechteck löschen von 400,44 nach 478,59
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'L';
	data[3]		=	0x90;
	data[4]		=	0x01;	//x1=400
	data[5]		=	0x2C;
	data[6]		=	0x00;	//y1=44
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x3B;
	data[10]	=	0x00;	//y2=59
	//Akkustand
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x90;
	data[15]	=	0x01;	//x=400
	data[16]	=	0x2C;
	data[17]	=	0x00;	//y=44
	if( akku < 10 )
	{
		data[18]	=	0x30;
		data[19]	=	0x2E;
		data[20]	=	buffer[0];
		data[21]	=	' ';
		data[22]	=	'V';
		data[23]	=	0x00;
		
		DisplaySendNBytes( data, 24 );
	}
	else if( akku < 100 )
	{
		data[18]	=	buffer[0];
		data[19]	=	0x2E;
		data[20]	=	buffer[1];
		data[21]	=	' ';
		data[22]	=	'V';
		data[23]	=	0x00;
		
		DisplaySendNBytes( data, 24 );
	}
	else
	{
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	0x2E;
		data[21]	=	buffer[2];
		data[22]	=	' ';
		data[23]	=	'V';
		data[24]	=	0x00;
		
		DisplaySendNBytes( data, 25 );
	}
	
	if( akku < 75 )
	{
		battery_low			=	TRUE;
		battery_recharged	=	FALSE;
	}
	else if( akku >= 75 )
	{
		battery_low			=	FALSE;
		battery_recharged	=	TRUE;
	}		
}

/** DISPLAY SHOW FLUSS ***********************************/
void DisplayShowFluss( short i )
{
	if( i == 0 )
	{
		SystemFlussIstwert();
		itoa( fluss_istwert, buffer );
		//Rechteck löschen von 400,95 nach 478,110
		data[0]		=	0x1B;
		data[1]		=	'R';
		data[2]		=	'L';
		data[3]		=	0x90;
		data[4]		=	0x01;	//x1=400
		data[5]		=	0x5F;
		data[6]		=	0x00;	//y1=95
		data[7]		=	0xDE;
		data[8]		=	0x01;	//x2=478
		data[9]		=	0x6E;
		data[10]	=	0x00;	//y2=110
		//Flusswert
		data[11]	=	0x1B;
		data[12]	=	'Z';
		data[13]	=	'L';
		data[14]	=	0x90;
		data[15]	=	0x01;	//x=400
		data[16]	=	0x5F;
		data[17]	=	0x00;	//y=95
		if( fluss_istwert < 10 )
		{
			data[18]	=	'0';
			data[19]	=	'0';
			data[20]	=	buffer[0];
		}
		else if( fluss_istwert < 100 )
		{
			data[18]	=	'0';
			data[19]	=	buffer[0];
			data[20]	=	buffer[1];
		}
		else if( fluss_istwert >= 100 )
		{
			data[18]	=	buffer[0];
			data[19]	=	buffer[1];
			data[20]	=	buffer[2];
		}
	}
	else if( i == 1 )
	{
		itoa( fluss_vorgabe, buffer );
		//Rechteck löschen von 121,127 nach 244,147
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x79;
		data[4]		=	0x00;	//x1=121
		data[5]		=	0x7F;
		data[6]		=	0x00;	//y1=127
		data[7]		=	0xF4;
		data[8]		=	0x00;	//x2=244
		data[9]		=	0x93;
		data[10]	=	0x00;	//y2=147
		//Flusswert
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x7D;
		data[15]	=	0x00;	//x=125
		data[16]	=	0x82;
		data[17]	=	0x00;	//y=130
		if( fluss_vorgabe < 10 )
		{
			data[18]	=	0x30;
			data[19]	=	0x30;
			data[20]	=	buffer[0];
		}
		else if( fluss_vorgabe < 100 )
		{
			data[18]	=	0x30;
			data[19]	=	buffer[0];
			data[20]	=	buffer[1];
		}
		else if( fluss_vorgabe >= 100 )
		{
			data[18]	=	buffer[0];
			data[19]	=	buffer[1];
			data[20]	=	buffer[2];
		}
	}
	data[21]	=	0x20;	//SPACE
	data[22]	=	0x6D;	//m
	data[23]	=	0x6C;	//l
	data[24]	=	0x2F;	// '/'
	data[25]	=	0x6D;	//m
	data[26]	=	0x69;	//i
	data[27]	=	0x6E;	//n
	data[28]	=	0x00;	//NUL

	DisplaySendNBytes( data, 29 );
}

/** DISPLAY SHOW MESSORT ***********************************/
void DisplayShowMessort( void )
{
	//Rechteck löschen von 210,1 nach 478,130
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0xD2;
	data[4]		=	0x00;	//x1=210
	data[5]		=	0x01;
	data[6]		=	0x00;	//y1=1
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x82;
	data[10]	=	0x00;	//y2=130
	//Schriftart auf groß stellen
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x46;	//F
	data[14]	=	0x06;	//6
	//Messortdaten anzeigen
	data[15]	=	0x1B;	//ESC
	data[16]	=	0x5A;	//Z
	data[17]	=	0x4C;	//L
	data[18]	=	0xDC;
	data[19]	=	0x00;	//x=220
	data[20]	=	0x64;
	data[21]	=	0x00;	//y=100
	data[22]	=	messort[0];
	data[23]	=	messort[1];
	data[24]	=	messort[2];
	data[25]	=	messort[3];
	data[26]	=	messort[4];
	data[27]	=	messort[5];
	data[28]	=	messort[6];
	data[29]	=	messort[7];
	data[30]	=	0x00;	//NUL
	//Schriftart auf normal stellen
	data[31]	=	0x1B;	//ESC
	data[32]	=	0x5A;	//Z
	data[33]	=	0x46;	//F
	data[34]	=	0x05;	//5

	DisplaySendNBytes( data, 35 );
}

/** DISPLAY SHOW PASSWORT ***********************************/
void DisplayShowPasswort( void )
{
	//Rechteck löschen von 210,1 nach 478,130
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0xD2;
	data[4]		=	0x00;	//x1=210
	data[5]		=	0x01;
	data[6]		=	0x00;	//y1=1
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x82;
	data[10]	=	0x00;	//y2=130
	//Schriftart auf groß stellen
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x46;	//F
	data[14]	=	0x06;	//6
	//Messortdaten anzeigen
	data[15]	=	0x1B;	//ESC
	data[16]	=	0x5A;	//Z
	data[17]	=	0x4C;	//L
	data[18]	=	0xDC;
	data[19]	=	0x00;	//x=220
	data[20]	=	0x64;
	data[21]	=	0x00;	//y=100
	data[22]	=	passwort[0];
	data[23]	=	passwort[1];
	data[24]	=	passwort[2];
	data[25]	=	passwort[3];
	data[26]	=	passwort[4];
	data[27]	=	passwort[5];
	data[28]	=	passwort[6];
	data[29]	=	passwort[7];
	data[30]	=	0x00;	//NUL
	//Schriftart auf normal stellen
	data[31]	=	0x1B;	//ESC
	data[32]	=	0x5A;	//Z
	data[33]	=	0x46;	//F
	data[34]	=	0x05;	//5

	DisplaySendNBytes( data, 35 );
}

/** DISPLAY SHOW MESSORT CURSOR ***********************************/
void DisplayShowMessortCursor( void )
{
	//Rechteck löschen von 210,131 nach 478,132
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0xD2;
	data[4]		=	0x00;	//x1=210
	data[5]		=	0x83;
	data[6]		=	0x00;	//y1=131
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x84;
	data[10]	=	0x00;	//y2=132
	//Cursor zeichnen
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x52;	//R
	data[13]	=	0x46;	//F
	if( messortindex == 0 )
	{
		data[14]	=	0xDD;
		data[15]	=	0x00;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0xEA;
		data[19]	=	0x00;
	}
	else if( messortindex == 1 )
	{
		data[14]	=	0xEC;
		data[15]	=	0x00;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0xF9;
		data[19]	=	0x00;
	}
	else if( messortindex == 2 )
	{
		data[14]	=	0xFB;
		data[15]	=	0x00;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x08;
		data[19]	=	0x01;
	}
	else if( messortindex == 3 )
	{
		data[14]	=	0x0A;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x17;
		data[19]	=	0x01;
	}
	else if( messortindex == 4 )
	{
		data[14]	=	0x19;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x26;
		data[19]	=	0x01;
	}
	else if( messortindex == 5 )
	{
		data[14]	=	0x28;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x35;
		data[19]	=	0x01;
	}
	else if( messortindex == 6 )
	{
		data[14]	=	0x37;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x44;
		data[19]	=	0x01;
	}
	else if( messortindex == 7 )
	{
		data[14]	=	0x46;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x53;
		data[19]	=	0x01;
	}
	data[20]	=	0x84;
	data[21]	=	0x00;	//y2=132
	data[22]	=	0x07;	//YELLOW

	DisplaySendNBytes( data, 23 );
}

/** DISPLAY SHOW DEVLEOPER MODE ***********************************/
void DisplayShowDeveloperMode( void )
{
	data[0]		=	0x1B;
	data[1]		=	'M';
	data[2]		=	'X';
	data[3]		=	0x7C;
	
	DisplaySendNBytes( data, 4 );
}	

/** DISPLAY SHOW BATTERY LOW ***********************************/
void DisplayShowBatteryLow( void )
{
	data[0]		=	0x1B;
	data[1]		=	'M';
	data[2]		=	'X';
	data[3]		=	0x7E;
	
	DisplaySendNBytes( data, 4 );
}

/** DISPLAY SHOW PASSWORT CURSOR ***********************************/
void DisplayShowPasswortCursor( void )
{
	//Rechteck löschen von 210,131 nach 478,132
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0xD2;
	data[4]		=	0x00;	//x1=210
	data[5]		=	0x83;
	data[6]		=	0x00;	//y1=131
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x84;
	data[10]	=	0x00;	//y2=132
	//Cursor zeichnen
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x52;	//R
	data[13]	=	0x46;	//F
	if( passwortindex == 0 )
	{
		data[14]	=	0xDD;
		data[15]	=	0x00;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0xEA;
		data[19]	=	0x00;
	}
	else if( passwortindex == 1 )
	{
		data[14]	=	0xEC;
		data[15]	=	0x00;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0xF9;
		data[19]	=	0x00;
	}
	else if( passwortindex == 2 )
	{
		data[14]	=	0xFB;
		data[15]	=	0x00;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x08;
		data[19]	=	0x01;
	}
	else if( passwortindex == 3 )
	{
		data[14]	=	0x0A;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x17;
		data[19]	=	0x01;
	}
	else if( passwortindex == 4 )
	{
		data[14]	=	0x19;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x26;
		data[19]	=	0x01;
	}
	else if( passwortindex == 5 )
	{
		data[14]	=	0x28;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x35;
		data[19]	=	0x01;
	}
	else if( passwortindex == 6 )
	{
		data[14]	=	0x37;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x44;
		data[19]	=	0x01;
	}
	else if( passwortindex == 7 )
	{
		data[14]	=	0x46;
		data[15]	=	0x01;
		data[16]	=	0x83;
		data[17]	=	0x00;	//y1=131
		data[18]	=	0x53;
		data[19]	=	0x01;
	}
	data[20]	=	0x84;
	data[21]	=	0x00;	//y2=132
	data[22]	=	0x07;	//YELLOW

	DisplaySendNBytes( data, 23 );
}

/** DISPLAY SHOW EICHGASMENGE ***********************************/
void DisplayShowEichgas( int i )
{
	itoa( eichgasmenge, buffer );

	if( i == 0 )
	{
		//Rechteck löschen von 149,147 nach 300,167
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x95;
		data[4]		=	0x00;	//x1=149
		data[5]		=	0x93;
		data[6]		=	0x00;	//y1=147
		data[7]		=	0x2C;
		data[8]		=	0x01;	//x2=300
		data[9]		=	0xA7;
		data[10]	=	0x00;	//y2=167
		//Eichgasmenge anzeigen
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x96;
		data[15]	=	0x00;	//x=150
		data[16]	=	0x96;
		data[17]	=	0x00;	//y=150
	}	
	else if( i == 1 )
	{
		//Rechteck löschen von 121,7 nach 244,27
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x79;
		data[4]		=	0x00;	//x1=121
		data[5]		=	0x07;
		data[6]		=	0x00;	//y1=7
		data[7]		=	0xF4;
		data[8]		=	0x00;	//x2=244
		data[9]		=	0x1B;
		data[10]	=	0x00;	//y2=27
		//Eichgasmenge anzeigen
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x7D;
		data[15]	=	0x00;	//x=125
		data[16]	=	0x0A;
		data[17]	=	0x00;	//y=10
	}	
	if( eichgasmenge < 10 )
	{
		data[18]	=	0x30;
		data[19]	=	0x30;
		data[20]	=	0x30;
		data[21]	=	0x2E;
		data[22]	=	buffer[0];
	}
	else if( eichgasmenge < 100 )
	{
		data[18]	=	0x30;
		data[19]	=	0x30;
		data[20]	=	buffer[0];
		data[21]	=	0x2E;
		data[22]	=	buffer[1];
	}
	else if( eichgasmenge < 1000 )
	{
		data[18]	=	0x30;
		data[19]	=	buffer[0];
		data[20]	=	buffer[1];
		data[21]	=	0x2E;
		data[22]	=	buffer[2];
	}
	else
	{
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	buffer[2];
		data[21]	=	0x2E;
		data[22]	=	buffer[3];
	}	
	data[23]	=	0x20;	//SPACE
	data[24]	=	0x6D;	//m
	data[25]	=	0x67;	//g
	data[26]	=	0x20;	//SPACE
	data[27]	=	0x54;	//T
	data[28]	=	0x48;	//H
	data[29]	=	0x54;	//T ... Eichgasmenge
	data[30]	=	0x00;	//NUL

	DisplaySendNBytes( data, 31 );
}

/** DISPLAY SHOW SD OK ***********************************/
void DisplayShowSDOK( void )
{
	if( initResults == FALSE )
	{
		data[0]		=	0x1B;	//ESC
		data[1]		=	'F';
		data[2]		=	'Z';
		data[3]		=	0x03;	//rot
		data[4]		=	0x00;
		
		DisplaySendNBytes( data, 5 );
	}	
	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x18;
	data[4]		=	0x00;
	data[5]		=	0xCC;
	data[6]		=	0x00;
	data[7]		=	'S';
	data[8]		=	'D';
	data[9]		=	0x00;
	
	DisplaySendNBytes( data, 10 );
	
	if( initResults == FALSE )
	{
		data[0]		=	0x1B;	//ESC
		data[1]		=	'F';
		data[2]		=	'Z';
		data[3]		=	0x01;
		data[4]		=	0x00;
		
		DisplaySendNBytes( data, 5 );
	}	
}

/** DISPLAY SHOW NETZBETRIEB ***********************************/
void DisplayShowNetzbetrieb( void )
{
	if( PORTBbits.RB10 == 0 )
	{
		data[0]		=	0x1B;	//ESC
		data[1]		=	'F';
		data[2]		=	'Z';
		data[3]		=	0x03;	//rot
		data[4]		=	0x00;
		
		DisplaySendNBytes( data, 5 );
	}	
	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x38;
	data[4]		=	0x00;
	data[5]		=	0xCC;
	data[6]		=	0x00;
	data[7]		=	'~';
	//data[8]		=	'E';
	//data[9]		=	'T';
	//data[10]	=	'Z';
	data[8]		=	0x00;
	
	DisplaySendNBytes( data, 9 );
	
	if( PORTBbits.RB10 == 0 )
	{
		data[0]		=	0x1B;	//ESC
		data[1]		=	'F';
		data[2]		=	'Z';
		data[3]		=	0x01;
		data[4]		=	0x00;
		
		DisplaySendNBytes( data, 5 );
	}	
}

/** DISPLAY SHOW SCALED GRID ***********************************/
void DisplayShowScaledGrid( void )
{
	data[0]		=	0x1B;
	data[1]		=	'M';
	data[2]		=	'X';
	
	if( skalierung <= 10 ) { data[3] = 0x70; }
	else if( skalierung <= 20 ) { data[3] = 0x71; }
	else if( skalierung <= 30 ) { data[3] = 0x72; }
	else if( skalierung <= 40 ) { data[3] = 0x73; }
	else if( skalierung <= 50 ) { data[3] = 0x74; }
	else if( skalierung <= 60 ) { data[3] = 0x75; }
	else if( skalierung <= 70 ) { data[3] = 0x76; }
	else if( skalierung <= 80 ) { data[3] = 0x77; }
	else if( skalierung <= 90 ) { data[3] = 0x78; }
	else { data[3] = 0x79; }
	
	DisplaySendNBytes( data, 4 );
}

/** DISPLAY SHOW NACK ***********************************/
void DisplayShowNACK( void )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	'F';
	data[2]		=	'Z';
	data[3]		=	0x03;	//rot
	data[4]		=	0x00;
		
	DisplaySendNBytes( data, 5 );
	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x3C;
	data[4]		=	0x00;
	data[5]		=	0xCC;
	data[6]		=	0x00;
	data[7]		=	'N';
	data[8]		=	'A';
	data[9]		=	'C';
	data[10]	=	'K';
	data[11]	=	0x00;
	
	DisplaySendNBytes( data, 12 );
	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'F';
	data[2]		=	'Z';
	data[3]		=	0x01;
	data[4]		=	0x00;
		
	DisplaySendNBytes( data, 5 );
}

/** DISPLAY SHOW AREA ***********************************/
void DisplayShowArea() {
	litoa( integral, buffer );
	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x48;
	data[4]		=	0x00;
	data[5]		=	0xCC;
	data[6]		=	0x00;
	data[7]		=	'A';
	data[8]		=	'R';
	data[9]		=	'E';
	data[10]	=	'A';
	data[11]	=	':';
	data[12]	=	' ';
	if( integral < 10 )
	{
		data[13]		=	buffer[0];
		data[14]		=	0x00;
		
		DisplaySendNBytes( data, 15 );
	}
	else if( integral < 100 )
	{
		data[13]		=	buffer[0];
		data[14]		=	buffer[1];
		data[15]		=	0x00;
		
		DisplaySendNBytes( data, 16 );
	}
	else if( integral < 1000 )
	{
		data[13]		=	buffer[0];
		data[14]		=	buffer[1];
		data[15]		=	buffer[2];
		data[16]		=	0x00;
		
		DisplaySendNBytes( data, 17 );
	}
	else if( integral < 10000 )
	{
		data[13]		=	buffer[0];
		data[14]		=	buffer[1];
		data[15]		=	buffer[2];
		data[16]		=	buffer[3];
		data[17]		=	0x00;
		
		DisplaySendNBytes( data, 18 );
	}
	else if( integral < 100000 )
	{
		data[13]		=	buffer[0];
		data[14]		=	buffer[1];
		data[15]		=	buffer[2];
		data[16]		=	buffer[3];
		data[17]		=	buffer[4];
		data[18]		=	0x00;
		
		DisplaySendNBytes( data, 19 );
	}
	else if( integral < 1000000 )
	{
		data[13]		=	buffer[0];
		data[14]		=	buffer[1];
		data[15]		=	buffer[2];
		data[16]		=	buffer[3];
		data[17]		=	buffer[4];
		data[18]		=	buffer[5];
		data[19]		=	0x00;
		
		DisplaySendNBytes( data, 20 );
	}
	else
	{
		data[13]		=	buffer[0];
		data[14]		=	buffer[1];
		data[15]		=	buffer[2];
		data[16]		=	buffer[3];
		data[17]		=	buffer[4];
		data[18]		=	buffer[5];
		data[19]		=	buffer[6];
		data[20]		=	0x00;
		
		DisplaySendNBytes( data, 21 );
	}
}
/** DISPLAY SHOW NO DATA ***********************************/
void DisplayShowNoData( void )
{
	data[0]		=	0x1B;
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x14;
	data[4]		=	0x00;
	data[5]		=	0x19;
	data[6]		=	0x00;
	#if defined( DEUTSCH )
	data[7]		=	'K';
	data[8]		=	'E';
	data[9]		=	'I';
	data[10]	=	'N';
	data[11]	=	'E';
	data[12]	=	' ';
	data[13]	=	'M';
	data[14]	=	'E';
	data[15]	=	'S';
	data[16]	=	'S';
	data[17]	=	'D';
	data[18]	=	'A';
	data[19]	=	'T';
	data[20]	=	'E';
	data[21]	=	'N';
	data[22]	=	0x00;
	
	DisplaySendNBytes( data, 23 );
	#endif
	
	#if defined( ENGLISCH )
	data[7]		=	'N';
	data[8]		=	'O';
	data[9]		=	' ';
	data[10]	=	'M';
	data[11]	=	'E';
	data[12]	=	'A';
	data[13]	=	'S';
	data[14]	=	'U';
	data[15]	=	'R';
	data[16]	=	'E';
	data[17]	=	'M';
	data[18]	=	'E';
	data[19]	=	'N';
	data[20]	=	'T';
	data[21]	=	'S';
	data[22]	=	0x00;
	
	DisplaySendNBytes( data, 23 );
	#endif
	
	DisplayShowDataDisableNextPage();
}

/** DISPLAY SHOW DATA END ***********************************/
void DisplayShowDataEnd( unsigned int position )
{
	unsigned int xpos		=	position - 1;
	unsigned int ypos		=	position - 1;
	
	unsigned int x1pos		=	0;
	unsigned int x2pos		=	0;
	unsigned int y1pos		=	0;
	
	xpos					=	xpos % 36;
	ypos					=	ypos % 12;
	
	if( xpos <= 11 )
	{
		x1pos				=	0x14;
	}
	else if( xpos <= 23 )
	{
		x1pos				=	0xBE;
	}
	else if( xpos <= 35 )
	{
		x2pos				=	0x01;
		x1pos				=	0x68;
	}
	
	y1pos					=	0x19 + ( ypos * 0x10 );
	
	data[0]					=	0x1B;
	data[1]					=	'Z';
	data[2]					=	'L';
	data[3]					=	x1pos;
	data[4]					=	x2pos;
	data[5]					=	y1pos;
	data[6]					=	0x00;
	
	data[7]					=	'E';
	data[8]					=	'N';
	data[9]					=	'D';	
	
	data[10]				=	0x00;
	
	DisplaySendNBytes( data, 11 );
	
	DisplayShowDataDisableNextPage();
}

/** DISPLAY SHOW DATA DISABLE NEXT PAGE ***********************************/
void DisplayShowDataDisableNextPage( void )
{
	//Button durch leeres Feld ersetzen
	data[0]		=	0x1B;
	data[1]		=	'U';
	data[2]		=	'I';
	data[3]		=	0x60;
	data[4]		=	0x00;
	data[5]		=	0xDC;
	data[6]		=	0x00;
	data[7]		=	0x09;
	//Knopf auf Zustand NOT_IN_USE setzen
	data[8]		=	0x1B;
	data[9]		=	'Y';
	data[10]	=	'X';
	data[11]	=	0x02;
	data[12]	=	0xFE;
	
	DisplaySendNBytes( data, 13 );
}

/** DISPLAY SHOW PEAKSCHRANKE ***********************************/
void DisplayShowPeakschranke( void )
{
	data[0]		=	0x1B;
	data[1]		=	'G';
	data[2]		=	'M';
	data[3]		=	0x03;
	data[4]		=	0x1B;
	data[5]		=	'F';
	data[6]		=	'M';
	data[7]		=	0x09;
	data[8]		=	0x10;
	data[9]		=	0x1B;
	data[10]	=	'G';
	data[11]	=	'Z';
	data[12]	=	0x02;
	data[13]	=	0x02;
	data[14]	=	0x1B;
	data[15]	=	'F';
	data[16]	=	'G';
	data[17]	=	0x09;
	data[18]	=	0x10;
	
	DisplaySendNBytes( data, 19 );
	
	if( measurementType == 'b' )
	{
		data[0]		=	0x1B;
		data[1]		=	'G';
		data[2]		=	'D';
		data[3]		=	0x40;
		data[4]		=	0x00;
		data[5]		=	0x14;
		data[6]		=	0x00;
		data[7]		=	0x40;
		data[8]		=	0x00;
		data[9]		=	0xC6;
		data[10]	=	0x00;
		data[11]	=	0x1B;
		data[12]	=	'G';
		data[13]	=	'D';
		data[14]	=	0x54;
		data[15]	=	0x00;
		data[16]	=	0x14;
		data[17]	=	0x00;
		data[18]	=	0x54;
		data[19]	=	0x00;
		data[20]	=	0xC6;
		data[21]	=	0x00;
		data[22]	=	0x1B;
		data[23]	=	'G';
		data[24]	=	'D';
		data[25]	=	0xB8;
		data[26]	=	0x00;
		data[27]	=	0x14;
		data[28]	=	0x00;
		data[29]	=	0xB8;
		data[30]	=	0x00;
		data[31]	=	0xC6;
		data[32]	=	0x00;
		data[33]	=	0x1B;
		data[34]	=	'G';
		data[35]	=	'D';
		data[36]	=	0xCC;
		data[37]	=	0x00;
		data[38]	=	0x14;
		data[39]	=	0x00;
		data[40]	=	0xCC;
		data[41]	=	0x00;
		data[42]	=	0xC6;
		data[43]	=	0x00;
		
		DisplaySendNBytes( data, 44 );
	}
	else
	{
		data[0]		=	0x1B;
		data[1]		=	'G';
		data[2]		=	'D';
		data[3]		=	0x68;
		data[4]		=	0x00;
		data[5]		=	0x14;
		data[6]		=	0x00;
		data[7]		=	0x68;
		data[8]		=	0x00;
		data[9]		=	0xC6;
		data[10]	=	0x00;
		data[11]	=	0x1B;
		data[12]	=	'G';
		data[13]	=	'D';
		data[14]	=	0x90;
		data[15]	=	0x00;
		data[16]	=	0x14;
		data[17]	=	0x00;
		data[18]	=	0x90;
		data[19]	=	0x00;
		data[20]	=	0xC6;
		data[21]	=	0x00;
		
		DisplaySendNBytes( data, 22 );
	}
	
	data[0]		=	0x1B;
	data[1]		=	'G';
	data[2]		=	'M';
	data[3]		=	0x00;
	data[4]		=	0x1B;
	data[5]		=	'F';
	data[6]		=	'G';
	data[7]		=	0x07;
	data[8]		=	0x01;
	data[9]		=	0x1B;
	data[10]	=	'G';
	data[11]	=	'Z';
	data[12]	=	0x01;
	data[13]	=	0x01;
	
	DisplaySendNBytes( data, 14 );		
}	

/** DISPLAY SHOW DATA ENABLE NEXT PAGE ***********************************/
void DisplayShowDataEnableNextPage( void )
{
	//Button durch "Vorspulen"-Pfeile ersetzen
	data[0]		=	0x1B;
	data[1]		=	'U';
	data[2]		=	'I';
	data[3]		=	0x60;
	data[4]		=	0x00;
	data[5]		=	0xDC;
	data[6]		=	0x00;
	data[7]		=	0x0A;
	//Knopf auf Zustand NOT_IN_USE setzen
	data[8]		=	0x1B;
	data[9]		=	'Y';
	data[10]	=	'X';
	data[11]	=	0x02;
	data[12]	=	0x6B;
	
	DisplaySendNBytes( data, 13 );
}

/** DISPLAY SHOW DAY VALUE FIRST ***********************************/
void DisplayShowDayValueFirst( unsigned char x1Pos, unsigned char x2Pos, unsigned char yPos )
{
	data[0]		=	0x1B;
	data[1]		=	'F';
	data[2]		=	'G';
	data[3]		=	0x03;
	data[4]		=	0x01;
	
	data[5]		=	0x1B;
	data[6]		=	'G';
	data[7]		=	'D';
	data[8]		=	x2Pos;
	data[9]		=	x1Pos;
	data[10]	=	yPos;
	data[11]	=	0x00;
	data[12]	=	x2Pos;
	data[13]	=	x1Pos;
	data[14]	=	yPos;
	data[15]	=	0x00;
		
	data[16]	=	0x1B;
	data[17]	=	'F';
	data[18]	=	'G';
	data[19]	=	0x07;
	data[20]	=	0x01;
	
	DisplaySendNBytes( data, 21 );
}

/** DISPLAY SHOW DAY VALUE ***********************************/
void DisplayShowDayValue( unsigned char x1Pos, unsigned char x2Pos, unsigned char yPos )
{
	data[0]		=	0x1B;
	data[1]		=	'F';
	data[2]		=	'G';
	data[3]		=	0x03;
	data[4]		=	0x01;
	
	data[5]		=	0x1B;
	data[6]		=	'G';
	data[7]		=	'W';
	data[8]		=	x2Pos;
	data[9]		=	x1Pos;
	data[10]	=	yPos;
	data[11]	=	0x00;
	
	data[12]	=	0x1B;
	data[13]	=	'F';
	data[14]	=	'G';
	data[15]	=	0x07;
	data[16]	=	0x01;
	
	DisplaySendNBytes( data, 17 );
}	

/** DISPLAY SHOW DATA DATUM ***********************************/
void DisplayShowDataDatum( void )
{
	//blaues Rechteck zeichnen
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'F';
	data[3]		=	0x3C;
	data[4]		=	0x00;
	data[5]		=	0x01;
	data[6]		=	0x00;
	data[7]		=	0x91;
	data[8]		=	0x00;
	data[9]		=	0x14;
	data[10]	=	0x00;
	data[11]	=	0x07;
	
	//Schrift weiss, transparent
	data[12]	=	0x1B;
	data[13]	=	'F';
	data[14]	=	'Z';
	data[15]	=	0x08;
	data[16]	=	0x00;
	
	//Datum schreiben
	data[17]	=	0x1B;
	data[18]	=	'Z';
	data[19]	=	'L';
	data[20]	=	0x3C;
	data[21]	=	0x00;
	data[22]	=	0x04;
	data[23]	=	0x00;
	data[24]	=	readDirectory[18];
	data[25]	=	readDirectory[19];
	data[26]	=	'.';
	data[27]	=	readDirectory[15];
	data[28]	=	readDirectory[16];
	data[29]	=	'.';
	data[30]	=	readDirectory[10];
	data[31]	=	readDirectory[11];
	data[32]	=	readDirectory[12];
	data[33]	=	readDirectory[13];
	data[34]	=	0x00;
	
	//Schrift schwarz, transparent
	data[35]	=	0x1B;
	data[36]	=	'F';
	data[37]	=	'Z';
	data[38]	=	0x01;
	data[39]	=	0x00;
	
	DisplaySendNBytes( data, 40 );
}

/** DISPLAY SHOW DATA RESULT ***********************************/
void DisplayShowDataResult( char* readData, unsigned int position )
{
	unsigned int i			=	0;
	
	unsigned int xpos		=	position - 1;
	unsigned int ypos		=	position - 1;
	
	unsigned int x1pos		=	0;
	unsigned int x2pos		=	0;
	unsigned int y1pos		=	0;
	
	xpos					=	xpos % 36;
	ypos					=	ypos % 12;
	
	if( xpos <= 11 )
	{
		x1pos				=	0x14;
	}
	else if( xpos <= 23 )
	{
		x1pos				=	0xBE;
	}
	else if( xpos <= 35 )
	{
		x2pos				=	0x01;
		x1pos				=	0x68;
	}
	
	y1pos					=	0x19 + ( ypos * 0x10 );
	
	data[0]					=	0x1B;
	data[1]					=	'Z';
	data[2]					=	'L';
	data[3]					=	x1pos;
	data[4]					=	x2pos;
	data[5]					=	y1pos;
	data[6]					=	0x00;
	
	for( i = 0; i < 14; i++ )
	{
		data[i+7]			=	readData[i];
	}	
	
	data[21]				=	0x00;
	
	DisplaySendNBytes( data, 22 );
	
	if( y1pos == 11 )
	{
		displayWait			=	0;
		while( displayWait < 2 ) { Nop(); }
	}	
}

/** DISPLAY CLEAR DATA CURSOR ***********************************/
void DisplayClearDataCursor( void )
{
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'L';
	data[3]		=	0x09;
	data[4]		=	0x00;
	data[5]		=	0x18;
	data[6]		=	0x00;
	data[7]		=	0x11;
	data[8]		=	0x00;
	data[9]		=	0xDB;
	data[10]	=	0x00;
	
	DisplaySendNBytes( data, 11 );
	
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'L';
	data[3]		=	0xB9;
	data[4]		=	0x00;
	data[5]		=	0x18;
	data[6]		=	0x00;
	data[7]		=	0xBB;
	data[8]		=	0x00;
	data[9]		=	0xDB;
	data[10]	=	0x00;
	
	DisplaySendNBytes( data, 11 );
	
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'L';
	data[3]		=	0x63;
	data[4]		=	0x01;
	data[5]		=	0x18;
	data[6]		=	0x00;
	data[7]		=	0x65;
	data[8]		=	0x01;
	data[9]		=	0xDB;
	data[10]	=	0x00;
	
	DisplaySendNBytes( data, 11 );
}	

/** DISPLAY SHOW DATA CURSOR ***********************************/
void DisplayShowDataCursor( unsigned int position )
{
	unsigned int xpos		=	position - 1;
	unsigned int ypos		=	position - 1;
	
	unsigned int x1pos		=	0;
	unsigned int x2pos		=	0;
	unsigned int y1pos		=	0;
	
	xpos					=	xpos % 36;
	ypos					=	ypos % 12;
	
	if( xpos <= 11 )
	{
		x1pos				=	0x10;
	}
	else if( xpos <= 23 )
	{
		x1pos				=	0xBA;
	}
	else if( xpos <= 35 )
	{
		x2pos				=	0x01;
		x1pos				=	0x64;
	}
	
	y1pos					=	0x19 + ( ypos * 0x10 );
	
	data[0]					=	0x1B;
	data[1]					=	'F';
	data[2]					=	'G';
	data[3]					=	0x07;
	data[4]					=	0x01;
	
	DisplaySendNBytes( data, 5 );
	
	data[0]					=	0x1B;
	data[1]					=	'G';
	data[2]					=	'D';
	data[3]					=	x1pos;
	data[4]					=	x2pos;
	data[5]					=	y1pos;
	data[6]					=	0x00;
	data[7]					=	x1pos;
	data[8]					=	x2pos;
	data[9]					=	y1pos + 0x0E;
	data[10]				=	0x00;
	
	DisplaySendNBytes( data, 11 );	
	
	data[0]					=	0x1B;
	data[1]					=	'F';
	data[2]					=	'G';
	data[3]					=	0x01;
	data[4]					=	0x01;
	
	DisplaySendNBytes( data, 5 );
}

/** DISPLAY SHOW MENGE ***********************************/
void DisplayShowMenge( void )
{
	itoa( menge, buffer );

	//Rechteck löschen von 121,27 nach 244,47
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x79;
	data[4]		=	0x00;	//x1=121
	data[5]		=	0x1B;
	data[6]		=	0x00;	//y1=27
	data[7]		=	0xF4;
	data[8]		=	0x00;	//x2=244
	data[9]		=	0x2F;
	data[10]	=	0x00;	//y2=47
	//Menge
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x7D;
	data[15]	=	0x00;	//x=125
	data[16]	=	0x1E;
	data[17]	=	0x00;	//y=30
	if( menge < 1000 )
	{
		data[18]	=	0x30;
		data[19]	=	buffer[0];
		data[20]	=	buffer[1];
		data[21]	=	buffer[2];
	}
	else
	{
		data[18]	=	0x31;
		data[19]	=	0x30;
		data[20]	=	0x30;
		data[21]	=	0x30;
	}	
	data[22]	=	0x20;	//SPACE
	data[23]	=	0x6E;	//n
	data[24]	=	0x67;	//g ... Menge
	data[25]	=	0x00;	//NUL

	DisplaySendNBytes( data, 26 );
}

/** DISPLAY SHOW VOLUMEN ***********************************/
void DisplayShowVolumen( void )
{
	itoa( volumen, buffer );

	//Rechteck löschen von 121,47 nach 244,67
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x79;
	data[4]		=	0x00;	//x1=121
	data[5]		=	0x2F;
	data[6]		=	0x00;	//y1=47
	data[7]		=	0xF4;
	data[8]		=	0x00;	//x2=244
	data[9]		=	0x43;
	data[10]	=	0x00;	//y2=67
	//Volumen
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x7D;
	data[15]	=	0x00;	//x=125
	data[16]	=	0x32;
	data[17]	=	0x00;	//y=50
	if( volumen < 100 )
	{
		data[18]	=	0x30;
		data[19]	=	buffer[0];
		data[20]	=	'.';
		data[21]	=	buffer[1];
	}
	else
	{
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	'.';
		data[21]	=	buffer[2];
	}
	data[22]	=	0x20;	//SPACE
	data[23]	=	0x6D;	//m
	data[24]	=	0x6C;	//l
	data[25]	=	0x00;	//NUL

	DisplaySendNBytes( data, 26 );
}

/** DISPLAY SHOW DATUM ***********************************/
void DisplayShowDatum( short i )
{
	if( i == 0 )
	{
		ClockReadDay();
		ClockReadMonth();
		ClockReadYear();
	}

	//Rechteck löschen von 121,67 nach 244,83
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x79;
	data[4]		=	0x00;	//x1=121
	data[5]		=	0x43;
	data[6]		=	0x00;	//y1=67
	data[7]		=	0xF4;
	data[8]		=	0x00;	//x2=244
	data[9]		=	0x53;
	data[10]	=	0x00;	//y2=83
	//Datum
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x7D;
	data[15]	=	0x00;	//x=125
	data[16]	=	0x46;
	data[17]	=	0x00;	//y=70
	data[18]	=	( 0x30 + day1 );
	data[19]	=	( 0x30 + day2 );
	data[20]	=	0x2E;
	data[21]	=	( 0x30 + month1 );
	data[22]	=	( 0x30 + month2 );
	data[23]	=	0x2E;
	data[24]	=	0x32;
	data[25]	=	0x30;
	data[26]	=	( 0x30 + year1 );
	data[27]	=	( 0x30 + year2 );
	data[28]	=	0x00;	//NUL

	DisplaySendNBytes( data, 29 );
}

/** DISPLAY SHOW UHRZEIT ***********************************/
void DisplayShowUhrzeit( short i, short j )
{
	if( j == 0 )
	{
		ClockReadMinutes();
		ClockReadHours();
	}

	if( i == 0 )
	{
		//Rechteck löschen von 400,7 nach 478,27
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x90;
		data[4]		=	0x01;	//x1=400
		data[5]		=	0x07;
		data[6]		=	0x00;	//y1=7
		data[7]		=	0xDE;
		data[8]		=	0x01;	//x2=478
		data[9]		=	0x1B;
		data[10]	=	0x00;	//y2=27
		//Uhrzeit
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x90;
		data[15]	=	0x01;	//x=400
		data[16]	=	0x0A;
		data[17]	=	0x00;	//y=10
	}
	else if( i == 1 )
	{
		//Rechteck löschen von 121,87 nach 244,103
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x79;
		data[4]		=	0x00;	//x1=121
		data[5]		=	0x57;
		data[6]		=	0x00;	//y1=87
		data[7]		=	0xF4;
		data[8]		=	0x00;	//x2=244
		data[9]		=	0x67;
		data[10]	=	0x00;	//y2=103
		//Uhrzeit
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x7D;
		data[15]	=	0x00;	//x=125
		data[16]	=	0x5A;
		data[17]	=	0x00;	//y=90
	}
	data[18]	=	( 0x30 + hrs1 );
	data[19]	=	( 0x30 + hrs2 );
	data[20]	=	0x3A;
	data[21]	=	( 0x30 + min1 );
	data[22]	=	( 0x30 + min2 );
	data[23]	=	0x00;	//NUL

	DisplaySendNBytes( data, 24 );
}

/** DISPLAY SHOW STARTZEIT ***********************************/
void DisplayShowStartzeit( void )
{
	//Rechteck löschen von 400,112 nach 478,127
	data[0]		=	0x1B;
	data[1]		=	'R';
	data[2]		=	'L';
	data[3]		=	0x90;
	data[4]		=	0x01;	//x1=400
	data[5]		=	0x70;
	data[6]		=	0x00;	//y1=112
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x7F;
	data[10]	=	0x00;	//y2=127
	//Startzeit
	data[11]	=	0x1B;
	data[12]	=	'Z';
	data[13]	=	'L';
	data[14]	=	0x90;
	data[15]	=	0x01;	//x=400
	data[16]	=	0x70;
	data[17]	=	0x00;	//y=112
	
	if( autoMeasurement )
	{
		data[18]	=	uhrzeitNaechsteMessung[0];
		data[19]	=	uhrzeitNaechsteMessung[1];
		data[20]	=	uhrzeitNaechsteMessung[2];
		data[21]	=	uhrzeitNaechsteMessung[3];
		data[22]	=	uhrzeitNaechsteMessung[4];
	}
	else
	{
		data[18]	=	'-';
		data[19]	=	'-';
		data[20]	=	':';
		data[21]	=	'-';
		data[22]	=	'-';
	}
	
	data[23]	=	0x00;

	DisplaySendNBytes( data, 24 );
}

/** DISPLAY SHOW SEKUNDEN ***********************************/
void DisplayShowSekunden( void )
{
	itoa( sekunden, buffer );
	
	//Rechteck löschen von 400,7 nach 478,27
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x90;
	data[4]		=	0x01;	//x1=400
	data[5]		=	0x07;
	data[6]		=	0x00;	//y1=7
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x1B;
	data[10]	=	0x00;	//y2=27
	//Sekunden
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x90;
	data[15]	=	0x01;	//x=400
	data[16]	=	0x0A;
	data[17]	=	0x00;	//y=10
	if( sekunden < 10 )
	{
		data[18]	=	buffer[0];
		data[19]	=	0x20;	//SPACE
		data[20]	=	's';
		data[21]	=	'e';
		#if defined( DEUTSCH )
		data[22]	=	'k';
		#endif
		#if defined( ENGLISCH )
		data[22]	=	'c';
		#endif
		data[23]	=	0x20;
		data[24]	=	0x20;
	}
	else if( sekunden < 100 )
	{
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	0x20;
		data[21]	=	's';
		data[22]	=	'e';
		#if defined( DEUTSCH )
		data[23]	=	'k';
		#endif
		#if defined( ENGLISCH )
		data[23]	=	'c';
		#endif
		data[24]	=	0x20;
	}
	else
	{
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	buffer[2];
		data[21]	=	0x20;
		data[22]	=	's';
		data[23]	=	'e';
		#if defined( DEUTSCH )
		data[24]	=	'k';
		#endif
		#if defined( ENGLISCH )
		data[24]	=	'c';
		#endif
	}		
	data[25]	=	0x00;	//NUL

	DisplaySendNBytes( data, 26 );
}

/** DISPLAY SHOW TEMPERATUR ***********************************/
/*
 *	Falls eine 0 übergeben wird, wird die Temperatur auf der 
 *	Position des Hauptbildschirms ausgegeben, bei einer 1
 *	auf der Position der Betriebsparameter
 */
void DisplayShowTemperatur( short i )
{
	if( i == 0 )					//Anzeige Temperatur Hauptbildschirm
	{
		SystemSaeuleIstwert();
		itoa( saeule_istwert, buffer );
		//Rechteck löschen von 400,78 nach 478,93
		data[0]		=	0x1B;
		data[1]		=	'R';
		data[2]		=	'L';
		data[3]		=	0x90;
		data[4]		=	0x01;	//x1=400
		data[5]		=	0x4E;
		data[6]		=	0x00;	//y1=78
		data[7]		=	0xDE;
		data[8]		=	0x01;	//x2=478
		data[9]		=	0x5D;
		data[10]	=	0x00;	//y2=93
		//Temperaturposition
		data[11]	=	0x1B;
		data[12]	=	'Z';
		data[13]	=	'L';
		data[14]	=	0x90;
		data[15]	=	0x01;	//x=400
		data[16]	=	0x4E;
		data[17]	=	0x00;	//y=78
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	0x2E;	//.
		data[21]	=	buffer[2];
		data[22]	=	' ';
		data[23]	=	0xF8;	//°
		data[24]	=	'C';
		data[25]	=	0x00;	//NUL

		DisplaySendNBytes( data, 26 );
	}
	else if( i == 1 )				//Anzeige Temperatur Betriebsparameter
	{
		itoa( temperatur, buffer );
		//Rechteck löschen von 121,107 nach 244,127
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x79;
		data[4]		=	0x00;	//x1=121
		data[5]		=	0x6B;
		data[6]		=	0x00;	//y1=107
		data[7]		=	0xF4;
		data[8]		=	0x00;	//x2=244
		data[9]		=	0x7F;
		data[10]	=	0x00;	//y2=127
		//Temperaturposition
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x7D;
		data[15]	=	0x00;	//x=125
		data[16]	=	0x6E;
		data[17]	=	0x00;	//y=110
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	0x20;	//SPACE
		data[21]	=	0xF8;	//°
		data[22]	=	0x43;	//C
		data[23]	=	0x00;	//NUL

		DisplaySendNBytes( data, 24 );
	}
	else if( i == 2 )				//Anzeige Betriebstemperatur Startup
	{
		itoa( temperatur, buffer );
		//Rechteck zeichnen von 197,197 nach 255,217
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x46;	//F
		data[3]		=	0xC5;
		data[4]		=	0x00;	//x1=197
		data[5]		=	0xC5;
		data[6]		=	0x00;	//y1=197
		data[7]		=	0xFF;
		data[8]		=	0x00;	//x2=255
		data[9]		=	0xD9;
		data[10]	=	0x00;	//y2=217
		data[11]	=	0x07;	//YELLOW
		//Temperaturposition
		data[12]	=	0x1B;	//ESC
		data[13]	=	0x5A;	//Z
		data[14]	=	0x4C;	//L
		data[15]	=	0xC8;
		data[16]	=	0x00;	//x=200
		data[17]	=	0xC8;
		data[18]	=	0x00;	//y=200
		data[19]	=	buffer[0];
		data[20]	=	buffer[1];
		data[21]	=	0x20;	//SPACE
		data[22]	=	0xF8;	//°
		data[23]	=	0x43;	//C
		data[24]	=	0x00;	//NUL

		DisplaySendNBytes( data, 25 );
	}
	else if( i == 3 )				//Anzeige aktuelle Temperatur Startup
	{
		itoa( saeule_istwert, buffer );
		//Rechteck zeichnen von 197,177 nach 256,197
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x46;	//F
		data[3]		=	0xC5;
		data[4]		=	0x00;	//x1=197
		data[5]		=	0xB1;
		data[6]		=	0x00;	//y1=177
		data[7]		=	0xFF;
		data[8]		=	0x00;	//x2=255
		data[9]		=	0xC5;
		data[10]	=	0x00;	//y2=197
		data[11]	=	0x07;	//YELLOW
		//Temperaturposition
		data[12]	=	0x1B;	//ESC
		data[13]	=	0x5A;	//Z
		data[14]	=	0x4C;	//L
		data[15]	=	0xC8;
		data[16]	=	0x00;	//x=200
		data[17]	=	0xB4;
		data[18]	=	0x00;	//y=180
		data[19]	=	buffer[0];
		data[20]	=	buffer[1];
		data[21]	=	0x20;	//SPACE
		data[22]	=	0xF8;	//°
		data[23]	=	0x43;	//C
		data[24]	=	0x00;	//NUL

		DisplaySendNBytes( data, 25 );
	}
	else if( i == 4 )				//Anzeige Temperaturänderung Betriebsparameter
	{
		itoa( saeule_vorgabe, buffer );
		//Rechteck löschen von 121,107 nach 244,127
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x79;
		data[4]		=	0x00;	//x1=121
		data[5]		=	0x6B;
		data[6]		=	0x00;	//y1=107
		data[7]		=	0xF4;
		data[8]		=	0x00;	//x2=244
		data[9]		=	0x7F;
		data[10]	=	0x00;	//y2=127
		//Temperaturposition
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x7D;
		data[15]	=	0x00;	//x=125
		data[16]	=	0x6E;
		data[17]	=	0x00;	//y=110
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	0x20;	//SPACE
		data[21]	=	0xF8;	//°
		data[22]	=	0x43;	//C
		data[23]	=	0x00;	//NUL

		DisplaySendNBytes( data, 24 );
	}
}

/** DISPLAY SHOW OK ********************************************/
void DisplayShowOK( int i )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x5A;	//Z
	data[2]		=	0x4C;	//L
	if( i == 0 )
	{
		data[3]		=	0xFA;
		data[4]		=	0x00;
		data[5]		=	0xB4;
		data[6]		=	0x00;
	}
	else if( i == 1 )
	{
		data[3]		=	0xFA;
		data[4]		=	0x00;
		data[5]		=	0xF0;
		data[6]		=	0x00;
	}
	data[7]		=	'O';
	data[8]		=	'K';
	data[9]		=	0x00;
	
	DisplaySendNBytes( data, 10 );
}	

/** DISPLAY SHOW MAIN ******************************************/
void DisplayShowMain( void )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	'M';
	data[2]		=	'N';
	data[3]		=	0x01;
	
	DisplaySendNBytes( data, 4 );
}	

/** DISPLAY SHOW DRUCK *****************************************/
/*
 *	Falls eine 0 übergeben wird, wird der Druck auf der 
 *	Position des Hauptbildschirms ausgegeben, bei einer 1
 *	auf der Position der Betriebsparameter
 */
void DisplayShowDruck( short i )
{
	if( i == 0 )
	{
		SystemVordruckIstwert();
		itoa( vordruck_istwert, buffer );
		//Rechteck löschen von 400,61 nach 478,76
		data[0]		=	0x1B;
		data[1]		=	'R';
		data[2]		=	'L';
		data[3]		=	0x90;
		data[4]		=	0x01;	//x1=400
		data[5]		=	0x3D;
		data[6]		=	0x00;	//y1=61
		data[7]		=	0xDE;
		data[8]		=	0x01;	//x2=478
		data[9]		=	0x4C;
		data[10]	=	0x00;	//y2=76
		//Druckposition
		data[11]	=	0x1B;
		data[12]	=	'Z';
		data[13]	=	'L';
		data[14]	=	0x90;
		data[15]	=	0x01;	//x=400
		data[16]	=	0x3D;
		data[17]	=	0x00;	//y=61
	}
	else if( i == 1 )
	{
		itoa( druck, buffer );
		//Rechteck löschen von 361,7 nach 478,27
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x69;
		data[4]		=	0x01;	//x1=361
		data[5]		=	0x07;
		data[6]		=	0x00;	//y1=7
		data[7]		=	0xDE;
		data[8]		=	0x01;	//x2=478
		data[9]		=	0x1B;
		data[10]	=	0x00;	//y2=27
		//Druckposition
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x6D;
		data[15]	=	0x01;	//x=365
		data[16]	=	0x0A;
		data[17]	=	0x00;	//y=10
	}
	else if( i == 2 )
	{
		itoa( vordruck_vorgabe, buffer );
		//Rechteck löschen von 361,7 nach 478,27
		data[0]		=	0x1B;	//ESC
		data[1]		=	0x52;	//R
		data[2]		=	0x4C;	//L
		data[3]		=	0x69;
		data[4]		=	0x01;	//x1=361
		data[5]		=	0x07;
		data[6]		=	0x00;	//y1=7
		data[7]		=	0xDE;
		data[8]		=	0x01;	//x2=478
		data[9]		=	0x1B;
		data[10]	=	0x00;	//y2=27
		//Druckposition
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x6D;
		data[15]	=	0x01;	//x=365
		data[16]	=	0x0A;
		data[17]	=	0x00;	//y=10
	}
	data[18]	=	buffer[0];
	data[19]	=	buffer[1];
	data[20]	=	0x20;	//SPACE
	data[21]	=	0x6D;	//m
	data[22]	=	0x62;	//b
	data[23]	=	0x61;	//a
	data[24]	=	0x72;	//r
	data[25]	=	0x00;	//NUL

	DisplaySendNBytes( data, 26 );

	pressure	=	( pressure >> 1 ) & 0x0FFF;
	SystemPause();
	SystemSetMA2( (unsigned int) pressure );
}

/** DISPLAY SHOW EMPFINDLICHKEIT ***********************************/
void DisplayShowEmpfindlichkeit( void )
{
	itoa( empfindlichkeit_vorgabe, buffer );

	//Rechteck löschen von 361,27 nach 478,47
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x69;
	data[4]		=	0x01;	//x1=361
	data[5]		=	0x1B;
	data[6]		=	0x00;	//y1=27
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x2F;
	data[10]	=	0x00;	//y2=47
	//Empfindlichkeit
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x6D;
	data[15]	=	0x01;	//x=365
	data[16]	=	0x1E;
	data[17]	=	0x00;	//y=30
	data[18]	=	buffer[0];
	data[19]	=	0x00;	//NUL

	DisplaySendNBytes( data, 20 );
}

/** DISPLAY SHOW MESSZYKLUS ***********************************/
void DisplayShowMesszyklus( void )
{
	itoa( messzyklus, buffer );

	//Rechteck löschen von 361,47 nach 478,67
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x69;
	data[4]		=	0x01;	//x1=361
	data[5]		=	0x2F;
	data[6]		=	0x00;	//y1=47
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x43;
	data[10]	=	0x00;	//y2=67
	//Messzyklus
	if( messzyklus == 20 || messzyklus == 40 )
	{
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x6D;
		data[15]	=	0x01;	//x=365
		data[16]	=	0x32;
		data[17]	=	0x00;	//y=50
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	0x20;	//SPACE
		data[21]	=	0x6D;	//m
		data[22]	=	0x69;	//i
		data[23]	=	0x6E;	//n
		data[24]	=	0x00;	//NUL
	}
	else
	{
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x6D;
		data[15]	=	0x01;	//x=365
		data[16]	=	0x32;
		data[17]	=	0x00;	//y=50
		if( messzyklus < 10 )
		{
			data[18]	=	0x30;
			data[19]	=	buffer[0];
		}	
		else if( messzyklus >= 10 && messzyklus < 20 )
		{
			data[18]	=	buffer[0];
			data[19]	=	buffer[1];
		}	
		else
		{
			data[18]	=	0x32;
			if( buffer[1] == 0x31 )
			{
				data[19]	=	0x30;
			}
			else if( buffer[1] == 0x32 )
			{
				data[19]	=	0x31;
			}
			else if( buffer[1] == 0x33 )
			{
				data[19]	=	0x32;
			}
			else if( buffer[1] == 0x34 )
			{
				data[19]	=	0x33;
			}
			else if( buffer[1] == 0x35 )
			{
				data[19]	=	0x34;
			}					
		}	
		data[20]	=	0x20;	//SPACE
		#if defined( DEUTSCH )
		data[21]	=	0x53;	//S
		data[22]	=	0x74;	//t
		data[23]	=	0x64;	//d
		#endif
		#if defined( ENGLISCH )
		data[21]	=	'h';
		data[22]	=	'r';
		data[23]	=	's';
		#endif
		data[24]	=	0x00;	//NUL
	}	

	DisplaySendNBytes( data, 25 );
}

/** DISPLAY SHOW EICHZYKLUS ***********************************/
void DisplayShowEichzyklus( void )
{
	itoa( eichzyklus, buffer );

	//Rechteck löschen von 361,67 nach 478,87
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x69;
	data[4]		=	0x01;	//x1=361
	data[5]		=	0x43;
	data[6]		=	0x00;	//y1=67
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x57;
	data[10]	=	0x00;	//y2=87
	//Eichzyklus
	if( eichzyklus == 20 || eichzyklus == 40 )
	{
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x6D;
		data[15]	=	0x01;	//x=365
		data[16]	=	0x46;
		data[17]	=	0x00;	//y=70
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	0x20;	//SPACE
		data[21]	=	0x6D;	//m
		data[22]	=	0x69;	//i
		data[23]	=	0x6E;	//n
		data[24]	=	0x00;	//NUL
	}
	else
	{
		data[11]	=	0x1B;	//ESC
		data[12]	=	0x5A;	//Z
		data[13]	=	0x4C;	//L
		data[14]	=	0x6D;
		data[15]	=	0x01;	//x=365
		data[16]	=	0x46;
		data[17]	=	0x00;	//y=70
		if( eichzyklus < 10 )
		{
			data[18]	=	0x30;
			data[19]	=	buffer[0];
		}	
		else if( eichzyklus >= 10 && eichzyklus < 20 )
		{
			data[18]	=	buffer[0];
			data[19]	=	buffer[1];
		}	
		else
		{
			data[18]	=	0x32;
			if( buffer[1] == 0x31 )
			{
				data[19]	=	0x30;
			}
			else if( buffer[1] == 0x32 )
			{
				data[19]	=	0x31;
			}
			else if( buffer[1] == 0x33 )
			{
				data[19]	=	0x32;
			}
			else if( buffer[1] == 0x34 )
			{
				data[19]	=	0x33;
			}
			else if( buffer[1] == 0x35 )
			{
				data[19]	=	0x34;
			}					
		}	
		data[20]	=	0x20;	//SPACE
		#if defined( DEUTSCH )
		data[21]	=	0x53;	//S
		data[22]	=	0x74;	//t
		data[23]	=	0x64;	//d
		#endif
		#if defined( ENGLISCH )
		data[21]	=	'h';
		data[22]	=	'r';
		data[23]	=	's';
		#endif
		data[24]	=	0x00;	//NUL
	}	

	DisplaySendNBytes( data, 25 );
}

/** DISPLAY SHOW SPUELZEIT ***********************************/
void DisplayShowSpuelzeit( void )
{
	itoa( spuelzeit, buffer );

	//Rechteck löschen von 361,87 nach 478,107
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x69;
	data[4]		=	0x01;	//x1=361
	data[5]		=	0x57;
	data[6]		=	0x00;	//y1=87
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x6B;
	data[10]	=	0x00;	//y2=107
	//Spuelzeit
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x6D;
	data[15]	=	0x01;	//x=365
	data[16]	=	0x5A;
	data[17]	=	0x00;	//y=90
	data[18]	=	buffer[0];
	data[19]	=	buffer[1];
	data[20]	=	0x20;	//SPACE
	data[21]	=	0x73;	//s
	data[22]	=	0x65;	//e
	#if defined( DEUTSCH )
	data[23]	=	0x6B;	//k
	#endif
	#if defined( ENGLISCH )
	data[23]	=	'c';
	#endif
	data[24]	=	0x00;	//NUL

	DisplaySendNBytes( data, 25 );
}

/** DISPLAY SHOW SPLITZEIT ***********************************/
void DisplayShowSplitzeit( void )
{
	itoa( splitzeit, buffer );

	//Rechteck löschen von 361,147 nach 478,167
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x69;
	data[4]		=	0x01;	//x1=361
	data[5]		=	0x93;
	data[6]		=	0x00;	//y1=147
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0xA7;
	data[10]	=	0x00;	//y2=167
	//Splitzeit
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x6D;
	data[15]	=	0x01;	//x=365
	data[16]	=	0x96;
	data[17]	=	0x00;	//y=150
	if( splitzeit < 10 )
	{
		data[18]	=	0x30;
		data[19]	=	0x30;
		data[20]	=	buffer[0];
	}
	else if( splitzeit < 100 )
	{
		data[18]	=	0x30;
		data[19]	=	buffer[0];
		data[20]	=	buffer[1];
	}	
	else
	{
		data[18]	=	buffer[0];
		data[19]	=	buffer[1];
		data[20]	=	buffer[2];
	}	
	data[21]	=	0x20;	//SPACE
	data[22]	=	0x73;	//s
	data[23]	=	0x65;	//e
	#if defined( DEUTSCH )
	data[24]	=	0x6B;	//k
	#endif
	#if defined( ENGLISCH )
	data[24]	=	'c';
	#endif
	data[25]	=	0x00;	//NUL

	DisplaySendNBytes( data, 26 );
}

/** DISPLAY SHOW KORREKTURFAKTOR ***********************************/
void DisplayShowKorrekturfaktor( void )
{
	itoa( korrekturfaktor, buffer );

	//Rechteck löschen von 366, 107 nach 478, 127
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x6E;
	data[4]		=	0x01;	//x1=366
	data[5]		=	0x6B;
	data[6]		=	0x00;	//y1=107
	data[7]		=	0xDE;
	data[8]		=	0x01;	//x2=478
	data[9]		=	0x7F;
	data[10]	=	0x00;	//y2=127
	//Spuelzeit
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x6E;
	data[15]	=	0x01;	//x=366
	data[16]	=	0x6E;
	data[17]	=	0x00;	//y=110
	if( korrekturfaktor < 100 )
	{
		data[18]	=	0x30;
		data[19]	=	0x2E;
		data[20]	=	buffer[0];
		data[21]	=	buffer[1];
	}
	else
	{
		data[18]	=	buffer[0];
		data[19]	=	0x2E;
		data[20]	=	buffer[1];
		data[21]	=	buffer[2];
	}
	data[22]	=	0x00;	//NUL

	DisplaySendNBytes( data, 23 );
}	

/** DISPLAY SHOW UEBERTRAGUNGSART ***********************************/
void DisplayShowUebertragungsart( void )
{
	//Rechteck löschen von 121,147 nach 245,167
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x79;
	data[4]		=	0x00;	//x1=121
	data[5]		=	0x93;
	data[6]		=	0x00;	//y1=147
	data[7]		=	0xF5;
	data[8]		=	0x00;	//x2=245
	data[9]		=	0xA7;
	data[10]	=	0x00;	//y2=167
	//Uebertragungsart
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x7D;
	data[15]	=	0x00;	//x=125
	data[16]	=	0x96;
	data[17]	=	0x00;	//y=150
	if( uebertragungsart == TRUE )
	{
		data[18]	=	0x34;
		data[19]	=	0x2D;
		data[20]	=	0x32;
		data[21]	=	0x30;
	}
	else
	{
		data[18]	=	0x30;
		data[19]	=	0x2D;
		data[20]	=	0x32;
		data[21]	=	0x30;
	}
	data[22]	=	0x20;	//SPACE
	data[23]	=	0x6D;	//m
	data[24]	=	0x41;	//A
	data[25]	=	0x00;	//NUL

	DisplaySendNBytes( data, 26 );
}

/** DISPLAY SHOW AUTOMESSUNG ***********************************/
void DisplayShowAutomessung( void )
{
	//Rechteck löschen von 364,127 nach 470,147
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x6C;
	data[4]		=	0x01;	//x1=364
	data[5]		=	0x7F;
	data[6]		=	0x00;	//y1=127
	data[7]		=	0xD6;
	data[8]		=	0x01;	//x2=470
	data[9]		=	0x93;
	data[10]	=	0x00;	//y2=147
	//Automessung
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x6D;
	data[15]	=	0x01;	//x=365
	data[16]	=	0x82;
	data[17]	=	0x00;	//y=130
	if( autoMeasurement == TRUE )
	{
		#if defined( DEUTSCH )
		data[18]	=	0x41;	//A
		data[19]	=	0x6E;	//n
		#endif
		#if defined( ENGLISCH )
		data[18]	=	'O';
		data[19]	=	'n';
		#endif
		data[20]	=	0x20;	//SPACE
	}
	else
	{
		#if defined( DEUTSCH )
		data[18]	=	0x41;	//A
		data[19]	=	0x75;	//u
		data[20]	=	0x73;	//s
		#endif
		#if defined( ENGLISCH )
		data[18]	=	'O';
		data[19]	=	'f';
		data[20]	=	'f';
		#endif
	}
	data[21]	=	0x00;	//NUL

	DisplaySendNBytes( data, 22 );
}

/** DISPLAY SHOW SKALIERUNG ***********************************/
void DisplayShowSkalierung( void )
{
	itoa( skalierung, buffer );

	//Rechteck löschen von 121,167 nach 245,187
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x79;
	data[4]		=	0x00;	//x1=121
	data[5]		=	0xA7;
	data[6]		=	0x00;	//y1=167
	data[7]		=	0xF5;
	data[8]		=	0x00;	//x2=245
	data[9]		=	0xBB;
	data[10]	=	0x00;	//y2=187
	//Skalierung
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x5A;	//Z
	data[13]	=	0x4C;	//L
	data[14]	=	0x7D;
	data[15]	=	0x00;	//x=125
	data[16]	=	0xAA;
	data[17]	=	0x00;	//y=170
	data[18]	=	buffer[0];
	data[19]	=	buffer[1];
	data[20]	=	0x20;	//SPACE
	data[21]	=	0x6D;	//m
	data[22]	=	0x67;	//g
	data[23]	=	0x00;	//NUL

	DisplaySendNBytes( data, 24 );
}

/** DISPLAY GET ERROR LOG ***********************************/
void DisplayGetErrorLog( unsigned long int logCount )
{	
	char filename[13];
	char buffer[8];
	
	FSFILE *log;
	char errorString[64]; 
	char displayString[64];
	char path[]	=	"\\ErrorLog\\0001\\01\\01";
	
	int i		=	0;
	int j		=	0;
	
	//ERRORLOGS VORHANDEN ?
	if( logCount < 1 )
	{
		DisplayShowNoErrorLogs();
		return;
	}
	
	FSchdir( ( char* ) 0x5C );
	if( FSchdir( path ) != 0 )
	{
		return;
	}	
	
	//VORBEREITEN DES STRINGS MIT DEM NAMEN DER TEXTDATEI
	filename[0]		=	'0';
	filename[1]		=	'0';
	filename[2]		=	'0';
	
	filename[8]		=	'.';
	filename[9]		=	't';
	filename[10]	=	'x';
	filename[11]	=	't';
	filename[12]	=	0x00;
	
	//SCHLEIFE, DIE MAXIMAL 10 ERRORLOG-EINTRÄGE HERAUSSUCHT
	for( i = 1; i <= 10; i++ )
	{
		litoa( logCount, buffer );
		if( logCount < 1 )
		{
			return;
		}
		else if( logCount < 10 )
		{
			filename[3]		=	'0';
			filename[4]		=	'0';
			filename[5]		=	'0';
			filename[6]		=	'0';
			filename[7]		=	buffer[0];
		}	
		else if( logCount < 100 )
		{
			filename[3]		=	'0';
			filename[4]		=	'0';
			filename[5]		=	'0';
			filename[6]		=	buffer[0];
			filename[7]		=	buffer[1];
		}
		else if( logCount < 1000 )
		{
			filename[3]		=	'0';
			filename[4]		=	'0';
			filename[5]		=	buffer[0];
			filename[6]		=	buffer[1];
			filename[7]		=	buffer[2];
		}
		else if( logCount < 10000 )
		{
			filename[3]		=	'0';
			filename[4]		=	buffer[0];
			filename[5]		=	buffer[1];
			filename[6]		=	buffer[2];
			filename[7]		=	buffer[3];
		}
		else 
		{
			filename[3]		=	buffer[0];
			filename[4]		=	buffer[1];
			filename[5]		=	buffer[2];
			filename[6]		=	buffer[3];
			filename[7]		=	buffer[4];
		}
		
		logCount--;
	
		log		=	FSfopen( filename, READ );
		FSfread( errorString, 64, 1, log );
		FSfclose( log );
		log		=	NULL;
		
		Nop();
		Nop();
		Nop();
		
		//STRING FÜR DISPLAY ZUSAMMENSETZEN
		displayString[0]		=	0x1B;
		displayString[1]		=	'Z';
		displayString[2]		=	'L';
		displayString[3]		=	0x0A;
		displayString[4]		=	0x00;
	
		//Y-POSITION BESTIMMEN
		if( i == 1 ) displayString[5] = 0x0A;
		else if( i == 2 ) displayString[5] = 0x1E;
		else if( i == 3 ) displayString[5] = 0x32;
		else if( i == 4 ) displayString[5] = 0x46;
		else if( i == 5 ) displayString[5] = 0x5A;
		else if( i == 6 ) displayString[5] = 0x6E;
		else if( i == 7 ) displayString[5] = 0x82;
		else if( i == 8 ) displayString[5] = 0x96;
		else if( i == 9 ) displayString[5] = 0xAA;
		else if( i == 10 ) displayString[5] = 0xBE;
		displayString[6]		=	0x00;
	
		//STRING KOPIEREN
		while( errorString[j] != 0x0A )
		{
			displayString[j+7]		=	errorString[j];
			j++;
		}
		displayString[j+7]		=	0x00;
		j+=8;
	
		DisplaySendNBytes( displayString, j );
		j		=	0;
	}
}	

/** DISPLAY SHOW ERROR LOG ***********************************/
void DisplayShowErrorLog( char* filename, int no )
{
	FSFILE *log;
	char errorString[64]; 
	char displayString[64];
	char path[]	=	"\\ErrorLog\\0001\\01\\01";
	int i		=	0;
	
	//DATEI ÖFFNEN, LESEN, SCHLIESSEN
	FSchdir( ( char* ) 0x5C );
	if( FSchdir( path ) != 0 )
	{
		return;
	}	
	log		=	FSfopen( *filename, READ );
	FSfread( errorString, 64, 1, log );
	FSfclose( log );
	log		=	NULL;
	
	//STRING FÜR DISPLAY ZUSAMMENSETZEN
	displayString[0]		=	0x1B;
	displayString[1]		=	'Z';
	displayString[2]		=	'L';
	displayString[3]		=	0x0A;
	displayString[4]		=	0x00;
	
	//Y-POSITION BESTIMMEN
	if( no == 1 ) displayString[5] = 0x0A;
	else if( no == 2 ) displayString[5] = 0x1E;
	else if( no == 3 ) displayString[5] = 0x32;
	else if( no == 4 ) displayString[5] = 0x46;
	else if( no == 5 ) displayString[5] = 0x5A;
	else if( no == 6 ) displayString[5] = 0x6E;
	else if( no == 7 ) displayString[5] = 0x82;
	else if( no == 8 ) displayString[5] = 0x96;
	else if( no == 9 ) displayString[5] = 0xAA;
	else if( no == 10 ) displayString[5] = 0xBE;
	displayString[6]		=	0x00;
	
	//STRING KOPIEREN
	while( errorString[i] != 0x0A )
	{
		displayString[i+7]		=	errorString[i];
		i++;
	}
	displayString[i+7]		=	0x00;
	i+=8;
	
	DisplaySendNBytes( displayString, i );
}	

/** DISPLAY SHOW NO ERROR LOGS ***********************************/
void DisplayShowNoErrorLogs( void )
{
	data[0]		=	0x1B;
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x0A;
	data[4]		=	0x00;
	data[5]		=	0x0A;
	data[6]		=	0x00;
	#if defined( DEUTSCH )
	data[7]		=	'K';
	data[8]		=	'e';
	data[9]		=	'i';
	data[10]	=	'n';
	data[11]	=	'e';
	data[12]	=	' ';
	data[13]	=	'S';
	data[14]	=	't';
	data[15]	=	'o';
	data[16]	=	'e';
	data[17]	=	'r';
	data[18]	=	'u';
	data[19]	=	'n';
	data[20]	=	'g';
	data[21]	=	'e';
	data[22]	=	'n';
	data[23]	=	0x00;
	
	DisplaySendNBytes( data, 24 );
	#endif
	#if defined( ENGLISCH )
	data[7]		=	'N';
	data[8]		=	'o';
	data[9]		=	' ';
	data[10]	=	'E';
	data[11]	=	'r';
	data[12]	=	'r';
	data[13]	=	'o';
	data[14]	=	'r';
	data[15]	=	's';
	data[16]	=	0x00;
	
	DisplaySendNBytes( data, 17 );
	#endif
}	

/** DISPLAY SHOW SPEICHERUNG ***********************************/
void DisplayShowSpeicherung( void )
{
	//gelbes Rechteck zeichnen
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x46;	//F
	data[3]		=	0x5C;
	data[4]		=	0x00;	//x1=92
	data[5]		=	0xBE;
	data[6]		=	0x00;	//y1=190
	data[7]		=	0x80;
	data[8]		=	0x01;	//x2=384
	data[9]		=	0xD2;
	data[10]	=	0x00;	//y2=210
	data[11]	=	0x07;
	//Schrift schwarz
	data[12]	=	0x1B;	//ESC
	data[13]	=	0x46;	//F
	data[14]	=	0x5A;	//Z
	data[15]	=	0x01;
	data[16]	=	0x00;
	//Schrift einblenden
	data[17]	=	0x1B;	//ESC
	data[18]	=	0x5A;	//Z
	data[19]	=	0x4C;	//L
	data[20]	=	0xB4;
	data[21]	=	0x00;	//x=180
	data[22]	=	0xC1;
	data[23]	=	0x00;	//y=193
	#if defined( DEUTSCH )
	data[24]	=	0x57;	//W
	data[25]	=	0x65;	//e
	data[26]	=	0x72;	//r
	data[27]	=	0x74;	//t
	data[28]	=	0x20;	//SPACE
	data[29]	=	0x67;	//g
	data[30]	=	0x65;	//e
	data[31]	=	0x73;	//s
	data[32]	=	0x70;	//p
	data[33]	=	0x65;	//e
	data[34]	=	0x69;	//i
	data[35]	=	0x63;	//c
	data[36]	=	0x68;	//h
	data[37]	=	0x65;	//e
	data[38]	=	0x72;	//r
	data[39]	=	0x74;	//t
	data[40]	=	0x00;	//NUL
	//Schrift weiß
	data[41]	=	0x1B;	//ESC
	data[42]	=	0x46;	//F
	data[43]	=	0x5A;	//Z
	data[44]	=	0x08;
	data[45]	=	0x00;

	DisplaySendNBytes( data, 46 );
	#endif
	#if defined( ENGLISCH )
	data[24]	=	' ';
	data[25]	=	' ';
	data[26]	=	'V';
	data[27]	=	'a';
	data[28]	=	'l';
	data[29]	=	'u';
	data[30]	=	'e';
	data[31]	=	' ';
	data[32]	=	's';
	data[33]	=	'a';
	data[34]	=	'v';
	data[35]	=	'e';
	data[36]	=	'd';
	data[37]	=	' ';
	data[38]	=	' ';
	data[39]	=	' ';
	data[40]	=	0x00;
	//Schrift weiß
	data[41]	=	0x1B;	//ESC
	data[42]	=	0x46;	//F
	data[43]	=	0x5A;	//Z
	data[44]	=	0x08;
	data[45]	=	0x00;

	DisplaySendNBytes( data, 46 );
	#endif
}

/** DISPLAY SHOW MEM INIT DONE ***********************************/
void DisplayShowMemInitDone( void )
{
	//gelbes Rechteck zeichnen
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x46;	//F
	data[3]		=	0x5C;
	data[4]		=	0x00;	//x1=92
	data[5]		=	0xBE;
	data[6]		=	0x00;	//y1=190
	data[7]		=	0x80;
	data[8]		=	0x01;	//x2=384
	data[9]		=	0xD2;
	data[10]	=	0x00;	//y2=210
	data[11]	=	0x07;
	//Schrift schwarz
	data[12]	=	0x1B;	//ESC
	data[13]	=	0x46;	//F
	data[14]	=	0x5A;	//Z
	data[15]	=	0x01;
	data[16]	=	0x00;
	//Schrift einblenden
	data[17]	=	0x1B;	//ESC
	data[18]	=	0x5A;	//Z
	data[19]	=	0x4C;	//L
	data[20]	=	0xDC;
	data[21]	=	0x00;	//x=180
	data[22]	=	0xC1;
	data[23]	=	0x00;	//y=193
	data[24]	=	'F';
	data[25]	=	'e';
	data[26]	=	'r';
	data[27]	=	't';
	data[28]	=	'i';
	data[29]	=	'g';
	data[30]	=	0x00;
	//Schrift weiß
	data[31]	=	0x1B;	//ESC
	data[32]	=	0x46;	//F
	data[33]	=	0x5A;	//Z
	data[34]	=	0x08;
	data[35]	=	0x00;

	DisplaySendNBytes( data, 36 );
}

/** DISPLAY SHOW RESET ERROR ***********************************/
void DisplayShowResetError( int i )
{
	//gelbes Rechteck zeichnen
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x46;	//F
	data[3]		=	0x5C;
	data[4]		=	0x00;	//x1=92
	data[5]		=	0xBE;
	data[6]		=	0x00;	//y1=190
	data[7]		=	0x80;
	data[8]		=	0x01;	//x2=384
	data[9]		=	0xD2;
	data[10]	=	0x00;	//y2=210
	data[11]	=	0x07;
	//Schrift schwarz
	data[12]	=	0x1B;	//ESC
	data[13]	=	0x46;	//F
	data[14]	=	0x5A;	//Z
	data[15]	=	0x01;
	data[16]	=	0x00;
	//Schrift einblenden
	data[17]	=	0x1B;	//ESC
	data[18]	=	0x5A;	//Z
	data[19]	=	0x4C;	//L
	data[20]	=	0xDC;
	data[21]	=	0x00;	//x=180
	data[22]	=	0xC1;
	data[23]	=	0x00;	//y=193
	data[24]	=	'R';
	data[25]	=	'e';
	data[26]	=	's';
	data[27]	=	'e';
	data[28]	=	't';
	data[29]	=	' ';
	data[30]	=	'E';
	data[31]	=	'r';
	data[32]	=	'r';
	data[33]	=	'o';
	data[34]	=	'r';
	data[35]	=	' ';
	if( i == 0 )
	{
		data[36]	=	'1';
	}
	else
	{
		data[36]	=	'0';
	}		
	data[37]	=	'>';
	if( i == 0 )
	{
		data[38]	=	'0';
	}
	else
	{
		data[38]	=	'1';
	}		
	data[39]	=	0x00;
	//Schrift weiß
	data[40]	=	0x1B;	//ESC
	data[41]	=	0x46;	//F
	data[42]	=	0x5A;	//Z
	data[43]	=	0x08;
	data[44]	=	0x00;

	DisplaySendNBytes( data, 45 );
}

/** DISPLAY SHOW UHR BLOCKIERT ***********************************/
void DisplayShowUhrBlockiert( void )
{
	//gelbes Rechteck zeichnen
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x46;	//F
	data[3]		=	0x5C;
	data[4]		=	0x00;	//x1=92
	data[5]		=	0xBE;
	data[6]		=	0x00;	//y1=190
	data[7]		=	0x80;
	data[8]		=	0x01;	//x2=384
	data[9]		=	0xD2;
	data[10]	=	0x00;	//y2=210
	data[11]	=	0x07;
	//Schrift schwarz
	data[12]	=	0x1B;	//ESC
	data[13]	=	0x46;	//F
	data[14]	=	0x5A;	//Z
	data[15]	=	0x01;
	data[16]	=	0x00;
	//Schrift einblenden
	data[17]	=	0x1B;	//ESC
	data[18]	=	0x5A;	//Z
	data[19]	=	0x4C;	//L
	data[20]	=	0xB0;
	data[21]	=	0x00;	//x=176
	data[22]	=	0xC1;
	data[23]	=	0x00;	//y=193
	#if defined( DEUTSCH )
	data[24]	=	'F';
	data[25]	=	'u';
	data[26]	=	'n';
	data[27]	=	'k';
	data[28]	=	't';
	data[29]	=	'i';
	data[30]	=	'o';
	data[31]	=	'n';
	data[32]	=	' ';
	data[33]	=	'b';
	data[34]	=	'l';
	data[35]	=	'o';
	data[36]	=	'c';
	data[37]	=	'k';
	data[38]	=	'i';
	data[39]	=	'e';
	data[40]	=	'r';
	data[41]	=	't';
	#endif
	#if defined( ENGLISCH )
	data[24]	=	'F';
	data[25]	=	'u';
	data[26]	=	'n';
	data[27]	=	'c';
	data[28]	=	't';
	data[29]	=	'i';
	data[30]	=	'o';
	data[31]	=	'n';
	data[32]	=	' ';
	data[33]	=	'd';
	data[34]	=	'i';
	data[35]	=	's';
	data[36]	=	'a';
	data[37]	=	'b';
	data[38]	=	'l';
	data[39]	=	'e';
	data[40]	=	'd';
	data[41]	=	' ';
	#endif
	data[42]	=	0x00;	//NUL
	//Schrift weiß
	data[43]	=	0x1B;	//ESC
	data[44]	=	0x46;	//F
	data[45]	=	0x5A;	//Z
	data[46]	=	0x08;
	data[47]	=	0x00;

	DisplaySendNBytes( data, 48 );
}

/** DISPLAY SHOW DATE CURSOR ***********************************/
void DisplayShowDateCursor( void )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x7D;
	data[4]		=	0x00;	//x1=125
	data[5]		=	0x52;
	data[6]		=	0x00;	//y1=82
	data[7]		=	0xC5;
	data[8]		=	0x00;	//x2=197
	data[9]		=	0x56;
	data[10]	=	0x00;	//y2=86
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x47;	//G
	data[13]	=	0x44;	//D
	if( datecursor == 1 )
	{
		data[14]	=	0x7D;
		data[15]	=	0x00;	//x1=125
		data[16]	=	0x54;
		data[17]	=	0x00;	//y1=84
		data[18]	=	0x8D;
		data[19]	=	0x00;	//x2=141
	}
	else if( datecursor == 2 )
	{
		data[14]	=	0x91;
		data[15]	=	0x00;	//x1=145
		data[16]	=	0x54;
		data[17]	=	0x00;	//y1=84
		data[18]	=	0xA1;
		data[19]	=	0x00;	//x2=161
	}
	else if( datecursor == 3 )
	{
		data[14]	=	0xB5;
		data[15]	=	0x00;	//x1=181
		data[16]	=	0x54;
		data[17]	=	0x00;	//y1=84
		data[18]	=	0xC5;
		data[19]	=	0x00;	//x2=197
	}
	data[20]	=	0x54;
	data[21]	=	0x00;	//y2=84

	DisplaySendNBytes( data, 22 );
}

/** DISPLAY SHOW TIME CURSOR ***********************************/
void DisplayShowTimeCursor( void )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x4C;	//L
	data[3]		=	0x7D;
	data[4]		=	0x00;	//x1=125
	data[5]		=	0x66;
	data[6]		=	0x00;	//y1=102
	data[7]		=	0xA1;
	data[8]		=	0x00;	//x2=161
	data[9]		=	0x6A;
	data[10]	=	0x00;	//y2=106
	data[11]	=	0x1B;	//ESC
	data[12]	=	0x47;	//G
	data[13]	=	0x44;	//D
	if( timecursor == 1 )
	{
		data[14]	=	0x7D;
		data[15]	=	0x00;	//x1=125
		data[16]	=	0x68;
		data[17]	=	0x00;	//y1=104
		data[18]	=	0x8D;
		data[19]	=	0x00;	//x2=141
	}
	else if( timecursor == 2 )
	{
		data[14]	=	0x91;
		data[15]	=	0x00;	//x1=145
		data[16]	=	0x68;
		data[17]	=	0x00;	//y1=104
		data[18]	=	0xA1;
		data[19]	=	0x00;	//x2=161
	}
	data[20]	=	0x68;
	data[21]	=	0x00;	//y2=104

	DisplaySendNBytes( data, 22 );
}

/** CLOCK INIT *************************************/
void ClockInit( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_STATUS_REG );

	I2CSendByte( 0x02 );			//set write enable bit

	I2CStop();

	ClockWriteEnable();
}

/** CLOCK WRITE ENABLE *************************************/
void ClockWriteEnable( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_STATUS_REG );

	I2CSendByte( 0x06 );			//set write control reg enable bit

	I2CStop();
}

/** CLOCK INIT DATE TIME *************************************/
void ClockInitDateTime( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_SECONDS_REG );

	I2CSendByte( 0x00 );
	I2CSendByte( 0x30 );
	I2CSendByte( ( 0x10 | 0x80 ) );
	I2CSendByte( 0x01 );
	I2CSendByte( 0x01 );
	I2CSendByte( 0x10 );
	I2CSendByte( 0x00 );
	I2CSendByte( 0x20 );

	I2CStop();
}

/** CLOCK SET CLOCK *************************************/
void ClockSetClock( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_SECONDS_REG );

	I2CSendByte( 0x00 );
	I2CSendByte( minutes );
	I2CSendByte( ( hours | 0x80 ) );
	I2CSendByte( day );
	I2CSendByte( month );
	I2CSendByte( year );
	I2CSendByte( 0x00 );
	I2CSendByte( 0x20 );

	I2CStop();
}

/** CLOCK READ MINUTES *************************************/
void ClockReadMinutes( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );				//word address 1
	I2CSendByte( CLOCK_MINUTES_REG );

	I2CRepStart();
	I2CSendByte( CLOCK_CONTROL_ADDRR );

	I2CReadBytePointer( &minutes, 1 );

	I2CStop();

	min1	=		( ( minutes >> 4 ) & 0x0F );
	min2	=		( minutes & 0x0F );
}

/** CLOCK READ HOURS *************************************/
void ClockReadHours( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );				//word address 1
	I2CSendByte( CLOCK_HOURS_REG );

	I2CRepStart();
	I2CSendByte( CLOCK_CONTROL_ADDRR );

	I2CReadBytePointer( &hours, 1 );

	I2CStop();

	hours	=		hours & 0x3F;

	hrs1	=		( ( hours >> 4 ) & 0x0F );
	hrs2	=		( hours & 0x0F );
}

/** CLOCK READ DAY *************************************/
void ClockReadDay( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );				//word address 1
	I2CSendByte( CLOCK_DAY_REG );

	I2CRepStart();
	I2CSendByte( CLOCK_CONTROL_ADDRR );

	I2CReadBytePointer( &day, 1 );

	I2CStop();

	day1	=		( ( day >> 4 ) & 0x0F );
	day2	=		( day & 0x0F );
}

/** CLOCK READ MONTH *************************************/
void ClockReadMonth( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );				//word address 1
	I2CSendByte( CLOCK_MONTH_REG );

	I2CRepStart();
	I2CSendByte( CLOCK_CONTROL_ADDRR );

	I2CReadBytePointer( &month, 1 );

	I2CStop();

	month1	=		( ( month >> 4 ) & 0x0F );
	month2	=		( month & 0x0F );
}

/** CLOCK READ YEAR *************************************/
void ClockReadYear( void )
{
	I2CStart();
	I2CSendByte( CLOCK_CONTROL_ADDRW );
	I2CSendByte( 0x00 );				//word address 1
	I2CSendByte( CLOCK_YEAR_REG );

	I2CRepStart();
	I2CSendByte( CLOCK_CONTROL_ADDRR );

	I2CReadBytePointer( &year, 1 );

	I2CStop();

	year1	=		( ( year >> 4 ) & 0x0F );
	year2	=		( year & 0x0F );
}

/** MEMORY INIT 3 **************************************/
void MemoryInit3( void )
{
	I2CStart();
	Nop(); Nop(); Nop();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x60 );
	Nop(); Nop(); Nop();

	I2CSendByte( 0x30 );	//Automessung
	Nop(); Nop(); Nop();
	I2CSendByte( 0x3C );	//Splitzeit
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//KalibrationReportNr high_byte
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//KalibrationReportNr med_byte
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//KalibrationReportNr low_byte
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//Errorcount high_byte
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//Errorcount med_byte
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//Errorcount low_byte
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();

	I2CStop();
}

/** MEMORY INIT 2 **************************************/
void MemoryInit2( void )
{
	I2CStart();
	Nop(); Nop(); Nop();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x40 );
	Nop(); Nop(); Nop();

	I2CSendByte( 0x30 );	//Messort 0
	Nop(); Nop(); Nop();
	I2CSendByte( 0x30 );	//Messort 1
	Nop(); Nop(); Nop();
	I2CSendByte( 0x30 );	//Messort 2
	Nop(); Nop(); Nop();
	I2CSendByte( 0x30 );	//Messort 3
	Nop(); Nop(); Nop();
	I2CSendByte( 0x30 );	//Messort 4
	Nop(); Nop(); Nop();
	I2CSendByte( 0x30 );	//Messort 5
	Nop(); Nop(); Nop();
	I2CSendByte( 0x30 );	//Messort 6
	Nop(); Nop(); Nop();
	I2CSendByte( 0x31 );	//Messort 7
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x01 );	//Uebertragungsart
	Nop(); Nop(); Nop();
	I2CSendByte( 0x28 );	//Skalierung
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//Messungszähler 1
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//Messungszähler 2
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//Messungszähler 3
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();

	I2CStop();
}
	
/** MEMORY INIT *************************************/
void MemoryInit( void )
{	
	I2CStart();
	Nop(); Nop(); Nop();
	while( I2CSendByte( CLOCK_EEPROM_ADDRW ) == 1 )
	{
		Nop(); Nop(); Nop(); Nop(); Nop(); Nop();
	}
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x50 );
	Nop(); Nop(); Nop();

	I2CSendByte( 0x00 );	//Eichgasmenge 1
	Nop(); Nop(); Nop();
	I2CSendByte( 0x7C );	//Eichgasmenge 2
	Nop(); Nop(); Nop();
	I2CSendByte( 0x01 );	//Menge 1
	Nop(); Nop(); Nop();
	I2CSendByte( 0x09 );	//Menge 2
	Nop(); Nop(); Nop();
	I2CSendByte( 0x0C );	//Volumen
	Nop(); Nop(); Nop();
	I2CSendByte( 0x2D );	//Temperatur
	Nop(); Nop(); Nop();
	I2CSendByte( 0x14 );	//Druck
	Nop(); Nop(); Nop();
	I2CSendByte( 0x01 );	//Empfindlichkeit
	Nop(); Nop(); Nop();
	I2CSendByte( 0x14 );	//Messzyklus
	Nop(); Nop(); Nop();
	I2CSendByte( 0x02 );	//Eichzyklus
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x3C );	//Spuelzeit
	Nop(); Nop(); Nop();
	I2CSendByte( 0x64 );	//Korrekturfaktor
	Nop(); Nop(); Nop();
	I2CSendByte( 0x96 );	//Fluss
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );
	Nop(); Nop(); Nop();
	I2CSendByte( 0x00 );	//taegliche Messung
	Nop(); Nop(); Nop();

	I2CStop();
	Nop(); Nop(); Nop();
}

/** MEMORY GET BETRIEBSPARAMETER *************************************/
void MemoryGetBetriebsparameter( void )
{
	MemoryGetEichgas();
	MemoryGetMenge();
	MemoryGetVolumen();
	MemoryGetTemperatur();
	MemoryGetFluss();
	MemoryGetUebertragungsart();
	MemoryGetSkalierung();
	MemoryGetDruck();
	MemoryGetEmpfindlichkeit();
	MemoryGetMesszyklus();
	MemoryGetEichzyklus();
	MemoryGetSpuelzeit();
	MemoryGetKorrekturfaktor();
	MemoryGetAutomessung();
	MemoryGetSplitzeit();
}

/** MEMORY GET FERNUEBERTRAGUNG *************************************/
void MemoryGetFernuebertragung( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( 0x4A );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	uebertragungsart	=	( BOOL ) I2CReadByte( 0 );
	skalierung			= 	( short ) I2CReadByte( 1 );

	I2CStop();
	
	Nop();
	Nop();
	Nop();
}

/** MEMORY GET MESSORT *************************************/
void MemoryGetMessort( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_MESSORT );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	I2CReadBytePointer( &messort[0], 0 );
	I2CReadBytePointer( &messort[1], 0 );
	I2CReadBytePointer( &messort[2], 0 );
	I2CReadBytePointer( &messort[3], 0 );
	I2CReadBytePointer( &messort[4], 0 );
	I2CReadBytePointer( &messort[5], 0 );
	I2CReadBytePointer( &messort[6], 0 );
	I2CReadBytePointer( &messort[7], 1 );

	I2CStop();
}

/** MEMORY GET TAEGLICHE MESSUNG *************************************/
void MemoryGetTaeglicheMessung( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_DAYMEAS );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	taeglicheMessung	=	( unsigned int ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET SKALIERUNG *************************************/
void MemoryGetSkalierung( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_SKALIERUNG );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	skalierung	=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET VOLUMEN *************************************/
unsigned short MemoryGetVolumen( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_VOLUMEN );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	volumen		=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
	
	volumen		=	volumen & 0x00FF;
	
	return volumen;
}

/** MEMORY GET TEMPERATUR *************************************/
void MemoryGetTemperatur( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_TEMP );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	temperatur	=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET FLUSS *************************************/
void MemoryGetFluss( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_FLUSS );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	fluss	=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
	
	fluss	=	fluss & 0x00FF;
}

/** MEMORY GET DRUCK *************************************/
void MemoryGetDruck( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_DRUCK );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	druck	=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET EMPFINDLICHKEIT *************************************/
void MemoryGetEmpfindlichkeit( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_EMPFINDLKT );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	empfindlichkeit		=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET MESSZYKLUS *************************************/
void MemoryGetMesszyklus( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_MESSZYKLUS );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	messzyklus		=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET EICHZYKLUS *************************************/
void MemoryGetEichzyklus( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_EICHZYKLUS );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	eichzyklus		=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET SPUELZEIT *************************************/
void MemoryGetSpuelzeit( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_SPUELZEIT );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	spuelzeit		=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET SPLITZEIT *************************************/
void MemoryGetSplitzeit( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_SPLITZEIT );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	splitzeit		=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET KORREKTURFAKTOR *************************************/
void MemoryGetKorrekturfaktor( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_KORREKTUR );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	korrekturfaktor		=	( unsigned short ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET EICHGAS *************************************/
void MemoryGetEichgas( void )
{
	unsigned short eichgas1;
	unsigned short eichgas2;
	
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_EICHGAS );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	eichgas1		=	( unsigned short ) I2CReadByte( 0 );
	eichgas2		=	( unsigned short ) I2CReadByte( 1 );
	
	I2CStop();

	eichgasmenge		=	eichgas1; 
	eichgasmenge		=	eichgasmenge << 8;
	eichgas2			=	eichgas2 & 0xFF;
	eichgasmenge		+=	eichgas2;
}

/** MEMORY GET MENGE *************************************/
void MemoryGetMenge( void )
{
	unsigned short menge1;
	unsigned short menge2;
	
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_MENGE );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	menge1		=	( unsigned short ) I2CReadByte( 0 );
	menge2		=	( unsigned short ) I2CReadByte( 1 );
	
	I2CStop();

	menge			=	menge1; 
	menge			=	menge << 8;
	menge2			=	menge2 & 0xFF;
	menge			+=	menge2;
}

/** MEMORY GET UEBERTRAGUNGSART *************************************/
void MemoryGetUebertragungsart( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_UEBERTRAG );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	uebertragungsart	=	( BOOL ) I2CReadByte( 1 );

	I2CStop();
}

/** MEMORY GET AUTOMESSUNG *************************************/
void MemoryGetAutomessung( void )
{
	unsigned short x;
	
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_AUTOMEAS );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	x	=	( unsigned short ) I2CReadByte( 1 );
	
	if( x == 0x31 )
	{
		autoMeasurement		=	TRUE;
	}	
	else
	{
		autoMeasurement		=	FALSE;
	}	

	I2CStop();
}

/** MEMORY SET MESSORT *************************************/
void MemorySetMessort( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_MESSORT );

	I2CSendByte( messort[0] );
	I2CSendByte( messort[1] );
	I2CSendByte( messort[2] );
	I2CSendByte( messort[3] );
	I2CSendByte( messort[4] );
	I2CSendByte( messort[5] );
	I2CSendByte( messort[6] );
	I2CSendByte( messort[7] );

	I2CStop();
}

/** MEMORY SET EICHGASMENGE *************************************/
void MemorySetEichgasmenge( void )
{
	unsigned short int eichgas1		=	eichgasmenge;
	unsigned short int eichgas2		=	eichgasmenge;
	
	eichgas1		=	eichgas1 >> 8;
	eichgas2		=	eichgas2 & 0xFF;
	
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_EICHGAS );

	I2CSendByte( eichgas1 );
	I2CSendByte( eichgas2 );

	I2CStop();
}

/** MEMORY SET MENGE *************************************/
void MemorySetMenge( void )
{
	unsigned short int menge1	=	menge;
	unsigned short int menge2	=	menge;

	menge1		=	menge1 >> 8;
	menge2		=	menge2 & 0xFF;

	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_MENGE );

	I2CSendByte( menge1 );
	I2CSendByte( menge2 );

	I2CStop();
}

/** MEMORY SET VOLUMEN *************************************/
void MemorySetVolumen( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_VOLUMEN );

	I2CSendByte( volumen );

	I2CStop();
}

/** MEMORY SET TEMPERATUR *************************************/
void MemorySetTemperatur( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_TEMP );

	I2CSendByte( saeule_vorgabe );

	I2CStop();
}

/** MEMORY SET DRUCK *************************************/
void MemorySetDruck( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_DRUCK );

	I2CSendByte( vordruck_vorgabe );

	I2CStop();
}

/** MEMORY SET EMPFINDLICHKEIT *************************************/
void MemorySetEmpfindlichkeit( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_EMPFINDLKT );

	I2CSendByte( empfindlichkeit_vorgabe );

	I2CStop();
}

/** MEMORY SET MESSZYKLUS *************************************/
void MemorySetMesszyklus( void )
{
	unsigned short int eichzyk1	=	eichzyklus;
	unsigned short int eichzyk2	=	eichzyklus;

	eichzyk1		=	eichzyk1 >> 8;
	eichzyk2		=	eichzyk2 & 0xFF;

	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_MESSZYKLUS );

	I2CSendByte( messzyklus );
	I2CSendByte( eichzyk1 );
	I2CSendByte( eichzyk2 );

	I2CStop();
}

/** MEMORY SET EICHZYKLUS *************************************/
void MemorySetEichzyklus( void )
{
	unsigned short int eichzyk1	=	eichzyklus;
	unsigned short int eichzyk2	=	eichzyklus;

	eichzyk1		=	eichzyk1 >> 8;
	eichzyk2		=	eichzyk2 & 0xFF;

	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_EICHZYKLUS );

	I2CSendByte( eichzyklus );

	I2CStop();
}

/** MEMORY SET FLUSS *************************************/
void MemorySetFluss( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_FLUSS );

	I2CSendByte( fluss_vorgabe );

	I2CStop();
}

/** MEMORY SET SPUELZEIT *************************************/
void MemorySetSpuelzeit( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_SPUELZEIT );

	I2CSendByte( spuelzeit );

	I2CStop();
}

/** MEMORY SET SPLITZEIT *************************************/
void MemorySetSplitzeit( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_SPLITZEIT );

	I2CSendByte( splitzeit );

	I2CStop();
}

/** MEMORY SET KORREKTURFAKTOR *************************************/
void MemorySetKorrekturfaktor( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_KORREKTUR );

	I2CSendByte( korrekturfaktor );

	I2CStop();
}

/** MEMORY SET TAEGLICHE MESSUNG *************************************/
void MemorySetTaeglicheMessung( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_DAYMEAS );

	I2CSendByte( taeglicheMessung );

	I2CStop();
}

/** MEMORY SET UEBERTRAGUNGSART *************************************/
void MemorySetUebertragungsart( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_UEBERTRAG );

	I2CSendByte( uebertragungsart );

	I2CStop();
}

/** MEMORY SET AUTOMESSUNG *************************************/
void MemorySetAutomessung( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_AUTOMEAS );

	if( autoMeasurement )
	{
		I2CSendByte( 0x31 );
	}
	else
	{
		I2CSendByte( 0x30 );
	}		

	I2CStop();
}

/** MEMORY SET SKALIERUNG *************************************/
void MemorySetSkalierung( void )
{
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_SKALIERUNG );

	I2CSendByte( skalierung );

	I2CStop();
}

/** MEMORY GET MEAS COUNT *************************************/
void MemoryGetMeasCount( void )
{
	unsigned long int temp;
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;

	//Get recent measurement count
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_MEASCOUNT );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	t1		=	I2CReadByte( 0 );
	t2		=	I2CReadByte( 0 );
	t3		=	I2CReadByte( 1 );

	I2CStop();

	//concatenate value
	temp		=	0;
	temp		=	t1;
	temp		=	temp << 8;
	temp		=	temp & 0x0000FF00;
	temp		=	temp + t2;
	temp		=	temp << 8;
	temp		=	temp & 0x00FFFF00;
	temp		=	temp + t3;

	meascount	=	temp;
}

/** MEMORY GET ERROR COUNT *************************************/
void MemoryGetErrorCount( void )
{
	unsigned long int temp;
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;

	//Get recent measurement count
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_ERRORLOG );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	t1		=	I2CReadByte( 0 );
	t2		=	I2CReadByte( 0 );
	t3		=	I2CReadByte( 1 );

	I2CStop();

	//concatenate value
	temp		=	0;
	temp		=	t1;
	temp		=	temp << 8;
	temp		=	temp & 0x0000FF00;
	temp		=	temp + t2;
	temp		=	temp << 8;
	temp		=	temp & 0x00FFFF00;
	temp		=	temp + t3;

	errorcount	=	temp;
}

/** MEMORY GET KAL REPORT NR *************************************/
void MemoryGetKalReportNr( void )
{
	unsigned long int temp;
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;

	//Get recent calibration report no
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_KALREPORT );

	I2CRepStart();
	I2CSendByte( CLOCK_EEPROM_ADDRR );

	t1		=	I2CReadByte( 0 );
	t2		=	I2CReadByte( 0 );
	t3		=	I2CReadByte( 1 );

	I2CStop();

	//concatenate value
	temp		=	0;
	temp		=	t1;
	temp		=	temp << 8;
	temp		=	temp & 0x0000FF00;
	temp		=	temp + t2;
	temp		=	temp << 8;
	temp		=	temp & 0x00FFFF00;
	temp		=	temp + t3;

	kalReportNr	=	temp;
}

/** MEMORY SET MEAS COUNT *************************************/
void MemorySetMeasCount( void )
{
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;

	//deconcatenate value
	t3			=	meascount & 0x000000FF;
	t2			=	meascount >> 8;
	t2			=	t2 & 0x000000FF;
	t1			=	meascount >> 16;
	t1			=	t1 & 0x000000FF;
	
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_MEASCOUNT );
	
	I2CSendByte( t1 );
	I2CSendByte( t2 );
	I2CSendByte( t3 );
	
	I2CStop();
}

/** MEMORY SET ERROR COUNT *************************************/
void MemorySetErrorCount( void )
{
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;

	//deconcatenate value
	t3			=	errorcount & 0x000000FF;
	t2			=	errorcount >> 8;
	t2			=	t2 & 0x000000FF;
	t1			=	errorcount >> 16;
	t1			=	t1 & 0x000000FF;
	
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_ERRORLOG );
	
	I2CSendByte( t1 );
	I2CSendByte( t2 );
	I2CSendByte( t3 );
	
	I2CStop();
}

/** MEMORY SET KAL REPORT NR *************************************/
void MemorySetKalReportNr( void )
{
	unsigned char t1;
	unsigned char t2;
	unsigned char t3;

	//deconcatenate value
	t3			=	kalReportNr & 0x000000FF;
	t2			=	kalReportNr >> 8;
	t2			=	t2 & 0x000000FF;
	t1			=	kalReportNr >> 16;
	t1			=	t1 & 0x000000FF;
	
	I2CStart();
	I2CSendByte( CLOCK_EEPROM_ADDRW );
	I2CSendByte( 0x00 );
	I2CSendByte( CLOCK_MEMORY_KALREPORT );
	
	I2CSendByte( t1 );
	I2CSendByte( t2 );
	I2CSendByte( t3 );
	
	I2CStop();
}

/** SYSTEM INIT **************************************/
void SystemInit( void )
{
	MemoryGetTemperatur();
	if( ( temperatur < 20 ) || ( temperatur > 60 ) ) { temperatur = defaultTemp; }
	saeule_vorgabe				=	temperatur;
	SystemSetSaeule();

	MemoryGetFluss();
	if( ( fluss < 100 ) || ( fluss > 200 ) ) { fluss = defaultFlow; }
	fluss_vorgabe				=	fluss;
	SystemSetFluss();

	MemoryGetEmpfindlichkeit();
	if( ( empfindlichkeit < 0 ) || ( empfindlichkeit > 9 ) ) { empfindlichkeit = defaultSensibility; }
	empfindlichkeit_vorgabe		=	empfindlichkeit;
	SystemSetEmpfindlichkeit();

	MemoryGetDruck();
	if( ( druck < 15 ) || ( druck > 70 ) ) { druck = defaultPressure; }
	vordruck_vorgabe			=	druck;
	SystemSetVordruck();
	
	MemoryGetSplitzeit();
	if( ( splitzeit < 1 ) || ( splitzeit > 70 ) ) { splitzeit = defaultSplittime; }
	SystemSetSplitzeit();		
}


/** SYSTEM SET DETEKTOR **************************************/
void SystemSetDetektor( void )
{
	unsigned short lbyte		=	0;
	unsigned short hbyte		=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	I2CSendByte( 0x01 );
	SystemPause();

	I2CSendByte( lbyte );
	SystemPause();
	I2CSendByte( hbyte );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM SET SAEULE **************************************/
void SystemSetSaeule( void )
{
	unsigned int temp			=	saeule_vorgabe * 10;

	unsigned short lbyte		=	temp;
	unsigned short hbyte		=	temp;

	lbyte						=	lbyte & 0x00FF;
	hbyte						=	hbyte >> 8;
	hbyte						=	hbyte & 0x00FF;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x02 );
	SystemPause();

	I2CSendByte( lbyte );
	SystemPause();
	I2CSendByte( hbyte );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM SET SPLITZEIT **************************************/
void SystemSetSplitzeit( void )
{
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x14 );
	SystemPause();

	I2CSendByte( splitzeit );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM SET FLUSS **************************************/
void SystemSetFluss( void )
{
	unsigned short int temp		=	fluss_vorgabe;

	unsigned short lbyte		=	temp;
	unsigned short hbyte		=	temp;

	lbyte						=	lbyte & 0x00FF;
	hbyte						=	hbyte >> 8;
	hbyte						=	hbyte & 0x00FF;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x03 );
	SystemPause();

	I2CSendByte( lbyte );
	SystemPause();
	I2CSendByte( hbyte );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM SET VORDRUCK **************************************/
void SystemSetVordruck( void )
{
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x04 );
	SystemPause();

	I2CSendByte( vordruck_vorgabe );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM SET EMPFINDLICHKEIT **************************************/
void SystemSetEmpfindlichkeit( void )
{
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x05 );
	SystemPause();

	I2CSendByte( empfindlichkeit_vorgabe );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM CALC PRESSURE **************************************/
/*
void SystemCalcPressure( int peakMitte )
{
	int newPressure		=	0;
	
	MemoryGetDruck();
	MemoryGetTemperatur();
	
	newPressure			=	peakMitte * druck;
	newPressure			=	( int ) ( ( double ) newPressure / ( double ) 150 );
	
	if( newPressure < 20 )
	{
		vordruck_vorgabe		=	20;
		druck					=	20;
		
		if( temperatur < 41 )
		{
			saeule_vorgabe 		=	40;
		}
		else
		{
			saeule_vorgabe			=	temperatur - 1;
		}	
		
		MemorySetTemperatur();
		SystemSetSaeule();

		displayWait				=	0;
		while( displayWait < 2 ) { Nop(); }
		MemorySetDruck();	
		SystemSetVordruck();
	}	
	else if( newPressure > 40 )
	{
		vordruck_vorgabe		=	40;
		druck					=	40;
		
		if( temperatur > 59 )
		{
			saeule_vorgabe		=	60;
		}
		else
		{	
			saeule_vorgabe			=	temperatur + 1;
		}	
		
		MemorySetTemperatur();
		SystemSetSaeule();

		displayWait				=	0;
		while( displayWait < 2 ) { Nop(); }
		MemorySetDruck();	
		SystemSetVordruck();
	}	
	else
	{
		vordruck_vorgabe		=	newPressure;
		druck					=	newPressure;
		
		MemorySetDruck();
		
		SystemSetVordruck();
	}	
}	
*/
/** SYSTEM SET MA1 **************************************/
void SystemSetMA1( unsigned int mA )
{
	unsigned short lbyte;
	unsigned short hbyte;
	
	if( uebertragungsart == TRUE )
	{
		if( mA < 830 )
		{
			mA		=	830;
		}	
	}	
	lbyte		=	mA & 0x00FF;
	hbyte		=	mA >> 8;
	hbyte		=	hbyte & 0x00FF;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x06 );
	SystemPause();

	I2CSendByte( lbyte );
	SystemPause();
	I2CSendByte( hbyte );
	SystemPause();

	I2CStop();
	SystemPause();
	
	IsMASet = TRUE;
}

/** SYSTEM SET OK **************************************/
void SystemSetOK( unsigned short k )
{
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x08 );
	SystemPause();

	if( k == 0 ) { I2CSendByte( 0x00 ); }
	else { I2CSendByte( 0x01 ); }
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM RESET ANALYTIC **************************************/
void SystemResetAnalytic( void )
{
	LATDbits.LATD1		=	0;
	displayWait			=	0;
	while( displayWait < 2 ) { Nop(); }
	
	while( LATDbits.LATD1 == 1 )
	{
		DisplayShowResetError( 0 );
		TRISDbits.TRISD1	=	0;
		LATDbits.LATD1		=	0;
		displayWait			=	0;
		while( displayWait < 2 ) { Nop(); }
	}	
	
	LATDbits.LATD1		=	1;
	displayWait			=	0;
	while( displayWait < 2 ) { Nop(); }
	
	while( LATDbits.LATD1 == 0 )
	{
		DisplayShowResetError( 1 );
		TRISDbits.TRISD1	=	0;
		LATDbits.LATD1		=	1;
		displayWait			=	0;
		while( displayWait < 2 ) { Nop(); }
	}	
	
	displayWait			=	0;
	while( displayWait < 48 ) { Nop(); }
	SystemInit();
}

/** SYSTEM START CALIBRATION **************************************/
void SystemStartCalibration( void )
{
	//FALLS AUTOMESSUNG AKTIV IST, NEUE STARTZEITEN BERECHNEN
	if( autoMeasurement )
	{
		SystemCalcNextRun( 'c' );
	}	
	meas6min		=	TRUE;			//6 minütige Messung

	previousValue[0]	=	0x00;
	previousValue[1]	=	0x00;
	previousValue[2]	=	0x00;
	previousValue[3]	=	0x00;
	previousValue[4]	=	0x00;
	previousValue[5]	=	0x00;
	previousValue[6]	=	0x00;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x09 );
	SystemPause();

	I2CSendByte( 0x01 );
	SystemPause();

	I2CStop();
	SystemPause();
	
	deviceActive		=	TRUE;
	SystemResetMesspunkte();
	messdatenptr		=	&messdaten[0];
}

/** SYSTEM START MEASUREMENT **************************************/
void SystemStartMeasurement( void )
{
	int j			=	0;
	
	//FALLS AUTOMESSUNG AKTIV IST, NEUE STARTZEITEN BERECHNEN
	if( autoMeasurement )
	{
		SystemCalcNextRun( 'm' );
	}	
	meas6min		=	TRUE;			//6 minütige Messung

	previousValue[0]	=	0x00;
	previousValue[1]	=	0x00;
	previousValue[2]	=	0x00;
	previousValue[3]	=	0x00;
	previousValue[4]	=	0x00;
	previousValue[5]	=	0x00;
	previousValue[6]	=	0x00;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	if( j == 1 ) { DisplayShowNACK(); }
	SystemPause();
	j = I2CSendByte( 0x10 );
	if( j == 1 ) { DisplayShowNACK(); }
	SystemPause();

	j = I2CSendByte( 0x01 );
	if( j == 1 ) { DisplayShowNACK(); }
	SystemPause();

	I2CStop();
	SystemPause();
	
	deviceActive		=	TRUE;
	SystemResetMesspunkte();
	messdatenptr		=	&messdaten[0];
}

/** SYSTEM START CALMEASUREMENT **************************************/
void SystemStartCalmeasurement( void )
{
	//FALLS AUTOMESSUNG AKTIV IST, NEUE STARTZEITEN BERECHNEN
	if( autoMeasurement )
	{
		SystemCalcNextRun( 'b' );
	}	
	meas6min		=	FALSE;			//12 minütige Messung

	previousValue[0]	=	0x00;
	previousValue[1]	=	0x00;
	previousValue[2]	=	0x00;
	previousValue[3]	=	0x00;
	previousValue[4]	=	0x00;
	previousValue[5]	=	0x00;
	previousValue[6]	=	0x00;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x11 );
	SystemPause();

	I2CSendByte( 0x01 );
	SystemPause();

	I2CStop();
	SystemPause();
	
	deviceActive		=	TRUE;
	SystemResetMesspunkte();
	messdatenptr		=	&messdaten[0];
}

/** SYSTEM START SYRINGE **************************************/
void SystemStartSyringe( void )
{	
	meas6min		=	TRUE;			//6 minütige Messung

	previousValue[0]	=	0x00;
	previousValue[1]	=	0x00;
	previousValue[2]	=	0x00;
	previousValue[3]	=	0x00;
	previousValue[4]	=	0x00;
	previousValue[5]	=	0x00;
	previousValue[6]	=	0x00;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x17 );
	SystemPause();

	I2CSendByte( 0x01 );
	SystemPause();

	I2CStop();
	SystemPause();
	
	deviceActive		=	TRUE;
	SystemResetMesspunkte();
	messdatenptr		=	&messdaten[0];
}

/** SYSTEM DETEKTOR ISTWERT **************************************/
void SystemDetektorIstwert( void )
{
	unsigned short detl		=	0;
	unsigned short deth		=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x31 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	detl				=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	deth				=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	detektor_istwert	=	deth;
	detektor_istwert	=	detektor_istwert << 8;
	detektor_istwert	=	detektor_istwert & 0xFF00;
	detl				=	detl & 0x00FF;
	detektor_istwert	+=	detl;
}

/** SYSTEM SAEULE ISTWERT **************************************/
void SystemSaeuleIstwert( void )
{
	unsigned short sael		=	0;
	unsigned short saeh		=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x32 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	sael			=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	saeh			=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	saeule_istwert		=	saeh;
	saeule_istwert		=	saeule_istwert << 8;
	saeule_istwert		=	saeule_istwert & 0xFF00;
	sael				=	sael & 0x00FF;
	saeule_istwert		+=	sael;
}

/** SYSTEM FLUSS ISTWERT **************************************/
void SystemFlussIstwert( void )
{
	unsigned short flul		=	0;
	unsigned short fluh		=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x33 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	flul			=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	fluh			=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	fluss_istwert		=	fluh;
	fluss_istwert		=	fluss_istwert << 8;
	fluss_istwert		=	fluss_istwert & 0xFF00;
	flul				=	flul & 0x00FF;
	fluss_istwert		+=	flul;
}

/** SYSTEM STATUS **************************************/
void DisplayShowSystemStatus( char c )
{
	data[0]		=	0x1B;	//ESC
	data[1]		=	0x52;	//R
	data[2]		=	0x46;	//F
	data[3]		=	0x81;
	data[4]		=	0x01;
	data[5]		=	0xDD;
	data[6]		=	0x00;
	data[7]		=	0xDE;
	data[8]		=	0x01;
	data[9]		=	0x0E;
	data[10]	=	0x01;
			
	if( ( ( c & 0x20 ) > 0 ) && ( ( c & 0x10 ) > 0 ) )
	{
		if( measurementType == 'm' )
		{

			data[11]	=	0x0D;	//YELLOW
		}	
		else if( measurementType == 'c' )
		{
			data[11]	=	0x04;	//GREEN
		}	
		else
		{
			if( sekunden <= 360 )
			{
				data[11]	=	0x04;	//YELLOW
			}
			else if( sekunden > 360 )
			{
				data[11]	=	0x0D;	//YELLOW
			}		
		}	
	}
	else
	{
		data[11]	=	0x07;	//YELLOW
	}	
	
	DisplaySendNBytes( data, 12 );
	
	if( malfunction )
	{
		data[0]		=	0x1B;
		data[1]		=	'Z';
		data[2]		=	'L';
		data[3]		=	0x82;
		data[4]		=	0x01;
		data[5]		=	0xDD;
		data[6]		=	0x00;
		#if defined( DEUTSCH )
		data[7]		=	'S';
		data[8]		=	'T';
		data[9]		=	'O';
		data[10]	=	'E';
		data[11]	=	'R';
		data[12]	=	'U';
		data[13]	=	'N';
		data[14]	=	'G';
		#endif
		#if defined( ENGLISCH )
		data[7]		=	'E';
		data[8]		=	'R';
		data[9]		=	'R';
		data[10]	=	'O';
		data[11]	=	'R';
		data[12]	=	' ';
		data[13]	=	' ';
		data[14]	=	' ';
		#endif
		data[15]	=	0x00;
		
		DisplaySendNBytes( data, 16 );
		
		if( errorCode == 1 )
		{
			data[0]		=	0x1B;
			data[1]		=	'Z';
			data[2]		=	'L';
			data[3]		=	0x82;
			data[4]		=	0x01;
			data[5]		=	0xED;
			data[6]		=	0x00;
			#if defined( DEUTSCH )
			data[7]		=	'F';
			data[8]		=	'L';
			data[9]		=	'U';
			data[10]	=	'S';
			data[11]	=	'S';
			#endif
			#if defined( ENGLISCH )
			data[7]		=	'F';
			data[8]		=	'L';
			data[9]		=	'O';
			data[10]	=	'W';
			data[11]	=	' ';
			#endif
			data[12]	=	0x00;
			
			DisplaySendNBytes( data, 13 );
		} 
		else if( errorCode == 2 )
		{
			data[0]		=	0x1B;
			data[1]		=	'Z';
			data[2]		=	'L';
			data[3]		=	0x82;
			data[4]		=	0x01;
			data[5]		=	0xED;
			data[6]		=	0x00;
			#if defined( DEUTSCH )
			data[7]		=	'F';
			data[8]		=	'L';
			data[9]		=	'A';
			data[10]	=	'E';
			data[11]	=	'C';
			data[12]	=	'H';
			data[13]	=	'E';
			#endif
			#if defined( ENGLISCH )
			data[7]		=	'A';
			data[8]		=	'R';
			data[9]		=	'E';
			data[10]	=	'A';
			data[11]	=	' ';
			data[12]	=	' ';
			data[13]	=	' ';
			#endif
			data[14]	=	0x00;
			
			DisplaySendNBytes( data, 15 );
		}
		else if( errorCode == 3 )
		{
			data[0]		=	0x1B;
			data[1]		=	'Z';
			data[2]		=	'L';
			data[3]		=	0x82;
			data[4]		=	0x01;
			data[5]		=	0xED;
			data[6]		=	0x00;
			#if defined( DEUTSCH )
			data[7]		=	'R';
			data[8]		=	'E';
			data[9]		=	'T';
			data[10]	=	'Z';
			data[11]	=	'E';
			data[12]	=	'I';
			data[13]	=	'T';
			#endif
			#if defined( ENGLISCH )
			data[7]		=	'R';
			data[8]		=	'E';
			data[9]		=	'T';
			data[10]	=	'T';
			data[11]	=	'I';
			data[12]	=	'M';
			data[13]	=	'E';
			#endif
			data[14]	=	0x00;
			
			DisplaySendNBytes( data, 15 );
		}		
		else if( errorCode == 4 )
		{
			data[0]		=	0x1B;
			data[1]		=	'Z';
			data[2]		=	'L';
			data[3]		=	0x82;
			data[4]		=	0x01;
			data[5]		=	0xED;
			data[6]		=	0x00;
			data[7]		=	'B';
			data[8]		=	'A';
			data[9]		=	'T';
			data[10]	=	'T';
			data[11]	=	'E';
			data[12]	=	'R';
			data[13]	=	'Y';
			data[14]	=	0x00;
			
			DisplaySendNBytes( data, 15 );
		}
		else if( errorCode == 5 )
		{
			data[0]		=	0x1B;
			data[1]		=	'Z';
			data[2]		=	'L';
			data[3]		=	0x82;
			data[4]		=	0x01;
			data[5]		=	0xED;
			data[6]		=	0x00;
			data[7]		=	'M';
			data[8]		=	'A';
			data[9]		=	' ';
			data[10]	=	'O';
			data[11]	=	'U';
			data[12]	=	'T';
			data[13]	=	'P';
			data[14]	=	'U';
			data[15]	=	'T';
			data[16]	=	0x00;
			
			DisplaySendNBytes( data, 17 );
		}
		
		return;
	}	
	
	if( invertStatus )
	{
		data[0]		=	0x1B;	//ESC
		data[1]		=	'F';
		data[2]		=	'Z';
		data[3]		=	0x01;
		data[4]		=	0x00;
		
		invertStatus	=	FALSE;
		
		DisplaySendNBytes( data, 5 );
	}	
	else
	{
		data[0]		=	0x1B;	//ESC
		data[1]		=	'F';
		data[2]		=	'Z';
		data[3]		=	0x08;
		data[4]		=	0x00;
		
		invertStatus	=	TRUE;
		
		DisplaySendNBytes( data, 5 );
	}	
	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x82;
	data[4]		=	0x01;
	data[5]		=	0xDD;
	data[6]		=	0x00;
	if( ( c & 0x80 ) > 0 )
	{
		#if defined( DEUTSCH )
		data[7]		=	'F';
		data[8]		=	'e';
		data[9]		=	'h';
		data[10]	=	'l';
		data[11]	=	'e';
		data[12]	=	'r';
		#endif
		#if defined( ENGLISCH )
		data[7]		=	'E';
		data[8]		=	'r';
		data[9]		=	'r';
		data[10]	=	'o';
		data[11]	=	'r';
		data[12]	=	' ';
		#endif
		data[13]	=	0x00;
		
		DisplaySendNBytes( data, 14 );
	}
	else if( ( c & 0x20 ) == 0 )
	{
		#if defined( DEUTSCH )
		data[7]		=	'L';
		data[8]		=	'e';
		data[9]		=	'e';
		data[10]	=	'r';
		data[11]	=	'l';
		data[12]	=	'a';
		data[13]	=	'u';
		data[14]	=	'f';
		#endif
		#if defined( ENGLISCH )
		data[7]		=	'I';
		data[8]		=	'd';
		data[9]		=	'l';
		data[10]	=	'e';
		data[11]	=	' ';
		data[12]	=	' ';
		data[13]	=	' ';
		data[14]	=	' ';
		#endif
		data[15]	=	0x00;
		
		DisplaySendNBytes( data, 16 );
	}
	else if( ( ( c & 0x20 ) > 0 ) && ( ( c & 0x10 ) == 0 ) )
	{
		#if defined( DEUTSCH )
		data[7]		=	'v';
		data[8]		=	'o';
		data[9]		=	'r';
		data[10]	=	'b';
		data[11]	=	'e';
		data[12]	=	'r';
		data[13]	=	'e';
		data[14]	=	'i';
		data[15]	=	't';
		data[16]	=	'e';
		data[17]	=	'n';
		#endif
		#if defined( ENGLISCH )
		data[7]		=	'p';
		data[8]		=	'r';
		data[9]		=	'e';
		data[10]	=	'p';
		data[11]	=	'a';
		data[12]	=	'r';
		data[13]	=	'i';
		data[14]	=	'n';
		data[15]	=	'g';
		data[16]	=	' ';
		data[17]	=	' ';
		#endif
		data[18]	=	0x00;
		
		DisplaySendNBytes( data, 19 );
	}
	else if( ( ( c & 0x20 ) > 0 ) && ( ( c & 0x10 ) > 0 ) )
	{
		if( measurementType == 'm' )
		{
			#if defined( DEUTSCH )
			data[7]		=	'M';
			data[8]		=	'e';
			data[9]		=	's';
			data[10]	=	's';
			data[11]	=	'u';
			data[12]	=	'n';
			data[13]	=	'g';
			data[14]	=	0x00;
		
			DisplaySendNBytes( data, 15 );
			#endif
			#if defined( ENGLISCH )
			data[7]		=	'M';
			data[8]		=	'e';
			data[9]		=	'a';
			data[10]	=	's';
			data[11]	=	'u';
			data[12]	=	'r';
			data[13]	=	'e';
			data[14]	=	'm';
			data[15]	=	'e';
			data[16]	=	'n';
			data[17]	=	't';
			data[18]	=	0x00;
			
			DisplaySendNBytes( data, 19 );
			#endif
		}
		else if( measurementType == 'c' )
		{
			#if defined( DEUTSCH )
			data[7]		=	'K';
			data[8]		=	'a';
			data[9]		=	'l';
			data[10]	=	'i';
			data[11]	=	'b';
			data[12]	=	'r';
			data[13]	=	'a';
			data[14]	=	't';
			data[15]	=	'i';
			data[16]	=	'o';
			data[17]	=	'n';
			#endif
			#if defined( ENGLISCH )
			data[7]		=	'C';
			data[8]		=	'a';
			data[9]		=	'l';
			data[10]	=	'i';
			data[11]	=	'b';
			data[12]	=	'r';
			data[13]	=	'a';
			data[14]	=	't';
			data[15]	=	'i';
			data[16]	=	'o';
			data[17]	=	'n';
			#endif
			data[18]	=	0x00;
			
			DisplaySendNBytes( data, 19 );
		}
		else
		{
			if( sekunden <= 360 )
			{
				#if defined( DEUTSCH )
				data[7]		=	'K';
				data[8]		=	'a';
				data[9]		=	'l';
				data[10]	=	'i';
				data[11]	=	'b';
				data[12]	=	'r';
				data[13]	=	'a';
				data[14]	=	't';
				data[15]	=	'i';
				data[16]	=	'o';
				data[17]	=	'n';
				#endif
				#if defined( ENGLISCH )
				data[7]		=	'C';
				data[8]		=	'a';
				data[9]		=	'l';
				data[10]	=	'i';
				data[11]	=	'b';
				data[12]	=	'r';
				data[13]	=	'a';
				data[14]	=	't';
				data[15]	=	'i';
				data[16]	=	'o';
				data[17]	=	'n';
				#endif
				data[18]	=	0x00;
			
				DisplaySendNBytes( data, 19 );
			}
			else if( sekunden > 360 )
			{
				#if defined( DEUTSCH )
				data[7]		=	'M';
				data[8]		=	'e';
				data[9]		=	's';
				data[10]	=	's';
				data[11]	=	'u';
				data[12]	=	'n';
				data[13]	=	'g';
				data[14]	=	0x00;
		
				DisplaySendNBytes( data, 15 );
				#endif
				#if defined( ENGLISCH )
				data[7]		=	'M';
				data[8]		=	'e';
				data[9]		=	'a';
				data[10]	=	's';
				data[11]	=	'u';
				data[12]	=	'r';
				data[13]	=	'e';
				data[14]	=	'm';
				data[15]	=	'e';
				data[16]	=	'n';
				data[17]	=	't';
				data[18]	=	0x00;
			
				DisplaySendNBytes( data, 19 );
				#endif
			}		
		}			
	}
	else
	{
		data[7]		=	'-';
		data[8]		=	'-';
		data[9]		=	'-';
		data[10]	=	0x00;
		
		DisplaySendNBytes( data, 11 );
	}	
	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x82;
	data[4]		=	0x01;
	data[5]		=	0xED;
	data[6]		=	0x00;
	if( ( c & 0x08 ) > 0 )
	{
		#if defined( DEUTSCH )
		data[7]		=	'S';
		data[8]		=	'p';
		data[9]		=	'l';
		data[10]	=	'i';
		data[11]	=	't';
		data[12]	=	't';
		data[13]	=	'e';
		data[14]	=	'n';
		data[15]	=	' ';
		#endif
		#if defined( ENGLISCH )
		data[7]		=	'S';
		data[8]		=	'p';
		data[9]		=	'l';
		data[10]	=	'i';
		data[11]	=	't';
		data[12]	=	't';
		data[13]	=	'i';
		data[14]	=	'n';
		data[15]	=	'g';
		#endif
		data[16]	=	0x00;
		
		DisplaySendNBytes( data, 17 );
	}
	else if( ( ( c & 0x04 ) > 0 ) && ( ( c & 0x02 ) > 0 ) )
	{
		data[7]		=	'I';
		data[8]		=	'n';
		data[9]		=	'j';
		data[10]	=	'.';
		#if defined( DEUTSCH )
		data[11]	=	'K';
		#endif
		#if defined( ENGLISCH )
		data[11]	=	'C';
		#endif
		data[12]	=	'a';
		data[13]	=	'l';
		data[14]	=	'g';
		data[15]	=	'a';
		data[16]	=	's';
		data[17]	=	0x00;
		
		DisplaySendNBytes( data, 18 );
	}
	else if( ( ( c & 0x04 ) > 0 ) && ( ( c & 0x02 ) == 0 ) )
	{
		data[7]		=	'I';
		data[8]		=	'n';
		data[9]		=	'j';
		data[10]	=	'.';
		data[11]	=	'M';
		data[12]	=	'e';
		#if defined( DEUTSCH )
		data[13]	=	's';
		#endif
		#if defined( ENGLISCH )
		data[13]	=	'a';
		#endif
		data[14]	=	's';
		data[15]	=	'g';
		data[16]	=	'a';
		data[17]	=	's';
		data[18]	=	0x00;
		
		DisplaySendNBytes( data, 19 );
	}
	else
	{
		data[7]		=	'-';
		data[8]		=	'-';
		data[9]		=	'-';
		data[10]	=	0x00;
		
		DisplaySendNBytes( data, 11 );
	}
	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'Z';
	data[2]		=	'L';
	data[3]		=	0x82;
	data[4]		=	0x01;
	data[5]		=	0xFD;
	data[6]		=	0x00;
	if( ( c & 0x20 ) > 0 )
	{
		if( measurementType == 'c' )
		{
			#if defined( DEUTSCH )
			data[7]		=	'K';
			#endif
			#if defined( ENGLISCH )
			data[7]		=	'C';
			#endif
			data[8]		=	'a';
			data[9]		=	'l';
			data[10]	=	'i';
			data[11]	=	'b';
			data[12]	=	'r';
			data[13]	=	'a';
			data[14]	=	't';
			data[15]	=	'i';
			data[16]	=	'o';
			data[17]	=	'n';
			data[18]	=	0x00;
			
			DisplaySendNBytes( data, 19 );
		}
		else if( measurementType == 'm' )
		{
			#if defined( DEUTSCH )
			data[7]		=	'M';
			data[8]		=	'e';
			data[9]		=	's';
			data[10]	=	's';
			data[11]	=	'u';
			data[12]	=	'n';
			data[13]	=	'g';
			data[14]	=	0x00;
		
			DisplaySendNBytes( data, 15 );
			#endif
			#if defined( ENGLISCH )
			data[7]		=	'M';
			data[8]		=	'e';
			data[9]		=	'a';
			data[10]	=	's';
			data[11]	=	'u';
			data[12]	=	'r';
			data[13]	=	'e';
			data[14]	=	'm';
			data[15]	=	'e';
			data[16]	=	'n';
			data[17]	=	't';
			data[18]	=	0x00;
		
			DisplaySendNBytes( data, 19 );
			#endif
		}
		else if( measurementType == 'b' )
		{
			#if defined( DEUTSCH )
			data[7]		=	'K';
			#endif
			#if defined( ENGLISCH )
			data[7]		=	'C';
			#endif
			data[8]		=	'a';
			data[9]		=	'l';
			data[10]	=	'.';
			data[11]	=	'&';
			data[12]	=	'M';
			data[13]	=	'e';
			#if defined( DEUTSCH )
			data[14]	=	's';
			#endif
			#if defined( ENGLISCH )
			data[14]	=	'a';
			#endif
			data[15]	=	's';
			data[16]	=	'.';
			data[17]	=	0x00;
			
			DisplaySendNBytes( data, 18 );
		}
		else
		{
			data[7]		=	'-';
			data[8]		=	'-';
			data[9]		=	'-';
			data[10]	=	0x00;
			
			DisplaySendNBytes( data, 11 );
		}	
	}
	else
	{
		data[7]		=	'-';
		data[8]		=	'-';
		data[9]		=	'-';
		data[10]	=	0x00;
		
		DisplaySendNBytes( data, 11 );
	}	

	// Schriftfarbe auf default setzen	
	data[0]		=	0x1B;	//ESC
	data[1]		=	'F';
	data[2]		=	'Z';
	data[3]		=	0x01;
	data[4]		=	0x00;
		
	DisplaySendNBytes( data, 5 );	
}

/** SYSTEM VORDRUCK ISTWERT **************************************/
void SystemVordruckIstwert( void )
{
	unsigned short presl	=	0;
	unsigned short presh	=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x34 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();
//Neu Mai 2021 Anfang
	presl			=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	presh			=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();
//Neu Mai 2021 Ende

//	vordruck_istwert	=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

//Neu Mai 2021 Anfang
	pressure	=	presh;
	pressure	=	pressure << 8;
	pressure	=	pressure & 0xFF00;
	presl		=	presl & 0x00FF;
	pressure	+=	presl;

	vordruck_istwert	=	(unsigned short)(pressure/350);
//Neu Mai 2021 Ende
}

/** SYSTEM DETEKTOR VORGABE **************************************/
void SystemDetektorVorgabe( void )
{
	unsigned short detl		=	0;
	unsigned short deth		=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x41 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	detl			=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	deth			=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	detektor_vorgabe	=	deth;
	detektor_vorgabe	=	detektor_vorgabe << 8;
	detektor_vorgabe	=	detektor_vorgabe & 0xFF00;
	detl				=	detl & 0x00FF;
	detektor_vorgabe	+=	detl;
}

/** SYSTEM SAEULE VORGABE **************************************/
void SystemSaeuleVorgabe( void )
{
	unsigned short sael		=	0;
	unsigned short saeh		=	0;
	
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x42 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	sael			=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	saeh			=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	saeule_vorgabe		=	saeh;
	saeule_vorgabe		=	saeule_vorgabe << 8;
	saeule_vorgabe		=	saeule_vorgabe & 0xFF00;
	sael				=	sael & 0x00FF;
	saeule_vorgabe		+=	sael;
}

/** SYSTEM FLUSS VORGABE **************************************/
void SystemFlussVorgabe( void )
{
	unsigned short flul		=	0;
	unsigned short fluh		=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x43 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	flul			=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	fluh			=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	fluss_vorgabe		=	fluh;
	fluss_vorgabe		=	fluss_vorgabe << 8;
	fluss_vorgabe		=	fluss_vorgabe & 0xFF00;
	flul				=	flul & 0x00FF;
	fluss_vorgabe		+=	flul;
}

/** SYSTEM VORDRUCK VORGABE **************************************/
void SystemVordruckVorgabe( void )
{
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x44 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	vordruck_vorgabe	=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM EMPFINDLICHKEIT VORGABE **************************************/
void SystemEmpfindlichkeitVorgabe( void )
{
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x45 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	empfindlichkeit_vorgabe	=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM CANCEL MEASUREMENT **************************************/
void SystemCancelMeasurement( void )
{
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x16 );
	SystemPause();
	
	I2CSendByte( 0x01 );
	SystemPause();
	
	I2CRepStart();
	SystemPause();
	
	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();
	
	system_status		=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();
	
	I2CStop();
	SystemPause();	
}	

/** SYSTEM MA1 **************************************/
void SystemMA1( void )
{
	unsigned short ma1l		=	0;
	unsigned short ma1h		=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x46 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	ma1l			=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	ma1h			=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	mA1				=	ma1h;
	mA1				=	mA1 << 8;
	mA1				=	mA1 & 0xFF00;
	mA1				+=	ma1l;
}

/** SYSTEM MA2 **************************************/
void SystemMA2( void )
{
	unsigned short ma2l		=	0;
	unsigned short ma2h		=	0;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x47 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	ma2l			=	( unsigned short ) I2CReadByte( 0 );
	SystemPause();
	ma2h			=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	mA2				=	ma2h;
	mA2				=	mA2 << 8;
	mA2				=	mA2 & 0xFF00;
	mA2				+=	ma2l;
}

/** SYSTEM OK **************************************/
int SystemOK( void )
{
	BOOL ok		=	FALSE;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return ok;
	}	
	SystemPause();
	I2CSendByte( 0x48 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	ok			=	( BOOL ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	return ok;
}

/** SYSTEM MESSGAS CALGAS **************************************/
int SystemMessCalgas( void )
{
	BOOL messgas	=	TRUE;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return messgas;
	}	
	SystemPause();
	I2CSendByte( 0x49 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	messgas		=	( BOOL ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	return messgas;
}

/** SYSTEM INJEKTION **************************************/
int SystemInjektion( void )
{
	BOOL injektion	=	FALSE;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return injektion;
	}	
	SystemPause();
	I2CSendByte( 0x50 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	injektion	=	( BOOL ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	return injektion;
}

/** SYSTEM SPUELUNG **************************************/
int SystemSpuelung( void )
{
	BOOL spuelung	=	FALSE;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return spuelung;
	}	
	SystemPause();
	I2CSendByte( 0x51 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	spuelung	=	( BOOL ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();

	return spuelung;
}

/** SYSTEM GET RECENT VALUE **************************************/
void SystemGetRecentValue( void )
{
	int i			=	0;
	int j			=	0;
	
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	for( i=0; i<DELAY; i++ ) { Nop(); }
	I2CSendByte( 0x54 );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	
	I2CRepStart();
	for( i=0; i<DELAY; i++ ) { Nop(); }
	
	I2CSendByte( SYSTEM_ADDRR );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	
	for( j = 0; j < 7; j++ )
	{
		recentValue[j]		=	( unsigned char ) I2CReadByte( 0 );
		for( i=0; i<DELAY; i++ ) { Nop(); }
	}
	
	recentValue[7]		=	( unsigned char ) I2CReadByte( 1 );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	
	I2CStop();
	for( i=0; i<DELAY; i++ ) { Nop(); }
}

/** SYSTEM GET ALL VALUES **************************************/
void SystemGetAllValues( void )
{
	int i			=	0;
	int j			=	0;
	int k			=	0;
	
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	for( i=0; i<DELAY; i++ ) { Nop(); }
	I2CSendByte( 0x53 );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	
	I2CRepStart();
	for( i=0; i<DELAY; i++ ) { Nop(); }
	
	I2CSendByte( SYSTEM_ADDRR );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	
	for( j = 0; j < 5; j++ )
	{
		recentValue[j]		=	I2CReadByte( 0 );
		for( i=0; i<DELAY; i++ ) { Nop(); }
	}
	
	k			=	recentValue[4];
	k			=	k << 8;
	k			=	k & 0xF0;
	k			=	k + recentValue[3];
	
	for( j = 0; j < k-1; j++ )
	{
		messdaten[j]		=	I2CReadByte( 0 );
		for( i=0; i<DELAY; i++ ) { Nop(); }
	}
	messdaten[j]		=	I2CReadByte( 1 );
	for( i=0; i<DELAY; i++ ) { Nop(); }
}

/** SYSTEM CALC NEXT RUN **************************************/
void SystemCalcNextRun( char c )
{
	MemoryGetBetriebsparameter();
	ClockReadHours();
	ClockReadMinutes();
	
	itoa( hrs1, buffer );
	uhrzeitLetzterLauf[0]		=	buffer[0];
	itoa( hrs2, buffer );
	uhrzeitLetzterLauf[1]		=	buffer[0];
	uhrzeitLetzterLauf[2]		=	0x3A;
	
	itoa( min1, buffer );
	uhrzeitLetzterLauf[3]		=	buffer[0];
	itoa( min2, buffer );
	uhrzeitLetzterLauf[4]		=	buffer[0];
	uhrzeitLetzterLauf[5]		=	0x00;
			
	if( ( c == 'm' ) || ( c == 'b' ) )
	{
		calcMinRec					=	min1;
		calcMinRec					=	calcMinRec * 10;
		calcMinRec					=	calcMinRec + min2;
		calcHourRec					=	hrs1;
		calcHourRec					=	calcHourRec * 10;
		calcHourRec					=	calcHourRec + hrs2;
					
		if( messzyklus == 20 || messzyklus == 40 )
		{
			calcMinNext					=	calcMinRec + messzyklus;
			calcMinNext					=	calcMinNext % 60;
			calcHourNext				=	calcMinRec + messzyklus;
			calcHourNext				=	floor( ( calcHourNext / 60 ) );
			calcHourNext				=	calcHourNext + calcHourRec;
			calcHourNext				=	calcHourNext % 24;
		}
		else if( messzyklus <= 19 )
		{
			calcMinNext					=	calcMinRec;
			calcHourNext				=	calcHourRec + messzyklus;
			calcHourNext				=	calcHourNext % 24;
		}	
		else
		{
			calcMinNext					=	calcMinRec;
			calcHourNext				=	calcHourRec + messzyklus - 1;
			calcHourNext				=	calcHourNext % 24;
		}		
		
		itoa( calcHourNext, buffer );
		if( calcHourNext < 10 )
		{
			uhrzeitNaechsteMessung[0]	=	0x30;
			uhrzeitNaechsteMessung[1]	=	buffer[0];
		}
		else
		{
			uhrzeitNaechsteMessung[0]	=	buffer[0];
			uhrzeitNaechsteMessung[1]	=	buffer[1];
		}	
		uhrzeitNaechsteMessung[2]	=	0x3A;
			
		itoa( calcMinNext, buffer );
		if( calcMinNext < 10 )
		{
			uhrzeitNaechsteMessung[3]	=	0x30;
			uhrzeitNaechsteMessung[4]	=	buffer[0];
		}
		else
		{
			uhrzeitNaechsteMessung[3]	=	buffer[0];
			uhrzeitNaechsteMessung[4]	=	buffer[1];
		}	
		uhrzeitNaechsteMessung[5]	=	0x00;
	}
		
	if( ( c == 'c' ) || ( c == 'b' ) )
	{
		calcMinRec					=	min1;
		calcMinRec					=	calcMinRec * 10;
		calcMinRec					=	calcMinRec + min2;
		calcHourRec					=	hrs1;
		calcHourRec					=	calcHourRec * 10;
		calcHourRec					=	calcHourRec + hrs2;
		
		if( eichzyklus == 20 || eichzyklus == 40 )
		{
			calcMinNext					=	calcMinRec + eichzyklus;
			calcMinNext					=	calcMinNext % 60;
			calcHourNext				=	calcMinRec + eichzyklus;
			calcHourNext				=	floor( ( calcHourNext / 60 ) );
			calcHourNext				=	calcHourNext + calcHourRec;
			calcHourNext				=	calcHourNext % 24;
		}
		else if( eichzyklus <= 19 )
		{
			calcMinNext					=	calcMinRec;
			calcHourNext				=	calcHourRec + eichzyklus;
			calcHourNext				=	calcHourNext % 24;
		}
		else
		{
			calcMinNext					=	calcMinRec;
			calcHourNext				=	calcHourRec + eichzyklus - 1;
			calcHourNext				=	calcHourNext % 24;
		}		
					
		itoa( calcHourNext, buffer );
		if( calcHourNext < 10 )
		{
			uhrzeitNaechsteKalib[0]		=	0x30;
			uhrzeitNaechsteKalib[1]		=	buffer[0];
		}
		else
		{
			uhrzeitNaechsteKalib[0]		=	buffer[0];
			uhrzeitNaechsteKalib[1]		=	buffer[1];
		}	
		uhrzeitNaechsteKalib[2]		=	0x3A;
			
		itoa( calcMinNext, buffer );
		if( calcMinNext < 10 )
		{
			uhrzeitNaechsteKalib[3]		=	0x30;
			uhrzeitNaechsteKalib[4]		=	buffer[0];
		}
		else
		{
			uhrzeitNaechsteKalib[3]		=	buffer[0];
			uhrzeitNaechsteKalib[4]		=	buffer[1];
		}	
		uhrzeitNaechsteKalib[5]		=	0x00;
	}	
}

/** SYSTEM GET TIME STRING **************************************/
void SystemGetTimeString( void )
{
	ClockReadHours();
	ClockReadMinutes();
	
	itoa( hrs1, buffer );
	uhrzeitAktuell[0]		=	buffer[0];
	itoa( hrs2, buffer );
	uhrzeitAktuell[1]		=	buffer[0];
	uhrzeitAktuell[2]		=	0x3A;
	
	itoa( min1, buffer );
	uhrzeitAktuell[3]		=	buffer[0];
	itoa( min2, buffer );
	uhrzeitAktuell[4]		=	buffer[0];
	uhrzeitAktuell[5]		=	0x00;
}	

/** SYSTEM STATUS **************************************/
//	Reads the system status of the device
//	Device status will be stored in the global variable
//	system_status

void SystemStatus( void )
{
	unsigned short status_old = system_status;

	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x40 );
	SystemPause();

	I2CRepStart();
	SystemPause();

	I2CSendByte( SYSTEM_ADDRR );
	SystemPause();

	system_status		=	( unsigned short ) I2CReadByte( 1 );
	SystemPause();

	I2CStop();
	SystemPause();
}

/** SYSTEM CHECK FLUSS *****************************************/
void SystemCheckFluss( void )
{
	SystemFlussIstwert();
	MemoryGetFluss();
	
	if( fluss_istwert <= ( fluss / 2 ) )
	{
		malfunction		=	TRUE;
		errorCode		=	1;
	}	
}	

/** SYSTEM CLEAR END FLAG *****************************************/
//	Clears the end flag of the device status

void SystemClearEndFlag( void )
{
	int i			=	0;
	
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	for( i=0; i<DELAY; i++ ) { Nop(); }
	I2CSendByte( 0x12 );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	I2CSendByte( 0x01 );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	I2CStop();
	for( i=0; i<DELAY; i++ ) { Nop(); }
}	

/** SYSTEM CLEAR RESET FLAG *****************************************/
//	Clears the reset flag of the device status

void SystemClearResetFlag( void )
{
	int i			=	0;
	
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	for( i=0; i<DELAY; i++ ) { Nop(); }
	I2CSendByte( 0x15 );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	I2CSendByte( 0x01 );
	for( i=0; i<DELAY; i++ ) { Nop(); }
	I2CStop();
	for( i=0; i<DELAY; i++ ) { Nop(); }
}	

/** SYSTEM START SPUELEN *****************************************/
void SystemStartSpuelen( void )
{
	int i			=	0;
	
	I2CStart();
	if( I2CSendByte( SYSTEM_ADDRW ) == 1 )
	{
		SystemPause();
		I2CStop();
		SystemPause();
		return;
	}	
	SystemPause();
	I2CSendByte( 0x13 );
	SystemPause();
	I2CSendByte( 0x01 );
	SystemPause();
	I2CStop();
	SystemPause();
}	

/** DELAY *********************************************************/
void Delay( void )
{
	Nop();Nop();Nop();Nop();Nop();Nop();
	Nop();Nop();Nop();Nop();Nop();Nop();
	Nop();Nop();Nop();Nop();Nop();Nop();
}	

/** ITOA **********************************************************/
//	Function gets an int value and stores the numbers in the 
//	char string in ascii code

void itoa(int value, char* str, int base)
{
    char *p = str, *p1 = str, tmp_char;
    int tmp_value;

    if (value < 0 && base == 10) {
        *p++ = '-';
        value = -value;
        p1++;
    }

    do {
        tmp_value = value;
        value /= base;
        *p++ = "0123456789ABCDEF"[tmp_value - value * base];
    } while (value);

    *p-- = '\0';

    while (p1 < p) {
        tmp_char = *p;
        *p-- = *p1;
        *p1++ = tmp_char;
    }
}

 
 /** LITOA **********************************************************/
//	Function gets an int value and stores the numbers in the 
//	char string in ascii code


void DisplayClearLine(unsigned char y)
{
    char data[10];


    data[0] = 0x1B;     // ESC
    data[1] = 'R';      // Rectangle
    data[2] = 'L';      // Line
    data[3] = 0;     // X1
    data[4] = y;        // Y1
    data[5] = 479;     // X2
    data[6] = y + 15 ;        // Y2

    DisplaySendNBytes(data, 7);
}





/**void DisplayDrawStaticLabels( void )
{
    // Draw "Date:"
    data[0] = 0x1B;
    data[1] = 0x5A;
    data[2] = 0x4C;
    data[3] = 0x5A;        // x = 90
    data[4] = 0x00;
    data[5] = 0x46;        // y = 70
    data[6] = 'D';
    data[7] = 'a';
    data[8] = 't';
    data[9] = 'e';
    data[10] = ':';
    data[11] = 0x00;
    DisplaySendNBytes(data, 12);

    // Draw "Time:"
    data[0] = 0x1B;
    data[1] = 0x5A;
    data[2] = 0x4C;
    data[3] = 0x5A;        // x = 90
    data[4] = 0x00;
    data[5] = 0x5A;        // y = 90
    data[6] = 'T';
    data[7] = 'i';
    data[8] = 'm';
    data[9] = 'e';
    data[10] = ':';
    data[11] = 0x00;
    DisplaySendNBytes(data, 12);
}*/


/** EOF display.c ************************************************************/
