#pragma once
#ifndef ADATABASE_H
#define ADATABASE_H
#include <dirent.h>
#include <unordered_map>
#include <sys/types.h>
#include "myassert.h"
#include "Table.h"

// 遍历目录文件的目录项，返回值表示是否成功遍历了所有目录项
//bool traverseDirItem(DIR* dir, void(*handler)(dirent*))
//{
//	dirent* d;
//	auto t = errno = 0;
//	while (d = readdir(dir)) {
//		handler(d);
//	}
//	return errno == t;
//}

inline bool eatSuffix(char* str, const char* suffix)
{
	int i = strlen(str) - 1;
	int j = strlen(suffix) - 1;
	if (i < j)
		return false;
	for (; j >= 0; i--, j--) {
		if (str[i] != suffix[j])
			return false;
	}
	str[i + 1] = 0;
	return true;
}

// TODO 单例模式，该类只能有一个对象
class ADataBase {
	std::unordered_map<std::string, Table*> map;
public:
	ADataBase();
	~ADataBase();
	Table* operator[](const std::string& name) const;
	bool exist(const std::string& name) const;
	Table* addTable(const std::string& name);
	bool delTable(const std::string& name);
};

inline ADataBase::ADataBase()
{
	// 打开目录
	DIR* dir = opendir("./");
	assert(dir != nullptr, "chdir");

	// 读取目录内容，建立map
	dirent* d = nullptr;
	while (d = readdir(dir)) {
		char* name = d->d_name;
		if (eatSuffix(name, Table::dbSuffix)) {
			map.emplace(name, new Table(name, true));
		}
	}
}

inline ADataBase::~ADataBase()
{
	// 遍历map，释放内存空间
	for (auto& e : map) {
		delete e.second;
		e.second = nullptr;
	}
}

inline Table* ADataBase::operator[](const std::string& name) const
{
	try {
		return map.at(name);
	}
	catch (std::out_of_range e) {
		return nullptr;
	}
}

inline bool ADataBase::exist(const std::string& name) const
{
	return operator[](name);
}

inline Table* ADataBase::addTable(const std::string& name)
{
	Table* ret = new Table(name);
	map.emplace(name, ret);
	return ret;
}

inline bool ADataBase::delTable(const std::string& name)
{
	Table* which = (*this)[name];
	if (!which)
		return false;
	which->drop();
	delete which;
	map.erase(name);
	return true;
}
#endif // !ADATABASE_H