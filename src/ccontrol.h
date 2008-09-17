/***************************************************************************
                          ccontrol.h  -  description                              
                             -------------------                                         
    begin                : Tue Aug 3 1999                                           
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


#ifndef CCONTROL_H
#define CCONTROL_H
/**
  *@author Thorsten Janke
  */
#include "noaddata.h"
#include "cgetlogo.h"
#include "cchecklogo.h"


#undef NOAD_VIEW

#ifdef NOAD_VIEW
extern "C"
{
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#include <inttypes.h>
/* since it doesn't seem to be defined on some platforms */
int XShmGetEventBase (Display *);
}
#endif

class CControl
{

public:
  /** constructor of class gets the document pointer and
    * main view pointer
    */
  CControl( noadData* data );
  /** destructor of CControl */
  ~CControl();
  /** pointer to the document */
  //MyNoadDoc* m_pDoc;
  noadData* m_pData;
  /** pointer to get logo object bot left */
  CGetLogo* m_pGetLogo3;
  /** pointer to get logo object bot right */
  CGetLogo* m_pGetLogo2;
  /** pointer to get logo object top right */
  CGetLogo* m_pGetLogo1;
  /** pointer to get logo object top left */
  CGetLogo* m_pGetLogo0;
  bool bCheckLogo0;
  bool bCheckLogo1;
  bool bCheckLogo2;
  bool bCheckLogo3;
  /** counter for sent out picture */
  int m_nPictureCounter;
  /** struct of detected logo informations */
  testlines* m_testhook;
  /** detected logo corner */
  int m_nLogoCorner;
  /**  is set to true if logo has found */
  bool m_bFound;
  /** true to stop or another no logo function */
  bool m_bStop;
  /** true to record or another logo function */
  bool m_bStart;

  /** called when timer is down
   * get new data and call newData() in cgetlogo objects
   */
  	void newData();
	void forceCorner( int iNewCorner );
  void foundLogo( struct testlines*, int iCornerID );

  #ifdef NOAD_VIEW
    /* X11 related variables */
    Display * display;
    Window window;
    GC gc;
    XVisualInfo vinfo;
    XShmSegmentInfo shminfo;
    int completion_type;

	int open_display ();
	void x11_close ();
	int shmerror;

//	int handle_error (Display * display, XErrorEvent * error);

	void * create_shm (int size);
	void destroy_shm ();
	void x11_event ();
  #endif

private:

  /** called if logo has found in corner top left */
  void foundLogo0( struct testlines* );
  /** called if logo has found in corner top right */
  void foundLogo1( struct testlines* );
  /** called if logo has found in corner bot right */
  void foundLogo2( struct testlines* );
  /** called if logo has found in corner top left */
  void foundLogo3( struct testlines* );
  /** slot called from checklogo if logo is available */
  void logo();
  /** slot called from checklogo if there is no logo */
  void nologo();
	void onCtrlReset();
	void stop();

};

#endif
