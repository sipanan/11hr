// v1,23
//			errorcode Auswertung ma / Battery low getauscht
// v1.22
//			Änderung der Reihenfolge beim Ende einer Messung:
//			Erst an UART senden, dann auf SD-Karte speichern
// v1.21
//			Senden der Kalreportnummer eingepflegt
// v1.20
// BUGFIX:	Bug bei Spritzenmessungen
//			Bei Spritzenmessungen muss das Ergebnis mit dem Äquivalentvolumen
//			des Gerätes multipliziert werden. Diese Multiplikation fehlte
// v1.19
// BUGFIX:	Bug im Update aus v.1.18.
//			Falls bei einer Kalibration mit anschl. Messung der Kalpeak nicht
//			richtig erkannt wird, löst die Software eine einzelne Kalibration aus.
//			Wurde gegen Kal&Mess ersetzt.
// v1.18
// UPDATE:	Da während der Automessung eine Kalibrierung immer mit einer Messung
//			zusammen fällt, wurde die Funktion "Nur Cal" herus für diesen Fall
//			entfernt.
// v1.17
// UPDATE:	Störung für Batterie kommt nun nicht, sobald der Akku 10V
//			unterschreitet,	sonder bei unter 7.5V.
// v1.16
// BUGFIX:	Falls das System automatisch eine Messung starten will, 
//			während bereits eine Messung läuft, so wird kein neuer Startzeitpunkt
//			ermittelt -> Messungen setzen aus
// v1.15
// BUGFIX: 	Logikfehler beim Wechsel von 0-20mA zu 4-20mA führte zum
//			Auslösen eines Fehlers. Wechsel zu 4-20mA also nicht möglich!
//			BEHOBEN

/** INCLUDES *******************************************************/
#include "definitions.h"
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

/** CONFIG *********************************************************/
_CONFIG1( JTAGEN_OFF & GCP_OFF & GWRP_OFF & FWDTEN_ON & ICS_PGx2 )
_CONFIG2( 0xF7FF & IESO_OFF & FCKSM_CSDCMD & OSCIOFNC_ON & POSCMOD_HS & FNOSC_PRIPLL & PLLDIV_DIV2 & IOL1WAY_ON )

/** VARIABLES ******************************************************/
#pragma udata
const char lang;
const ROM int			SN					=	7749;		//device number

#if defined( DEUTSCH )
	const ROM char			lang				=	DE;
#endif

//#if defined( ENGLISCH )
//	const ROM char			lang				=	UK;
//#endif


const ROM char			SWVersion[6]		=	"v1.23";
const ROM char			pwd_master[9]		=	"00000110";
const ROM char			pwd[9]				=	"00008806";
BOOL 					WriteFlag			= 	FALSE;
BOOL					restart				=	TRUE;
BOOL					disableClock		=	FALSE;
BOOL volatile			btemperatur			=	FALSE;
BOOL volatile			spuelen				=	FALSE;
BOOL volatile			writeStartLog		=	TRUE;
unsigned int			sec10				=	0;
int						pcStatusCount		=	0;
char					status_h			=	0x00;
char					status_l			=	0x00;
char					hTime_h				=	0x00;
char					hTime_l				=	0x00;
char					lTime_h				=	0x00;
char					lTime_l				=	0x00;
char					hByte_h				=	0x00;
char					hByte_l				=	0x00;
char					lByte_h				=	0x00;
char					lByte_l				=	0x00;
static SearchRec		level1;
static SearchRec		level2;
static SearchRec		level3;
static SearchRec		level4;
static SearchRec		csvfile;
static char 			curr_dir[25];
static char				file_data[512];
FSFILE 					*copyFile;
FSFILE 					*logFile;
FS_DISK_PROPERTIES		diskProperties;
char previous_date[25];



#if defined(__C30__) || defined(__C32__)
LUN_FUNCTIONS LUN[MAX_LUN + 1] = 
{
    {
        &MDD_SDSPI_MediaInitialize,
        &MDD_SDSPI_ReadCapacity,
        &MDD_SDSPI_ReadSectorSize,
        &MDD_SDSPI_MediaDetect,
        &MDD_SDSPI_SectorRead,
        &MDD_SDSPI_WriteProtectState,
        &MDD_SDSPI_SectorWrite
    }
};
#endif

/* Standard Response to INQUIRY command stored in ROM 	*/
const ROM InquiryResponse inq_resp = {
	0x00,		// peripheral device is connected, direct access block device
	0x80,           // removable
	0x04,	 	// version = 00=> does not conform to any standard, 4=> SPC-2
	0x02,		// response is in format specified by SPC-2
	0x20,		// n-4 = 36-4=32= 0x20
	0x00,		// sccs etc.
	0x00,		// bque=1 and cmdque=0,indicates simple queueing 00 is obsolete,
			// but as in case of other device, we are just using 00
	0x00,		// 00 obsolete, 0x80 for basic task queueing
	{'M','i','c','r','o','c','h','p'
    },
	// this is the T10 assigned Vendor ID
	{'M','a','s','s',' ','S','t','o','r','a','g','e',' ',' ',' ',' '
    },
	{'0','0','0','1'
    }
};

/** PRIVATE PROTOTYPES *********************************************/
void		CheckTransferStatus( void );
int 		DirectoryLevel1( void );
int 		DirectoryLevel2( void );
int 		DirectoryLevel3( void );
int 		DirectoryLevel4( void );
void 		GraphInitGraph( void );
void 		GraphNewValue( unsigned short val );
void 		GraphRefresh( void );
static void InitializeSystem(void);
void 		ProcessIO(void);
void 		prvSetupTimerInterrupt( void );
short int 	ReadAkku( void );
WORD_VAL 	ReadPOT(void);
void 		SDCopyContent( void );
int 		SDGetFiles( void );
void		SystemEndRun( void );
void 		UARTInit( int baudRate );
void		UART1Init( int baudRate );
void		UARTSendPC( char* datum );
void		UARTSendInfo( char c, int i );
void 		UARTPutChar( char c );
void		UART1PutChar( char c );
char 		UARTGetChar( void );
char		UART1GetChar( void );
void		UART1GetPCString( void );
void 		UARTSendString( char* s, int length, int type );
void 		USBDeviceTasks(void);
void 		writeData( void );
void 		YourHighPriorityISRCode(void);
void 		YourLowPriorityISRCode(void);
char		CharToHex( int byte, unsigned char c );
char		IntToHex( int byte, int i );
char		UnsignedLongIntToHex( int byte, unsigned long int li );
char		LongIntToHex( int byte, long int li );
char		UnsignedIntToHex( int byte, unsigned int i );
char		UnsignedShortToHex( int byte, unsigned short s );
void		SystemLogError( void );
void 		SystemLogTimeOfAutoStart( int type );

#if defined(__C30__)
    #if defined(PROGRAMMABLE_WITH_USB_HID_BOOTLOADER)
//        void __attribute__ ((address(0x1404))) ISRTable(){
//        
//        }
    #endif
#endif

/** DECLARATIONS ***************************************************/
#pragma code

