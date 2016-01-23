#ifndef Z_MEMORY_STATE_H
#define Z_MEMORY_STATE_H

template<class T>
class MemoryState
{
public:
	void*   operator new(size_t sz, const char* filename, int line)
	{
		  void *p = malloc(sz);
		  return p;
	}
	void    operator delete(void *p, const char*, int)
	{ 
		free(p);
	}
};
#endif