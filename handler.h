#pragma once
#ifndef HANDLER_H
#define HANDLER_H
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "node.h"
#include "data_type.h"
#include "ADataBase.h"

using std::cerr;
using std::cout;
using std::endl;
using std::string;
using std::vector;
using std::pair;
using std::set;
using std::out_of_range;
using std::map;

static ADataBase db;
const char* todo = "You have an error in your SQL syntax. ";
const char* error = "You have an error in your SQL syntax. ";

// ������
inline void create_table_handle(const string& name, const vector<Data_def> fields)
{
	Table* table = db[name];
	if (table) {
		// �ñ��Ѵ���
		cerr << "Table '" << name << "' already exists." << endl;
	}
	else {
		bool right = false;	// ��ʾ�����Ƿ���ȷ (�Ƿ����ظ����ԡ��Ƿ���ָ������)
		// ����Ƿ���ָ������
		for (const auto& field : fields) {
			if (field.attrs & Field::PRIM_KEY) {
				right = true;
				break;
			}
		}
		if (!right) {
			cerr << "Primary key not specified." << endl;
			return;
		}
		table = db.addTable(name);
		TableMeta& meta = table->getMeta();
		for (const auto& field : fields) {
			if (meta.exist(field.name)) {
				// �������Ѵ���
				cerr << "Duplicate column name '" << field.name << "'." << endl;
				db.delTable(name);	// ??? may occur error
				right = false;
				break;
			}
			else if (field.dtype.dtype == Data_type::DType::CHAR) {
				meta.addField(field.name, Field::STRING, field.attrs, (uint16_t)field.dtype.lenOfStr);
			}
			else {
				meta.addField(field.name, field.dtype.dtype, field.attrs);
			}
		}
		if (right) {
			table->init();
			cout << "Query OK, 0 rows affected." << endl;
		}
	}
}

// ɾ����
inline void drop_table_handle(const string& name)
{
	if (!db.delTable(name)) {
		cerr << "Unknown table '" << name << "'." << endl;
	}
	else {
		cout << "Query OK, 0 rows affected." << endl;
	}
}

