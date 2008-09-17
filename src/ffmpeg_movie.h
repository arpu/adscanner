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
#include "movie_load.h"
#include "progress.h"
//#include <stdint.h>
//#include <inttypes.h>


extern "C" { 
#include <ffmpeg/avcodec.h>
#include <ffmpeg/avformat.h>
#include <ffmpeg/swscale.h>
}

#define FFMPEG_PACKET_MAX_SIZE 100

class CFFMPEGLoader : public CMovieLoader {
private:

	AVFormatContext *pFormatCon,*pAFormatCon;
	AVCodecContext	*pVCodecCon,*pACodecCon;
    AVStream		*pAudioStream, *pVideoStream;	//for recording
	int		videoStream;
	int		audioStream;
	AVCodec *pCodec;
	AVFrame	*pFrame,*pFrameRGB;
	bool bAudioIsNeeded, bVideoIsNeeded;
	UINT iPKTVideoLength,iPKTAudioLength;
	AVPacket pktLastVideo[FFMPEG_PACKET_MAX_SIZE];
	AVPacket pktLastAudio[FFMPEG_PACKET_MAX_SIZE];
	uint8_t *audio_pkt_data;
	int		audio_pkt_size;
	unsigned char	*pBuffer;
	UINT	iBufferSize;
	bool bOutput,bLock;

	uint64_t i64PTS,i64DTS;
	static void open_stream(AVFormatContext *oc, AVStream *st);
	static AVStream *add_video_stream(AVFormatContext *oc, int codec_id, const AVCodecContext *Con);
	static AVStream *add_audio_stream(AVFormatContext *oc, int codec_id, const AVCodecContext *Con);
	int64_t pGetVideoTS() const {return pktLastVideo[0].pts<0?pktLastVideo[0].dts:pktLastVideo[0].pts;}
public:
	void Init();
	bool InitOSD();
	bool Open(const char *filename, const bool &bOutput=false);
//	CFFMPEGLoader::CMovieInformation RetrieveVideoInfo();
	int  LoadFrame();
	void Delete();
	void SetRect(const UINT &x, const UINT &y, const UINT &width, const UINT &height);
	void CheckEvent();

	int ReadAudioData(unsigned char *buf, const UINT &buf_size);
	void DeleteOlderPTS(const int64_t &pts);
	int ReadVideoData(unsigned char **buf, UINT *width, UINT *height);
	int ReadVideoData(unsigned char **buf, UINT *width, UINT *height, int dst_pix_fmt, unsigned char **yuv_buf);

	void UseAudio()  {bAudioIsNeeded=true;}
	void ClearAudio(){bAudioIsNeeded=false;}
	void UseVideo()  {bVideoIsNeeded=true;}
	void ClearVideo(){bVideoIsNeeded=false;}

	AVFrame *GetYUV() const {return pFrame;}
	const char *GetName() { return pFormatCon->filename; }

	int64_t GetActualDTS() const {return pFormatCon->streams[videoStream]->cur_dts;}
	int64_t GetActualPTS() const {if(iPKTVideoLength>0) return av_q2d(pFormatCon->streams[videoStream]->time_base)*pGetVideoTS()/av_q2d(pFormatCon->streams[audioStream]->time_base); return 0;}
	int64_t GetAudioLateness() const {if(iPKTAudioLength>0&&iPKTVideoLength>0) return av_q2d(pFormatCon->streams[videoStream]->time_base)*pGetVideoTS() - pktLastAudio[0].pts*av_q2d(pFormatCon->streams[audioStream]->time_base); return 0;}
	bool GetPTSAvailable() const {return (iPKTAudioLength<1||iPKTVideoLength<1||(pktLastAudio[0].pts!=AV_NOPTS_VALUE&&pktLastAudio[0].pts!=AV_NOPTS_VALUE));}

	int GetBitrate() const {return pFormatCon->bit_rate;}
	int64_t GetDuration() const {
		return pFormatCon->duration/AV_TIME_BASE+1;
	}
	int GetFPS() const {
		if(videoStream>=0)
			return pFormatCon->streams[videoStream]->time_base.den/pFormatCon->streams[videoStream]->time_base.num; //some errors with mp4
		return 25;
	}
	uint64_t GetActualFrame() const {
		if(videoStream>=0)
			if(pFormatCon->streams[videoStream]->time_base.den!=0)
				return pFormatCon->streams[videoStream]->cur_dts*pFormatCon->streams[videoStream]->time_base.num * AV_TIME_BASE*GetFPS()/(AV_TIME_BASE*pFormatCon->streams[videoStream]->time_base.den);
		return 0;
	}

	void Seek(const UINT &iSec, const uint64_t &iFrame=0, const bool &bExact=true, const int &dst_pix_fmt=-1);

	void ResetBufferSize() {iBufferSize=FFMPEG_PACKET_MAX_SIZE;}
	void SetBufferSize(const UINT &size) {iBufferSize=size>FFMPEG_PACKET_MAX_SIZE?FFMPEG_PACKET_MAX_SIZE:size;}
	UINT GetBufferSizeAudio() const {return iPKTAudioLength;}
	UINT GetBufferSizeVideo() const {return iPKTVideoLength;}

	//for tests
	void SaveFrame(int iFrame, const char *add="");
	void SaveFrame(const char *fn, int wanted_width=0, int wanted_height=0);
	void ShowFrameConsole(const unsigned char *buf);

//**********FOR WRITING**********
	bool CreateMovie(const char *filename, const AVOutputFormat *format, const AVCodecContext *VideoCon, const AVCodecContext *AudioCon);
	void CloseWriting();
	AVOutputFormat *GetFormat() {return pFormatCon->oformat;}

	void DeleteBuffer();
	void CopyPacktes(CFFMPEGLoader *FFMPEG, const uint64_t &firstFrame, const uint64_t &lastFrame);
	//void SkipToFrame(const uint64_t &lastFrame) {i64PTS=lastFrame;}
	void WriteFrame(const AVFrame *frame);

	AVPacket *GetNextPacket();

	int GetMinBufferSize() const {return iPKTVideoLength<iPKTAudioLength?iPKTVideoLength:iPKTAudioLength;}

	const AVCodecContext *GetVideoCon() {return pVCodecCon;}
	const AVCodecContext *GetAudioCon() {return pACodecCon;}
protected:
	AVPacket *GetNextVideoPacket() {if(iPKTVideoLength<=0) return NULL; return &pktLastVideo[0];}
	AVPacket *GetNextAudioPacket() {if(iPKTAudioLength<=0) return NULL; return &pktLastAudio[0];}
	void PopVideoPacket();
	void PopAudioPacket();
};
