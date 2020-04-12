#ifndef _CAESTREAMINGDECODER_H_
#define _CAESTREAMINGDECODER_H_

#include "SADefaultAllocator.hpp"
#include "CAEDataStream.hpp"

class CAEStreamingDecoder: public SADefaultAllocator
{
public:
	// 0x4f2810
	CAEStreamingDecoder(CAEDataStream *dataStream);
	// 0x4f2860
	void operator delete(void* mem);

	virtual bool Initialise() = 0;
	virtual unsigned long FillBuffer(void *dest, size_t size) = 0;
	virtual long GetStreamLengthMs() = 0;
	virtual long GetStreamPlayTimeMs() = 0;
	virtual void SetCursor(unsigned long pos) = 0;
	virtual int GetSampleRate() = 0;
	// 0x4f2830
	virtual ~CAEStreamingDecoder();
	virtual int GetStreamID() = 0;

public:
	CAEDataStream *dataStream;
};

#endif
