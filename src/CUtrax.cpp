/**
 * Copyright (c) 2020 Miku AuahDark
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

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
