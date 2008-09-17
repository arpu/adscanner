/***************************************************************************
                          noaddata.cpp  -  description
                             -------------------
    begin                : Sun Mar 10 2002
    copyright            : (C) 2002/2004 by theNoad #709GRW
    email                : theNoad@SoftHome.net
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>


#include "noaddata.h"
#include "ctoolbox.h"

int SysLogLevel = 1;
#define IS_TOP_LEFT  (m_nLogoCorner == UNKNOWN || m_nLogoCorner == TOP_LEFT)
#define IS_TOP_RIGHT (m_nLogoCorner == UNKNOWN || m_nLogoCorner == TOP_RIGHT)
#define IS_BOT_RIGHT (m_nLogoCorner == UNKNOWN || m_nLogoCorner == BOT_RIGHT)
#define IS_BOT_LEFT  (m_nLogoCorner == UNKNOWN || m_nLogoCorner == BOT_LEFT)

noadData::noadData()
{
  bYUVSource = true;
  bUseExternalMem = false;
  iSet = NULL;
  
  /** set screen size for unknown tv cards*/
  m_nGrabWidth = GRAB_WIDTH;
  m_nGrabHeight = GRAB_HEIGHT;
  similarCutoff = (int)(m_nGrabWidth * m_nGrabHeight / 4 * .91);
  //dsyslog(LOG_INFO, "noadData similarCutoff is %ld", similarCutoff);


  /** get logo settings */
  m_fMainFilter = 0.9;
  m_fRunFilter = 0.5;
  m_nTimeInterval = 500;
  m_nMaxAverage = 245;
  #ifdef VNOAD
  if( bYUVSource )
    m_nMinAverage = 20;
  else
  #endif
    m_nMinAverage = 20;
  m_nDif2Pictures = 2;
  m_nFilterFrames = 15;
  m_nCheckFrames = 80;
  m_nFilterInit = 80;
  m_nTopLinesToIgnore = 0;
  m_nBottomLinesToIgnore = 0;
  m_nBlackLinesTop = 0;
  m_nBlackLinesBottom = 0;
  m_nBlackLinesLeft = 0;
  m_nBlackLinesRight = 0;
  m_nMonoFrameValue = 0;
  m_isMonoFrame = -1;
  extLogoSearch = false;
  // set video buffer first time to NULL
  video_buffer_mem = NULL;

  #ifdef VNOAD
  for( int i = 0; i < NUMPICS; i++)
  {
    video_buffer_mem2[i] = NULL;
    video_buffer_mem3[i] = NULL;
  }
  iCompPics = 0;
  #endif
  
  m_bFound = false;
  m_nLogoCorner = UNKNOWN;

  // create the checklogo object
  m_pCheckLogo = NULL;

  videoDir = new char[2048];
  strcpy( videoDir, "/video" );

  // free corner buffers
  m_chColorCorner3 = NULL;
  m_chColorCorner2 = NULL;
  m_chColorCorner1 = NULL;
  m_chColorCorner0 = NULL;
  m_chGreyCorner3 = NULL;
  m_chGreyCorner2 = NULL;
  m_chGreyCorner1 = NULL;
  m_chGreyCorner0 = NULL;

  strcpy(infoLine, "");

  resetSceneChangeDetection();
}

noadData::~noadData()
{
  // free corner buffers
  delete [] m_chColorCorner3;
  delete [] m_chColorCorner2;
  delete [] m_chColorCorner1;
  delete [] m_chColorCorner0;
  delete [] m_chGreyCorner3;
  delete [] m_chGreyCorner2;
  delete [] m_chGreyCorner1;
  delete [] m_chGreyCorner0;
  if( m_pCheckLogo != NULL )
    delete m_pCheckLogo;
  m_pCheckLogo = NULL;
  #ifdef VNOAD
  clearPics();
  #endif
}

// called to init the buffer when video told the document the maximum size
void noadData::initBuffer()
{
  // free corner buffers
  if(m_chColorCorner3)
    delete [] m_chColorCorner3;
  if(m_chColorCorner2)
    delete [] m_chColorCorner2;
  if(m_chColorCorner1)
    delete [] m_chColorCorner1;
  if(m_chColorCorner0)
    delete [] m_chColorCorner0;
  if(m_chGreyCorner3)
    delete [] m_chGreyCorner3;
  if(m_chGreyCorner2)
    delete [] m_chGreyCorner2;
  if(m_chGreyCorner1)
    delete [] m_chGreyCorner1;
  if(m_chGreyCorner0)
    delete [] m_chGreyCorner0;

  // settings for corner
  m_nBorderX  = m_nGrabWidth / 32;
  m_nBorderYTop  = m_nGrabWidth / 32;
  if( m_nTopLinesToIgnore > m_nBorderYTop )
    m_nBorderYTop = m_nTopLinesToIgnore - m_nBorderYTop;
  m_nBorderYBot  = m_nGrabWidth / 32;
  if( m_nBottomLinesToIgnore > m_nBorderYBot )
    m_nBorderYBot = m_nBottomLinesToIgnore - m_nBorderYBot;
  m_nSizeX   = m_nGrabWidth / 5;
//  m_nSizeY   = m_nGrabWidth / 5;
  m_nSizeY   = m_nGrabHeight / 4;

  // allocate memory for the color corners
  m_chColorCorner0 = new char[m_nSizeX*m_nSizeY*BYTES_PER_PIXEL];
  m_chColorCorner1 = new char[m_nSizeX*m_nSizeY*BYTES_PER_PIXEL];
  m_chColorCorner2 = new char[m_nSizeX*m_nSizeY*BYTES_PER_PIXEL];
  m_chColorCorner3 = new char[m_nSizeX*m_nSizeY*BYTES_PER_PIXEL];
  // set color corner to black for the first time
  memset( m_chColorCorner0, 100, m_nSizeX*m_nSizeY*BYTES_PER_PIXEL );
  memset( m_chColorCorner1, 100, m_nSizeX*m_nSizeY*BYTES_PER_PIXEL );
  memset( m_chColorCorner2, 100, m_nSizeX*m_nSizeY*BYTES_PER_PIXEL );
  memset( m_chColorCorner3, 100, m_nSizeX*m_nSizeY*BYTES_PER_PIXEL );

  // allocate memory for the grey corners
  m_chGreyCorner0 = new char[m_nSizeX*m_nSizeY];
  m_chGreyCorner1 = new char[m_nSizeX*m_nSizeY];
  m_chGreyCorner2 = new char[m_nSizeX*m_nSizeY];
  m_chGreyCorner3 = new char[m_nSizeX*m_nSizeY];
  // set grey corner to black for the first time
  memset( m_chGreyCorner0, 0, m_nSizeX*m_nSizeY );
  memset( m_chGreyCorner1, 0, m_nSizeX*m_nSizeY );
  memset( m_chGreyCorner2, 0, m_nSizeX*m_nSizeY );
  memset( m_chGreyCorner3, 0, m_nSizeX*m_nSizeY );

  // create the checklogo object
  if( m_pCheckLogo != NULL )
    delete m_pCheckLogo;
  m_pCheckLogo = new CCheckLogo( this );
  resetSceneChangeDetection();
  if( iSet )
    delete [] iSet;
  iSet = new int[m_nGrabWidth*2];
}

