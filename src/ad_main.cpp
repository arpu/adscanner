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

UINT	CAdvertisement::CCutlist::sNum	= 0;
UINT	CAdvertisement::CCutlist::sSize	= 0;

bool CAdvertisement::Init(const char *filename) {

	pData=new noadData();
	pData->initBuffer();

	//open video file
	Movie=new CFFMPEGLoader();
	Movie->Init();
	if(!Movie->Open(filename))
		return false;
	Movie->SetBufferSize(30);

	cctrl=NULL;

	bLogo=createLogo();

	return true;
}

void CAdvertisement::Delete() {
	Movie->Delete();
}

/*
void CAdvertisement::CreateCutlist() {
	CFFMPEGLoader Output;
	Output.Init();
	Output.CreateMovie("test2.avi",Movie->GetFormat(),Movie->GetVideoCon(),Movie->GetAudioCon());
	Output.CopyPacktes(Movie,0,25*100);
	Output.CloseWriting();
	Output.Delete();

	return;
	/*createCuts(DETECT_LENGTH,Movie->GetDuration());

	FILE *pFile=fopen("cut.cutlist","w");
	fputs("[General]\nApplication=Josh's Cut\n",pFile);
	fputs("Version=0.0.0.1\ncomment1=\n",pFile);
	fputs("IntendedCutApplicationName=VirtualDub\nIntendedCutApplication=Josh's Cut\n",pFile);
	fputs("IntendedCutApplicationVersion=0.0.0.1\nVDUseSmartRendering=0\n",pFile);
	fputs("NoOfCuts=4\n[Info]\nAuthor=autocreation\nRatingByAuthor=3\n",pFile);
	fputs("EPGError=0\nActualContent=\nMissingBeginning=0\n",pFile);
	fputs("MissingEnding=0\nMissingVideo=0\nMissingAudio=0\nOtherError=0\nOtherErrorDescription=\nSuggestedMovieName=Tr\nUserComment=\n",pFile);
	char buffer[512];
	UINT num=0;
	for(UINT i=0; i<iNumCuts; i++) { //last should be Add
		double d1=((UINT)pCuts[i].frame)/(double)Movie->GetFPS(),d2=((UINT)pCuts[i].duration)/(double)Movie->GetFPS();
		sprintf(buffer,"[Cut%d]\nStart=%f\nDuration=%f\n",i,d1,d2);
		fputs(buffer,pFile);
	}
	fclose(pFile);*//*
	bool *b=new bool[Movie->GetDuration()/DETECT_LENGTH+1];
	bAudio=false;
	bLogo=true;
	for(UINT i=0; i<Movie->GetDuration(); i+=DETECT_LENGTH) {
#ifdef _CONSOLE
		cout<<"\r"<<i*100/Movie->GetDuration()<<"%\n";
#endif
		b[i/DETECT_LENGTH] = detectChange(i,1)>CHECKLOGO_PERCENT;}


//	CFFMPEGLoader Output;
	Output.Init();
	Output.CreateMovie("test2.avi",Movie->GetFormat(),Movie->GetVideoCon(),Movie->GetAudioCon());
	
	cout<<endl;
	FILE *pFile=fopen("cut.cutlist","w");
	fputs("[General]\nApplication=Josh's Cut\n",pFile);
	fputs("Version=0.0.0.1\ncomment1=\n",pFile);
	fputs("IntendedCutApplicationName=VirtualDub\nIntendedCutApplication=Josh's Cut\n",pFile);
	fputs("IntendedCutApplicationVersion=0.0.0.1\nVDUseSmartRendering=0\n",pFile);
	fputs("NoOfCuts=4\n[Info]\nAuthor=autocreation\nRatingByAuthor=3\n",pFile);
	fputs("EPGError=0\nActualContent=\nMissingBeginning=0\n",pFile);
	fputs("MissingEnding=0\nMissingVideo=0\nMissingAudio=0\nOtherError=0\nOtherErrorDescription=\nSuggestedMovieName=Tr\nUserComment=\n",pFile);
	char buffer[512];
	UINT num=0;
	UINT end=0;
	double d1=0,d2=0;
	for(UINT i=0; i<Movie->GetDuration()/DETECT_LENGTH+1; i++) {
		if(b[i]==true) {
			UINT begin=i*DETECT_LENGTH;
			if(i!=0) {
				begin-=DETECT_LENGTH/2;
				//while(begin<Movie->GetDuration()&&checkLogo(begin+=5,5,1,2,1,0,true)<CHECKLOGO_PERCENT);
				//begin-=5;
				//while(begin<Movie->GetDuration()&&checkLogo(begin++,1,1,7,1)<CHECKLOGO_PERCENT);
				d1=exactChange((begin-DETECT_LENGTH/2)*Movie->GetFPS(),DETECT_LENGTH*Movie->GetFPS())/(double)Movie->GetFPS();

			}
			sprintf(buffer,"[Cut%d]\nStart=%f\n",num++,d1);
			fputs(buffer,pFile);
			for(; i<Movie->GetDuration()/DETECT_LENGTH+1; i++)
				if(!b[i]) break;
			end=i*DETECT_LENGTH;
			if(i!=0) {
				end-=DETECT_LENGTH/2;
				//while(end<Movie->GetDuration()&&checkLogo(end-=5,5,1,2,1,0,true)>CHECKLOGO_PERCENT);
				//end+=5;
				//while(end<Movie->GetDuration()&&checkLogo(end--,1,1,7,1)>CHECKLOGO_PERCENT);
				d2=exactChange((end-DETECT_LENGTH/2)*Movie->GetFPS(),DETECT_LENGTH*Movie->GetFPS())/(double)Movie->GetFPS();

				//Output.CopyPacktes(Movie,begin*Movie->GetFPS(),end*Movie->GetFPS());
			}
			sprintf(buffer,"Duration=%f\n",d2-d1);
			fputs(buffer,pFile);
		}
	}
	fclose(pFile);
	Output.CloseWriting();
	Output.Delete();

	for(UINT i=0; i<Movie->GetDuration()/DETECT_LENGTH+1; i++) {
		if(b[i]) cout<<"+";
		else	 cout<<" ";
	}
}*/

