%{

/* Declarations section */
#include <stdio.h>
#include "source.hpp"
#include "parser.tab.hpp"
#include "hw3_output.hpp"

char handleAscii(char first, char sec);

extern int yylineno;
extern char* yytext;
extern int yyleng;
extern int yylex();
extern char textbuff[1024];
extern char* textbuffptr;
extern std::vector<Node*> allocatedNodes;


%}

%option yylineno
%option noyywrap
digit   		([0-9])
nonzero         ([1-9])
letter  		([a-zA-Z])
whitespace		([\t\n\r ])
string          (["])
relational      ((\<=)|(\>=)|(\<)|(\>))
equal           ((==)|(!=))
binop           ((\+)|(\-))
mult            ((\*)|(\/))
backslash       \x5C
printableascii  ([\x20-\x21\x23-\x5B\x5D-\x7E])
comment \/\/[^\n\r]*

%x STRINGS
%x ESCAPESEQ
%x ASCII
%%

{whitespace}				;
"int"                       {yylval = new Node(yylineno, "INT");allocatedNodes.push_back(yylval); return INT;}
"byte"                      {yylval = new Node(yylineno, "BYTE");allocatedNodes.push_back(yylval);return BYTE;}
"b"                         {yylval = new Node(yylineno, "B");allocatedNodes.push_back(yylval); return B;}
"bool"                      {yylval = new Node(yylineno, "BOOL");allocatedNodes.push_back(yylval);return BOOL;}
"and"                       {yylval = new Node(yylineno, yytext);allocatedNodes.push_back(yylval); return AND;}
"or"                        {yylval = new Node(yylineno, yytext);allocatedNodes.push_back(yylval); return OR;}
"not"                       {yylval = new Node(yylineno, "BOOL");allocatedNodes.push_back(yylval); return NOT;}
"true"                      {yylval = new Node(yylineno, "BOOL");allocatedNodes.push_back(yylval); return TRUE;}
"false"                     {yylval = new Node(yylineno, "BOOL");allocatedNodes.push_back(yylval); return FALSE;}
"return"                    {yylval = new Node(yylineno, "RETURN");allocatedNodes.push_back(yylval); return RETURN;}
"if"                        {yylval = new Node(yylineno, "IF");allocatedNodes.push_back(yylval); return IF;}
"else"                      {yylval = new Node(yylineno, "ELSE");allocatedNodes.push_back(yylval); return ELSE;}
"while"                     {yylval = new Node(yylineno, "WHILE");allocatedNodes.push_back(yylval); return WHILE;}
"break"                     {yylval = new Node(yylineno, "BREAK");allocatedNodes.push_back(yylval); return BREAK;}
"continue"                  {yylval = new Node(yylineno, "CONTINUE");allocatedNodes.push_back(yylval); return CONTINUE;}
";"                         {yylval = new Node(yylineno, "SC");allocatedNodes.push_back(yylval); return SC;}
"("                         {yylval = new Node(yylineno, "LPAREN");allocatedNodes.push_back(yylval); return LPAREN;}
")"                         {yylval = new Node(yylineno, "RPAREN");allocatedNodes.push_back(yylval); return RPAREN;}
"{"                         {yylval = new Node(yylineno, "LBRACE");allocatedNodes.push_back(yylval); return LBRACE;}
"}"                         {yylval = new Node(yylineno, "RBRACE");allocatedNodes.push_back(yylval); return RBRACE;}
"="                         {yylval = new Node(yylineno, "ASSIGN");allocatedNodes.push_back(yylval); return ASSIGN;}
{relational}                {yylval = new Node(yylineno, yytext);allocatedNodes.push_back(yylval); return RELATIONAL;}
{equal}                     {yylval = new Node(yylineno, yytext);allocatedNodes.push_back(yylval); return EQUAL;}
{binop}                     {yylval = new Node(yylineno, yytext);allocatedNodes.push_back(yylval); return BINOP;}
{mult}                      {yylval = new Node(yylineno, yytext);allocatedNodes.push_back(yylval); return MULT;}
{letter}+({letter}|{digit})*		{yylval = new IdNode(yylineno, yytext, "", "");allocatedNodes.push_back(yylval); return ID;}
"0"                         {yylval = new NumNode(yylineno, stoi(yytext), "INT", "");allocatedNodes.push_back(yylval); return NUM;}
{nonzero}{digit}*          	{yylval = new NumNode(yylineno, stoi(yytext), "INT", "");allocatedNodes.push_back(yylval); return NUM;}
{comment}                   ;
(\"([^\n\r\"\\]|\\[rnt"\\])+\")     {yylval = new StringNode(yylineno, yytext, "STRING", "");allocatedNodes.push_back(yylval); return STRING;}
.                                   {output::errorLex(yylineno); exit(0);};

%%