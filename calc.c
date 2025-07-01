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
#include "pc.h"

/** Variablen für die Ableitungen und Integration **/
long double function1[20];
long double function2[20];
long double ableitung1[20];
long double ableitung2[20];
long double summeXY1;
long double summeXY2;
long double summeX1;
long double summeX2;
long double summeY1;
long double summeY2;
long double divisor;
long double overhead				=	0;
long double schwelle1				=	2;
long double schwelle2				=	-1;
unsigned int anzahlWerte			=	13;

int bereich					=	0;
int bereichCount			=	0;
int retZeit1Min				=	120;
int retZeit1Max				=	180;
int retZeit2Min				=	480;
int retZeit2Max				=	540;

int peakSignalStart			=	0;
int peakSignalMitte			=	0;
int peakSignalEnde			=	0;

BOOL empty1					=	TRUE;
BOOL empty2					=	TRUE;

//NEUE VARIABLEN FUER DIE NEUE ABLEITUNG NOVEMBER 2020
double ableitung_neu[720];

struct mark{
	double x;
	double y;
	enum {Zero, Positiv, Negativ} vz;
};
int markIndex;
struct mark marks[20];
int indexMax;
int marksMax[2];

double p1;
double p2;
double p3;
double p2y;
double m;

double integralBereich[2];
int vzCnt;



/** Funktionen *************************************/

/** AbleitungInit *************************************************
 *
 *	Funktion berechnet den Divisor, den Stützstellenabstand
 *	und Initialisiert die beiden Ableitungsarrays mit 0
 *
 *	Wird regelmäßig vor dem Beginn einer Ableitungsbestimmtung
 *	ausgeführt
 *
 ******************************************************************/
void AbleitungInit( void )
{
	short i					=	0;
	long int abstand		=	0;
	
	for( i = 1; i < anzahlWerte; i++ )
	{
		abstand				=	abstand + ( i * i );
	}
	divisor					=	abstand * anzahlWerte;
	abstand					=	0;
	for( i = 1; i < anzahlWerte; i++ )
	{
		abstand				=	abstand + i;
	}
	summeX1					=	abstand;
	summeX2					=	abstand;
	divisor					=	divisor - ( abstand * abstand );
	for( i = 0; i < anzahlWerte; i++ )
	{
		ableitung1[i]		=	0;
		ableitung2[i]		=	0;
	}
	
	peakStart				=	0;
	peakMitte				=	0;
	peakEnde				=	0;
}

/** ABLEITUNG 1 CALC **********************************************
 *
 *	Funktion bekommt einen neuen Wert übergeben, sortiert den
 *	ersten Wert aus, fügt den neuen als letzten hinzu und berechnet
 *	die erste Ableitung
 *
 ******************************************************************/
void Ableitung1Calc( long double value )
{
	short i					=	0;
	
	if( empty1 )
	{
		summeY1				=	0;
		empty1				=	FALSE;
		for( i = 0; i < anzahlWerte; i++ )
		{
			function1[i]	=	value;
			summeY1			=	summeY1 + value;
		}
	}
	else
	{
		summeY1				=	summeY1 - function1[0];
		for( i = 1; i < anzahlWerte; i++ )
		{
			function1[i-1]	=	function1[i];
		}
		function1[anzahlWerte-1]		=	value;
		summeY1							=	summeY1 + value;
	}
	summeXY1				=	0;
	for( i = 0; i < anzahlWerte; i++ )
	{
		summeXY1			=	summeXY1 + ( function1[i] * i );
	}
	for( i = 1; i < anzahlWerte; i++ )
	{
		ableitung1[i-1]		=	ableitung1[i];
	}
	summeXY1				=	summeXY1 * anzahlWerte;
	summeXY1				-=	( summeX1 * summeY1 );
	ableitung1[anzahlWerte-1]			=	summeXY1 / divisor;
}

