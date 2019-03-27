#include "node.h"
// 获取变量列表
void Node::getVarList(std::set<std::string>& varlist)
{
	if (isUnary()) {
		if (isFunction()) {
			// TODO 目前只在where子句中调用该函数，所以不应出现聚集函数，后面支持处理having子句是另外一回事
			throw "Invalid use of group function.";
		}
		else {
			UnOP* unop = (UnOP*)this;
			unop->son->getVarList(varlist);
		}
		return;
	}
	if (isBinary()) {
		BiOP* biop = (BiOP*)this;
		biop->left->getVarList(varlist);
		biop->right->getVarList(varlist);
		return;
	}
	if (isTernary()) {
		BETWEEN* bt = (BETWEEN*)this;
		bt->what->getVarList(varlist);
		bt->min->getVarList(varlist);
		bt->max->getVarList(varlist);
		return;
	}
	if (type == IDENTIFIER) {
		varlist.insert(((ID*)this)->name);
		return;
	}
	if (type == STAR) {
		varlist.insert("*");
	}
}

bool Node::evalBool(const std::map<std::string, Const*>& m)
{
	Const* temp = eval(m);
	if (temp->type == (NType)NULL)
	{
		delete temp;
		return false;
	}
	if (temp->type != BOOL_CONST)
		throw "no bool type";
	bool b = ((BOOL*)temp)->b;
	delete temp;
	return b;
}

Const* Node::eval(const std::map<std::string, Const*>& m)
{
	if (type == IDENTIFIER)
	{
		const std::string& name = ((ID*)this)->name;
		if (m.count(name))
		{
			Const* val = m.at(name);
			Const* valCopy;
			switch (val->type)
			{
			case LONGLONG_CONST:
				valCopy = new LONGLONG(*((LONGLONG*)val));
				break;
			case DOUBLE_CONST:
				valCopy = new DOUBLE(*(DOUBLE*)val);
				break;
			case BOOL_CONST:
				valCopy = new BOOL(*(BOOL*)val);
				break;
			case STRING_CONST:
				valCopy = new STRING(*(STRING*)val);
				break;
			case NULL:
				valCopy = new NULLConst();
				break;
			default:
				throw "What the fuck! Pass me longlong, double, bool, string or null; no int\
	, no real, and no smallint!!!";
			}
			return valCopy;
		}
		else // error
			throw "Not value for this id";
	}
	if (isLeaf())
	{
		switch (type)
		{
		case STAR:
			throw "Deal with * by yourself!";
		case NULL:
			return new NULLConst();
		case LONGLONG_CONST:
			return new LONGLONG((LONGLONG&)*this);
		case DOUBLE_CONST:
			return new DOUBLE((DOUBLE&)*this);
		case STRING_CONST:
			return new STRING((STRING&)*this);
		case BOOL_CONST:
			return new BOOL((BOOL&)*this);
		}
	}
	if (isBinary())
	{
		Const* left = ((BiOP*)this)->left->eval(m);
		Const* right = ((BiOP*)this)->right->eval(m);
		Const* ret;
		switch (type)
		{
		case PLUS:
			ret = *left + *right;
			break;
		case MINUS:
			ret = *left - *right;
			break;
		case MULTIPLY:
			ret = *left * *right;
			break;
		case DIVIDE:
			ret = *left / *right;
			break;
		case G:
			ret = *left > *right;
			break;
		case L:
			ret = *left < *right;
			break;
		case LE:
			ret = *left <= *right;
			break;
		case GE:
			ret = *left >= *right;
			break;
		case NE:
			ret = *left != *right;
			break;
		case E:
			ret = *left == *right;
			break;
		case OR:
			ret = *left || *right;
			break;
		case ANDOP:
			ret = *left&&*right;
			break;
			// TODO
		default:
			delete left;
			delete right;
			throw "Unknowned error";
		}
		delete left;
		delete right;
		return ret;
	}
	if (isUnary())
	{
		Const* son = ((UnOP*)this)->son->eval(m);
		Const* ret;
		switch (type)
		{
		case UMINUS:
			ret = -*son;
			break;
		case ISNULL:
			ret = new BOOL(son->type == (NType)NULL);
			break;
		case ISNOTNULL:
			ret = new BOOL(son->type != (NType)NULL);
			break;
		case NOT:
			ret = !*son;
			break;
		case COUNT: case SUM: case AVG: case MAX: case MIN:
			delete son;
			throw "I can't deal with funcion";
		default:
			delete son;
			throw "Unknowned error";
		}
		delete son;
		return ret;
	}
	if (isTernary())
	{
		Const* what = ((BETWEEN*)this)->what->eval(m);
		Const* min = ((BETWEEN*)this)->min->eval(m);
		Const* max = ((BETWEEN*)this)->max->eval(m);

		//*(*what >= *min) && *(*what <= *max);
		BOOL* cond1 = *what >= *min;
		BOOL* cond2 = *what <= *max;
		BOOL* ret = *cond1 && *cond2;
		delete cond1;
		delete cond2;

		delete what;
		delete min;
		delete max;
		return ret;
	}
	throw "Unknowned error";
}










