#include "..\common.h"
#include "../../include/net/EventLoop.h"

namespace muma
{

	EventLoop::EventLoop()
		: _quit(false),
		_looping(false),
		_threadId(::GetCurrentThreadId()), 
		_poller(new Poller(this)),
		_timerQueue(new TimerQueue(this))
	{
	}

	EventLoop::~EventLoop()
	{
		assert(isInLoopThread());
		DeleteAllChannel(NULL);
	}

	Channel* EventLoop::NewChannel(SOCKET socket, HANDLE handle, Channel::FD_TYPE fdtype)
	{
		Channel* chan = new Channel(this, socket, handle, fdtype);

		if (isInLoopThread())
		{
			AddChannel(chan);
		}
		else
		{
			queueInLoop(std::bind(&EventLoop::AddChannel, this, chan));
		}

		return chan;
	}

	void EventLoop::DeleteChannel(Channel* chan, const Functor& cb)
	{
		assert(chan->ownerLoop() == this);

		if (isInLoopThread())
		{
			RemoveChannel(chan, cb);
		}
		else
		{
			queueInLoop(std::bind(&EventLoop::RemoveChannel, this, chan, cb));
		}
	}

	void EventLoop::DeleteAllChannel(const Functor& cb)
	{
		if (isInLoopThread())
		{
			RemoveAllChannel(cb);
		}
		else
		{
			queueInLoop(std::bind(&EventLoop::RemoveAllChannel, this, cb));
		}

	}


	void EventLoop::loop()
	{
		assert(!_looping);
		assert(isInLoopThread());

		_quit = false;
		_looping = true;
		std::vector<Channel*> activeChannels;

		while(!_quit)
		{
			activeChannels.clear();
			_poller->poll(g_pollTimeountMs, &activeChannels);

			for (std::vector<Channel*>::iterator iter = activeChannels.begin();
				iter != activeChannels.end(); ++iter)
			{
				(*iter)->handleEvent();
			}

			doPendingFunctors();
		}

		_looping = false;
	}

	void EventLoop::quit()
	{
		_quit = true;

		if(!isInLoopThread())
			_poller->wakeup();
	}

	void EventLoop::queueInLoop(const Functor& cb)
	{
		_mutex.lock();
		_pendingFunctors.push_back(cb);
		_mutex.unlock();

		_poller->wakeup();
	}

	void EventLoop::runInLoop(const Functor& cb)
	{
		if (!isInLoopThread())
		{
			cb();
		}
		else
		{
			queueInLoop(cb);
		}
	}

	TimerId EventLoop::runAt(const Timestamp& time, const TimerCallback& cb)
	{
		return _timerQueue->addTimer(cb, time, 0);
	}

	TimerId EventLoop::runAfter(int64_t delay, const TimerCallback& cb)
	{
		Timestamp time(addTime(Timestamp::now(), delay));
		return runAt(time, cb);
	}

	TimerId EventLoop::runEvery(int64_t interval, const TimerCallback& cb)
	{
		Timestamp time(addTime(Timestamp::now(), interval));
		return _timerQueue->addTimer(cb, time, interval);
	}

	void EventLoop::cancel(TimerId timerId)
	{
		return _timerQueue->cancel(timerId);
	}

	void EventLoop::cancel(TimerId timerId, const Functor& cb)
	{
		return _timerQueue->cancel(timerId, cb);
	}

	int EventLoop::updateForChannel(Channel* channel)
	{
		 assert(channel->ownerLoop() == this);
		 assert(isInLoopThread());

		return _poller->updateChannel(channel);
	}

	void EventLoop::removeForChannel(Channel* channel)
	{
		assert(channel->ownerLoop() == this);
		assert(isInLoopThread());

		_poller->removeChannel(channel);
	}

	void EventLoop::doPendingFunctors()
	{
		std::vector<Functor> functors;

		_mutex.lock();
		functors.swap(_pendingFunctors);
		_mutex.unlock();

		for (size_t i = 0; i < functors.size(); ++i)
		{
			functors[i]();
		}
	}

	void EventLoop::AddChannel(Channel* chan)
	{
		_channels.push_back(chan);
		chan->setIndexForLoop(_channels.size() - 1);
	}

	void EventLoop::RemoveChannel(Channel* chan, const Functor& cb)
	{
		unsigned int index = chan->indexForLoop();
		unsigned int totalsize = _channels.size();
		if (totalsize > 0 && index < totalsize)
		{
			chan->remove();

			if (index != totalsize - 1)
			{
				std::iter_swap(_channels.begin() + index, _channels.end() - 1);
				_channels[index]->setIndexForLoop(index);
			}

			_channels.pop_back();
			chan->setIndexForLoop(EventLoop::kInvalidChannelIndex);

			delete chan;

			if(cb)
				cb();
		}
	}

	void EventLoop::RemoveAllChannel(const Functor& cb)
	{
		_poller->clearChannels();

		std::vector<Channel*>::iterator iter = _channels.begin();
		for (; iter != _channels.end(); ++iter)
		{
			delete (*iter);
		}

		_channels.clear();

		if (cb)
			cb();

	}
}