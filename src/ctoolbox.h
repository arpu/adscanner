/***************************************************************************
                          ctoolbox.h  -  description                              
                             -------------------                                         
    begin                : Wed Aug 4 1999                                           
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


#ifndef CTOOLBOX_H
#define CTOOLBOX_H


/**
  *@author Thorsten Janke
  */

/** CToolBox contains functions for
  * filter_tp and getting average
  * of picture
  */

class CToolBox {

public:

  /** constructor */
	CToolBox();
  /** destructor */
  ~CToolBox();
  /** filter_tp chSrc to chDest with nSize and
    * feedback coefficient fC2
    */
  void filter_tp( char* chDest, char* chSrc, int nSize, float fC2);
  /** returns average of chSrc with size nSize  */
  int getAverage( char* chSrc, int nSize );	
  /** filter one byte */
  void filter_tp( char* chDest, char* chSrc, float fC2 );

};

#endif





