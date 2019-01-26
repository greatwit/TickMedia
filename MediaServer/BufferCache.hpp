

#ifndef __buffercache_hpp__
#define __buffercache_hpp__

#include <stdlib.h>

struct evbuffer;

class BufferCache {
public:
	BufferCache();
	~BufferCache();

	int append( const void * buffer, int len = 0 );
	int append( const BufferCache * buffer );
	void erase( int len );
	void reset();
	const void * getBuffer() const;
	size_t getSize() const;
	int take( char * buffer, int len );

	char * getLine();
	const void * find( const void * key, size_t len );

	BufferCache * take();

private:
	struct evbuffer * mBuffer;

	friend class SP_IOChannel;
};

#endif