/*CAdvertisement::SStates*/bool CAdvertisement::detectChange(const UINT iSec, const UINT iJump) {
	/*CAdvertisement::SStates state;

	if(bAudio)
;//		checkAudio(&state,iSec,iJump);

	if(bLogo)
		/*state=checkLogo(iSec,DETECT_LENGTH,iJump,1,1);
*/
	return checkLogo(iSec,DETECT_LENGTH/12,DETECT_LENGTH/25,2,1)>0.5;
}

void CAdvertisement::SAdData::Init(noadData *pData,CControl **cctrl,UINT ulTopBlL, UINT ulBotBlL) {
	ulTopBlackLines = 0;
	ulBotBlackLines = 0;
	blackFrames		= 0;
	if(*cctrl)
		delete *cctrl;
	*cctrl = NULL;
	checkedFrames=0;
	pData->m_nTopLinesToIgnore = ulTopBlL;
	pData->m_nBottomLinesToIgnore = ulBotBlL;
	pData->initBuffer();
	pData->m_bFound = false;
	pData->m_nLogoCorner = UNKNOWN;
	*cctrl = new CControl(pData);
}

bool CAdvertisement::createLogo() {		//=detectLogo in Noad
	AdvertData.Init(pData,&cctrl,0,0);

	//TODO: search for saved logofiles

	//what's a pitty, nothing found so search one
	
	int64_t iAStartpos[] =	//change this to something like this: 30%*Length of movie, 50%...
	{
		Movie->GetDuration()/4,
	       4*Movie->GetDuration()/10, 5*Movie->GetDuration()/10, 6*Movie->GetDuration()/10,
	    0,   Movie->GetDuration()/10, 2*Movie->GetDuration()/10, 3*Movie->GetDuration()/10,
		   7*Movie->GetDuration()/10, 8*Movie->GetDuration()/10, 9*Movie->GetDuration()/10,
	    Movie->GetDuration(),
		-1
	};

	if(Movie->GetDuration()>12*60)
		iAStartpos[0]=11*60;

	UINT iPart=0;
	do {
		AdvertData.Init(pData,&cctrl,0,0);

		if( doLogoDetection(iAStartpos[iPart], (UINT)(3500/Movie->GetFPS())) ) {		//AT LEAST >80 (m_nCheckFrames!!!) NOAD checks 3000 frames!
			 //if( checkLogo2(  iAStartpos[iPart], 3500/Movie->GetFPS() ,3,1,1 )>CHECKLOGO_PERCENT ) {
//#error "ANEDERN nicht perfekt"
			UINT num=0,i=0;
			AdvertData.ulBotBlackLines/=AdvertData.checkedFrames;
			AdvertData.ulTopBlackLines/=AdvertData.checkedFrames;
			while(iAStartpos[i]>=0) {
				if(i==iPart) {i++;continue;}
				if( checkLogo(  iAStartpos[i], (UINT)(3500/Movie->GetFPS()) ,3,2,1 )>CHECKLOGO_PERCENT ) 
					num++;
				if(num>=2) break;
				i++;
			}
			if(num>=2) {
				 switch(pData->m_nLogoCorner) {
					 case TOP_LEFT: cout<<"top left\n"; break;
					 case TOP_RIGHT: cout<<"top right\n"; break;
					 case BOT_LEFT: cout<<"bottom left\n"; break;
					 case BOT_RIGHT: cout<<"bottom right\n"; break;
					 default: cout<<"ERROR\n"; break;
				 }
				 //CHANGE HERE pData->saveCheckData("logo.dat",true);
				 return true;
			 }
		}
		 iPart++;
	} while(iAStartpos[iPart]>=0&&iAStartpos[iPart]<Movie->GetDuration());

	return false;
}

bool CAdvertisement::doLogoDetection(const UINT startSec, const UINT lengthSec) {
	if(startSec>Movie->GetDuration()) return false;

	unsigned char *v_buffer=NULL,*yuv_buffer=NULL;
	UINT width=0,height=0;
	int l=0;

	Movie->UseVideo();
	Movie->ClearAudio();

	Movie->LoadFrame();
	for(UINT iCur=startSec; iCur<=Movie->GetDuration()&&iCur<=startSec+lengthSec&&AdvertData.checkedFrames<1000; iCur+=2) {
		//get movie data
		Movie->Seek(iCur,0,false);
		for(UINT j=0; j<Movie->GetFPS(); j++) {

			while( (l=Movie->ReadVideoData(&yuv_buffer,&width,&height))==0 )
				if(Movie->LoadFrame()<0) return false;
			if(l<0) break;

			//system("cls");
			//Movie->ShowFrameConsole(yuv_buffer);
			drawCallback(width,height,yuv_buffer);
			//if(AdvertData.checkedFrames%10==0)
			//	Movie->SaveFrame(AdvertData.checkedFrames,"ch\\");
		
			if(pData->m_bFound)  {
				Movie->UseAudio();
				return pData->m_bFound;
			}

		}
	}

	Movie->UseAudio();

	return pData->m_bFound;
}

