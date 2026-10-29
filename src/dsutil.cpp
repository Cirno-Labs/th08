// The file from 0x4716e0: the DirectX 8 SDK's dsutil.cpp, as the game
// builds it.

#include "dsutil.h"
#include "unsorted.h"

// From the SDK's dxutil.h.
#define SAFE_DELETE(p) { if(p) { delete (p); (p)=NULL; } }
#define SAFE_DELETE_ARRAY(p) { if(p) { delete[] (p); (p)=NULL; } }
#define SAFE_RELEASE(p) { if(p) { (p)->Release(); (p)=NULL; } }
// From the SDK's dxerr8.h, as in a release build.
#define DXTRACE_ERR(str, hr) (hr)

CSoundManager::CSoundManager()
{
    m_pDS = NULL;
}

CSoundManager::~CSoundManager()
{
    SAFE_RELEASE(m_pDS);
}
