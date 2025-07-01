#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
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

#define DEV_OK				0x01
#define DEV_CALGAS			0x02
#define DEV_INJEKTION		0x04
#define DEV_SPLIT			0x08
#define DEV_STARTED			0x10
#define DEV_ACTIVE			0x20
#define DEV_ENDED			0x40
#define DEV_RESET			0x80

void Delay( void );
void DisplayShowErrorCopy( void );
void DisplayWait( void );
void DisplayTest( void );
char DisplayBufferInfo( void );
void DisplayClearDataCursor( void );
void DisplayGetBuffer( void );
void DisplayLockSystem( void );
void DisplaySendNBytes( char* data, unsigned short length );
void DisplayRestartDisplay( void );
void DisplayRefreshMain( void );
void DisplayRequest( char c2, char c3, char c4 );
void DisplaySaveData( void );
void DisplaySaveDataComplete( void );
void DisplaySaveDataInsert( void );
void DisplaySaveDataPWFalsch( void );
void DisplaySaveDataReturn( void );
void DisplaySendBargraphValue( unsigned short s );
void DisplaySendBargraphTotalNumber( unsigned int i );
void DisplaySendBargraphRecentNumber( unsigned int i );
void DisplaySetLanguage( char lang );
void DisplayShowMessergebnis( void );
void DisplayGetErrorLog( unsigned long int logCount );
void DisplayShowErrorLog( char* filename, int no );
void DisplayShowNoErrorLogs( void );
void DisplayShowSignal( void );
void DisplayShowAkku( void );
void DisplayShowFluss( short i );
void DisplayShowMessort( void );
void DisplayShowMessortCursor( void );
void DisplayShowEichgas( int i );
void DisplayShowMenge( void );
void DisplayShowVolumen( void );
void DisplayShowDatum( short i );
void DisplayShowOK( int i );
void DisplayShowNoData( void );
void DisplayShowDataEnd( unsigned int position );
void DisplayShowDataDisableNextPage();
void DisplayShowDataEnableNextPage();
void DisplayShowDataDatum( void );
void DisplayShowDataResult( char* data, unsigned int position );
void DisplayShowDayValueFirst( unsigned char x1Pos, unsigned char x2Pos, unsigned char yPos );
void DisplayShowDayValue( unsigned char x1Pos, unsigned char x2Pos, unsigned char yPos );
void DisplayShowWert( int i );
void DisplayShowUhrzeit( short i, short j );
void DisplayShowTemperatur( short i );
void DisplayShowDruck( short i );
void DisplayShowEmpfindlichkeit( void );
void DisplayShowMesszyklus( void );
void DisplayShowMain( void );
void DisplayShowDeveloperMode( void );
void DisplayShowEichzyklus( void );
void DisplayShowScaledGrid( void );
void DisplayShowSpuelzeit( void );
void DisplayShowUhrBlockiert( void );
void DisplayShowKorrekturfaktor( void );
void DisplayShowBatteryLow( void );
void DisplayShowUebertragungsart( void );
void DisplayShowSplitzeit( void );
void DisplayShowPeakschranke( void );
void DisplayShowCalflaeche( void );
void DisplayShowAutomessung( void );
void DisplayShowSkalierung( void );
void DisplayShowSDOK( void );
void DisplayShowNetzbetrieb( void );
void DisplayShowMemInitDone( void );
void DisplayShowSpeicherung( void );
void DisplayShowStartzeit( void );
void DisplayShowArea( void );
void DisplayShowReset( void );
void DisplayShowDateCursor( void );
void DisplayShowDataCursor( unsigned int position );
void DisplayShowTimeCursor( void );
void DisplayShowPasswort( void );
void DisplayShowResetError( int i );
void DisplayShowSerialNumber( int sn, char* SWVersion );
void DisplayShowPasswortCursor( void );
void DisplayShowSystemStatus( char c );
void DisplayShowSekunden( void );
void ClockGetDatestring( char *s );
void ClockInit( void );
void ClockInitDateTime( void );
void ClockWriteEnable( void );
void ClockSetClock( void );
void ClockReadMinutes( void );
void ClockReadHours( void );
void ClockReadDay( void );
void ClockReadMonth( void );
void ClockReadYear( void );
void MemoryInit( void );
void MemoryInit2( void );
void MemoryInit3( void );
void MemoryGetBetriebsparameter( void );
void MemoryGetFernuebertragung( void );
void MemoryGetMessort( void );
void MemoryGetMenge( void );
unsigned short MemoryGetVolumen( void );
void MemoryGetTemperatur( void );
void MemoryGetSplitzeit( void );
void MemoryGetFluss( void );
void MemoryGetDruck( void );
void MemoryGetEmpfindlichkeit( void );
void MemoryGetKalReportNr( void );
void MemoryGetMesszyklus( void );
void MemoryGetEichzyklus( void );
void MemoryGetSpuelzeit( void );
void MemoryGetKorrekturfaktor( void );
void MemoryGetAutomessung( void );
void MemoryGetTaeglicheMessung( void );
void MemoryGetSkalierung( void );
void MemoryGetEichgas( void );
void MemoryGetUebertragungsart( void );
void MemoryGetMeasCount( void );
void MemoryGetErrorCount( void );
void MemorySetMeasCount( void );
void MemorySetErrorCount( void );
void MemorySetMessort( void );
void MemorySetEichgasmenge( void );
void MemorySetMenge( void );
void MemorySetVolumen( void );
void MemorySetTemperatur( void );
void MemorySetDruck( void );
void MemorySetFluss( void );
void MemorySetKalReportNr( void );
void MemorySetSplitzeit( void );
void MemorySetEmpfindlichkeit( void );
void MemorySetMesszyklus( void );
void MemorySetEichzyklus( void );
void MemorySetSpuelzeit( void );
void MemorySetKorrekturfaktor( void );
void MemorySetTaeglicheMessung( void );
void MemorySetUebertragungsart( void );
void MemorySetAutomessung( void );
void MemorySetSkalierung( void );
void GraphSetColorBlue( void );
void GraphInitGraph( void );
void GraphDeleteGraph( void );
void GraphDrawLine( int i );
void GraphDrawLineIntegral( int start, int ende, int mitte );
void GraphDrawLineIntegral_neu( int start, int ende, int mitte, double p2y );
void GraphShowPeak( int start, int mitte );
void GraphNewValue( unsigned short val );
void GraphDrawGraph( void );
void GraphRefresh( void );
void SystemCalcNextRun( char c );
void SystemCalcPressure( int peakMitte );
void SystemInit( void );
void SystemCancelMeasurement( void );
void SystemGetRecentValue( void );
void SystemGetTimeString( void );
void SystemGetAllValues( void );
void SystemResetAnalytic( void );
void SystemSetDetektor( void );
void SystemSetSaeule( void );
void SystemSetFluss( void );
void SystemCheckFluss( void );
void SystemSetVordruck( void );
void SystemSetSplitzeit( void );
void SystemSetEmpfindlichkeit( void );
void SystemSetMA1( unsigned int mA );
void SystemSetMA2( unsigned int mA );
void SystemSetOK( unsigned short k );
void SystemStartCalibration( void );
void SystemStartMeasurement( void );
void SystemStartSyringe( void );
void SystemClearActiveStarted( void );
void SystemResetMesspunkte( void );
void SystemStartCalmeasurement( void );
void SystemStartSpuelen( void );
void SystemDetektorIstwert( void );
void SystemSaeuleIstwert( void );
void SystemPause( void );
void SystemFlussIstwert( void );
void SystemVordruckIstwert( void );
void SystemDetektorVorgabe( void );
void SystemSaeuleVorgabe( void );
void SystemFlussVorgabe( void );
void SystemVordruckVorgabe( void );
void SystemEmpfindlichkeitVorgabe( void );
void SystemMA1( void );
void SystemMA2( void );
int SystemOK( void );
int SystemMessCalgas( void );
int SystemInjektion( void );
int SystemSpuelung( void );
void SystemStatus( void );
void SystemClearEndFlag( void );
void SystemClearResetFlag( void );


short int ReadAkku( void );

unsigned char *litoa( unsigned long int i, unsigned char *s);

#endif

/** EOF display.h ***********************************************************/