// called to clean the testline list
void noadData::deleteTestlines( testlines** tl )
{
  testlines* del_line;
  testpair*  next_pair;
  testpair*  del_pair;

  while ( *tl )
  {
    if ( (*tl)->pair )
    {
       next_pair = (*tl)->pair;
       while ( next_pair )
       {
         del_pair = next_pair;
         next_pair = next_pair->next;
         delete [] del_pair;
       }
    }
    del_line = *tl;
    *tl = (*tl)->next;
    delete [] del_line;
  }

  *tl = NULL;
}

/** copy data into color corner buffer */
void noadData::setColorCorners()
{
  CToolBox tool;

  int x,y, count0=0, count1=0, count2=0, count3=0;
  long lCount = 0;

  for ( y = 0; y < m_nGrabHeight; y++ )
  {
    for ( x = 0; x < m_nGrabWidth; x++ )
    {
      int lc3 = lCount*BYTES_PER_PIXEL;
      /** the both top corners */
      if ( y>=m_nBorderYTop && y<m_nSizeY+m_nBorderYTop)
      {
        //* top left corner */
        if ( IS_TOP_LEFT && x>=m_nBorderX && x<m_nBorderX+m_nSizeX )
        {
          tool.filter_tp( &m_chColorCorner0[count0++],
            &video_buffer_mem[lc3], m_fRunFilter );
          tool.filter_tp( &m_chColorCorner0[count0++],
            &video_buffer_mem[lc3+1], m_fRunFilter );
          tool.filter_tp( &m_chColorCorner0[count0++],
            &video_buffer_mem[lc3+2], m_fRunFilter );
          #ifdef USE_RGB32
          count0++;
          #endif
        }
        //* top right corner */
        if ( IS_TOP_RIGHT && x>=m_nGrabWidth-m_nBorderX-m_nSizeX && x< m_nGrabWidth-m_nBorderX )
        {
          tool.filter_tp( &m_chColorCorner1[count1++],
            &video_buffer_mem[lc3],m_fRunFilter );
          tool.filter_tp( &m_chColorCorner1[count1++],
            &video_buffer_mem[lc3+1], m_fRunFilter );
          tool.filter_tp( &m_chColorCorner1[count1++],
            &video_buffer_mem[lc3+2], m_fRunFilter );
          #ifdef USE_RGB32
          count1++;
          #endif
        }
      }
      /** the both bottom corners */
      if ( y>=m_nGrabHeight-m_nBorderYBot-m_nSizeY && y<m_nGrabHeight-m_nBorderYBot )
      {
        //* bot left corner */
        if ( IS_BOT_LEFT && x>=m_nBorderX && x<m_nBorderX+m_nSizeX )
        {
          tool.filter_tp( &m_chColorCorner3[count3++],
            &video_buffer_mem[lc3], m_fRunFilter );
          tool.filter_tp( &m_chColorCorner3[count3++],
            &video_buffer_mem[lc3+1], m_fRunFilter );
          tool.filter_tp( &m_chColorCorner3[count3++],
            &video_buffer_mem[lc3+2], m_fRunFilter );
          #ifdef USE_RGB32
          count3++;
          #endif
        }
        //* bot right corner */
        if ( IS_BOT_RIGHT && x>=m_nGrabWidth-m_nBorderX-m_nSizeX && x< m_nGrabWidth-m_nBorderX )
        {
          tool.filter_tp( &m_chColorCorner2[count2++],
            &video_buffer_mem[lc3], m_fRunFilter );
          tool.filter_tp( &m_chColorCorner2[count2++],
            &video_buffer_mem[lc3+1], m_fRunFilter );
          tool.filter_tp( &m_chColorCorner2[count2++],
            &video_buffer_mem[lc3+2], m_fRunFilter );
          #ifdef USE_RGB32
          count2++;
          #endif
        }
      }
      lCount++;
    }
  }
}

/** set the grey corners from color corners
  * use values for color components
  */
void noadData::setGreyCorners( float fRed, float fGreen, float fBlue)
{
  int x, y, n=0;
  int mb, mg, mr;
  float fSum, fCompRed, fCompGreen, fCompBlue;
  
  for ( y=0; y< m_nSizeY; y++ )
  {
    for ( x=0; x< m_nSizeX; x++ )
    {
      mb = n*BYTES_PER_PIXEL;
      mg = mb+1;
      mr = mg+1;
      /** set corner top left */
      if( IS_TOP_LEFT )
      {
        fCompBlue  = (unsigned char)m_chColorCorner0[mb];
        fCompGreen = (unsigned char)m_chColorCorner0[mg];
        fCompRed   = (unsigned char)m_chColorCorner0[mr];
        fSum = fRed*fCompRed + fGreen*fCompGreen + fBlue*fCompBlue;
        m_chGreyCorner0[n] = (int)fSum;
  	  }
      /** set corner top right */
      if( IS_TOP_RIGHT )
      {
        fCompBlue  = (unsigned char)m_chColorCorner1[mb];
        fCompGreen = (unsigned char)m_chColorCorner1[mg];
        fCompRed   = (unsigned char)m_chColorCorner1[mr];
        fSum = fRed*fCompRed + fGreen*fCompGreen + fBlue*fCompBlue;
        m_chGreyCorner1[n] = (int)fSum;
   	}
      /** set corner bot right */
      if( IS_BOT_RIGHT )
      {
        fCompBlue  = (unsigned char)m_chColorCorner2[mb];
        fCompGreen = (unsigned char)m_chColorCorner2[mg];
        fCompRed   = (unsigned char)m_chColorCorner2[mr];
        fSum = fRed*fCompRed + fGreen*fCompGreen + fBlue*fCompBlue;
        m_chGreyCorner2[n] = (int)fSum;
   	}
      /** set corner bot left */
      if( IS_BOT_LEFT )
      {
        fCompBlue  = (unsigned char)m_chColorCorner3[mb];
        fCompGreen = (unsigned char)m_chColorCorner3[mg];
        fCompRed   = (unsigned char)m_chColorCorner3[mr];
        fSum = fRed*fCompRed + fGreen*fCompGreen + fBlue*fCompBlue;
        m_chGreyCorner3[n] = (int)fSum;
   	}
      n++;
    }
  }
}