double CAdvertisement::checkLogo2(const UINT &startSec, const UINT &lengthSec, const UINT &jumpSec, const UINT &numFrames, const UINT &jumpFrames, const uint64_t &startFrame, const bool &bExact)
{
	if(jumpFrames==0||jumpSec==0) return 0;

	UINT iLogosFound = 0, iChecked=0;
	unsigned char *v_buffer=NULL,*yuv_buffer=NULL;
	UINT width=0,height=0;
	int l=0;

	Movie->UseVideo();
	Movie->ClearAudio();

	Movie->LoadFrame();

	UINT blTop=0,blBottom=0;
	AdvertData.blackFrames=0;

	for(UINT i=startSec; i<=Movie->GetDuration()&& i<lengthSec+startSec; i+=jumpSec) {
		
		for(uint64_t j=startFrame; j<numFrames+startFrame; j+=jumpFrames) {
			//get movie data
			if(j==startFrame||jumpFrames!=1)
				Movie->Seek(i,j,bExact);

			while( (l=Movie->ReadVideoData(&yuv_buffer,&width,&height))==0 )
				if(Movie->LoadFrame()<0) return false;
			if(l<0) break;


			checkCallback(width,height,yuv_buffer);

			blTop+=pData->m_nBlackLinesTop;
			blBottom+=pData->m_nBlackLinesBottom;

			iChecked++;
			iLogosFound += pData->m_pCheckLogo->isLogo ? 1 : 0;
			AdvertData.blackFrames += AdvertData.bBlackFrame ? 1 : 0;
			/*if(pData->m_pCheckLogo->isLogo)
				Movie->SaveFrame(iChecked,"rec\\");
			else
				Movie->SaveFrame(iChecked,"adv\\");*/
		}
	}

	if(iChecked>0) {
		blTop/=iChecked;
		blBottom/=iChecked;
	}

	return iLogosFound/(double)iChecked/* >= CHECKLOGO_PERCENT*/;
}

double CAdvertisement::checkLogo(const UINT &startSec, const UINT &lengthSec, const UINT &jumpSec, const UINT &numFrames, const UINT &jumpFrames, const uint64_t &startFrame, const bool &bExact)
{
	if(jumpFrames==0||jumpSec==0) return false;

	UINT iLogosFound = 0, iChecked=0;
	unsigned char *v_buffer=NULL,*yuv_buffer=NULL;
	UINT width=0,height=0;
	int l=0;

	Movie->UseVideo();
	Movie->ClearAudio();

	Movie->LoadFrame();

	UINT blTop=0,blBottom=0;
	AdvertData.blackFrames=0;

	for(UINT i=startSec; i<=Movie->GetDuration()&& i<lengthSec+startSec; i+=jumpSec) {
		
		for(uint64_t j=startFrame; j<numFrames+startFrame; j+=jumpFrames) {
			//get movie data
			if(j==startFrame||jumpFrames!=1)
				Movie->Seek(i,j,bExact);

			while( (l=Movie->ReadVideoData(&yuv_buffer,&width,&height))==0 )
				if(Movie->LoadFrame()<0) return false;
			if(l<0) break;


			checkCallback(width,height,yuv_buffer);

			blTop+=pData->m_nBlackLinesTop;
			blBottom+=pData->m_nBlackLinesBottom;

			iChecked++;
			iLogosFound += pData->m_pCheckLogo->isLogo ? 1 : 0;
			AdvertData.blackFrames += AdvertData.bBlackFrame ? 1 : 0;
			/*if(pData->m_pCheckLogo->isLogo)
				Movie->SaveFrame(iChecked,"rec\\");
			else
				Movie->SaveFrame(iChecked,"adv\\");*/
		}
	}

	if(iChecked>0) {
		blTop/=iChecked;
		blBottom/=iChecked;
	}

	double f1=1,f2=1;
	if(blTop<AdvertData.ulTopBlackLines) 
		f1=blTop*0.8/AdvertData.ulTopBlackLines+0.2;//(height-2*AdvertData.ulTopBlackLines)/(double)height + 2*AdvertData.ulTopBlackLines/(double)height* blTop/(double)AdvertData.ulTopBlackLines;

	if(blBottom<AdvertData.ulBotBlackLines) 
		f2=blBottom*0.8/AdvertData.ulBotBlackLines+0.2;//f2=(height-2*AdvertData.ulBotBlackLines)/(double)height + 2*AdvertData.ulBotBlackLines/(double)height* blBottom/(double)AdvertData.ulBotBlackLines;

	//if(f2>f1) f1=f2;
	//f1=1;	//change

	return f1*f2*iLogosFound/(double)iChecked/* >= CHECKLOGO_PERCENT*/;
}


void CAdvertisement::checkCallback( int width, int height, void *yufbuf )
{
  if( StdCallBack( width, height, yufbuf) )
  {
	  AdvertData.bBlackFrame=false;	//inputs doesn't matter
	  if(pData->GetAvgBrightness(0,NULL)<17)
	AdvertData.bBlackFrame=true;	//inputs doesn't matter
	const unsigned char *temp[4];
	temp[0]=(unsigned char*)Movie->GetYUV()->data[0];
	temp[1]=(unsigned char*)Movie->GetYUV()->data[1];
	temp[2]=(unsigned char*)Movie->GetYUV()->data[2];
	temp[3]=(unsigned char*)Movie->GetYUV()->data[3];
    pData->detectBlackLines((unsigned char *)temp);
    pData->setCorners();
    pData->m_pCheckLogo->newData();
  }
}

