/******************************************************************************\
 * Technische Universitaet Darmstadt, Institut fuer Nachrichtentechnik
 * Copyright (c) 2001
 *
 * Author(s):
 *	Volker Fischer
 *
 * Description:
 *	Audio source encoder/decoder
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

#include "AudioSourceDecoder.h"
#include <iostream>


/* Implementation *************************************************************/

void
CAudioSourceDecoder::ProcessDataInternal(CParameter & Parameters)
{
	int i;
    	bool bCurBlockOK;
    	bool bGoodValues = false;

	short*				psDecOutSampleBuf;
	int	j;




    Parameters.Lock();
    Parameters.vecbiAudioFrameStatus.Init(0);
    Parameters.vecbiAudioFrameStatus.ResetBitAccess();
    Parameters.Unlock();

    /* Check if something went wrong in the initialization routine */
    if (DoNotProcessData)
    {
        return;
    }

    //cerr << "got one logical frame of length " << pvecInputData->Size() << " bits" << endl;

    /* Text Message ********************************************************** */
    if (bTextMessageUsed)
    {
        /* Decode last four bytes of input block for text message */
        for (i = 0; i < SIZEOF__BYTE * NUM_BYTES_TEXT_MESS_IN_AUD_STR; i++)
            vecbiTextMessBuf[i] = (*pvecInputData)[iInputBlockSize - SIZEOF__BYTE * NUM_BYTES_TEXT_MESS_IN_AUD_STR + i];

        TextMessage.Decode(vecbiTextMessBuf);
    }

    /* Audio data header parsing ********************************************* */
    /* Check if audio shall not be decoded */
    if (DoNotProcessAudDecoder)
    {
        return;
    }

    /* Reset bit extraction access */
    (*pvecInputData).ResetBitAccess();


	/* parsing */
	if (eAudioCoding == CAudioParam::AC_AAC)
	{
		/* AAC super-frame-header ----------------------------------------------- */
		int iPrevBorder = 0;
		for (i = 0; i < iNumBorders; i++)
		{
			/* Frame border in bytes (12 bits) */
			const int iFrameBorder = (*pvecInputData).Separate(12);

			/* The lenght is difference between borders */
			veciFrameLength[i] = iFrameBorder - iPrevBorder;
			iPrevBorder = iFrameBorder;
		}

		/* Byte-alignment (4 bits) in case of 10 audio frames */
		if (iNumBorders == 9)
			(*pvecInputData).Separate(4); 

		/* Frame length of last frame */
		veciFrameLength[iNumBorders] = iAudioPayloadLen - iPrevBorder;

		/* Check if frame length entries represent possible values */
		bGoodValues = true;
		for (i = 0; i < iNumAudioFrames; i++)
		{
			if ((veciFrameLength[i] < 0) ||
				(veciFrameLength[i] > iMaxLenOneAudFrame))
			{
					bGoodValues = false;
			}
		}

		if (bGoodValues == true)
		{
			/* Higher-protected part -------------------------------------------- */
			for (i = 0; i < iNumAudioFrames; i++)
			{
				/* Extract higher protected part bytes (8 bits per byte) */
				for (j = 0; j < iNumHigherProtectedBytes; j++)
					audio_frame[i][j] = (*pvecInputData).Separate(8);

				/* Extract CRC bits (8 bits) */
				aac_crc_bits[i] = (*pvecInputData).Separate(8);
			}


			/* Lower-protected part --------------------------------------------- */
			for (i = 0; i < iNumAudioFrames; i++)
			{
				/* First calculate frame length, derived from higher protected part
				   frame length and total size */
				const int iNumLowerProtectedBytes = 
					veciFrameLength[i] - iNumHigherProtectedBytes;

				/* Extract lower protected part bytes (8 bits per byte) */
				for (j = 0; j < iNumLowerProtectedBytes; j++)
					audio_frame[i][iNumHigherProtectedBytes + j] = 
						(*pvecInputData).Separate(8);
			}
		}
	}
	else if(eAudioCoding == CAudioParam::AC_xHE_AAC)
	{
		
		/* xHEAAC parser */

		// must update iNumAudioFrames at the end of parsing routine
	    	
	    	unsigned frameBorderCount = (*pvecInputData).Separate(4);
	    	unsigned bitReservoirLevel = (*pvecInputData).Separate(4);
		_BYTE xHEAAC_CRC = (*pvecInputData).Separate(8);

	    	xHEAAC_CRCObject.Reset(8);
	    	xHEAAC_CRCObject.AddByte((frameBorderCount << 4) | bitReservoirLevel);
			
	    	if(bGoodValues = xHEAAC_CRCObject.CheckCRC(xHEAAC_CRC)) 
		{
	       	
	    	}
	    	else 
		{
	        	//cerr << endl << "superframe crc bad but will hope the frameBorderCount is OK" << endl;
	    	}
			
#ifdef DBG_PRINT	
fprintf(pFile, "-----------audiosuperframe crc:%d;iTotalFrameSizeByte:%d;iPayloadWrite:%d;--------------\n",
	bGoodValues, iTotalFrameSize, iPayloadWrite);
fflush(pFile);
#endif
			
	    // TODO handle reservoir
	    	unsigned bitResLevel = (bitReservoirLevel + 1) * 384 * iNumberChannels;
	    	unsigned directory_offset = iTotalFrameSize  - (2 * frameBorderCount);
	    	int iPayloadStart = iPayloadWrite;  //start positon of current payload write
	    	
	    	//cerr << start << " bytes left from previous superframe" << endl;
	    	//cerr << "payload start " << start << " bit reservoir level " << bitReservoirLevel << " bitResLevel " << bitResLevel << " superframe size " << superFrameSize << " directory offset " << 8*directory_offset << " bits " << directory_offset << " bytes" << endl;
	    	// get the payload

		//checking sanity of directory_offset
		if(directory_offset > iTotalFrameSize)
			directory_offset = iTotalFrameSize;
			
		// note: payload is cyclical ring buffer	
	    	for(i = 2; i < directory_offset; i++) 
		{
			xHEAACPayload[iPayloadWrite] = (*pvecInputData).Separate(8);

			iPayloadWrite = ((iPayloadWrite + 1) & xHEAAC_PAYLOAD_BUFFSIZE_MASK);

	    	}
			
		if(frameBorderCount > 0) 
		{
	       	// get the directory
			for(i = int(frameBorderCount - 1); i >= 0; i--)
				sDirectoryCache[i] = (*pvecInputData).Separate(16);
			
			// note: 	ParsingFrame and xHEAACFrameSize are all modular arithmatic operation
	        	for(i = 0; i < int(frameBorderCount); i++)
			{
	            		unsigned frameBorderIndex = ((sDirectoryCache[i] >> 4) & 0xfff); //(*pvecInputData).Separate(12);  //separate backward
	            		unsigned frameBorderCountRepeat = (sDirectoryCache[i] & 0x0f); //(*pvecInputData).Separate(4);//separate backward
						
	            		if(frameBorderCountRepeat != frameBorderCount) 
				{
	             		   	
					cerr << "unequal frameBorderCount between head and directory! " << endl;
					
					// even CRC check is NOT ok, you can use the frameBorderCountRepeat to establish correct frameBorderCount
#ifdef DBG_PRINT	
fprintf(pFile,"unequal frameBorderCount between head and directory!\n");
fflush(pFile);
#endif
					
	            		}
						
	            		//cerr << "border " << i << " of " << frameBorderCountRepeat << "/" << frameBorderCount << " starts at " << hex << frameBorderIndex << dec << endl;
			   
			      // set the borders relative to the start including the payload bytes from previous superframes
			      //special handle for first border
			    	if(0 == i)  
				{
					switch(frameBorderIndex) 
					{
				        	case 0xffe: // delayed from previous superframe
				            		//cerr << "first frame has two bytes in previous superframe" << endl;
				            		xHEAACFrameStart[ParsingFrame] = iPayloadStart - 2;

							if(xHEAACFrameStart[ParsingFrame] < 0)
								xHEAACFrameStart[ParsingFrame] += xHEAAC_PAYLOAD_BUFFSIZE;
				 
							break;
							
				        	case 0xfff: // the start of the audio frame at the last byte of the Payload section of the previous audio super frame
				            		//cerr << "first frame has one byte in previous superframe" << endl;
				            		
				            		xHEAACFrameStart[ParsingFrame] = iPayloadStart - 1;

							if(xHEAACFrameStart[ParsingFrame] < 0)
								xHEAACFrameStart[ParsingFrame] += xHEAAC_PAYLOAD_BUFFSIZE;
				 
							break;

						default: // boundary in this superframe

							if(frameBorderIndex > directory_offset)
							{

								frameBorderIndex = directory_offset;
							

#ifdef DBG_PRINT	
								fprintf(pFile,"abnormal frameBorderIndex:%d!\n", frameBorderIndex);
								fflush(pFile);
#endif

							}

							xHEAACFrameStart[ParsingFrame] 
								= ((frameBorderIndex + iPayloadStart)  & xHEAAC_PAYLOAD_BUFFSIZE_MASK);   
						
							break;
				       }

			    	}
				else
				{

					if(frameBorderIndex > directory_offset)
					{
						frameBorderIndex = directory_offset;

#ifdef DBG_PRINT	
						fprintf(pFile,"abnormal frameBorderIndex:%d!\n", frameBorderIndex);
						fflush(pFile);
#endif
					}


					xHEAACFrameStart[ParsingFrame] 
						= ((frameBorderIndex + iPayloadStart) & xHEAAC_PAYLOAD_BUFFSIZE_MASK);   


				}
	            		//cerr << "border 0 is " << borders[0] << " bytes from start of payload" << endl;


				if(PrevParsingFrame >= 0) //guarding against un-init xHEAACFrameStart
				{
					xHEAACFrameSize[PrevParsingFrame] 
						= xHEAACFrameStart[ParsingFrame] - xHEAACFrameStart[PrevParsingFrame];

					if(xHEAACFrameSize[PrevParsingFrame] < 0)
						xHEAACFrameSize[PrevParsingFrame] += xHEAAC_PAYLOAD_BUFFSIZE;

				}

				
				ParsingFrame = ((ParsingFrame + 1) & xHEAAC_FRAME_BUFFSIZE_MASK);
				PrevParsingFrame = ((PrevParsingFrame + 1) & xHEAAC_FRAME_BUFFSIZE_MASK);
	            		
	       	}
	  
	    	}

		/* update audio frames count, note this is module arithmatic */
		if(PrevParsingFrame >= 0)
		{	

			iNumAudioFrames = PrevParsingFrame - DecodeFrame; 

			if(iNumAudioFrames < 0)
				iNumAudioFrames += xHEAAC_FRAME_BUFFSIZE;

		}
		else
			iNumAudioFrames = 0;

#ifdef DBG_PRINT	
fprintf(pFile,"iNumAudioFrames:%d;\n", iNumAudioFrames);
fflush(pFile);
#endif

	}
	




    /* Audio decoding ******************************************************** */
    /* Init output block size to zero, this variable is also used for
       determining the position for writing the output vector */
    iOutputBlockSize = 0;
    //cerr << "audio superframe with " << pAudioSuperFrame->getNumFrames() << " frames" << endl;
	for (j = 0; j < iNumAudioFrames; j++)
    	{
        	bool bCurBlockFaulty = false; // just for Opus or any other codec with FEC

       	if (bGoodValues)
	       {
				
			if (eAudioCoding == CAudioParam::AC_AAC)
			{
				
				/* Prepare data vector with CRC at the beginning (the definition
				   with faad2 DRM interface) */
				vecbyPrepAudioFrame[0] = aac_crc_bits[j];

				for (i = 0; i < veciFrameLength[j]; i++)
					vecbyPrepAudioFrame[i + 1] = audio_frame[j][i];


				/* Call decoder routine */
	//				psDecOutSampleBuf = (short*) faacDecDecode(HandleAACDecoder,
	//					&DecFrameInfo, &vecbyPrepAudioFrame[0], veciFrameLength[j] + 1);


				/* Prepare one audio frame */
				UINT uiBufferSize = veciFrameLength[j] + 1;
				UINT uibytesValid;

				
				UCHAR *pData[1];  // only one layer is used here!
						
				uibytesValid = uiBufferSize;
				pData[0] = &vecbyPrepAudioFrame[0];

				
				AAC_DECODER_ERROR err = aacDecoder_Fill(hDecoder, pData, &uiBufferSize, &uibytesValid);

				if(err != AAC_DEC_OK) 
				{
				    cerr << "fill failed " << int(err) << endl;

				
				    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
				}
				else
				{
					/*TODO: 
					decoder internal buffer is full, not able to absorb all data in current audio frame(due to error in 
					 current frame), have to flush it to accept full audio frame for the next iteration */
					if(0 != uibytesValid) 
					{

					    	cerr << " input bytes, bytes left " << uibytesValid << endl;
						//return CAudioCodec::DECODER_ERROR_UNKNOWN;



					}


				}

				CStreamInfo *pinfo = aacDecoder_GetStreamInfo(hDecoder);

				if (pinfo == NULL) 
				{

				    cerr << "No stream info" << endl;
				    //return nullptr; this breaks everything!
				}


					
				//cerr << "Decode";
				//logAOT(*pinfo);
				//logFlags(*pinfo);
				//logNumbers(*pinfo);
				//cerr << endl;

				if(pinfo->aacNumChannels == 0) 
				{
				    cerr << "zero output channels: " << err << endl;
				    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
				}
				else 
				{
//				    cerr << pinfo->aacNumChannels << " aac channels " << endl;
				}


				size_t output_size = unsigned(pinfo->frameSize * pinfo->numChannels);
				if(sizeof (decode_buf) < sizeof(_SAMPLE) * output_size) 
				{
				    cerr << "can't fit output into decoder buffer" << endl;
				    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
				}


				memset(decode_buf, 0, sizeof(_SAMPLE) * output_size);
				err = aacDecoder_DecodeFrame(hDecoder, decode_buf, int(output_size), 0);


				psDecOutSampleBuf = decode_buf;



				if (err != AAC_DEC_OK)
				{
					bCurBlockOK = false; /* Set error flag */

				}
				else
				{

				
					bCurBlockOK = true;

					/* Conversion from _SAMPLE vector to _REAL vector for resampling.
					   ATTENTION: We use a vector which was allocated inside
					   the AAC decoder! */
					if (pinfo->numChannels == 1)
					{
						/* Change type of data (short -> real) */
						for (i = 0; i < iLenDecOutPerChan; i++)
						{	vecTempResBufInLeft[i] = psDecOutSampleBuf[i];
							vecTempResBufInRight[i] = psDecOutSampleBuf[i];

						}	

					}
					else
					{
						/* Stereo */
						for (i = 0; i < iLenDecOutPerChan; i++)
						{
							vecTempResBufInLeft[i] = psDecOutSampleBuf[i * 2];
							vecTempResBufInRight[i] = psDecOutSampleBuf[i * 2 + 1];
						}

					}
				}

			}
			else if (eAudioCoding == CAudioParam::AC_xHE_AAC)
			{


#ifdef DBG_PRINT	

	fprintf(pFile,"*-------------------------------*;\n");

	fprintf(pFile,"decoder frame:%d; size:%d;iPayloadRead:%d;\n", 
		 DecodeFrame, xHEAACFrameSize[DecodeFrame], xHEAACFrameStart[DecodeFrame]);
	fflush(pFile);
#endif

				/* Prepare one audio frame */
				int	iPayloadRead;
				UINT uiBufferSize = xHEAACFrameSize[DecodeFrame];
				UINT uibytesValid;
				
				UCHAR *pData[1];  // only one layer is used here!

				/* restrict read input size to possible maximum size */
				if(uiBufferSize > sizeof(AudioFrame))  
				{
					uiBufferSize = sizeof(AudioFrame);	

#ifdef DBG_PRINT	
	fprintf(pFile,"Input buffer overflow!\n"); 
	fflush(pFile);
#endif
				}
						
				uibytesValid = uiBufferSize;
				pData[0] = &AudioFrame[0];

				iPayloadRead = xHEAACFrameStart[DecodeFrame];
				DecodeFrame = ((DecodeFrame + 1) & xHEAAC_FRAME_BUFFSIZE_MASK);


				for(i = 0; i < uibytesValid; i++)
				{
					
					AudioFrame[i] = xHEAACPayload[iPayloadRead];

					iPayloadRead = ((iPayloadRead + 1) & xHEAAC_PAYLOAD_BUFFSIZE_MASK);

				}


				xHEAAC_CRCObject.Reset(16);


				/*WARNIG: uibytesValid MUST BE LARGER THAN 2, OTHERWISE NO FURTHER
				   CRC CHECK OVER XHE FRAME IS NEEDED !! */

				if(uibytesValid < 2)
					bCurBlockOK = false;

				else
				{
					for(i = 0; i < (uibytesValid - 2); i++)
					{
						xHEAAC_CRCObject.AddByte(AudioFrame[i]);

					}
				
					if(bCurBlockOK = xHEAAC_CRCObject.CheckCRC((((UINT)AudioFrame[i]) << 8)|AudioFrame[i + 1])) 
					{

						AAC_DECODER_ERROR err = aacDecoder_Fill(hDecoder, pData, &uiBufferSize, &uibytesValid);
						
						
						if(err != AAC_DEC_OK) 
						{
						    cerr << "fill failed " << int(err) << endl;

#ifdef DBG_PRINT					
			fprintf(pFile,"fill error;\n");
			fflush(pFile);
#endif						
						    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
						}
						else
						{
							/*TODO: 
							decoder internal buffer is full, not able to absorb all data in current audio frame(due to error in 
							 current frame), have to flush it to accept full audio frame for the next iteration */
							if(0 != uibytesValid) 
							{

							    	cerr << " input bytes, bytes left " << uibytesValid << endl;
								//return CAudioCodec::DECODER_ERROR_UNKNOWN;



							}


						}

						CStreamInfo *pinfo = aacDecoder_GetStreamInfo(hDecoder);

						if (pinfo == NULL) 
						{
#ifdef DBG_PRINT	
			fprintf(pFile,"GetStreamInfo error;\n");
			fflush(pFile);
#endif					
						    cerr << "No stream info" << endl;
						    //return nullptr; this breaks everything!
						}

#ifdef DBG_PRINT	
			fprintf(pFile,"aacNumChannels:%d, frameSize:%d, uibytesValid:%d;\n", 
							pinfo->aacNumChannels, pinfo->frameSize, uibytesValid);
			fflush(pFile);
#endif						
						//cerr << "Decode";
						//logAOT(*pinfo);
						//logFlags(*pinfo);
						//logNumbers(*pinfo);
						//cerr << endl;

						if(pinfo->aacNumChannels == 0) 
						{
						    cerr << "zero output channels: " << err << endl;
						    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
						}
						else 
						{
	//					    cerr << pinfo->aacNumChannels << " xhe-aac channels " << endl;
						}


						size_t output_size = unsigned(pinfo->frameSize * pinfo->numChannels);
						if(sizeof (decode_buf) < sizeof(_SAMPLE) * output_size) 
						{
						    cerr << "can't fit output into decoder buffer" << endl;
						    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
						}
#ifdef DBG_PRINT	
			fprintf(pFile,"output_size:%d;\n", output_size);
			fflush(pFile);
#endif

						memset(decode_buf, 0, sizeof(_SAMPLE) * output_size);

						err = aacDecoder_DecodeFrame(hDecoder, decode_buf, int(output_size), 0);

						if(err == AAC_DEC_OK) 
						{

#ifdef DBG_PRINT	
			fprintf(pFile,"DecodeFrame OK;\n");
			fflush(pFile);
#endif
							/*frame length varied from frame to frame */
							iLenDecOutPerChan = pinfo->frameSize;

							iResOutBlockSize =
								iLenDecOutPerChan * outputSampleRate / iAudioSampleRate;

							/* Additional buffers needed for resampling since we need conversation
							   between _REAL and _SAMPLE. */
							vecTempResBufInLeft.Init(iLenDecOutPerChan, (_REAL) 0.0);
							vecTempResBufInRight.Init(iLenDecOutPerChan, (_REAL) 0.0);

							if (pinfo->numChannels == 1)
							{
							    /* Mono */
							    // << "mono " << pinfo->frameSize << endl;
							    	for(int i = 0; i<pinfo->frameSize; i++) 
								{
							        	vecTempResBufInLeft[int(i)] = _REAL(decode_buf[i]) / 2.0;
							        	vecTempResBufInRight[int(i)] = _REAL(decode_buf[i]) / 2.0;
							    	}
							}
							else
							{
							    /* Stereo docs claim non-interleaved but we are getting interleaved! */
							    //cerr << "stereo " << iResOutBlockSize << endl;
							    	for(int i = 0; i< pinfo->frameSize; i++) 
								{
							        	vecTempResBufInLeft[int(i)] = _REAL(decode_buf[2*i]);
							        	vecTempResBufInRight[int(i)] = _REAL(decode_buf[2*i+1]);
							    	}
							}
									
						    
						}
						else
						{
							bCurBlockOK = false;


#ifdef DBG_PRINT						
		fprintf(pFile,"DecodeFrame error:0x%x; \n", err);
		fflush(pFile);
#endif
							if(err == AAC_DEC_NOT_ENOUGH_BITS) 
							{
							    cerr << "not enough bits in input buffer." << endl;
								
							    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
							}
							else if(err == AAC_DEC_PARSE_ERROR) 
							{
							    cerr << "error parsing bitstream." << endl;
							    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
							}

							else if(err == AAC_DEC_OUTPUT_BUFFER_TOO_SMALL) 
							{
							    cerr << "The provided output buffer is too small." << endl;
							    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
							}

							else if(err == AAC_DEC_OUT_OF_MEMORY) 
							{
							    cerr << "Heap returned NULL pointer. Output buffer is invalid." << endl;
							    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
							}
							else if(err == AAC_DEC_UNKNOWN) 
							{
							    cerr << "Error condition is of unknown reason, or from a another module. Output buffer is invalid." << endl;
							    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
							}
							else 
							{
							    cerr << "other error " << hex << int(err) << dec << endl;
							    //return CAudioCodec::DECODER_ERROR_UNKNOWN;
							}


						}

					}
					else
					{
#ifdef DBG_PRINT	
		fprintf(pFile,"Audio frame CRC error!\n"); 
		fflush(pFile);
#endif
					}

				}

				if(false == bCurBlockOK)	
				{

					//abort, this is corrupted frame (at the begin)

					//maybe fill in null for this frame?
					// or better defer that to reverb process? 
					
				}

			

			}
			else
				bCurBlockOK = false;
			
	/*------------------ my xhe ends here --------------------------------------------------------*/

			if(iAudioSampleRate != outputSampleRate) 
			{

	                	if(bCurBlockOK == true) 
				{
		                    /* Resample data */
		                    if(iResOutBlockSize != vecTempResBufOutLeft.Size()) 
					{ 
						// NOOP for AAC, needed for xHE-AAC
						vecTempResBufOutLeft.Init(iResOutBlockSize, 0.0);
						vecTempResBufOutRight.Init(iResOutBlockSize, 0.0);

		                    }

					if(ResampleObjL.GetMaxInputSize() != vecTempResBufInLeft.Size())
					{
						_REAL rRatio = _REAL(outputSampleRate) / _REAL(iAudioSampleRate);
						
						ResampleObjL.Init(vecTempResBufInLeft.Size(), rRatio);
						ResampleObjR.Init(vecTempResBufInLeft.Size(), rRatio);

						cerr << "ResampleObjL/R Init;"<< endl;

						

					}

 					if(vecTempResBufOutLeft.Size() == ResampleObjL.GetMaxOutputSize())
 					{
		                    	ResampleObjL.Resample(vecTempResBufInLeft, vecTempResBufOutLeft);
		                    	ResampleObjR.Resample(vecTempResBufInRight, vecTempResBufOutRight);
 					}

	             		}
				else
				{

		                    if(iResOutBlockSize != vecTempResBufOutLeft.Size()) 
					{ 
						// NOOP for AAC, needed for xHE-AAC
						vecTempResBufOutLeft.Init(iResOutBlockSize, 0.0);
						vecTempResBufOutRight.Init(iResOutBlockSize, 0.0);
		                    }
				

					
				}

						
	            	}
	            	else 
			{

	                	if(bCurBlockOK == true) 
				{

		                    if(iResOutBlockSize != vecTempResBufOutLeft.Size()) 
					{ 
						/* iResOutBlockSize is dynamically change for xhe-aac: frame length varied from frame to frame */
						// NOOP for AAC, needed for xHE-AAC
						vecTempResBufOutLeft.Init(iResOutBlockSize, 0.0);
						vecTempResBufOutRight.Init(iResOutBlockSize, 0.0);


		                    }

					if(vecTempResBufInLeft.Size() != vecTempResBufOutLeft.Size())
					{
						cerr << "vecTempResBufInLeft and vecTempResBufOutLeft mismatched;"<< endl;

						exit(0);

					}
								
					for(i = 0; i < vecTempResBufInLeft.Size(); i++)
					{
						vecTempResBufOutLeft[i] = vecTempResBufInLeft[i];
						vecTempResBufOutRight[i] = vecTempResBufInRight[i];
					}

	                	}
				else
				{


		                    if(iResOutBlockSize != vecTempResBufOutLeft.Size()) 
					{ 
						// NOOP for AAC, needed for xHE-AAC
						vecTempResBufOutLeft.Init(iResOutBlockSize, 0.0);
						vecTempResBufOutRight.Init(iResOutBlockSize, 0.0);
		                    }

	


				}
				
				
	            	}


	             	/* OPH: add frame status to vector for RSCI */
	            	Parameters.Lock();
	            	Parameters.vecbiAudioFrameStatus.Add(bCurBlockOK == true ? 0 : 1);
	            	Parameters.Unlock();
	        }
	        else
	        {
	            	/* DRM super-frame header was wrong, set flag to "bad block" */
	            	bCurBlockOK = false;
	            	/* OPH: update audio status vector for RSCI */
	            	Parameters.Lock();
	            	Parameters.vecbiAudioFrameStatus.Add(1);
	            	Parameters.Unlock();

					
	        }

		// This code is independent of particular audio source type and should work with all codecs
		
		/* Postprocessing of audio blocks, status informations -------------- */
		/*not applicale for different sizes in consecutive frames ie. xhe-aac*/
		ETypeRxStatus status = reverb.apply(bCurBlockOK, bCurBlockFaulty, vecTempResBufOutLeft, vecTempResBufOutRight);
	

		if (bCurBlockOK && !bCurBlockFaulty)
		{
		    	/* Increment correctly decoded audio blocks counter */
		    	iNumCorDecAudio++;
		}

		

		Parameters.Lock();
		Parameters.ReceiveStatus.SLAudio.SetStatus(status);
		Parameters.ReceiveStatus.LLAudio.SetStatus(status);
		Parameters.AudioComponentStatus[unsigned(Parameters.GetCurSelAudioService())].SetStatus(status);
		Parameters.Unlock();

		/* Conversion from _REAL to _SAMPLE with special function */
		if(bCurBlockOK)
			iDynamicMaxOutputBlockSize = iMaxOutputBlockSize;
		else 
		{
			
			if(iDynamicMaxOutputBlockSize == iMaxOutputBlockSize)
			{
				//Correct block -> Error block transition, cap on max number of
				// continuous garbage output lest overwhelm the downstream buffer
				iDynamicMaxOutputBlockSize = (iMaxOutputBlockSize >> 1);


			}
			else
			{
				iDynamicMaxOutputBlockSize = 0;

			}

		}


		for (int i = 0; (i < iResOutBlockSize) && (iOutputBlockSize < iDynamicMaxOutputBlockSize); i++, iOutputBlockSize += 2)
		{
		    (*pvecOutputData)[iOutputBlockSize] = Real2Sample(vecTempResBufOutLeft[i]);	/* Left channel */
		    (*pvecOutputData)[iOutputBlockSize + 1] = Real2Sample(vecTempResBufOutRight[i]);	/* Right channel */
		}



/*

		cerr <<"frameNumber:" << j <<"bGoodvalue:" << bGoodValues << " bCurBlockOK: " << bCurBlockOK<< "iOutputBlockSize:" <<iOutputBlockSize <<endl;
		{
		    double l = 0.0, r = 0.0;
		    for(int i=0; i<vecTempResBufOutLeft.Size(); i++) {
		        l += (vecTempResBufOutLeft[i]) * (vecTempResBufOutLeft[i]);
		        r += (vecTempResBufOutRight[i]) * (vecTempResBufOutRight[i]);
		    }
		    cerr << "energy after resampling and reverb left " << (l/vecTempResBufOutLeft.Size()) << " right " << (r/vecTempResBufOutRight.Size()) << endl;
		}

	
*/

		if(iOutputBlockSize==0) {
//		    cerr << "iOutputBlockSize is zero" << endl;
		}
		else 
		{
		    double d=0.0;
		    for (int i = 0; i < iOutputBlockSize; i++)
		    {
		        double n = (*pvecOutputData)[i];
		        d += n*n;
		    }
//		    cerr << "energy after converting " << iOutputBlockSize << " samples back to int " << sqrt(d/iOutputBlockSize) << endl;


			if(iOutputBlockSize == iMaxOutputBlockSize) {
			    cerr << "iOutputBlockSize is full, possible overflow of audio frame " << endl;
			}

		}

	}
}

