#pragma once

#include "../../include/net/Channel.h"

#include <vector>
#include <map>
//#include <memory>

namespace muma
{

class EventLoop;

class Poller
{
	typedef std::vector<Channel*> ChannelList;

public:
	Poller(EventLoop* loop);
	~Poller(void);

	int updateChannel(Channel* channel);
	void removeChannel(Channel* channel);
	void clearChannels();

	void poll(int timeoutMs, ChannelList* activeChannels);
	void wakeup();

	int GetAvailableChannelCapcity(){
		return kMaximumPollChannelNum - _events.size();
	}

public:
	const static int MAX_CHANNEL_CAPCITY = MAXIMUM_WAIT_OBJECTS-1;

private:
	void AddChannelEvent(HANDLE event, Channel* chan, int netEvents);
	void RemoveChannelEvent(unsigned int index);

	void UpdateSockChanEvent(Channel* chan);
	void UpdateTimerChanEvent(Channel* chan);

	int GetSockNetEventMask(Channel* chan);

private:
	typedef struct ChannelEvent
	{
		HANDLE hEvent;
		Channel* chan;
		int netEvents;
	}ChannelEvent;

	typedef std::vector<HANDLE> EventList;
	typedef std::vector<ChannelEvent> ChannelEventList;

	EventLoop* _ownerLoop;
	HANDLE _wakeupEvent;

	EventList	_events;
	ChannelEventList _chanEvents;

	static const int kMaximumPollChannelNum = MAXIMUM_WAIT_OBJECTS;
};


}
