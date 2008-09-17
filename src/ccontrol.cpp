/***************************************************************************
                          ccontrol.cpp  -  description                              
                             -------------------                                         
    begin                : Tue Aug 3 1999                                           
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


//#include <syslog.h>
#include <stdlib.h>

#include "ccontrol.h"

extern int SysLogLevel;

//#define esyslog(a...) void( (SysLogLevel > 0) ? syslog(a) : void() )
//#define isyslog(a...) void( (SysLogLevel > 1) ? syslog(a) : void() )
//#define //dsyslog(a...) void( (SysLogLevel > 2) ? syslog(a) : void() )

CControl::CControl( noadData* data )
{
//  //dsyslog(LOG_INFO, "CControl::CControl");
  m_pData = data;	

  /** reset some values */
  m_nPictureCounter = 0;
  m_bFound = false;
  m_nLogoCorner = UNKNOWN;

  /** create CGetLogos */
  m_pGetLogo0 = new CGetLogo( this, m_pData, m_pData->m_chGreyCorner0, TOP_LEFT );
  m_pGetLogo1 = new CGetLogo( this, m_pData, m_pData->m_chGreyCorner1, TOP_RIGHT );
  m_pGetLogo2 = new CGetLogo( this, m_pData, m_pData->m_chGreyCorner2, BOT_RIGHT );
  m_pGetLogo3 = new CGetLogo( this, m_pData, m_pData->m_chGreyCorner3, BOT_LEFT );


  bCheckLogo0 = true;
  bCheckLogo1 = true;
  bCheckLogo2 = true;
  bCheckLogo3 = true;
  #ifdef NOAD_VIEW
  open_display();
  #endif
}

#define DELETENULL(p) (delete (p), p = NULL)

CControl::~CControl()
{
//  //dsyslog(LOG_INFO, "CControl::~CControl");
  #ifdef NOAD_VIEW
  x11_close();
  #endif
  DELETENULL( m_pGetLogo3);
  DELETENULL(m_pGetLogo2);
  DELETENULL(m_pGetLogo1);
  DELETENULL(m_pGetLogo0);
}

void CControl::foundLogo0( struct testlines* tl ) {

  m_pData->m_nLogoCorner = m_nLogoCorner = TOP_LEFT;
  m_pData->m_bFound = m_bFound = true;
	
  m_pData->m_pCheckLogo->setLineHook(tl);
  m_pData->m_pCheckLogo->setCornerData(m_pData->m_chGreyCorner0);
}

void CControl::foundLogo1( struct testlines* tl )
{
  m_pData->m_nLogoCorner = m_nLogoCorner = TOP_RIGHT;
  m_pData->m_bFound = m_bFound = true;

  m_pData->m_pCheckLogo->setLineHook(tl);
  m_pData->m_pCheckLogo->setCornerData(m_pData->m_chGreyCorner1);
}

void CControl::foundLogo2( struct testlines* tl )
{
  m_pData->m_nLogoCorner = m_nLogoCorner = BOT_RIGHT;
  m_pData->m_bFound = m_bFound = true;
  m_pData->m_pCheckLogo->setLineHook(tl);
  m_pData->m_pCheckLogo->setCornerData(m_pData->m_chGreyCorner2);
}

void CControl::foundLogo3( struct testlines* tl )
{
  m_pData->m_nLogoCorner = m_nLogoCorner = BOT_LEFT;
  m_pData->m_bFound = m_bFound = true;
  m_pData->m_pCheckLogo->setLineHook(tl);
  m_pData->m_pCheckLogo->setCornerData(m_pData->m_chGreyCorner3);
}
void CControl::foundLogo( struct testlines* tl, int iCornerID )
{
	switch(iCornerID)
	{
		case TOP_LEFT:		foundLogo0(tl); break;
		case TOP_RIGHT:	foundLogo1(tl); break;
		case BOT_RIGHT:	foundLogo2(tl); break;
		case BOT_LEFT:		foundLogo3(tl); break;
	}
}


void CControl::onCtrlReset() {

  /*
  if ( !m_pData->m_bStart ) {
     m_pData->m_bStart = true;
     m_pData->m_bStop = false;
  }
	*/
  m_pGetLogo0->reset();
  m_pGetLogo1->reset();
  m_pGetLogo2->reset();
  m_pGetLogo3->reset();

  if ( m_bFound )
    m_pData->m_pCheckLogo->reset();

  m_bFound = false;
}

/** called to stop logo detection  */
void CControl::stop()
{
  /*
  if ( !m_pData->m_bStop ) {
     //m_pRemote->sendCommandSequence( m_pDoc->nologo_command );
     m_pData->m_bStart = false;
     m_pData->m_bStop = true;
  }
	*/
	
  m_pGetLogo0->reset();
  m_pGetLogo1->reset();
  m_pGetLogo2->reset();
  m_pGetLogo3->reset();

  if ( m_bFound )
     m_pData->m_pCheckLogo->reset();

  m_bFound = false;

}



/** called from timerEvent */
void CControl::newData()
{
	////dsyslog(LOG_INFO, "CControl::GrabImage");
  m_pData->setCorners();
  m_nPictureCounter++ ;

  /** call the get logo object after soft filter has init */
  if ( m_nPictureCounter > 2 && !m_bFound ) {
         if( bCheckLogo0 ) m_pGetLogo0->newData();
         if( bCheckLogo1 ) m_pGetLogo1->newData();
         if( bCheckLogo2 ) m_pGetLogo2->newData();
         if( bCheckLogo3 ) m_pGetLogo3->newData();
  }
  if ( m_bFound )
    m_pData->m_pCheckLogo->newData();

}