// ��������
inline bool check_type(const Field& field, const Node* v)
{
	switch (field.getType())
	{
	case Field::INT:
	case Field::SMALLINT:
	case Field::BIGINT:
		if (v->type != Node::LONGLONG_CONST && v->type != (Node::NType)NULL) {
			cerr << "The type of '" << field << "' must be INT/SMALLINT/BIGINT." << endl;
			return false;
		}
		break;
	case Field::REAL:
	case Field::DOUBLE:
		if (v->type != Node::DOUBLE_CONST && v->type != (Node::NType)NULL) {
			cerr << "The type of '" << field << "' must be REAL/DOUBLE." << endl;
			return false;
		}
		break;
	case Field::BOOL:
		if (v->type != Node::BOOL_CONST && v->type != (Node::NType)NULL) {
			cerr << "The type of '" << field << "' must be BOOLEAN." << endl;
			return false;
		}
		break;
	case Field::STRING:
		if (v->type != Node::STRING_CONST && v->type != (Node::NType)NULL) {
			cerr << "The type of '" << field << "' must be STRING." << endl;
			return false;
		}
		if (v->type == Node::STRING_CONST && ((STRING*)v)->str.length() >= field.getSize()) { // field.getSize() ����\0����
			cerr << "Data too long for column '" << field << "' at row 1." << endl;
			return false;
		}
		break;
	}
	return true;
}
inline bool have_all_necessary_field(const vector<Field>& defs, const vector<string>& fields)
{
	std::unordered_set<string> set(fields.begin(), fields.end());
	bool ret = true;
	for (const Field& def : defs) {
		if (def.isPrimKey() && set.find(def.getName()) == set.end()) {
			cerr << "Missing primary field '" << def << "'." << endl;
			ret = false;
		}
		if (def.isNotNull() && set.find(def.getName()) == set.end()){
			cerr << "Missing not-null field '" << def << "'." << endl;
			ret = false;
		}
	}
	return ret;
}
inline bool check_for_insert(const vector<Field>& fields, const vector<Node*>& values)
{
	if (values.size() != fields.size()) { // �ж����������Ƿ�ƥ��
		cerr << "Column count doesn't match value count at row 1." << endl;
		return false;
	}
	// �ж��Ƿ����nullֵ��ͻ
	int i = 0;
	for (Node* v : values) {
		const Field& field = fields[i++];
		if (v->type == (Node::NType)NULL && (field.isNotNull() || field.isPrimKey())) {
			cerr << "Column '" << field << "' cannot be null." << endl;
			return false;
		}
	}
	// �ж�������ֵ�������Ƿ�һһƥ��
	i = 0;
	for (const Field& field : fields) {
		Node* v = values[i++];
		if (!check_type(field, v))
			return false;
	}
	return true;
}
inline bool check_for_insert(const TableMeta& meta, const vector<string>& fields, const vector<Node*>& values)
{
	if (fields.size() != values.size()) { // �ж����������Ƿ�ƥ��
		cerr << "Column count doesn't match value count at row 1." << endl;
		return false;
	}
	// Ϲд���Ե�
	for (const string& field : fields) {
		if (!meta.exist(field)) {
			cerr << "Unknown column '" << field << "' in field list." << endl;
			return false;
		}
	}
	// �ж��Ƿ������е������ԣ������е�not-null����
	const auto& def = meta.getFields();
	if (!have_all_necessary_field(def, fields))
		return false;

	// �ж��Ƿ����nullֵ��ͻ
	int i = 0;
	for (Node* v : values) {
		const Field& field = meta.of(fields[i++]);
		if (v->type == (Node::NType)NULL && (field.isNotNull() || field.isPrimKey())) {
			cerr << "Column '" << field << "' cannot be null." << endl;
			return false;
		}
	}
	// �ж�������ֵ�������Ƿ�һһƥ��
	i = 0;
	for (Node* v : values) {
		const Field& field = meta.of(fields[i++]);
		if (!check_type(field, v))
			return false;
	}
	return true;
}
inline void insert_into_pool(TuplePool& pool, const vector<Field>& fields, const vector<Node*>& values)
{
	// ������ȷ���ſ��Ե��ñ�����
	Tuple tuple = pool.insert();
	int i = 0;
	for (const Field& field : fields) {
		Node* v = values[i];
		if (v->type == (Node::NType)NULL) {
			tuple.setNull(i, true);
		}
		else switch (field.getType())
		{
		case Field::INT:
			tuple.set(i, (int)((LONGLONG*)v)->number);
			break;
		case Field::SMALLINT:
			tuple.set(i, (short)((LONGLONG*)v)->number);
			break;
		case Field::BIGINT:
			tuple.set(i, (long)((LONGLONG*)v)->number);
			break;
		case Field::REAL:
			tuple.set(i, (float)((DOUBLE*)v)->number);
			break;
		case Field::DOUBLE:
			tuple.set(i, (double)((DOUBLE*)v)->number);
			break;
		case Field::BOOL:
			tuple.set(i, (bool)((BOOL*)v)->b);
			break;
		case Field::STRING:
			tuple.set(i, StrWrapper(((STRING*)v)->str));
			break;
		}
		i++;
	}
}
inline void insert_into_pool(Table* table, const vector<string>& fields, const vector<Node*>& values)
{
	// ������ȷ���ſ��Ե��ñ�����
	TuplePool& pool = table->getPool();
	Tuple tuple = pool.insert();
	const TableMeta& meta = table->getMeta();
	int i = 0, size = meta.getFields().size();
	// ���ָ���������У��� trueʱ��ʾ��ָ��������
	bool* tag = new bool[size] {false};
	for (Node* v : values) {
		int pos = meta.posOf(fields[i]);
		tag[pos] = true;
		if(v->type == (Node::NType)NULL) {
			tuple.setNull(pos, true);
		}
		else switch (meta.of(fields[i]).getType())
		{
		case Field::INT:
			tuple.set(pos, (int)((LONGLONG*)v)->number);
			break;
		case Field::SMALLINT:
			tuple.set(pos, (short)((LONGLONG*)v)->number);
			break;
		case Field::BIGINT:
			tuple.set(pos, (long)((LONGLONG*)v)->number);
			break;
		case Field::REAL:
			tuple.set(pos, (float)((DOUBLE*)v)->number);
			break;
		case Field::DOUBLE:
			tuple.set(pos, (double)((DOUBLE*)v)->number);
			break;
		case Field::BOOL:
			tuple.set(pos, (bool)((BOOL*)v)->b);
			break;
		case Field::STRING:
			tuple.set(pos, StrWrapper(((STRING*)v)->str));
			break;
		}
		i++;
	}
	// ����ûָ���������У�����Ϊnull
	for (i = 0; i < size; i++) {
		if (!tag[i]) {
			tuple.setNull(i, true);
		}
	}
	delete[] tag;
}
inline void insert_into_table_handle(Table* table, const vector<string>& fields, const vector<Node*>& values)
{
	for (Node* v : values) {
		if (v->type == Node::STAR) { // �ķ���Ʋ��ã�
			cerr << error << endl;
			return;
		}
	}

	const TableMeta& meta = table->getMeta();
	if (fields.empty()) { // û��ָ�������������
		const auto& fields = meta.getFields();
		if (!check_for_insert(fields, values))	// ������
			return;
		insert_into_pool(table->getPool(), fields, values);
	}
	else { // ָ���˾����������
		if (!check_for_insert(meta, fields, values))	// �ж������Ƿ�һһƥ��
			return;
		insert_into_pool(table, fields, values);
	}
	cout << "Query OK, 1 rows affected." << endl;
}
inline void insert_into_table_handle(const string& name, const vector<string>& fields, const vector<Node*>& values)
{
	Table* table = db[name];
	if (!table) // ������ָ���ı�
		cerr << "Table '" << name << "' doesn't exist." << endl;
	else
		insert_into_table_handle(table, fields, values);

	// free memory
	for (Node* v : values) {
		delete v;
	}
}