/** sets the running corners */
void noadData::setCorners()
{
  if ( video_buffer_mem != NULL )
  {
    if( bYUVSource )
      setYUVGreyCorners();
    else
    {
      setColorCorners();
      setGreyCorners( 0.3, 0.59, 0.11 );
    }
  }
}

// set the grey corners from YUV-Plane
void noadData::setYUVGreyCorners()
{
  CToolBox tool;

  int count0=0, count3=0;
  int topend = m_nBorderYTop+m_nSizeY;
  int bottStart = m_nGrabHeight-m_nBorderYBot-m_nSizeY;
  int bottEnd = m_nGrabHeight-m_nBorderYBot;
  register int linestart;
  int rightLogoOffset = m_nGrabWidth-m_nBorderX-m_nSizeX;
  for ( int y = m_nBorderYTop; y<topend; y++)
  {
     linestart = y*m_nGrabWidth;
     //linestart = y*(GRAB_WIDTH+8);
     memcpy(&m_chGreyCorner0[count0],&video_buffer_mem[linestart+m_nBorderX],m_nSizeX);
     memcpy(&m_chGreyCorner1[count0],&video_buffer_mem[linestart+rightLogoOffset],m_nSizeX);
     count0 += m_nSizeX;
  }
  for ( int y = bottStart; y<bottEnd; y++ )
  {
     linestart = y*m_nGrabWidth;
     //linestart = y*(GRAB_WIDTH+16);
     memcpy(&m_chGreyCorner3[count3],&video_buffer_mem[linestart+m_nBorderX],m_nSizeX);
     memcpy(&m_chGreyCorner2[count3],&video_buffer_mem[linestart+rightLogoOffset],m_nSizeX);
     count3 += m_nSizeX;
  }
/*  if( logocb != NULL )
  {
    // set results to view
    //sprintf( m_pView->m_strHeadline, "found %d couples", nCouples );
    //m_pView->update();
    char *buffer = NULL;
    asprintf(&buffer,"data set");
    //logocb(99,buffer,video_buffer_mem,m_nSizeX,m_nSizeY);
    logocb(0,buffer,m_chGreyCorner0,m_nSizeX,m_nSizeY);
    logocb(1,buffer,m_chGreyCorner1,m_nSizeX,m_nSizeY);
    logocb(2,buffer,m_chGreyCorner2,m_nSizeX,m_nSizeY);
    logocb(3,buffer,m_chGreyCorner3,m_nSizeX,m_nSizeY);
    delete buffer;
  }*/
}

void writeInt( FILE *fd, const char */*name*/, int iVal)
{
  fwrite(&iVal, sizeof(int), 1, fd);
}
int readInt( FILE *fd, const char *name, int *iVal, bool ignoredata = false)
{
  int iDummy;
  if(ignoredata)
     fread(&iDummy, sizeof(int), 1, fd );
  else
     fread(iVal, sizeof(int), 1, fd );
//  //dsyslog(LOG_INFO, "noadData Load %s %d", name, *iVal);
  return *iVal;
}
void noadData::saveCheckData( const char *name, bool bFullnameGiven )
{
  FILE *fd;
  char fname[2048];
  if( !bFullnameGiven )
  {
    strcpy( &fname[0], videoDir );
    strcat( fname, "/");
    strcat( fname, name );
  }
  else
    strcpy( fname, name );
  //dsyslog(LOG_INFO, "noadData saveCheckData to %s", fname);
  fd = fopen(fname, "wb");
  if( fd )
  {
    writeInt( fd, "m_nFilterInit", m_nFilterInit);
    writeInt( fd, "m_nFilterFrames", m_nFilterFrames);
    writeInt( fd, "m_nDif2Pictures", m_nDif2Pictures);
    writeInt( fd, "m_nMaxAverage", m_nMaxAverage);
    writeInt( fd, "m_nTimeInterval", m_nTimeInterval);
    writeInt( fd, "m_nSizeX", m_nSizeX);
    writeInt( fd, "m_nSizeY", m_nSizeY);
    writeInt( fd, "m_nBorderX", m_nBorderX);
    writeInt( fd, "m_nBorderYTop", m_nBorderYTop);
    writeInt( fd, "m_nBorderYBot", m_nBorderYBot);
    writeInt( fd, "m_nGrabHeight", m_nGrabHeight);
    writeInt( fd, "m_nGrabWidth", m_nGrabWidth);
    writeInt( fd, "m_nMinAverage", m_nMinAverage);
    writeInt( fd, "m_nCheckFrames", m_nCheckFrames);
    writeInt( fd, "m_nLogoCorner", m_nLogoCorner);
    writeInt( fd, "m_bFound", m_bFound);
    m_pCheckLogo->save( fd );
    fclose( fd );
  }
 // else
    //dsyslog(LOG_INFO, "noadData can't open %s", fname);
}

void noadData::logCheckData()
{
  //dsyslog( LOG_INFO, "  m_nFilterInit %d", m_nFilterInit);
  //dsyslog( LOG_INFO, "  m_nFilterFrames %d", m_nFilterFrames);
  //dsyslog( LOG_INFO, "  m_nDif2Pictures %d", m_nDif2Pictures);
  //dsyslog( LOG_INFO, "  m_nMaxAverage %d", m_nMaxAverage);
  //dsyslog( LOG_INFO, "  m_nTimeInterval %d", m_nTimeInterval);
  //dsyslog( LOG_INFO, "  m_nSizeX %d", m_nSizeX);
  //dsyslog( LOG_INFO, "  m_nSizeY %d", m_nSizeY);
  //dsyslog( LOG_INFO, "  m_nBorderX %d", m_nBorderX);
  //dsyslog( LOG_INFO, "  m_nBorderYTop %d", m_nBorderYTop);
  //dsyslog( LOG_INFO, "  m_nBorderYBot %d", m_nBorderYBot);
  //dsyslog( LOG_INFO, "  m_nGrabHeight %d", m_nGrabHeight);
  //dsyslog( LOG_INFO, "  m_nGrabWidth %d", m_nGrabWidth);
  //dsyslog( LOG_INFO, "  m_nMinAverage %d", m_nMinAverage);
  //dsyslog( LOG_INFO, "  m_nCheckFrames %d", m_nCheckFrames);
  //dsyslog( LOG_INFO, "  m_nLogoCorner %d", m_nLogoCorner);
  //dsyslog( LOG_INFO, "  m_bFound %d", m_bFound);
  m_pCheckLogo->log();
}

