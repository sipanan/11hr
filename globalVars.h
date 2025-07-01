#ifndef GLOBALVARS_H
#define GLOBALVARS_H

#include "GenericTypeDefs.h"

// BOOLs
extern BOOL startup;
extern BOOL checkMasterPwd;
extern BOOL checkPwd;
extern BOOL volatile deviceActive;
extern BOOL volatile deviceStarted;
extern BOOL fileCopiedRefresh;
extern BOOL checkValue;
extern BOOL checkPoll;
extern BOOL initResults;
extern BOOL drawgraph;
extern BOOL meas6min;
extern BOOL startAutoMeas;
extern BOOL autoMeasurement;
extern BOOL calmeas;
extern BOOL cal;
extern BOOL meas;
extern BOOL saveData;
extern BOOL uebertragungsart;
extern BOOL battery_low;
extern BOOL battery_recharged;
extern BOOL malfunction;
extern BOOL lockSystem;
extern BOOL copyAll;
extern BOOL syringe;
extern BOOL IsMASet;
extern BOOL measurementNotOkay;

// Messdaten & Status
extern unsigned long int meascount;
extern unsigned long int errorcount;
extern unsigned long int calFlaeche;
extern unsigned long int kalReportNr;
extern long int __attribute__((far)) integral;
extern unsigned int messergebnis;
extern unsigned int sekunden;
extern unsigned int displayWait;
extern unsigned int checkFlussCount;
extern unsigned short malfunctionCount;
extern unsigned long int calFlaecheMin;


extern int defaultTemp;
extern int defaultFlow;
extern int defaultSensibility;
extern int defaultPressure;
extern int defaultSplittime;


extern char __attribute__((far)) current_date[25];
extern unsigned char __attribute__((far)) readDirectory[21];
extern unsigned char __attribute__((far)) buffer[6];
extern unsigned short __attribute__((far)) messpunkte[240];
extern char __attribute__((far)) messort[9];
extern unsigned short eichgasmenge;
extern unsigned short empfindlichkeit_vorgabe;
extern unsigned int signal;
extern unsigned int baselineConstant;
extern unsigned short skalierung;
extern unsigned short volatile temperatur;
extern unsigned short saeule_istwert;
extern unsigned int saeule_vorgabe;
extern int flaeche_nr;
extern int counter;
extern unsigned short vordruck_istwert;
extern unsigned short fluss_istwert;
extern unsigned short empfindlichkeit;

extern unsigned short volatile system_status;
extern unsigned short akku;
extern int messdatum;
extern int __attribute__((far)) messdaten[720];

extern unsigned char __attribute__((far)) recentValue[8];
extern unsigned char __attribute__((far)) previousValue[8];
extern int *messdatenptr;
extern int *messdatenende;
extern char measurementType;
extern unsigned int taeglicheMessung;

extern char __attribute__((far)) uhrzeitLetzterLauf[6];
extern char __attribute__((far)) uhrzeitNaechsteMessung[6];
extern char __attribute__((far)) uhrzeitNaechsteKalib[6];
extern char __attribute__((far)) uhrzeitAktuell[6];
extern unsigned int calcMinRec;
extern unsigned int calcHourRec;
extern unsigned int calcMinNext;
extern unsigned int calcHourNext;

extern float numberOfFiles;
extern float percentPerFile;
extern unsigned char percentComplete;
extern unsigned int filesNo;
extern unsigned short filesCopiedRefresh;
extern unsigned int filesTotal;
extern unsigned int filesCopied;

extern unsigned int mA1;
extern unsigned int mA2;
extern unsigned int pageNo;
extern unsigned int dateiNo;
extern unsigned int recentFileNo;

extern int __attribute__((far)) peakStart;
extern int __attribute__((far)) peakMitte;
extern int __attribute__((far)) peakEnde;
extern int __attribute__((far)) peakStartC;
extern int __attribute__((far)) peakMitteC;
extern int __attribute__((far)) peakEndeC;

extern unsigned int errorCode;
extern char pcString[4];
extern int timecountaux;

// Funktionen
void InitVariables(void);
void SystemClearActiveStarted(void);
void SystemResetMesspunkte(void);

#endif
