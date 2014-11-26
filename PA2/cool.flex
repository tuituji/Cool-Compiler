/*
 *  The scanner definition for COOL.
 */

/*
 *  Stuff enclosed in %{ %} in the first section is copied verbatim to the
 *  output, so headers and global definitions are placed here to be visible
 * to the code in the file.  Don't remove anything that was here initially
 */
%{
#include <cool-parse.h>
#include <stringtab.h>
#include <utilities.h>

/* The compiler assumes these identifiers. */
#define yylval cool_yylval
#define yylex  cool_yylex

/* Max size of string constants */
#define MAX_STR_CONST 1025
#define YY_NO_UNPUT   /* keep g++ happy */

extern FILE *fin; /* we read from this file */

/* define YY_INPUT so we read from the FILE fin:
 * This change makes it possible to use this scanner in
 * the Cool compiler.
 */
#undef YY_INPUT
#define YY_INPUT(buf,result,max_size) \
	if ( (result = fread( (char*)buf, sizeof(char), max_size, fin)) < 0) \
		YY_FATAL_ERROR( "read() in flex scanner failed");

char string_buf[MAX_STR_CONST]; /* to assemble string constants */
char *string_buf_ptr;

extern int curr_lineno;
extern int verbose_flag;

extern YYSTYPE cool_yylval;

unsigned int comment_level = 0;
// curr_lineno = 1;
/*
 *  Add Your own definitions here
 */
inline int string_buf_full(char* string_buf_ptr, char* string_buf)
{
	return (string_buf_ptr - string_buf >= MAX_STR_CONST)?1:0;
}

%}

/*
 * Define names for regular expressions here.
 */

DARROW     =>
ELSE       [Ee][Ll][Ss][Ee]
CLASS      Class|class|CLASS
IF         if
FI         fi|FI
IN         in
INHERITS   inherits|iNHerITs
LET        let|Let|lEt
LOOP       loop|LOOp
POOL       pool|PoOl
THEN       then|THEN|ThEn
WHILE      while|whIle
CASE       case
ESAC       e[Ss]a[Cc]
OF         of
NEW        new
ISVOID     isvoid|IsvoiD 
INT_CONST  [0-9]+
TRUE       t[Rr][Uu][Ee]
FALSE      f[Aa][Ll][Ss][Ee]

NEWLINE    \n

ASSIGN     <-
OTHER      [{};:,\.()<=\+\-~@\*/]
LE         <=
NOT        not|NOT

TYPEID     [A-Z][A-Za-z0-9_]*
OBJECTID   [a-z][A-Za-z0-9_]*

WHITESPACE [ \t\r]*

%x comment_line
%x comment_star
%x string_state
%x string_too_long
%x string_null
INVISIABLE_CHAR [\r\f\v]

LEFT_ASCII .*

%%

 /*
  *  Nested comments
  */


 /*
  *  The multiple-character operators.
  */
{DARROW}		{ return (DARROW); }
{ELSE} { return ELSE; }

{INT_CONST} {  
	cool_yylval.symbol = inttable.add_string(yytext);
	return INT_CONST;
}

{TRUE} {
	cool_yylval.boolean = 1; 
	return BOOL_CONST;
}

{FALSE} {
	cool_yylval.boolean = 0; 
	return BOOL_CONST;
}


{CLASS} { return CLASS; }
{FI} { return FI; }
{IF} { return IF; }
{IN} { return IN; }
{INHERITS} { return INHERITS; }
{LET} { return LET; }
{LOOP} {return LOOP; }
{POOL} {return POOL; }
{THEN} {return THEN; }
{WHILE} {return WHILE; }
{CASE} {return CASE; }
{ESAC} {return ESAC; }
{OF}  {return OF; }
{NEW} {return NEW; }
{ISVOID} {return ISVOID; }

{LE} {
	return LE;
}

{NOT} {
	return NOT;
}

{ASSIGN} {
	return ASSIGN;
}

{OTHER} {
	return (char)*yytext;
}

{TYPEID} {
	cool_yylval.symbol = idtable.add_string(yytext);
	return TYPEID;
}

{OBJECTID} {
	cool_yylval.symbol = idtable.add_string(yytext);
	return OBJECTID;
}



--   BEGIN(comment_line);
<comment_line>[^\n]*
<comment_line>\n  ++curr_lineno; BEGIN(INITIAL);
 