/** Ableitung2Calc *********************************/
void Ableitung2Calc( long double value )
{
	short i					=	0;
	
	if( empty2 )
	{
		summeY2				=	0;
		empty2				=	FALSE;
		for( i = 0; i < anzahlWerte; i++ )
		{
			function2[i]	=	value;
			summeY2			=	summeY2 + value;
		}
	}
	else
	{
		summeY2				=	summeY2 - function2[0];
		for( i = 1; i < anzahlWerte; i++ )
		{
			function2[i-1]	=	function2[i];
		}
		function2[anzahlWerte-1]		=	value;
		summeY2				=	summeY2 + value;
	}
	summeXY2				=	0;
	for( i = 0; i < anzahlWerte; i++ )
	{
		summeXY2			=	summeXY2 + ( function2[i] * i );
	}
	for( i = 1; i < anzahlWerte; i++ )
	{
		ableitung2[i-1]		=	ableitung2[i];
	}
	summeXY2				=	summeXY2 * anzahlWerte;
	summeXY2				-=	( summeX2 * summeY2 );
	ableitung2[anzahlWerte-1]			=	summeXY2;
}

/** Ableitung ***************************************/
void AbleitungAb0( int type )
{
	int i					=	0;
	overhead				=	anzahlWerte + anzahlWerte / 2;
	
	empty1					=	TRUE;
	empty2					=	TRUE;
	
	peakStart				=	0;
	peakMitte				=	0;
	peakEnde				=	0;
	peakStartC				=	0;
	peakMitteC				=	0;
	peakEndeC				=	0;
	
	bereich					=	0;
	
	for( i = 0; i < 359+overhead; i++ )
	{
		if( i >= 359 )
		{
			Ableitung1Calc( messdaten[359] );
			Ableitung2Calc( ableitung1[0] );
		}
		else
		{
			Ableitung1Calc( messdaten[i] );
			Ableitung2Calc( ableitung1[0] );
		}
		if( i >= overhead )
		{
			switch( bereich )
			{
				case 0:
				{
					if( ( ableitung1[0] >= 0 ) && ( ableitung2[anzahlWerte-1] >= 0 )
						&& ( ableitung1[0] > schwelle1 ) )
					{
						peakStart			=	i - overhead;
						bereichCount		=	0;
						bereich				=	1;
					}
					break;
				}	
				case 1:
				{
					if( ( ableitung1[0] >= 0 ) && ( ableitung2[anzahlWerte-1] >= 0 )
						&& ( ableitung1[0] > schwelle1 ) )
					{
						bereichCount++;
					}	
					else
					{
						bereich				=	0;
						break;
					}	
					if( bereichCount > 10 )
					{
						bereichCount = 0; //qu
						bereich	= 2;
					}
					break;	
				}
				case 2:
				{
					if( ( ableitung1[0] >= 0 ) && ( ableitung2[anzahlWerte-1] < 0 ) )
					{
						bereich				=	3;
					}	
					else if( ableitung1[0] < 0 )
					{
						bereichCount++;  //qu
						if(bereichCount>5) { //qu
							bereich				=	0;
							break;
						}
					}
					break;	
				}
				case 3:
				{
					if( ( ableitung1[0] < 0 ) && ( ableitung2[anzahlWerte-1] < 0 ) )
					{
						peakMitte			=	i-overhead;
						bereich				=	4;
					}	
					if( ( ableitung1[0] >= 0 ) && ( ableitung2[anzahlWerte-1] >= 0 )
						&& ( ableitung1[0] > schwelle1 ) )
					{
						bereich				=	0;
						break;
					}
					break;	
				}	
				case 4:
				{
					if( ableitung2[anzahlWerte-1] >= 0 )
					{
						bereich				=	5;
						bereichCount		=	0;
					}
					if( ( i > 359 ) && ( peakEnde == 0 ) )	
					{	
						if( ( ( retZeit1Min <= peakMitte ) && ( peakMitte <= retZeit1Max ) )
							|| ( ( retZeit2Min <= peakMitte ) && ( peakMitte <= retZeit2Max ) ) )
						{
							peakEnde		=	i-overhead;
							GraphDrawLine( peakStart );
							GraphDrawLine( peakEnde );
							GraphDrawLineIntegral( peakStart, peakEnde, peakMitte );	
							if( type == 0 )
							{
								Integrate( 0 );
								peakStartC		=	peakStart;
								peakMitteC		=	peakMitte;
								peakEndeC		=	peakEnde;
								i				=	1000;
							}	
							else if( type == 1 )
							{
								Integrate( 1 );
								i				=	1000;
							}	
							bereich			=	0;
						}
						else
						{
							GraphShowPeak( peakStart, peakMitte );
							bereich			=	0;
							if( uebertragungsart == TRUE )
							{
								SystemSetMA1( 830 );
							}
							else
							{
								SystemSetMA1( 0 );
							}	
							messergebnis	=	0;
							DisplayShowMessergebnis();
						}		
					}
					break;
				}
				case 5:
				{
					if( ableitung1[0] > schwelle2 )
					{
						bereichCount++;
						if( bereichCount > 9 )
						{	
							if( ( ( retZeit1Min <= peakMitte ) && ( peakMitte <= retZeit1Max ) )
								|| ( ( retZeit2Min <= peakMitte ) && ( peakMitte <= retZeit2Max ) ) )
							{
								peakEnde		=	i-overhead-bereichCount;
								GraphDrawLine( peakStart );
								GraphDrawLine( peakEnde );
								GraphDrawLineIntegral( peakStart, peakEnde, peakMitte );
								if( type == 0 )
								{
									Integrate( 0 );
									peakStartC		=	peakStart;
									peakMitteC		=	peakMitte;
									peakEndeC		=	peakEnde;
									i				=	1000;
								}	
								else if( type == 1 )
								{
									Integrate( 1 );
									i				=	1000;
								}		
								bereich			=	0;
							}
							else
							{
								GraphShowPeak( peakStart, peakMitte );
								bereich			=	0;
								if( uebertragungsart == TRUE )
								{
									SystemSetMA1( 830 );
								}
								else
								{
									SystemSetMA1( 0 );
								}	
								messergebnis	=	0;
								DisplayShowMessergebnis();
							}		
						}
					}	
					else
					{
						bereichCount		=	0;
						if( i > 359 )
						{	
							if( ( ( retZeit1Min <= peakMitte ) && ( peakMitte <= retZeit1Max ) )
								|| ( ( retZeit2Min <= peakMitte ) && ( peakMitte <= retZeit2Max ) ) )
							{
								peakEnde		=	i-overhead;
								GraphDrawLine( peakStart );
								GraphDrawLine( peakEnde );
								GraphDrawLineIntegral( peakStart, peakEnde, peakMitte );
								if( type == 0 )
								{
									Integrate( 0 );
									peakStartC		=	peakStart;
									peakMitteC		=	peakMitte;
									peakEndeC		=	peakEnde;
									i				=	1000;
								}	
								else if( type == 1 )
								{
									Integrate( 1 );
									i				=	1000;
								}	
								bereich			=	0;
							}
							else
							{
								GraphShowPeak( peakStart, peakMitte );
								bereich			=	0;
								if( uebertragungsart == TRUE )
								{
									SystemSetMA1( 830 );
								}
								else
								{
									SystemSetMA1( 0 );
								}	
								messergebnis	=	0;
								DisplayShowMessergebnis();
							}		
						}		
					}
					break;
				default:
					break;	
				}							
			}	
		}			
	}	
}			