bool noadData::loadCheckData( const char *name, bool bFullnameGiven )
{
  bool ignoreData = false;
  FILE *fd;
  char fname[2048];
  if( !bFullnameGiven)
  {
    strcpy( &fname[0], videoDir );
    strcat( fname, "/");
    strcat( fname, name );
  }
  else
    strcpy( fname, name );
  //dsyslog(LOG_INFO, "noadData loadCheckData from %s", fname);
  fd = fopen(fname, "rb");
  if( fd )
  {
    readInt( fd, "m_nFilterInit", &m_nFilterInit,ignoreData);
    //dsyslog(LOG_INFO, "noadData Load %s %d", "m_nFilterInit", m_nFilterInit);
    readInt( fd, "m_nFilterFrames", &m_nFilterFrames,ignoreData);
    //dsyslog(LOG_INFO, "noadData Load %s %d", "m_nFilterFrames", m_nFilterFrames);
    readInt( fd, "m_nDif2Pictures", &m_nDif2Pictures,ignoreData);
    readInt( fd, "m_nMaxAverage", &m_nMaxAverage,ignoreData);
    readInt( fd, "m_nTimeInterval", &m_nTimeInterval,ignoreData);
    readInt( fd, "m_nSizeX", &m_nSizeX,ignoreData);
    readInt( fd, "m_nSizeY", &m_nSizeY,ignoreData);
    readInt( fd, "m_nBorderX", &m_nBorderX,ignoreData);
    readInt( fd, "m_nBorderYTop", &m_nBorderYTop,ignoreData);
    readInt( fd, "m_nBorderYBot", &m_nBorderYBot,ignoreData);
    readInt( fd, "m_nGrabHeight", &m_nGrabHeight,ignoreData);
    readInt( fd, "m_nGrabWidth", &m_nGrabWidth,ignoreData);
    readInt( fd, "m_nMinAverage", &m_nMinAverage,ignoreData);
    readInt( fd, "m_nCheckFrames", &m_nCheckFrames,ignoreData);
    readInt( fd, "m_nLogoCorner", &m_nLogoCorner);


    similarCutoff = (int)(m_nGrabWidth * m_nGrabHeight / 4 * .91);
    //dsyslog(LOG_INFO, "noadData similarCutoff is now %ld", similarCutoff);

    int iVal;
    m_bFound = readInt( fd, "m_bFound", &iVal) > 0;
    m_pCheckLogo->load( fd );
    fclose( fd );
    switch( m_nLogoCorner )
    {
      case 0: m_pCheckLogo->setCornerData(m_chGreyCorner0); break;
      case 1: m_pCheckLogo->setCornerData(m_chGreyCorner1); break;
      case 2: m_pCheckLogo->setCornerData(m_chGreyCorner2); break;
      case 3: m_pCheckLogo->setCornerData(m_chGreyCorner3); break;
    }
    return true;
  }
  else
    //dsyslog(LOG_INFO, "noadData can't open %s", fname);
  return false;
}

void noadData::setGrabSize( int width, int height )
{
  //dsyslog(LOG_INFO, "noadData setGrabSize from %d %d to %d %d", m_nGrabWidth,m_nGrabHeight,width, height);
  if( m_nGrabWidth != width || m_nGrabHeight != height )
  {
    //dsyslog( LOG_INFO, "init grabbing with %d %d = %d pixels", width, height, width*height);
    m_nGrabWidth = width;
    m_nGrabHeight = height;
    initBuffer();
    similarCutoff = (int)(m_nGrabWidth * m_nGrabHeight / 4 * .91);
    //dsyslog(LOG_INFO, "noadData similarCutoff is now %ld", similarCutoff);
  }
}


int noadData::countvals(unsigned char *_src[3], int line, int width, int /*height*/, int border)
{
  //int *iSet = new int[width*2];
  int iHalfWidth = width>>1;
  int iHalfLine = line>>1;
  int iHalfOffset = iHalfWidth*iHalfLine;
  unsigned char *src = _src[0] + width * line;
  #ifdef VNOAD
  unsigned char *usrc = _src[1] + iHalfOffset;
  unsigned char *vsrc = _src[2] + iHalfOffset;
  #endif
  memset( iSet,0,width*2*sizeof(int));

  border = 0;
  #ifdef VNOAD
  border = 0;
  int ySum = 0;
  int iColSum = 0;
  #endif
  
  int uSum = 0;
  int vSum = 0;
  for(int i = border; i < width-(border); i++)
  {
    iSet[src[i]] = 1;
    //iSet[*src++] = 1;
    #ifdef VNOAD
    ySum += src[i];
    int y = src[i];
    int u = usrc[i/2];
    int v = vsrc[i/2];
    iColSum += ((y+u+v)/3);
    #endif
  }
  int iyCount = 0;
  for( int i = 0; i < width;i++)
    iyCount += iSet[i];
  //memset( iSet,0,width*sizeof(int));
  src = _src[1]+iHalfOffset;
  for(int i = 0; i < iHalfWidth; i++)
  {
    //iSet[src[i]] = 1;
    uSum += src[i];
    //iSet[*src] = 1;
    //uSum += *src++;
  }
  //int iuCount = 0;
  //for( int i = 0; i < iHalfWidth;i++)
  //  iuCount += iSet[i];
  //memset( iSet,0,width*sizeof(int));
  src = _src[2]+iHalfOffset;
  for(int i = 0; i < iHalfWidth; i++)
  {
    //iSet[src[i]] = 1;
    vSum += src[i];
    //iSet[*src] = 1;
    //vSum += *src++;
  }
  //int ivCount = 0;
  #ifdef VNOAD
  //int iFaktor = ySum/width-(border) + uSum/width/2 + vSum/width/2;
  #endif
  //for( int i = 0; i < iHalfWidth;i++)
  //  ivCount += iSet[i];
  //delete [] iSet;
  int iRetval = iyCount;
  iRetval += abs( 128 - (uSum/(iHalfWidth)));
  iRetval += abs( 128 - (vSum/(iHalfWidth)));
  return iRetval;
}

int countvalsV(unsigned char *_src[3], bool isRight, int width, int height )
{
	int lineSize = width;
	int column;
	int line;
	int lumi;
	int oldLumi = 0;
	int startPos;
  int start = isRight ? width-1 : 0;
  int kMaxVert = 50;
	int end = !isRight ? kMaxVert : start - kMaxVert;
	int increment = isRight ? -1 : 1;
	int black;
	double ratio;
	double oldRatio = 0.0;
  int kMaxLineLuminance = 35;
  int kFactor = 20;
	column = start;
	while (column != end)
	{
		lumi = 0;
		black = 0;
		for (line = 0; line < height; line++)
		{
			startPos = line * lineSize;
			lumi += _src[0][startPos + column];
			if (_src[0][startPos + column] < 21)
				black++;
		}
		lumi /= height;
		ratio = ((double) black) / ((double) height);
		if (column == start)
		{
			/*
      if (left)
				firstCol = lumi;
			else
				lastCol = lumi;
			*/
      if (lumi > kMaxLineLuminance)
				break;
		}
		else
		{
			if (oldRatio / ratio > 2.0)
				break;
			if (lumi > (kFactor * oldLumi) / 10)
				break;
			if (ratio < 0.7)
			{
				column = end;
				break;
			}
		}
		oldRatio = ratio;
		oldLumi = lumi;
		column += increment;
	}
/*
	if (column != end)
	{
		if (left)
			this->newBorder.left = column;
		else
			this->newBorder.right = start - column;
	}
*/
	if (column != end)
  {
    if( isRight )
      return start-column;
    else
      return column;
  }
  return 0;
}