int main(void)
{   
/** DECLARATION OF LOCAL VARIABLES *********************************/
    unsigned int* data;
    unsigned int i;
	unsigned short graph_refresh_count		=	0;
	unsigned short temp_low;
	unsigned short temp_high;

int time = 0;

//ZUM TESTEN
Nop();
RCON = 0x0000;

	
    data = &i;					//set the data pointer for the FSfwrite function
    InitializeSystem();			//initialize system ports  
    for( i = 0; i < 50000; i++ )
    {
	    Nop(); Nop(); Nop(); Nop(); Nop();
	    Nop(); Nop(); Nop(); Nop(); Nop();
	    Nop(); Nop(); Nop(); Nop(); Nop();
	    Nop(); Nop(); Nop(); Nop(); Nop();
	    Nop(); Nop(); Nop(); Nop(); Nop();
	    Nop(); Nop(); Nop(); Nop(); Nop();
	    Nop(); Nop(); Nop(); Nop(); Nop();
	    Nop(); Nop(); Nop(); Nop(); Nop();
	} 
    InitVariables();			//initialize global variables
    initResults = FSInit();		//initialize MDD File System
    mInitPOT();                 //initialize the A/D
    logFile = NULL;			    //The log file is not open so let's make sure the pointer is NULLed out
    USBDeviceInit();			//initialize the USB device
	I2CInit( 70 );				//initialize I²C
	UARTInit( 25 );				//initialize UART2
	UART1Init( 51 );			//initialize UART1
	GraphInitGraph();			//initialize graph
	prvSetupTimerInterrupt();	//initialize Timer 1

#if defined( INIT_CLOCK ) || defined( INIT_SET_CLOCK )
	ClockInit();
	ClockWriteEnable();
	#if defined( INIT_SET_CLOCK )
	ClockInitDateTime();
	#endif
#endif
#if defined( INIT_MEMORY )
	MemoryInit();
	displayWait		=	0;
	while( displayWait < 2 ) { Nop(); }
	MemoryInit2();
	displayWait		=	0;
	while( displayWait < 2 ) { Nop(); }
	MemoryInit3();
	displayWait		=	0;
	while( displayWait < 2 ) { Nop(); }
#endif
	
	SystemInit();
	
	DisplaySetLanguage( lang );
    DisplayRestartDisplay();
	
	ClockGetDatestring( current_date );
	
	GraphSetColorBlue();
	
	MemoryGetUebertragungsart();
	
//STARTUP-SEQUENZ
	temperatur		=	0;
	SystemStartSpuelen();
	DisplayShowSerialNumber( SN, &SWVersion[0] );
	displayWait		=	0;
	while( displayWait < 30 ) { Nop(); }
	SystemStatus();
	while( restart )
	{
		Delay();
		Delay();
		Delay();
		Delay();
		Delay();
		Delay();
		Delay();
		Delay();
		Delay();
		Delay();
		if( checkPoll )
		{
			SystemStatus();
			if( ( system_status & DEV_RESET ) > 0 )
			{
				SystemClearResetFlag();
			}	
			if( temperatur == 0 )
			{
				MemoryGetTemperatur();
				if( ( temperatur < 20 ) || ( temperatur > 60 ) )
				{
					temperatur		=	defaultTemp;
				}	
				saeule_vorgabe	=	temperatur;
				SystemSetSaeule();
				temp_low		=	temperatur - 3;
				temp_high		=	temperatur + 3;	
			}
			else
			{
				if( !btemperatur )
				{
					//TEMPERATUR POLLEN
					SystemSaeuleIstwert();
					SystemSetSaeule();
					DisplayShowTemperatur( 3 );
					DisplayShowTemperatur( 2 );
					if( ( saeule_istwert/ 10 ) >= temp_low && ( saeule_istwert/ 10 ) <= temp_high )
					{
						btemperatur		=	TRUE;
						DisplayShowOK( 0 );
					}	
				}	
			}		
			
			//SPÜLEN
			SystemStatus();
			displayWait		=	0;
			while( displayWait < 2 ) { Nop(); }
			if( ( system_status & DEV_CALGAS ) == 0 )
			{
				spuelen			=	TRUE;
				DisplayShowOK( 1 );
			}
			else
			{
				spuelen			=	FALSE;
			}		
			
			if( btemperatur == TRUE && spuelen == TRUE )
			{
				restart	 		=	FALSE;
				if( uebertragungsart == TRUE )
				{
					mA1			=	830;
					SystemSetMA1( mA1 );
					mA2			=	830;
					SystemSetMA2( mA2 );
				}
				else
				{
					mA1			=	0;
					SystemSetMA1( mA1 );
					mA2			=	0;
					SystemSetMA2( mA2 );
				}
				DisplayShowMain();		
			}
			checkPoll		=	FALSE;	
		}
	}	
	
	checkFlussCount		=	0;
	
	MemoryGetAutomessung();
	
	for( i = 0; i < DELAY2; i++ )
	{
		Nop();Nop();Nop();
	}

	SystemSetOK( 1 );
	
	if( autoMeasurement )
	{
		SystemGetTimeString();
		
		Nop();
		Nop();
		Nop();
		
		displayWait		=	0;
		while( displayWait < 10 ) { Nop(); }

		DisplayRequest( '4', '0', '1' );		// 401 CalMeas 402 Meas 403 Cal
		
		DisplayRequest( '0', '0', '1' );		// Anzeige Parameter Hauptbildschirm
	}	
	
    #if defined(USB_INTERRUPT)
        USBDeviceAttach();
    #endif
    
    checkPoll		=	TRUE;

    while(1)
    {
	    if( checkPoll )
	    {
		    
		    initResults = FSInit();
		    
//CHECK: STÖRUNG
	    if( deviceActive && checkFlussCount == 360 && !syringe )
	    {
		    SystemCheckFluss();
		} 
		    
//POLL: Störung?
		if( malfunction )
		{
			if( malfunctionCount >= 3 )
			{
				SystemSetMA1( 0 );
				SystemSetMA2( 0 );
				SystemSetOK( 0 );
				DisplayShowSystemStatus( 's' );
				SystemLogError();
				lockSystem			=	TRUE;
				saveData			=	FALSE;
				DisplayLockSystem();
				SystemCancelMeasurement();
			//STÖRFALL RETENTIONSZEIT BZW. FLÄCHE BZW: FLUSS
				while( lockSystem )
				{
					if( checkPoll )
					{
						//DEADLOCK BIS ENTER-TASTE GEDRÜCKT WIRD
						if( DisplayBufferInfo() > 0 )
						{ 
							DisplayGetBuffer(); 
						}
					}
				}
SystemLogTimeOfAutoStart( 1 );
			}	
			else
			{
		//STÖRFALL FLÄCHE -> EMPFINDLICHKEIT ERHÖHEN
				if( errorCode == 2 )
				{
					MemoryGetEmpfindlichkeit();
					if( empfindlichkeit < 9 )
					{
						empfindlichkeit_vorgabe		=	empfindlichkeit + 2;
						MemorySetEmpfindlichkeit();
						displayWait		=	0;
						while( displayWait < 2 ) Nop();
					}	
				}
		//STÖRFALL MA OUTPUT -> OUTPUT AUF GRUNDEINSTELLUNG ZURÜCK
//qu errorcode von 4 auf 5 geändert
				else if( errorCode == 5 )
				{
					MemoryGetUebertragungsart();
					if( uebertragungsart )
					{
						mA1		=	830;
						SystemSetMA1( mA1 );
						mA2		=	830;
						SystemSetMA2( mA2 );
					}
					else
					{
						mA1		=	0;
						SystemSetMA1( mA1 );
						mA2		=	0;
						SystemSetMA2( mA2 );
					}
				}
				malfunctionCount++;
				SystemLogError();
				saveData			=	FALSE;
				SystemCancelMeasurement();
				malfunction			=	FALSE;
		//STÖRFALL BATTERIE -> DEADLOCK UND NEU EINSCHALTEN
//qu errorcode von 5 auf 4 geändert
				if( errorCode == 4 )
				{
					SystemSetMA1( 0 );
					SystemSetMA2( 0 );
					SystemSetOK( 0 );
					DisplayShowBatteryLow();
					lockSystem			=	TRUE;
					DisplayLockSystem();
					SystemCancelMeasurement();
					while( lockSystem )
					{
						Nop();
					}
				}	
			}
			displayWait		=	0;
			while( displayWait < 6 )
			{
				Nop();
			}		
		}	    
	    
//POLL: System Status
        SystemStatus();
        
//POLL: System restartet?
        if( ( system_status & DEV_RESET ) > 0 )
        {
	        if( ( system_status & DEV_ENDED ) > 0 )
	       	{
		       SystemClearEndFlag();
		   	} 
		   	if( ( system_status & DEV_ACTIVE ) > 0 )
		   	{
			   SystemClearActiveStarted();
			}	
			SystemClearResetFlag();
			DisplayShowReset();
			
			displayWait			=	0;
			while( displayWait < 12 ) { Nop(); }
	    } 
        
//POLL: File System und SD card ok?
		if( MDD_SDSPI_MediaDetect() == TRUE )
		{
			initResults 	=	TRUE;
		}	
		else
		{
			initResults		=	FALSE;
		}	

//POLL: Automessung aktiv?
        if( autoMeasurement )
        {
	        if( disableClock == FALSE )
	        {
		        SystemGetTimeString();
		    } 
	        
	        if( strcmp( uhrzeitAktuell, uhrzeitNaechsteMessung ) == 0 )
	        {
		        meas				=	TRUE;
		    } 
		    if( strcmp( uhrzeitAktuell, uhrzeitNaechsteKalib ) == 0 )
		    {
			    cal					=	TRUE;
			} 
			if( cal )
			{
				meas				=	FALSE;
				cal					=	FALSE;
				if( ( system_status & DEV_ENDED ) > 0 )
				{
					SystemClearEndFlag();
				}		
				DisplayRequest( '4', '0', '1' );
			}
			else if( meas )
			{
				meas				=	FALSE;
				if( ( system_status & DEV_ENDED ) > 0 )
				{
					SystemClearEndFlag();
				}		
				DisplayRequest( '4', '0', '2' );
			}
	    } 
//POLL: Messung beendet?
		if( ( system_status & DEV_ENDED ) > 0 )
		{
			Nop();
			Nop();
			Nop();

			SystemClearActiveStarted();
				
			Nop();
			Nop();
			Nop();

			messdatenende		=	messdatenptr;
			messdatenende--;
			messdatenptr		=	&messdaten[0];

			if( saveData )
			{
				SystemEndRun();
				saveData		=	FALSE;
			}
			
			if( calFlaeche < calFlaecheMin )
			{
				malfunction		=	TRUE;
				if( calFlaeche > 0 )
					{ errorCode	=	2; }
				else
					{ errorCode	=	3; }
			}
			if( !malfunction )
			{
				malfunctionCount	=	0;
			}	
			
			//überprüfen, ob ein Messwert angelegt wurde, sonst 0 anlegen
			//nur für DEDORIERER, da sonst 0 nicht korrekt angelegt wird
			#if defined DEODOR
				if( IsMASet == FALSE ) {
					if( uebertragungsart )
						SystemSetMA1( 830 );
					else
						SystemSetMA1( 0 );
					IsMASet = TRUE;
					errorcount = 0;
					messergebnis = 0;
        					DisplayShowMessergebnis();
				}	
			#endif
		}	
//POLL: MESSUNG LÄUFT ?
        if( deviceActive && deviceStarted )
        {
	//letzen Datenpunkt holen
	        saveData			=	TRUE;
	        SystemGetRecentValue();
		     
    //an Hand Zeit mit vorhergehendem Datenpunkt vergleichen
        	if( ( recentValue[2] == previousValue[2] ) && ( recentValue[3] == previousValue[3] ) )
        	{
   	//Datenpunkt gleich -> nichts machen
	    	    Nop();
	    	}  
	    	else
	    	{

timecountaux++;

time = 0;
time = time + (recentValue[3] << 8);
time = time + recentValue[2];
UARTSendInfo('T', time);
UARTSendInfo('T', timecountaux);

if(((recentValue[3]<<8)+recentValue[2])-((previousValue[3]<<8)+previousValue[2])!=1)
{Nop();}
else{
	//Datenpunkte unterschiedlich
	//Datenpunkt als letzten "merken"
			    previousValue[0]	=	recentValue[0];
			    previousValue[1]	=	recentValue[1];		//status
			    previousValue[2]	=	recentValue[2];		//time_l
			    previousValue[3]	=	recentValue[3];		//time_h
			    previousValue[4]	=	recentValue[4];
			    previousValue[5]	=	recentValue[5];
			    previousValue[6]	=	recentValue[6];		//signal_l
			    previousValue[7]	=	recentValue[7];		//signal_h
	//neue Nutzdaten in Array übernehmen
				messdatum			=	recentValue[7];
				messdatum			=	messdatum << 8;
				messdatum			=	messdatum + recentValue[6];
				*messdatenptr		=	messdatum;
				messdatenptr++;
				signal				=	messdatum;
				sekunden++;
	//Graph zeichnen
	//bei 6 minütiger messung jeden dritten punkt auslassen
	//bei 12 minütiger messung jeden dritten punkt zeichnen
			    if( 
			    	( ( graph_refresh_count == 0 ) && ( meas6min == TRUE ) )
			    	|| ( ( graph_refresh_count == 1 ) && ( meas6min == TRUE ) )
			    	|| ( ( graph_refresh_count == 2 ) && ( meas6min == FALSE ) )
			      )
			    {
				    GraphNewValue( ( 200 - ( unsigned short ) ( ( messdatum ) * 0.04395 ) ) + baselineConstant );
				    GraphRefresh();
				} 
				if( graph_refresh_count == 2 )
				{
					graph_refresh_count			=	0;
				}
				else	
				{
					graph_refresh_count++;
				}
				
				hByte_h		=	CharToHex( 0x31, recentValue[7] );
				hByte_l		=	CharToHex( 0x30, recentValue[7] );
				lByte_h		=	CharToHex( 0x31, recentValue[6] );
				lByte_l		=	CharToHex( 0x30, recentValue[6] );
				hTime_h		=	CharToHex( 0x31, recentValue[3] );
				hTime_l		=	CharToHex( 0x30, recentValue[3] );
				lTime_h		=	CharToHex( 0x31, recentValue[2] );
				lTime_l		=	CharToHex( 0x30, recentValue[2] );

				UART1PutChar( 'M' );
				UART1PutChar( hByte_h );
				UART1PutChar( hByte_l );
				UART1PutChar( lByte_h );
				UART1PutChar( lByte_l );
				UART1PutChar( hTime_h );
				UART1PutChar( hTime_l );
				UART1PutChar( lTime_h );
				UART1PutChar( lTime_l );
				UART1PutChar( 0x0D );
				UART1PutChar( 0x0A );
}	
			}			
			Nop();
			Nop();
			Nop();
	    }
	    else
	    {
		    SystemGetRecentValue();
			    
		    signal			=	recentValue[7];
			signal			=	signal << 8;
			signal			=	signal + recentValue[6];
		} 
	//Messung läuft aktuell nicht
		SystemStatus();
	    if( ( system_status & DEV_ACTIVE ) > 0 )
	    {
		    deviceActive		=	TRUE;
		}
		else
		{
			deviceActive		=	FALSE;
		}	
		if( ( system_status & DEV_STARTED ) > 0 )
		{
			deviceStarted		=	TRUE;
		}
		else
		{
			deviceStarted		=	FALSE;
		}	
//POLL: Display Send Buffer	
		if( DisplayBufferInfo() > 0 )
		{ 
			DisplayGetBuffer(); 
		}
//POLL: Display Refresh
		if( sec10 >= 10 )
		{
			DisplayRefreshMain();
			sec10 = 0;
		}
//POLL: Status und Werte an PC ausgeben?
		if( pcStatusCount >= 3 )
		{
			UART1PutChar( 'S' );
			//Seriennummer
			UART1PutChar( IntToHex( 0x33, SN ) );
			UART1PutChar( IntToHex( 0x32, SN ) );
			UART1PutChar( IntToHex( 0x31, SN ) );
			UART1PutChar( IntToHex( 0x30, SN ) );
			//Signal
			UART1PutChar( CharToHex( 0x31, recentValue[7] ) );
			UART1PutChar( CharToHex( 0x30, recentValue[7] ) );
			UART1PutChar( CharToHex( 0x31, recentValue[6] ) );
			UART1PutChar( CharToHex( 0x30, recentValue[6] ) );
			//Akkuspannung
			UART1PutChar( UnsignedIntToHex( 0x33, akku ) );
			UART1PutChar( UnsignedIntToHex( 0x32, akku ) );
			UART1PutChar( UnsignedIntToHex( 0x31, akku ) );
			UART1PutChar( UnsignedIntToHex( 0x30, akku ) );
			//Druck
			UART1PutChar( UnsignedShortToHex( 0x31, vordruck_istwert ) );
			UART1PutChar( UnsignedShortToHex( 0x30, vordruck_istwert ) );
			//Temperatur
			UART1PutChar( UnsignedIntToHex( 0x33, saeule_istwert ) );
			UART1PutChar( UnsignedIntToHex( 0x32, saeule_istwert ) );
			UART1PutChar( UnsignedIntToHex( 0x31, saeule_istwert ) );
			UART1PutChar( UnsignedIntToHex( 0x30, saeule_istwert ) );
			//Fluss
			UART1PutChar( UnsignedIntToHex( 0x33, fluss_istwert ) );
			UART1PutChar( UnsignedIntToHex( 0x32, fluss_istwert ) );
			UART1PutChar( UnsignedIntToHex( 0x31, fluss_istwert ) );
			UART1PutChar( UnsignedIntToHex( 0x30, fluss_istwert ) );
			//Empfindlichkeit
			UART1PutChar( UnsignedShortToHex( 0x31, empfindlichkeit ) );
			UART1PutChar( UnsignedShortToHex( 0x30, empfindlichkeit ) );
			//Systemstatus
			UART1PutChar( CharToHex( 0x31, system_status ) );
			UART1PutChar( CharToHex( 0x30, system_status ) );
			//Messtyp
			UART1PutChar( CharToHex( 0x31, measurementType ) );
			UART1PutChar( CharToHex( 0x30, measurementType ) );
			//Zeit
			UART1PutChar( CharToHex( 0x31, recentValue[3] ) );
			UART1PutChar( CharToHex( 0x30, recentValue[3] ) );
			UART1PutChar( CharToHex( 0x31, recentValue[2] ) );
			UART1PutChar( CharToHex( 0x30, recentValue[2] ) );
			UART1PutChar( 0x0D );
			UART1PutChar( 0x0A );
			
			pcStatusCount		=	0;
		}	
		else
		{
			pcStatusCount++;
		}
//POLL: Passwort zur Datenübertragung korrekt?
		if( checkPwd )
		{
			DisplaySaveDataInsert();
			displayWait		=	0;
			while( displayWait < 2 ) { Nop(); }
		//USB Stick gemeldet?
			if( PORTCbits.RC1 == 1 )
			{
				percentPerFile		=	0;
				percentComplete		=	0;
				filesNo				=	0;
				filesCopiedRefresh	=	0;

				DisplaySaveData();

				diskProperties.new_request		=	TRUE;
				do
				{
					FSGetDiskProperties( &diskProperties );
				} while( diskProperties.properties_status == FS_GET_PROPERTIES_STILL_WORKING );

				numberOfFiles		=	diskProperties.results.total_clusters - diskProperties.results.free_clusters;
				percentPerFile		=	200 / numberOfFiles;

				DisplaySendBargraphTotalNumber( ( int ) numberOfFiles );
				DisplaySendBargraphRecentNumber( ( int ) filesNo );

				LATCbits.LATC2		=	1;
				UARTSendString( "56AB", 4, 1 );
				SDCopyContent();
				LATCbits.LATC2		=	0;
				DisplaySaveDataComplete();
				while( PORTCbits.RC1 == 1 ) { Nop(); }
				DisplaySaveDataReturn();
				checkPwd			=	FALSE;
			}
		}
//POLL: Masterpasswort eingegeben?
		if( checkMasterPwd )
		{
			DisplayShowDeveloperMode();
			
			checkMasterPwd			=	FALSE;
		}	
//POLL: Char auf UART1 ausgeben
		if( U1STAbits.URXDA == 1 )
		{
			UART1GetPCString();
			PCRequest();
		}	
//POLL: Akkuleistung zu niedrig?
		if( battery_low )
		{
			if( autoMeasurement )
			{
				autoMeasurement		=	FALSE;
			}
			malfunction		=	TRUE;
			errorCode		=	4;
			//DisplayShowBatteryLow();
		}	
//POLL: mA-Ausgabe testen
#if defined ODOR
		if( ( uebertragungsart == TRUE ) && ( mA1 < 830 ) )
		{
			malfunction		=	TRUE;
			errorCode		=	5;
		}
		if( writeStartLog )
		{
			SystemLogTimeOfAutoStart( 0 );
			writeStartLog = FALSE;
		}	
#endif
//POLL: Werte grad geckeckt? -> unchecken
		checkPoll				=	FALSE;
    }//end if( checkPoll )
    
    }//end while(1)
    
}//end main