/** Ableitung ***************************************/
void AbleitungAb360( void )
{
	int i					=	360;
	overhead				=	anzahlWerte + anzahlWerte / 2;
	
	empty1					=	TRUE;
	empty2					=	TRUE;
	
	peakStart				=	0;
	peakMitte				=	0;
	peakEnde				=	0;
	
	bereich					=	0;
	
	for( i = 360; i < 719+overhead; i++ )
	{
		if( i >= 719 )
		{
			Ableitung1Calc( messdaten[719] );
			Ableitung2Calc( ableitung1[0] );
		}
		else
		{
			Ableitung1Calc( messdaten[i] );
			Ableitung2Calc( ableitung1[0] );
		}
		if( i >= overhead )
		{
			switch( bereich )
			{
				case 0:
				{
					if( ( ableitung1[0] >= 0 ) && ( ableitung2[anzahlWerte-1] >= 0 )
						&& ( ableitung1[0] > schwelle1 ) )
					{
						peakStart			=	i - overhead;
						bereichCount		=	0;
						bereich				=	1;
					}
					break;
				}	
				case 1:
				{
					if( ( ableitung1[0] >= 0 ) && ( ableitung2[anzahlWerte-1] >= 0 )
						&& ( ableitung1[0] > schwelle1 ) )
					{
						bereichCount++;
					}	
					else
					{
						bereich				=	0;
						break;
					}	
					if( bereichCount > 10 )
					{
						bereich				=	2;
					}
					break;	
				}
				case 2:
				{
					if( ( ableitung1[0] >= 0 ) && ( ableitung2[anzahlWerte-1] < 0 ) )
					{
						bereich				=	3;
					}	
					else if( ableitung1[0] < 0 )
					{
						bereich				=	0;
						break;
					}
					break;	
				}
				case 3:
				{
					if( ( ableitung1[0] < 0 ) && ( ableitung2[anzahlWerte-1] < 0 ) )
					{
						peakMitte			=	i-overhead;
						bereich				=	4;
					}	
					if( ( ableitung1[0] >= 0 ) && ( ableitung2[anzahlWerte-1] >= 0 )
						&& ( ableitung1[0] > schwelle1 ) )
					{
						bereich				=	0;
						break;
					}
					break;	
				}	
				case 4:
				{
					if( ableitung2[anzahlWerte-1] >= 0 )
					{
						bereich				=	5;
						bereichCount		=	0;
					}	
					if( i > 719 )
					{	
						if( ( ( retZeit1Min <= peakMitte ) && ( peakMitte <= retZeit1Max ) )
							|| ( ( retZeit2Min <= peakMitte ) && ( peakMitte <= retZeit2Max ) ) )
						{
							peakEnde		=	i-overhead;
							GraphDrawLine( peakStart );
							GraphDrawLine( peakEnde );
							GraphDrawLineIntegral( peakStart, peakEnde, peakMitte );

							//Falls die Kalibrationskurve nicht gefunden wurde, muss auch der zweite Peak nicht integriert werden
							if(calFlaeche<calFlaecheMin)return;
							Integrate( 1 );
							i				=	1000;

							bereich			=	0;
						}
						else
						{
							GraphShowPeak( peakStart, peakMitte );
	
							bereich			=	0;
							if( uebertragungsart == TRUE )
							{
								SystemSetMA1( 830 );
							}
							else
							{
								SystemSetMA1( 0 );
							}	
							messergebnis	=	0;
							DisplayShowMessergebnis();
						}		
					}
					break;
				}
				case 5:
				{
					if( ableitung1[0] > schwelle2 )
					{
						bereichCount++;
						if( bereichCount > 9 )
						{	
							if( ( ( retZeit1Min <= peakMitte ) && ( peakMitte <= retZeit1Max ) )
								|| ( ( retZeit2Min <= peakMitte ) && ( peakMitte <= retZeit2Max ) ) )
							{
								peakEnde		=	i-overhead-bereichCount;
								GraphDrawLine( peakStart );
								GraphDrawLine( peakEnde );
								GraphDrawLineIntegral( peakStart, peakEnde, peakMitte );

								//Falls die Kalibrationskurve nicht gefunden wurde, muss auch der zweite Peak nicht integriert werden
								if(calFlaeche<calFlaecheMin)return;
								Integrate( 1 );
								i				=	1000;

								bereich			=	0;
							}
							else
							{
								GraphShowPeak( peakStart, peakMitte );
	
								bereich			=	0;
								if( uebertragungsart == TRUE )
								{
									SystemSetMA1( 830 );
								}
								else
								{
									SystemSetMA1( 0 );
								}	
								messergebnis	=	0;
								DisplayShowMessergebnis();
							}		
						}
					}	
					else
					{
						bereichCount		=	0;
						if( i > 719 )
						{	
							if( ( ( retZeit1Min <= peakMitte ) && ( peakMitte <= retZeit1Max ) )
								|| ( ( retZeit2Min <= peakMitte ) && ( peakMitte <= retZeit2Max ) ) )
							{
								peakEnde		=	i-overhead;
								GraphDrawLine( peakStart );
								GraphDrawLine( peakEnde );
								GraphDrawLineIntegral( peakStart, peakEnde, peakMitte );

								//Falls die Kalibrationskurve nicht gefunden wurde, muss auch der zweite Peak nicht integriert werden
								if(calFlaeche<calFlaecheMin)return;
								Integrate( 1 );
								i				=	1000;

								bereich			=	0;
							}
							else
							{
								GraphShowPeak( peakStart, peakMitte );
	
								bereich			=	0;
								if( uebertragungsart == TRUE )
								{
									SystemSetMA1( 830 );
								}
								else
								{
									SystemSetMA1( 0 );
								}	
								messergebnis	=	0;
								DisplayShowMessergebnis();
							}		
						}		
					}
					break;
				default:
					break;	
				}							
			}	
		}			
	}	
}	