void noadData::detectBlackLines(unsigned char *buf)
{
  bool bNonBlack = false;
  m_nBlackLinesTop = 0;
  m_nBlackLinesBottom = 0;
  m_nBlackLinesTop2 = 0;
  m_nBlackLinesBottom2 = 0;
  m_nBlackLinesLeft = 0;
  m_nBlackLinesRight = 0;
  int i;
  int ii;
  int iBytesPerPixel = isYUVSource() ? 1 : BYTES_PER_PIXEL;
  int cutval = isYUVSource() ? 20/*83*/ : 9;
//  int iPixCount = m_nGrabWidth * iBytesPerPixel;
  if( isYUVSource() )
  {
    if( buf == NULL )
      return;
    unsigned char **bufs =(unsigned char **)buf;
    //m_nBlackLinesTop2 = 8;
    // start at line 9, sometimes lines above have garbage
    for( i = 8; i < m_nGrabHeight/2 && !bNonBlack; i++ )
    {
      if( countvals(bufs, i, m_nGrabWidth, m_nGrabHeight,m_nBorderX+m_nSizeX) > cutval )
        bNonBlack = true;
      else
        m_nBlackLinesTop2++;
    }
    m_nBlackLinesTop2 &= ~1;
    bNonBlack = false;
    for( i = m_nGrabHeight-2; i > 0 && i > m_nBlackLinesTop2 && !bNonBlack; i-- )
    {
      if( countvals(bufs, i, m_nGrabWidth, m_nGrabHeight,m_nBorderX+m_nSizeX) > cutval )
        bNonBlack = true;
      else
        m_nBlackLinesBottom2++;
    }
    if( m_nBlackLinesBottom2 &1 )
      m_nBlackLinesBottom2--;

    m_nBlackLinesTop = m_nBlackLinesTop2;
    m_nBlackLinesBottom = m_nBlackLinesBottom2;

    // not useful:
    //m_nBlackLinesLeft = countvalsV(bufs, false, m_nGrabWidth, m_nGrabHeight );
    //m_nBlackLinesRight = countvalsV(bufs, true, m_nGrabWidth, m_nGrabHeight );

    // YUV-Blacklines:
    // Y-Values <= cutval. max differenz inline <= maxdiff
/*
    int iMaxDiff = 150;
    for( i = 0; i < m_nGrabHeight && !bNonBlack; i++ )
    {
      int linestart = i * iPixCount;
      int iLineMin = 255;
      int iLineMax = 0;
      int iCutvalCount = 0;
      for( ii = 0; (ii < iPixCount) && !bNonBlack; ii++ )
      {
        int iVal = (unsigned char)video_buffer_mem[linestart+ii];
        if( iVal > cutval && ++iCutvalCount > 5 )
          bNonBlack = true;
        else
        {
          if( iVal > iLineMax )
            iLineMax = iVal;
          else if( iVal < iLineMin )
            iLineMin = iVal;
          if( ii > 5 )
            if( (iLineMax - iLineMin) > iMaxDiff )
              bNonBlack = true;
        }
      }
      if( !bNonBlack )
        m_nBlackLinesTop++;
    }
    bNonBlack = false;
    for( i = m_nGrabHeight-1; i > 0 && !bNonBlack; i-- )
    {
      int linestart = i * iPixCount;
      int iLineMin = 255;
      int iLineMax = 0;
      int iCutvalCount = 0;
      for( ii = 0; (ii < iPixCount) && !bNonBlack; ii++ )
      {
        int iVal = (unsigned char)video_buffer_mem[linestart+ii];
        if( iVal > cutval && ++iCutvalCount > 50 )
	{
          bNonBlack = true;
//          //dsyslog(LOG_INFO, "ival(%d,%d) > cutval ", iVal,iCutvalCount);
	  //logline(&video_buffer_mem[linestart],iPixCount);
	}
        else
        {
          if( iVal > iLineMax )
            iLineMax = iVal;
          else if( iVal < iLineMin )
            iLineMin = iVal;
          if( ii > 5 )
            if( (iLineMax - iLineMin) > iMaxDiff )
	    {
              bNonBlack = true;
//              //dsyslog(LOG_INFO, "diff(%d) > iMaxDiff ", (iLineMax - iLineMin));
              //logline(&video_buffer_mem[linestart],iPixCount);
	    }
        }
      }
      if( !bNonBlack )
        m_nBlackLinesBottom++;
      ////dsyslog(LOG_INFO, "noadData detectBlackLines iLineMin=%d iLineMax=%d", iLineMin,iLineMax);
    }
*/
  }
  else
  {
    for( i = 0; i < m_nGrabHeight && !bNonBlack; i++ )
    {
      int linestart = i * (m_nGrabWidth * iBytesPerPixel);
      int iLineSum = 0;
      for( ii = 0; ii < m_nGrabWidth * iBytesPerPixel; ii++ )
      {
        iLineSum += (unsigned char)video_buffer_mem[linestart+ii];
      }
      ////dsyslog(LOG_INFO, "noadData iLineSum %d ", iLineSum);
      if( iLineSum > m_nGrabWidth*cutval )
        bNonBlack = true;
      else
        m_nBlackLinesTop++;
    }
    bNonBlack = false;
    for( i = m_nGrabHeight-1; i > 0 && !bNonBlack; i-- )
    {
      int linestart = i * (m_nGrabWidth * iBytesPerPixel);
      int iLineSum = 0;
      for( ii = 0; ii < m_nGrabWidth * iBytesPerPixel; ii++ )
      {
        iLineSum += video_buffer_mem[linestart+ii];
      }
      if( iLineSum > m_nGrabWidth*cutval )
        bNonBlack = true;
      else
        m_nBlackLinesBottom++;
    }
  }
}

