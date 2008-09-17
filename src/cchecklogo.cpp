/***************************************************************************
                          cchecklogo.cpp  -  description                              
                             -------------------                                         
    begin                : Sat Aug 7 1999                                           
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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cchecklogo.h"
#include "noaddata.h"

CCheckLogo::CCheckLogo( noadData* data ){

  m_pData = data;

  // allocate and set memory of view analyse data **************
  m_chAnalyseData = new char[m_pData->m_nSizeX*m_pData->m_nSizeY];
  memset(m_chAnalyseData, 0, m_pData->m_nSizeX*m_pData->m_nSizeY);
  #ifdef VNOAD
  m_chAnalyseData2 = new char[m_pData->m_nSizeX*m_pData->m_nSizeY];
  memset(m_chAnalyseData2, 255, m_pData->m_nSizeX*m_pData->m_nSizeY);
  m_chAnalyseData3 = new char[m_pData->m_nSizeX*m_pData->m_nSizeY];
  memset(m_chAnalyseData3, 128, m_pData->m_nSizeX*m_pData->m_nSizeY);
  #endif

  m_linehook = NULL;
  m_chCornerData = NULL;

/*
  //  create and set the view object ************************************
  m_pView = new CCheckLogoView( / *m_pDoc, * /m_pData, parent );
  m_pView->m_chPicture = m_chAnalyseData;
  m_pView->setGeometry( 0, 0, m_pData->m_nSizeX+20, m_pData->m_nSizeY+20 );
  m_pView->show();
*/
  // init the logo and nologo counter
  m_nLogo = 0;
  m_nNoLogo = 0;
  isLogo	= false;
  iTreshold = 1;
  iLineOffset = 0;
  totalPairsOk = 0;
  deltaPairsOk = 0;
  totalChecks = 0;
}
CCheckLogo::~CCheckLogo()
{

  delete [] m_chAnalyseData;
  m_chAnalyseData=NULL;
  #ifdef VNOAD
  delete [] m_chAnalyseData2;
  delete [] m_chAnalyseData3;
  #endif
//  delete m_pView;
  if( m_linehook )
    m_pData->deleteTestlines( &m_linehook );

}

/** void CCheckLogo::newData()
  * called from parent object to show that there are new data available
  */
void CCheckLogo::newData()
{
  if(!m_linehook)
  {
    isLogo = false;
    return;
  }
  // call the checkTestlines function to check logo status
  int okPairs = checkTestlines( m_chCornerData, m_linehook, iLineOffset, iXOffset );
  if( isLogo )
  {
    totalPairsOk += okPairs;
    totalChecks++;
    deltaPairsOk = totalPairsOk/totalChecks;
  }
  if( !isLogo && m_linehook !=  NULL && m_pData->extLogoSearch )
  {
    logocbfunc oldlogocb = logocb;
    logocb = 0;
    //syslog( LOG_INFO,"CCheckLogo::newData new lineOffset is %d over %d", iLineOffset,m_linehook->line);
    // save treschold during ex-scan
    int localTreshold = iTreshold;
    iTreshold = 0;
    int i = m_linehook->line;
    if( i < 200 )
    {
      int iLineCount = 0;
      struct testlines* tl = m_linehook;
      while(tl)
      {
        iLineCount++;
        tl = tl->next;
      }
      int iMax = m_pData->m_nSizeY - iLineCount-m_linehook->line;
      int iMaxPairs = 0;
      for( int i = -m_linehook->line; i < iMax && !isLogo; i++ )
      {
        int iPairsOk = checkTestlines( m_chCornerData, m_linehook, i );
        if( isLogo )
        {
          if( iPairsOk > iMaxPairs )
          {
            iMaxPairs = iPairsOk;
            iLineOffset = i;
            //syslog(LOG_INFO, "CCheckLogo::newData new lineOffset is %d over %d with %d pairs of %d ", iLineOffset,m_linehook->line,iMaxPairs,deltaPairsOk);
          }
        }
      }
      logocb = oldlogocb;
    }
    iTreshold = localTreshold;

  }
}