/********************************************************************
 * Function:        static void InitializeSystem(void)
 *******************************************************************/
static void InitializeSystem(void)
{
    #if defined(__C30__)
        AD1PCFGL = 0xFFFF;
		REFOCON = 0x8A00;
		TRISBbits.TRISB15	=	0;

		//UART
		TRISFbits.TRISF4	=	1;
		TRISFbits.TRISF5	=	0;
		TRISFbits.TRISF12	=	1;
		TRISFbits.TRISF13	=	0;
		LATFbits.LATF4		=	0;
		LATFbits.LATF5		=	0;
		LATFbits.LATF12		=	0;
		LATFbits.LATF13		=	0;
		
		TRISFbits.TRISF2	=	1;
		TRISFbits.TRISF3	=	0;
		LATFbits.LATF2		=	0;
		LATFbits.LATF3		=	0;

		//CPU COMMUNICATION
		TRISCbits.TRISC1	=	1;
		TRISCbits.TRISC2	=	0;
		TRISGbits.TRISG6	=	0;
		TRISGbits.TRISG3	=	1;
		LATCbits.LATC1		=	0;
		LATCbits.LATC2		=	0;
		LATCbits.LATC3		=	0;
		LATGbits.LATG6		=	0;	
		
		//ANALYTIC RESET
		TRISDbits.TRISD1	=	0;
		LATDbits.LATD1		=	1;	
    #endif

    #if defined(USE_USB_BUS_SENSE_IO)
    tris_usb_bus_sense = INPUT_PIN; // See HardwareProfile.h
    #endif
    
    #if defined(USE_SELF_POWER_SENSE_IO)
    tris_self_power = INPUT_PIN;	// See HardwareProfile.h
    #endif

    #if defined(PIC24FJ256GB110_PIM)

    RPINR20bits.SDI1R = 23;
    RPOR7bits.RP15R = 7;
    RPOR0bits.RP0R = 8;    

    //enable a pull-up for the card detect, just in case the SD-Card isn't attached
    //  then lets have a pull-up to make sure we don't think it is there.
    CNPU5bits.CN68PUE = 1; 

    #endif

}//end InitializeSystem

/********************************************************************
 * Function:        void ProcessIO(void)
 *******************************************************************/
void ProcessIO(void)
{   
    // User Application USB tasks
    if((USBDeviceState < CONFIGURED_STATE)||(USBSuspendControl==1)) return;

    MSDTasks();  
}//end ProcessIO

/******************************************************************************
 * Function:        WORD_VAL ReadPOT(void)
 *****************************************************************************/
WORD_VAL ReadPOT(void)
{
    WORD_VAL w;

    #if defined(__C30__) || defined(__C32__)
        #if defined(PIC24FJ256GB110_PIM)
            AD1CHS = 0x5;           //MUXA uses AN5

            // Get an ADC sample
            AD1CON1bits.SAMP = 1;           //Start sampling
            for(w.Val=0;w.Val<1000;w.Val++); //Sample delay, conversion start automatically
            AD1CON1bits.SAMP = 0;           //Start sampling
            for(w.Val=0;w.Val<1000;w.Val++); //Sample delay, conversion start automatically
            while(!AD1CON1bits.DONE);       //Wait for conversion to complete

        #endif

        w.Val = ADC1BUF0;

    #endif

    return w;
}//end ReadPOT

/******************************************************************************
 * Function:        void _USB1Interrupt(void)
 *****************************************************************************/
#if 0
void __attribute__ ((interrupt)) _USB1Interrupt(void)
{
    #if !defined(self_powered)
        if(U1OTGIRbits.ACTVIF)
        {
            IEC5bits.USB1IE = 0;
            U1OTGIEbits.ACTVIE = 0;
            IFS5bits.USB1IF = 0;
        
            //USBClearInterruptFlag(USBActivityIFReg,USBActivityIFBitNum);
            USBClearInterruptFlag(USBIdleIFReg,USBIdleIFBitNum);
            //USBSuspendControl = 0;
        }
    #endif
}
#endif

/*******************************************************************
 * Function:        prvSetupTimerInterrupt
 *******************************************************************/
void prvSetupTimerInterrupt( void )
{
	/** SETUP TIMER ****************************************************/
	T1CON				=	0x00;		//stops timer and resets control registers
	TMR1				=	0x00;		//clear contents of timer register
	PR1					=	0x3509;		//load period registers with value 0xFFFF;
	IPC0bits.T1IP		=	0x01;		//setup timer1 interrupt priority
	IFS0bits.T1IF		=	0;			//clear timer1 interrupt flag
	IEC0bits.T1IE		=	1;			//enable timer1 interrupts
	T1CONbits.TCKPS0	=	1;
    T1CONbits.TCKPS1	=	1;
	T1CONbits.TON		=	1;			//start timer 1 with prescale 1:1 and clock
										//source set to internal instruction cycle
}

/*******************************************************************
 * Function:        ReadAkku
 *******************************************************************/
short int ReadAkku( void )
{
	WORD_VAL w;
	double y			=	0;

	#if defined(__C30__) || defined(__C32__)
        #if defined(PIC24FJ256GB110_PIM)
            AD1CHS = 0x5;           //MUXA uses AN5

            // Get an ADC sample
            AD1CON1bits.SAMP = 1;           //Start sampling
            for(w.Val=0;w.Val<1000;w.Val++); //Sample delay, conversion start automatically
            AD1CON1bits.SAMP = 0;           //Start sampling
            for(w.Val=0;w.Val<1000;w.Val++); //Sample delay, conversion start automatically
            while(!AD1CON1bits.DONE);       //Wait for conversion to complete

        #endif

        w.Val = ADC1BUF0;


    #endif

	y				=	( double ) w.Val;
	y				=	y * 0.1678;

	return ( short int ) y;
}

/*******************************************************************
 * Function:        _T1Interrupt
 *******************************************************************/
void __attribute__((__interrupt__,__auto_psv__)) _T1Interrupt( void )
{
	IFS0bits.T1IF	=	0;
	sec10++;
	displayWait++;
	checkFlussCount++;
	if( filesCopiedRefresh == 50 )
	{
		fileCopiedRefresh	=	TRUE;
		filesCopiedRefresh	=	0;
	}
	else
	{
		filesCopiedRefresh++;
	}
	checkPoll				=	TRUE;
	ClrWdt();
}

/*******************************************************************
 * Function:        CheckTransferStatus
 *******************************************************************/
void CheckTransferStatus( void )
{
	if( ( filesNo * percentPerFile ) >= ( percentComplete + 1 ) )
	{
		percentComplete		=	( ceil ) ( filesNo * percentPerFile );
		if( percentComplete <= 200 )
		{
			DisplaySendBargraphValue( percentComplete );
		}
	}
	if( fileCopiedRefresh == TRUE )
	{
		fileCopiedRefresh	=	FALSE;
		DisplaySendBargraphRecentNumber( filesNo );
	}
}

/*******************************************************************
 * Function:        CharToHex
 *******************************************************************/
char CharToHex( int byte, unsigned char c )
{
	char res;
	res			=	c;
	if( byte == 0x31 )
	{
		res		=	res >> 4;
	}
	res			=	res & 0x0F;
	if( res == 0b00000000 ) return '0';
	else if( res == 0b00000001 ) return '1';
	else if( res == 0b00000010 ) return '2';
	else if( res == 0b00000011 ) return '3';
	else if( res == 0b00000100 ) return '4';
	else if( res == 0b00000101 ) return '5';
	else if( res == 0b00000110 ) return '6';
	else if( res == 0b00000111 ) return '7';
	else if( res == 0b00001000 ) return '8';
	else if( res == 0b00001001 ) return '9';
	else if( res == 0b00001010 ) return 'A';
	else if( res == 0b00001011 ) return 'B';
	else if( res == 0b00001100 ) return 'C';
	else if( res == 0b00001101 ) return 'D';
	else if( res == 0b00001110 ) return 'E';
	else return 'F';
}

/*******************************************************************
 * Function:        UnsignedShortToHex
 *******************************************************************/