void CAdvertisement::drawCallback( int width, int height, void *yufbuf )
{
  if( StdCallBack( width, height, yufbuf) )
  {
    if(cctrl == NULL )
    {
      cctrl = new CControl(pData);
    }
	const unsigned char *temp[4];
	temp[0]=(unsigned char*)Movie->GetYUV()->data[0];
	temp[1]=(unsigned char*)Movie->GetYUV()->data[1];
	temp[2]=(unsigned char*)Movie->GetYUV()->data[2];
	temp[3]=(unsigned char*)Movie->GetYUV()->data[3];
    pData->detectBlackLines((unsigned char *)temp);
	AdvertData.ulTopBlackLines += pData->m_nBlackLinesTop;
    AdvertData.ulBotBlackLines += pData->m_nBlackLinesBottom;
    AdvertData.checkedFrames++;
    pData->setCorners();
    if( pData->m_bFound == false )
    {
      cctrl->newData();
    }
    else
    {
      bool bOldState = pData->m_pCheckLogo->isLogo;
      pData->m_pCheckLogo->newData();
    }
    #ifdef VNOAD
    if( fCallBack )
      fCallBack(iCurrentDecodedFrame,buffer,width,height, yufbuf);
    #endif
  }
}


bool CAdvertisement::StdCallBack(int width, int height, void *yuvbuf )
{
//  totalDecodedFrames++;
//  lastYUVBuf = yuvbuf;
  
  if( pData && yuvbuf)
  {
    // bei Änderung der Bildgrösse daten zurücksetzen
    if( width != pData->m_nGrabWidth || height != pData->m_nGrabHeight )
    {
      delete cctrl;
      cctrl = NULL;
      pData->setGrabSize( width, height );
    }
    pData->setExternalMem(yuvbuf);
    return true;
  }
  return false;
}

/*
void CAdvertisement::AddCut(const CAdvertisement::SCut &cut) {
	pCuts = (SCut*)realloc(pCuts,sizeof(SCut)*(++iNumCuts));
	pCuts[iNumCuts-1]=cut;
}

void CAdvertisement::createCuts(const UINT &minAdvertLength, const UINT &movieLength) {
	//Movie should begin 3 Minutes after start!
	SCut temp;
	UINT pos=0;

	for(int i=6*60; i>0; i-=6) {
		if(checkLogo(i-6,6,1,24,12)<CHECKLOGO_PERCENT) {
			pos=i;
			temp.frame=0;
			temp.duration=i*Movie->GetFPS();
			AddCut(temp);
			cout<<"found Advert\n";
			break;
		}
	}
	/*const UINT l_arr=6*2;
	double arr[l_arr];
	double average=0;

	for(UINT i=0; i<l_arr; i++) {
		arr[i]=checkLogo(i*30,30,6,24,12);
		average+=arr[i]/l_arr;}

	double max=CHECKLOGO_PERCENT*CHECKLOGO_PERCENT/4;
	UINT pos=0;
	for(UINT i=1; i<l_arr-1; i++) {
		if((arr[i]-arr[i-1])*(arr[i]-arr[i+1])>max) {
			pos=i;
			max=(arr[i]-arr[i-1])*(arr[i]-arr[i+1]);}
	}

	if(pos==0) {
		cout<<"no beginning found\n";
		UINT found=0;
		for(UINT i=0; i<l_arr; i++) {
			if(arr[i]<=average) 
				found++;
			else found=0;

			if(found>=minAdvertLength/(2*30)) {
				cout<<"found Advert\n";
				temp.frame=(i-found)*Movie->GetFPS();
				temp.duration=found*Movie->GetFPS();
				AddCut(temp);
				pos=i;
			}
		}
	}
	else {
		temp.frame=0;
		temp.duration=pos*Movie->GetFPS();
		AddCut(temp);}*

	pos+=minAdvertLength;

	double *pPer=(double*)av_malloc(sizeof(double)*Movie->GetDuration()/(minAdvertLength*2));//new double[Movie->GetDuration()/(minAdvertLength*2)+1];
	
	for(UINT i=pos; i<Movie->GetDuration(); i+=minAdvertLength*4) {
		pPer[i/minAdvertLength*4] = checkLogo(i,minAdvertLength*4,minAdvertLength/2,2,1);
	}

	UINT found=0,start=0;
	for(UINT i=pos; i<Movie->GetDuration(); i+=minAdvertLength*4) {

		if(pPer[i/minAdvertLength*4]<CHECKLOGO_PERCENT*1.6||found>0) {
			bool b=true, bVor=found>0;

			for(UINT j=0; j<minAdvertLength*4; j+=minAdvertLength) {
				b&= (checkLogo(i+j,minAdvertLength,minAdvertLength/15,2,1)<CHECKLOGO_PERCENT);
				if(bVor&&!b) {	//end
					temp.frame=start*Movie->GetFPS();
					temp.duration=(i+j-start)*Movie->GetFPS();
					AddCut(temp);
					cout<<"found Add\n";
					start=0;
				}
				else if(!bVor&&b) { //start
					start=i+j;
				}
				bVor=b;
			}

			if(b) found++;
			else found=0;
		}
		else {
			found=0;
			if(start!=0) {
				temp.frame=start*Movie->GetFPS();
				temp.duration=(i-start)*Movie->GetFPS();
				AddCut(temp);
				start=0;
				cout<<"found Add\n";
			}
		}

	}

	//if(pPer) delete [] pPer;
	//av_free(pPer);

	for(UINT i=0; i<iNumCuts; i++) {
		temp.frame = exactChange(pCuts[i].frame,Movie->GetFPS()*minAdvertLength);
		temp.duration = exactChange(pCuts[i].duration+pCuts[i].frame, Movie->GetFPS()*minAdvertLength)-temp.frame;
		pCuts[i]=temp;
	}
}*/