"(*" { 
	BEGIN(comment_star);
	++comment_level;
}
<comment_star>\(\* {
	++comment_level;
}
<comment_star>[^*\n\(]*  { }

<comment_star>\*\) {
	--comment_level;
	if(comment_level == 0) {
		BEGIN(INITIAL);
	}
}
<comment_star>"*"[^\*\)\n]*  { }
<comment_star>\n  {
	++curr_lineno;
}
<comment_star>\( { }
<comment_star><<EOF>>  {
	cool_yylval.error_msg = "EOF in comment";
	BEGIN(INITIAL);
	return ERROR;
}

"*)" {
	cool_yylval.error_msg = "Unmatched *)";
	return ERROR;
}

">" {
	cool_yylval.error_msg = yytext;
	return ERROR;
}

{INVISIABLE_CHAR} {;}

\" {   // to deal with string
	BEGIN(string_state); 
	string_buf_ptr = string_buf;
	string_buf[0] = '\0';
}

<string_state>\0 {
	BEGIN(string_null);
	//BEGIN(INITIAL);
	cool_yylval.error_msg = "String contains null character.";
	return ERROR;
}

<string_state>\\\0 {
	BEGIN(string_null);
//	BEGIN(INITIAL);
	cool_yylval.error_msg = "String contains null character.";
	return ERROR;
}

<string_state>\\[ntfb\\] {
	if(string_buf_full(string_buf_ptr + 1, string_buf)){
		BEGIN(string_too_long);
        string_buf[0] = '\0';
        string_buf_ptr = string_buf;
        cool_yylval.error_msg = "String constant too long";
        return ERROR;
	}
	switch(yytext[1]) {
		case 'n':
			*string_buf_ptr = '\n'; break;
		case 't':
			*string_buf_ptr = '\t'; break;
		case 'f':
			*string_buf_ptr = '\f'; break;
		case 'b':
			*string_buf_ptr = '\b'; break;
		case '\\':
			*string_buf_ptr = '\\'; break;
		default:
			break;	
	}
	*(++string_buf_ptr)= '\0';
}

<string_state>\\[0-9a-z] {
	if(string_buf_full(string_buf_ptr + 1, string_buf)){
		BEGIN(string_too_long);
        string_buf[0] = '\0';
        string_buf_ptr = string_buf;
        cool_yylval.error_msg = "String constant too long";
        return ERROR;
	}
	*string_buf_ptr = yytext[1];
	*(++string_buf_ptr)= '\0';
}


<string_state>\\[\\\t\b\f] {
	if(string_buf_full(string_buf_ptr + 1, string_buf)){
		BEGIN(string_too_long);
        string_buf[0] = '\0';
        string_buf_ptr = string_buf;
        cool_yylval.error_msg = "String constant too long";
        return ERROR;
	}
	*string_buf_ptr = yytext[1];
	*(++string_buf_ptr)= '\0';
}

<string_state>\\\\ {
	*string_buf_ptr = '\\';
	*(++string_buf_ptr)= '\0';
}

<string_state>\\- {
	*string_buf_ptr = '-';
	*(++string_buf_ptr)= '\0';
}

<string_state>\\\" {
	*string_buf_ptr = '\"';
	*(++string_buf_ptr)= '\0';
}

<string_state>[^\0\\\"\n]*  {
	if(string_buf_full(string_buf_ptr + strlen(yytext), string_buf)){
		BEGIN(string_too_long);
		string_buf[0] = '\0';
		string_buf_ptr = string_buf;
		cool_yylval.error_msg = "String constant too long";
		return ERROR;

	}
	memcpy(string_buf_ptr, yytext, strlen(yytext));
	string_buf_ptr += strlen(yytext);
	*string_buf_ptr = '\0';
}
<string_state>\"  { 
	cool_yylval.symbol = idtable.add_string(string_buf); 
	BEGIN(INITIAL); 
	return STR_CONST;
}
<string_state>\\\n {
	++curr_lineno;
	if(string_buf_full(string_buf_ptr + 1, string_buf)){
		BEGIN(string_too_long);
        string_buf[0] = '\0';
        string_buf_ptr = string_buf;
        cool_yylval.error_msg = "String constant too long";
        return ERROR;
	}
	*string_buf_ptr = '\n';
	*(++string_buf_ptr)= '\0';
}

<string_state>[^\\\"]?\n {
	cool_yylval.error_msg = "Unterminated string constant";
	++curr_lineno;
	BEGIN(INITIAL);
	return ERROR;
}

<string_state><<EOF>> {
	cool_yylval.error_msg = "EOF in string";
	BEGIN(INITIAL);
	return ERROR;
}

<string_too_long>[^\"]*  {  }
<string_too_long>\" {
	BEGIN(INITIAL);
}

<string_null>[^\"\n]* { }
<string_null>[\"\n] { 
	BEGIN(INITIAL);
}

{NEWLINE} { ++curr_lineno; }

{WHITESPACE} {;}

. {
	cool_yylval.error_msg = yytext;
	return ERROR;
}
 /*
  * Keywords are case-insensitive except for the values true and false,
  * which must begin with a lower-case letter.
  */


 /*
  *  String constants (C syntax)
  *  Escape sequence \c is accepted for all characters c. Except for 
  *  \n \t \b \f, the result is c.
  *
  */


%%