char UnsignedShortToHex( int byte, unsigned short s )
{
	unsigned short res;
	res			=	s;
	if( byte == 0x31 )
	{
		res		=	res >> 4;
	}
	res			=	res & 0x0F;
	if( res == 0b00000000 ) return '0';
	else if( res == 0b00000001 ) return '1';
	else if( res == 0b00000010 ) return '2';
	else if( res == 0b00000011 ) return '3';
	else if( res == 0b00000100 ) return '4';
	else if( res == 0b00000101 ) return '5';
	else if( res == 0b00000110 ) return '6';
	else if( res == 0b00000111 ) return '7';
	else if( res == 0b00001000 ) return '8';
	else if( res == 0b00001001 ) return '9';
	else if( res == 0b00001010 ) return 'A';
	else if( res == 0b00001011 ) return 'B';
	else if( res == 0b00001100 ) return 'C';
	else if( res == 0b00001101 ) return 'D';
	else if( res == 0b00001110 ) return 'E';
	else return 'F';
}	

/*******************************************************************
 * Function:        UnsignedIntToHex
 *******************************************************************/
char UnsignedIntToHex( int byte, unsigned int i )
{
	unsigned int res;
	res			=	i;
	if( byte == 0x31 )
	{
		res		=	res >> 4;
	}
	else if( byte == 0x32 )
	{
		res		=	res >> 8;
	}	
	else if( byte == 0x33 )
	{
		res		=	res >> 12;
	}	
	res			=	res & 0x000F;
	if( res == 0b00000000 ) return '0';
	else if( res == 0b00000001 ) return '1';
	else if( res == 0b00000010 ) return '2';
	else if( res == 0b00000011 ) return '3';
	else if( res == 0b00000100 ) return '4';
	else if( res == 0b00000101 ) return '5';
	else if( res == 0b00000110 ) return '6';
	else if( res == 0b00000111 ) return '7';
	else if( res == 0b00001000 ) return '8';
	else if( res == 0b00001001 ) return '9';
	else if( res == 0b00001010 ) return 'A';
	else if( res == 0b00001011 ) return 'B';
	else if( res == 0b00001100 ) return 'C';
	else if( res == 0b00001101 ) return 'D';
	else if( res == 0b00001110 ) return 'E';
	else return 'F';
}	

/*******************************************************************
 * Function:        IntToHex
 *******************************************************************/
char IntToHex( int byte, int i )
{
	int res;
	res			=	i;
	if( byte == 0x31 )
	{
		res		=	res >> 4;
	}
	else if( byte == 0x32 )
	{
		res		=	res >> 8;
	}	
	else if( byte == 0x33 )
	{
		res		=	res >> 12;
	}	
	res			=	res & 0x000F;
	if( res == 0b00000000 ) return '0';
	else if( res == 0b00000001 ) return '1';
	else if( res == 0b00000010 ) return '2';
	else if( res == 0b00000011 ) return '3';
	else if( res == 0b00000100 ) return '4';
	else if( res == 0b00000101 ) return '5';
	else if( res == 0b00000110 ) return '6';
	else if( res == 0b00000111 ) return '7';
	else if( res == 0b00001000 ) return '8';
	else if( res == 0b00001001 ) return '9';
	else if( res == 0b00001010 ) return 'A';
	else if( res == 0b00001011 ) return 'B';
	else if( res == 0b00001100 ) return 'C';
	else if( res == 0b00001101 ) return 'D';
	else if( res == 0b00001110 ) return 'E';
	else return 'F';
}

/*******************************************************************
 * Function:        UnsignedLongIntToHex
 *******************************************************************/
char UnsignedLongIntToHex( int byte, unsigned long int li )
{
	unsigned long int res;
	res			=	li;
	if( byte == 0x31 )
	{
		res		=	res >> 4;
	}
	else if( byte == 0x32 )
	{
		res		=	res >> 8;
	}	
	else if( byte == 0x33 )
	{
		res		=	res >> 12;
	}	
	else if( byte == 0x34 )
	{
		res		=	res >> 16;
	}	
	else if( byte == 0x35 )
	{
		res		=	res >> 20;
	}	
	else if( byte == 0x36 )
	{
		res		=	res >> 24;
	}	
	else if( byte == 0x37 )
	{
		res		=	res >> 28;
	}	
	res			=	res & 0x000F;
	if( res == 0b00000000 ) return '0';
	else if( res == 0b00000001 ) return '1';
	else if( res == 0b00000010 ) return '2';
	else if( res == 0b00000011 ) return '3';
	else if( res == 0b00000100 ) return '4';
	else if( res == 0b00000101 ) return '5';
	else if( res == 0b00000110 ) return '6';
	else if( res == 0b00000111 ) return '7';
	else if( res == 0b00001000 ) return '8';
	else if( res == 0b00001001 ) return '9';
	else if( res == 0b00001010 ) return 'A';
	else if( res == 0b00001011 ) return 'B';
	else if( res == 0b00001100 ) return 'C';
	else if( res == 0b00001101 ) return 'D';
	else if( res == 0b00001110 ) return 'E';
	else return 'F';
}	

/*******************************************************************
 * Function:        LongIntToHex
 *******************************************************************/
char LongIntToHex( int byte, long int li )
{
	long int res;
	res			=	li;
	if( byte == 0x31 )
	{
		res		=	res >> 4;
	}
	else if( byte == 0x32 )
	{
		res		=	res >> 8;
	}	
	else if( byte == 0x33 )
	{
		res		=	res >> 12;
	}	
	else if( byte == 0x34 )
	{
		res		=	res >> 16;
	}	
	else if( byte == 0x35 )
	{
		res		=	res >> 20;
	}	
	else if( byte == 0x36 )
	{
		res		=	res >> 24;
	}	
	else if( byte == 0x37 )
	{
		res		=	res >> 28;
	}	
	res			=	res & 0x000F;
	if( res == 0b00000000 ) return '0';
	else if( res == 0b00000001 ) return '1';
	else if( res == 0b00000010 ) return '2';
	else if( res == 0b00000011 ) return '3';
	else if( res == 0b00000100 ) return '4';
	else if( res == 0b00000101 ) return '5';
	else if( res == 0b00000110 ) return '6';
	else if( res == 0b00000111 ) return '7';
	else if( res == 0b00001000 ) return '8';
	else if( res == 0b00001001 ) return '9';
	else if( res == 0b00001010 ) return 'A';
	else if( res == 0b00001011 ) return 'B';
	else if( res == 0b00001100 ) return 'C';
	else if( res == 0b00001101 ) return 'D';
	else if( res == 0b00001110 ) return 'E';
	else return 'F';
}		

/*******************************************************************
 * Function:        DirectoryLevel1
 *******************************************************************/
int DirectoryLevel1( void )
{
	char temp_l1[9];

	FSchdir( ( char* ) 0x5C );

	if( FindFirst( "*", ( unsigned int ) 0x3F, &level1 ) == 1 )
	{
		//KEINE ORDNER GEFUNDEN
		return 1;
	}
	else
	{
		strcpy( temp_l1, level1.filename );
		FSchdir( level1.filename );
		FSgetcwd( curr_dir, 24 );

		filesNo++;

		Nop();
		Nop();
		Nop();

		DirectoryLevel2();

		FSchdir( ".." );

		while( FindNext( &level1 ) != 1 )
		{
			if( strcmp( temp_l1, level1.filename ) == 0 )
			{
				return 0;
			}
			strcpy( temp_l1, level1.filename );
			FSchdir( level1.filename );
			FSgetcwd( curr_dir, 24 );

			filesNo++;

			Nop();
			Nop();
			Nop();

			DirectoryLevel2();

			FSchdir( ".." );
		}
	}
	return 0;
}

/*******************************************************************
 * Function:        DirectoryLevel2
 *******************************************************************/
int DirectoryLevel2( void )
{
	char temp_l2[9];

	if( FindFirst( "*", ( unsigned int ) 0x3F, &level2 ) == 1 )
	{
		return 1;
	}
	else
	{
		FindNext( &level2 );
		FindNext( &level2 );
		strcpy( temp_l2, level2.filename );
		FSchdir( level2.filename );
		FSgetcwd( curr_dir, 24 );

		filesNo++;

		Nop();
		Nop();
		Nop();

		DirectoryLevel3();

		FSchdir( ".." );

		while( FindNext( &level2 ) != 1 )
		{
			if( strcmp( temp_l2, level2.filename ) == 0 )
			{
				return 0;
			}
			strcpy( temp_l2, level2.filename );
			FSchdir( level2.filename );
			FSgetcwd( curr_dir, 24 );

			filesNo++;

			Nop();
			Nop();
			Nop();

			DirectoryLevel3();

			FSchdir( ".." );
		}
	}
	return 0;
}

/*******************************************************************
 * Function:        DirectoryLevel3
 *******************************************************************/
int DirectoryLevel3( void )
{
	char temp_l3[9];

	if( FindFirst( "*", ( unsigned int ) 0x3F, &level3 ) == 1 )
	{
		return 1;
	}
	else
	{
		FindNext( &level3 );
		FindNext( &level3 );
		strcpy( temp_l3, level3.filename );
		FSchdir( level3.filename );
		FSgetcwd( curr_dir, 24 );

		filesNo++;

		Nop();
		Nop();
		Nop();

		DirectoryLevel4();

		FSchdir( ".." );

		while( FindNext( &level3 ) != 1 )
		{
			if( strcmp( temp_l3, level3.filename ) == 0 )
			{
				return 0;
			}
			strcpy( temp_l3, level3.filename );
			FSchdir( level3.filename );
			FSgetcwd( curr_dir, 24 );

			filesNo++;

			Nop();
			Nop();
			Nop();

			DirectoryLevel4();

			FSchdir( ".." );
		}
	}
	return 0;
}

/*******************************************************************
 * Function:        DirectoryLevel4
 *******************************************************************/
int DirectoryLevel4( void )
{
	char temp_l4[9];

	if( FindFirst( "*", ( unsigned int ) 0x3F, &level4 ) == 1 )
	{
		return 1;
	}
	else
	{
		FindNext( &level4 );
		FindNext( &level4 );
		strcpy( temp_l4, level4.filename );
		FSchdir( level4.filename );
		FSgetcwd( curr_dir, 24 );

		filesNo++;

		UARTSendString( curr_dir, 20, 1 );

		Nop();
		Nop();
		Nop();

		SDGetFiles();

		FSchdir( ".." );

		while( FindNext( &level4 ) != 1 )
		{
			if( strcmp( temp_l4, level4.filename ) == 0 )
			{
				return 0;
			}
			strcpy( temp_l4, level4.filename );
			FSchdir( level4.filename );
			FSgetcwd( curr_dir, 24 );

			filesNo++;

			UARTSendString( curr_dir, 20, 1 );

			Nop();
			Nop();
			Nop();

			SDGetFiles();

			FSchdir( ".." );
		}
	}
	return 0;
}

/*******************************************************************
 * Function:        SDCopyContent
 *******************************************************************/
void SDCopyContent( void )
{
	DirectoryLevel1();

	while( PORTFbits.RF12 == 0 );
	UARTPutChar( 0x7E );
}	

/*******************************************************************
 * Function:        SDGetFiles
 *******************************************************************/
