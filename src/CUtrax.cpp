#include <cstdint>

#include "CUtrax.hpp"

extern uintptr_t gtasaBase;

CUtrax* CUtrax::GetInstance()
{
	static CUtrax *instance = nullptr;
	if (instance == nullptr)
		instance = (CUtrax *) (gtasaBase + 0x76b970);
	return instance;
}

bool CUtrax::LoadQuickTime()
{
	return ((bool(__thiscall*)(CUtrax*)) (gtasaBase + 0xe7c70))(this);
}

bool CUtrax::ReadSaUtraxDat()
{
	return ((bool(__thiscall*)(CUtrax*)) (gtasaBase + 0xf2fd0))(this);
}

char* CUtrax::GetTrackPath(int trackID)
{
	return ((char*(__thiscall*)(CUtrax*, int)) (gtasaBase + 0xf3050))(this, trackID);
}

bool CUtrax::Initialize()
{
	return ((bool(__thiscall*)(CUtrax*)) (gtasaBase + 0xf35b0))(this);
}

CAEStreamingDecoder* CUtrax::LoadUserTrack(int trackID)
{
	return ((CAEStreamingDecoder*(__thiscall*)(CUtrax*, int)) (gtasaBase + 0xf35f0))(this, trackID);
}

bool CUtrax::StartWriteUtraxThread()
{
	return ((bool(__thiscall*)(CUtrax*)) (gtasaBase + 0xf4ba0))(this);
}

DWORD __stdcall CUtrax::WriteUtraxThread()
{
	using WriteFunc = DWORD(__stdcall *)(CUtrax*);
	return ((WriteFunc) (gtasaBase + 0xf4a20))(this);
}
