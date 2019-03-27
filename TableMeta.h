#pragma once
#ifndef TABLEMETA_H
#define TABLEMETA_H
#include <cstdint>
#include <vector>
#include <queue>
#include <unordered_map>
#include <fstream>
#include "Field.h"

// type alias
using HoleQueue = std::queue<int>;

class TableMeta {
	uint16_t tupleSize = 0;		// 每一个元组的大小
	std::vector<Field> fields;	// 模式，字段集
	HoleQueue holeQ;			// 空洞队列，存储的是元组在文件中的“下标”

	std::unordered_map<std::string, int> map;	// 将字段名称映射为伪指针

	friend class Table;
	friend class Tuple;
	//template <int n> 
	friend class TuplePool;
	friend std::ofstream& operator<<(std::ofstream& fout, TableMeta& meta);
	friend std::ifstream& operator>>(std::ifstream& fin, TableMeta& meta);
public:
	TableMeta() = default;
	~TableMeta() = default;
	void addField(const std::string& name, uint8_t type, uint8_t info = 0, uint16_t size = 0);
	const std::vector<Field>& getFields() const;
	void init();
	void save(const std::string& fileName);
	void load(const std::string& fileName);
	int posOf(const std::string& fieldName) const;
	const Field& of(const std::string& fieldName) const;
	bool exist(const std::string& fieldName) const;
};

// 添加字段
inline void TableMeta::addField(const std::string& name, uint8_t type, uint8_t info, uint16_t size)
{
	// 处理 该字段的size
	if (type == Field::STRING) {
		// 若为字符串，则需要多一个字节存放 \0 (即调用该函数时，不用先自行考虑 \0 )
		size++;
	}
	else size = Field::sizeOf[type];

	// 处理 offset
	uint16_t offset = tupleSize;

	// 处理 元组大小
	tupleSize += size;

	// 处理 map
	map[name] = fields.size();

	// 添加字段信息
	fields.emplace_back(name, type, info, size, offset);
}

// 获得表的模式，即字段集
inline const std::vector<Field>& TableMeta::getFields() const
{
	return fields;
}

// 完成最终的初始化
inline void TableMeta::init()
{
	// 完成元组大小的最后修改，要加上一些表示信息的bit位
	tupleSize += (fields.size() + 8) / 8;
}

// called in save()
inline std::ofstream& operator<<(std::ofstream& fout, TableMeta& meta)
{
	// 元组大小
	fout.write((char*)&meta.tupleSize, sizeof meta.tupleSize);
	// 属性数量
	uint16_t num = (uint16_t)meta.fields.size();
	fout.write((char*)&num, sizeof num);
	// 模式
	for (auto& field : meta.fields) {
		fout << field;
	}
	// holeQ
	while (!meta.holeQ.empty()) {
		auto t = meta.holeQ.front();
		fout.write((char*)&t, sizeof t);
		meta.holeQ.pop();
	}
	return fout;
}

// 保存表的元数据
inline void TableMeta::save(const std::string& fileName)
{
	std::ofstream fout(fileName, std::ios::binary);
	fout << *(this);
	fout.close();
}

// called in load()
inline std::ifstream& operator>>(std::ifstream& fin, TableMeta& meta)
{
	// 元组大小
	fin.read((char*)&meta.tupleSize, sizeof meta.tupleSize);
	// 属性数量
	uint16_t num;
	fin.read((char*)&num, sizeof num);
	// 模式
	auto& fields = meta.fields;
	fields.reserve(num);
	int i = 0;
	while (num--) {
		Field field;
		fields.push_back(field);
		fin >> fields[i++];
	}
	// holeQ
	HoleQueue::value_type t;
	while (fin.read((char*)&t, sizeof t)) {
		meta.holeQ.push(t);
	}
	return fin;
}

// 从文件载入表的元数据
inline void TableMeta::load(const std::string& fileName)
{
	std::ifstream fin(fileName, std::ios::binary);
	fin >> *(this);
	fin.close();

	// 一些复原工作。复原map，以及模式中每个字段的offset
	int i = 0;
	uint16_t offset = 0;
	for (Field& field : fields) {
		map[field.name] = i++;
		field.offset = offset;
		offset += field.size;
	}
}

// 将属性名映射为该属性在属性集中的下标。返回-1则表示不存在该属性
inline int TableMeta::posOf(const std::string& name) const
{
	try {
		return map.at(name);
	}
	catch (std::out_of_range err) {
		return -1;
	}
}

// 请确保的确存在该属性
inline const Field& TableMeta::of(const std::string& name) const
{
	return fields[posOf(name)];
}
// 是否存在指定的属性
inline bool TableMeta::exist(const std::string& name) const
{
	return posOf(name) >= 0;
}
#endif // !TABLEMETA_H