// ��ѯ
using select_expr_list = vector<pair<Node*, string>>;
using opt_groupby = pair<string, Node*>;
using opt_orderby = pair<string, bool>;
inline void select_count(Table* table, bool star, int pos)
{
	int count = 0;
	const auto& fields = table->getMeta().getFields();
	TuplePool& pool = table->getPool();
	while (true) { // TODO �Ż����ɱ���ά��һ��countֵ���Ͳ���ÿ�ζ�������
		try {
			Tuple tuple = pool.next();
			count++;
		}
		catch (out_of_range err) {
			break;
		}
	}
	cout << "COUNT(" << (star ? "*" : fields[pos].getName()) << ") " << count << endl;
}
inline void select_sum(Table* table, int pos)
{
	//const auto& fields = table->getMeta().getFields();
	//if (fields[pos].getType() == Field::STRING ||
	//	fields[pos].getType() == Field::BOOL) { // �޷���͵�����
	//	cerr << "�޷���͵�����" << endl;
	//	return;
	//}
	cerr << todo << "[SUM(..)]" << endl;
}
inline void select_avg(Table* table, int pos)
{
	//const auto& fields = table->getMeta().getFields();
	//if (fields[pos].getType() == Field::STRING ||
	//	fields[pos].getType() == Field::BOOL) { // �޷���͵�����
	//	cerr << "�޷���ƽ��������" << endl;
	//	return;
	//}
	cerr << todo << "[AVG(..)]" << endl;
}
inline void select_max(Table* table, int pos)
{
	cerr << todo << "[MAX(..)]" << endl;
}
inline void select_min(Table* table, int pos)
{
	cerr << todo << "[MIN(..)]" << endl;
}
inline bool check_first_row(Table* table, const select_expr_list& cols, const opt_groupby& groupby, bool& showAllFields)
{
	if (!groupby.first.empty()) { // TODO Ŀǰ����֧��group by
		cerr << todo << "[GROUP BY]" << endl;
		return false;
	}

	const TableMeta& meta = table->getMeta();
	for (const auto& col : cols) {
		Node* node = cols.front().first;
		if (node->type == Node::STAR) { // �����Ǻţ�size����Ϊ1
			showAllFields = true;
			break;
		}
		else if (node->isFunction()) {
			const string& id = ((Function*)(node))->id;
			if (id != "*" && !meta.exist(id)) { // xx(id)��idֻ�����Ǻţ������Ǵ��ڵ�������
				cerr << "Unknown column '" << id << "' in field list." << endl;
				return false;
			}
			else switch (node->type)
			{
			case Node::COUNT:
				select_count(table, id == "*", meta.posOf(id));
				return false; // !!! ��Ϊ�����������ÿ��ֻ�ܵ���һ���ۼ�����
			case Node::SUM:
				select_sum(table, meta.posOf(id));
				return false;
			case Node::AVG:
				select_avg(table, meta.posOf(id));
				return false;
			case Node::MAX:
				select_max(table, meta.posOf(id));
				return false;
			case Node::MIN:
				select_min(table, meta.posOf(id));
				return false;
			}
		}
		else if (node->type == Node::IDENTIFIER) {
			const string& name = ((ID*)node)->name;
			if (!meta.exist(name)) { // Ϲд��������
				cerr << "Unknown column '" << name << "' in field list." << endl;
				return false;
			}
		}
		else { // TODO Ŀǰ����֧����select..from֮����ֱ��ʽ
			cerr << todo << "[EXPR]" << endl;
			return false;
		}
	}
	return true;
}
inline void show_first_row(Table* table, const select_expr_list& cols) // not use
{
	const TableMeta& meta = table->getMeta();
	if (true) {
		for (const Field& field : meta.getFields()) {
			cout << field << ' ';
		}
		cout << endl;
		return;
	}
	for (const auto& col : cols) {
		if (!col.second.empty()) { // �б���
			cout << col.second << ' ';
			continue;
		}
		const string* name = nullptr;
		if (col.first->type == Node::IDENTIFIER) {
			name = &((ID*)(col.first))->name;
			cout << *name << ' ';
		}
		else if (col.first->isFunction()) {
			Function* fun = (Function*)(col.first);
			name = &(fun)->id;
			cout << fun->getFuncName() << '(' << *name << ") ";
		}
	}
	cout << endl;
}
inline void show_which_fields(const TableMeta& meta, vector<int>& pos, bool showAllFields, const select_expr_list& cols)
{
	const auto& fields = meta.getFields();
	pos.reserve(fields.size());
	if (showAllFields) { // ��ʾ�����ֶε����
		for (int i = 0; i < fields.size(); i++)
			pos.push_back(i);
	}
	else for (const auto& col : cols) { // ָ���˾����ֶε����
		ID* node = (ID*)(col.first);
		pos.push_back(meta.posOf(node->name));
	}
}
inline void show_all_tuple(Table* table, const vector<int> pos)
{
	TuplePool& pool = table->getPool();
	const auto& fields = table->getMeta().getFields();
	while (true) {
		try {
			Tuple tuple = pool.next();
			for (int i : pos) {
				if (tuple.isNull(i)) {
					cout << "NULL ";
				}
				else switch (fields[i].getType())
				{
				case Field::INT:
					cout << tuple.get<int>(i) << ' ';
					break;
				case Field::SMALLINT:
					cout << tuple.get<short>(i) << ' ';
					break;
				case Field::BIGINT:
					cout << tuple.get<long>(i) << ' ';
					break;
				case Field::REAL:
					cout << tuple.get<float>(i) << ' ';
					break;
				case Field::DOUBLE:
					cout << tuple.get<double>(i) << ' ';
					break;
				case Field::BOOL:
					cout << (tuple.get<bool>(i) ? "true" : "false") << ' ';
					break;
				case Field::STRING:
					cout << tuple.get(i) << ' ';
					break;
				}
			}
			cout << endl;
		}
		catch (out_of_range err) {
			return;
		}
	}
}
inline void select_all_from_table(Table* table, const select_expr_list& cols, bool showAllFields)
{
	// ��ӡ��ͷ TODO

	// ��ӡ����
	vector<int> pos; // ����Ҫ��ӡ�����Ե��±�
	show_which_fields(table->getMeta(), pos, showAllFields, cols);
	show_all_tuple(table, pos);
}
inline bool check_where_stmt(Table* table, Node* where, set<string>& vars)
{
	try {
		where->getVarList(vars);
	}
	catch (const char* err) { // ������where�Ӿ���ʹ�þۼ�����
		cerr << err << endl;
		return false;
	}
	const TableMeta& meta = table->getMeta();
	for (const string& var : vars) {
		if (var == "*") { // �ķ���Ʋ��ã�
			cerr << error << endl;
			return false;
		}
		if(!meta.exist(var)){ // Ϲд����
			cerr << "Unknown column '" << var << "' in field list." << endl;
			return false;
		}
	}
	return true;
}
inline void select_from_table(Table* table, const select_expr_list& cols, const set<string>& vars, Node* where, bool showAllFields)
{
	map<string, Const*> map;
	const TableMeta& meta = table->getMeta();
	const auto& fields = meta.getFields();
	TuplePool& pool = table->getPool();

	vector<int> vcr; // ����Ҫ��ӡ�����Ե��±�
	show_which_fields(meta, vcr, showAllFields, cols);

	while (true) {
		try {
			Tuple tuple = pool.next();
			for (const string& var : vars) {
				int pos = meta.posOf(var);
				if (tuple.isNull(pos)) {
					map[var] = new NULLConst;
				}
				else switch (fields[pos].getType())
				{
				case Field::INT:
					map[var] = new LONGLONG(tuple.get<int>(pos));
					break;
				case Field::SMALLINT:
					map[var] = new LONGLONG(tuple.get<short>(pos));
					break;
				case Field::BIGINT:
					map[var] = new LONGLONG(tuple.get<long>(pos));
					break;
				case Field::REAL:
					map[var] = new DOUBLE(tuple.get<float>(pos));
					break;
				case Field::DOUBLE:
					map[var] = new DOUBLE(tuple.get<double>(pos));
					break;
				case Field::BOOL:
					map[var] = new BOOL(tuple.get<bool>(pos));
					break;
				case Field::STRING:
					map[var] = new STRING(tuple.get(pos));
					break;
				}
			}
			bool right;
			try {
				right = where->evalBool(map);
				for (const auto& p : map) { // free memory TODO
					delete p.second;
				}
			}
			catch (const char* msg) {
				cerr << error << endl;
				for (const auto& p : map) { // free memory
					delete p.second;
				}
				return;
			}
			if (right) {
				for (int i : vcr) { // TODO ���ع�
					if (tuple.isNull(i)) {
						cout << "NULL ";
					}
					else switch (fields[i].getType())
					{
					case Field::INT:
						cout << tuple.get<int>(i) << ' ';
						break;
					case Field::SMALLINT:
						cout << tuple.get<short>(i) << ' ';
						break;
					case Field::BIGINT:
						cout << tuple.get<long>(i) << ' ';
						break;
					case Field::REAL:
						cout << tuple.get<float>(i) << ' ';
						break;
					case Field::DOUBLE:
						cout << tuple.get<double>(i) << ' ';
						break;
					case Field::BOOL:
						cout << (tuple.get<bool>(i) ? "true" : "false") << ' ';
						break;
					case Field::STRING:
						cout << tuple.get(i) << ' ';
						break;
					}
				}
				cout << endl;
			}
		}
		catch (out_of_range err) {
			return;
		}
	}
}
inline void select_from_table(Table* table, const select_expr_list& cols,
	Node* where, const opt_groupby& grouby, const opt_orderby& orderby)
{
	if (!orderby.first.empty()) { // TODO Ŀǰ��֧�� order by
		cerr << todo << "[ORDER BY]" << endl;
		return;
	}

	bool showAllFields = false; // ��ʾ�Ƿ���ʾ��������

	// ���select...from֮���Ƿ�����������оۼ������Ļ������Ѿ���ɽ�������
	if (!check_first_row(table, cols, grouby, showAllFields))
		return;

	if (!where) { // ȫ������
		select_all_from_table(table, cols, showAllFields);
		return; // !!!
	}

	// ���where�Ӿ����Ƿ����������
	set<string> vars;
	if (!check_where_stmt(table, where, vars))
		return;
	select_from_table(table, cols, vars, where, showAllFields);
}
inline void select_from_table_handle(
	bool showAll, const select_expr_list& fields, const string& tableName,
	Node* where, const opt_groupby& groupby, const opt_orderby& orderby)
{
	Table* table = db[tableName];
	if (!table) { // ������ָ���ı�
		cerr << "Table '" << tableName << "' doesn't exist." << endl;
	}
	else if (showAll) {
		select_from_table(table, fields, where, groupby, orderby);
	}
	else { // TODO distinct Ŀǰ��֧��ȥ����ʾ
		cout << todo << "[DISTINCT]" << endl;
	}

	// free memory
	for (const auto& field : fields) {
		delete field.first;
	}
	delete where;
	delete groupby.second;
}