void Integrate( int type )
{
	int i				=	0;
	long double trapez	=	0;
	
	long double temp	=	0;
	
	integral			=	0;
	
	MemoryGetSkalierung();
	MemoryGetEichgas();
	MemoryGetUebertragungsart();
	
	for( i = peakStart; i <= peakEnde; i++ )
	{
		integral		=	integral + messdaten[i];
	}	

	trapez				=	messdaten[peakStart] + messdaten[peakEnde];
	trapez				=	trapez / 2;
	trapez				=	trapez * ( peakEnde - peakStart );
	
	
	integral			=	integral - trapez;
	
	if( syringe ) {
		DisplayShowArea();
	}	
	
	if( type == 0 )
	{
		calFlaeche		=	integral;
		integral		=	0;
		if( calFlaeche < calFlaecheMin )
		{
			mA2			=	0;
//			SystemSetMA2( mA2 );
			malfunction	=	TRUE;
			if( calFlaeche > 0 )
				{ errorCode	=	2; }
			else
				{ errorCode	=	3; }
		}	
		else
		{
			temp		=	( double ) calFlaeche / ( double ) 500000;
			if( uebertragungsart )
			{
				temp	=	temp * 16 + 4;
				mA2		=	floor( ( temp * 4095 ) / 20 );
				SystemSetMA2( mA2 );
			}
			else
			{
				temp	=	temp * 20;
				mA2		=	floor( ( temp * 4095 ) / 20 );
//				SystemSetMA2( mA2 );
			}		
		}	
	}
	else if( type == 1 )
	{
		if( malfunction )
		{
			return;
		}	
		
		flaeche_nr		=	0;
		
		if( calFlaeche < calFlaecheMin )
		{
			messergebnis	=	0;
			if( uebertragungsart )
			{
				mA1			=	830;
				SystemSetMA1( mA1 );
			}
			else
			{
				mA1			=	0;
				SystemSetMA1( mA1 );
			}		
		}
		else
		{
			messergebnis	=	floor( ( eichgasmenge ) * integral / calFlaeche );
			if ( syringe ) {
				messergebnis = ( messergebnis * MemoryGetVolumen() ) / 10;
			}	
			temp			=	( double ) messergebnis / ( double ) skalierung;
			temp			=	temp / 10;
			if( uebertragungsart )
			{
				temp		=	temp * 16 + 4;
				mA1			=	floor( ( temp * 4095 ) / 20 );
				SystemSetMA1( mA1 );
			}
			else
			{
				temp		=	temp * 20;
				mA1			=	floor( ( temp * 4095 ) / 20 );
				SystemSetMA1( mA1 );
			}		
		}	
	}		
	
	Nop();
	Nop();
	Nop();
}	


