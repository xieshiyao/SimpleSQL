%require "3.2"
%language "c++"

%define api.value.type variant
%define api.token.constructor

%define api.token.prefix {TOK_}
%token
	EOF 0 "end of file"
	PLUS	"+"
	MINUS	"-"
	STAR	"*"
	SLASH	"/"
	LP		"("
	RP		")"
	COMMA	","
	PERIOD	"."
	SEMI	";"
	ANDOP	"&&"
	OR		"||"
	E		"="
	GE		">="
	G		">"
	LE		"<="
	L		"<"
	NE		"!="
	/* NE		"<>" */
	NOT		"!"

%left "||"
%left "&&"
%nonassoc IS
%nonassoc "!" /* Why here??? */
%left BETWEEN
%left "=" "!=" "<>"
%left "<" ">" "<=" ">="
%left "+" "-"
%left "*" "/"
%nonassoc UMINUS

%token <long long> LONGLONG_CONST
%token <double> DOUBLE_CONST
%token <string> STRING_CONST ID
%token TRUE FALSE
%token NULL
//yy::parser::token::TOK_NULL

%token ADDOP AND ALL ALTER AS ASC BY CASCADE CLUSTER COLUMN CREATE DELETE DESC DISTINCT DROP EXCEPT EXISTS FOREIGN FROM GROUP HAVING IN INDEX INSERT INTERSECT INTO IS KEY LIKE ON ORDER PRIMARY REFERENCES RENAME RESTRICT SELECT SET SUM TABLE TO UNION UNIQUE UPDATE VALUES VIEW WHERE EXIT

%token INT BIGINT SMALLINT REAL BOOLEAN CHAR DOUBLEPRECISION

%token COUNT AVG MAX MIN

%code requires{
#include <string>
#include <utility>
#include <stdio.h>
#include <iostream>
#include "data_type.h"
#include "node.h"
#include "handler.h"
using namespace std;

#define DEBUG
#ifndef DEBUG
#define YY_(X) ""
#endif
}

%code provides{
	yy::parser::symbol_type yylex();
	char PS1[] = "SQL> ";
	char PS2[] = "---> ";
	bool printPS2 = false;
}
%%


stmt_list	: %empty			{ /* cout << PS1; */ }
			| stmt_list stmt 	{ /* cout << PS1; */ printPS2 = false; }
			| stmt_list error ";"	{
				cerr << "You have an error in your SQL syntax." << endl;
				/* cout << PS1; */
				printPS2 = false;
				yyerrok;
			}
			;

stmt		: select_stmt 
			| create_table_stmt 
			| drop_stmt 
			| insert_stmt 
			| update_stmt 
			| delete_stmt 
			| EXIT		{ return 0; }
			;


/* ----- select_stmt begin ----- */
select_stmt	: SELECT select_opts select_expr_list 
				FROM ID 
				opt_where
				opt_groupby
				opt_orderby ";" {
				select_from_table_handle($select_opts, $select_expr_list, 
					$ID, $opt_where,
					$opt_groupby, $opt_orderby);
			}
			;

%type <bool> select_opts;
select_opts	: %empty	{ $$ = true; }
			| ALL		{ $$ = true; }
			| DISTINCT	{ $$ = false; }
			;

%type <vector<pair<Node*,string>>> select_expr_list;
select_expr_list	: select_expr_list "," select_expr 	{
							if($1.back().first->type==Node::STAR ||
							$select_expr.first->type==Node::STAR)
								throw syntax_error("error: When you have written '*', you can't write any other things.");
							$$ = $1; $$.push_back($select_expr);
						}
					| select_expr						{ $$.push_back($select_expr); }
					;

%type <pair<Node*,string>> select_expr;
select_expr	: expr opt_as_alias	{ $$ = make_pair($expr, $opt_as_alias); }
			;

%type <string> opt_as_alias;
opt_as_alias	: %empty	{ $$ = ""; }
				| AS ID		{ $$ = $ID; }
				| ID		{ $$ = $ID; }
				;

