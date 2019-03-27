#pragma once
#ifndef BITSET_H
#define BITSET_H
// 非常简易的BitSet
// 设计原则：not-copy-just-reference (浅拷贝)
using unit_t = uint8_t;
class BitSet {
	static constexpr size_t unit_size = 8 * sizeof(unit_t);
	static constexpr unit_t max_power = 1 << (unit_size - 1);

	unit_t* ref;
	int pos;
public:
	BitSet(unit_t* p = nullptr) :ref(p) {}

	BitSet& operator()(int pos) {
		this->pos = pos;
		return *this;
	}

	BitSet& operator[](int pos) {
		this->pos = pos;
		return *this;
	}

	operator bool() const {
		return *(ref + pos / unit_size) << (pos % unit_size) & max_power;
	}

	BitSet& operator=(int v) {
		if (v)
			*(ref + pos / unit_size) |= max_power >> (pos % unit_size);
		else
			*(ref + pos / unit_size) &= ~(max_power >> (pos % unit_size));
		return *this;
	}
};
#endif // !BITSET_H