void noadData::checkMonoFrame( int framenum, unsigned char **_src)
{
#define MINMAX(a,b) { if(a##Min > b ) a##Min = b; if( a##Max < b) a##Max = b; }
  int size = m_nGrabWidth*m_nGrabHeight;
  int uvsize = (m_nGrabWidth/2)*(m_nGrabHeight/2);
  int width = m_nGrabWidth;
  int *iSet = new int[256];
  unsigned char *src = _src[0];
  memset( iSet,0,256*sizeof(int));

  for(int i = width*2; i < size-2*width; i++)
  {
    iSet[src[i]] = 1;
  }
  int iyCount = 0;
  for( int i = 0; i < 256;i++)
    iyCount += iSet[i];

  memset( iSet,0,256*sizeof(int));
  src = _src[1];
  for(int i = width/2; i < uvsize-width/2; i++)
  {
    iSet[src[i]] = 1;
  }
  int iuCount = 0;
  for( int i = 0; i < 256;i++)
    iuCount += iSet[i];

  memset( iSet,0,256*sizeof(int));
  src = _src[2];
  for(int i = width/2; i < uvsize-width/2; i++)
  {
    iSet[src[i]] = 1;
  }
  int ivCount = 0;
  for( int i = 0; i < 256;i++)
    ivCount += iSet[i];

#ifdef VNOAD
  //dsyslog(LOG_INFO, "dif values in pic %d: %d %d %d s-count %3d",
    framenum, iyCount,iuCount,ivCount, iyCount+iuCount+ivCount);
#endif
  m_nMonoFrameValue =  iyCount+iuCount+ivCount;
  delete [] iSet;
}

bool pointInRect(int x, int y, int left, int top, int right, int bottom)
{
  if( y >= top && y <= bottom && x >= left && x <= right)
    return true;
  return false;
}

bool noadData::CheckFrameIsBlank(int framenum, unsigned char **buf )
{
    m_isMonoFrame = 0;
    const unsigned int max_brightness = 135; // check this value, org was 120
    const unsigned int test_brightness = 80;
    const int pass_start[7] = {0, 4, 0, 2, 0, 1, 0};
    const int pass_inc[7] = {8, 8, 4, 4, 2, 2, 1};
    const int pass_ystart[7] = {0, 0, 4, 0, 2, 0, 1};
    const int pass_yinc[7] = {8, 8, 8, 4, 4, 2, 2};
    bool isDim = false;

    int l, r, t, b;
    l = r = t = b = 0;

    m_pCheckLogo->  getLogoRect(l, t, r, b);
    
    int width = m_nGrabWidth;
    int height = m_nGrabHeight;
    int yPos;
    if (!width || !height)
        return(false);

    if( l > 4 )
      l -= 4;
    if( t > 4 )
      t -= 4;
    if( b > 0 )
      b += 4;
    if( r > 0 )
      r += 4;
    unsigned char *frame_ptr = (unsigned char *)getVideoBuffer();//buf[0];
    // go through the image in png interlacing style testing if blank
    for(int pass = 0; pass < 7; pass++)
    {
        for(int y = pass_ystart[pass]; y < height; y += pass_yinc[pass])
        {
            yPos = y * width;
            for(int x = pass_start[pass]; x < width; x += pass_inc[pass])
            {
                if( !pointInRect(x,y,l,t,r,b) )
                {
                  if (frame_ptr[yPos + x] > max_brightness)
                  {
                      ////dsyslog(LOG_INFO, "CheckFrameIsBlank of frame %d gives false(1) at %d %d with %d",framenum, x, y,frame_ptr[y * width + x] );
                      return(false);
                  }
                  if (frame_ptr[yPos + x] > test_brightness)
                      isDim = true;
                }
            }
        }
    }

    // frame is dim so test average
    if (isDim)
    {
        int avg = GetAvgBrightness(framenum,buf);

        if (avg > 35)
        {
            //dyslog(LOG_INFO, "CheckFrameIsBlank of frame %d gives false(2)",framenum);
            return(false);
        }
    }
    ////dsyslog(LOG_INFO, "CheckFrameIsBlank of frame %d gives true",framenum);
    m_isMonoFrame = 1;
    return(true);
}

int noadData::GetAvgBrightness(int framenum, unsigned char **buf )
{
  int brightness = 0;
  int pixels = 0;
  int yPos;

  unsigned char *frame_ptr = (unsigned char *)getVideoBuffer();//buf[0];
  for(int y = 0; y < m_nGrabHeight; y += 4)
  {
    yPos = y * m_nGrabWidth;
    for(int x = 0; x < m_nGrabWidth; x += 4)
    {
      brightness += frame_ptr[yPos + x];
      pixels++;
    }
  }
  ////dsyslog(LOG_INFO, "AvgBrightness of frame %d is %d",framenum, brightness/pixels);
  return(brightness/pixels);
}

void noadData::setUseExternalMem(bool b)
{
  if( !bUseExternalMem )
  {
    delete [] video_buffer_mem;
    video_buffer_mem = NULL;
  }
  bUseExternalMem = b;
}

void noadData::setExternalMem(char *extMem)
{
  setUseExternalMem(true);
  video_buffer_mem = extMem;
  m_isMonoFrame = -1;
}

void noadData::resetSceneChangeDetection()
{
  lastFrameWasSceneChange = false;
  memset(lastHistogram, 0, sizeof(lastHistogram));
  memset(histogram, 0, sizeof(histogram));
  lastHistogram[0] = -1;
  histogram[0] = -1;
  sceneHasChanged = false;
}

void noadData::getHistogram(simpleHistogram &dest)
{
  unsigned char *frame_ptr = (unsigned char *)getVideoBuffer();
  memset(dest, 0, sizeof(simpleHistogram));
  for(register int y = 0; y < m_nGrabHeight; y += 2 )
  {
    int linestart = y * m_nGrabWidth;
    for(register int x = 0; x < m_nGrabWidth; x += 2 )
      dest[frame_ptr[linestart + x]]++;
  }
}

bool noadData::areSimilar(simpleHistogram &hist1,simpleHistogram &hist2)
{
  long similar = 0;
//  long delta = 0;

  for(register int i = 0; i < 256; i++)
  {
    if (hist1[i] < hist2[i])
      similar += hist1[i];
    else
      similar += hist2[i];
//    delta += abs(hist1[i]-hist2[i]);
  }
  //if (similar < (m_nGrabWidth * m_nGrabHeight / 4 * .91))
  if (similar < similarCutoff)
  {
    return(false);
  }
  ////dsyslog(LOG_INFO,"similarity is %d of %d, delta is %d",similar,similarCutoff,delta);
//  if( delta < 12000 )
//    return(true);
//  return(false);
  return(true);
}

