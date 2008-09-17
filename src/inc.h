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

#include <iostream>
#include <stdint.h>
#include <cstdlib>

using namespace std;

typedef unsigned int UINT;
typedef unsigned char uint8_t;

//#ifndef WIN32
//#if defined(__amd64__)
//#define INT64_C
//#endif
//#endif

#define _STDC_CONSTANT_MACROS	//because of AV_NOPTS_VALUE
//#define INT64_C(val) val##i64	//because of AV_NOPTS_VALUE, only that worked
#define INT64_C
#define __STDC_CONSTANT_MACROS
#include <stdint.h>


#define 	INT8_MAX   0x7f
#define 	INT8_MIN   (-INT8_MAX - 1)
#define 	UINT8_MAX   (__CONCAT(INT8_MAX, U) * 2U + 1U)
#define 	INT16_MAX   0x7fff
#define 	INT16_MIN   (-INT16_MAX - 1)
#define 	UINT16_MAX   (__CONCAT(INT16_MAX, U) * 2U + 1U)
#define 	INT32_MAX   0x7fffffffL
#define 	INT32_MIN   (-INT32_MAX - 1L)
#define 	UINT32_MAX   (__CONCAT(INT32_MAX, U) * 2UL + 1UL)
#define 	INT64_MAX   0x7fffffffffffffffLL
#define 	INT64_MIN   (-INT64_MAX - 1LL)
#define 	UINT64_MAX   (__CONCAT(INT64_MAX, U) * 2ULL + 1ULL)

#define MURKS cout<<__LINE__<<endl;


template<class C> class CTemplateBuffer {
private:
	UINT *sSize;
	UINT *sNum;
	UINT l,aL;
	C *data;
public:
	CTemplateBuffer(UINT *Size, UINT *Num) {l=0; aL=0; data = NULL;sSize=Size;sNum=Num;}
	~CTemplateBuffer(){DeleteFull();}

	void Init() { l=0; aL=*sSize; data = new C[aL];}
	void Delete() { (*sNum)++; l+=l/2; *sSize+=l/ *sNum-*sSize/ *sNum; l=0; }
	void DeleteFull() { if(data) delete [] data; (*sNum)++; l+=l/2; *sSize+=l/ *sNum-*sSize/ *sNum; l=aL=0; data=NULL;}
	void Reset() { (*sNum)++; l+=l/2; *sSize+=l/ *sNum-*sSize/ *sNum; l=0; }

	UINT GetLength() const {return l;}
	C *GetData() {return data;}
	void AddData(const C *buffer, const UINT &len) {
		if(len+l>aL) {
			data=(C*)realloc(data,sizeof(C)*(len+l));
			aL=len+l;}
		for(UINT i=l; i<len+l; i++)
			data[i]=buffer[i-l];
		l+=len;
	}
	void DeletePart(const UINT &from, const UINT &to) {
		if(from>=l||to>l) return;
		UINT d=to-from;
		l-=d;
		for(UINT i=from; i<l; i++)
			data[i]=data[i+d];
	}
	C &operator[](const UINT &index) {
#ifdef DEBUG_CB
		if(index>=l)	return 0;
#endif
		return data[index];}
	C GetCopy(const UINT &index) const {
#ifdef DEBUG_CB
		if(index>=l)	return 0;
#endif
		return data[index];}
};
