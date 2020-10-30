#include "..\common.h"
#include "../../include/net/Buffer.h"
#include "../../include/net/SocketsOps.h"


namespace muma
{

const char Buffer::kCRLF[] = "\r\n";

int32_t Buffer::readFd(SOCKET fd, int* savedErrno)
{
	char extrabuf[65536];
	WSABUF bufs[2];
	const size_t writable = writableBytes();

	bufs[0].buf = begin()+_writerIndex;
	bufs[0].len = writable;
	bufs[1].buf = extrabuf;
	bufs[1].len = sizeof(extrabuf);
	
	int32_t recvdNum = sockets::readv(fd, bufs, 2);
	if (recvdNum > 0) 
	{
		if (recvdNum <= writable)
		{
			_writerIndex += recvdNum;
		}
		else
		{
			_writerIndex = _buffer.size();
			append(extrabuf, recvdNum - writable);
		}

		return recvdNum;
	}
	else if (recvdNum == 0) 
	{
		return 0;
	}
	else if (recvdNum < 0)
	{
		if (savedErrno != NULL)
			*savedErrno = sockets::getLastError();

		return -1;
	}
}


}