//NEUE ABLEITUNG NOVEMBER 2020 UM DIESEN KLEINEN HUBBEL VORNE WEG ZU BEKOMMEN
void AbleitungInit_neu(void) {

	int i = 0;
	measurementNotOkay = FALSE;

	for( i=0; i<720; i++) {
		ableitung_neu[i] = 0;
	}

	markIndex = 0;
	for( i=0; i<20; i++ ) {
		marks[i].x = 0.0;
		marks[i].y = 0.0;
	}
}

void AbleitungCalc(char type) {

	//BUG: checkPwd wird gefüllt und ist dann nicht mehr FALSE, daher:
	checkPwd = FALSE;

	int count = type == 'b' ? 720 : 360;
	int bereichCnt = count == 360 ? 1 : 0;

	int n = 15;
	int schwelle = 50;
	int i = n;
	int j;

	int lower = 20;
	int upper = 340;

	enum _vorzeichen {Zero, Positiv, Negativ}vorzeichen;

	int vzCnt;
	int markIndex;

	double i1;
	double i2;
	double i3;

	long double temp = 0;

	marksMax[0] = 0;
	marksMax[1] = 0;

	integralBereich[0] = 0.0;
	integralBereich[1] = 0.0;

	MemoryGetSkalierung();
	MemoryGetEichgas();
	MemoryGetUebertragungsart();
	
	do {
		for( i=n; i<count-1-n; i++) {
			ableitung_neu[i] = Regression(i, n);
		}

		if( ableitung_neu[upper]<(-schwelle) )
			vorzeichen = Negativ;
		else if( ableitung_neu[upper]>schwelle )
			vorzeichen = Positiv;
		else
			vorzeichen = Zero;

		vzCnt = 0;
		markIndex = 0;
		indexMax = 0;

		for( i=lower; i<=upper; i++ ) {
			if((vorzeichen==Negativ || vorzeichen==Zero) && ableitung_neu[i]>schwelle) {
				vorzeichen = Positiv;
				marks[markIndex].x = i;
				marks[markIndex].y = messdaten[i];
				marks[markIndex].vz = vorzeichen;
				markIndex++;
				vzCnt++;
			}
			else if((vorzeichen==Positiv || vorzeichen==Zero) && ableitung_neu[i]<(-1*schwelle)) {
				vorzeichen = Negativ;
				marks[markIndex].x = i;
				marks[markIndex].y = messdaten[i];
				marks[markIndex].vz = vorzeichen;
				markIndex++;
				vzCnt++;
			}
			else if(abs(ableitung_neu[i])<schwelle/3) {
				if(vorzeichen==Positiv || vorzeichen==Negativ) {
					vorzeichen = Zero;
					marks[markIndex].x = i;
					marks[markIndex].y = messdaten[i];
					marks[markIndex].vz = vorzeichen;
					markIndex++;
					vzCnt++;
				}
			vorzeichen = Zero;
			}
		}

		if(vzCnt>3) {
			for( i=0; i<vzCnt-2; i++) {
				if((marks[i+1].x-marks[i].x)<15) {
					marks[i].x = (marks[i].x+marks[i+1].x)/2;
					marks[i].y = messdaten[(int)(marks[i].x)];

					if(ableitung_neu[(int)marks[i].x]>schwelle) {
						marks[i].vz = Positiv;
					}
					else if(ableitung_neu[(int)marks[i].x]<(-schwelle)) {
						marks[i].vz = Negativ;
					}
					else {
						marks[i].vz = Zero;
					}

					j = i+1;
					vzCnt--;
					while(j<=vzCnt-1) {
						marks[j] = marks[j+1];
						j++;
					}
					i--;
				}
			}

			for( i=0; i<vzCnt; i++ ){
				GraphDrawLine(marks[i].x);
				if(marks[i].y > marks[indexMax].y) {
					indexMax = i;
				}
			}
			marksMax[bereichCnt] = indexMax;


//Überprüfung: Gibt es mindestens drei Marks und sind die Werte dieser Art: y1 < y2 > y3?
if((vzCnt < 3) 											//Keine drei Markierungen gefunden
	|| (indexMax == 0 || indexMax == vzCnt) 			//Maximum des Peak am Anfang oder Ende
	|| (messdaten[(int)marks[indexMax].x] < messdaten[(int)marks[indexMax-1].x]) 	//Peakmitte falsch
	|| (messdaten[(int)marks[indexMax].x] < messdaten[(int)marks[indexMax+1].x])){	//Peakmitte falsch
	measurementNotOkay = TRUE;
	return;
} else {
	measurementNotOkay = FALSE;
}

			p1 = marks[0].x;
			p2 = marks[indexMax-1].x;
			p3 = marks[vzCnt].x;

			m = (messdaten[(int)(p3)]-messdaten[(int)(p1)]) / (p3-p1);
			p2y = (p2-p1)*m+messdaten[(int)(p1)];

//Testweise die Gerade unter dem Integral zeichnen
GraphDrawLineIntegral_neu((int)marks[indexMax-1].x, (int)marks[indexMax+1].x, (int)marks[indexMax].x, p2y);

			i1 = 0;
			i2 = 0;
			i3 = 0;

			for( i=(int)p1; i<=((int)p2)-1; i++ ) {
				i1 = i1+messdaten[i];
			}
			for( i=(int)p2; i<=(int)p3; i++ ) {
				i2 = i2+messdaten[i];
			}
			i3 = i1+i2;
			i3 = i3-((messdaten[(int)p1]+messdaten[(int)p3])/2*(p3-p1));
			i1 = i1-((messdaten[(int)p1]+p2y)/2*(p2-p1));
			i2 = i2-((p2y+messdaten[(int)p3])/2*(p3-p2));
			i2 = i2+i3*(i2/(i2+i1));

			integralBereich[bereichCnt] = i3;
		}

/// START /// BIS ENDE AUS ALTER ABLEITUNG KOPIERT
/// MESSERGEBNIS BESTIMMEN, AN MA ANLEGEN UND 

		if( syringe ) {
			DisplayShowArea();
		}	
	
		if( type == 'c' || (type == 'b' && bereichCnt == 1 ))
		{
			integral 		= 	i3;		// WERT AUS NEUER BERECHNUNG
			calFlaeche		=	integral;
			integral		=	0;
			if( calFlaeche < calFlaecheMin )
			{
				mA2			=	0;
//				SystemSetMA2( mA2 );
				malfunction	=	TRUE;
				if( calFlaeche > 0 )
					{ errorCode	=	2; }
				else
					{ errorCode	=	3; }
			}	
			else
			{
				temp		=	( double ) calFlaeche / ( double ) 500000;
				if( uebertragungsart )
				{
					temp	=	temp * 16 + 4;
					mA2		=	floor( ( temp * 4095 ) / 20 );
//					SystemSetMA2( mA2 );
				}
				else
				{
					temp	=	temp * 20;
					mA2		=	floor( ( temp * 4095 ) / 20 );
//					SystemSetMA2( mA2 );
				}		
			}	
		}
		else if( type == 'm' )
		{
			if( malfunction )
			{
				return;
			}	
		
			flaeche_nr		=	0;
		
			if( calFlaeche < calFlaecheMin )
			{
				messergebnis	=	0;
				if( uebertragungsart )
				{
					mA1			=	830;
					SystemSetMA1( mA1 );
				}
				else
				{
					mA1			=	0;
					SystemSetMA1( mA1 );
				}		
			}
			else
			{
				//messergebnis	=	floor( ( eichgasmenge ) * integral / calFlaeche );
				messergebnis	=	floor( ( eichgasmenge ) * i3 / calFlaeche );
				if ( syringe ) {
					messergebnis = ( messergebnis * MemoryGetVolumen() ) / 10;
				}	
				temp			=	( double ) messergebnis / ( double ) skalierung;
				temp			=	temp / 10;
				if( uebertragungsart )
				{
					temp		=	temp * 16 + 4;
					mA1			=	floor( ( temp * 4095 ) / 20 );
					SystemSetMA1( mA1 );
				}
				else
				{
					temp		=	temp * 20;
					mA1			=	floor( ( temp * 4095 ) / 20 );
					SystemSetMA1( mA1 );
				}		
			}	
		}		

		/// ENDE ALTER CODE
		/// ENDE ALTER CODE
		/// ENDE ALTER CODE

		bereichCnt++;
		upper = upper+360;
		lower = lower+360;

		markIndex = 0;
		for( i=0; i<20; i++ ) {
			marks[i].x = 0.0;
			marks[i].y = 0.0;
		}
	} while(bereichCnt<2);
}

