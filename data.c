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
#include "data.h"
#include "definitions.h"

/** VARIABLES *******************************************************/
unsigned short day_h;
unsigned short day_l;
unsigned short month_h;
unsigned short month_l;
unsigned short year_h;
unsigned short year_l;
static SearchRec findData;
FSFILE *readData				=	NULL;
char filename[13];
char readFile[15];
char readBuffer[20];
unsigned char file_read[2048];
unsigned int SN = 0;   // Seriennummer z. B. 1234


void DataInitRead( char *date )
{
	int i;
	
	for( i = 0; i < 20; i++ )
	{
		readDirectory[i]	=	date[i];
	}
	readDirectory[i]	=	0x00;
		
	year_h		=	date[12];
	year_l		=	date[13];
	month_h		=	date[15];
	month_l		=	date[16];
	day_h		=	date[18];
	day_l		=	date[19];
}	

void DataGetPreviousDay( void )
{
	if( day_l >= 0x32 )
	{
		day_l--;
	}	
	else if( ( day_l == 0x31 ) && ( day_h >= 0x31 ) )
	{
		day_l		=	0x30;
	}	
	else if( ( day_l == 0x30 ) && ( day_h >= 0x31 ) )
	{
		day_l		=	0x39;
		day_h--;
	}	
	else if( ( day_l == 0x31 ) && ( day_h == 0x30 ) )
	{
		if( ( month_h == 0x31 ) && ( month_l == 0x32 ) )
		{
			month_l		=	0x31;
			day_h		=	0x33;
			day_l		=	0x30;
		}	
		else if( ( month_h == 0x31 ) && ( month_l == 0x31 ) )
		{
			month_l		=	0x30;
			day_h		=	0x33;
			day_l		=	0x31;
		}	
		else if( ( month_h == 0x31 ) && ( month_l == 0x30 ) )
		{
			month_h		=	0x30;
			month_l		=	0x39;
			day_h		=	0x33;
			day_l		=	0x30;
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x39 ) )
		{
			month_l		=	0x38;
			day_h		=	0x33;
			day_l		=	0x31;
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x38 ) )
		{
			month_l		=	0x37;
			day_h		=	0x33;
			day_l		=	0x31;
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x37 ) )
		{
			month_l		=	0x36;
			day_h		=	0x33;
			day_l		=	0x30;
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x36 ) )
		{
			month_l		=	0x35;
			day_h		=	0x33;
			day_l		=	0x31;
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x35 ) )
		{
			month_l		=	0x34;
			day_h		=	0x33;
			day_l		=	0x30;
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x34 ) )
		{
			month_l		=	0x33;
			day_h		=	0x33;
			day_l		=	0x31;
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x33 ) )
		{
			month_l		=	0x32;
			day_h		=	0x32;
			if( DataIsLeapYear() ) {
				day_l		=	0x39;
			}	
			else {
				day_l		=	0x38;
			}	
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x32 ) )
		{
			month_l		=	0x31;
			day_h		=	0x33;
			day_l		=	0x31;
		}	
		else if( ( month_h == 0x30 ) && ( month_l == 0x31 ) )
		{
			if( year_l > 0x30 )
			{
				year_l--;
				month_h		=	0x31;
				month_l		=	0x32;
				day_h		=	0x33;
				day_l		=	0x31;
			}	
			else if( ( year_l == 0x00 ) && ( year_h > 0x00 ) )
			{
				year_h--;
				year_l		=	0x39;
				month_h		=	0x31;
				month_l		=	0x32;
				day_h		=	0x33;
				day_l		=	0x31;
			}	
		}	
	}
	
	readDirectory[12]		=	year_h;
	readDirectory[13]		=	year_l;
	readDirectory[15]		=	month_h;
	readDirectory[16]		=	month_l;
	readDirectory[18]		=	day_h;
	readDirectory[19]		=	day_l;
		
}	

