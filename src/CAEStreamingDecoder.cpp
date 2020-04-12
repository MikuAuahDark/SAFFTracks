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