// ����
using set_col_list = vector<pair<string, Node*>>;
inline bool check_set_stmt(Table* table, const set_col_list& cols)
{
	const TableMeta& meta = table->getMeta();
	for (const auto& col : cols) {
		const string& name = col.first;
		// Ϲд���Ե�
		if (!meta.exist(name)) {
			cerr << "Unknown column '" << name << "' in field list." << endl;
			return false;
		}
		Node* v = col.second;
		const Field& field = meta.of(name);
		// nullֵ��ͻ��
		if (v->type == (Node::NType)NULL && (field.isNotNull() || field.isPrimKey())) {
			cerr << "Column '" << field << "' cannot be null." << endl;
			return false;
		}
		// �ж�������ֵ�������Ƿ�һһƥ��
		if (!check_type(field, col.second)) {
			return false;
		}
	}
	return true;
}
inline void set_which_fields(const TableMeta& meta, const set_col_list& cols, vector<int>& pos)
{
	pos.reserve(cols.size());
	for (const auto& col : cols) {
		pos.push_back(meta.posOf(col.first));
	}
}
inline void update_tuples_of_table(Table* table, const set_col_list& cols)
{
	int count = 0;
	const TableMeta& meta = table->getMeta();
	const auto& fields = meta.getFields();
	TuplePool& pool = table->getPool();
	while (true)
	{
		try {
			Tuple tuple = pool.next();
			for (const auto& col : cols) {
				int pos = meta.posOf(col.first);
				Node* v = col.second;
				if (v->type == (Node::NType)(NULL)) {
					tuple.setNull(pos, true);
				}
				else {
					tuple.setNull(pos, false);
					switch (fields[pos].getType()) {
					case Field::INT:
						tuple.set(pos, (int)((LONGLONG*)v)->number);
						break;
					case Field::SMALLINT:
						tuple.set(pos, (short)((LONGLONG*)v)->number);
						break;
					case Field::BIGINT:
						tuple.set(pos, (long)((LONGLONG*)v)->number);
						break;
					case Field::REAL:
						tuple.set(pos, (float)((DOUBLE*)v)->number);
						break;
					case Field::DOUBLE:
						tuple.set(pos, (double)((DOUBLE*)v)->number);
						break;
					case Field::BOOL:
						tuple.set(pos, (bool)((BOOL*)v)->b);
						break;
					case Field::STRING:
						tuple.set(pos, StrWrapper(((STRING*)v)->str));
						break;
					}
				}
			}
			count++;
		}
		catch (out_of_range err) {
			break;
		}
	}
	cout << "Query OK, " << count <<" rows affected." << endl;
}
inline void update_tuples_of_table(Table* table, const set_col_list& cols, Node* where, const set<string>& vars)
{
	int count = 0;
	map<string, Const*> map;
	const TableMeta& meta = table->getMeta();
	const auto& fields = meta.getFields();
	TuplePool& pool = table->getPool();
	while (true)
	{
		try {
			Tuple tuple = pool.next();
			for (const string& var : vars) {
				int pos = meta.posOf(var);
				if (tuple.isNull(pos)) {
					map[var] = new NULLConst;
				}
				else switch (fields[pos].getType())
				{
				case Field::INT:
					map[var] = new LONGLONG(tuple.get<int>(pos));
					break;
				case Field::SMALLINT:
					map[var] = new LONGLONG(tuple.get<short>(pos));
					break;
				case Field::BIGINT:
					map[var] = new LONGLONG(tuple.get<long>(pos));
					break;
				case Field::REAL:
					map[var] = new DOUBLE(tuple.get<float>(pos));
					break;
				case Field::DOUBLE:
					map[var] = new DOUBLE(tuple.get<double>(pos));
					break;
				case Field::BOOL:
					map[var] = new BOOL(tuple.get<bool>(pos));
					break;
				case Field::STRING:
					map[var] = new STRING(tuple.get(pos));
					break;
				}
			}
			bool right;
			try {
				right = where->evalBool(map);
				for (const auto& p : map) { // free memory
					delete p.second;
				}
			}
			catch (const char* msg) {
				cerr << error << endl;
				for (const auto& p : map) { // free memory
					delete p.second;
				}
				return;
			}
			if (!right) {
				continue;
			}
			for (const auto& col : cols) {
				int pos = meta.posOf(col.first);
				Node* v = col.second;
				if (v->type == (Node::NType)(NULL)) {
					tuple.setNull(pos, true);
				}
				else {
					tuple.setNull(pos, false);
					switch (fields[pos].getType()) {
					case Field::INT:
						tuple.set(pos, (int)((LONGLONG*)v)->number);
						break;
					case Field::SMALLINT:
						tuple.set(pos, (short)((LONGLONG*)v)->number);
						break;
					case Field::BIGINT:
						tuple.set(pos, (long)((LONGLONG*)v)->number);
						break;
					case Field::REAL:
						tuple.set(pos, (float)((DOUBLE*)v)->number);
						break;
					case Field::DOUBLE:
						tuple.set(pos, (double)((DOUBLE*)v)->number);
						break;
					case Field::BOOL:
						tuple.set(pos, (bool)((BOOL*)v)->b);
						break;
					case Field::STRING:
						tuple.set(pos, StrWrapper(((STRING*)v)->str));
						break;
					}
				}
			}
			count++;
		}
		catch (out_of_range err) {
			break;
		}
	}
	cout << "Query OK, " << count << " rows affected." << endl;
}
inline void update_tuples_of_table(Table* table, const set_col_list& cols, Node* where)
{
	if (!check_set_stmt(table, cols)) { // ���set�Ӿ����Ƿ�������������
		return;
	}

	if (!where) { // û��where�Ӿ����������Ԫ��
		update_tuples_of_table(table, cols);
		return;
	}

	// ���where�Ӿ����Ƿ����������
	set<string> vars;
	if (!check_where_stmt(table, where, vars)) {
		return;
	}
	update_tuples_of_table(table, cols, where, vars);
}
inline void update_tuples_of_table(const string& name, const set_col_list& cols, Node* where)
{
	Table* table = db[name];
	if (!table) { // ������ָ���ı�
		cerr << "Table '" << name << "' doesn't exist." << endl;
	}
	else {
		update_tuples_of_table(table, cols, where);
	}

	// free memory
	for (const auto& col : cols) {
		delete col.second;
	}
	delete where;
}

