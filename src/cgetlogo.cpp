/***************************************************************************
                          cgetlogo.cpp  -  description                              
                             -------------------                                         
    begin                : Fri Aug 6 1999                                           
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

#include <stdlib.h>
#include <stdio.h>
#include "cgetlogo.h"
#include "ccontrol.h"
#include "ctoolbox.h"

#include <string.h>

logocbfunc logocb = NULL;

CGetLogo::CGetLogo( CControl* _caller, noadData* data, char* chRunData, int iCornerID )
{
  caller = _caller;
  /** set the data pointer */
  m_pData = data;

  /** allocate memory for reference corner */
  m_chRefPicture = new char[m_pData->m_nSizeX*m_pData->m_nSizeY];
  /** allocate memory for main filter corner */
  m_chFilterData = new char[m_pData->m_nSizeX*m_pData->m_nSizeY];
  /** allocate memory for test view  corner */
  m_chTestViewData = new char[m_pData->m_nSizeX*m_pData->m_nSizeY];

  // hook of the testpoint list
  m_linehook = NULL;
  // set pointer to the considered corner data
  m_chRunData = chRunData;

  reset( true );

  CornerID = iCornerID;

}

CGetLogo::~CGetLogo()
{
  delete [] m_chFilterData;
  delete [] m_chRefPicture;
  delete [] m_chTestViewData;
  m_chFilterData=NULL;
  m_chRefPicture=NULL;
  m_chTestViewData=NULL;
}


/** called from parent if new data are available
  * distribute functions
  */
void CGetLogo::newData()
{
  CToolBox tool;

  /** check if new corner is accepted */
  if ( checkBlock( m_chRunData, m_chRefPicture ) )
  {
      /** check if there are enough filtered corners */
      if ( m_nFilteredCorner < m_pData->m_nFilterFrames )
      {
         tool.filter_tp( m_chFilterData, m_chRefPicture,
                         m_pData->m_nSizeX*m_pData->m_nSizeY, m_pData->m_fMainFilter );
         m_nFilteredCorner++;
      }
      else
      {
         m_nCheckedCorner++;
         /** first time set testlines */
         if ( m_nCheckedCorner == 1 )
            setDiversions( m_chFilterData );	
         /** check testlines */
         if ( m_nCheckedCorner <= m_pData->m_nCheckFrames )
         {
            m_nPointsLeft = updateDiversions( m_chFilterData, m_chRefPicture );
            if ( m_nPointsLeft <= 20 )
               reset();  // not enough -> once again
         }
         if ( m_nPointsLeft > 20 && m_nCheckedCorner == m_pData->m_nCheckFrames )
         { 
             if ( setTestlines( m_chFilterData ) < 10 )
             {
                 reset();  // not enough -> once again
             }
             else
             	caller->foundLogo(m_linehook,CornerID);
//!!!                 emit foundLogo( m_linehook ); // gratulation -> the logo has found
         }
      }
      if( logocb != NULL )
      {
        // set results to view
        //sprintf( m_pView->m_strHeadline, "found %d couples", nCouples );
        //m_pView->update();
        char *buffer = NULL;
//        asprintf(&buffer,"checked %d corners", m_nCheckedCorner);
        logocb(CornerID,buffer,m_chTestViewData,m_pData->m_nSizeX,m_pData->m_nSizeY);
        delete buffer;
      }
  }
}

/** reset all buffers for a new run */
void CGetLogo::reset( bool bFirst )
{
  /** set the buffers to init values */
  memcpy( m_chRefPicture, m_chRunData, m_pData->m_nSizeX*m_pData->m_nSizeY );
  memset( m_chFilterData, m_pData->m_nFilterInit, m_pData->m_nSizeX*m_pData->m_nSizeY );
  memcpy( m_chTestViewData, m_chFilterData, m_pData->m_nSizeX*m_pData->m_nSizeY );

  m_nFilteredCorner = 0;
  m_nCheckedCorner = 0;

  // if we have set the testlines, delete the allocated memory
  if ( m_linehook )
     m_pData->deleteTestlines( &m_linehook );

  m_linehook = NULL;

}

/** returns true if user condition for picture change ok */
bool CGetLogo::checkBlock( char* chNew, char* chRef )
{
  int i, nAverage;
  float fTemp = 0;
  CToolBox tool;

  /** get Average of new picture */
  nAverage = tool.getAverage( chNew, m_pData->m_nSizeX*m_pData->m_nSizeY );
	
  /** check maximum average ( dont set very bright blocks ) */
  if ( nAverage > m_pData->m_nMaxAverage )
    return false;

  /** check minimum average ( dont set to dark blocks ) */
  if ( nAverage < m_pData->m_nMinAverage )
    return false;

  /** check the difference between ref picture and new picture
    * set the ref picture only if the dif > dif2pictures set as % in doc
    */
  for ( i = 0; i < m_pData->m_nSizeX*m_pData->m_nSizeY; i++ )
      fTemp+= abs((unsigned char)chNew[i] - (unsigned char)chRef[i]);

  if ( fTemp <= m_pData->m_nSizeX*m_pData->m_nSizeY*2.55*m_pData->m_nDif2Pictures )
     return false;

	
  /** this block is ok */
  memcpy( chRef, chNew, m_pData->m_nSizeX*m_pData->m_nSizeY );
	
 return true;

}



