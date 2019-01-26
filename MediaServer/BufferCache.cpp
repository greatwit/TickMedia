

#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>

#include "BufferCache.hpp"

#include "config.h"
#include "event.h"

BufferCache :: BufferCache()
{
	mBuffer = evbuffer_new();
}

BufferCache :: ~BufferCache()
{
	evbuffer_free( mBuffer );
	mBuffer = NULL;
}

int BufferCache :: append( const void * buffer, int len )
{
	len = len <= 0 ? strlen( (char*)buffer ) : len;

	return evbuffer_add( mBuffer, (void*)buffer, len );
}

int BufferCache :: append( const BufferCache * buffer )
{
	if( buffer->getSize() > 0 ) {
		return append( buffer->getBuffer(), buffer->getSize() );
	} else {
		return 0;
	}
}

void BufferCache :: erase( int len )
{
	evbuffer_drain( mBuffer, len );
}

void BufferCache :: reset()
{
	erase( getSize() );
}

const void * BufferCache :: getBuffer() const
{
	if( NULL != EVBUFFER_DATA( mBuffer ) ) {
		((char*)(EVBUFFER_DATA( mBuffer )))[ getSize() ] = '\0';
		return EVBUFFER_DATA( mBuffer );
	} else {
		return "";
	}
}

size_t BufferCache :: getSize() const
{
	return EVBUFFER_LENGTH( mBuffer );
}

char * BufferCache :: getLine()
{
	return evbuffer_readline( mBuffer );
}

int BufferCache :: take( char * buffer, int len )
{
	len = evbuffer_remove( mBuffer, buffer, len - 1);
	buffer[ len ] = '\0';

	return len;
}

BufferCache * BufferCache :: take()
{
	BufferCache * ret = new BufferCache();

	struct evbuffer * tmp = ret->mBuffer;
	ret->mBuffer = mBuffer;
	mBuffer = tmp;

	return ret;
}

const void * BufferCache :: find( const void * key, size_t len )
{
	//return (void*)evbuffer_find( mBuffer, (u_char*)key, len );

	struct evbuffer * buffer = mBuffer;
	u_char * what = (u_char*)key;

	size_t remain = buffer->off;
	u_char *search = buffer->buffer;
	u_char *p;

	while (remain >= len) {
		if ((p = (u_char*)memchr(search, *what, (remain - len) + 1)) == NULL)
			break;

		if (memcmp(p, what, len) == 0)
			return (p);

		search = p + 1;
		remain = buffer->off - (size_t)(search - buffer->buffer);
	}

	return (NULL);
}

