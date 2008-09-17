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

#pragma once
#include "noaddata.h"
#include "ccontrol.h"
#include "inc.h"
#include "ffmpeg_movie.h"

#define CHECKLOGO_PERCENT	0.5
#define DETECT_LENGTH		60
#define STABLE_TIME			20

class CAdvertisement {
public:

	struct SCutInfo {
		UINT numAds;
		struct DATA {
			uint64_t averDur, start; } *data;
	};

private:
	noadData *pData;
	void	 *lastYUVBuf;
	CControl *cctrl;
	bool bLogo,bAudio;
	CFFMPEGLoader *Movie;

	struct SStates {
		int audio;
		int video;
		SStates(){Init();}
		void Init() {audio=video=0;}
		void SetAdAudioBegins() {audio++;}
		void SetAdVideoBegins() {video++;}
		void SetAdAudioStops()  {audio--;}
		void SetAdVideoStops()  {video--;}
	};
	struct SAdData {
		UINT ulTopBlackLines,ulBotBlackLines,checkedFrames;
		UINT blackFrames;
		bool bBlackFrame;
		void Init(noadData *pData,CControl **cctrl,UINT ulTopBlL, UINT ulBotBlL);
	};
	struct SCut {
		uint64_t frame;
		uint64_t duration;	//in frames
	};

	class CCutlist {
	private:
		static UINT sNum;
		static UINT sSize;
		CTemplateBuffer<SCut> List;
		bool bStart;
		bool bMarkedCut;
	public:
		CCutlist():List(&sSize,&sNum) {bStart=true;bMarkedCut=true;}
		void Save(const char *filename, const char *moviename, const double &fps);
		void Save(string *save, const char *moviename, const double &fps);

		void AddStart(const uint64_t &start);
		void AddEnd(const uint64_t &end);
		void Add(const uint64_t &v);
		void Convert(const uint64_t &dur, const bool &markedKeep);
	};

	SAdData AdvertData;
	CCutlist Cutlist;
	string  FileName;

	bool createLogo();
	bool doLogoDetection(const UINT startSec, const UINT lengthSec);
	double checkLogo(const UINT &startSec, const UINT &lengthSec, const UINT &jumpSec, const UINT &numFrames, const UINT &jumpFrames, const uint64_t &startFrame=0, const bool &bExact=false);
	double checkLogo2(const UINT &startSec, const UINT &lengthSec, const UINT &jumpSec, const UINT &numFrames, const UINT &jumpFrames, const uint64_t &startFrame=0, const bool &bExact=false);
	bool checkAudio(const UINT startSec, const UINT lengthSec, const UINT jumpSec, const UINT numFrames, const UINT jumpFrames);
	void checkCallback( int width, int height, void *yufbuf );
	void drawCallback ( int width, int height, void *yufbuf );
	bool StdCallBack  ( int width, int height, void *yuvbuf );
	bool detectChange(const UINT iSec, const UINT iJump);
	//void createCuts(const UINT &minAdvertLength, const UINT &movieLength);	//in seconds
	//void AddCut(const SCut &cut);
	uint64_t exactChange(const uint64_t &guessed, const UINT &arround, const bool &bOnlyExact=false);
	UINT IsStable(const uint64_t &frame, const bool &bStart, const bool &bBlack);
	uint64_t checkRecursive(const bool &bOld, const bool &bNew, const uint64_t &pos, const UINT &duration);
	uint64_t checkRecursive2(const bool &bStart, const uint64_t &st, const uint64_t &end, const float &a, const float &b);
	bool Init(const char *filename);
	void Delete();
public:

	void ScanAdvert(const char *filename, const UINT &minAdvertLength);
	void CreatePropMap(const char *filename, const UINT &num, float *array, Progress *progress);
	void Save(const char *filename);
	void Save(string *save);

	//CONSOLE FUNCTIONS
	bool CreateCutlist(const char *filename, const char *cutlist, const string &author, const UINT &minAdvertLength, const UINT &security, const int &start, const int &end, const UINT &jumpAfter, const bool &bScreenshot, const UINT &screens_dis, const UINT &screens_num, const UINT &screens_width, const UINT &screens_height);
};