#define MIN_DIFF 5
int CCheckLogo::checkTestlines( char* chSrc, struct testlines* tl, int yoffset, int xoffset )
{
  //syslog(LOG_INFO,"CCheckLogo::checkTestlines %p %p %d %d",chSrc, tl, yoffset, xoffset);
  int sizeY = m_pData->m_nSizeY;
  int sizeX = m_pData->m_nSizeX;
  // ** copy picture to analyse view picture **
  memcpy( m_chAnalyseData, chSrc, sizeX*sizeY );
  #ifdef VNOAD
  memset(m_chAnalyseData2, 255, sizeX*sizeY);
  memset(m_chAnalyseData3, 128, sizeX*sizeY);
  #endif
  
  int nSumPair=0, nRestPair=0, nUnknown=0;
  int line;
  bool bUnknown = false, bLogo = false, bNoLogo = false;
  testpair* pair = NULL;
  float fTempPos, fTempNeg;
  // ** do it while lines available
  while ( tl ) {
    line = tl->line;
    pair = tl->pair;
    while ( pair )
    {
      int lineOffset = sizeX*(line+yoffset);
      int xpos = pair->x_pos;
      int xneg = pair->x_neg;
      #ifdef VNOAD
      m_chAnalyseData3[lineOffset+xpos] = 255;
      m_chAnalyseData3[lineOffset+xneg] = 0;
      #endif
      nSumPair++;
      fTempPos = (unsigned char)chSrc[lineOffset+xpos+2]
               - (unsigned char)chSrc[lineOffset+xpos];
      fTempNeg = (unsigned char)chSrc[lineOffset+xneg+2]
               - (unsigned char)chSrc[lineOffset+xneg];
      if ( fTempPos >= MIN_DIFF && fTempNeg <= -MIN_DIFF ){
         // ** the considered pair is ok *************
         nRestPair++;
         // ** set them to the view **
         m_chAnalyseData[lineOffset+xpos] = 255;
         m_chAnalyseData[lineOffset+xneg] = 255;
         #ifdef VNOAD
         m_chAnalyseData2[lineOffset+xpos] = 0;
         m_chAnalyseData2[lineOffset+xneg] = 0;
         #endif
      }
      else {
        // ** check if the area is to dark or to bright **
        // ** check both points of pos and neg positions *
        if ( (unsigned char)chSrc[lineOffset+xpos] >   200 ||
             (unsigned char)chSrc[lineOffset+xpos+1] > 200 ||
             (unsigned char)chSrc[lineOffset+xneg] >   200 ||
             (unsigned char)chSrc[lineOffset+xneg+1] > 200 )
                   nUnknown++;
		
     }
     pair = pair->next;
    }
   	tl= tl->next;
  }
  if( logocb != NULL )
  {
    //m_pView->m_chPicture = m_chAnalyseData;
    // set the views headline string
    printf( " %d/%d - %d unknown ",
                                  nRestPair, nSumPair, nUnknown );
  }
//   m_pView->m_chPicture = m_chAnalyseData;
   // set the views headline string
//   sprintf( m_pView->m_strHeadline, " %d/%d - %d unknown ", nRestPair, nSumPair, nUnknown );
   // update the view		
//   m_pView->update();

    int CutOff = nSumPair/3;
    if( yoffset != iLineOffset )
    {
      CutOff = (nSumPair*8)/10;
    }
   // decide the logo status
   if ( nRestPair >= CutOff ) {
      bLogo=true;
   }
   // dont check the unknown sum if logo is
   // always detected
   else {
      if ( nUnknown > CutOff )
           bUnknown = true;
      else
         bNoLogo = true;
   }

   if( yoffset != iLineOffset )
   {
     if ( bLogo )
       isLogo = true;
     else
       isLogo = false;
   }
   else
   {
     if ( bUnknown )
     {
        #ifdef xVNOAD
        emit unknown();
        #endif
     }
     else {
       if ( bLogo ) {
          m_nLogo++;
          // threshold of iTreshold times
          if ( m_nLogo > iTreshold ) {
              #ifdef xVNOAD
              emit logo();
              #endif
              isLogo = true;
              m_nNoLogo = 0;
          }
       }
       if ( bNoLogo ) {
           m_nNoLogo++;
           // threshold of iTreshold times
           if ( m_nNoLogo > iTreshold ) {
               isLogo = false;
               #ifdef xVNOAD
               emit nologo();
               #endif
               m_nLogo = 0;
           }
       }
    }
  }
  //syslog(LOG_INFO, "CCheckLogo::checkTestlines%3d %3d %3d", nSumPair,nRestPair,nUnknown);
	
 return nRestPair;		
}

/** void CCheckLogo::reset()
  * called to delete possible testlines
  */
void CCheckLogo::reset(){

  m_linehook = NULL;
  m_nLogo = 0;
  m_nNoLogo = 0;
  isLogo = false;

  // set the view back to black and headline to no data
  memset( m_chAnalyseData,0, m_pData->m_nSizeY*m_pData->m_nSizeX );
//  m_pView->m_chPicture = m_chAnalyseData;
//  sprintf( m_pView->m_strHeadline, "no data" );
  // update it
//  m_pView->update();

}