uint64_t CAdvertisement::exactChange(const uint64_t &guessed, const UINT &arround, const bool &bOnlyExact) {
	if(arround<=0) return guessed;

	double arr[11];
	UINT pos=0, size=arround;
	uint64_t i=guessed,start=0;
	if(i>arround) {
		i-=arround;
		size*=2;
	}
	size/=10;
	if(size==0) return guessed;
	
	start=i;
	for(; pos<11; i+=size, pos++) {
		arr[pos] = checkLogo(0,1,2, size, size<5?1:size/5, i,true);
		if(AdvertData.blackFrames>0) {
			for(UINT j=0; j<size; j+=size>25?size/25:1) {
				checkLogo(0,1,2, 1, 1, i+j,true);
				UINT errors=0;
				if(AdvertData.blackFrames>0&& ((errors=IsStable(i+j, arr[0]<CHECKLOGO_PERCENT, true))<3||errors>17) )
					return j+i;
			}
		}
	}
	

	double min=0;//(arr[0]-arr[1])*(arr[0]-arr[1])+(arr[0]-arr[1])*(arr[0]-arr[1]);
	int64_t best=0;

	double before=0;
	for(int n=0; n<9; n++) {
		before+=arr[n];
		if(n==0) continue;
		double after=0;
		for(UINT j=n+1; j<11; j++)
			after+=arr[j];
		after/=10-n;
		if( (after-before/(n+1))*(after-before/(n+1)) > min) {
			min=(after-before/(n+1))*(after-before/(n+1));
			best=n;
		}
	}

	if(min>CHECKLOGO_PERCENT*CHECKLOGO_PERCENT)
		return exactChange(start+size*best-size,2*size);
	else if(bOnlyExact&&min>CHECKLOGO_PERCENT*CHECKLOGO_PERCENT/4)
		return guessed;

	//didn't find anything -> check before and after
	i=guessed;
	if(i>6*arround) i-=arround/2*6;
	else i=0;
	min=1;
	double max=0;
	for(pos=0; pos<5; pos++,i+=arround/2) {
		arr[0]=checkLogo(0,1,2, arround/2, arround/2<10?1:arround/20, i);
		if(arr[0]<min) {
			best=pos;
			min=arr[0];}
		if(arr[0]>max)
			max=arr[0];
	}

	i=guessed+arround/2;
	for(pos=0; pos<5; pos++,i+=arround/2) {
		arr[0]=checkLogo(0,1,2, arround/2, arround/2<10?1:arround/20, i);
		if(arr[0]<min) {
			best=pos+5;
			min=arr[0];}
		if(arr[0]>max)
			max=arr[0];
	}
	
	if(min<CHECKLOGO_PERCENT&&max>CHECKLOGO_PERCENT) {
		if(best<5) {
			return exactChange(guessed-arround/2*(6-best),arround/2);
		}
		else {
			return exactChange(guessed+arround/2*(best-4),arround/2);
		}
	}

	return guessed;
}

UINT CAdvertisement::IsStable(const uint64_t &frame, const bool &bStart, const bool &bBlack) {
	double before[10];
	double	after[10];
	UINT pos=0;

	if(!bBlack) {
		for(uint64_t i=frame>STABLE_TIME*Movie->GetFPS()?frame-STABLE_TIME*Movie->GetFPS():0; i<frame+STABLE_TIME*Movie->GetFPS(); i+=STABLE_TIME*Movie->GetFPS()/10) {
			double res=checkLogo(0,1,2, STABLE_TIME*Movie->GetFPS()/10, STABLE_TIME*Movie->GetFPS()/100, i);
			if(pos>=5)	after[pos-5]=res;
			else		before[pos] =res;
			pos++;
		}
	}
	else {
		for(uint64_t i=frame>STABLE_TIME*Movie->GetFPS()?frame-STABLE_TIME*Movie->GetFPS():0; i<frame; i+=STABLE_TIME*Movie->GetFPS()/10)
			before[pos++]=checkLogo(0,1,2, STABLE_TIME*Movie->GetFPS()/10, STABLE_TIME*Movie->GetFPS()/100, i);
		pos=0;
		for(uint64_t i=frame+5*Movie->GetFPS(); i<frame+5*Movie->GetFPS()+STABLE_TIME*Movie->GetFPS(); i+=STABLE_TIME*Movie->GetFPS()/10)
			after[pos++]=checkLogo(0,1,2, STABLE_TIME*Movie->GetFPS()/10, STABLE_TIME*Movie->GetFPS()/100, i);
	}

	UINT errorBefore=0,errorAfter=0;
	for(UINT i=0; i<10; i++) {
		if(bStart&&before[i]>CHECKLOGO_PERCENT) errorBefore++;
		else if(!bStart&&before[i]<CHECKLOGO_PERCENT) errorBefore++;
		
		if(!bStart&&after[i]>CHECKLOGO_PERCENT) errorAfter++;
		else if(bStart&&after[i]<CHECKLOGO_PERCENT) errorAfter++;
	}

	return errorBefore+errorAfter;
}