BOOL DataIsLeapYear() {
	int year = 0;
	
	year = ( year_h - 0x30 );
	year = year * 10;
	year = year + ( year_l - 0x30 );
	year = year + 2000;
	
	if( year % 4 == 0 ) {
		if( year % 100 == 0 ) {
			if( year % 400 == 0 ) {
				return TRUE;
			}	
			else {
				return FALSE;
			}	
		}	
		else {
			return TRUE;
		}	
	}	
	else {
		return FALSE;
	}	
}	

void DataShowDayResults( unsigned int pageNo )
{
	BOOL directoryExists		=	FALSE;
	unsigned int firstFile		=	0;
	unsigned int lastFile		=	0;
	unsigned int i				=	0;
	unsigned int j				=	0;
	
	//Display löschen
	readFile[0]		=	0x1B;
	readFile[1]		=	'R';
	readFile[2]		=	'L';
	readFile[3]		=	0x01;
	readFile[4]		=	0x00;
	readFile[5]		=	0x15;
	readFile[6]		=	0x00;
	readFile[7]		=	0xDE;
	readFile[8]		=	0x01;
	readFile[9]		=	0xDA;
	readFile[10]	=	0x00;
	
	DisplaySendNBytes( readFile, 11 );
	
	//Datum anzeigen
	DisplayShowDataDatum();
		
	//In Ordner wechseln
	FSchdir( ( char* ) 0x5C );
	if( FSchdir( readDirectory ) == 0 )
	{
		directoryExists		=	TRUE;
	}
	
	//Ordner exisitert nicht, somit auch keine Daten
	if( directoryExists == FALSE )
	{
		DisplayShowNoData();
		return;
	}	
	else
	{
		//Daten vorhanden:
		//Filenamen vorbereiten
		itoa( SN, buffer );
		
		filename[0]		=	buffer[0];
		filename[1]		=	buffer[1];
		filename[2]		=	buffer[2];
		filename[3]		=	buffer[3];
		filename[4]		=	0x30;
		
		filename[8]		=	'.';
		filename[9]		=	't';
		filename[10]	=	'x';
		filename[11]	=	't';
		filename[12]	=	0x00;
		
		//Seite bestimmen, angezeigt werden 36 Messergebnisse pro Seite
		lastFile		=	37 + ( 36 * pageNo );
		
		//Schleife, die 36 Mal durchlaufen wird
		for( firstFile = ( 36 * pageNo ) + 1; firstFile < lastFile; firstFile++ )
		{
			//Filenamen vervollständigen (Zahl am Ende ersetzen)
			itoa( firstFile, buffer );
			if( firstFile < 10 )
			{
				filename[5]		=	0x30;
				filename[6]		=	0x30;
				filename[7]		=	buffer[0];
			}
			else if( firstFile < 100 )
			{
				filename[5]		=	0x30;
				filename[6]		=	buffer[0];
				filename[7]		=	buffer[1];
			}
			else
			{
				filename[5]		=	buffer[0];
				filename[6]		=	buffer[1];
				filename[7]		=	buffer[2];
			}			
			
			//Datei suchen
			if( FindFirst( filename, ( unsigned int ) 0x3F, &findData ) == 0 )
			{
				//Datei öffnen
				readData		=	FSfopen( findData.filename, READ );
				//Cursor auf Position Uhrzeit setzen
				FSfseek( readData, 185, SEEK_SET );
				//String mit Uhrzeit lesen
				FSfread( &readBuffer, 20, 1, readData );
				
				//Uhrzeit im String suchen
				for( i = 1; i < 20; i++ )
				{
					if( readBuffer[i] == ' ' )
					{
						if( readBuffer[i-1] == ':' )
						{
							j		=	i + 1;
						}	
					}	
				}	
				
				//Uhrzeit in String einbauen
				readFile[0]		=	readBuffer[j];
				readFile[1]		=	readBuffer[j+1];
				readFile[2]		=	readBuffer[j+2];
				readFile[3]		=	readBuffer[j+3];
				readFile[4]		=	readBuffer[j+4];
				readFile[5]		=	' ';
				readFile[6]		=	':';
				
				//Cursor auf Position Messergebnis setzen
				FSfseek( readData, 288, SEEK_SET );
				//String mit Messergebnis lesen
				FSfread( &readBuffer, 20, 1, readData );
				
				//Messergenis in String suchen
				j				=	0;
				for( i = 1; i < 20; i++ )
				{
					if( readBuffer[i] == ' ' )
					{
						if( readBuffer[i-1] == ':' )
						{
							j	=	i + 1;
						}	
					}	
				}	
				
				//Messergebnis in String einbinden
				readFile[7]		=	readBuffer[j];
				readFile[8]		=	readBuffer[j+1];
				readFile[9]		=	readBuffer[j+2];
				readFile[10]	=	readBuffer[j+3];
				readFile[11]	=	' ';
				readFile[12]	=	'm';
				readFile[13]	=	'g';
				readFile[14]	=	0x00;
				
				//Datei schliessen
				FSfclose( readData );
				readData		=	NULL;
				
				//String anzeigen
				DisplayShowDataResult( readFile, firstFile );
			}	
			else
			{
				//Datei nicht gefunden:
				//Anzeige keine Daten + Taste nächste Seite disablen + Funktion verlassen
				DisplayShowDataEnd( firstFile );
				dateiNo			=	firstFile - 1;
				return;
			}
			DisplayShowDataEnableNextPage();
		}
		dateiNo			=	firstFile - 1;	
	}	
}

