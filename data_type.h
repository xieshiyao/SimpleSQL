#ifndef DATA_TYPE_H
#define DATA_TYPE_H
struct Data_type {
	enum DType {
		INT, 
		SMALLINT,
		BIGINT,
		REAL, 
		DOUBLEPRECISION,
		BOOLEAN,
		CHAR
	};
	DType dtype;
	int lenOfStr; // only for CHAR(...) type
	Data_type(){}
	Data_type(DType t,int len=0):dtype(t),lenOfStr(len){}
};

struct Data_def {	// the type of create_definition
	std::string name;
	Data_type dtype;
	unsigned char attrs; // bits
	Data_def() {}
	Data_def(const std::string& na,const Data_type& dt,unsigned char ats):
		name(na),dtype(dt),attrs(ats) {}
};
#endif
