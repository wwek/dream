// Stub drm_kiwiaudio.cpp for standalone DRM build
#include "drm_kiwiaudio.h"
#include "../dream/sound/AudioFileIn.h"

// CKiwiCommon implementations
CKiwiCommon::CKiwiCommon(bool capture) :
    is_capture(capture), blocking(false), device_changed(false),
    xrun(false), framesPerBuffer(0), iBufferSize(0), samplerate(0), xruns(0) {}

CKiwiCommon::~CKiwiCommon() {}

void CKiwiCommon::Enumerate(std::vector<string>& names, std::vector<string>& descriptions) {
    (void)names;
    (void)descriptions;
}

void CKiwiCommon::SetDev(std::string sNewDevice) {
    dev = sNewDevice;
}

std::string CKiwiCommon::GetDev() {
    return dev;
}

bool CKiwiCommon::Init(int iSampleRate, int iNewBufferSize, bool bNewBlocking) {
    (void)iSampleRate;
    (void)iNewBufferSize;
    (void)bNewBlocking;
    return false;
}

void CKiwiCommon::ReInit() {}

bool CKiwiCommon::Write(CVector<short>& psData) {
    (void)psData;
    return false;
}

void CKiwiCommon::Close() {}

// CSoundOutKiwi implementations
CSoundOutKiwi::CSoundOutKiwi() : hw(false) {}

CSoundOutKiwi::~CSoundOutKiwi() {}

bool CSoundOutKiwi::Init(int iSampleRate, int iNewBufferSize, bool bNewBlocking) {
    return hw.Init(iSampleRate, iNewBufferSize, bNewBlocking);
}

void CSoundOutKiwi::Close() {
    hw.Close();
}

bool CSoundOutKiwi::Write(CVector<short>& psData) {
    return hw.Write(psData);
}

// CAudioFileIn stub implementations - only implement methods not defined inline in the header
CAudioFileIn::CAudioFileIn() :
    interleaved(0), eFmt(fmt_other), pFileReceiver(NULL),
    iSampleRate(0), iRequestedSampleRate(0), iBufferSize(0),
    iFileSampleRate(0), iFileChannels(0), pacer(NULL),
    ResampleObjL(NULL), ResampleObjR(NULL), buffer(NULL), iOutBlockSize(0) {}

CAudioFileIn::~CAudioFileIn() {}

void CAudioFileIn::SetFileName(const std::string& strFileName, FileTyper::type type) {
    (void)strFileName;
    (void)type;
}

bool CAudioFileIn::Init(int iNewSampleRate, int iNewBufferSize, bool bNewBlocking) {
    (void)iNewSampleRate;
    (void)iNewBufferSize;
    (void)bNewBlocking;
    return false;
}

void CAudioFileIn::Close() {}

bool CAudioFileIn::Read(CVector<short>& psData) {
    (void)psData;
    return false;
}
