#ifndef I2C_H
#define I2C_H
#endif

#define DELAY					150
#define DELAY2					500

#define DISPLAY_ADDRW			0xDC
#define DISPLAY_ADDRR			0xDD
#define CLOCK_CONTROL_ADDRW		0xDE
#define CLOCK_CONTROL_ADDRR		0xDF
#define SYSTEM_ADDRW			0x10
#define SYSTEM_ADDRR			0x11

#define CLOCK_STATUS_REG		0x3F
#define CLOCK_SECONDS_REG		0x30
#define CLOCK_MINUTES_REG		0x31
#define CLOCK_HOURS_REG			0x32
#define CLOCK_DAY_REG			0x33
#define CLOCK_MONTH_REG			0x34
#define CLOCK_YEAR_REG			0x35
#define CLOCK_DOTW_REG			0x36
#define CLOCK_Y2K_REG			0x37

#define CLOCK_EEPROM_ADDRW		0xAE
#define CLOCK_EEPROM_ADDRR		0xAF

#define CLOCK_MEMORY_MESSORT	0x40
#define CLOCK_MEMORY_UEBERTRAG	0x4A
#define CLOCK_MEMORY_SKALIERUNG	0x4B
#define CLOCK_MEMORY_MEASCOUNT	0x4C

#define CLOCK_MEMORY_EICHGAS	0x50
#define CLOCK_MEMORY_MENGE		0x52
#define CLOCK_MEMORY_VOLUMEN	0x54
#define CLOCK_MEMORY_TEMP		0x55
#define CLOCK_MEMORY_DRUCK		0x56
#define CLOCK_MEMORY_EMPFINDLKT	0x57
#define CLOCK_MEMORY_MESSZYKLUS	0x58
#define CLOCK_MEMORY_EICHZYKLUS	0x59
#define CLOCK_MEMORY_SPUELZEIT	0x5B
#define CLOCK_MEMORY_KORREKTUR	0x5C
#define CLOCK_MEMORY_FLUSS		0x5D
#define CLOCK_MEMORY_AUTOMEAS	0x60
#define CLOCK_MEMORY_DAYMEAS	0x5F
#define CLOCK_MEMORY_SPLITZEIT	0x61
#define CLOCK_MEMORY_KALREPORT	0x62
#define CLOCK_MEMORY_ERRORLOG	0x65

void I2CInit( int BRG );
void I2CResetBus( void );
void I2CStart( void );
void I2CStop( void );
void I2CRepStart( void );
int I2CSendByte( char data );
void I2CWriteByte( char addr, char data );
void I2CWriteNBytes( char addr, char* data, int cnt );
char I2CReadByte( int n );
void I2CReadBytePointer( char* c, int i );

/** EOF i2c.h ***************************************************************/