void DataShowGraphResults( void )
{
	BOOL directoryExists		=	FALSE;
	unsigned int hour			=	0;
	unsigned int minute			=	0;
	unsigned int messwert		=	0;
	unsigned int xPos			=	0;
	unsigned char x1Pos			=	0;
	unsigned char x2Pos			=	0;
	unsigned char yPos			=	0;
	unsigned int i				=	0;
	unsigned int j				=	0;
	unsigned int k				=	0;
	unsigned int skale			=	0;
	double scale				=	0;
	
	//Datum anzeigen
	DisplayShowDataDatum();
		
	//In Ordner wechseln
	FSchdir( ( char* ) 0x5C );
	if( FSchdir( readDirectory ) == 0 )
	{
		directoryExists		=	TRUE;
	}
	
	//Ordner exisitert nicht, somit auch keine Daten
	if( directoryExists == FALSE )
	{
		DisplayShowNoData();
		return;
	}	
	else
	{
		//Daten vorhanden:
		//Filenamen vorbereiten
		itoa( SN, buffer );
		
		filename[0]		=	buffer[0];
		filename[1]		=	buffer[1];
		filename[2]		=	buffer[2];
		filename[3]		=	buffer[3];
		filename[4]		=	0x30;
		
		filename[8]		=	'.';
		filename[9]		=	't';
		filename[10]	=	'x';
		filename[11]	=	't';
		filename[12]	=	0x00;
		
		//Schleife, die 108 Mal durchlaufen wird
		for( i = 1; i <= 108; i++ )
		{
			//Filenamen vervollständigen (Zahl am Ende ersetzen)
			itoa( i, buffer );
			if( i < 10 )
			{
				filename[5]		=	0x30;
				filename[6]		=	0x30;
				filename[7]		=	buffer[0];
			}
			else if( i < 100 )
			{
				filename[5]		=	0x30;
				filename[6]		=	buffer[0];
				filename[7]		=	buffer[1];
			}
			else
			{
				filename[5]		=	buffer[0];
				filename[6]		=	buffer[1];
				filename[7]		=	buffer[2];
			}			
			
			//Datei suchen
			if( FindFirst( filename, ( unsigned int ) 0x3F, &findData ) == 0 )
			{
				//Datei öffnen
				readData		=	FSfopen( findData.filename, READ );
				//Cursor auf Position Uhrzeit setzen
				FSfseek( readData, 185, SEEK_SET );
				//String mit Uhrzeit lesen
				FSfread( &readBuffer, 20, 1, readData );
				
				//Uhrzeit im String suchen
				for( j = 1; j < 20; j++ )
				{
					if( readBuffer[j] == ' ' )
					{
						if( readBuffer[j-1] == ':' )
						{
							k		=	j + 1;
						}	
					}	
				}	
				
				//Studen und Minuten merken
				readFile[0]		=	readBuffer[k];		//std_h
				readFile[1]		=	readBuffer[k+1];	//std_l
				readFile[2]		=	readBuffer[k+3];	//min_h
				readFile[3]		=	readBuffer[k+4];	//min_l
				
				//Cursor auf Position Messergebnis setzen
				FSfseek( readData, 288, SEEK_SET );
				//String mit Messergebnis lesen
				FSfread( &readBuffer, 20, 1, readData );
				
				//Messergenis in String suchen
				k				=	0;
				for( j = 1; j < 20; j++ )
				{
					if( readBuffer[j] == ' ' )
					{
						if( readBuffer[j-1] == ':' )
						{
							k	=	j + 1;
						}	
					}	
				}	
				
				//Messergebnis in String einbinden
				//kleiner 10?
				if( readBuffer[k+1] == '.' )
				{
					readFile[4]		=	0x30;
					readFile[5]		=	readBuffer[k];
					readFile[6]		=	readBuffer[k+1];
					readFile[7]		=	readBuffer[k+2];
				}	
				else
				{
					readFile[4]		=	readBuffer[k];		//vorkomma_h
					readFile[5]		=	readBuffer[k+1];	//vorkomma_l
					readFile[6]		=	readBuffer[k+2];	//.
					readFile[7]		=	readBuffer[k+3];	//nachkomma
				}	
				
				//Datei schliessen
				FSfclose( readData );
				readData		=	NULL;
				
				//ASCII in Zahlen umwandeln		
				if( readFile[0] == 0x30 ) { hour = 0; }
				else if( readFile[0] == 0x31 ) { hour = 10; }
				else if( readFile[0] == 0x32 ) { hour = 20; }
				
				if( readFile[1] == 0x30 ) { hour += 0; }
				else if( readFile[1] == 0x31 ) { hour += 1; }
				else if( readFile[1] == 0x32 ) { hour += 2; }
				else if( readFile[1] == 0x33 ) { hour += 3; }
				else if( readFile[1] == 0x34 ) { hour += 4; }
				else if( readFile[1] == 0x35 ) { hour += 5; }
				else if( readFile[1] == 0x36 ) { hour += 6; }
				else if( readFile[1] == 0x37 ) { hour += 7; }
				else if( readFile[1] == 0x38 ) { hour += 8; }
				else if( readFile[1] == 0x39 ) { hour += 9; }
				
				if( readFile[2] == 0x30 ) { minute = 0; }
				else if( readFile[2] == 0x31 ) { minute = 1; }
				else if( readFile[2] == 0x32 ) { minute = 2; }
				else if( readFile[2] == 0x33 ) { minute = 3; }
				else if( readFile[2] == 0x34 ) { minute = 4; }
				else if( readFile[2] == 0x35 ) { minute = 5; }
				
				if( readFile[4] == 0x30 ) { messwert = 0; }
				else if( readFile[4] == 0x31 ) { messwert = 100; }
				else if( readFile[4] == 0x32 ) { messwert = 200; }
				else if( readFile[4] == 0x33 ) { messwert = 300; }
				else if( readFile[4] == 0x34 ) { messwert = 400; }
				else if( readFile[4] == 0x35 ) { messwert = 500; }
				else if( readFile[4] == 0x36 ) { messwert = 600; }
				else if( readFile[4] == 0x37 ) { messwert = 700; }
				else if( readFile[4] == 0x38 ) { messwert = 800; }
				else if( readFile[4] == 0x39 ) { messwert = 900; }
				
				if( readFile[5] == 0x30 ) { messwert += 0; }
				else if( readFile[5] == 0x31 ) { messwert += 10; }
				else if( readFile[5] == 0x32 ) { messwert += 20; }
				else if( readFile[5] == 0x33 ) { messwert += 30; }
				else if( readFile[5] == 0x34 ) { messwert += 40; }
				else if( readFile[5] == 0x35 ) { messwert += 50; }
				else if( readFile[5] == 0x36 ) { messwert += 60; }
				else if( readFile[5] == 0x37 ) { messwert += 70; }
				else if( readFile[5] == 0x38 ) { messwert += 80; }
				else if( readFile[5] == 0x39 ) { messwert += 90; }
				
				if( readFile[7] == 0x30 ) { messwert += 0; }
				else if( readFile[7] == 0x31 ) { messwert += 1; }
				else if( readFile[7] == 0x32 ) { messwert += 2; }
				else if( readFile[7] == 0x33 ) { messwert += 3; }
				else if( readFile[7] == 0x34 ) { messwert += 4; }
				else if( readFile[7] == 0x35 ) { messwert += 5; }
				else if( readFile[7] == 0x36 ) { messwert += 6; }
				else if( readFile[7] == 0x37 ) { messwert += 7; }
				else if( readFile[7] == 0x38 ) { messwert += 8; }
				else if( readFile[7] == 0x39 ) { messwert += 9; }
				
				//xPos bestimmen
				xPos		=	25 + ( hour * 18 ) + ( minute * 3 );
				
				if( xPos < 256 )
				{
					x1Pos		=	0x00;
					x2Pos		=	xPos;
				}
				else
				{
					x1Pos		=	0x01;
					x2Pos		=	xPos & 0x00FF;
				}		
				
				//Skalierungsfaktor bestimmen
				skale		=	skalierung / 10;
				if( ( skalierung % 10 ) != 0 )
				{
					skale		=	skale + 1;
				}	
				skale		=	skale * 10;
				scale		=	( double ) 160 / ( double ) skale;
				scale		=	scale / 10;
				
				//yPos bestimmen
				yPos		=	( unsigned char ) ( 200 - ( messwert * scale ) );
				
				if( i == 1 )
				{
					//erster Punkt
					DisplayShowDayValueFirst( x1Pos, x2Pos, yPos );
				}
				else
				{	
					//weitere Werte zeichnen
					DisplayShowDayValue( x1Pos, x2Pos, yPos );
				}	
			}	
			else
			{
				//Datei nicht gefunden
				return;
			}
		}	
	}	
}	