// Const

BOOL* Const::operator!()
{
	if (type == (NType)NULL)
		return new BOOL(false);
	if (type != BOOL_CONST)
		throw "NOT what?";
	return new BOOL(!((BOOL*)this)->b);
}
BOOL* Const::operator&&(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new BOOL(false);
	if (type != BOOL_CONST || right.type != BOOL_CONST)
		throw "no boolean expression in two side of &&";
	return new BOOL(((BOOL*)this)->b && ((BOOL&)right).b);
}
BOOL* Const::operator||(const Const& right)
{
	if (type == (NType)NULL)// left is NULL
	{
		if (right.type == (NType)NULL)
			return new BOOL(false);
		if (right.type == BOOL_CONST)
			return new BOOL(((BOOL&)right).b);
		throw "no boolean expression in two side of ||";
	}
	if (right.type == (NType)NULL)// left is not null, right is NULL
	{
		if (type == BOOL_CONST)
			return new BOOL(((BOOL*)this)->b);
		throw "no boolean expression in two side of ||";
	}
	if (type != BOOL_CONST || right.type != BOOL_CONST)// both left and right is not NULL
		throw "no boolean expression in two side of ||";
	return new BOOL(((BOOL*)this)->b || ((BOOL&)right).b);
}
BOOL* Const::operator>(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new BOOL(false);
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new BOOL(((LONGLONG*)this)->number > ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new BOOL(((DOUBLE*)this)->number > ((DOUBLE&)right).number);
	case BOOL_CONST:
		throw "can't compare values of two bool";
	case STRING_CONST:
		return new BOOL(((STRING*)this)->str > ((STRING&)right).str);
	}
}
BOOL* Const::operator<(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new BOOL(false);
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new BOOL(((LONGLONG*)this)->number < ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new BOOL(((DOUBLE*)this)->number < ((DOUBLE&)right).number);
	case BOOL_CONST:
		throw "can't compare values of two bool";
	case STRING_CONST:
		return new BOOL(((STRING*)this)->str < ((STRING&)right).str);
	}
}
BOOL* Const::operator<=(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new BOOL(false);
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new BOOL(((LONGLONG*)this)->number <= ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new BOOL(((DOUBLE*)this)->number <= ((DOUBLE&)right).number);
	case BOOL_CONST:
		throw "can't compare values of two bool";
	case STRING_CONST:
		return new BOOL(((STRING*)this)->str <= ((STRING&)right).str);
	}
}
BOOL* Const::operator>=(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new BOOL(false);
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new BOOL(((LONGLONG*)this)->number >= ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new BOOL(((DOUBLE*)this)->number >= ((DOUBLE&)right).number);
	case BOOL_CONST:
		throw "can't compare values of two bool";
	case STRING_CONST:
		return new BOOL(((STRING*)this)->str >= ((STRING&)right).str);
	}
}
BOOL* Const::operator==(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new BOOL(false);
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new BOOL(((LONGLONG*)this)->number == ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new BOOL(((DOUBLE*)this)->number == ((DOUBLE&)right).number);
	case BOOL_CONST:
		return new BOOL(((BOOL*)this)->b == ((BOOL&)right).b);
	case STRING_CONST:
		return new BOOL(((STRING*)this)->str == ((STRING&)right).str);
	}
}
BOOL* Const::operator!=(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new BOOL(false);
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new BOOL(((LONGLONG*)this)->number != ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new BOOL(((DOUBLE*)this)->number != ((DOUBLE&)right).number);
	case BOOL_CONST:
		return new BOOL(((BOOL*)this)->b != ((BOOL&)right).b);
	case STRING_CONST:
		return new BOOL(((STRING*)this)->str != ((STRING&)right).str);
	}
}
Const* Const::operator+(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new NULLConst();
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new LONGLONG(((LONGLONG*)this)->number + ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new DOUBLE(((DOUBLE*)this)->number + ((DOUBLE&)right).number);
	case BOOL_CONST:
		throw "bool add bool";
	case STRING_CONST:
		return new STRING(((STRING*)this)->str + ((STRING&)right).str);
	}
}
Const* Const::operator-(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new NULLConst();
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new LONGLONG(((LONGLONG*)this)->number - ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new DOUBLE(((DOUBLE*)this)->number - ((DOUBLE&)right).number);
	case BOOL_CONST:
		throw "bool minus bool";
	case STRING_CONST:
		throw "You can not subtract a string from an other string. Are you crazy?";
	}
}
Const* Const::operator*(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new NULLConst();
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new LONGLONG(((LONGLONG*)this)->number*((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new DOUBLE(((DOUBLE*)this)->number*((DOUBLE&)right).number);
	case BOOL_CONST:
		throw "bool multiply bool";
	case STRING_CONST:
		throw "string multiply string??? EXCUSE ME???";
	}
}
Const* Const::operator/(const Const& right)
{
	if (type == (NType)NULL || right.type == (NType)NULL)
		return new NULLConst();
	if (type != right.type)
		throw "Unmatched type";
	switch (type)
	{
	case LONGLONG_CONST:
		return new LONGLONG(((LONGLONG*)this)->number / ((LONGLONG&)right).number);
	case DOUBLE_CONST:
		return new DOUBLE(((DOUBLE*)this)->number / ((DOUBLE&)right).number);
	case BOOL_CONST:
		throw "bool add bool";
	case STRING_CONST:
		throw "string divide string??? EXCUSE ME???";
	}
}
Const* Const::operator-()
{
	if (type == (NType)NULL)
		return new NULLConst();
	switch (type)
	{
	case LONGLONG_CONST:
		return new LONGLONG(-((LONGLONG*)this)->number);
	case DOUBLE_CONST:
		return new DOUBLE(-((DOUBLE*)this)->number);
	case BOOL_CONST:
		return new BOOL(!((BOOL*)this)->b);
	case STRING_CONST:
		throw "What's the opposite of a string? You stupid.";
	}
}






std::ostream& operator<<(std::ostream& os, const Const& cst)
{
	switch (cst.type)
	{
	case Const::LONGLONG_CONST:
		os << ((LONGLONG&)cst).number;
		break;
	case Const::DOUBLE_CONST:
		os << ((DOUBLE&)cst).number;
		break;
	case Const::BOOL_CONST:
		os << (((BOOL&)cst).b ? "true" : "false");
		break;
	case Const::STRING_CONST:
		os << ((STRING&)cst).str;
		break;
	case NULL:
		os << "null";
		break;
	}
	return os;
}