/** set the testlines of mask data to testline struct */
int CGetLogo::setTestlines( char* chMask )
{
  testlines* templine=NULL;
  testlines* newline=NULL;
  testpair*  newpair=NULL;
  testpair*  temppair=NULL;
  // ** get the size of corner ***
  int sizeY = m_pData->m_nSizeY;
  int sizeX = m_pData->m_nSizeX;

  int nCouples = 0;
  bool bFound = false;
  int x,y;
  int l2r, xpos=-1, xneg=-1;

  // ** get all lines down and set available testpairs to lines **
  for ( y=0; y<sizeY; y++ )
  {
    newline = NULL;
    l2r=0;
    for ( x=0; x<sizeX-1; x++ )
    {
       xpos = 0;
       if ( chMask[y*sizeX+x] == 1 ) // check this
       {
            xpos = x; xneg = 0;
            // ** get next negative diversion ***
            while ( l2r < sizeX-1 && !bFound )
            {
                 if ( chMask[y*sizeX+l2r] == 2 )
                 {
                     xneg = l2r;
                     bFound = true;
                 }
                 l2r++;
            }
            bFound = false;
       }
       // ** if we have found a pair, so set it to the line **
       if ( xpos >0 && xneg >0 )
       {
           // ** create the first line if not available
           nCouples++;
           if ( !newline )
              newline = new_testline( y );
           // ** set the struct of found test pair *******
           newpair = new testpair[sizeof(struct testpair)];
           newpair->next = NULL;
           newpair->x_pos = xpos;
           newpair->x_neg = xneg; 	

           // ** keep this line ***********
           templine = newline;
           // ** check the first pair of line
           if ( !templine->pair )	
                templine->pair = newpair;
           else {
              temppair = templine->pair;
              // ** go to the end ***********	
              while ( temppair->next )
                  temppair = temppair->next;
              // ** set the new pair to the end
              temppair->next = newpair;
           }					
       }				
  	}
    // ** the first line including pairs ??
    if ( !m_linehook && newline )
        m_linehook = newline;
    else
       // ** check if there is a line including pairs
       if ( newline )
       {
          templine = m_linehook;
          // ** one time again - go to the end
          while ( templine->next )
              templine = templine->next;
          // ** here we are *********
          templine->next = newline;
       }
  }

  if( logocb != NULL )
  {
    // set results to view
    //sprintf( m_pView->m_strHeadline, "found %d couples", nCouples );
    //m_pView->update();
    logocb(CornerID,0,m_chTestViewData,m_pData->m_nSizeX,m_pData->m_nSizeY);
  }
  return nCouples;
}

/** called to calculate the numerate diversions of two neighb. points */
void CGetLogo::setDiversions( char* chSrc )
{
  int x, y;
  int sizeX = m_pData->m_nSizeX;
  int sizeY = m_pData->m_nSizeY;
  float fTemp;

  // ** get diversion of two points and set them to chSrc
  // ** set only direction no: 0, pos: 1, neg:2
  for ( y=0; y<sizeY; y++ )
  {
     for ( x=0; x<sizeX-2; x++ )
     {
         int offset = y*sizeX;
         fTemp = (unsigned char)chSrc[offset+x+2]  // check this
                -(unsigned char)chSrc[offset+x];
         if ( fTemp > 0 )
            chSrc[offset+x] = 1;
         if ( fTemp < 0 )
            chSrc[offset+x] = 2;
         if ( fTemp == 0 )
            chSrc[offset+x] = 0;	
     }
  }

}

/** called to check the diversions in updated picture and to update the mask */
int CGetLogo::updateDiversions( char* chMask, char* chSrc )
{
  int x,y,nPointsLeft = 0;
  int sizeX = m_pData->m_nSizeX;
  int sizeY = m_pData->m_nSizeY;
  float fTemp;
  int mask;

  // ** copy updated picture to the view data **
  memcpy( m_chTestViewData, chSrc, sizeX*sizeY );

  for ( y=0; y<sizeY; y++ )
  {
     for ( x=0; x<sizeX-2; x++ )
     {
         int offset = y*sizeX;
         mask = chMask[offset+x];
         // ** update only diversion if point bright is smaller than 200
         if ( (unsigned char)chSrc[offset+x+2] < 200 && // check this
              (unsigned char)chSrc[offset+x] < 200 )
         {
           if ( mask == 1 )
           {
               fTemp = (unsigned char)chSrc[offset+x+2]
                      -(unsigned char)chSrc[offset+x];
               if ( fTemp <= 0 )
                  chMask[offset+x] = 0;
               else
                  nPointsLeft++;
           }
           if ( mask == 2 )
           {
              fTemp = (unsigned char)chSrc[offset+x+2]
                     -(unsigned char)chSrc[offset+x];
              if ( fTemp >= 0 )
                    chMask[offset+x] = 0;
              else
                  nPointsLeft++;
           }
         }
         else
            if ( mask > 0 )
               nPointsLeft++;
     }
  }

  // ** set the testpoints to the view **
  for ( y=0; y<sizeY; y++ )
  {
     for ( x=0; x<sizeX-2; x++ ) {
         if ( (unsigned char)chMask[y*sizeX+x] != 0 )
               m_chTestViewData[y*sizeX+x] = 255;     // check this
               //m_chTestViewData[y*sizeY+x] = 255;     // check this
     }
  }
 return nPointsLeft;

}

/** called to get a new allocated testine */
testlines* CGetLogo::new_testline( int line )
{
  // ** allocate new memory for a testline *****************
  testlines* temp = new testlines[sizeof(struct testlines)];
	
  temp->line = line;
  temp->next = NULL;
  temp->pair = NULL;

 return temp;
}
