#pragma once

#include <functional>
#include <memory>
#include "../base/Mutex.h"
#include "../Types.h"
#include "../base/Object.h"

namespace muma
{
	class EventLoop;
	class Channel : Object
	{
	public:
		typedef std::function<void()> EventCallback;
		typedef enum
		{
			FD_OF_SOCKET = 1,
			FD_OF_TIMER,
		}FD_TYPE;

		void setReadCallback(const EventCallback& cb);
		void setWriteCallback(const EventCallback& cb);
		void setCloseCallback(const EventCallback& cb);
		void setAcceptCallback(const EventCallback& cb);
		void setErrorCallback(const EventCallback& cb);

		void enableReading();
		void enableWriting();
		void enableAccepting();
		void enableClosing();
		
		void disableReading();
		void disableWriting();
		void disableAccepting();
		void disableClosing();
		void disableAll();

		bool isNoneEvent() const { 
			//assert(_loop->isInLoopThread());
			return _events == NONE_EVENT; 
		}

		bool isEnableReadEvent() const { 
			//assert(_loop->isInLoopThread());
			return READ_EVENT == (_events & READ_EVENT); 
		}

		bool isEnableWriteEvent() const { 
			//assert(_loop->isInLoopThread());
			return WRITE_EVENT == (_events & WRITE_EVENT);
		}

		bool isEnableAcceptEvent() const { 
			//assert(_loop->isInLoopThread());
			return ACCEPT_EVENT == (_events & ACCEPT_EVENT); 
		}

		bool isEnableCloseEvent() const { 
			//assert(_loop->isInLoopThread());
			return CLOSE_EVENT == (_events & CLOSE_EVENT); 
		}

		void remove();

		EventLoop* ownerLoop() { return _loop; }
		HANDLE handle() const { return _handle; }
		SOCKET socketHandle() const { return _socket; }
		FD_TYPE fdtype(){ return _fdtype; }

	private:
		friend class Poller;
		friend class EventLoop;

		Channel();
		Channel(EventLoop* loop, SOCKET socket, HANDLE handle, FD_TYPE fdtype);
		~Channel();

		void handleEvent();

		void clear_revent() { _revents = NONE_EVENT; }
		void set_read_revent() { _revents |= READ_EVENT; }
		void set_write_revent() { _revents |= WRITE_EVENT; }
		void set_close_revent() { _revents |= CLOSE_EVENT; }
		void set_accpet_revent() { _revents |= ACCEPT_EVENT; }
		void set_error_revent() { _revents |= ERROR_EVENT; }

		unsigned int indexForPoll() { return _indexForPoll; }
		void setIndexForPoll(unsigned int  index) { _indexForPoll = index; }

		unsigned int indexForLoop() { return _indexForLoop; }
		void setIndexForLoop(unsigned int  index) { _indexForLoop = index; }

	private:
		//void handleEventFunc();

		void setReadCallbackFunc(const EventCallback& cb);
		void setWriteCallbackFunc(const EventCallback& cb);
		void setCloseCallbackFunc(const EventCallback& cb);
		void setAcceptCallbackFunc(const EventCallback& cb);
		void setErrorCallbackFunc(const EventCallback& cb);

		void enableReadingFunc();
		void enableWritingFunc();
		void enableAcceptingFunc();
		void enableClosingFunc();

		void disableReadingFunc();
		void disableWritingFunc();
		void disableAcceptingFunc();
		void disableClosingFunc();
		void disableAllFunc();

		void removeFunc();

	private:
		EventLoop* _loop;
		HANDLE  _handle;
		SOCKET _socket;
		FD_TYPE _fdtype; 

		int _events;
		int _revents;
		unsigned int _indexForPoll;
		unsigned int _indexForLoop;

		EventCallback _readCallback;
		EventCallback _writeCallback;
		EventCallback _closeCallback;
		EventCallback _acceptCallback;
		EventCallback _errorCallback;

		typedef enum EventType
		{
			NONE_EVENT = 0,
			READ_EVENT = 0x01,
			WRITE_EVENT = 0x02,
			CLOSE_EVENT = 0x04,
			ACCEPT_EVENT = 0x08,

			ERROR_EVENT = 0x100,
		}EventType;
	};

}
