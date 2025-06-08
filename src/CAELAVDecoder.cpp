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

#define NOMINMAX

#include <algorithm>

#include "CAELAVDecoder.hpp"

extern "C"
{
#include "libavutil/avutil.h"
}

static int readFromDataStream(void *opaque, uint8_t *buf, int size)
{
	CAEDataStream *dataStream = (CAEDataStream *) opaque;
	int result = (int) dataStream->FillBuffer(buf, size);
	if (result == 0 && size != 0)
		return AVERROR_EOF;
	return result;
}

static int64_t seekFromDataStream(void *opaque, int64_t offset, int whence)
{
	CAEDataStream *dataStream = (CAEDataStream *) opaque;
	whence = whence & (~int(AVSEEK_FORCE));

	if (whence | AVSEEK_SIZE)
		return dataStream->length;
	else
	{
		ULARGE_INTEGER dummy;
		LARGE_INTEGER off = {};
		off.QuadPart = offset;

		if (FAILED(dataStream->Seek(off, whence, &dummy)))
			return -1;

		return dataStream->GetCurrentPosition();
	}
}

CAELAVDecoder::CAELAVDecoder(CAEDataStream *dataStream)
: CAEStreamingDecoder(dataStream)
, signature("LAV")
, formatContext(nullptr)
, codecContext(nullptr)
, ioContext(nullptr)
, packet(nullptr)
, frame(nullptr)
, resampler(nullptr)
, pos(0)
, targetIndex(-1)
, sampleRate(0)
, init0(false)
, init(false)
, frameConsumed(false)
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

	// Find first audio stream
	for (int i = 0; i < (int) formatContext->nb_streams; i++)
	{
		if (formatContext->streams[i]->codecpar->codec_type == AVMediaType::AVMEDIA_TYPE_AUDIO && targetIndex == -1)
			targetIndex = i;
		else
			// Discard the rest
			formatContext->streams[i]->discard = AVDiscard::AVDISCARD_ALL;
	}

	if (targetIndex == -1)
		// No audio stream, probably video-only file
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
	if (ReadFrame() != ReadStatus::OK)
		return false;

	// Let's obey GTASA checks. If duration < 7s then false
	if (GetStreamLength() < 7.0)
		return false;

	constexpr AVChannelLayout stereo = AV_CHANNEL_LAYOUT_STEREO;
	sampleRate = std::min(frame->sample_rate, 48000);
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
	// We always assume s16 and 2 channels, so divide by 4
	constexpr int sampleFrame = sizeof(int16_t) * 2;
	int sampleCount = size / sampleFrame;
	size_t amountOfDecoded = 0;
	uint8_t *buf = (uint8_t *) dest;

	// Fill the buffers until full
	while (sampleCount > 0)
	{
		uint8_t **frameData = nullptr;
		int frameSampleCount = 0;

		int remaining = swr_get_out_samples(resampler, 0);
		if (remaining < sampleCount)
		{
			if (ReadFrame() == ReadStatus::OK)
			{
				frameData = frame->data;
				frameSampleCount = frame->nb_samples;
			}
			else if (remaining == 0)
			{
				// No more samples
				break;
			}
		}

		uint8_t *outbuf[2] = {buf, nullptr};
		int decoded = std::max(swr_convert(resampler, outbuf, sampleCount, frameData, frameSampleCount), 0);
		frameConsumed = frameData != nullptr;

		// Update
		amountOfDecoded += decoded;
		sampleCount -= decoded;
		buf += decoded * sampleFrame;
	}

	return amountOfDecoded * sampleFrame;
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

	return (long) (std::max(pos, 0.0) * 1000.0);
}

void CAELAVDecoder::SetCursor(unsigned long pos)
{
	constexpr AVRational timeBase = {1, AV_TIME_BASE};

	// Convert to stream-specific timestamp
	int64_t ts = int64_t(pos) * timeBase.den / (timeBase.num * 1000ULL);

	// Flush decode buffers
	avformat_flush(formatContext);
	avcodec_flush_buffers(codecContext);
	
	if (resampler)
	{
		// Flush resampler
		swr_convert(resampler, nullptr, 0, nullptr, 0);
		int outSize = swr_get_out_samples(resampler, 0);
		if (outSize > 0)
			swr_drop_output(resampler, outSize);
	}

	// Seek
	avformat_seek_file(formatContext, -1, std::numeric_limits<int64_t>::min(), ts, std::numeric_limits<int64_t>::max(), 0);
}

int CAELAVDecoder::GetSampleRate()
{
	return sampleRate;
}

int CAELAVDecoder::GetStreamID()
{
	return dataStream->trackID;
}

CAELAVDecoder::ReadStatus CAELAVDecoder::ReadPacket()
{
	do
	{
		if (packet->buf != nullptr)
			av_packet_unref(packet);

		int r = av_read_frame(formatContext, packet);
		if (r >= 0)
			return ReadStatus::OK;
		else if (r == AVERROR_EOF)
			return ReadStatus::End;
		else
			return ReadStatus::Error;
	}
	while (packet->stream_index != targetIndex);

	return ReadStatus::Error;
}

CAELAVDecoder::ReadStatus CAELAVDecoder::ReadFrame()
{
	if (frame->buf[0])
	{
		if (!frameConsumed)
			return ReadStatus::OK;

		av_frame_unref(frame);
	}

	frameConsumed = false;

	while (true)
	{
		// Pull frame
		int r = avcodec_receive_frame(codecContext, frame);
		if (r == AVERROR_EOF)
			return ReadStatus::End;
		else if (r == AVERROR(EAGAIN))
		{
			// No frame to be pulled. Send some packet
			AVPacket *targetPacket = nullptr;
			ReadStatus rs = ReadPacket();
			if (rs == ReadStatus::Error)
				return ReadStatus::Error;
			else if (rs == ReadStatus::OK)
				targetPacket = packet;

			int r = avcodec_send_packet(codecContext, targetPacket);
			if (r == AVERROR_EOF)
				return ReadStatus::End;
			else if (r < 0)
				return ReadStatus::Error;
		}
		else if (r < 0)
			return ReadStatus::Error;
		else
		{
			// Successfully got frame. Compute position
			const AVRational &timeBase = formatContext->streams[targetIndex]->time_base;
			if (timeBase.den != 0)
				pos = (1.0 * frame->pts * timeBase.num / (timeBase.den * 1.0));

			return ReadStatus::OK;
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
