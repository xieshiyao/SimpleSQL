#pragma once
#ifndef TABLE_H
#define TABLE_H
#include "bplustree.h"
#include "myassert.h"
#include "TuplePool.h"

class Table {
	std::string name;	// 表名
	TableMeta meta;		// 元数据
	TuplePool/*<20>*/ pool;	// 元组缓冲池
	bplus_tree* index;	// 索引，B+树

	static constexpr int bptBlockSize = 2048;		// b+树结点块的大小

	void initMeta();
	void initPool();
	void drop() const;

	friend class ADataBase;
public:
	static constexpr const char* dbSuffix = ".db";		// 各种后缀
	static constexpr const char* metaSuffix = "_METADATA";
	static constexpr const char* idxSuffix = "_INDEX";
	static constexpr const char* bootSuffix = "_INDEX_BOOT";

	Table(const std::string& name, bool load = false);
	~Table();
	TableMeta& getMeta();
	void saveMeta();

	TuplePool& getPool();

	void initIdx();
	void deinitIdx();

	void init();
};

// ctor，参数tag表示是否从文件中恢复数据，默认为false
inline Table::Table(const std::string& name, bool tag) :name(name),
	pool(name + dbSuffix), index(nullptr)
{
	if (tag) {
		meta.load(name + metaSuffix);
		initPool();
	}
}

// dtor
inline Table::~Table()
{
	//deinitIdx();
	saveMeta();
}

// 完成表的元数据的最终初始化，在添加完所有的字段之后调用
inline void Table::initMeta()
{
	meta.init();
}

// 获得表的元数据
inline TableMeta& Table::getMeta()
{
	return meta;
}

// 保存表的元数据
inline void Table::saveMeta()
{
	meta.save(name + metaSuffix);
}

// 获得表的元组缓冲池
inline TuplePool& Table::getPool()
{
	return pool;
}

// 初始化元组缓冲池，在添加完所有的字段之后调用，而且要先调用initMeta()
inline void Table::initPool()
{
	pool.init(meta);
}

// 创建主索引，或初次创建、或从文件导入 TODO
inline void Table::initIdx()
{
	/*std::string idxFileName = name + idxSuffix;
	int keySize = 0;
	for (const Field& field : meta.getFields()) {
		if (field.isPrimKey())
			keySize += field.size;
	}
	index = bplus_tree_init((char*)idxFileName.c_str(), bptBlockSize, keySize);*/
}

// 删除索引，回收内存空间， called in dtor
inline void Table::deinitIdx()
{
	bplus_tree_deinit(index);
	index = nullptr;
}

// 删除表，以及对应的索引文件
inline void Table::drop() const
{
	int ret = ::remove((name + dbSuffix).c_str());
	ret += ::remove((name + metaSuffix).c_str());
	//assert(ret == 0, "remove");
	ret += ::remove((name + idxSuffix).c_str());
	ret += ::remove((name + bootSuffix).c_str());
}

inline void Table::init()
{
	initMeta();
	initPool();
}
#endif