double Regression(int index, int n) {

	double k_div_60 = 0.0;

	double a1 = 0.0;
	double sxy = 0.0;
	double sx = 0.0;
	double sy = 0.0;
	double sx2 = 0.0;
	int j, k, l;

	j = index;
	l = n/2;
	if(index < l) {
		j = 0;
	}
	for(k=index-l; k<=j+l; k++) {
		k_div_60 = k/60.0;

		sx = sx + k_div_60;
		sy = sy + messdaten[k];
		sxy = sxy + k_div_60 * messdaten[k];
		sx2 = sx2 + k_div_60 * k_div_60;
	}
	a1 = (n * sxy - sy * sx) / (n * sx2 - sx * sx);
	return a1;
}

void Integrate_neu(int type){
	int i				=	0;
	long double trapez	=	0;
	
	long double temp	=	0;
	
	integral			=	0;
	
	MemoryGetSkalierung();
	MemoryGetEichgas();
	MemoryGetUebertragungsart();
	
	for( i = peakStart; i <= peakEnde; i++ )
	{
		integral		=	integral + messdaten[i];
	}	

	trapez				=	messdaten[peakStart] + messdaten[peakEnde];
	trapez				=	trapez / 2;
	trapez				=	trapez * ( peakEnde - peakStart );	
	
	integral			=	integral - trapez;
	
	if( syringe ) {
		DisplayShowArea();
	}	
	
	if( type == 0 )
	{
		calFlaeche		=	integral;
		integral		=	0;
		if( calFlaeche < calFlaecheMin )
		{
			mA2			=	0;
//			SystemSetMA2( mA2 );
			malfunction	=	TRUE;
			if( calFlaeche > 0 )
				{ errorCode	=	2; }
			else
				{ errorCode	=	3; }
		}	
		else
		{
			temp		=	( double ) calFlaeche / ( double ) 500000;
			if( uebertragungsart )
			{
				temp	=	temp * 16 + 4;
				mA2		=	floor( ( temp * 4095 ) / 20 );
//				SystemSetMA2( mA2 );
			}
			else
			{
				temp	=	temp * 20;
				mA2		=	floor( ( temp * 4095 ) / 20 );
//				SystemSetMA2( mA2 );
			}		
		}	
	}
	else if( type == 1 )
	{
		if( malfunction )
		{
			return;
		}	
		
		flaeche_nr		=	0;
		
		if( calFlaeche < calFlaecheMin )
		{
			messergebnis	=	0;
			if( uebertragungsart )
			{
				mA1			=	830;
				SystemSetMA1( mA1 );
			}
			else
			{
				mA1			=	0;
				SystemSetMA1( mA1 );
			}		
		}
		else
		{
			messergebnis	=	floor( ( eichgasmenge ) * integral / calFlaeche );
			if ( syringe ) {
				messergebnis = ( messergebnis * MemoryGetVolumen() ) / 10;
			}	
			temp			=	( double ) messergebnis / ( double ) skalierung;
			temp			=	temp / 10;
			if( uebertragungsart )
			{
				temp		=	temp * 16 + 4;
				mA1			=	floor( ( temp * 4095 ) / 20 );
				SystemSetMA1( mA1 );
			}
			else
			{
				temp		=	temp * 20;
				mA1			=	floor( ( temp * 4095 ) / 20 );
				SystemSetMA1( mA1 );
			}		
		}	
	}		
	
	Nop();
	Nop();
	Nop();
}	
