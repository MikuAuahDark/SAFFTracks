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

#include "CAEStreamingDecoder.hpp"

extern uintptr_t gtasaBase;

CAEStreamingDecoder::CAEStreamingDecoder(CAEDataStream* dataStream)
{
	using Constructor = void(__thiscall*)(CAEStreamingDecoder*, CAEDataStream*);
	((Constructor) (gtasaBase + 0xf2810))(this, dataStream);
}

void CAEStreamingDecoder::operator delete(void* mem)
{
	((void(__thiscall*)(void*, int)) (gtasaBase + 0xf2860))(mem, 1);
}

CAEStreamingDecoder::~CAEStreamingDecoder()
{
	((void(__thiscall*)(CAEStreamingDecoder*)) (gtasaBase + 0xf2830))(this);
}

constexpr size_t size = sizeof(CAEStreamingDecoder);
static_assert(size == 0x8, "CAEStreamingDecoder size mismatch");