// ɾ��
inline void delete_tuples_of_table(Table* table)
{
	table->getPool().removeAll();
	cout << "Delete all tuples OK." << endl;;
}
inline void delete_tuples_of_table(Table* table, Node* where, const set<string>& vars)
{
	map<string, Const*> map;
	const TableMeta& meta = table->getMeta();
	const auto& fields = meta.getFields();
	TuplePool& pool = table->getPool();
	int count = 0;
	while (true)
	{
		try {
			Tuple tuple = pool.next();
			for (const string& var : vars) {
				int pos = meta.posOf(var);
				if (tuple.isNull(pos)) {
					map[var] = new NULLConst;
				}
				else switch (fields[pos].getType())
				{
				case Field::INT:
					map[var] = new LONGLONG(tuple.get<int>(pos));
					break;
				case Field::SMALLINT:
					map[var] = new LONGLONG(tuple.get<short>(pos));
					break;
				case Field::BIGINT:
					map[var] = new LONGLONG(tuple.get<long>(pos));
					break;
				case Field::REAL:
					map[var] = new DOUBLE(tuple.get<float>(pos));
					break;
				case Field::DOUBLE:
					map[var] = new DOUBLE(tuple.get<double>(pos));
					break;
				case Field::BOOL:
					map[var] = new BOOL(tuple.get<bool>(pos));
					break;
				case Field::STRING:
					map[var] = new STRING(tuple.get(pos));
					break;
				}
			}
			bool right;
			try {
				right = where->evalBool(map);
				for (const auto& p : map) { // free memory
					delete p.second;
				}
			}
			catch (const char* msg) {
				cerr << error << endl;
				for (const auto& p : map) { // free memory
					delete p.second;
				}
				return;
			}
			if (right) {
				pool.remove();
				count++;
			}
		}
		catch (out_of_range err) {
			break;
		}
	}
	cout << "Delete OK, " <<  count << " rows affected." << endl;;
}
inline void delete_tuples_of_table(const string& name, Node* where)
{
	Table* table = db[name];
	if (!table) { // ������ָ���ı�
		cerr << "Table '" << name << "' doesn't exist." << endl;
	}
	else if (!where) { // û��where�Ӿ���������ʾɾ������Ԫ��
		delete_tuples_of_table(table);
	}
	else { // ��where�Ӿ�����
		set<string> vars;
		if (!check_where_stmt(table, where, vars)) { // ���where�Ӿ����Ƿ����������
			return;
		}
		delete_tuples_of_table(table, where, vars);
	}

	// free memory
	delete where;
}
#endif // !HANDLER_H
