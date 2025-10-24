/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 * Volker Fischer, Ollie Haffenden
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 *
\******************************************************************************/

#ifndef AUDIOSOURCEDECODER_H
#define AUDIOSOURCEDECODER_H

#include "../GlobalDefinitions.h"
#include "../Parameter.h"
#include "../util/Modul.h"
#include "../util/CRC.h"
#include "../TextMessage.h"
#include "../resample/Resample.h"
#include "../datadecoding/DataDecoder.h"
#include "../util/Utilities.h"
#include "AudioCodec.h"
//#include "../MSC/audiosuperframe.h"
#include "reverb.h"

#ifdef HAVE_SPEEX
# include "../resample/speexresampler.h"
#else
# include "../resample/caudioresample.h"
#endif


#include "fdk-aac/aacdecoder_lib.h"
#include "fdk-aac/FDK_audio.h"
#include "../SDC/SDC.h"
#include <cstring>


/* Definitions ****************************************************************/
/* Forgetting factor for audio blocks in case CRC was wrong */
#define FORFACT_AUD_BL_BAD_CRC			((_REAL) 0.6)


#define 	xHEAAC_PAYLOAD_BUFFSIZE_MASK   	((uint32_t)0xfff)	
#define 	xHEAAC_FRAME_BUFFSIZE_MASK   		((uint32_t)0x1f)	

#define 	xHEAAC_PAYLOAD_BUFFSIZE 		(xHEAAC_PAYLOAD_BUFFSIZE_MASK + 1) //ring buffer, make it 2^power to ease calculater
#define 	xHEAAC_FRAME_BUFFSIZE		(xHEAAC_FRAME_BUFFSIZE_MASK+1)   //ring buffer

/*prevent module outpu buffer overruns with xHE-AAC (as detected by clang asan), otherwise, 
    output audio will be clipped/skipped and output buffer of audio driver will underflow sporadically 
   even NO frame error is present */
#define    OUTPUT_BUFFER_OVERHEAD_MARGIN     (8u)   

//#define DBG_PRINT    //OUTPUT DEBUG TO FILE

/* Classes ********************************************************************/

class CAudioSourceDecoder : public CReceiverModul<_BINARY, _SAMPLE>
{
public:
	CAudioSourceDecoder();

	virtual ~CAudioSourceDecoder();

	bool CanDecode(CAudioParam::EAudCod eAudCod) {
	    switch (eAudCod)
	    {
	    case CAudioParam::AC_NONE: return true;
	    case CAudioParam::AC_AAC:  return true;
	    case CAudioParam::AC_OPUS: return false;
	    case CAudioParam::AC_xHE_AAC: return true;
	    default: return false;
	    }
	    return false;
	}
	int GetNumCorDecAudio();
	void SetReverbEffect(const bool bNER) {
	    bUseReverbEffect = bNER;
	}
	bool GetReverbEffect() {
	    return bUseReverbEffect;
	}

	bool bWriteToFile;

protected:

	/* General */
	bool DoNotProcessData;
	bool DoNotProcessAudDecoder;
	int iNumCorDecAudio;

	/* Text message */
	bool bTextMessageUsed;
	CTextMessageDecoder TextMessage;
	CVector<_BINARY> vecbiTextMessBuf;


	int					iTotalFrameSize;   //byte unit


	/* Resampling */
	int outputSampleRate;

#ifdef HAVE_SPEEX
    	SpeexResampler ResampleObjL;
    	SpeexResampler ResampleObjR;
#else
    	CAudioResample ResampleObjL;
    	CAudioResample ResampleObjR;
#endif

	CVector<_REAL> vecTempResBufInLeft;
	CVector<_REAL> vecTempResBufInRight;
	CVector<_REAL> vecTempResBufOutLeft;
	CVector<_REAL> vecTempResBufOutRight;


	CAudioParam::EAudCod eAudioCoding;

	CVector<_BYTE>		vecbyPrepAudioFrame;

	CVector<_BYTE>		aac_crc_bits;
	CMatrix<_BYTE>		audio_frame;

	CVector<int>			veciFrameLength;

	int					iNumAudioFrames;
	int					iNumBorders;
	int					iResOutBlockSize;
	int					iNumHigherProtectedBytes;
	int					iMaxLenOneAudFrame;
	int					iLenDecOutPerChan;
	int 					iAudioSampleRate;

	int 					iBadBlockCount;
	std::string 			audiodecoder;
	int					iAudioPayloadLen;
	int					iLenAudLow;

	int 					iDynamicMaxOutputBlockSize; //throttle the (continuous) garbage written to output buffer
	
	/* Drop-out masking (reverberation) */
	bool 				bUseReverbEffect;
	bool					bAudioWasOK;




	CCRC				xHEAAC_CRCObject;

	CVector<_BYTE> 		xHEAACPayload;	//ring buffer
	CVector<int>			xHEAACFrameSize;  //ring buffer
	CVector<int>			xHEAACFrameStart;  //ring buffer


	int					iPayloadWrite;


	int					PrevParsingFrame;
	int					ParsingFrame;
	int					DecodeFrame;
	int					iNumberChannels;

	_DIRECTORY			sDirectoryCache[16];	
	

    	HANDLE_AACDECODER 	hDecoder;
   
	_BYTE				AudioFrame[1536];  //maximum frame size (with 2 channels at the most)
   
    	_SAMPLE 				decode_buf[13840];


#ifdef DBG_PRINT	

	 FILE* 				pFile;

#endif



    	virtual void InitInternal(CParameter& Parameters);
    	virtual void ProcessDataInternal(CParameter& Parameters);
    	Reverb reverb;
		
};

#endif // AUDIOSOURCEDECODER_H
