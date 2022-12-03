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

#include "CAELAVDecoder.hpp"

extern "C"
{
#include "libavutil/avutil.h"
}

static int readFromDataStream(void *opaque, uint8_t *buf, int size)
{
	CAEDataStream *dataStream = (CAEDataStream *) opaque;
	return dataStream->FillBuffer(buf, size);
}

static int64_t seekFromDataStream(void *opaque, int64_t offset, int whence)
{
	CAEDataStream *dataStream = (CAEDataStream *) opaque;

	if (whence == AVSEEK_SIZE)
		return dataStream->length;
	else
	{
		ULARGE_INTEGER dummy;
		LARGE_INTEGER off;
		off.QuadPart = offset;

		if (FAILED(dataStream->Seek(off, whence, &dummy)))
			return -1;

		return dataStream->GetCurrentPosition();
	}
}

inline AVChannelLayout makeNativeChannelLayout(int nchannels, uint64_t mask)
{
	AVChannelLayout layout {};
	layout.order = AV_CHANNEL_ORDER_NATIVE;
	layout.nb_channels = nchannels;
	layout.opaque = nullptr;
	layout.u.mask = mask;
	return layout;
}

#ifdef AV_CHANNEL_LAYOUT_MASK
#undef AV_CHANNEL_LAYOUT_MASK
#define AV_CHANNEL_LAYOUT_MASK makeNativeChannelLayout
#endif

CAELAVDecoder::CAELAVDecoder(CAEDataStream *dataStream)
: CAEStreamingDecoder(dataStream)
, signature("LAV")
, formatContext(nullptr)
, codecContext(nullptr)
, ioContext(nullptr)
, packet(nullptr)
, frame(nullptr)
, resampler(nullptr)
, targetIndex((unsigned int) -1)
, sampleRate(0)
, init0(false)
, init(false)
{
	unsigned char *buffer = (unsigned char *) av_malloc(4096);
	if (buffer == nullptr) return;

	ioContext = avio_alloc_context(buffer, 4096, 0, dataStream, readFromDataStream, nullptr, seekFromDataStream);
	if (ioContext == nullptr) return;

	formatContext = avformat_alloc_context();
	if (formatContext == nullptr) return;

	frame = av_frame_alloc();
	if (frame == nullptr) return;

	packet = av_packet_alloc();
	if (packet == nullptr) return;

	// Setup metadata
	memset(&metadata, 0, sizeof(Metadata));
	metadata.filename = strrchr(dataStream->filename, '\\');
	if (metadata.filename == nullptr)
		metadata.filename = dataStream->filename;
	else
		metadata.filename++;

	init0 = true;
}

CAELAVDecoder::~CAELAVDecoder()
{
	if (resampler)
		swr_free(&resampler);
	if (frame)
		av_frame_free(&frame);
	if (packet)
		av_packet_free(&packet);
	if (codecContext)
		avcodec_free_context(&codecContext);
	if (formatContext)
		avformat_close_input(&formatContext);
	if (ioContext)
	{
		av_freep(&ioContext->buffer);
		avio_context_free(&ioContext);
	}
	
	// No need to clear metadata, the memory is owned by libavformat
}

bool CAELAVDecoder::Initialise()
{
	if (!init0) return false;
	if (init) return true;

	formatContext->pb = ioContext;
	formatContext->flags |= AVFMT_FLAG_CUSTOM_IO;

	// Open input from CAEDataStream
	if (avformat_open_input(&formatContext, dataStream->filename, nullptr, nullptr) < 0)
		return false;

	// Retrieve stream info
	if (avformat_find_stream_info(formatContext, nullptr) < 0)
		return false;

	// Retrieve metadata
	AVDictionaryEntry *tag = nullptr;
	while ((tag = av_dict_get(formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX)))
	{
		if (metadata.title == nullptr && strcmpi(tag->key, "title") == 0)
			metadata.title = tag->value;
		else if (metadata.artist == nullptr && strcmpi(tag->key, "artist") == 0)
			metadata.artist = tag->value;
		else if (metadata.album == nullptr && strcmpi(tag->key, "album") == 0)
			metadata.album = tag->value;
		else if (metadata.genre == nullptr && strcmpi(tag->key, "genre") == 0)
			metadata.genre = tag->value;
		else if (metadata.title && metadata.artist && metadata.album && metadata.genre)
			break;
	}

	// Find audio stream
	for (unsigned int i = 0; targetIndex == (unsigned int) -1 && i < formatContext->nb_streams; i++)
	{
		if (formatContext->streams[i]->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO)
			targetIndex = i;
		else
			formatContext->streams[i]->discard = AVDiscard::AVDISCARD_ALL;
	}

	if (targetIndex == (unsigned int) -1)
		// No audio stream, probably video-only file lol
		return false;

	// Find decoder for this codec
	AVCodecParameters *param = formatContext->streams[targetIndex]->codecpar;
	const AVCodec *codec = avcodec_find_decoder(param->codec_id);
	if (codec == nullptr)
		return false;

	// Create new codec context
	codecContext = avcodec_alloc_context3(codec);
	if (codecContext == nullptr)
		return false;

	// Copy parameters
	if (avcodec_parameters_to_context(codecContext, param) < 0)
		return false;

	// Open codec context
	if (avcodec_open2(codecContext, codec, nullptr) < 0)
		return false;

	// Read single packet
	if (!ReadFrame(frame))
		return false;

	// Let's obey GTASA checks. If duration < 7s then false
	if (GetStreamLength() < 7.0)
		return false;

	AVChannelLayout stereo = AV_CHANNEL_LAYOUT_STEREO;
	sampleRate = frame->sample_rate > 48000 ? 48000 : frame->sample_rate;
	// Setup swr
	if (swr_alloc_set_opts2(&resampler,
		// output options
		&stereo, AV_SAMPLE_FMT_S16, sampleRate,
		// input options
		&frame->ch_layout, (AVSampleFormat) frame->format, frame->sample_rate,
		// dunno
		0, nullptr
	) != 0)
		return false;

	// Initialize resampler
	if (swr_init(resampler) < 0)
		return false;

	return init = true;
}