// analyzes every 1 out of 4 pixels (every other column in every other row)
bool noadData::CheckSceneHasChanged(void)
{
  sceneHasChanged = false;
//    unsigned char *frame_ptr = (unsigned char *)getVideoBuffer();
    if (!m_nGrabWidth || !m_nGrabHeight)
        return(false);

    if (lastHistogram[0] == -1)
    {
      getHistogram(lastHistogram);
      return(false);
    }

    memcpy(lastHistogram, histogram, sizeof(histogram));

    // compare current frame with last frame here
    getHistogram(histogram);
    if (lastFrameWasSceneChange)
    {
        memcpy(lastHistogram, histogram, sizeof(histogram));
        return(false);
    }

    if( !areSimilar(histogram,lastHistogram) )
    {
       memcpy(lastHistogram, histogram, sizeof(histogram));
       sceneHasChanged = true;
       return(true);
    }
    return(false);
}

#ifdef VNOAD
void noadData::storePic(int n)
{
  if( n < 0 || n > (NUMPICS-1) )
    return;
  //if( video_buffer_mem2 != NULL )
  //  delete [][] video_buffer_mem2;
  if( video_buffer_mem2[n] == NULL )
  {
    video_buffer_mem2[n] = new char[m_nGrabWidth*m_nGrabHeight];
    video_buffer_mem3[n] = new char[m_nGrabWidth*m_nGrabHeight];
  }
  memcpy(video_buffer_mem2[n],video_buffer_mem,m_nGrabWidth*m_nGrabHeight);
  switch( m_nLogoCorner )
  {
    case UNKNOWN:   break;
    case TOP_LEFT:  break;
    case TOP_RIGHT:
      for( int i = m_nBorderYTop; i < m_nBorderYTop + m_nGrabHeight/5; i++)
      {
        for( int ii = m_nGrabWidth-m_nBorderX; ii < m_nGrabWidth; ii++)
          video_buffer_mem2[n][i*m_nGrabWidth+ii] = 0;
      }
    break;
    case BOT_RIGHT: break;
    case BOT_LEFT:  break;
  }
}

void noadData::storeCompPic()
{
  if( iCompPics < NUMPICS-1 )
    memcpy(video_buffer_mem3[iCompPics++],video_buffer_mem,m_nGrabWidth*m_nGrabHeight);
  else
  {
    for( int i = 1; i < NUMPICS; i++)
      video_buffer_mem3[i-1] = video_buffer_mem3[i];
    memcpy(video_buffer_mem3[iCompPics],video_buffer_mem,m_nGrabWidth*m_nGrabHeight);
  }
}

void noadData::clearPics()
{
  for( int i = 0; i < NUMPICS; i++)
  {
    if( video_buffer_mem2[i] != NULL )
      delete [] video_buffer_mem2[i];
    if( video_buffer_mem3[i] != NULL )
      delete [] video_buffer_mem3[i];
  }
  iCompPics = 0;
}

int countBits(unsigned char c)
{
  int iRet = 0;
  for( int i = 0; i < 8; i++)
  {
    if( c&1)
      iRet++;
    c>>=1;
  }
  return iRet;
}

int noadData::checkPics()
{
  int iRet = 0;
  int size = m_nGrabWidth*m_nGrabHeight;
  int iDelta = 0;
  int iCount = 0;
  storeCompPic();
  if( iCompPics < NUMPICS-1 )
    return 2000000000;
  for( int i = 0; i < NUMPICS; i++)
  {
    if( video_buffer_mem2[i] != NULL )
    {
      iCount++;
      iDelta = 0;
      for( int j = 0; j < size; j++ )
       //iDelta += countBits(video_buffer_mem2[i][j] ^ video_buffer_mem[j]);
       iDelta += abs(video_buffer_mem2[i][j]-(video_buffer_mem[j]&video_buffer_mem2[i][j]));
      iDelta /= size;
      iRet += iDelta;
    }
  }
  return iRet/iCount;
}

void noadData::modifyPic(int /*framenum*/,unsigned char **_src)
{
#define MINMAX(a,b) { if(a##Min > b ) a##Min = b; if( a##Max < b) a##Max = b; }
  int size = m_nGrabWidth*m_nGrabHeight;
  int uvsize = (m_nGrabWidth/2)*(m_nGrabHeight/2);
  int width = m_nGrabWidth;
  int *iSet = new int[256];
  int *iSet2 = new int[256*3];
  int yMin = 255;
  int yMax = 0;
  int uMin = 255;
  int uMax = 0;
  int vMin = 255;
  int vMax = 0;
  int cMin = 255*3;
  int cMax = 0;
  unsigned char *src = _src[0];
  unsigned char *usrc = _src[1];
  unsigned char *vsrc = _src[2];
  memset( iSet,0,256*sizeof(int));
  memset( iSet2,0,256*3*sizeof(int));

  int ySum = 0;
  int uSum = 0;
  int vSum = 0;
  int iColSum = 0;
  for(int i = width*2; i < size-2*width; i++)
  {
    iSet[src[i]] = 1;
    MINMAX(y,src[i]);
    ySum += src[i];
    int y = src[i];
    int uvIndex = (i/width)*width/2+(i%width)/2;
    int u = usrc[uvIndex];
    int v = vsrc[uvIndex];
    int color = y+u+v;
    MINMAX(c,color);
    iSet2[color] = 1;
    iColSum += (color/3);//((y+u+v)/3);
  }
  iColSum /= size;
  int iyCount = 0;
  for( int i = 0; i < 256;i++)
    iyCount += iSet[i];
  int icCount = 0;
  for( int i = 0; i < 256*3;i++)
    icCount += iSet2[i];

  memset( iSet,0,256*sizeof(int));
  src = _src[1];
  for(int i = width/2; i < uvsize-width/2; i++)
  {
    iSet[src[i]] = 1;
    MINMAX(u,src[i]);
    uSum += src[i];
  }
  int iuCount = 0;
  for( int i = 0; i < 256;i++)
    iuCount += iSet[i];

  memset( iSet,0,256*sizeof(int));
  src = _src[2];
  for(int i = width/2; i < uvsize-width/2; i++)
  {
    iSet[src[i]] = 1;
    MINMAX(v,src[i]);
    vSum += src[i];
  }
  int ivCount = 0;
  for( int i = 0; i < 256;i++)
    ivCount += iSet[i];

//  //dsyslog(LOG_INFO, "dif values in pic %d: s-count %3d",
//    framenum, iyCount+iuCount+ivCount);
/*
  //dsyslog(LOG_INFO, "dif values in pic %d: y-diff %3d y-count %3d u-diff %3d u-count %3d v-diff %3d v-count %3d c-diff %3d c-count %3d s-count %3d",
    framenum,yMax-yMin,iyCount,uMax-uMin,iuCount,vMax-vMin,ivCount,cMax-cMin,icCount, iyCount+iuCount+ivCount);
*/
/*
  //dsyslog(LOG_INFO, "dif values in pic %d: y %3d %3d %3d u %3d %3d %3d v %3d %3d %3d c = %3d %3d %3d %3d",
    framenum,yMin,yMax,iyCount,uMin,uMax,iuCount,vMin,vMax,ivCount,cMin,cMax,iColSum,icCount );
*/
/*
  //dsyslog(LOG_INFO, "dif values in line %d: y %d u %d v %d  sum %d dy=%d, du=%d,dv=%d, f=%d, colsum=%d,dCol=%d",
    line,iyCount,iuCount,ivCount,iyCount+iuCount+ivCount,
    ySum/width-(border), uSum/width/2, vSum/width/2,
    iFaktor,iColSum,iColSum/(width-(border)));
*/
/*
  //dsyslog(LOG_INFO, "dif in line %d: iycount %d f=%d, colsum=%d,dCol=%d",
    line,iyCount,iFaktor,iColSum,iColSum/(width-(border)));
*/
  delete [] iSet;
  delete [] iSet2;
/*
  if( video_buffer_mem2[0] != NULL )
  {
    int size = m_nGrabWidth*m_nGrabHeight;
    for( int i = 0; i < size; i++)
      dest[0][i] = dest[0][i] | video_buffer_mem2[0][i];
  }
*/
/*
  if( video_buffer_mem2 != NULL )
  {
    int size = m_nGrabWidth*m_nGrabHeight;
    for( int i = 0; i < size; i++)
      dest[0][i] = dest[0][i] & video_buffer_mem2[i];
  }
*/
}

