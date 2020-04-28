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

#ifndef _CAELAVDECODER_H_
#define _CAELAVDECODER_H_

#include "CAEStreamingDecoder.hpp"

extern "C"
{
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libswresample/swresample.h"
}

class CAELAVDecoder: public CAEStreamingDecoder
{
public:
	CAELAVDecoder(CAEDataStream *dataStream);
	~CAELAVDecoder() override;
	
	bool Initialise() override;
	unsigned long FillBuffer(void *dest, size_t size) override;
	long GetStreamLengthMs() override;
	long GetStreamPlayTimeMs() override;
	void SetCursor(unsigned long pos) override;
	int GetSampleRate() override;
	int GetStreamID() override;

	struct Metadata
	{
		const char
			*filename,
			*title,
			*artist,
			*album,
			*genre;
	};

private:
	char signature[4];
	Metadata metadata;
	AVFormatContext *formatContext;
	AVCodecContext *codecContext;
	AVIOContext *ioContext;
	AVPacket *packet;
	AVFrame *frame;
	SwrContext *resampler;

	int targetIndex;
	int sampleRate;
	bool init0;
	bool init;

	bool ReadPacket();
	bool ReadFrame(AVFrame *frame);
};

#endif
