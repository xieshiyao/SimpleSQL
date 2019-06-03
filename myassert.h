#pragma once
#ifndef MY_ASSERT_H
#define MY_ASSERT_H
#include <cstdio>
#include <cstdlib>
#undef assert

#define ASSERT
inline void assert(bool condition, const char* msg)
{
#ifdef ASSERT
	if (!condition) {
		perror(msg);
		exit(EXIT_FAILURE);
	}
#endif // ASSERT
}
#endif // !MY_ASSERT_H
