#pragma once

#include "InetAddress.h"
#include <memory>
#include <functional>
#include "TimerId.h"


namespace muma
{
	class Channel;
	class EventLoop;

	class TcpConnector
	{
	public:
		typedef std::function<void(int sockfd)> NewConnectionCallback;
		typedef std::function<void(int errorcode)> ConnectExceptionMsgCallback;

		TcpConnector(EventLoop* loop, const InetAddress& serverAddr
			, bool isretry, int retryIntervalMs, int timeout, const NewConnectionCallback& newconnCb
			, const ConnectExceptionMsgCallback& connExcptCb);
		~TcpConnector();

		void setNewConnectionCallback(const NewConnectionCallback& cb)
		{
			newConnectionCallback_ = cb;
		}

		void setConnectExceptionCallback(const ConnectExceptionMsgCallback& cb)
		{
			connectExcptMsgCallback_ = cb;
		}

		void start();  // can be called in any thread
		void stop();  // can be called in any thread
		void restart();

		const InetAddress& serverAddress() const { return serverAddr_; }

	public:
		enum ConnectionErrorcode {
			connect_error_unknown = -1,
			connect_failed = 1,
			connect_connecting,
			connect_connected,
			connect_failed_and_willRetry,
			connect_failed_timeout,
		};
	private:
		enum States { kDisconnected, kConnecting, kConnected };

		void setState(States s) { state_ = s; }
		void startInLoop();
		void stopInLoop();
		void restartInLoop();

		void connect();
		void handleWrite();
		void handleError();
		void retry(int sockfd);
		void retryInLoop();
		SOCKET removeAndResetChannel();
		void resetChannel();
		void checkTimeoutInLoop();
		void cancelCheckTimeoutInLoop();

		EventLoop* loop_;
		InetAddress serverAddr_;
		States state_;  // FIXME: use atomic variable
		
		Channel* channel_;
		bool isSocketTransfer_;

		NewConnectionCallback newConnectionCallback_;
		ConnectExceptionMsgCallback connectExcptMsgCallback_;

		bool isretry;
		int retryDelayMs_;
		int timeout_;
		TimerId timeid_;
		TimerId timeidForTimeout_;
	};
}
