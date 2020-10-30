// muma_test.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "../../include/base/Timestamp.h"
#include "../../include/base/ThreadPool.h"
#include "../../include/base/Timestamp.h"
#include <iostream>

void fun();

int main()
{
	muma::ThreadPool pool;

	int ret = pool.start(3);
	
	muma::Timestamp dt = muma::Timestamp::now();

	for(int i=0; i<50000; ++i)
	{
		pool.execute_task(fun);
	}

	//while (1)
	////{
	//	pool.execute_task(fun);
	//}


	
	
	while (1)
	{
		if (pool.queueSize() == 0)
		{
			muma::Timestamp dt2 = muma::Timestamp::now();
			int64_t count = dt2.milliSecondsSince1970() - dt.milliSecondsSince1970();
			std::cout << "---------------" << std::endl;
			std::cout << count << std::endl;
			break;
		}
		else
		{
			//Sleep(1);
		}
	//	Sleep(1);
	}

	char d;
	std::cin >> d;

	pool.stop();

	//char d;
	//std::cin >> d;

    return 0;
}


void fun()
{
	std::cout << "exe fun" << std::endl;
}

void fun1(int a)
{
	std::cout << "exe fun1" << std::endl;
	Sleep(1);
}

void fun2(int a, int b)
{
	std::cout << "exe fun2" << std::endl;
	Sleep(1000);
}
