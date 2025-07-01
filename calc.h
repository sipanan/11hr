#ifndef CALC_H
#define CALC_H

void AbleitungInit( void );
void Ableitung1Calc( long double value );
void Ableitung2Calc( long double value );
void AbleitungAb0( int type );
void AbleitungAb360( void );
void Integrate( int type );

//NEUE ABLEITUNG NOVEMBER 2020 WEGEN DEM OFTMALS KLEINEN HUBBEL VORNE WEG
void AbleitungInit_neu(void);
void AbleitungCalc(char type);
void IntegralCalc(void);
double Regression(int index, int n);
void Integrate_neu(int type);

#endif

//EOF