%type <Node*> opt_where;
opt_where	: %empty		{ $$ = nullptr; }
			| WHERE expr	{ $$ = $expr; }
			;

%type <pair<string,Node*>> opt_groupby;
opt_groupby	: %empty					{ /* $$ = make_pair(string(""), nullptr); */ }
			| GROUP BY ID				{ $$ = make_pair($ID, nullptr); }
			| GROUP BY ID HAVING expr	{ $$ = make_pair($ID, $expr); }
			;

%type <pair<string,bool>> opt_orderby;
opt_orderby	: %empty					{ /* $$ = make_pair(string(""), false); */ }
			| ORDER BY ID opt_asc_desc	{ $$ = make_pair($ID, $opt_asc_desc); }
			;

%type <bool> opt_asc_desc;
opt_asc_desc	: %empty	{ $$ = true; }
				| ASC		{ $$ = true; }
				| DESC		{ $$ = false; }
				;
/* ----- select_stmt end ----- */


/* ----- delete_stmt begin ----- */
delete_stmt	: DELETE FROM ID opt_where ";"	{
				delete_tuples_of_table($ID, $opt_where);
			}
			;
/* ----- delete_stmt end ----- */


/* ----- insert_stmt begin ----- */
insert_stmt	: INSERT 
				INTO ID opt_col_names
				VALUES "(" insert_vals ")" ";"	{
					insert_into_table_handle($ID, $opt_col_names, $insert_vals);
			}
			;

%type <vector<string>> opt_col_names;
opt_col_names	: %empty	{}
			| "(" column_list ")"	{ $$ = $column_list; }
			;

%type <vector<string>> column_list;
column_list	: column_list "," ID	{ $$ = $1; $$.push_back($ID); }
			| ID					{ $$.push_back($ID); }
			;

%type <vector<Node*>> insert_vals;
insert_vals	: insert_vals "," constant	{ $$ = $1; $$.push_back($constant); }
			| constant					{ $$.push_back($constant); }
			;

/* ----- insert_stmt end ----- */


/* ----- update_stmt begin ----- */
update_stmt	: UPDATE ID
				SET set_col_list
				opt_where ";" {
				update_tuples_of_table($ID, $set_col_list, $opt_where);
			}
			;

%type <vector<pair<string,Node*>>> set_col_list;
set_col_list	: set_col_list "," set_col	{ $$ = $1; $$.push_back($set_col); }
				| set_col				{ $$.push_back($set_col); }
				;

%type <pair<string,Node*>> set_col;
set_col			: ID "=" constant	{ $$ = make_pair($ID,$constant); }
				;

/* ----- update_stmt end ----- */


/* ----- create_table_stmt begin ----- */
create_table_stmt	: CREATE TABLE ID "(" create_col_list[col_list] ")" ";" {
						create_table_handle($ID, $col_list);
					}
					;

%type <vector<Data_def>> create_col_list;
create_col_list		: create_col_list "," create_definition	{ $$ = $1; $$.push_back($3); }
					| create_definition						{ $$.push_back($1); }
					;

%type <Data_def> create_definition;
create_definition	: ID data_type column_attrs	{ $$ = Data_def($1, $2, $3); }
					;

%type <Data_type> data_type;
data_type			: INT			{ $$ = Data_type(Data_type::DType::INT); }
					| BIGINT		{ $$ = Data_type(Data_type::DType::BIGINT); }
					| SMALLINT		{ $$ = Data_type(Data_type::DType::SMALLINT); }
					| REAL			{ $$ = Data_type(Data_type::DType::REAL); }
					| BOOLEAN		{ $$ = Data_type(Data_type::DType::BOOLEAN); }
					| CHAR "(" LONGLONG_CONST ")"	{ $$ = Data_type(Data_type::DType::CHAR, $3); }
					| DOUBLEPRECISION				{ $$ = Data_type(Data_type::DType::DOUBLEPRECISION); }
					;

%type <unsigned char> column_attrs;
column_attrs		: column_attrs column_attr	{ $$ = $1 | $2; }
					| %empty					{ $$ = 0; }
					;