void DataShowMeasurement( unsigned int measurementNo )
{
	unsigned int i, j, type;
	
	//DIRECTORY WECHSELN
	FSchdir( ( char* ) 0x5C );
	FSchdir( readDirectory );
	
	itoa( SN, buffer );
	
	//FILENAME VORBEREITEN
	filename[0]		=	buffer[0];
	filename[1]		=	buffer[1];
	filename[2]		=	buffer[2];
	filename[3]		=	buffer[3];
	filename[4]		=	0x30;
		
	filename[8]		=	'.';
	filename[9]		=	't';
	filename[10]	=	'x';
	filename[11]	=	't';
	filename[12]	=	0x00;
	
	itoa( measurementNo, buffer );
	
	//FILENAMEN VERVOLLSTÄNDIGEN
	if( measurementNo < 10 )
	{
		filename[5]		=	0x30;
		filename[6]		=	0x30;
		filename[7]		=	buffer[0];
	}	
	else if( measurementNo < 100 )
	{
		filename[5]		=	0x30;
		filename[6]		=	buffer[0];
		filename[7]		=	buffer[1];
	}	
	else
	{
		filename[5]		=	buffer[0];
		filename[6]		=	buffer[1];
		filename[7]		=	buffer[2];
	}
	
	//DATEI ÖFFNEN, LESEN, SCHLIESSEN
	readData			=	FSfopen( filename, READ );
	FSfread( file_read, 2048, 1, readData );
	FSfclose( readData );
	readData			=	NULL;
	
	//INFO ART DER MESSUNG SUCHEN
	for( i = 10; i < 620; i++ )
	{
		if( file_read[i] == ' ' )
		{
			if( file_read[i-1] == ':' )
			{
				if( file_read[i-2] == 'g' )
				{
					if( file_read[i-3] == 'n' )
					{
						if( file_read[i-4] == 'u' )
						{
							if( file_read[i+1] == 'c' )
							{
								type		=	0;
								i			=	630;
							}
							else if( file_read[i+1]	== 'm' )
							{
								type		=	1;
								i			=	630;
							}
							else if( file_read[i+1] == 'b' )
							{
								type		=	2;
								i			=	630;
							}		
						}	
					}	
				}	
			}	
		}	
	}	
	
	//MESSDATEN SUCHEN
	j = 0;
	for( i = 200; i < 620; i++ )
	{
		if( file_read[i] == ':' )
		{
			if( file_read[i-1] == ':' )
			{
				if( file_read[i-2] == ':' )
				{
					if( file_read[i-3] == 'N' )
					{
						if( file_read[i-4] == 'I' )
						{
							j			=	i + 2;
							i			=	630;
						}	
					}	
				}	
			}	
		}	
	}		
	
	//MESSDATEN IN ARRAY KOPIEREN	
	for( i = 0; i < 1440; i+=2 )
	{
		messdaten[i/2]		=	file_read[j+1];
		messdaten[i/2]		=	messdaten[i/2] << 8;
		messdaten[i/2]		=	messdaten[i/2] += file_read[j];
		j+=2;
	}
	
	//MESSDATEN ZEICHNEN
	if( ( type == 0 ) || ( type == 1 ) )
	{
		meas6min		=	TRUE;
		for( i = 0; i < 360; i+=3 )
		{
			GraphNewValue( 200 - ( unsigned short ) ( ( messdaten[i+1] ) * 0.04395 ) );
			GraphNewValue( 200 - ( unsigned short ) ( ( messdaten[i+2] ) * 0.04395 ) );
//			messpunkte[i]	=	( 200 - ( unsigned short ) ( ( file_read_int[i] - 512 ) * 0.11726 ) );
//			messpunkte[i+1]	=	( 200 - ( unsigned short ) ( ( file_read_int[i+1] - 512 ) * 0.11726 ) );
		}
	}
	else if( type == 2 )
	{
		meas6min		=	FALSE;
		for( i = 0; i < 720; i+= 3 )
		{
			GraphNewValue( 200 - ( unsigned short ) ( ( messdaten[i] ) * 0.04395 ) ) ;
		}	
	}			

	//DISPLAY LÖSCHEN	
	buffer[0]		=	0x1B;
	buffer[1]		=	'D';
	buffer[2]		=	'L';
	DisplaySendNBytes( buffer, 3 );
	
	//GRAPH ZEICHNEN
	GraphDrawGraph();

#if defined ABLEITUNG_ALT		
	AbleitungInit();
	if( type == 0 )
	{
		AbleitungAb0( 0 );
	}	
	else if( type == 1 )
	{
		AbleitungAb0( 1 );
	}	
	else if( type == 2 )
	{
		AbleitungAb0( 0 );
		AbleitungInit();
		AbleitungAb360();
	}
#endif

#if defined ABLEITUNG_NEU
	AbleitungInit_neu();
	AbleitungCalc(type == 2 ? 'b' : 'm');
#endif

	
	DisplayShowMessergebnis();	
}

//EOF