void
CAudioSourceDecoder::InitInternal(CParameter & Parameters)
{

#ifdef DBG_PRINT	
	pFile = fopen("AudioSourceDec.dat", "w");
#endif

    /* Init error flags and output block size parameter. The output block
       size is set in the processing routine. We must set it here in case
       of an error in the initialization, this part in the processing
       routine is not being called */
    DoNotProcessAudDecoder = false;
    DoNotProcessData = false;
    iOutputBlockSize = 0;

    /* Set audiodecoder to empty string - means "unknown" and "can't decode" to GUI */
    audiodecoder = "FDK2.1";

    try
    {
        Parameters.Lock();

        /* Init counter for correctly decoded audio blocks */
        iNumCorDecAudio = 0;

        /* Init "audio was ok" flag */
        bAudioWasOK = true;

         /* Get number of total input bits for this module */
         iInputBlockSize = Parameters.iNumAudioDecoderBits;

        /* Get current selected audio service */
        CService& service = Parameters.Service[unsigned(Parameters.GetCurSelAudioService())];

        /* Get current audio coding */
        eAudioCoding = service.AudioParam.eAudioCoding;

        /* The requirement for this module is that the stream is used and the service is an audio service. Check it here */
        if ((service.eAudDataFlag != CService::SF_AUDIO) || (service.AudioParam.iStreamID == STREAM_ID_NOT_USED))
        {
            throw CInitErr(ET_ALL);
        }

        /* Current audio stream ID */
        unsigned iCurAudioStreamID =  unsigned(service.AudioParam.iStreamID);

        iTotalFrameSize = Parameters.Stream[iCurAudioStreamID].iLenPartA+Parameters.Stream[iCurAudioStreamID].iLenPartB;


        /* Init text message application ------------------------------------ */
       if (service.AudioParam.bTextflag)
       {
		bTextMessageUsed = true;

            	/* Get a pointer to the string */
            	TextMessage.Init(&service.AudioParam.strTextMessage);

            	/* Init vector for text message bytes */
            	vecbiTextMessBuf.Init(SIZEOF__BYTE * NUM_BYTES_TEXT_MESS_IN_AUD_STR);

            	iTotalFrameSize -= NUM_BYTES_TEXT_MESS_IN_AUD_STR;
        }
       else 
	{
            bTextMessageUsed = false;
       }
 
 
	if (eAudioCoding == CAudioParam::AC_AAC)
	{

		/* Init for AAC decoding -------------------------------------------- */
		int iAACSampleRate, iNumHeaderBytes;

		/* Length of higher protected part of audio stream */
		const int iLenAudHigh =
			Parameters.Stream[iCurAudioStreamID].iLenPartA;

		/* Set number of AAC frames in a AAC super-frame */
		switch (service.AudioParam.eAudioSamplRate)
		{ /* only 12 kHz and 24 kHz is allowed */
		case CAudioParam::AS_12KHZ:
			iNumAudioFrames = 5;
			iNumHeaderBytes = 6;
			iAACSampleRate = 12000;
			break;

		case CAudioParam::AS_24KHZ:
			iNumAudioFrames = 10;
			iNumHeaderBytes = 14;
			iAACSampleRate = 24000;
			break;

		default:
			/* Some error occurred, throw error */
			throw CInitErr(ET_AUDDECODER);
			break;
		}

		/* Number of borders */
		iNumBorders = iNumAudioFrames - 1;


		/* In case of SBR, AAC sample rate is half the total sample rate. Length
		   of output is doubled if SBR is used */
		if (service.AudioParam.
			eSBRFlag == CAudioParam::SB_USED)
		{
			iAudioSampleRate = iAACSampleRate * 2;
			iLenDecOutPerChan = AUD_DEC_TRANSFROM_LENGTH * 2;
		}
		else
		{
			iAudioSampleRate = iAACSampleRate;
			iLenDecOutPerChan = AUD_DEC_TRANSFROM_LENGTH;
		}

		/* The audio_payload_length is derived from the length of the audio
		   super frame (data_length_of_part_A + data_length_of_part_B)
		   subtracting the audio super frame overhead (bytes used for the audio
		   super frame header() and for the aac_crc_bits) (5.3.1.1, Table 5) */
		iAudioPayloadLen =
			iTotalFrameSize - iNumHeaderBytes - iNumAudioFrames;

		/* Check iAudioPayloadLen value, only positive values make sense */
		if (iAudioPayloadLen < 0)
			throw CInitErr(ET_AUDDECODER);

		/* Calculate number of bytes for higher protected blocks */
		iNumHigherProtectedBytes =
			(iLenAudHigh - iNumHeaderBytes - iNumAudioFrames /* CRC bytes */) /
			iNumAudioFrames;

		if (iNumHigherProtectedBytes < 0)
			iNumHigherProtectedBytes = 0;

		/* AAC decoder ------------------------------------------------------ */
		/* The maximum length for one audio frame is "iAudioPayloadLen". The
		   regular size will be much shorter since all audio frames share the
		   total size, but we do not know at this time how the data is 
		   split in the transmitter source coder */
		iMaxLenOneAudFrame = iAudioPayloadLen;
		audio_frame.Init(iNumAudioFrames, iMaxLenOneAudFrame);

		/* Init vector which stores the data with the CRC at the beginning
		   ("+ 1" for CRC) */
		vecbyPrepAudioFrame.Init(iMaxLenOneAudFrame + 1);

		/* Init storage for CRCs and frame lengths */
		aac_crc_bits.Init(iNumAudioFrames);
		veciFrameLength.Init(iNumAudioFrames);

		/* Init AAC-decoder */
//			faacDecInitDRM(HandleAACDecoder, iAACSampleRate, iDRMchanMode);

		

		//:DecOpen(const CAudioParam& AudioParam, int& iAudioSampleRate)

		unsigned type9Size;
		UCHAR *t9;
		
		hDecoder = aacDecoder_Open(TT_DRM, 3);

#ifdef DBG_PRINT	
fprintf(pFile, "hDecoder=0x%x;\n",hDecoder);
fflush(pFile);
#endif

		if(hDecoder == NULL) 
		{
		   	/* No xHE-AAC decoder available */
			throw CInitErr(ET_AUDDECODER);

		}

		vector<uint8_t> type9 = service.AudioParam.getType9Bytes();
		type9Size = unsigned(type9.size());
		t9 = &type9[0];

		AAC_DECODER_ERROR err = aacDecoder_ConfigRaw (hDecoder, &t9, &type9Size);

		if(err == AAC_DEC_OK) 
		{
#ifdef DBG_PRINT	
fprintf(pFile, "aacDecoder_ConfigRaw=0x%x;\n", err);
fflush(pFile);
#endif		
		    	CStreamInfo *pinfo = aacDecoder_GetStreamInfo(hDecoder);
		    	if (pinfo == NULL) 
			{

#ifdef DBG_PRINT	
fprintf(pFile, "aacDecoder_GetStreamInfo error;\n");
fflush(pFile);
#endif			
		        	cerr << "DecOpen No stream info" << endl;

			   	/* No xHE-AAC decoder available */
				throw CInitErr(ET_AUDDECODER);
			
		    	}


		    	if(pinfo->extSamplingRate != 0) 
			{
		        	iAudioSampleRate = pinfo->extSamplingRate; // get from AudioParam if codec couldn't get it
		    	}

#ifdef DBG_PRINT	
fprintf(pFile, "iAudioSampleRate:%d, aot:%d;\n", pinfo->extSamplingRate, pinfo->aot);
fflush(pFile);
#endif
		    	if(pinfo->aot == AOT_USAC) 
				cerr << "USAC bit stream! " << endl;  //USAC bitstream

		    	else if(pinfo->aot == AOT_DRM_USAC) 
				cerr << "USAC bit stream! " << endl; //USAC bitstream

		    	else 
				cerr << "NOT conforming USAC bit stream! " << endl;

		    

		}
		else
		{
			/* xHE-AAC decoder error */
			throw CInitErr(ET_AUDDECODER);

		}


	}
	else if (eAudioCoding == CAudioParam::AC_xHE_AAC)
	{
		/* Init for AC_xHEAAC decoding --------------------------------------- */
	
		/* init values for parsing */
		
		xHEAACPayload.Init(xHEAAC_PAYLOAD_BUFFSIZE);	// ring bufffer actually!
		xHEAACFrameSize.Init(xHEAAC_FRAME_BUFFSIZE, 0);
		xHEAACFrameStart.Init(xHEAAC_FRAME_BUFFSIZE, 0);


		
		iPayloadWrite = 0;
		
		DecodeFrame = ParsingFrame = 0;

		PrevParsingFrame = -1;  //negative means un-init state

		/*WARNING: following values are NOT determined untill after decoder */
		iLenDecOutPerChan = 0;

		iNumAudioFrames = 0; //only init for shortlog, it should be updated within process
		

		//:DecOpen(const CAudioParam& AudioParam, int& iAudioSampleRate)

		unsigned type9Size;
		UCHAR *t9;
		
		hDecoder = aacDecoder_Open(TT_DRM, 3);

#ifdef DBG_PRINT	
fprintf(pFile, "hDecoder=0x%x; \n", hDecoder);
fflush(pFile);
#endif


		// provide a default value for iAudioSampleRate in case we can't do better. TODO xHEAAC
		int iDefaultSampleRate;
		switch (service.AudioParam.eAudioSamplRate)
		{

		case CAudioParam::AS_9_6KHZ:
		    	iDefaultSampleRate = 9600;
		    	break;
		
		case CAudioParam::AS_12KHZ:
		    	iDefaultSampleRate = 12000;
		    	break;

		case CAudioParam::AS_16KHZ:
		    	iDefaultSampleRate = 16000;
		    	break;

		case CAudioParam::AS_19_2KHZ:
		    	iDefaultSampleRate = 19200;
		    	break;


		case CAudioParam::AS_24KHZ:
		    	iDefaultSampleRate = 24000;
		    	break;


		case CAudioParam::AS_32KHZ:
		    	iDefaultSampleRate = 32000;
		    	break;

		case CAudioParam::AS_38_4KHZ:
		    	iDefaultSampleRate = 38400;
		    	break;

		case CAudioParam::AS_48KHZ:
		    	iDefaultSampleRate = 48000;
		    	break;

		default:
		    iDefaultSampleRate = 12000;
		    break;
		}

		if (service.AudioParam.eSBRFlag == CAudioParam::SB_USED)
		{
		    	iDefaultSampleRate = iDefaultSampleRate * 2;
		}

		if(hDecoder == NULL) 
		{
		    	iAudioSampleRate = iDefaultSampleRate;
				
		   	/* No xHE-AAC decoder available */
			throw CInitErr(ET_AUDDECODER);

		}

		vector<uint8_t> type9 = service.AudioParam.getType9Bytes();
		type9Size = unsigned(type9.size());
		t9 = &type9[0];

		AAC_DECODER_ERROR err = aacDecoder_ConfigRaw (hDecoder, &t9, &type9Size);

		if(err == AAC_DEC_OK) 
		{
#ifdef DBG_PRINT	
fprintf(pFile, "aacDecoder_ConfigRaw=0x%x, type9Size:%d;\n", err, type9Size);
fflush(pFile);
#endif		
		    	CStreamInfo *pinfo = aacDecoder_GetStreamInfo(hDecoder);
		    	if (pinfo == NULL) 
			{

#ifdef DBG_PRINT	
fprintf(pFile, "aacDecoder_GetStreamInfo error;\n");
fflush(pFile);
#endif			
		        	cerr << "DecOpen No stream info" << endl;
		        	iAudioSampleRate = iDefaultSampleRate;

			   	/* No xHE-AAC decoder available */
				throw CInitErr(ET_AUDDECODER);
			
		    	}

		    	iAudioSampleRate = pinfo->extSamplingRate;


		    	if(iAudioSampleRate == 0) 
			{
		        	iAudioSampleRate = iDefaultSampleRate; // get from AudioParam if codec couldn't get it
		    	}

#ifdef DBG_PRINT	
fprintf(pFile, "iAudioSampleRate:%d, aot:%d;\n", pinfo->extSamplingRate, pinfo->aot);
fflush(pFile);
#endif
		    	if(pinfo->aot == AOT_USAC) 
				cerr << "USAC bit stream! " << endl;  //USAC bitstream

		    	else if(pinfo->aot == AOT_DRM_USAC) 
				cerr << "USAC bit stream! " << endl; //USAC bitstream

		    	else 
				cerr << "NOT conforming USAC bit stream! " << endl;

		    

		}
		else
		{
			/* xHE-AAC decoder error */
			throw CInitErr(ET_AUDDECODER);

		}

	}
	else
	{
		/* Audio codec not supported */
		throw CInitErr(ET_AUDDECODER);
	}

	iNumberChannels = 
		((service.AudioParam.eAudioMode == CAudioParam::AM_MONO) ? 1 : 2);


        if(bWriteToFile)
        {
//           codec->openFile(Parameters);
        }
        else 
	{
//            codec->closeFile();
        }



        /* set string for GUI */
        Parameters.audiodecoder = audiodecoder;

        /* Set number of Audio frames for log file */
        Parameters.iNumAudioFrames = iNumAudioFrames;

        outputSampleRate = Parameters.GetAudSampleRate();
        Parameters.Unlock();

        /* Since we do not do Mode E or correct for sample rate offsets here (yet), we do not
           have to consider larger buffers. An audio frame always corresponds to 400 ms */
        int iMaxLenResamplerOutput = int(_REAL(outputSampleRate) * 0.4 /* 400ms */  * 2 /* for stereo */ );
		iMaxLenResamplerOutput *= OUTPUT_BUFFER_OVERHEAD_MARGIN;    // to prevent buffer overruns with xHE-AAC (as detected by clang asan)

        if(iAudioSampleRate != outputSampleRate) {
            _REAL rRatio = _REAL(outputSampleRate) / _REAL(iAudioSampleRate);
            /* Init resample objects */
            ResampleObjL.Init(iLenDecOutPerChan, rRatio);
            ResampleObjR.Init(iLenDecOutPerChan, rRatio);
        }

        iResOutBlockSize = outputSampleRate * iLenDecOutPerChan / iAudioSampleRate;

        //cerr << "output block size per channel " << iResOutBlockSize << " = samples " << iLenDecOutPerChan << " * " << Parameters.GetAudSampleRate() << " / " << iAudioSampleRate << endl;

        /* Additional buffers needed for resampling since we need conversation
           between _REAL and _SAMPLE. We have to init the buffers with
           zeros since it can happen, that we have bad CRC right at the
           start of audio blocks */
        vecTempResBufInLeft.Init(iLenDecOutPerChan, 0.0);
        vecTempResBufInRight.Init(iLenDecOutPerChan, 0.0);
        vecTempResBufOutLeft.Init(iResOutBlockSize, 0.0);
        vecTempResBufOutRight.Init(iResOutBlockSize, 0.0);

        reverb.Init(outputSampleRate, bUseReverbEffect);

        /* With this parameter we define the maximum length of the output
           buffer. The cyclic buffer is only needed if we do a sample rate
           correction due to a difference compared to the transmitter. But for
           now we do not correct and we could stay with a single buffer
           Maybe TODO: sample rate correction to avoid audio dropouts */
        iMaxOutputBlockSize = iMaxLenResamplerOutput;

	  iDynamicMaxOutputBlockSize = 0;

  cerr << "init: iResOutBlockSize=" << iResOutBlockSize << endl;

		
    }

    catch(CInitErr CurErr)
    {
        Parameters.Unlock();

        switch (CurErr.eErrType)
        {
        case ET_ALL:
            /* An init error occurred, do not process data in this module */
            DoNotProcessData = true;
            break;

        case ET_AUDDECODER:
            /* Audio part should not be decoded, set flag */
            DoNotProcessAudDecoder = true;
            break;
        }

        /* In all cases set output size to zero */
        iOutputBlockSize = 0;
    }
}

int
CAudioSourceDecoder::GetNumCorDecAudio()
{
    /* Return number of correctly decoded audio blocks. Reset counter
       afterwards */
    const int iRet = iNumCorDecAudio;

    iNumCorDecAudio = 0;

    return iRet;
}


CAudioSourceDecoder::CAudioSourceDecoder()
    :	bWriteToFile(false), TextMessage(false),
      bUseReverbEffect(true)
{

	hDecoder = NULL;


}

CAudioSourceDecoder::~CAudioSourceDecoder()
{
 
    	if(hDecoder != NULL)
	{
        	aacDecoder_Close(hDecoder);
        	hDecoder = NULL;

	}

	
}


