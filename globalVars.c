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
#pragma message("calFlaecheMin visible")



/** VARIABLES *******************************************************/
BOOL startup;
BOOL checkMasterPwd;
BOOL checkPwd;
BOOL volatile deviceActive;
BOOL volatile deviceStarted;
BOOL fileCopiedRefresh;
BOOL checkValue;
BOOL checkPoll;
BOOL initResults;
BOOL drawgraph;
BOOL meas6min;
BOOL startAutoMeas;
BOOL autoMeasurement;
BOOL calmeas;
BOOL cal;
BOOL meas;
BOOL saveData;
BOOL uebertragungsart;
BOOL battery_low;
BOOL battery_recharged;
BOOL malfunction;
BOOL lockSystem;
BOOL copyAll;
BOOL syringe;
BOOL IsMASet;
//NEUE BERECHNUNG AB NOVEMBER 2020
BOOL measurementNotOkay;

unsigned long int meascount;
unsigned long int errorcount;
unsigned long int calFlaeche;
unsigned long int calFlaecheMin = 50000;

long int integral				=	0;
unsigned int messergebnis;
unsigned int sekunden;
unsigned int displayWait;
unsigned int checkFlussCount	=	0;
unsigned short malfunctionCount	=	0;

char current_date[25];
unsigned char readDirectory[21];

unsigned char buffer[6];
unsigned short messpunkte[240];
char messort[9]					=	"00000000";
unsigned short eichgasmenge;
unsigned short empfindlichkeit_vorgabe	=	0;
unsigned int signal;
unsigned int baselineConstant	=	0;
unsigned short skalierung;
unsigned short volatile temperatur;
unsigned short int saeule_istwert;
unsigned int saeule_vorgabe		=	0;
int flaeche_nr					=	0;
int counter						=	0;
unsigned short vordruck_istwert	=	0;
unsigned short int fluss_istwert	=	0;
unsigned short empfindlichkeit		=	1;

unsigned short volatile system_status;
unsigned short 		akku		=	50;
int messdatum;
int messdaten[720];
unsigned char recentValue[8];
unsigned char previousValue[8];
int *messdatenptr;
int *messdatenende;
char measurementType;
unsigned int taeglicheMessung;

char uhrzeitLetzterLauf[6];
char uhrzeitNaechsteMessung[6];
char uhrzeitNaechsteKalib[6];
char uhrzeitAktuell[6];
unsigned int calcMinRec;
unsigned int calcHourRec;
unsigned int calcMinNext;
unsigned int calcHourNext;


int defaultTemp = 37;
int defaultFlow = 100;
int defaultSensibility = 5;
int defaultPressure = 150;
int defaultSplittime = 10;



float numberOfFiles;
float percentPerFile;
unsigned char percentComplete;
unsigned int filesNo;
unsigned short filesCopiedRefresh;
unsigned int filesTotal;
unsigned int filesCopied;

unsigned int mA1;
unsigned int mA2;

unsigned int pageNo;
unsigned int dateiNo;
unsigned int recentFileNo;

int peakStart				=	0;
int peakMitte				=	0;
int peakEnde				=	0;
int peakStartC				=	0;
int peakMitteC				=	0;
int peakEndeC				=	0;

unsigned int errorCode		=	0;
unsigned long int kalReportNr	=	0;

char pcString[4];

//aux
int timecountaux = 0;

void InitVariables( void )
{
	startup							=	TRUE;
	checkMasterPwd					=	FALSE;
	checkPwd						=	FALSE;
	deviceActive					=	FALSE;
	deviceStarted					=	FALSE;
	fileCopiedRefresh				=	FALSE;
	checkValue						=	FALSE;
	checkPoll						=	FALSE;
	drawgraph						=	FALSE;
	meas6min						=	FALSE;
	autoMeasurement					=	FALSE;
	calmeas							=	FALSE;
	cal								=	FALSE;
	meas							=	FALSE;
	saveData						=	FALSE;
	uebertragungsart				=	TRUE;
	startAutoMeas					=	FALSE;
	battery_low						=	FALSE;
	battery_recharged				=	FALSE;
	lockSystem						=	FALSE;
	malfunction						=	FALSE;
	copyAll							=	TRUE;
	syringe							=	FALSE;
	
	system_status					=	0;
	calFlaeche						=	0;
	messergebnis					=	0;
	signal							=	0;
	sekunden						=	0;
	
	eichgasmenge					=	50;
	
	taeglicheMessung				=	0;
	measurementType					=	' ';
	messdatum						=	0;
	messdatenptr					=	&messdaten[0];
	messdatenende					=	&messdaten[0];
	calcMinRec						=	0;
	calcHourRec						=	0;
	calcMinNext						=	0;
	calcHourNext					=	0;
	numberOfFiles					=	0;
	percentPerFile					=	0;
	percentComplete					=	0;
	filesNo							=	0;
	filesCopiedRefresh				=	0;
	filesTotal						=	0;
	filesCopied						=	0;
	
	mA1								=	0;
	mA2								=	0;
	
	pageNo							=	0;
	
	pcString[0]						=	' ';
	pcString[1]						=	' ';
	pcString[2]						=	' ';
	pcString[3]						=	' ';

	//aux variable
	timecountaux = 0;
}	

void SystemClearActiveStarted( void )
{
	deviceActive		=	( BOOL ) ( deviceActive & 0x00 );
	deviceStarted		=	( BOOL ) ( deviceStarted & 0x00 );
}	

void SystemResetMesspunkte( void )
{
	int i				=	0;
	
	for( i = 0; i < 720; i++ )
	{
		messdaten[i]	=	0;
	}	
}	

// EOF