void CCheckLogo::setLineHook( struct testlines* testlines )
{
  ////dsyslog(LOG_INFO, "CCheckLogo::setLineHook(%p)", testlines);
  m_linehook = testlines;
}

void CCheckLogo::setCornerData( char* cornerData )
{
	////dsyslog(LOG_INFO, "CCheckLogo::setCornerData(%p)", cornerData);
	m_chCornerData = cornerData;
}

extern void writeInt( FILE *fd, const char *name, int iVal);
extern int readInt( FILE *fd, const char *, int *iVal, bool ignoredata = false);
void CCheckLogo::save( FILE *fd )
{
  struct testlines* tl = m_linehook;
  int line;
  testpair* pair = NULL;

  writeInt( fd, "m_nNoLogo", m_nNoLogo);
  writeInt( fd, "m_nLogo", m_nLogo);

  line = 0;
  while ( tl )
  {
    line++;
    tl= tl->next;
  }
  writeInt( fd, "n_lines", line);
  tl = m_linehook;
  while ( tl )
  {
    line = tl->line;
    writeInt( fd, "line", line );
    pair = tl->pair;
    int iPairCount = 0;
    while ( pair )
    {
      iPairCount++;
      pair = pair->next;
    }
    writeInt( fd, "n_pairs", iPairCount);
    pair = tl->pair;
    while ( pair )
    {
      writeInt( fd, "pair_x_pos", pair->x_pos );
      writeInt( fd, "pair_x_neg", pair->x_neg );
      pair = pair->next;
    }
    tl= tl->next;
  }
}

void CCheckLogo::log()
{
  struct testlines* tl = m_linehook;
  int line;
  testpair* pair = NULL;

  line = 0;
  while ( tl )
  {
    line++;
    tl= tl->next;
  }
  ////dsyslog( LOG_INFO, "    n_lines %d", line);
  tl = m_linehook;
  while ( tl )
  {
    line = tl->line;
    ////dsyslog( LOG_INFO, "    line %d", line );
    pair = tl->pair;
    int iPairCount = 0;
    while ( pair )
    {
      iPairCount++;
      pair = pair->next;
    }
    ////dsyslog( LOG_INFO, "    n_pairs %d", iPairCount);
    pair = tl->pair;
    while ( pair )
    {
     // //dsyslog( LOG_INFO, "    pair_x_pos %d", pair->x_pos );
      ////dsyslog( LOG_INFO, "    pair_x_neg %d", pair->x_neg );
      pair = pair->next;
    }
    tl= tl->next;
  }
}

void CCheckLogo::getLogoRect(int &left, int &top, int &right, int &bottom)
{
  struct testlines* tl = m_linehook;
  testpair* pair = NULL;

  top = bottom = 0;
  left = 99999;
  right = 0;
  tl = m_linehook;
  if( tl )
    top = tl->line;
  while ( tl )
  {
    pair = tl->pair;
    while ( pair )
    {
      if(pair->x_pos < left )
        left = pair->x_pos;
      if(pair->x_pos > right )
        right = pair->x_pos;
      pair = pair->next;
    }
    if( tl->next == 0 )
       bottom = tl->line;
    tl= tl->next;
  }
}

/** called to get a new allocated testine */
testlines* CCheckLogo::new_testline( int line )
{

  ////dsyslog(LOG_INFO, "CGetLogo::new_testline(%d)", line);
  // ** allocate new memory for a testline *****************
  testlines* temp = new testlines[sizeof(struct testlines)];
	
  temp->line = line;
  temp->next = NULL;
  temp->pair = NULL;

 return temp;
}

void CCheckLogo::load( FILE *fd )
{
	int n_lines;
	readInt( fd, "m_nNoLogo", &m_nNoLogo);
	readInt( fd, "m_nLogo", &m_nLogo);
	readInt( fd, "lines", &n_lines);
   if (m_linehook )
   	m_pData->deleteTestlines( &m_linehook );
   m_linehook = 0;
   testlines* tl = NULL;
	while( n_lines-- )
	{
		int line;
		readInt( fd, "line", &line);
		if( tl != NULL )
		{
			tl->next = new_testline( line );
			tl = tl->next;
		}
		else
		{
			tl = new_testline( line );
			setLineHook( tl );
		}
		int iPairs = 0;
		testpair* pair = NULL;
		readInt( fd, "pairs", &iPairs);
		while( iPairs--)
		{
			if( pair == NULL )
			{
				tl->pair = pair = new testpair;
			}
			else
			{
				pair->next = new testpair;
				pair = pair->next;
			}
			pair->next = NULL;
			readInt( fd, "pair_x_pos", &pair->x_pos );
			readInt( fd, "pair_x_neg", &pair->x_neg );
		}		
	}
  iLineOffset = 0;
}