%type <unsigned char> column_attr;
column_attr			: NOT NULL		{ $$ = 0x1; }
					| PRIMARY KEY	{ $$ = 0x2; }
					| UNIQUE		{ $$ = 0x4; }
					;
/* ----- create_table_stmt end ----- */


/* ----- drop_stmt begin ----- */
drop_stmt	: DROP TABLE ID ";" /* opt_restrict_cascade */	{ 
				drop_table_handle($ID);
			}
			;

		/* TODO */
//opt_restrict_cascade	: %empty
//						| RESTRICT
//						| CASCADE
//						;
/* ----- drop_stmt end ----- */


%type <Node*> constant;
constant	: LONGLONG_CONST	{ $$ = new LONGLONG($1); }
			| DOUBLE_CONST		{ $$ = new DOUBLE($1); }
			| STRING_CONST		{ $$ = new STRING($1); }
			| TRUE				{ $$ = new BOOL(true); }
			| FALSE				{ $$ = new BOOL(false); }
			| NULL				{ $$ = new NULLConst(); }
			| "*"				{ $$ = new Node(Node::STAR); }
			;

%type <Node*> expr;
expr		: constant		/* $$ = $1 */
			;

expr		: expr "+" expr			{ $$ = new BiOP(Node::NType::PLUS, $1, $3); }
			| expr "-" expr			{ $$ = new BiOP(Node::NType::MINUS, $1, $3); }
			| expr "*" expr			{ $$ = new BiOP(Node::NType::MULTIPLY, $1, $3); }
			| expr "/" expr			{ $$ = new BiOP(Node::NType::DIVIDE, $1, $3); }
			| "-" expr %prec UMINUS	{ $$ = new UnOP(Node::NType::UMINUS, $2); }
			| "(" expr ")"			{ $$ = $2; }
			;

expr		: expr "||" expr	{ $$ = new BiOP(Node::NType::OR, $1, $3); }
			| expr "&&" expr	{ $$ = new BiOP(Node::NType::ANDOP, $1, $3); }
			| NOT expr			{ $$ = new UnOP(Node::NType::NOT, $2); }
			| expr comp expr	%prec ">"	{ $$ = new BiOP($comp, $1, $3); }
			;

expr		: expr IS NULL		{ $$ = new UnOP(Node::NType::ISNULL, $1); }
			| expr IS NOT NULL	{ $$ = new UnOP(Node::NType::ISNOTNULL, $1); }
			;

expr		: expr BETWEEN expr AND expr %prec BETWEEN	{ $$ = new BETWEEN($1, $3, $5); }
			;

expr		: COUNT "(" "*" ")"		{ $$ = new Function(Node::NType::COUNT, "*"); }
			| COUNT "(" ID ")"		{ $$ = new Function(Node::NType::COUNT, $ID); }
			| SUM "(" ID ")"		{ $$ = new Function(Node::NType::SUM, $ID); }
			| AVG "(" ID ")"		{ $$ = new Function(Node::NType::AVG, $ID); }
			| MAX "(" ID ")"		{ $$ = new Function(Node::NType::MAX, $ID); }
			| MIN "(" ID ")"		{ $$ = new Function(Node::NType::MIN, $ID); }
			| ID					{ $$ = new ID($ID); }
			;

%type <Node::NType> comp;
comp		: "="	{ $$ = Node::NType::E; }
			| ">="	{ $$ = Node::NType::GE; }
			| ">"	{ $$ = Node::NType::G; }
			| "<="	{ $$ = Node::NType::LE; }
			| "<"	{ $$ = Node::NType::L; }
			| "!="	{ $$ = Node::NType::NE; }
			| "<>"	{ $$ = Node::NType::NE; }
			;


%%
#include"lex.c"
int main()
{
	yy::parser parse;
	return parse();
}

void yy::parser::error(const string& msg)
{
	if(msg.length() == 0)
		return;
	cerr << msg << '\n';
}

// write your function definition here
