%{
#define YY_DECL \
	yy::parser::symbol_type yylex()
%}
%option noyywrap nodefault case-insensitive
%s BTWMODE
%{
#include <ctype.h>
#include <stdlib.h>
#define Function _Function
	#include <readline/readline.h>
	#include <readline/history.h>
#undef Function
#define YY_INPUT(buf,result,max_size) result = mygetinput(buf, max_size);

static int mygetinput(char *buf, int size) 
{
	//printf("in mygetinput\n");
	char *line;
	if (feof(yyin))  
		return YY_NULL;
	char*prompt = printPS2 ? PS2 : PS1;
	printPS2 = true;
	line = readline(prompt);
	if(!line)        
		return YY_NULL;
	if(strlen(line) > size-2)
	{
		fprintf(stderr,"input line too long\n");
		return YY_NULL; 
	}
	sprintf(buf,"%s\n",line);
	add_history(line);
	free(line);
	//printf("strlen(buf): %d\n",strlen(buf));
	return strlen(buf);
}
//int oldstate;
%}
	/*	delim	[ \t\n]
		ws		{delim}+ */
letter		[A-Za-z_]
digit		[0-9]
id			{letter}({letter}|{digit})*
number		-?{digit}+
float		-?{digit}+"."{digit}*|-?"."{digit}+
string		'[^'\n]*'
badstring	'[^'\n]*$
%%
	/* integers and floats */
{number}		return yy::parser::make_LONGLONG_CONST(stoll(yytext));
{float}			return yy::parser::make_DOUBLE_CONST(stod(yytext));
	/* booleans */
TRUE			return yy::parser::make_TRUE();
FALSE			return yy::parser::make_FALSE();
	/* strings */
{string}		yytext[yyleng-1]='\0';return yy::parser::make_STRING_CONST(yytext+1);
{badstring}		throw yy::parser::syntax_error("'\\n' in string. Please type ';' to end and restart.");	/* newline(\n) in string */
	/* operators */
"+"				return yy::parser::make_PLUS();
"-"				return yy::parser::make_MINUS();
"*"				return yy::parser::make_STAR();
"/"				return yy::parser::make_SLASH();
"("				return yy::parser::make_LP();
")"				return yy::parser::make_RP();
"!"	/* NOT */	return yy::parser::make_NOT();
","				return yy::parser::make_COMMA();
"."				return yy::parser::make_PERIOD();
";"				return yy::parser::make_SEMI();
"&&"			return yy::parser::make_ANDOP();
"||"			return yy::parser::make_OR();
"="				return yy::parser::make_E();
">="			return yy::parser::make_GE();
">"				return yy::parser::make_G();
"<="			return yy::parser::make_LE();
"<"				return yy::parser::make_L();
"!="			return yy::parser::make_NE();
"<>" 			return yy::parser::make_NE();
	/* functions */
COUNT		return yy::parser::make_COUNT();
AVG			return yy::parser::make_AVG();
MAX			return yy::parser::make_MAX();
MIN			return yy::parser::make_MIN();

	/* comments */
#.*					/* ignores */
"--"[ \t].*			/* ignores */

	/* {ws}	*/ /* whitespaces */ /* ignores */
[ \t]+			/* ignores */
[\n]			{ /* printPS2 && cout << PS2; printPS2 = true; */ }

<<EOF>>			return yy::parser::make_EOF();

	/* keywords */
<BTWMODE>AND	BEGIN(INITIAL); return yy::parser::make_AND();
ADD				return yy::parser::make_ADDOP();
ALL				return yy::parser::make_ALL();
ALTER			return yy::parser::make_ALTER();
AND				return yy::parser::make_ANDOP();
ASC				return yy::parser::make_ASC();
AS				return yy::parser::make_AS();
BETWEEN			BEGIN(BTWMODE); return yy::parser::make_BETWEEN();
BIGINT			return yy::parser::make_BIGINT();
BOOLEAN			return yy::parser::make_BOOLEAN();
BY				return yy::parser::make_BY();
CASCADE			return yy::parser::make_CASCADE();
CHAR			return yy::parser::make_CHAR();
CLUSTER			return yy::parser::make_CLUSTER();
COLUMN			return yy::parser::make_COLUMN();
CREATE			return yy::parser::make_CREATE();
DELETE			return yy::parser::make_DELETE();
DESC			return yy::parser::make_DESC();
DISTINCT		return yy::parser::make_DISTINCT();
DOUBLE[ \t\n]+PRECISION		return yy::parser::make_DOUBLEPRECISION();
DROP			return yy::parser::make_DROP();
EXCEPT			return yy::parser::make_EXCEPT();
EXISTS			return yy::parser::make_EXISTS();
EXIT			return yy::parser::make_EXIT();
FOREIGN			return yy::parser::make_FOREIGN();
FROM			return yy::parser::make_FROM();
GROUP			return yy::parser::make_GROUP();
HAVING			return yy::parser::make_HAVING();
INDEX			return yy::parser::make_INDEX();
IN				return yy::parser::make_IN();		
INSERT			return yy::parser::make_INSERT();
INTERSECT		return yy::parser::make_INTERSECT();
INTO			return yy::parser::make_INTO();
INT(TEGER)?		return yy::parser::make_INT();
IS				return yy::parser::make_IS();
KEY				return yy::parser::make_KEY();
LIKE			return yy::parser::make_LIKE();
NOT				return yy::parser::make_NOT();
NULL			return yy::parser::make_NULL();
ON				return yy::parser::make_ON();
ORDER			return yy::parser::make_ORDER();
OR				return yy::parser::make_OR();
PRIMARY			return yy::parser::make_PRIMARY();
REAL			return yy::parser::make_REAL();
REFERENCES		return yy::parser::make_REFERENCES();
RENAME			return yy::parser::make_RENAME();
RESTRICT		return yy::parser::make_RESTRICT();
SELECT			return yy::parser::make_SELECT();
SET				return yy::parser::make_SET();
SMALLINT		return yy::parser::make_SMALLINT();
SUM				return yy::parser::make_SUM();
TABLE			return yy::parser::make_TABLE();
TO				return yy::parser::make_TO();
UNION			return yy::parser::make_UNION();
UNIQUE			return yy::parser::make_UNIQUE();
UPDATE			return yy::parser::make_UPDATE();
VALUES			return yy::parser::make_VALUES();
VIEW			return yy::parser::make_VIEW();
WHERE			return yy::parser::make_WHERE();

	/* identifiers */
{id}	{
			for(int i = 0; i < yyleng; i++)
				yytext[i] = toupper(yytext[i]);
			return yy::parser::make_ID(yytext);
}

	/* .	throw yy::parser::syntax_error("invalid char: " + std::string(yytext)); */
.		throw yy::parser::syntax_error("'" + std::string(yytext) + "' is invalid. Please type ';' to end and restart.");

