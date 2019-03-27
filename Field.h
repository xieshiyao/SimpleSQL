#pragma once
#ifndef FIELD_H
#define FIELD_H
#include <cstdint>
#include <fstream>
#include <string>

class Field {
	std::string name;	// 名称
	uint8_t type;		// 类型，见下面 enum
	uint8_t info;		// 属性，[00000, UNIQUE, 主键, NOT-NULL]
	uint16_t size;		// 大小
	uint16_t offset;	// 偏移量 size 与 offset 相关联，在文件中只存size
	static constexpr uint16_t baseSize = 3;	// 字段存在文件中，至少要 3Byte
public:
	enum :uint8_t {
		INT,		// int
		SMALLINT,	// short
		BIGINT,		// long		linux(64bit)下 sizeof(long) = 8
		REAL,		// float
		DOUBLE,		// double
		BOOL,		// bool
		STRING		// string 非可变长 长度必须大于0
	};

	static const uint16_t sizeOf[6];
	/*= {
		sizeof(int),
		sizeof(short),
		sizeof(long),
		sizeof(float),
		sizeof(double),
		sizeof(bool)
	};*/

	static constexpr uint8_t NOT_NULL = 0x01;
	static constexpr uint8_t PRIM_KEY = 0x02;
	static constexpr uint8_t UNIQUE = 0x04;

	Field() = default;
	Field(const std::string& name, uint8_t type, uint8_t info, uint16_t size, uint16_t offset);

	uint8_t getType() const;
	uint16_t getSize() const;
	const std::string& getName() const;

	bool isPrimKey() const;
	bool isNotNull() const;
	bool isUnique() const;

	friend class Table;
	friend class Tuple;
	friend class TableMeta;
	friend std::ostream& operator<<(std::ostream& cout, const Field& field);
	friend std::ofstream& operator<<(std::ofstream& fout, const Field& field);
	friend std::ifstream& operator>>(std::ifstream& fin, Field& field);
};

const uint16_t Field::sizeOf[6] = {
	sizeof(int),
	sizeof(short),
	sizeof(long),
	sizeof(float),
	sizeof(double),
	sizeof(bool)
};

inline Field::Field(const std::string& name, uint8_t type, uint8_t info, uint16_t size, uint16_t offset)
	:name(name), type(type), info(info), size(size), offset(offset) {}

inline uint8_t Field::getType() const
{
	return type;
}

inline uint16_t Field::getSize() const
{
	return size;
}

inline const std::string& Field::getName() const
{
	return name;
}

inline bool Field::isPrimKey() const
{
	return info & PRIM_KEY;
}

inline bool Field::isNotNull() const
{
	return info & NOT_NULL;
}

inline bool Field::isUnique() const
{
	return info & UNIQUE;
}

inline std::ostream& operator<<(std::ostream& cout, const Field& field)
{
	cout << field.name;
	return cout;
}

inline std::ofstream& operator<<(std::ofstream& fout, const Field& field)
{
	// 名称长度
	uint8_t len = (uint8_t)field.name.length();
	fout.write((char*)&len, sizeof len);
	// 名称
	fout.write(field.name.data(), field.name.length());
	// 类型
	fout.write((char*)&field.type, sizeof field.type);
	// 属性
	fout.write((char*)&field.info, sizeof field.info);
	// 大小，当字段类型为STRING时，才需要写大小到文件
	if (field.type == Field::STRING)
		fout.write((char*)&field.size, sizeof field.size);
	return fout;
}

inline std::ifstream& operator>>(std::ifstream& fin, Field& field)
{
	// 名称长度
	uint8_t len;
	fin.read((char*)&len, sizeof len);
	// 名称
	field.name.reserve(len);
	char ch;
	while (len--) {
		fin.read(&ch, sizeof ch);
		field.name += ch;
	}
	// 类型
	fin.read((char*)&field.type, sizeof field.type);
	// 属性
	fin.read((char*)&field.info, sizeof field.info);
	// 大小
	if (field.type == Field::STRING)
		fin.read((char*)&field.size, sizeof field.size);
	else
		field.size = Field::sizeOf[field.type];
	return fin;
}
#endif // !FIELD_H