size_t CAELAVDecoder::FillBuffer(void *dest, size_t size)
{
	// We always assume s16 and 2 channels, hence division by 4
	int sampleCount = size >> 2;
	int amountOfDecoded = 0;
	int32_t *buf = (int32_t *) dest;

	// Fill the buffers until full
	while (sampleCount > 0)
	{
		if (!ReadFrame(frame))
			break;

		uint8_t *outbuf[2] = {(uint8_t *) buf, nullptr};
		// Resample
		int decoded = swr_convert(resampler, outbuf, sampleCount, (const uint8_t **) &frame->data[0], frame->nb_samples);

		// Update
		amountOfDecoded += decoded;
		sampleCount -= decoded;
		buf += decoded;
	}

	return amountOfDecoded * 4;
}

long CAELAVDecoder::GetStreamLengthMs()
{
	if (init)
		return (long) (GetStreamLength() * 1000.0);
	else
		return -1;
}

long CAELAVDecoder::GetStreamPlayTimeMs()
{
	if (!init)
		return -1;

	AVRational &timeBase = formatContext->streams[targetIndex]->time_base;
	if (timeBase.den == 0)
		return -1;

	return (long) (frame->pts * timeBase.num * 1000 / timeBase.den);
}

void CAELAVDecoder::SetCursor(unsigned long pos)
{
	AVRational &timeBase = formatContext->streams[targetIndex]->time_base;
	if (timeBase.den == 0)
		// Cannot divide by 0
		return;

	// Convert to stream-specific timestamp
	int64_t ts = int64_t(pos) * timeBase.den / (timeBase.num * 1000ULL);
	// Flush decode buffers
	avcodec_flush_buffers(codecContext);
	
	if (resampler)
	{
		// Flush resampler
		int outSize = swr_get_out_samples(resampler, 0);
		if (outSize > 0)
		{
			// s16, 2 channels, lshift by 2
			uint8_t *buffers[2] = {(uint8_t *) av_malloc(outSize << 2), nullptr};
			swr_convert(resampler, buffers, outSize, nullptr, 0);
			av_free(buffers[0]);
		}
	}

	// Seek
	av_seek_frame(formatContext, targetIndex, ts, AVSEEK_FLAG_BACKWARD);
}

int CAELAVDecoder::GetSampleRate()
{
	return sampleRate;
}

int CAELAVDecoder::GetStreamID()
{
	return dataStream->trackID;
}

bool CAELAVDecoder::ReadPacket()
{
	if (packet->buf != nullptr)
		av_packet_unref(packet);

	do
	{
		if (av_read_frame(formatContext, packet) < 0)
			return false;
	}
	while (packet->stream_index != targetIndex);

	return true;
}

bool CAELAVDecoder::ReadFrame(AVFrame *frame)
{
	// FFmpeg 4.0's send/receive style API
	if (frame->buf[0] != nullptr)
		av_frame_unref(frame);

	while (true)
	{
		if (!ReadPacket())
			return false;

		int r = avcodec_send_packet(codecContext, packet);
		if (r < 0)
			return false;

		while (r >= 0)
		{
			r = avcodec_receive_frame(codecContext, frame);
			if (r == AVERROR(EAGAIN))
				break;
			else if (r < 0)
				return false;

			return true;
		}
	}
}

double CAELAVDecoder::GetStreamLength()
{
	AVStream *temp = formatContext->streams[targetIndex];
	AVRational timeBase;
	int64_t duration;

	if (temp->duration == AV_NOPTS_VALUE)
	{
		if (formatContext->duration == AV_NOPTS_VALUE)
			// TODO: Enumerate all frames?
			return -1;
		else
		{
			timeBase = {1, AV_TIME_BASE};
			duration = formatContext->duration;
		}
	}
	else
	{
		timeBase = temp->time_base;
		duration = temp->duration;
	}

	// Cannot divide by 0
	if (timeBase.den == 0)
		return -1;
	else
		return 1.0 * duration * timeBase.num / (timeBase.den * 1.0);
}
