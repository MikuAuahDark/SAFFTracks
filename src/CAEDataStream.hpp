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

#ifndef _CAEDATASTREAM_H_
#define _CAEDATASTREAM_H_

#include <ObjIdl.h>

#include "SADefaultAllocator.hpp"

class CAEDataStream: public IStream, public SADefaultAllocator
{
public:
	// 0x4dc620
	CAEDataStream(int trackID, char *filename, DWORD startPosition, DWORD length, int encrypted);
	// 0x4dc490
	~CAEDataStream();

	// 0x4dc410
	HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** objout) override;
	// 0x4dc460
	ULONG STDMETHODCALLTYPE AddRef() override;
	// 0x4dc5b0
	ULONG STDMETHODCALLTYPE Release() override;
	// 0x4dc320
	HRESULT STDMETHODCALLTYPE Read(void* dest, ULONG size, ULONG* readed) override;
	// 0x4dc4d0
	HRESULT STDMETHODCALLTYPE Write(const void* src, ULONG size, ULONG* written) override;
	// 0x4dc340
	HRESULT STDMETHODCALLTYPE Seek(LARGE_INTEGER offset, DWORD whence, ULARGE_INTEGER *newOffset) override;
	// 0x4dc4e0
	HRESULT STDMETHODCALLTYPE SetSize(ULARGE_INTEGER newsize) override;
	// 0x4dc4f0
	HRESULT STDMETHODCALLTYPE CopyTo(IStream *target, ULARGE_INTEGER size, ULARGE_INTEGER *readed, ULARGE_INTEGER *written) override;
	// 0x4dc500
	HRESULT STDMETHODCALLTYPE Commit(DWORD flags) override;
	// 0x4dc510
	HRESULT STDMETHODCALLTYPE Revert() override;
	// 0x4dc520
	HRESULT STDMETHODCALLTYPE LockRegion(ULARGE_INTEGER offset, ULARGE_INTEGER size, DWORD type) override;
	// 0x4dc530
	HRESULT STDMETHODCALLTYPE UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType) override;
	// 0x4dc3a0
	HRESULT STDMETHODCALLTYPE Stat(STATSTG *statout, DWORD flags) override;
	// 0x4dc540
	HRESULT STDMETHODCALLTYPE Clone(IStream **target) override;

	// 0x4dc1c0
	size_t FillBuffer(void *dest, size_t size);
	// 0x4dc230
	unsigned long GetCurrentPosition();
	// 0x4dc2b0
	bool Initialise();

	// Ensure all types are in this exact order!
public:
	HANDLE fileHandle;
	char *filename;
	bool isOpen;
	unsigned long currentPosition;
	unsigned long startPosition;
	unsigned long length;
	unsigned int trackID;
	bool isEncrypted;
	ULONG refCount;
};

#endif
