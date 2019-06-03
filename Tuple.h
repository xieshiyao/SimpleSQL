#pragma once
#ifndef TUPLE_H
#define TUPLE_H
#include "BitSet.h"
#include "TableMeta.h"

class StrWrapper {
	const char* data;
public:
	StrWrapper(const char* data) :data(data) {}
	StrWrapper(const std::string& str) :data(str.c_str()) {}
	const char* operator&() {
		return data;
	}
};

// not-copy-just-reference
class Tuple {
	//template <int n>
	friend class TuplePool;

	uint8_t* data;			// 数据
	const TableMeta* meta;	// 所属的表的元数据
	BitSet info;			// 信息位，函数 isEmpty()/setEmpty()/isNull()/setNull() 均依赖于此

public:
	Tuple(uint8_t* data, const TableMeta& meta);
	~Tuple() = default;
	operator bool() const;
	const char* get(int pos) const;
	template <typename T> T get(int pos) const;
	template <typename T> void set(int pos, T t);
	bool isNull(int pos);
	void setNull(int pos, bool isNull);
	bool isEmpty();
	void setEmpty(bool isEmpty);
};

inline Tuple::Tuple(uint8_t* data, const TableMeta& meta)
	:data(data), meta(&meta),
	info(data + (meta.tupleSize - (meta.fields.size() + 8) / 8)) {}

inline Tuple::operator bool() const
{
	return data;
}

inline const char* Tuple::get(int pos) const
{
	return (const char*)(data + meta->fields[pos].offset);
}

template<typename T>
inline T Tuple::get(int pos) const
{
	return *(T*)(data + meta->fields[pos].offset);
}

template<typename T>
inline void Tuple::set(int pos, T t)
{
	const Field& which = meta->fields[pos];
	memcpy(data + which.offset, &t, which.size);
}

inline bool Tuple::isNull(int pos)
{
	return info[pos];
}

inline void Tuple::setNull(int pos, bool isNull)
{
	info[pos] = isNull;
}

inline bool Tuple::isEmpty()
{
	return info[meta->fields.size()];
}

inline void Tuple::setEmpty(bool isEmpty)
{
	info[meta->fields.size()] = isEmpty;
}
#endif // !TUPLE_H
