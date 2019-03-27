#ifndef KEY_TYPE_H
#define KEY_TYPE_H
#include <cstddef>
class key {
	size_t size_;
public:
	size_t size() const;

};

inline size_t key::size() const {
	return size_;
}
#endif