int SDGetFiles( void )
{
	char temp;
	char temp_file[13];
	int i			=	0;
	int j			=	0;

	if( FindFirst( "*.txt", ( unsigned int ) 0x3F, &csvfile ) == 1 )
	{
		return 1;
	}
	else
	{
		strcpy( temp_file, csvfile.filename );

		filesNo++;
		
		if( ( copyAll ) || ( temp_file[0] != 'X' ) )
		{

		UARTSendString( temp_file, 13, 2 );

//FILEINHALT LESEN UND SENDEN
		copyFile		=	FSfopen( csvfile.filename, READ );

		for( j = 0; j < 3; j++ )
		{
			FSfread( file_data, 64, 8, copyFile );
			for( i = 0; i < 512; i++ )
			{
				while( PORTFbits.RF12 == 0 );
				UARTPutChar( file_data[i] );

				Nop();
				Nop();
			}
		}
		FSfread( file_data, 511, 1, copyFile );
		for( i = 0; i < 511; i++ )
		{
			while( PORTFbits.RF12 == 0 );
			UARTPutChar( file_data[i] );
			
			Nop();
			Nop();
		}
		
//DATEINAMEN ÄNDERN ZU BEREITS KOPIERT
		if( !copyAll )
		{
			temp				=	temp_file[0];
			temp_file[0]		=	'X';
			FSrename( temp_file, copyFile );
			temp_file[0]		=	temp;
		}	
		
		}	

		FSfclose( copyFile );
		copyFile		=	NULL;

		CheckTransferStatus();
//ENDE FILEINHALT LESEN UND SENDEN

		while( FindNext( &csvfile ) != 1 )
		{
			if( strcmp( temp_file, csvfile.filename ) == 0 )
			{
				return 0;
			}
			strcpy( temp_file, csvfile.filename );

			filesNo++;
			
			if( ( copyAll ) || ( temp_file[0] != 'X' ) )
			{

			UARTSendString( temp_file, 13, 2 );

//FILEINHALT LESEN UND SENDEN
			copyFile		=	FSfopen( csvfile.filename, READ );

			for( j = 0; j < 3; j++ )
			{
				FSfread( file_data, 64, 8, copyFile );
				for( i = 0; i < 512; i++ )
				{
					while( PORTFbits.RF12 == 0 );
					UARTPutChar( file_data[i] );

					Nop();
					Nop();
				}
			}
			FSfread( file_data, 511, 1, copyFile );
			for( i = 0; i < 511; i++ )
			{
				while( PORTFbits.RF12 == 0 );
				UARTPutChar( file_data[i] );
				
				Nop();
				Nop();
			}
			
//DATEINAMEN ÄNDERN ZU BEREITS KOPIERT
			if( !copyAll )
			{
				temp				=	temp_file[0];
				temp_file[0]		=	'X';
				FSrename( temp_file, copyFile );
				temp_file[0]		=	temp;
			}	
			
			FSfclose( copyFile );
			copyFile		=	NULL;

			CheckTransferStatus();
//ENDE FILEINHALT LESEN UND SENDEN

			Nop();
			Nop();
			Nop();
			}
		}
	}
	return 0;
}

/*******************************************************************
 * Function:        DirectoryExists
 *******************************************************************/
BOOL DirectoryExists()
{
	FSchdir( ( char* ) 0x5C );
	if( FSchdir( current_date ) != 0 )
	{
		return FALSE;
	}
	else
	{
		return TRUE;
	}		
} 	

/*******************************************************************
 * Function:        SystemLogError
 *******************************************************************/
void SystemLogError( void )
{
	char header[128];
	char errorDir[25];
	char filename[13];
	unsigned char buffer[8];
	
	DWORD size;
	unsigned int puffer;
	
	int i				=	0;
	
	ClockGetDatestring( errorDir );
	SystemGetTimeString();
	
	//Tag
	header[0]		=	errorDir[18];
	header[1]		=	errorDir[19];
	header[2]		=	'.';
	//Monat
	header[3]		=	errorDir[15];
	header[4]		=	errorDir[16];
	header[5]		=	'.';
	//Jahr
	header[6]		=	errorDir[10];
	header[7]		=	errorDir[11];
	header[8]		=	errorDir[12];
	header[9]		=	errorDir[13];
	header[10]		=	' ';
	header[11]		=	'(';
	//Zeit
	header[12]		=	uhrzeitAktuell[0];
	header[13]		=	uhrzeitAktuell[1];
	header[14]		=	uhrzeitAktuell[2];
	header[15]		=	uhrzeitAktuell[3];
	header[16]		=	uhrzeitAktuell[4];
	header[17]		=	')';
	header[18]		=	':';
	header[19]		=	' ';
	
	errorDir[0] 	=	0x5C;
	errorDir[1] 	=	'E';
	errorDir[2] 	=	'r';
	errorDir[3] 	=	'r';
	errorDir[4] 	=	'o';
	errorDir[5] 	=	'r';
	errorDir[6] 	=	'L';
	errorDir[7] 	=	'o';
	errorDir[8] 	=	'g';
	errorDir[9] 	=	0x5C;
	errorDir[10] 	=	'0';
	errorDir[11] 	=	'0';
	errorDir[12] 	=	'0';
	errorDir[13] 	=	'1';
	errorDir[14] 	=	0x5C;
	errorDir[15] 	=	'0';
	errorDir[16] 	=	'1';
	errorDir[17] 	=	0x5C;
	errorDir[18] 	=	'0';
	errorDir[19] 	=	'1';
	errorDir[20] 	=	0x00;
	
	//Fehler
	#if defined( DEUTSCH )
	if( errorCode == 1 )
	{
		header[20]	=	'G';
		header[21]	=	'a';
		header[22]	=	's';
		header[23]	=	'f';
		header[24]	=	'l';
		header[25]	=	'u';
		header[26]	=	's';
		header[27]	=	's';
		header[28]	=	' ';
		header[29]	=	'z';
		header[30]	=	'u';
		header[31]	=	' ';
		header[32]	=	'n';
		header[33]	=	'i';
		header[34]	=	'e';
		header[35]	=	'd';
		header[36]	=	'r';
		header[37]	=	'i';
		header[38]	=	'g';
		header[39]	=	0x0A;
		header[40]	=	0x0D;
		header[41]	=	0x00;
	}
	else if( errorCode == 2 )
	{
		header[20]	=	'K';
		header[21]	=	'a';
		header[22]	=	'l';
		header[23]	=	'i';
		header[24]	=	'b';
		header[25]	=	'r';
		header[26]	=	'a';
		header[27]	=	't';
		header[28]	=	'i';
		header[29]	=	'o';
		header[30]	=	'n';
		header[31]	=	's';
		header[32]	=	'f';
		header[33]	=	'l';
		header[34]	=	'a';
		header[35]	=	'e';
		header[36]	=	'c';
		header[37]	=	'h';
		header[38]	=	'e';
		header[39]	=	' ';
		header[40]	=	'z';
		header[41]	=	'u';
		header[42]	=	' ';
		header[43]	=	'k';
		header[44]	=	'l';
		header[45]	=	'e';
		header[46]	=	'i';
		header[47]	=	'n';
		header[48]	=	0x0A;
		header[49]	=	0x0D;
		header[50]	=	0x00;
	}
	else if( errorCode == 3 )
	{
		header[20]	=	'M';
		header[21]	=	'e';
		header[22]	=	's';
		header[23]	=	's';
		header[24]	=	'p';
		header[25]	=	'e';
		header[26]	=	'a';
		header[27]	=	'k';
		header[28]	=	' ';
		header[29]	=	'a';
		header[30]	=	'u';
		header[31]	=	's';
		header[32]	=	's';
		header[33]	=	'e';
		header[34]	=	'r';
		header[35]	=	'h';
		header[36]	=	'a';
		header[37]	=	'l';
		header[38]	=	'b';
		header[39]	=	' ';
		header[40]	=	'S';
		header[41]	=	'c';
		header[42]	=	'h';
		header[43]	=	'r';
		header[44]	=	'a';
		header[45]	=	'n';
		header[46]	=	'k';
		header[47]	=	'e';
		header[48]	=	'n';
		header[49]	=	0x0A;
		header[50]	=	0x0D;
		header[51]	=	0x00;
	}
	else if( errorCode == 4 )
	{
		header[20]	=	'B';
		header[21]	=	'a';
		header[22]	=	't';
		header[23]	=	't';
		header[24]	=	'e';
		header[25]	=	'r';
		header[26]	=	'i';
		header[27]	=	'e';
		header[28]	=	's';
		header[29]	=	'p';
		header[30]	=	'a';
		header[31]	=	'n';
		header[32]	=	'n';
		header[33]	=	'u';
		header[34]	=	'n';
		header[35]	=	'g';
		header[36]	=	' ';
		header[37]	=	'z';
		header[38]	=	'u';
		header[39]	=	' ';
		header[40]	=	'n';
		header[41]	=	'i';
		header[42]	=	'e';
		header[43]	=	'd';
		header[44]	=	'r';
		header[45]	=	'i';
		header[46]	=	'g';
		header[47]	=	0x0A;
		header[48]	=	0x0D;
		header[49]	=	0x00;
	}
	else if( errorCode == 5 )
	{
		header[20]	=	'm';
		header[21]	=	'A';
		header[22]	=	'-';
		header[23]	=	'U';
		header[24]	=	'e';
		header[25]	=	'b';
		header[26]	=	'e';
		header[27]	=	'r';
		header[28]	=	't';
		header[29]	=	'r';
		header[30]	=	'a';
		header[31]	=	'g';
		header[32]	=	'u';
		header[33]	=	'n';
		header[34]	=	'g';
		header[35]	=	's';
		header[36]	=	'f';
		header[37]	=	'e';
		header[38]	=	'h';
		header[39]	=	'l';
		header[40]	=	'e';
		header[41]	=	'r';
		header[42]	=	0x0A;
		header[43]	=	0x0D;
		header[44]	=	0x00;
	}
	else
	{
		header[20]	=	'N';
		header[21]	=	'i';
		header[22]	=	'c';
		header[23]	=	'h';
		header[24]	=	't';
		header[25]	=	' ';
		header[26]	=	'd';
		header[27]	=	'o';
		header[28]	=	'k';
		header[29]	=	'u';
		header[30]	=	'm';
		header[31]	=	'e';
		header[32]	=	'n';
		header[33]	=	't';
		header[34]	=	'i';
		header[35]	=	'e';
		header[36]	=	'r';
		header[37]	=	't';
		header[38]	=	'e';
		header[39]	=	' ';
		header[40]	=	'S';
		header[41]	=	't';
		header[42]	=	'o';
		header[43]	=	'e';
		header[44]	=	'r';
		header[45]	=	'u';
		header[46]	=	'n';
		header[47]	=	'g';
		header[48]	=	0x0A;
		header[49]	=	0x0D;
		header[50]	=	0x00;
	}	
	#endif
	
	#if defined( ENGLISCH )
	if( errorCode == 1 )
	{
		header[20]	=	'G';
		header[21]	=	'a';
		header[22]	=	's';
		header[23]	=	'f';
		header[24]	=	'l';
		header[25]	=	'o';
		header[26]	=	'w';
		header[27]	=	' ';
		header[28]	=	'l';
		header[29]	=	'o';
		header[30]	=	'w';
		header[31]	=	0x0A;
		header[32]	=	0x0D;
		header[33]	=	0x00;
	}
	else if( errorCode == 2 )
	{
		header[20]	=	'C';
		header[21]	=	'a';
		header[22]	=	'l';
		header[23]	=	'g';
		header[24]	=	'a';
		header[25]	=	's';
		header[26]	=	'a';
		header[27]	=	'r';
		header[28]	=	'e';
		header[29]	=	'a';
		header[30]	=	' ';
		header[31]	=	't';
		header[32]	=	'o';
		header[33]	=	'o';
		header[34]	=	' ';
		header[35]	=	's';
		header[36]	=	'm';
		header[37]	=	'a';
		header[38]	=	'l';
		header[39]	=	'l';
		header[40]	=	0x0A;
		header[41]	=	0x0D;
		header[42]	=	0x00;
	}
	else if( errorCode == 3 )
	{
		header[20]	=	'P';
		header[21]	=	'e';
		header[22]	=	'a';
		header[23]	=	'k';
		header[24]	=	' ';
		header[25]	=	'o';
		header[26]	=	'u';
		header[27]	=	't';
		header[28]	=	' ';
		header[29]	=	'o';
		header[30]	=	'f';
		header[31]	=	' ';
		header[32]	=	'b';
		header[33]	=	'o';
		header[34]	=	'r';
		header[35]	=	'd';
		header[36]	=	'e';
		header[37]	=	'r';
		header[38]	=	's';
		header[39]	=	0x0A;
		header[40]	=	0x0D;
		header[41]	=	0x00;
	}
	else if( errorCode == 4 )
	{
		header[20]	=	'B';
		header[21]	=	'a';
		header[22]	=	't';
		header[23]	=	't';
		header[24]	=	'e';
		header[25]	=	'r';
		header[26]	=	'y';
		header[27]	=	' ';
		header[28]	=	'c';
		header[29]	=	'h';
		header[30]	=	'a';
		header[31]	=	'r';
		header[32]	=	'g';
		header[33]	=	'e';
		header[34]	=	' ';
		header[35]	=	'l';
		header[36]	=	'o';
		header[37]	=	'w';
		header[38]	=	0x0A;
		header[39]	=	0x0D;
		header[40]	=	0x00;
	}
	else if( errorCode == 5 )
	{
		header[20]	=	'm';
		header[21]	=	'A';
		header[22]	=	'-';
		header[23]	=	'o';
		header[24]	=	'u';
		header[25]	=	't';
		header[26]	=	'p';
		header[27]	=	'u';
		header[28]	=	't';
		header[29]	=	' ';
		header[30]	=	'n';
		header[31]	=	'o';
		header[32]	=	't';
		header[33]	=	' ';
		header[34]	=	'O';
		header[35]	=	'K';
		header[36]	=	0x0A;
		header[37]	=	0x0D;
		header[38]	=	0x00;
	}
	else
	{
		header[20]	=	'U';
		header[21]	=	'n';
		header[22]	=	'd';
		header[23]	=	'o';
		header[24]	=	'c';
		header[25]	=	'u';
		header[26]	=	'm';
		header[27]	=	'e';
		header[28]	=	'n';
		header[29]	=	't';
		header[30]	=	'e';
		header[31]	=	'd';
		header[32]	=	' ';
		header[33]	=	'e';
		header[34]	=	'r';
		header[35]	=	'r';
		header[36]	=	'o';
		header[37]	=	'r';
		header[38]	=	0x0A;
		header[39]	=	0x0D;
		header[40]	=	0x00;
	}	
	#endif
	
	MemoryGetErrorCount();
	errorcount++;
	
	litoa( errorcount, buffer );
	
	if( errorcount< 10 )
	{
		filename[0]		=	'0';
		filename[1]		=	'0';
		filename[2]		=	'0';
		filename[3]		=	'0';
		filename[4]		=	'0';
		filename[5]		=	'0';
		filename[6]		=	'0';
		filename[7]		=	buffer[0];
		filename[8]		=	'.';
		filename[9]		=	't';
		filename[10]	=	'x';
		filename[11]	=	't';
		filename[12]	=	0x00;
	}
	else if( errorcount < 100 )
	{
		filename[0]		=	'0';
		filename[1]		=	'0';
		filename[2]		=	'0';
		filename[3]		=	'0';
		filename[4]		=	'0';
		filename[5]		=	'0';
		filename[6]		=	buffer[0];
		filename[7]		=	buffer[1];
		filename[8]		=	'.';
		filename[9]		=	't';
		filename[10]	=	'x';
		filename[11]	=	't';
		filename[12]	=	0x00;
	}
	else if( errorcount < 1000 )
	{
		filename[0]		=	'0';
		filename[1]		=	'0';
		filename[2]		=	'0';
		filename[3]		=	'0';
		filename[4]		=	'0';
		filename[5]		=	buffer[0];
		filename[6]		=	buffer[1];
		filename[7]		=	buffer[2];
		filename[8]		=	'.';
		filename[9]		=	't';
		filename[10]	=	'x';
		filename[11]	=	't';
		filename[12]	=	0x00;
	}
	else if( errorcount < 10000 )
	{
		filename[0]		=	'0';
		filename[1]		=	'0';
		filename[2]		=	'0';
		filename[3]		=	'0';
		filename[4]		=	buffer[0];
		filename[5]		=	buffer[1];
		filename[6]		=	buffer[2];
		filename[7]		=	buffer[3];
		filename[8]		=	'.';
		filename[9]		=	't';
		filename[10]	=	'x';
		filename[11]	=	't';
		filename[12]	=	0x00;
	}			
	else
	{
		filename[0]		=	'0';
		filename[1]		=	'0';
		filename[2]		=	'0';
		filename[3]		=	buffer[0];
		filename[4]		=	buffer[1];
		filename[5]		=	buffer[2];
		filename[6]		=	buffer[3];
		filename[7]		=	buffer[4];
		filename[8]		=	'.';
		filename[9]		=	't';
		filename[10]	=	'x';
		filename[11]	=	't';
		filename[12]	=	0x00;
	}	
	
	FSchdir( ( char* ) 0x5C );
	if( FSchdir( errorDir ) != 0 )
	{
		FSmkdir( errorDir );
		FSchdir( errorDir );
	}
	
	MemorySetErrorCount();
	
	logFile			=	FSfopen( filename, APPEND );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	
	size		=	FSftell( logFile );
	puffer		=	2048 - size;
	
	while( puffer > 0 )
	{
		if( puffer > 100 )
		{
			for( i = 0; i < 99; i++ )
			{
				header[i]		=	0x20;
			}	
			header[i]		=	0x00;
		}
		else
		{
			for( i = 0; i < puffer; i++ )
			{
				header[i]		=	0x20;
			}	
			header[i]		=	0x00;
		}	
		
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
		
		size		=	FSftell( logFile );
		puffer		=	2048 - size;
	}	
	FSfclose( logFile );	
    
	logFile				=	NULL;
}	

