// Stub drm_kiwiaudio.h for standalone DRM build
#ifndef _DRM_KIWIAUDIO_H
#define _DRM_KIWIAUDIO_H

#include "../dream/sound/soundinterface.h"

using std::string;

class CKiwiCommon: public CSelectionInterface
{
public:
    CKiwiCommon(bool);
    virtual     ~CKiwiCommon();

    virtual void	Enumerate(std::vector<string>& names, std::vector<string>& descriptions);
    virtual void	SetDev(std::string sNewDevice);
    virtual std::string	GetDev();

    bool		Init(int iSampleRate, int iNewBufferSize, bool bNewBlocking);
    void        ReInit();
    bool		Write(CVector<short>& psData);
    void        Close();

    int xruns;

protected:
    std::vector<string> names;
    std::string dev;
    bool is_capture, blocking, device_changed, xrun;
    int framesPerBuffer;
    int iBufferSize;
    double samplerate;
};

class CSoundOutKiwi: public CSoundOutInterface
{
public:
    CSoundOutKiwi();
    virtual         ~CSoundOutKiwi();
    virtual void	Enumerate(std::vector<string>& names, std::vector<string>& descriptions)
    {
        hw.Enumerate(names, descriptions);
    }
    virtual void	SetDev(std::string sNewDevice)
    {
        hw.SetDev(sNewDevice);
    }
    virtual std::string	GetDev()
    {
        return hw.GetDev();
    }
    virtual std::string	GetVersion()
    {
        return "kiwiaudio out stub";
    }
    virtual bool	Init(int iSampleRate, int iNewBufferSize, bool bNewBlocking);
    virtual void    Close();
    virtual bool	Write(CVector<short>& psData);

protected:
    CKiwiCommon hw;
};

#endif
