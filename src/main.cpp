/***************************************************************************
author: Joshua Hampp
date: 2008/01

property of Joshua Hampp
***************************************************************************/

/*
Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

   1. Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
   2. Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
   3. The name of the author may not be used to endorse or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE AUTHOR `Joshua Hampp`AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED 
TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL
THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
POSSIBILITY OF SUCH DAMAGE.
*/

#include "ad_main.h"

int av_log_level;
AVCodec *first_avcodec;

#ifndef _LIB

#include <string>

string GL_filename, GL_author, GL_cutlist;
int GL_start_handling=0, GL_end_handling=0;
UINT GL_safety=0, GL_jump_after=5*60, GL_step=150, GL_screens_num=1, GL_screens_dis=1, GL_screens_width=0, GL_screens_height=0;
bool GL_bScreeshot=false;
#ifndef WIN32
enum _WHICH {_W_NONE,_W_FILENAME,_W_AUTHOR,_W_START_H,_W_END_H,_W_CUTLIST,_W_STEP,_W_SAFETY,_W_JUMP_AFTER,_W_SCREENSHOTS,_W_SCREENSHOTS_SETTINGS};
#endif

bool Analyze(const int &argc, const char *argv[]) {
#ifndef WIN32
	_WHICH wh=_W_NONE;
#else
	enum _WHICH {_W_NONE,_W_FILENAME,_W_AUTHOR,_W_START_H,_W_END_H,_W_CUTLIST,_W_STEP,_W_SAFETY,_W_JUMP_AFTER,_W_SCREENSHOTS,_W_SCREENSHOTS_SETTINGS} wh=_WHICH::_W_NONE;
#endif

	for(UINT i=1; i<argc; i++) {
		string cmd = argv[i];

		if(cmd=="-f"||cmd=="--filename")
			wh=_W_FILENAME;
		else if(cmd=="-a"||cmd=="--author")
			wh=_W_AUTHOR;
		else if(cmd=="-s"||cmd=="--start_handling")
			wh=_W_START_H;
		else if(cmd=="-e"||cmd=="--end_handling")
			wh=_W_END_H;
		else if(cmd=="-c"||cmd=="--cutlist")
			wh=_W_CUTLIST;
		else if(cmd=="-d"||cmd=="--advert_distance")
			wh=_W_STEP;
		else if(cmd=="-c"||cmd=="--safety")
			wh=_W_SAFETY;
		else if(cmd=="-j"||cmd=="--jump_after")
			wh=_W_JUMP_AFTER;
		else if(cmd=="-r"||cmd=="--screenshots")
			wh=_W_SCREENSHOTS;
		else if(cmd=="--screenshots-settings")
			wh=_W_SCREENSHOTS_SETTINGS;
		else if(cmd=="-h"||cmd=="--help") {
			wh=_W_NONE;

			cout<<" Advertisement Scanner by @Hampp, Joshua, 2008 "<<endl;
			cout<<"-----------------------------------------------"<<endl;
			cout<<"using noad and ffmepg, thanks!"<<endl<<endl;
			cout<<"Commandline Parameters:"<<endl;

			cout<<endl<<"-f --filename"<<endl;
			cout<<"\tfilename of video file"<<endl;

			cout<<endl<<"-a --author"<<endl;
			cout<<"\tauthor of cutlist, optional"<<endl;

			cout<<endl<<"-c --cutlist"<<endl;
			cout<<"\tname of cutlist, optional"<<endl;

			cout<<endl<<"-s --start_handling"<<endl;
			cout<<"\thow the beginning of the movie is handled (never, always, intelligent), optional"<<endl;
			cout<<"\tstandard: never"<<endl;

			cout<<endl<<"-e --end_handling"<<endl;
			cout<<"\thow the end of the movie is handled (never, always, intelligent), optional"<<endl;
			cout<<"\tstandard: never"<<endl;

			cout<<endl<<"-d --advert_distance"<<endl;
			cout<<"\tstep width which is used to scan video in seconds, optional"<<endl;
			cout<<"\tstandard: "<<GL_step<<" sec."<<endl;

			cout<<endl<<"-c --safety"<<endl;
			cout<<"\tadds n seconds before or after cut, optional"<<endl;
			cout<<"\tstandard: "<<GL_safety<<" sec."<<endl;

			cout<<endl<<"-j --jump_after"<<endl;
			cout<<"\tafter a cut is finished jump over next n seconds, optional"<<endl;
			cout<<"\tstandard: "<<GL_jump_after<<" sec."<<endl;
			
			cout<<endl<<"-r --screenshots"<<endl;
			cout<<"\tcreates screenshots while scanning"<<endl;
			cout<<"\texample: --screenshots true"<<endl;
			cout<<"\tstandard: false"<<endl;

			cout<<endl<<"--screenshots-settings"<<endl;
			cout<<"\tset settings for screenshots"<<endl;
			cout<<"\t(NUM,DISTANCE IN SECONDS,WIDTH,HEIGHT)"<<endl;
			cout<<"\tNUM=number of screenshots per cut"<<endl;
			cout<<"\tDISTANCE=distance between shortcuts in seconds"<<endl;
			cout<<"\tWIDTH=width in pixels, original size = 0"<<endl;
			cout<<"\tHEIGHT=height in pixels, original size = 0"<<endl;
			cout<<"\tstandard: ("<<GL_screens_num<<","<<GL_screens_dis<<","<<GL_screens_width<<","<<GL_screens_height<<")"<<endl;
		}
		else {
			switch(wh) {
				case _W_FILENAME: {
					GL_filename=cmd;
				}break;
				case _W_AUTHOR: {
					GL_author=cmd;
				}break;
				case _W_CUTLIST: {
					GL_cutlist=cmd;
				}break;
				case _W_SCREENSHOTS: {
					GL_bScreeshot=(cmd=="true");
									 }break;
				case _W_SCREENSHOTS_SETTINGS: {
					UINT *ar[4]={&GL_screens_num,&GL_screens_dis,&GL_screens_width,&GL_screens_height};
					UINT pos=0;
					for(UINT pp=0; pp<4; pp++) {
						*ar[pp]=0;
						pos++;
						while(pos<cmd.length()) {
							if(!isdigit(cmd[pos])) break;
							*ar[pp]*=10;
							*ar[pp]+=cmd[pos++]-'0';
						}
					}
									 }break;
				case _W_START_H: {
					if(cmd=="always")
						GL_start_handling=1;
					else if(cmd=="intelligent")
						GL_start_handling=2;
					else
						GL_start_handling=0;
								 }break;
				case _W_END_H: {
					if(cmd=="always")
						GL_end_handling=1;
					else if(cmd=="intelligent")
						GL_end_handling=2;
					else
						GL_end_handling=0;
								 }break;
				case _W_STEP: {
					GL_step=0;
					UINT pos=0;
					while(pos<cmd.length()) {
						if(!isdigit(cmd[pos])) break;
						GL_step*=10;
						GL_step+=cmd[pos++]-'0';
					}
				}break;
				case _W_SAFETY: {
					GL_safety=0;
					UINT pos=0;
					while(pos<cmd.length()) {
						if(!isdigit(cmd[pos])) break;
						GL_safety*=10;
						GL_safety+=cmd[pos++]-'0';
					}
				}break;
				case _W_JUMP_AFTER: {
					GL_jump_after=0;
					UINT pos=0;
					while(pos<cmd.length()) {
						if(!isdigit(cmd[pos])) break;
						GL_jump_after*=10;
						GL_jump_after+=cmd[pos++]-'0';
					}
				}break;
				default: {
					cout<<"ERROR: command not recognized\n";
				}break;
			}
		}
	}

	if(GL_step<=0) {
		cout<<"ERROR: advert_distance must be greater than 0\nuse --help for more information\n";
		return false;
	}

	if(GL_filename.length()<=0) {
		cout<<"ERROR: you must specify filename\nuse --help for more information\n";
		return false;
	}

	if(GL_cutlist.length()<=0)
		GL_cutlist=GL_filename+".cutlist";

	return true;
}

int main(int argc, char *argv[]) {
	
	//check arguments
	if(!Analyze(argc,(const char **)argv)) {
		cout<<"\nPlease check your settings, something went wrong\nGo on with any key...";
		getchar();
		return 1;
	}

	CAdvertisement Ad;

	cout<<"\nSCANNING, PLEASE WAIT\n\n";

	if(!Ad.CreateCutlist(GL_filename.c_str(),GL_cutlist.c_str(),GL_author,GL_step,GL_safety,GL_start_handling,GL_end_handling,GL_jump_after,GL_bScreeshot,GL_screens_dis,GL_screens_num,GL_screens_width,GL_screens_height)) {
		cout<<"\nPlease check your settings, something went wrong\nGo on with any key...";
		getchar();
		return 1;
	}

	return 0;
}
#endif
