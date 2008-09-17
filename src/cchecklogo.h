/***************************************************************************
                          cchecklogo.h  -  description                              
                             -------------------                                         
    begin                : Sat Aug 7 1999                                           
    copyright            : (C) 1999 by Thorsten Janke                         
    email                : janke@studST.fh-muenster.de   
                         : http://www.ktet.fh-muenster.de/ina                 
            
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef CCHECKLOGO_H
#define CCHECKLOGO_H
#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdio.h>

/**
  *@author Thorsten Janke
  */
//#include "mynoaddoc.h"
//#include "noaddata.h"
//#include "cchecklogoview.h"
class noadData;

/** The CCheckLogo class is able to check the status of tv logo
  * emittes the logo, nologo and unknown signal
  */
class CCheckLogo
{
public:
	/** the constructor of the class gets the document pointer doc
    * and the pointer to the main view
    */
  CCheckLogo( noadData* data );
  /** destructor */
  ~CCheckLogo();
   /** called from parent object to show that there are new data available */
  void newData();
  /** called to delete possible testlines */
  void reset();

  void setLineHook( struct testlines* );
  void setCornerData( char* );
  bool isLogo;

  testlines* new_testline( int line );
  void save( FILE *fd );
  void log();
  void load( FILE *fd );
  void setTreshold(int i ) { iTreshold = i; }
  int getTreshold() { return iTreshold; }
  void getLogoRect(int &left, int &top, int &right, int &bottom);
private:
  /** check all reference diversions in the updated picture,
    * decide the logo status at the moment and emit the membership signal
    */
  int iLineOffset;
  int iXOffset;
  int checkTestlines( char* chSrc, struct testlines*	 tl, int lineOffset=0, int xOffset=0 );
  /** pointer to the view */
//  CCheckLogoView* m_pView;
  /** pointer to the document */
//  MyNoadDoc* m_pDoc;
  noadData* m_pData;
  /**  temp buffer of data to show */
  char* m_chAnalyseData;
  #ifdef VNOAD
  char* m_chAnalyseData2;
  char* m_chAnalyseData3;
  #endif
  /** value of detected nologo one after another */
  int m_nNoLogo;
  /** value of detected logo one after another */
  int m_nLogo;
  /** pointer to the testpoints */
  struct testlines* m_linehook;
  /** pointer to the picture corner which is set from ccontrol */
  char* m_chCornerData;
  int iTreshold;
  
  long totalPairsOk;
  int deltaPairsOk;
  int totalChecks;

public:
  /** signal logo emitted if logo is detected */
  void logo();
  /** signal nologo emitted of no logo detected */
  void nologo();
  /** signal unknown emitted if there are not enough
    * test pairs available at the moment
    * -> the picture is to bright !
    */
  void unknown();


};

#endif
