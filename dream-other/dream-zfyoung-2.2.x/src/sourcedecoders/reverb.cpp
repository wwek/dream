#include "reverb.h"

Reverb::Reverb()
{

}

#define MAX_FRAME_SIZE     (13840u)  //max input audio frame size in sample per channel
void Reverb::Init(int outputSampleRate, bool bUse)
{
    /* Clear reverberation object */
    AudioRev.Init(1.0 /* seconds delay */, outputSampleRate);
    AudioRev.Clear();
    bUseReverbEffect = bUse;
    OldLeft.Init(1, 0.0);
    OldRight.Init(1, 0.0);

    	FIFOLeft.Init(MAX_FRAME_SIZE);  
 	FIFORight.Init(MAX_FRAME_SIZE);
}

ETypeRxStatus Reverb::apply(bool bCurBlockOK, bool bCurBlockFaulty, CVector<_REAL>& CurLeft, CVector<_REAL>& CurRight)
{

	
    	int iResOutBlockSize = CurLeft.Size();
 
 

	if(WorkLeft.Size()!= iResOutBlockSize)
	{
		WorkLeft.Init(iResOutBlockSize, 0.0);

		WorkRight.Init(iResOutBlockSize, 0.0);
	}
	
	int i, j;
	int OldSize = OldLeft.Size();
	bool sign = false; 
	for(i = iResOutBlockSize - 1, j = OldSize - 1; i >= 0; i--)
	{
		/*periodic expansion */
		WorkLeft[i] = OldLeft[j];
		WorkRight[i] = OldRight[j];

		if(sign == false)
		{
			if(j = 0)
			{
				j++;
                            sign = true;				
   
			}
			else	
				j--;

		}
		else
		{
			if(j == (OldSize - 1))
			{
				j--;

				sign = false;

			}
			else
				j++;


		}
	}
	


    ETypeRxStatus status = DATA_ERROR;
    if (bCurBlockOK == false)
    {
        if (bAudioWasOK)
        {
            /* Post message to show that CRC was wrong (yellow light) */
            status = DATA_ERROR;

            /* Fade-out old block to avoid "clicks" in audio. We use linear
               fading which gives a log-fading impression */
            for (int i = 0; i < iResOutBlockSize; i++)
            {
                /* Linear attenuation with time of OLD buffer */
                const _REAL rAtt = 1.0 - _REAL(i / iResOutBlockSize);

                WorkLeft[i] *= rAtt;
                WorkRight[i] *= rAtt;

                if (bUseReverbEffect)
                {
                    /* Fade in input signal for reverberation to avoid clicks */
                    const _REAL rAttRev = _REAL( i / iResOutBlockSize);

                    /* Cross-fade reverberation effect */
                    const _REAL rRevSam = (1.0 - rAtt) * AudioRev.ProcessSample(WorkLeft[i] * rAttRev, WorkRight[i] * rAttRev);

                    /* Mono reverbration signal */
                    WorkLeft[i] += rRevSam;
                    WorkRight[i] += rRevSam;
                }
            }

            /* Set flag to show that audio block was bad */
            bAudioWasOK = false;
        }
        else
        {
            status = CRC_ERROR;

            if (bUseReverbEffect)
            {
                /* Add Reverberation effect */
                for (int i = 0; i < iResOutBlockSize; i++)
                {
                    /* Mono reverberation signal */
                    WorkLeft[i] = WorkRight[i] = AudioRev.ProcessSample(0, 0);
                }
            }
	     else
	     {

                for (int i = 0; i < iResOutBlockSize; i++)
                {
                    /* Mono reverberation signal */
                    WorkLeft[i] = WorkRight[i] = 0.0;
                }

	     	}
        }


    }
    else
    {
        /* Increment correctly decoded audio blocks counter */
        if (bCurBlockFaulty) {
            status = DATA_ERROR;
        }
        else {
            status = RX_OK;
        }

        if (bAudioWasOK == false)
        {


            /* Fade-in new block to avoid "clicks" in audio. We use linear
               fading which gives a log-fading impression */
            for (int i = 0; i < iResOutBlockSize; i++)
            {
                /* Linear attenuation with time */
                const _REAL rAtt = _REAL( i / iResOutBlockSize);

 		   _REAL rRevSam = 0.0;
                if (bUseReverbEffect)
                {

                    /* Fade in input signal for reverberation to avoid clicks */
                    const _REAL rAttRev = 1.0 - _REAL( i / iResOutBlockSize);

				
                    /* Cross-fade reverberation effect */
                    rRevSam = (1.0 - rAtt) * AudioRev.ProcessSample(WorkLeft[i] * rAttRev, WorkRight[i] * rAttRev);

                }
				
                WorkLeft[i] = CurLeft[i] * rAtt + rRevSam;
                WorkRight[i] = CurRight[i] * rAtt+ rRevSam;


				
            }

            /* Reset flag */
            bAudioWasOK = true;
        }
        else
        {
        	
		for (int i = 0; i < iResOutBlockSize; i++)
              { 
              	WorkLeft[i] = CurLeft[i];
                	WorkRight[i] = CurRight[i];

		}


        }
		
    }

    if(OldLeft.Size()!=iResOutBlockSize) OldLeft.Init(iResOutBlockSize, 0.0);
    if(OldRight.Size()!=iResOutBlockSize) OldRight.Init(iResOutBlockSize, 0.0);

 	/*store current block for next iteration*/

   if(bCurBlockOK == true)	
   { 
   	for (int i = 0; i < iResOutBlockSize; i++)
    	{
       	 OldLeft[i] = CurLeft[i];
       	 OldRight[i] = CurRight[i];
    	}
   }
	/* store processed data to FIFO and pop out old data to output */
   for (int i = 0; i < iResOutBlockSize; i++)
    {

	CurLeft[i] = FIFOLeft.Get();
	FIFOLeft.Add(WorkLeft[i]);
	
 	CurRight[i] = FIFORight.Get();
	FIFORight.Add(WorkRight[i]);
    }

    if(bCurBlockOK == false)	
   { 
   	for (int i = 0; i < iResOutBlockSize; i++)
    	{
       	 OldLeft[i] = CurLeft[i];
       	 OldRight[i] = CurRight[i];
    	}
   }


    return status;
}


#if 0
ETypeRxStatus Reverb::Myapply(bool bCurBlockOK, bool bCurBlockFaulty, CVector<_REAL>& CurLeft, CVector<_REAL>& CurRight)
{
    int iResOutBlockSize = CurLeft.Size();

    bool okToReverb = bUseReverbEffect;

    vector<_REAL> tempLeft, tempRight;
   

    ETypeRxStatus status = DATA_ERROR;
    if (bCurBlockOK == false)
    {
        if (bAudioWasOK)
        {
            /* Post message to show that CRC was wrong (yellow light) */
            status = DATA_ERROR;

 

            /* Set flag to show that audio block was bad */
            bAudioWasOK = false;
        }
        else
        {
            status = CRC_ERROR;
			
        }

    }
    else
    {
        /* Increment correctly decoded audio blocks counter */
        if (bCurBlockFaulty) {
            status = DATA_ERROR;
        }
        else {
            status = RX_OK;
        }

        if (bAudioWasOK == false)
        {
            /* Reset flag */
            bAudioWasOK = true;
        }
    }

    /* Store reverberated block into output */
	if(bCurBlockOK == false)
	{   

		for (int i = 0; i < iResOutBlockSize; i++)
	    	{
	        	CurLeft[i] = 0;
	        	CurRight[i] = 0;
	    	}

	}   
    return status;
}

#endif

