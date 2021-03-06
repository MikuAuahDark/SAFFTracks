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

#include <string>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
}

void main();
void unload();

BOOL WINAPI DllMain(HINSTANCE _, DWORD why, LPVOID __)
{
	switch (why)
	{
		case DLL_PROCESS_ATTACH:
		{
			main();
			break;
		}
		case DLL_PROCESS_DETACH:
		{
			unload();
			break;
		}
	}

	return 1;
}

/////////////////////////////
// End of boilerplate code //
//  Real code starts here  //
/////////////////////////////

#include <new>

#include "CAEDataStream.hpp"
#include "CAELAVDecoder.hpp"
#include "CAEUserRadioTrackManager.hpp"

static bool silentPatched = false; // true if SilentPatch patches a function
static bool patched[] = {
	false, // 0x4f31f0 patch
	false, // decoder ID 6 patch
	false, // 0x4f35f0
};

uintptr_t gtasaBase = 0;

// Our function to override CAEUserRadioTrackManager::LoadUserTrack
CAEStreamingDecoder *CAEUserRadioTrackManager::LoadUserTrackOverride(int trackID)
{
	// To be honest this is 70% rewrite of CAEUserRadioTrackManager::LoadUserTrack method
	CAEUserRadioTrackManager &inst = *CAEUserRadioTrackManager::GetInstance();

	if (inst.utraxLoaded1 == false)
		return nullptr;

	if (trackID < 0 || trackID > inst.trackCount)
		return nullptr;

	// We don't really care of the decoder/stream ID, so we just check if it's not 0
	int decoderID = inst.tracks[trackID].decoderID;
	if (decoderID == 0)
		return nullptr;

	// "path" is freed by CAEDataStream destructor
	char *path = inst.GetTrackPath(trackID);
	if (path == nullptr)
		return nullptr;

	CAEDataStream *dataStream = new CAEDataStream(trackID, path, 0, 0, false);
	if (dataStream->Initialise() == false)
	{
		delete dataStream;
		return nullptr;
	}

	// Now create new CAELAVDecoder
	return new CAELAVDecoder(dataStream);
}

int __stdcall getAudioFileTypeOverride(const char *str)
{
	static std::string userTracksDir;
	if (userTracksDir.empty())
	{
		// InitUserDirectories call
		userTracksDir = ((const char*(__stdcall *)()) (gtasaBase + 0x344fb0))();
		userTracksDir += "\\User Tracks\\";
	}

	std::string filename = str;
	std::string path;

	// Normalize paths because GTASA sometimes getting silly
	if (filename.find(userTracksDir) == 0)
	{
		size_t p = filename.find("\\\\");
		if (p == std::string::npos)
			path = filename;
		else
			path = filename.substr(0, p) + filename.substr(p + 1);
	}
	else
		path = userTracksDir + filename;

	// Let's open it with libav
	AVFormatContext *formatContext = nullptr;
	if (avformat_open_input(&formatContext, path.c_str(), nullptr, nullptr) < 0)
		return 0;

	if (avformat_find_stream_info(formatContext, nullptr) < 0)
	{
		avformat_close_input(&formatContext);
		return 0;
	}

	int audioStream = -1;
	// Find audio stream
	for (unsigned int i = 0; i < formatContext->nb_streams; i++)
	{
		if (formatContext->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO)
			audioStream = i;
		else
			formatContext->streams[i]->discard = AVDISCARD_ALL;
	}
	if (audioStream == -1)
	{
		// No audio stream
		avformat_close_input(&formatContext);
		return 0;
	}

	AVCodecID audioCodecID = formatContext->streams[audioStream]->codecpar->codec_id;
	avformat_close_input(&formatContext);

	// Ensure we have the decoder for particular codec ID
	if (avcodec_find_decoder(audioCodecID) == nullptr)
		return 0;

	// Return stream ID by their codec ID
	switch (audioCodecID)
	{
		// Vorbis
		case AV_CODEC_ID_VORBIS:
			return 1;
		// WAV
		case AV_CODEC_ID_PCM_S16LE:
			return 2;
		// WMA family
		case AV_CODEC_ID_WMALOSSLESS:
		case AV_CODEC_ID_WMAPRO:
		case AV_CODEC_ID_WMAV1:
		case AV_CODEC_ID_WMAV2:
		case AV_CODEC_ID_WMAVOICE:
			return 3;
		// MP3 is 4 if system has QuickTime, 3 otherwise.
		case AV_CODEC_ID_MP3:
			return CAEUserRadioTrackManager::GetInstance()->decoderSupported.quickTimeSupported ? 4 : 3;
		// AAC
		case AV_CODEC_ID_AAC:
			return 4;
		// SilentPatch assign stream ID 5 to FLAC audio
		case AV_CODEC_ID_FLAC:
			return 5;
		// And other formats provided by this plugin will be using stream ID 6
		default:
			return 6;
	}
}