void CControl::forceCorner( int iNewCorner )
{
//	//dsyslog(LOG_INFO, "CControl::forceCorner %d", iNewCorner);
	if( iNewCorner >= 0 && iNewCorner <= 3 )
	{
		bCheckLogo0 = iNewCorner == 0;
	  	bCheckLogo1 = iNewCorner == 1;
	  	bCheckLogo2 = iNewCorner == 2;
	  	bCheckLogo3 = iNewCorner == 3;
		onCtrlReset();
	}
}

#ifdef NOAD_VIEW
int CControl::open_display ()
{


display = XOpenDisplay(NULL);
window = XCreateSimpleWindow(display, RootWindow(display, 0), 1, 1, 500, 500, 0, BlackPixel (display, 0), BlackPixel(display, 0));
XMapWindow(display, window);
XFlush(display);



/*
    int major;
    int minor;
    Bool pixmaps;
    XVisualInfo visualTemplate;
    XVisualInfo * XvisualInfoTable;
    XVisualInfo * XvisualInfo;
    int number;
    int i;
    XSetWindowAttributes attr;
    XGCValues gcValues;

    display = XOpenDisplay (NULL);
    if (! (display)) {
	fprintf (stderr, "Can not open display\n");
	return 1;
    }

    if ((XShmQueryVersion (display, &major, &minor,
			   &pixmaps) == 0) ||
	(major < 1) || ((major == 1) && (minor < 1))) {
	fprintf (stderr, "No xshm extension\n");
	return 1;
    }

    completion_type =
	XShmGetEventBase (display) + ShmCompletion;

    // list truecolor visuals for the default screen
    visualTemplate.c_class = TrueColor;
    visualTemplate.screen = DefaultScreen (display);
    XvisualInfoTable = XGetVisualInfo (display,
				       VisualScreenMask | VisualClassMask,
				       &visualTemplate, &number);
    if (XvisualInfoTable == NULL) {
	fprintf (stderr, "No truecolor visual\n");
	return 1;
    }

    // find the visual with the highest depth
    XvisualInfo = XvisualInfoTable;
    for (i = 1; i < number; i++)
	if (XvisualInfoTable[i].depth > XvisualInfo->depth)
	    XvisualInfo = XvisualInfoTable + i;

    vinfo = *XvisualInfo;
    XFree (XvisualInfoTable);

    attr.background_pixmap = None;
    attr.backing_store = NotUseful;
    attr.border_pixel = 0;
    attr.event_mask = 0;
    // fucking sun blows me - you have to create a colormap there...
    attr.colormap = XCreateColormap (display,
				     RootWindow (display,
						 vinfo.screen),
				     vinfo.visual, AllocNone);
    window =
	XCreateWindow (display,
		       DefaultRootWindow (display),
		       0 , 0 , m_pData->m_nSizeX, m_pData->m_nSizeY,
		       0 , vinfo.depth,
		       InputOutput, vinfo.visual,
		       (CWBackPixmap | CWBackingStore | CWBorderPixel |
			CWEventMask | CWColormap), &attr);

    gc = XCreateGC (display, window, 0, &gcValues);
*/
    return 0;
}

//static int shmerror = 0;

int handle_error (Display * display, XErrorEvent * error)
{
//    shmerror = 1;
    return 0;
}

void * CControl::create_shm (int size)
{
    shminfo.shmid = shmget (IPC_PRIVATE, size, IPC_CREAT | 0777);
    if (shminfo.shmid == -1)
	goto error;

    (void *)(shminfo.shmaddr) = shmat (shminfo.shmid, 0, 0);
    if (shminfo.shmaddr == (char *)-1)
	goto error;

    /* on linux the IPC_RMID only kicks off once everyone detaches the shm */
    /* doing this early avoids shm leaks when we are interrupted. */
    /* this would break the solaris port though :-/ */
    /* shmctl (instance->shminfo.shmid, IPC_RMID, 0); */

    /* XShmAttach fails on remote displays, so we have to catch this event */

    XSync (display, False);
    XSetErrorHandler (handle_error);

    shminfo.readOnly = True;
    if (! (XShmAttach (display, &(shminfo))))
	shmerror = 1;

    XSync (display, False);
    XSetErrorHandler (NULL);
    if (shmerror) {
    error:
	fprintf (stderr, "cannot create shared memory\n");
	return NULL;
    }

    return shminfo.shmaddr;
}

void CControl::destroy_shm ()
{
    XShmDetach (display, &(shminfo));
    shmdt (shminfo.shmaddr);
    shmctl (shminfo.shmid, IPC_RMID, 0);
}

void CControl::x11_event ()
{
    XEvent event;
    char * addr;
    int i;

    XNextEvent (display, &event);
    if (event.type == completion_type) {
	addr = (shminfo.shmaddr +
		((XShmCompletionEvent *)&event)->offset);
/*???
	for (i = 0; i < 3; i++)
	    if (addr == frame[i].ximage->data)
		frame[i].wait_completion = 0;
*/
    }
}

void CControl::x11_close ()
{
//    x11_instance_t * instance = (x11_instance_t *) _instance;
    int i;

//!!!    libvo_common_free_frames ((vo_instance_t *) instance);
    for (i = 0; i < 3; i++) {
//	while (frame[i].wait_completion)
//	    x11_event (instance);
//	XDestroyImage (instance->frame[i].ximage);
    }
//    destroy_shm ();

//    XFreeGC (display, gc);
    XDestroyWindow (display, window);
    XCloseDisplay (display);
}


#endif
