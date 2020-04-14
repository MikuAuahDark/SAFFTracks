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

#include "CAEDataStream.hpp"

extern uintptr_t gtasaBase;

CAEDataStream::CAEDataStream(int trackID, char* filename, DWORD startPosition, DWORD length, int encrypted)
{
	using Constructor = void(__thiscall*)(CAEDataStream*, int, char*, DWORD, DWORD, int);
	((Constructor) (gtasaBase + 0xdc620))(this, trackID, filename, startPosition, length, encrypted);
}

CAEDataStream::~CAEDataStream()
{
	((void(__thiscall*)(CAEDataStream*)) (gtasaBase + 0xdc490))(this);
}

HRESULT __stdcall CAEDataStream::QueryInterface(REFIID riid, void** objout)
{
	using QIFunc = HRESULT(__stdcall *)(CAEDataStream*, REFIID, void**);
	return ((QIFunc) (gtasaBase + 0xdc410))(this, riid, objout);
}

ULONG __stdcall CAEDataStream::AddRef()
{
	using AddRefFunc = HRESULT(__stdcall *)(CAEDataStream*);
	return ((AddRefFunc) (gtasaBase + 0xdc460))(this);
}

ULONG __stdcall CAEDataStream::Release()
{
	using ReleaseFunc = HRESULT(__stdcall *)(CAEDataStream*);
	return ((ReleaseFunc) (gtasaBase + 0xdc5b0))(this);
}

HRESULT __stdcall CAEDataStream::Read(void* dest, ULONG size, ULONG* readed)
{
	using ReadFunc = HRESULT(__stdcall *)(CAEDataStream*, void*, ULONG, ULONG*);
	return ((ReadFunc) (gtasaBase + 0xdc320))(this, dest, size, readed);
}

HRESULT __stdcall CAEDataStream::Write(const void* src, ULONG size, ULONG* written)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CAEDataStream::Seek(LARGE_INTEGER offset, DWORD whence, ULARGE_INTEGER* newOffset)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CAEDataStream::SetSize(ULARGE_INTEGER newsize)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CAEDataStream::CopyTo(IStream* target, ULARGE_INTEGER size, ULARGE_INTEGER* readed, ULARGE_INTEGER* written)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CAEDataStream::Commit(DWORD flags)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CAEDataStream::Revert()
{
	return E_NOTIMPL;
}

HRESULT __stdcall CAEDataStream::LockRegion(ULARGE_INTEGER offset, ULARGE_INTEGER size, DWORD type)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CAEDataStream::UnlockRegion(ULARGE_INTEGER libOffset, ULARGE_INTEGER cb, DWORD dwLockType)
{
	return E_NOTIMPL;
}

HRESULT __stdcall CAEDataStream::Stat(STATSTG* statout, DWORD flags)
{
	using StatFunc = HRESULT(__stdcall *)(CAEDataStream*, STATSTG*, DWORD);
	return ((StatFunc) (gtasaBase + 0xdc3a0))(this, statout, flags);
}

HRESULT __stdcall CAEDataStream::Clone(IStream** target)
{
	return E_NOTIMPL;
}

int CAEDataStream::FillBuffer(void* dest, ULONG size)
{
	using FillFunc = int(__thiscall*)(CAEDataStream*, void*, ULONG);
	return ((FillFunc) (gtasaBase + 0xdc1c0))(this, dest, size);
}

LONG CAEDataStream::GetCurrentPosition()
{
	return ((LONG(__thiscall*)(CAEDataStream*)) (gtasaBase + 0xdc230))(this);
}

bool CAEDataStream::Initialise()
{
	return ((bool(__thiscall*)(CAEDataStream*)) (gtasaBase + 0xdc2b0))(this);
}

constexpr size_t size = sizeof(CAEDataStream);
static_assert(size == 0x28, "CAEDataStream size mismatch");
