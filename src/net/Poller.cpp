#include "..\common.h"
#include "../../include/net/Poller.h"
#include "../../include/net/Channel.h"
#include "../../include/net/EventLoop.h"


namespace muma
{


Poller::Poller(EventLoop* loop)
: _ownerLoop(loop)
{
	_wakeupEvent = ::WSACreateEvent();
	assert(_wakeupEvent != NULL);
	AddChannelEvent(_wakeupEvent, NULL, 0);
}

Poller::~Poller(void)
{
	clearChannels();
}

void Poller::poll(int timeoutMs, ChannelList* activeChannels)
{
	const DWORD EVENT_NUM = _events.size();
	DWORD ret = ::WaitForMultipleObjects(EVENT_NUM, &(*_events.begin()), FALSE, timeoutMs);
	if (ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + EVENT_NUM)
	{
		if (ret != WAIT_OBJECT_0) 
		{
			unsigned int index;
			int startIndex = 0;
			
			Channel* chan;
			DWORD waitEventNum;
			int fdtype;

			do
			{
				index = ret - WAIT_OBJECT_0 + startIndex;
				assert(_chanEvents[index].hEvent == _events[index]);

				chan = _chanEvents[index].chan;
				chan->clear_revent();

				fdtype = chan->fdtype();
				assert(fdtype == Channel::FD_OF_SOCKET || fdtype == Channel::FD_OF_TIMER);

				switch (fdtype)
				{
				case Channel::FD_OF_SOCKET:
					::WSAResetEvent(_events[index]);
					UpdateSockChanEvent(chan);
					break;
				case Channel::FD_OF_TIMER:
					UpdateTimerChanEvent(chan);
					break;
				}

				activeChannels->push_back(chan);

				if (++index >= EVENT_NUM)
					break;

				waitEventNum = EVENT_NUM - index;
				startIndex = index;

				ret = ::WaitForMultipleObjects(waitEventNum, &(*(_events.begin() + startIndex)), FALSE, 0);
			} while (ret >= WAIT_OBJECT_0 && ret < WAIT_OBJECT_0 + waitEventNum);
		}
		else
		{
			//index of 0 is wakeup event;
			::WSAResetEvent(_events[0]);   //wakeup event must be quit for sequence
			return;
		}
	}
}


int Poller::updateChannel(Channel* channel)
{
	assert(_ownerLoop->isInLoopThread());

	//find channel existance 
	ChannelEvent* channelEvent = NULL;
	unsigned int index = channel->indexForPoll();
	if (index < _chanEvents.size()) {
		channelEvent = &_chanEvents[index];
		
		assert(channelEvent->chan == channel);
	}

	Channel::FD_TYPE fdtype = channel->fdtype();
	assert(fdtype == Channel::FD_OF_SOCKET || fdtype == Channel::FD_OF_TIMER);
	if (NULL == channelEvent)
	{
		// a new one
		assert(_events.size() < MAXIMUM_WAIT_OBJECTS);

		HANDLE hEvent = NULL;
		int netEvents = 0;
		switch(fdtype)
		{
			case Channel::FD_OF_SOCKET:
				hEvent = ::WSACreateEvent();
				netEvents = GetSockNetEventMask(channel);
				break;
			case Channel::FD_OF_TIMER:
				hEvent = channel->handle();
				break;
		}
		AddChannelEvent(hEvent, channel, netEvents);
		if(fdtype == Channel::FD_OF_SOCKET)
		{
			::WSAEventSelect(channel->socketHandle(), hEvent, netEvents);
		}
	}
	else
	{
		// update existing one
		if(fdtype == Channel::FD_OF_SOCKET)
		{
			channelEvent->netEvents = GetSockNetEventMask(channel);
			::WSAEventSelect(channel->socketHandle(), channelEvent->hEvent, channelEvent->netEvents);
		}
	}


  return 0;
}


void Poller::removeChannel(Channel* channel)
{
	assert(_ownerLoop->isInLoopThread());

	//find channel existance 
	ChannelEvent* chanEvent = NULL;
	unsigned int index = channel->indexForPoll();
	if (index < _chanEvents.size()) 
	{
		chanEvent = &_chanEvents[index];
		assert(chanEvent->chan == channel);
	
		Channel::FD_TYPE fdtype = channel->fdtype();
		assert(fdtype == Channel::FD_OF_SOCKET || fdtype == Channel::FD_OF_TIMER);
		if (fdtype == Channel::FD_OF_SOCKET)
		{
			if (chanEvent->netEvents != 0)
				::WSAEventSelect(channel->socketHandle(), chanEvent->hEvent, 0);

			::CloseHandle(chanEvent->hEvent);
		}

		RemoveChannelEvent(index);
	}
}

void Poller::clearChannels()
{
	assert(_chanEvents.size() == _events.size());

	ChannelEventList::iterator it = _chanEvents.begin();
	for (; it != _chanEvents.end(); ++it)
	{
		Channel* chan = it->chan;
		if (chan == NULL || chan->fdtype() == Channel::FD_OF_SOCKET)
		{
			if (chan != NULL && it->netEvents != 0)
			{
				::WSAEventSelect(chan->socketHandle(), it->hEvent, 0);
			}
			::CloseHandle(it->hEvent);
		}
	}

	_events.clear();
	_chanEvents.clear();
}

void Poller::wakeup()
{
	::WSASetEvent(_wakeupEvent);
}

void Poller::AddChannelEvent(HANDLE event, Channel* chan, int netEvents) 
{
	_events.push_back(event);

	ChannelEvent chanEvent;
	chanEvent.hEvent = event;
	chanEvent.chan = chan;
	chanEvent.netEvents = netEvents;
	_chanEvents.push_back(chanEvent);

	if(chan != NULL)
		chan->setIndexForPoll(_chanEvents.size()-1);
}

void Poller::RemoveChannelEvent(unsigned int index)
{
	unsigned int totalsize = _chanEvents.size();
	if (index < totalsize)
	{
		Channel* chan = _chanEvents[index].chan;
		if (index != totalsize - 1)
		{
			std::iter_swap(_events.begin() + index, _events.end() - 1);
			std::iter_swap(_chanEvents.begin() + index, _chanEvents.end() - 1);

			_chanEvents[index].chan->setIndexForPoll(index);
		}

		_events.pop_back();
		_chanEvents.pop_back();

		chan->setIndexForPoll(EventLoop::kInvalidChannelIndex);
	}
}

void Poller::UpdateSockChanEvent(Channel* chan)
{
	assert(chan->fdtype() == Channel::FD_OF_SOCKET);

	WSANETWORKEVENTS netEvents;
	::WSAEnumNetworkEvents(chan->socketHandle(), NULL, &netEvents);

	if(netEvents.lNetworkEvents & FD_READ)
	{
		if(0 == netEvents.iErrorCode[FD_READ_BIT])
			chan->set_read_revent();
		else
			chan->set_error_revent();
	}

	if(netEvents.lNetworkEvents & FD_WRITE)
	{
		if(0 == netEvents.iErrorCode[FD_WRITE_BIT])
			chan->set_write_revent();
		else
			chan->set_error_revent();
	}

	if(netEvents.lNetworkEvents & FD_CLOSE)
		chan->set_close_revent();

	if(netEvents.lNetworkEvents & FD_ACCEPT)
	{
		if(0 == netEvents.iErrorCode[FD_ACCEPT_BIT])
			chan->set_accpet_revent();
		else
			chan->set_error_revent();
	}
}


void Poller::UpdateTimerChanEvent(Channel* chan)
{
	chan->set_read_revent();
}


int Poller::GetSockNetEventMask(Channel* chan)
{
	int netEvents = 0;

	if(chan->isEnableCloseEvent())
		netEvents |= FD_CLOSE;  //FD_CLOSE must be existed

	if(chan->isEnableReadEvent())
		netEvents |= FD_READ;

	if(chan->isEnableWriteEvent())
		netEvents |= FD_WRITE;

	if(chan->isEnableAcceptEvent())
		netEvents |= FD_ACCEPT;

	return netEvents;
}

}
