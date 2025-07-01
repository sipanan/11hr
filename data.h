#ifndef DATA_H
#define DATA_H

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

void DataInitRead( char* date );
void DataGetPreviousDay( void );
void DataShowDayResults( unsigned int pageNo );
void DataShowGraphResults( void );
void DataShowMeasurement( unsigned int measurementNo );
BOOL DataIsLeapYear( void );

#endif

//EOF

