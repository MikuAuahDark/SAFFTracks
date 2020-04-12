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

private:
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