void logline( unsigned char *src, int width)
{
  char *buf;
  buf = new char[width*5];
  buf[0] = '\0';
  for( int i = 0; i < width; i++ )
     sprintf(&buf[i*3],"%02X ",src[i]);
  //dsyslog(LOG_INFO, "line %s",buf);
  delete [] buf;
}

void logcdiffs(unsigned char *src, int width, char *text)
{
  int iMin = 999;
  int iMax = 0;
  for( int i = 0; i < width; i++ )
  {
    if( src[i] < iMin ) iMin = src[i];
    if( src[i] > iMax ) iMax = src[i];
  }
  //dsyslog(LOG_INFO, "%s min = %d, max = %d, diff = %d",text,iMin,iMax, iMax-iMin);
}

#ifdef VNOAD
void rgbline(unsigned char *_src, int /*line*/, int width, int height)
{
  char *buf;
  buf = new char[4096];
  buf[0] = '\0';
  unsigned char *y = _src;
  unsigned char *u = _src + (width*height);
  unsigned char *v = u + (width*height/4);
  int r;
  int g;
  int b;
  for( int i = 0; i < 10; i++ )
  {
    r = y[i] + v[i/2];
    g = y[i] - 0.166667*u[i/2] - 0.5*v[i/2];
    b = y[i] + u[i/2];
    //dsyslog(LOG_INFO, "rgbline: %3d %3d %3d  -->  %3d %3d %3d",y[i],u[i/2],v[i/2],r,g,b);
    sprintf( &buf[strlen(buf)], "%02X%02X%02X ", r,g,b);
  }
  for( int i = width/2-5; i < width/2+5; i++ )
  {
    r = y[i] + v[i/2];
    g = y[i] - 0.166667*u[i/2] - 0.5*v[i/2];
    b = y[i] + u[i/2];
    //dsyslog(LOG_INFO, "rgbline: %3d %3d %3d  -->  %3d %3d %3d",y[i],u[i/2],v[i/2],r,g,b);
    sprintf( &buf[strlen(buf)], "%02X%02X%02X ", r,g,b);
  }
  for( int i = width-10; i < width; i++ )
  {
    r = y[i] + v[i/2];
    g = y[i] - 0.166667*u[i/2] - 0.5*v[i/2];
    b = y[i] + u[i/2];
    //dsyslog(LOG_INFO, "rgbline: %3d %3d %3d  -->  %3d %3d %3d",y[i],u[i/2],v[i/2],r,g,b);
    sprintf( &buf[strlen(buf)], "%02X%02X%02X ", r,g,b);
  }
  //dsyslog(LOG_INFO, "rgbline: %s",buf);
  delete [] buf;
}

int orgcountvals(unsigned char *_src[3], int line, int width, int /*height*/)
{
  int *iSet = new int[width*2];
  unsigned char *src = _src[0]+width*line;
  memset( iSet,0,width*2*sizeof(int));
  for(int i = 0; i < width; i++)
    iSet[src[i]] = 1;
  int iyCount = 0;
  for( int i = 0; i < width;i++)
    iyCount += iSet[i];
  memset( iSet,0,width*sizeof(int));
  src = _src[1]+width/2*line/2;
  for(int i = 0; i < width/2; i++)
    iSet[src[i]] = 1;
  int iuCount = 0;
  for( int i = 0; i < width/2;i++)
    iuCount += iSet[i];
  memset( iSet,0,width*sizeof(int));
  src = _src[2]+width/2*line/2;
  for(int i = 0; i < width/2; i++)
    iSet[src[i]] = 1;
  int ivCount = 0;
  for( int i = 0; i < width/2;i++)
    ivCount += iSet[i];
//  //dsyslog(LOG_INFO, "dif values in line %d: y %d u %d v %d  sum %d",line,iyCount,iuCount,ivCount,iyCount+iuCount+ivCount);
  delete [] iSet;
//  return (iyCount+iuCount+ivCount)/3;
  return iyCount;
}

void linesum(unsigned char *_src, int line, int width, int height)
{
  int iSum = 0;
  unsigned char *src = _src;
  src += line*width;
  for( int i = 0; i < width; i++ )
  {
    iSum += src[i]-16;
  }
  src = _src + (width*height) + line*width/2;
  for( int i = 0; i < width/2; i++ )
  {
    iSum += src[i];
  }
  src = _src + (width*height) + line*width + line*width/2;
  for( int i = 0; i < width/2; i++ )
  {
    iSum += src[i];
  }
  //dsyslog(LOG_INFO, "linesum(%d) = %d",line,iSum);
}


void noadData::testFunc(int framenum,unsigned char **_src)
{
  checkMonoFrame( framenum, _src);
  /*
  for( int i = m_nGrabHeight-74; i < m_nGrabHeight-74+10; i++)
  {
    fprintf(stderr, "line %03d: ",i);
    unsigned char *lsrc = _src[0]+(i*m_nGrabWidth);
    for(int j = 0; j < 50; j++)
      fprintf(stderr, "%02X ",lsrc[j]);
    fprintf(stderr, "\n");
  }
  fprintf(stderr, "\n");
  */
}
#endif

#endif
