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

#ifndef _CUTRAX_H_
#define _CUTRAX_H_

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "CAEStreamingDecoder.hpp"

class CUtrax
{
public:
	struct Track
	{
		int trackID;
		size_t pathLength;
		int decoderID;
	};

	static CUtrax *GetInstance();
	// No destructor
	~CUtrax() = delete;

	// 0x4e7c70
	bool LoadQuickTime();
	// 0x4f2fd0
	bool ReadSaUtraxDat();
	// 0x4f3050
	char *GetTrackPath(int trackID);
	// 0x4f35b0
	bool Initialize();
	// 0x4f35f0
	CAEStreamingDecoder *LoadUserTrack(int trackID);
	// 0x4f4ba0
	bool StartWriteUtraxThread();
	// 0x4f4a20
	DWORD __stdcall WriteUtraxThread();

	// IF YOU WANT TO EXPORT THIS FILE TO YOUR REVERSING PROJECT
	// MAKE SURE YOU REMOVE THIS ONE FUNCTION BECAUSE IT'S USED
	// TO INJECT STUFF IN THIS PROJECT
	CAEStreamingDecoder *LoadUserTrackOverride(int trackID);

public:
	Track *tracks;
	bool utraxLoaded1;
	bool utraxLoaded2;
	unsigned short trackCount;
	union
	{
		bool decoderSupportedArray[8];
		struct
		{
			bool
				nullStreamSupported,   // always false
				vorbisSupported,       // always true
				waveSupported,         // always true
				windowsMediaSupported, // true if wmvcore.dll loaded
				quickTimeSupported,
				flacSupported;         // SilentPatch-specific
		};
	} decoderSupported;
	DWORD scanThreadID; // set when game scanning user tracks
	HANDLE scanThreadHandle; // set when game scanning user tracks
	int utraxState; // from 0..3, if 0, 2 variable above is set then this set to 1

private:
	// No constructor
	CUtrax();
};

#endif