void main()
{
	gtasaBase = (uintptr_t) GetModuleHandleA(nullptr);

	// Check SilentPatch presence
	{
		uint8_t *songExtAddr = (uint8_t *) (gtasaBase + 0xf3210);
		uintptr_t addr =
			uint32_t(songExtAddr[0]) |
			(uint32_t(songExtAddr[1]) << 8) |
			(uint32_t(songExtAddr[2]) << 16) |
			(uint32_t(songExtAddr[3]) << 24);
		silentPatched = addr != (gtasaBase + 5028648);
	}
	
	// Patch 0x4f31f0 with getAudioFileTypeOverride
	// e9 &getAudioFileTypeOverride - (gtasaBase + 0xf31f0) - 5
	{
		DWORD oldProt = 0;
		DWORD newProt = PAGE_READWRITE;
		if (!VirtualProtect((void *) (gtasaBase + 0xf31f0), 0x10, newProt, &oldProt))
			return;

		uint8_t *dest = (uint8_t *) (gtasaBase + 0xf31f0);
		uintptr_t jumpAddr = (uintptr_t) &getAudioFileTypeOverride - (gtasaBase + 0xf31f0) - 5;
		dest[0] = 0xe9;
		dest[1] = (uint8_t) jumpAddr;
		dest[2] = (uint8_t) (jumpAddr >> 8);
		dest[3] = (uint8_t) (jumpAddr >> 16);
		dest[4] = (uint8_t) (jumpAddr >> 24);

		VirtualProtect((void *) (gtasaBase + 0xf31f0), 0x10, oldProt, &newProt);
		patched[0] = true;
	}

	// Toggle decoder ID 6 to true in CAEUserRadioTrackManager singleton
	CAEUserRadioTrackManager::GetInstance()->decoderSupported.decoderSupportedArray[6] = patched[1] = true;

	// Patch CAEUserRadioTrackManager::LoadUserTrack (0x4f35f0) to CAEUserRadioTrackManager::LoadUserTrackOverride
	{
		DWORD oldProt = 0;
		DWORD newProt = PAGE_READWRITE;
		if (!VirtualProtect((void *) (gtasaBase + 0xf35f0), 0x10, newProt, &oldProt))
			return;

		uint8_t *dest = (uint8_t *) (gtasaBase + 0xf35f0);

		uintptr_t loadUserTrackOverride;
		__asm { // why
			mov eax, (CAEUserRadioTrackManager::LoadUserTrackOverride)
			mov loadUserTrackOverride, eax
		}
		uintptr_t jumpAddr = loadUserTrackOverride - (gtasaBase + 0xf35f0) - 5;

		dest[0] = 0xe9;
		dest[1] = (uint8_t) jumpAddr;
		dest[2] = (uint8_t) (jumpAddr >> 8);
		dest[3] = (uint8_t) (jumpAddr >> 16);
		dest[4] = (uint8_t) (jumpAddr >> 24);

		VirtualProtect((void *) (gtasaBase + 0xf35f0), 0x10, oldProt, &newProt);
		patched[2] = true;
	}
}

void unload()
{
	// From reverse order
	// Restore CAEUserRadioTrackManager::LoadUserTrack patch
	if (patched[2])
	{
		DWORD oldProt = 0;
		DWORD newProt = PAGE_READWRITE;
		if (!VirtualProtect((void *) (gtasaBase + 0xf35f0), 0x10, newProt, &oldProt))
			return;

		uint8_t *dest = (uint8_t *) (gtasaBase + 0xf35f0);

		dest[0] = 0x6a;
		dest[1] = 0xff;
		dest[2] = 0x68;
		dest[3] = 0xc7;
		dest[4] = 0xc1;

		VirtualProtect((void *) (gtasaBase + 0xf35f0), 0x10, oldProt, &newProt);
		patched[2] = false;
	}

	// Set decoder ID 6 to false
	if (patched[1])
		patched[1] = CAEUserRadioTrackManager::GetInstance()->decoderSupported.decoderSupportedArray[6] = false;

	// Revert patch 0x4f31f0
	if (patched[0])
	{
		DWORD oldProt = 0;
		DWORD newProt = PAGE_READWRITE;
		if (!VirtualProtect((void *) (gtasaBase + 0xf31f0), 0x10, newProt, &oldProt))
			return;

		uint8_t *dest = (uint8_t *) (gtasaBase + 0xf31f0);

		dest[0] = 0x8b;
		dest[1] = 0x44;
		dest[2] = 0x24;
		dest[3] = 0x04;
		dest[4] = 0x55;

		VirtualProtect((void *) (gtasaBase + 0xf31f0), 0x10, oldProt, &newProt);
		patched[0] = false;
	}
}
