/***************************************************************************
                          ctoolbox.cpp  -  description                              
                             -------------------                                         
    begin                : Wed Aug 4 1999                                           
    copyright            : (C) 1999 by Thorsten Janke                         
    email                : janke@studST.fh-muenster.de                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/


#include "ctoolbox.h"

CToolBox::CToolBox()
{
}


CToolBox::~CToolBox()
{
}

/** CToolBox::filter_tp( char... )
  * filter_tp chSrc to chDest with nSize and
  * feedback coefficient fC2
  */
void CToolBox::filter_tp( char* chDest, char* chSrc, int nSize, float fC2){

  #if 1
  int nRun;
   unsigned char fData;;
   unsigned char fFilterData;
  for ( nRun= 0; nRun< nSize; nRun++ ) {
      fData = (unsigned char) chSrc[nRun];
      fFilterData = (unsigned char) chDest[nRun];
      fFilterData = (fData >> 1)
               + (fFilterData>>1);
      chDest[nRun] = (int)fFilterData;			
   }
  #else
  int nRun;
  float fData, fFilterData;
		
  for ( nRun= 0; nRun< nSize; nRun++ ) {
      fData = (unsigned char) chSrc[nRun];
      fFilterData = (unsigned char) chDest[nRun];
      fFilterData = (1- fC2)* fData
                     + fC2* fFilterData;
      if ( fFilterData > 255 )
        fFilterData = 255;
      chDest[nRun] = (int)fFilterData;			
  }
  #endif
}

/**	returns average of chSrc with size nSize  */
int CToolBox::getAverage( char* chSrc, int nSize ){

  float fSum = 0;

  for( int i=0; i< nSize; i++ )
    fSum += (unsigned char)chSrc[i];

  fSum = fSum / (nSize) ;

 return (int)fSum;

}

/** filter one byte */
void CToolBox::filter_tp( char* chDest, char* chSrc, float fC2 ){

  #if 1
   unsigned char fData = (unsigned char) *chSrc;
   unsigned char fFilterData = (unsigned char) *chDest;
   fFilterData = (fData >> 1)
               + (fFilterData>>1);
   *chDest = fFilterData;			
  #else
  float fData, fFilterData;
		
   fData = (unsigned char) *chSrc;
   fFilterData = (unsigned char) *chDest;
   fFilterData = (1- fC2)* fData
               + fC2* fFilterData;
   if ( fFilterData > 255 )
      fFilterData = 255;
   *chDest = (int)fFilterData;			
   #endif
}