void CAdvertisement::ScanAdvert(const char *filename, const UINT &minAdvertLength) {
	FileName = filename;
	Init(filename);
	SCut temp;

	bool bStatusOld=false, bStatus=false;
	for(UINT i=0; i<Movie->GetDuration(); i+=minAdvertLength) {
#ifdef _CONSOLE
		cout<<"\r"<<i*100/Movie->GetDuration()<<"%\n";
#endif
		bStatus = checkLogo(i,5,1,2,1)>CHECKLOGO_PERCENT*CHECKLOGO_PERCENT;

		if(i!=0&&bStatus!=bStatusOld) {
			temp.frame = checkRecursive(bStatusOld, bStatus,(i-minAdvertLength)*Movie->GetFPS(), (minAdvertLength+5)*Movie->GetFPS());
			
			for(uint64_t j=temp.frame-200; j<temp.frame+200; j+=15) {
				checkLogo(0,1,2, 15, 3, j);
				if(AdvertData.blackFrames>0) {
					for(uint64_t l=j; l<j+15; l++) {
						checkLogo(0,1,2, 1, 1, l);
						if(AdvertData.blackFrames>0) {
							temp.frame=j;
							j=temp.frame+10000;
							break;}
					}
				}
			}

			if(bStatus)
				Cutlist.AddStart(temp.frame);
			else
				Cutlist.AddEnd(temp.frame);
		}

		bStatusOld=bStatus;
	}

	Delete();

}

uint64_t CAdvertisement::checkRecursive2(const bool &bStart, const uint64_t &st, const uint64_t &end, const float &a, const float &b) {
	if(end-st<5)
		return (end+st)/2;

	float ar[5]={a,0,0,0,b};
	const int jump=(end-st)/5;
	const int check=jump>5?5:jump;

	//five in a row
	int p=1;
	for(uint64_t i=st+jump; p<4; i+=jump,p++)
		ar[p] = checkLogo(0,1,2, check,1, i, jump<25);

	p=0;
	float max=0;
	if(bStart) {
		//max of delta ar
		for(int i=0; i<4; i++)
			if(ar[i+1]-ar[i]>max) {
				p=i;
				max=ar[i+1]-ar[i];}
	}
	else {
		//max of delta ar
		for(int i=0; i<4; i++)
			if(ar[i]-ar[i+1]>max) {
				p=i;
				max=ar[i]-ar[i+1];}
	}

	if(max<0.5)
		cout<<"nothing good\n";

	//got new borders
	return checkRecursive2(bStart,st+p*jump,st+(p+1)*jump,ar[p],ar[p+1]);
}

void CAdvertisement::CreatePropMap(const char *filename, const UINT &num, float *array, Progress *progress) {
	FileName = filename;
	Init(filename);

	/*for(UINT i=0; i<num; i++) {
		array[i] = checkLogo(i/(float)num*Movie->GetDuration(),1,2,5,1);
		if(progress)
			progress->Set(i*100/num);
	}
	return;*/

	for(UINT i=0; i<num; i++) array[i]=0.f;

	UINT start=0;
	double a=1.;
	static const int step=150;
	bool bOpen=false;
	bool bFirst=true;
	for(UINT i=0; i<Movie->GetDuration(); i+=step) {
		double b=checkLogo(i,1,2,5,1);

		if(b-a>0.5) {/*
			i-=60;
			UINT last=30;
			bool bB=false;
			while(last>0) {
				last/=2;
				a=checkLogo(i,1,2,5,1);
				b=checkLogo(i+last,1,2,5,1);
				if(b-a<=0.5) {
					if(bB) break;
					last*=2;
					i+=last;
					bB=true;
				}
				else bB=false;
			}
			start=i*num/Movie->GetDuration();*/

			start=checkRecursive2(true,(i-step)*Movie->GetFPS(),(i)*Movie->GetFPS(),a,b)/Movie->GetFPS();
			bOpen=true;
		}
		else if(a-b>0.5) {/*
			if(i>=60) {
				i-=60;
				UINT last=30;
				bool bB=false;
				while(last>0) {
					last/=2;
					a=checkLogo(i,1,2,5,1);
					b=checkLogo(i+last,1,2,5,1);
					if(a-b<=0.5) {
						if(bB) break;
						last*=2;
						i+=last;
						bB=true;
					}
					else bB=false;
				}
			}

			for(UINT j=start; j<i*num/Movie->GetDuration(); j++)
				array[j]=1.;

			i+=5*60;*/

			UINT end=checkRecursive2(false,(i-step)*Movie->GetFPS(),(i)*Movie->GetFPS(),a,b)/Movie->GetFPS();
			for(UINT j=start*num/Movie->GetDuration(); j<end*num/Movie->GetDuration(); j++)
				array[j]=1.;
			if(!bFirst)
				i+=5*60;

			bOpen=false;
			bFirst=false;

			cout<<start<<" - "<<end<<endl;
		}

		a=b;
		if(progress)
			progress->Set(i*100/Movie->GetDuration());
	}

	if(bOpen)
		for(UINT j=start*num/Movie->GetDuration(); j<num; j++)
			array[j]=1.;

	Delete();
	if(progress)
		progress->Reset();
}

