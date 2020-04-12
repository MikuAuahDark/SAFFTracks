#ifndef _SADEFAULTALLOCATOR_H_
#define _SADEFAULTALLOCATOR_H_

class SADefaultAllocator
{
public:
	// 0x82119a
	void *operator new(size_t size);
	// 0x8214bd
	void operator delete(void *mem);
};

#endif