/*******************************************************************
 * Function:        SystemLogTimeOfAutoStart
 *******************************************************************/
void SystemLogTimeOfAutoStart( int type )
{
	char header[128];
	char startDir[25];
	char filename[13];
	unsigned char buffer[8];
	
	DWORD size;
	unsigned int puffer;
	
	int i				=	0;
	
	ClockGetDatestring( startDir );
	SystemGetTimeString();
	
	//Tag
	header[0]		=	startDir[18];
	header[1]		=	startDir[19];
	header[2]		=	'.';
	//Monat
	header[3]		=	startDir[15];
	header[4]		=	startDir[16];
	header[5]		=	'.';
	//Jahr
	header[6]		=	startDir[10];
	header[7]		=	startDir[11];
	header[8]		=	startDir[12];
	header[9]		=	startDir[13];
	header[10]		=	' ';
	header[11]		=	'(';
	//Zeit
	header[12]		=	uhrzeitAktuell[0];
	header[13]		=	uhrzeitAktuell[1];
	header[14]		=	uhrzeitAktuell[2];
	header[15]		=	uhrzeitAktuell[3];
	header[16]		=	uhrzeitAktuell[4];
	header[17]		=	')';
	header[18]		=	':';
	header[19]		=	' ';
	
	startDir[0] 	=	0x5C;
	startDir[1] 	=	'S';
	startDir[2] 	=	't';
	startDir[3] 	=	'a';
	startDir[4] 	=	'r';
	startDir[5] 	=	't';
	startDir[6] 	=	'L';
	startDir[7] 	=	'o';
	startDir[8] 	=	'g';
	startDir[9] 	=	0x5C;
	startDir[10] 	=	'0';
	startDir[11] 	=	'0';
	startDir[12] 	=	'0';
	startDir[13] 	=	'1';
	startDir[14] 	=	0x5C;
	startDir[15] 	=	'0';
	startDir[16] 	=	'1';
	startDir[17] 	=	0x5C;
	startDir[18] 	=	'0';
	startDir[19] 	=	'1';
	startDir[20] 	=	0x00;
	
	//Fehler
	#if defined( DEUTSCH )
	if( type == 0 ) {
		header[20]	=	'A';
		header[21]	=	'u';
		header[22]	=	't';
		header[23]	=	'o';
		header[24]	=	'm';
		header[25]	=	'.';
		header[26]	=	' ';
		header[27]	=	'S';
		header[28]	=	'y';
		header[29]	=	's';
		header[30]	=	't';
		header[31]	=	'e';
		header[32]	=	'm';
		header[33]	=	's';
		header[34]	=	't';
		header[35]	=	'a';
		header[36]	=	'r';
		header[37]	=	't';
		header[38]	=	0x0A;
		header[39]	=	0x0D;
		header[40]	=	0x00;
	} else if( type == 1) {
		header[20]	=	'S';
		header[21]	=	't';
		header[22]	=	'a';
		header[23]	=	'r';
		header[24]	=	't';
		header[25]	=	' ';
		header[26]	=	'n';
		header[27]	=	'a';
		header[28]	=	'c';
		header[29]	=	'h';
		header[30]	=	' ';
		header[31]	=	'q';
		header[32]	=	'u';
		header[33]	=	'i';
		header[34]	=	't';
		header[35]	=	't';
		header[36]	=	'i';
		header[37]	=	'e';
		header[38]	=	'r';
		header[39]	=	'e';
		header[40]	=	'n';
		header[41]	=	0x0A;
		header[42]	=	0x0D;
		header[43]	=	0x00;

	}
	#endif
	
	#if defined( ENGLISCH )
	if( type == 0 ) {
		header[20]	=	'A';
		header[21]	=	'u';
		header[22]	=	't';
		header[23]	=	'o';
		header[24]	=	'm';
		header[25]	=	'.';
		header[26]	=	' ';
		header[27]	=	's';
		header[28]	=	'y';
		header[29]	=	's';
		header[30]	=	't';
		header[31]	=	'e';
		header[32]	=	'm';
		header[33]	=	' ';
		header[34]	=	's';
		header[35]	=	't';
		header[36]	=	'a';
		header[37]	=	'r';
		header[38]	=	't';
		header[39]	=	0x0A;
		header[40]	=	0x0D;
		header[41]	=	0x00;
	} else if( type == 1 ) {
		header[20]	=	'S';
		header[21]	=	't';
		header[22]	=	'a';
		header[23]	=	'r';
		header[24]	=	't';
		header[25]	=	' ';
		header[26]	=	'a';
		header[27]	=	'f';
		header[28]	=	't';
		header[29]	=	'e';
		header[30]	=	'r';
		header[31]	=	' ';
		header[32]	=	'a';
		header[33]	=	'c';
		header[34]	=	'k';
		header[35]	=	'n';
		header[36]	=	'o';
		header[37]	=	'w';
		header[38]	=	'l';
		header[39]	=	'e';
		header[40]	=	'd';
		header[41]	=	'g';
		header[42]	=	'e';
		header[43]	=	0x0A;
		header[44]	=	0x0D;
		header[45]	=	0x00;
	}
	#endif
	

		filename[0]		=	'A';
		filename[1]		=	'u';
		filename[2]		=	't';
		filename[3]		=	'o';
		filename[4]		=	'S';
		filename[5]		=	't';
		filename[6]		=	'r';
		filename[7]		=	't';
		filename[8]		=	'.';
		filename[9]		=	't';
		filename[10]	=	'x';
		filename[11]	=	't';
		filename[12]	=	0x00;

	
	FSchdir( ( char* ) 0x5C );
	if( FSchdir( startDir ) != 0 )
	{
		FSmkdir( startDir );
		FSchdir( startDir );
	}
		
	logFile			=	FSfopen( filename, WRITE );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );

	size		=	FSftell( logFile );
	puffer		=	2048 - size;
	
	while( puffer > 0 )
	{
		if( puffer > 100 )
		{
			for( i = 0; i < 99; i++ )
			{
				header[i]		=	0x20;
			}	
			header[i]		=	0x00;
		}
		else
		{
			for( i = 0; i < puffer; i++ )
			{
				header[i]		=	0x20;
			}	
			header[i]		=	0x00;
		}	
		
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
		
		size		=	FSftell( logFile );
		puffer		=	2048 - size;
	}	
	
	FSfclose( logFile );	
    
	logFile				=	NULL;
}

/*******************************************************************
 * Function:        SystemEndRun
 *******************************************************************/