bool CAdvertisement::CreateCutlist(const char *filename, const char *cutlist, const string &author, const UINT &step, const UINT &security, const int &tyStart, const int &tyEnd, const UINT &jumpAfter, const bool &bScreenshot, const UINT &screens_dis, const UINT &screens_num, const UINT &screens_width, const UINT &screens_height) {
	FileName = filename;
	if(!Init(filename)) return false;

	string moviename=filename;
	{
		int p=moviename.find("\\");
		if(p==-1)
			p=moviename.find("/");
		while(p!=-1) {
			moviename.erase(0,++p);

			p=moviename.find("\\");
			if(p==-1)
				p=moviename.find("/");
		}
	}

	FILE *pFile = fopen(cutlist,"w");
	if(!pFile) return false;

	//Info
	fputs( "[Info]\nAuthor=", pFile);
	fputs( author.c_str(), pFile);
	fputs( "\nRatingByAuthor=0\nEPGError=0\n", pFile);
	fputs("ActualContent=\n",pFile);
	fputs("MissingBeginning=0\n",pFile);
	fputs("MissingEnding=0\n",pFile);
	fputs("MissingVideo=0\nMissingAudio=0\nOtherError=0\nOtherErrorDescription=\nSuggestedMovieName=",pFile);
	fputs(moviename.c_str(),pFile);
	fputs("\nUserComment=\n",pFile);
	char buffer[512]={};


	UINT cut_num=0;
	uint64_t start=0;
	double a=1.;
	bool bOpen=false;
	bool bFirst=true;
	for(UINT i=0; i<Movie->GetDuration(); i+=step) {
		double b=checkLogo(i,1,2,5,1);

		if(b-a>0.5) {
			start=checkRecursive2(true,(i-step)*Movie->GetFPS(),(i)*Movie->GetFPS(),a,b);
			if(start>security)
				start-=security*Movie->GetFPS();
			bOpen=true;

			if(bScreenshot) {
				char buffer[512];
				for(UINT t=0; t<screens_num; t++) {
					Movie->Seek(0,start+screens_dis*Movie->GetFPS()*(t/(double)screens_num+0.5));
					sprintf(buffer,"%s%da%d.png",filename,cut_num,t);
					Movie->SaveFrame(buffer,screens_width,screens_height);
				}
			}
		}
		else if(a-b>0.5&&i>=step) {
			uint64_t end=checkRecursive2(false,(i-step)*Movie->GetFPS(),(i)*Movie->GetFPS(),a,b);
			if(!bFirst)
				i+=jumpAfter;

			bool bWrite=true;
			if(bFirst) {
				switch(tyStart) {
					case 1: bWrite=false;
						break;
					case 2:
						if(start<=0) bWrite=false;
						break;
					default:break;
				}
			}

			if(bWrite&&end>start) {
				sprintf(buffer,"[Cut%d]\n",cut_num++);
				fputs(buffer,pFile);
				sprintf(buffer,"Start=%f\n",start/(double)Movie->GetFPS());
				fputs(buffer,pFile);
				sprintf(buffer,"Duration=%f\n",(end-start)/(double)Movie->GetFPS());
				fputs(buffer,pFile);
			}

			if(bScreenshot) {
				char buffer[512];
				for(UINT t=0; t<screens_num; t++) {
					Movie->Seek(0,end+screens_dis*Movie->GetFPS()*(t/(double)screens_num+0.5));
					sprintf(buffer,"%s%db%d.png",filename,cut_num,t);
					Movie->SaveFrame(buffer,screens_width,screens_height);
				}
			}

			bOpen=false;
			bFirst=false;
			start=end;
		}

		a=b;
	}

	if(bOpen) {
		bool bWrite=true;
		switch(tyEnd) {
			case 1: case 2:bWrite=false;
				break;
			default:break;
		}
		if(bWrite) {
			sprintf(buffer,"[Cut%d]\n",cut_num++);
			fputs(buffer,pFile);
			sprintf(buffer,"Start=%f\n",start/(double)Movie->GetFPS());
			fputs(buffer,pFile);
			sprintf(buffer,"Duration=%f\n",(Movie->GetDuration()*Movie->GetFPS()-start)/(double)Movie->GetFPS());
			fputs(buffer,pFile);
		}
	}

	//General
	fputs("[General]\nApplication=Josh's Cut\nVersion=0.0.0.1\ncomment1=\nIntendedCutApplicationName=Josh's FFMEPGCut\nIntendedCutApplication=Josh's FFMPEGCut\nIntendedCutApplicationVersion=0.0.0.1\nVDUseSmartRendering=1\nApplyToFile=",pFile);
	fputs(moviename.c_str(),pFile);
	sprintf(buffer,"\nNoOfCuts=%d\n",cut_num);
	fputs(buffer,pFile);

	fclose(pFile);
	Delete();

	return true;
}

uint64_t CAdvertisement::checkRecursive(const bool &bOld, const bool &bNew, const uint64_t &pos, const UINT &duration) {
	if(duration<=1) 
		return pos;

	bool bAct=false;

	if(duration/Movie->GetFPS()>5)
		bAct = checkLogo(0,5,1, 2,1, pos+duration/2-2*Movie->GetFPS())>CHECKLOGO_PERCENT;
	else
		bAct = checkLogo(0,1,2, duration/10,duration/150==0?1:duration/150, pos+duration/2-2)>CHECKLOGO_PERCENT;

	if(bAct==bOld)
		return checkRecursive(bOld,bNew,pos+duration/2,duration/2);
	else if(bAct==bNew)
		return checkRecursive(bOld,bNew,pos,duration/2);

	return pos;
}

