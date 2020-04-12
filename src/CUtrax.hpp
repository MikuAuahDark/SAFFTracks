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
