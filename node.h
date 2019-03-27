#ifndef NODE_H
#define NODE_H
#include<string>
#include<map>
#include<iostream>
#include<set>
struct Const;
struct BiOP;
struct UnOP;
struct BOOL;

struct Node {// *(STAR) IS NODE
	enum NType {
		/* NULL = 0 */
		STAR = 1, // "*" which is not operater
		IDENTIFIER, // identifier
		LONGLONG_CONST, DOUBLE_CONST, STRING_CONST, BOOL_CONST, // constants

		UMINUS,	// unary operator -
		ISNULL, ISNOTNULL,// IS NULL		IS NOT NULL
		COUNT, SUM, AVG, MAX, MIN,// function
		NOT, // !/NOT

		PLUS, MINUS, MULTIPLY, DIVIDE, // +  -  *  /
		ANDOP, OR, // AND/&&  OR/||
		E, GE, G, LE, L, NE, // =  >=  >  <=  <  !=/<>

		BETWEENAND,// BETWEEN AND
	};
	const NType type;

	Node(NType t) :type(t) {};
	virtual ~Node() {}
	bool isLeaf() { 
		return type >= NType(NULL) && type <= BOOL_CONST; 
	}
	bool isUnary() { 
		return type >= UMINUS && type <= NOT; 
	}
	bool isBinary() { 
		return type >= PLUS && type <= NE; 
	}
	bool isTernary() { 
		return type == BETWEENAND; 
	}
	bool isFunction() {
		return type >= COUNT && type <= MIN;
	}

	void getVarList(std::set<std::string>& varlist);
	bool evalBool(const std::map<std::string, Const*>& m);
	Const* eval(const std::map<std::string, Const*>& m);
};
// 一元运算，包括各种聚集函数
struct UnOP :public Node { // !/NOT  ISNULL  ISNOTNULL  -
	Node* son;
	UnOP(NType t, Node *s) :Node(t), son(s) {}
	~UnOP() {
		delete son;
	}
};
// 二元运算
struct BiOP :public Node { // AND/&& OR/|| = >= > <= < !=/<>
	Node* left;
	Node* right;
	BiOP(NType t, Node* l, Node* r) :Node(t), left(l), right(r) {}
	~BiOP() {
		delete left;
		delete right;
	}
};
// 唯一的3元运算，between...and
struct BETWEEN :public Node {
	Node* what;
	Node* min;
	Node* max;
	BETWEEN(Node* w, Node* mi, Node* ma) :Node(BETWEENAND), what(w), min(mi), max(ma) {}
	~BETWEEN() {
		delete what;
		delete min;
		delete max;
	}
};
// 函数
struct Function :public Node {
	const std::string id;
	Function(NType funType, const std::string& s) :Node(funType), id(s) {}
	std::string getFuncName() {
		switch (type)
		{
		case COUNT:
			return "COUNT";
		case SUM:
			return "SUM";
		case AVG:
			return "AVG";
		case MAX:
			return "MAX";
		case MIN:
			return "MIN";
		}
	}
};
struct ID :public Node {
	const std::string name;
	ID(const std::string& n) :Node(IDENTIFIER), name(n) {}
};


struct Const :public Node {
	Const(NType t) :Node(t) {}

	BOOL* operator&&(const Const& right);
	BOOL* operator||(const Const& right);
	BOOL* operator!();

	BOOL * operator>(const Const & right);
	BOOL * operator<(const Const & right);
	BOOL * operator<=(const Const & right);
	BOOL * operator>=(const Const & right);
	BOOL* operator==(const Const& right);
	BOOL* operator!=(const Const& right);

	Const* operator+(const Const& right);
	Const* operator-(const Const& right);
	Const* operator*(const Const& right);
	Const * operator/(const Const & right);

	Const* operator-();
};
struct LONGLONG :public Const {
	long long number;
	LONGLONG(long long ll) :Const(LONGLONG_CONST), number(ll) {}
};
struct DOUBLE :public Const {
	double number;
	DOUBLE(double db) :Const(DOUBLE_CONST), number(db) {}
};
struct STRING :public Const {
	const std::string str;
	STRING(const std::string& s) :Const(STRING_CONST), str(s) {}
};
struct BOOL :public Const {
	const bool b;
	BOOL(bool b_) :Const(BOOL_CONST), b(b_) {}
};
struct NULLConst :public Const {
	NULLConst() :Const(NType(NULL)) {}
};

std::ostream& operator<<(std::ostream&, const Const&);
#endif