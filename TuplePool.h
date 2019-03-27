#pragma once
#ifndef TUPLEPOOL_H
#define TUPLEPOOL_H
#include <cstring>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "myassert.h"
#include "Tuple.h"

//template <size_t max_size = 1024>
class TuplePool {
	int fd;				// 表文件的描述符
	size_t fs;			// 表文件的大小
	int last;			// 最后一个元组
	int cap;			// 缓冲池容量
	int which;			// 当前遍历到哪一条元组，which >= 0
	TableMeta* meta;	// 所属的表的元数据
	uint8_t* pool;		// 缓冲池
	uint8_t* mmf;		// 内存映射文件，将分页管理交由OS完成，且加快了文件读写 (少了一次内核缓存)

	static constexpr size_t max_size = 128;

	uint8_t* ptr(int which) const {
		return pool + which * meta->tupleSize;
	}

	off_t off(int which) const {
		return which * meta->tupleSize;
	}

	Tuple seek(int which) const { // 在内存映射中seek
		return Tuple(mmf + which * meta->tupleSize, *meta);
	}

	// 建立内存映射
	void mmap(size_t len) {
		mmf = (uint8_t*)::mmap(nullptr, len, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
		assert(mmf != MAP_FAILED, "mmap");
		/*if (mmf == MAP_FAILED)
			mmf = nullptr;*/
	}

	// 重建内存映射，增加大小为size的空间
	void mremap(size_t size) {
		mmf = (uint8_t*)::mremap(mmf, fs, fs + size, MREMAP_MAYMOVE);
		assert(mmf != MAP_FAILED, "mremap");
	}

	friend class Table;
public:
	TuplePool(const std::string& name)
		:fd(open(name.c_str(), O_RDWR | O_CREAT, 0644)),which(0),
		last(-1), meta(nullptr), pool(nullptr), mmf(nullptr) {
		assert(fd >= 0, "open");
		// init fs
		struct stat m;
		int ret = fstat(fd, &m);
		assert(ret == 0, "fstat");
		fs = m.st_size;
	}

	// 真正可以初始化的时机是添加完所有的字段
	void init(TableMeta& meta) {
		this->meta = &meta;
		cap = max_size / meta.tupleSize;
		pool = new uint8_t[cap * meta.tupleSize]{ 0 };
		if (fs)
			mmap(fs);
	}
	
	~TuplePool() {
		// 将pool中的元组写到磁盘，然后才可以delete以释放内存
		if(meta)
			save();	// TODO 为了方便，这里重用了save()，里面有mmap操作，而后面就析构了，效率有点低。
		delete[] pool;

		// 关闭文件描述符
		close(fd);

		// 同步修改到文件，并munmap
		if (mmf) {
			int ret = msync(mmf, fs, MS_SYNC);
			assert(ret == 0, "msync");
			ret = munmap(mmf, fs);
			assert(ret == 0, "munmap");
		}
	} 

	// 往缓冲池中插入空元组，并返回该元组
	Tuple insert() {	// 不要怀疑，这里可以直接返回变量，而不用返回引用，靠编译器进行RVO优化，下同
		if (size() >= cap) {
			save();
			last = -1;
		}
		last++;
		return Tuple(ptr(last), *meta);
	}

	// 删除所有元组
	void removeAll() {
		last = -1;
		int ret;
		if (mmf) { // 这个时候可能还没有建立内存映射
			ret = munmap(mmf, fs);
			assert(ret == 0, "munmap");
			mmf = nullptr; // !!!
		}
		fs = 0; // 不能在解除内存映射之前修改fs，及截断文件
		ret = ftruncate(fd, 0);
		assert(ret == 0, "ftruncate");
	}

	void remove() {
		int which = this->which - 1;
		if (which < size()) { // 从缓冲池中删除
			if (which != last) {
				memcpy(ptr(which), ptr(last), meta->tupleSize);
			}
			last--;
		}
		else { // 从内存映射中删除，即置相关标志位为空
			seek(which - size()).setEmpty(true);
			HoleQueue& q = meta->holeQ;
			q.push(which - size());
		}
	}

	Tuple operator[](int which) const {	// 请确保 0 <= which < size()
		return Tuple(ptr(which), *meta);
	}

	Tuple next() {
		if (which < size()) { // 从pool中拿元组
			return this->operator[](which++);
		}
		else { // 从内存映射中拿元组
			int max = fs / meta->tupleSize + size();
			while (which < max && seek(which - size()).isEmpty()) {
				which++;
			}
			if(which == max) {
				which = 0;
				throw std::out_of_range("a23187");
			}
			else {
				int i = which - size();
				which++;
				return seek(i);
			}
		}
	}

	void save() {
		HoleQueue& q = meta->holeQ;
		for (int i = 0; i < size(); i++) {
			if (q.empty()) {
				// append to db file
				off_t ret1 = lseek(fd, 0, SEEK_END);
				assert(ret1 != -1, "lseek");

				size_t dSize = (size() - i) * meta->tupleSize;	// 新增部分的大小
				ssize_t ret2 = write(fd, ptr(i), dSize);
				assert(ret2 == dSize, "write");
				
				if (mmf)
					mremap(dSize);	// 当有append操作时，必须重建内存映射，这也是mmap方法的一个缺点
				else
					mmap(fs + dSize);
				fs += dSize;
				break;	// !!!
			}
			else {
				// reuse the hole in db file
				ssize_t ret = pwrite(fd, ptr(i), meta->tupleSize, off(q.front()));
				assert(ret == meta->tupleSize, "pwrite");
				q.pop();
			}
		}
	}

	int size() const {
		return last + 1;
	}

	int capacity() const {
		return cap;
	}
};
// TODO 不嫌麻烦的话，可以把成员函数的定义移到类模板之外进行
#endif // !TUPLEPOOL_H