void SystemEndRun( void )
{
	char filename[13];
	char datum[11];
	char header[512];
	
	DWORD size;
	unsigned int puffer;
	
	int i				=	0;
	
	sekunden			=	0;
	baselineConstant	=	0;

	ClockGetDatestring( current_date );
	GraphDeleteGraph();
	DisplayShowPeakschranke();
	GraphDrawGraph();
	SystemGetTimeString();

	for( i = 0; i <= 10; i++ )
	{
		datum[i]		=	current_date[i+10];
	}

	MemoryGetTaeglicheMessung();
	MemoryGetMeasCount();
	MemoryGetKalReportNr();

	if( DirectoryExists() )
	{
		//gleicher Tag/Messort, keinen neuen Ordner erstellen, Messnummer nicht zurücksetzen
		taeglicheMessung++;
		FSchdir( ( char* ) 0x5C );
		FSchdir( current_date );
	}
	else
	{
		//anderer Tag/Messort, neuer Ordner, Messnummer zurücksetzen
		taeglicheMessung		=	1;
		FSchdir( ( char* ) 0x5C );
		FSmkdir( current_date );
		FSchdir( current_date );
	}

	MemorySetTaeglicheMessung();
	meascount++;

	itoa( SN, buffer );
	filename[0]			=	buffer[0];
	filename[1]			=	buffer[1];
	filename[2]			=	buffer[2];
	filename[3]			=	buffer[3];
	filename[4]			=	0x30;
	filename[5]			=	0x30;
	itoa( taeglicheMessung, buffer );
	if( taeglicheMessung < 10 )
	{
		filename[6]		=	0x30;
		filename[7]		=	buffer[0];
	}
	else if( taeglicheMessung < 100 )
	{
		filename[6]		=	buffer[0];
		filename[7]		=	buffer[1];
	}
	filename[8]			=	0x2E;
	filename[9]			=	't';
	filename[10]		=	'x';
	filename[11]		=	't';
	filename[12]		=	0x00;






//////////////////////////////////////////////////////////////////////
//ABLEITUNG ALT
//////////////////////////////////////////////////////////////////////
	#if defined ABLEITUNG_ALT
	AbleitungInit();
	if( measurementType == 'c' )
	{
		kalReportNr		=	meascount;
		AbleitungAb0( 0 );
	}	
	else if( measurementType == 'm' )
	{
		AbleitungAb0( 1 );
	}	
	else if( measurementType == 'b' )
	{
		kalReportNr		=	meascount;
		AbleitungAb0( 0 );
		AbleitungInit();
		AbleitungAb360();
	}	
	#endif
//////////////////////////////////////////////////////////////////////
//ABLEITUNG NEU
//////////////////////////////////////////////////////////////////////
	#if defined ABLEITUNG_NEU
	AbleitungInit_neu();
	kalReportNr = measurementType == ('c' || 'b') ? meascount : kalReportNr;
	AbleitungCalc(measurementType);
	#endif
//////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////







	MemorySetMeasCount();

//Daten an UART ausgeben
	UARTSendPC(&datum[0]);
	
	logFile				=	FSfopen( filename, APPEND );
	
//KOPF SCHREIBEN
//Gerätenummer
	sprintf( ( char* ) &header[0], "Gerätenummer: %u\n", SN );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Gesamtmessungszähler
	if( meascount < 10 )
	{
		sprintf( ( char* ) &header[0], "Gerätemessung #: %lu       \n\n", meascount );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( meascount < 100 )
	{
		sprintf( ( char* ) &header[0], "Gerätemessung #: %lu      \n\n", meascount );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( meascount < 1000 )
	{
		sprintf( ( char* ) &header[0], "Gerätemessung #: %lu     \n\n", meascount );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( meascount < 10000 )
	{
		sprintf( ( char* ) &header[0], "Gerätemessung #: %lu    \n\n", meascount );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( meascount < 100000 )
	{
		sprintf( ( char* ) &header[0], "Gerätemessung #: %lu   \n\n", meascount );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( meascount < 1000000 )
	{
		sprintf( ( char* ) &header[0], "Gerätemessung #: %lu  \n\n", meascount );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( meascount < 10000000 )
	{
		sprintf( ( char* ) &header[0], "Gerätemessung #: %lu \n\n", meascount );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}
	else if( meascount < 100000000 )
	{
		sprintf( ( char* ) &header[0], "Gerätemessung #: %lu\n\n", meascount );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}
//Messort
	sprintf( ( char* ) &header[0], "Messort: %s\n", messort );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Messdatum
	sprintf( ( char* ) &header[0], "Messdatum: %s\n", datum );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//tägliche Messung Nr.
	if( taeglicheMessung < 10 )
	{
		sprintf( ( char* ) &header[0], "heutige Messung #: %d \n", taeglicheMessung );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else
	{
		sprintf( ( char* ) &header[0], "heutige Messung #: %d\n", taeglicheMessung );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
//Art der Messung
	sprintf( ( char* ) &header[0], "Art der Messung: %c - ", measurementType );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	if( measurementType == 'b' )
	{
		sprintf( ( char* ) &header[0], "Kalibration & Messung\n" );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( measurementType == 'm' )
	{
		sprintf( ( char* ) &header[0], "Messung              \n" );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else
	{
		sprintf( ( char* ) &header[0], "Kalibration          \n" );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
//Beginn der Messung
	sprintf( ( char* ) &header[0], "Beginn der Messung: %s\n", uhrzeitLetzterLauf );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Ende der Messung
	sprintf( ( char* ) &header[0], "Ende der Messung: %s\n\n", uhrzeitAktuell );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Justage
	if( calFlaeche < 10 )
	{
		sprintf( ( char* ) &header[0], "Justagefläche: %lu      \n", calFlaeche );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( calFlaeche < 100 )
	{
		sprintf( ( char* ) &header[0], "Justagefläche: %lu     \n", calFlaeche );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( calFlaeche < 1000 )
	{
		sprintf( ( char* ) &header[0], "Justagefläche: %lu    \n", calFlaeche );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( calFlaeche < 10000 )
	{
		sprintf( ( char* ) &header[0], "Justagefläche: %lu   \n", calFlaeche );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( calFlaeche < 100000 )
	{
		sprintf( ( char* ) &header[0], "Justagefläche: %lu  \n", calFlaeche );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( calFlaeche < 1000000 )
	{
		sprintf( ( char* ) &header[0], "Justagefläche: %lu \n", calFlaeche );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
	else if( calFlaeche < 10000000 )
	{
		sprintf( ( char* ) &header[0], "Justagefläche: %lu\n", calFlaeche );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}	
//Konzentration
	sprintf( ( char* ) &header[0], "Gaskonzentration: %f mg THT\n", ( ( double ) eichgasmenge ) / 10 );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//berechnete Fläche
	if( integral < 10 )
	{
		sprintf( ( char* ) &header[0], "Pruefgasfläche: %lu      \n", integral );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}
	else if( integral < 100 )
	{
		sprintf( ( char* ) &header[0], "Pruefgasfläche: %lu     \n", integral );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}
	else if( integral < 1000 )
	{
		sprintf( ( char* ) &header[0], "Pruefgasfläche: %lu    \n", integral );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}
	else if( integral < 10000 )
	{
		sprintf( ( char* ) &header[0], "Pruefgasfläche: %lu   \n", integral );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}
	else if( integral < 100000 )
	{
		sprintf( ( char* ) &header[0], "Pruefgasfläche: %lu  \n", integral );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}
	else if( integral < 1000000 )
	{
		sprintf( ( char* ) &header[0], "Pruefgasfläche: %lu \n", integral );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}
	else if( integral < 10000000 )
	{
		sprintf( ( char* ) &header[0], "Pruefgasfläche: %lu\n", integral );
		FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	}				
//berechneter Messwert
	sprintf( ( char* ) &header[0], "Messergebnis: %f mg THT\n", ( ( double ) messergebnis ) / 10 );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Peakanfang
	sprintf( ( char* ) &header[0], "Peakanfang: %d Sek.\n", peakStart );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Peakmitte
    sprintf( ( char* ) &header[0], "Peakmitte: %d Sek.\n", peakMitte );
    FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );	
//Peakende
	sprintf( ( char* ) &header[0], "Peakende: %d Sek.\n", peakEnde );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Peakanfang C
	sprintf( ( char* ) &header[0], "Peakanfang C: %d Sek.\n", peakStartC );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Peakmitte C
	sprintf( ( char* ) &header[0], "Peakmitte C: %d Sek.\n", peakMitteC );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Peakende C
	sprintf( ( char* ) &header[0], "Peakende C: %d Sek.\n", peakEndeC );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
//Kalibration Report Nr
	sprintf( ( char* ) &header[0], "KalReport Nr.: %lu \n", kalReportNr );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );

	
//Messdaten
	sprintf( ( char* ) &header[0], "\n\n\n:::DATA BEGIN:::\n" );
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );

	FSfwrite( ( const void* ) &messdaten[0], 1, sizeof( messdaten ), logFile );
	
	size		=	FSftell( logFile );
	puffer		=	2048 - size;
	
	for( i = 0; i < puffer; i++ )
	{
		header[i]		=	0x20;
	}
	header[i]		=	0x00;
	FSfwrite( ( const void* ) &header[0], 1, strlen( header ), logFile );
	
	size		=	FSftell( logFile );
	puffer		=	2048 - size;
	
	FSfclose( logFile );	
    
	logFile				=	NULL;		
	
	Nop();
	Nop();
	Nop();
	
	DisplayShowCalflaeche();
	
	MemorySetKalReportNr();
	
	SystemStatus();
	
	Nop();
	Nop();
	Nop();
	
	syringe		=	FALSE;
}	

/*******************************************************************
 * Function:        UARTSendPC
 *******************************************************************/
void UARTSendPC( char* datum )
{
	//PC: Bezeichner
	UART1PutChar( 'E' );
	//PC: Seriennummer
	UART1PutChar( IntToHex( 0x33, SN ) );
	UART1PutChar( IntToHex( 0x32, SN ) );
	UART1PutChar( IntToHex( 0x31, SN ) );
	UART1PutChar( IntToHex( 0x30, SN ) );
	//PC: Meascount
	UART1PutChar( UnsignedLongIntToHex( 0x37, meascount ) );
	UART1PutChar( UnsignedLongIntToHex( 0x36, meascount ) );
	UART1PutChar( UnsignedLongIntToHex( 0x35, meascount ) );
	UART1PutChar( UnsignedLongIntToHex( 0x34, meascount ) );
	UART1PutChar( UnsignedLongIntToHex( 0x33, meascount ) );
	UART1PutChar( UnsignedLongIntToHex( 0x32, meascount ) );
	UART1PutChar( UnsignedLongIntToHex( 0x31, meascount ) );
	UART1PutChar( UnsignedLongIntToHex( 0x30, meascount ) );
	//PC: Datum
	UART1PutChar( CharToHex( 0x31, datum[0] ) );
	UART1PutChar( CharToHex( 0x30, datum[0] ) ); 
	UART1PutChar( CharToHex( 0x31, datum[1] ) );
	UART1PutChar( CharToHex( 0x30, datum[1] ) );
	UART1PutChar( CharToHex( 0x31, datum[2] ) );
	UART1PutChar( CharToHex( 0x30, datum[2] ) );
	UART1PutChar( CharToHex( 0x31, datum[3] ) );
	UART1PutChar( CharToHex( 0x30, datum[3] ) );
	UART1PutChar( CharToHex( 0x31, datum[5] ) );
	UART1PutChar( CharToHex( 0x30, datum[5] ) );
	UART1PutChar( CharToHex( 0x31, datum[6] ) );
	UART1PutChar( CharToHex( 0x30, datum[6] ) );
	UART1PutChar( CharToHex( 0x31, datum[8] ) );
	UART1PutChar( CharToHex( 0x30, datum[8] ) );
	UART1PutChar( CharToHex( 0x31, datum[9] ) );
	UART1PutChar( CharToHex( 0x30, datum[9] ) );
	//PC: taegliche Messung
	UART1PutChar( UnsignedIntToHex( 0x33, taeglicheMessung ) );
	UART1PutChar( UnsignedIntToHex( 0x32, taeglicheMessung ) );
	UART1PutChar( UnsignedIntToHex( 0x31, taeglicheMessung ) );
	UART1PutChar( UnsignedIntToHex( 0x30, taeglicheMessung ) );
	//PC: Messtyp
	UART1PutChar( CharToHex( 0x31, measurementType ) );
	UART1PutChar( CharToHex( 0x30, measurementType ) ); 
	//PC: Startzeit
	UART1PutChar( CharToHex( 0x31, uhrzeitLetzterLauf[0] ) );
	UART1PutChar( CharToHex( 0x30, uhrzeitLetzterLauf[0] ) );
	UART1PutChar( CharToHex( 0x31, uhrzeitLetzterLauf[1] ) );
	UART1PutChar( CharToHex( 0x30, uhrzeitLetzterLauf[1] ) );
	UART1PutChar( CharToHex( 0x31, uhrzeitLetzterLauf[3] ) );
	UART1PutChar( CharToHex( 0x30, uhrzeitLetzterLauf[3] ) );
	UART1PutChar( CharToHex( 0x31, uhrzeitLetzterLauf[4] ) );
	UART1PutChar( CharToHex( 0x30, uhrzeitLetzterLauf[4] ) );
	//PC: Zeitpunkt Ende
	UART1PutChar( CharToHex( 0x31, uhrzeitAktuell[0] ) );
	UART1PutChar( CharToHex( 0x30, uhrzeitAktuell[0] ) );
	UART1PutChar( CharToHex( 0x31, uhrzeitAktuell[1] ) );
	UART1PutChar( CharToHex( 0x30, uhrzeitAktuell[1] ) );
	UART1PutChar( CharToHex( 0x31, uhrzeitAktuell[3] ) );
	UART1PutChar( CharToHex( 0x30, uhrzeitAktuell[3] ) );
	UART1PutChar( CharToHex( 0x31, uhrzeitAktuell[4] ) );
	UART1PutChar( CharToHex( 0x30, uhrzeitAktuell[4] ) );
	//PC: Kalibrationsfläche
	UART1PutChar( UnsignedLongIntToHex( 0x37, calFlaeche ) );
	UART1PutChar( UnsignedLongIntToHex( 0x36, calFlaeche ) );
	UART1PutChar( UnsignedLongIntToHex( 0x35, calFlaeche ) );
	UART1PutChar( UnsignedLongIntToHex( 0x34, calFlaeche ) );
	UART1PutChar( UnsignedLongIntToHex( 0x33, calFlaeche ) );
	UART1PutChar( UnsignedLongIntToHex( 0x32, calFlaeche ) );
	UART1PutChar( UnsignedLongIntToHex( 0x31, calFlaeche ) );
	UART1PutChar( UnsignedLongIntToHex( 0x30, calFlaeche ) );
	//PC: Eichgasmenge
	UART1PutChar( UnsignedShortToHex( 0x31, eichgasmenge ) );
	UART1PutChar( UnsignedShortToHex( 0x30, eichgasmenge ) );
	//PC: Messgasfläche
	UART1PutChar( LongIntToHex( 0x37, integral ) );
	UART1PutChar( LongIntToHex( 0x36, integral ) );
	UART1PutChar( LongIntToHex( 0x35, integral ) );
	UART1PutChar( LongIntToHex( 0x34, integral ) );
	UART1PutChar( LongIntToHex( 0x33, integral ) );
	UART1PutChar( LongIntToHex( 0x32, integral ) );
	UART1PutChar( LongIntToHex( 0x31, integral ) );
	UART1PutChar( LongIntToHex( 0x30, integral ) );
	//PC: Messergebnis
	UART1PutChar( UnsignedIntToHex( 0x33, messergebnis ) );
	UART1PutChar( UnsignedIntToHex( 0x32, messergebnis ) );
	UART1PutChar( UnsignedIntToHex( 0x31, messergebnis ) );
	UART1PutChar( UnsignedIntToHex( 0x30, messergebnis ) );
	//PC: Peakstart
	UART1PutChar( IntToHex( 0x33, peakStart ) );
	UART1PutChar( IntToHex( 0x32, peakStart ) );
	UART1PutChar( IntToHex( 0x31, peakStart ) );
	UART1PutChar( IntToHex( 0x30, peakStart ) );
	//PC: Peakmitte
	UART1PutChar( IntToHex( 0x33, peakMitte ) );
	UART1PutChar( IntToHex( 0x32, peakMitte ) );
	UART1PutChar( IntToHex( 0x31, peakMitte ) );
	UART1PutChar( IntToHex( 0x30, peakMitte ) );
	//PC: Peakende
	UART1PutChar( IntToHex( 0x33, peakEnde ) );
	UART1PutChar( IntToHex( 0x32, peakEnde ) );
	UART1PutChar( IntToHex( 0x31, peakEnde ) );
	UART1PutChar( IntToHex( 0x30, peakEnde ) );
	//PC: PeakstartC
	UART1PutChar( IntToHex( 0x33, peakStartC ) );
	UART1PutChar( IntToHex( 0x32, peakStartC ) );
	UART1PutChar( IntToHex( 0x31, peakStartC ) );
	UART1PutChar( IntToHex( 0x30, peakStartC ) );
	//PC: PeakmitteC
	UART1PutChar( IntToHex( 0x33, peakMitteC ) );
	UART1PutChar( IntToHex( 0x32, peakMitteC ) );
	UART1PutChar( IntToHex( 0x31, peakMitteC ) );
	UART1PutChar( IntToHex( 0x30, peakMitteC ) );
	//PC: PeakendeC
	UART1PutChar( IntToHex( 0x33, peakEndeC ) );
	UART1PutChar( IntToHex( 0x32, peakEndeC ) );
	UART1PutChar( IntToHex( 0x31, peakEndeC ) );
	UART1PutChar( IntToHex( 0x30, peakEndeC ) );
	//PC: Kalibrationsreportnummer
	UART1PutChar( UnsignedLongIntToHex( 0x37, kalReportNr ) );
	UART1PutChar( UnsignedLongIntToHex( 0x36, kalReportNr ) );
	UART1PutChar( UnsignedLongIntToHex( 0x35, kalReportNr ) );
	UART1PutChar( UnsignedLongIntToHex( 0x34, kalReportNr ) );
	UART1PutChar( UnsignedLongIntToHex( 0x33, kalReportNr ) );
	UART1PutChar( UnsignedLongIntToHex( 0x32, kalReportNr ) );
	UART1PutChar( UnsignedLongIntToHex( 0x31, kalReportNr ) );
	UART1PutChar( UnsignedLongIntToHex( 0x30, kalReportNr ) );
	//PC: Ende
	UART1PutChar( 0x0D );
	UART1PutChar( 0x0A );
}
/*******************************************************************
 * Function:        UARTSendPC
 *******************************************************************/
void UARTSendInfo( char c, int i )
{
	//PC: Bezeichner
	UART1PutChar( c );
	//PC: Seriennummer
	UART1PutChar( IntToHex( 0x33, i ) );
	UART1PutChar( IntToHex( 0x32, i ) );
	UART1PutChar( IntToHex( 0x31, i ) );
	UART1PutChar( IntToHex( 0x30, i ) );
	//PC: Ende
	UART1PutChar( 0x0D );
	UART1PutChar( 0x0A );
}
/*******************************************************************
 * Function:        UARTGetChar
 *******************************************************************/
char UARTGetChar( void )
{
	char temp;
	while( IFS1bits.U2RXIF == 0 );
	temp				=	U2RXREG;
	IFS1bits.U2RXIF		=	0;
	return temp;
}

/*******************************************************************
 * Function:        UART1GetChar
 *******************************************************************/
char UART1GetChar( void )
{
	char temp;
	while( IFS0bits.U1RXIF == 0 );
	temp				=	U1RXREG;
	IFS0bits.U1RXIF		=	0;
	return temp;
}

/*******************************************************************
 * Function:        UART1GetPCString
 *******************************************************************/
void UART1GetPCString( void )
{
	while( IFS0bits.U1RXIF == 0 );
	pcString[0]			=	U1RXREG;
	pcString[1]			=	U1RXREG;
	pcString[2]			=	U1RXREG;
	pcString[3]			=	U1RXREG;
	IFS0bits.U1RXIF		=	0;
}

/*******************************************************************
 * Function:        UARTInit
 *******************************************************************/
void UARTInit( int baudRate )
{
	U2BRG				=	baudRate;
	U2MODE				=	0x8000;
	U2STA				=	0x0400;

	RPINR19bits.U2RXR	=	10;
	RPOR8bits.RP17R		=	5;

	IFS1bits.U2RXIF		=	0;
}

/*******************************************************************
 * Function:        UART1Init
 *******************************************************************/
void UART1Init( int baudRate )
{
	U1BRG				=	baudRate;
	U1MODE				=	0x8000;
	U1STA				=	0x0400;

	RPINR18bits.U1RXR	=	41;
	RPOR15bits.RP30R	=	3;

	IFS0bits.U1RXIF		=	0;
}

/*******************************************************************
 * Function:        UARTPutChar
 *******************************************************************/
void UARTPutChar( char c )
{
	while( U2STAbits.UTXBF == 1 );
	U2TXREG				=	c;
}

/*******************************************************************
 * Function:        UART1PutChar
 *******************************************************************/
void UART1PutChar( char c )
{
	while( U1STAbits.UTXBF == 1 );
	U1TXREG				=	c;
}

/*******************************************************************
 * Function:        UARTSendString
 *******************************************************************/
void UARTSendString( char* s, int length, int type )
{
	int i				=	0;
	unsigned short bcc	=	0;

	//ERST TYP SENDEN...
	if( type == 1 )
	{
		LATFbits.LATF13		=	1;
		while( PORTFbits.RF12 == 0 );
		UARTPutChar( 0x31 );
		bcc				+=	1;
	}
	else if( type == 2 )
	{
		LATFbits.LATF13		=	1;
		while( PORTFbits.RF12 == 0 );
		UARTPutChar( 0x32 );
		bcc				+=	2;
	}

	//...DANN STRING
	LATFbits.LATF13		=	1;
	for( i = 0; i < length; i++ )
	{
		while( PORTFbits.RF12 == 0 );
		UARTPutChar( s[i] );
		bcc				+=	s[i];
	}
/*
	//CHECKSUMME SENDEN
	while( PORTFbits.RF12 == 0 );
	UARTPutChar( ( char ) bcc );
*/
	while( PORTFbits.RF12 == 0 );
	UARTPutChar( 0x00 );
}

/********************************************************************
 Function:        void USBCB_SOF_Handler(void)
 *******************************************************************/
void USBCB_SOF_Handler(void)
{
}

/*******************************************************************
 * Function:        void USBCBCheckOtherReq(void)
 *******************************************************************/
void USBCBCheckOtherReq(void)
{
    USBCheckMSDRequest();
}//end

/*******************************************************************
 * Function:        void USBCBErrorHandler(void)
 *******************************************************************/
void USBCBErrorHandler(void)
{
}

/*******************************************************************
 * Function:        void USBCBInitEP(void)
 *******************************************************************/
void USBCBInitEP(void)
{
    #if (MSD_DATA_IN_EP == MSD_DATA_OUT_EP)
        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    #else
        USBEnableEndpoint(MSD_DATA_IN_EP,USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
        USBEnableEndpoint(MSD_DATA_OUT_EP,USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
    #endif

    USBMSDInit();
}

/********************************************************************
 * Function:        void USBCBSendResume(void)
 *******************************************************************/
void USBCBSendResume(void)
{
    static WORD delay_count;
    
    USBResumeControl = 1;                // Start RESUME signaling
    
    delay_count = 1800U;                // Set RESUME line for 1-13 ms
    do
    {
        delay_count--;
    }while(delay_count);
    USBResumeControl = 0;
}

/*******************************************************************
 * Function:        void USBCBStdSetDscHandler(void)
 *******************************************************************/
void USBCBStdSetDscHandler(void)
{
    // Must claim session ownership if supporting this request
}//end

/******************************************************************************
 * Function:        void USBCBSuspend(void)
 *****************************************************************************/
void USBCBSuspend(void)
{
    #if defined(__C30__)
    #if 0
        U1EIR = 0xFFFF;
        U1IR = 0xFFFF;
        U1OTGIR = 0xFFFF;
        IFS5bits.USB1IF = 0;
        IEC5bits.USB1IE = 1;
        U1OTGIEbits.ACTVIE = 1;
        U1OTGIRbits.ACTVIF = 1;
        Sleep();
    #endif
    #endif
}

/******************************************************************************
 * Function:        void USBCBWakeFromSuspend(void)
 *****************************************************************************/
void USBCBWakeFromSuspend(void)
{
}

/*******************************************************************
 * Function:        BOOL USER_USB_CALLBACK_EVENT_HANDLER(
 *******************************************************************/
BOOL USER_USB_CALLBACK_EVENT_HANDLER(USB_EVENT event, void *pdata, WORD size)
{
    switch(event)
    {
        case EVENT_CONFIGURED: 
            USBCBInitEP();
            break;
        case EVENT_SET_DESCRIPTOR:
            USBCBStdSetDscHandler();
            break;
        case EVENT_EP0_REQUEST:
            USBCBCheckOtherReq();
            break;
        case EVENT_SOF:
            USBCB_SOF_Handler();
            break;
        case EVENT_SUSPEND:
            USBCBSuspend();
            break;
        case EVENT_RESUME:
            USBCBWakeFromSuspend();
            break;
        case EVENT_BUS_ERROR:
            USBCBErrorHandler();
            break;
        case EVENT_TRANSFER:
            Nop();
            break;
        default:
            break;
    }      
    return TRUE; 
}

/** EOF main.c ***************************************************************/

