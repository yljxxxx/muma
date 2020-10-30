#pragma once

#include <Windows.h>
#include <stdint.h>

namespace muma
{

typedef UINT_PTR handle_t;

typedef struct DateTime
{
	uint32_t year;
	uint16_t month;
	uint16_t day;
	uint16_t hour;
	uint16_t minute;
	uint16_t second;
	uint16_t milsecs;
}DateTime;

typedef enum tcpProtocal
{
	TCP = 0,
	UDP,
}tcpProtocal;
}
