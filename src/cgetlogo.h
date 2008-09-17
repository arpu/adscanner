/***************************************************************************
                          cgetlogo.h  -  description                              
                             -------------------                                         
    begin                : Fri Aug 6 1999                                           
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


#ifndef CGETLOGO_H
#define CGETLOGO_H

/**analyse corner and set test points
  *@author Thorsten Janke
  */
#include "noaddata.h"

class CControl;
class CGetLogo
{
	int CornerID;
	CControl *caller;
public:
	
  /** constructor gets the doc pointer, the pointer to the data of corner
    * and the pointer to the split view
    */
  CGetLogo( CControl* caller, noadData* data, char* chRunData,
 int iCornerID );
  /** destructor of this class */
  ~CGetLogo();
  /** called from parent if new data are available
	  * distribute functions
    */
  void newData();
  /** returns true if user condition for picture change ok */
  bool checkBlock( char* chNew, char* chRef );
  /** reset all buffers for a new run */
  void reset( bool bFirst = false );
  /** set the testlines of src data to testline struct */
  int setTestlines( char* chMask );
  /** called to calculate the numerate diversions of two neighb. points */
  void setDiversions( char* chSrc );
  /** called to check the diversions in updated picture and to update the mask */
  int updateDiversions( char* chMask, char* chSrc );
  /** called to get a new allocated testline */
  testlines* new_testline( int line );
  /** pointer to the Data-Object */
  noadData* m_pData;
  /** main buffer of tp filtered Data */
  char* m_chFilterData;
  /** buffer of last accepted picture */
  char* m_chRefPicture;
  /** buffer of running soft filtered grey data */
  char* m_chRunData;
  /** buffer of running soft filtered color data */
  char* m_chRunColorData;
  /** counter of accepted corners */
  int m_nFilteredCorner;
  /** counter of check corner */
  int m_nCheckedCorner;
  /** testpoints are drawn in this data buffer  */
  char* m_chTestViewData;
  /** the rest test points */
  int m_nPointsLeft;
  /** the first testline element */
  testlines* m_linehook;

  /** signal emitted if logo has found */
	void foundLogo( struct testlines* );


};

#endif




















































