#pragma once
#ifndef PRIMKEY_H
#include <cstddef>
class PrimKey {
	char* str;
	size_t size_;
public:
	PrimKey();
	size_t size();
};
#endif // !PRIMKEY_H