void CAdvertisement::CCutlist::AddStart(const uint64_t &start) {
	SCut temp;
	temp.duration = 0;
	temp.frame = start;
	List.AddData(&temp,1);
}

void CAdvertisement::CCutlist::AddEnd(const uint64_t &end) {
	if(List.GetLength()<1) AddStart(0);

	List[List.GetLength()-1].duration = end-List[List.GetLength()-1].frame;
}

void CAdvertisement::CCutlist::Convert(const uint64_t &dur, const bool &markedKeep) {
	if(markedKeep!=bMarkedCut) return;

	uint64_t last=0,temp=0;
	for(UINT i=0; i<List.GetLength(); i++) {
		temp=List[i].frame+List[i].duration;

		List[i].duration = List[i].frame-last;
		List[i].frame = last;

		last = temp;
	}
	{
		SCut temp;
		temp.frame=last;
		temp.duration = dur-last;
		List.AddData(&temp,1);
	}
	for(UINT i=0; i<List.GetLength(); i++) {
		if(List[i].duration>=0)
			List.DeletePart(i,i+1);
	}

	bMarkedCut = !bMarkedCut;
}

void CAdvertisement::CCutlist::Save(const char *filename, const char *moviename, const double &fps) {
	FILE *pFile=fopen(filename,"w");
	char buffer[1024];

	fputs("[General]\nApplication=Josh's Cut\n",pFile);
	fputs("Version=0.0.0.1\ncomment1=\n",pFile);
	fputs("IntendedCutApplicationName=VirtualDub\nIntendedCutApplication=Josh's Cut\n",pFile);
	fputs("IntendedCutApplicationVersion=0.0.0.1\nVDUseSmartRendering=0\nNoOfCuts=",pFile);
	sprintf(buffer,"%d\n",List.GetLength());
	fputs(buffer,pFile);
	string temp = moviename;
	if(temp.find_last_of("\\")!=-1)
		temp.erase(0,temp.find_last_of("\\")+1);
	if(temp.find_last_of("/")!=-1)
		temp.erase(0,temp.find_last_of("/")+1);
	fputs("ApplyToFile=",pFile);
	fputs(temp.c_str(),pFile);
	fputs("\n[Info]\nAuthor=autocreation\nRatingByAuthor=3\n",pFile);
	fputs("EPGError=0\nActualContent=\nMissingBeginning=0\n",pFile);
	fputs("MissingEnding=0\nMissingVideo=0\nMissingAudio=0\nOtherError=0\nOtherErrorDescription=\nUserComment=\nSuggestedMovieName=",pFile);
	fputs(buffer,pFile);

	UINT num=0;
	for(UINT i=0; i<List.GetLength(); i++) { //last should be Add
		double d1=(List[i].frame)/fps,d2=(List[i].duration)/fps;
		sprintf(buffer,"[Cut%d]\nStart=%f\nDuration=%f\n",i,d1,d2);
		fputs(buffer,pFile);
	}
	fclose(pFile);
}

void CAdvertisement::CCutlist::Save(string *save, const char *moviename, const double &fps) {
	if(!save) return;

	save->clear();
	char buffer[1024];

	(*save) += "[General]\nApplication=Josh's Cut\n";
	(*save) += "Version=0.0.0.1\ncomment1=\n";
	(*save) += "IntendedCutApplicationName=VirtualDub\nIntendedCutApplication=Josh's Cut\n";
	(*save) += "IntendedCutApplicationVersion=0.0.0.1\nVDUseSmartRendering=0\nNoOfCuts=";
	sprintf(buffer,"%d\n",List.GetLength());
	(*save) += buffer;
	string temp = moviename;
	if(temp.find_last_of("\\")!=-1)
		temp.erase(0,temp.find_last_of("\\")+1);
	if(temp.find_last_of("/")!=-1)
		temp.erase(0,temp.find_last_of("/")+1);
	(*save) += "ApplyToFile=";
	(*save) += temp;
	(*save) += "\n[Info]\nAuthor=autocreation\nRatingByAuthor=3\n";
	(*save) += "EPGError=0\nActualContent=\nMissingBeginning=0\n";
	(*save) += "MissingEnding=0\nMissingVideo=0\nMissingAudio=0\nOtherError=0\nOtherErrorDescription=\nUserComment=\nSuggestedMovieName=";
	(*save) += buffer;

	UINT num=0;
	for(UINT i=0; i<List.GetLength(); i++) { //last should be Add
		double d1=(List[i].frame)/fps,d2=(List[i].duration)/fps;
		sprintf(buffer,"[Cut%d]\nStart=%f\nDuration=%f\n",i,d1,d2);
		(*save) += buffer;
	}
}

void CAdvertisement::Save(const char *filename) {
	//Cutlist.Convert(Movie->GetDuration()*Movie->GetFPS(),true);
	Cutlist.Save(filename,FileName.c_str(),Movie->GetFPS());
}

void CAdvertisement::Save(string *save) {
	//Cutlist.Convert(Movie->GetDuration()*Movie->GetFPS(),true);
	Cutlist.Save(save,FileName.c_str(),Movie->GetFPS());
}
