#ifndef PINAPL_IMPARSER_H_
#define PINAPL_IMPARSER_H_

#include <base.h>
#include <lexer.h>


void imparser_init(lexer *l);

bool imparser_paren_open(token *);
bool imparser_paren_close(token *);
bool imparser_bracket_open(token *);
bool imparser_bracket_close(token *);
bool imparser_brace_open(token *);
bool imparser_brace_close(token *);
bool imparser_colon(token *);
bool imparser_semicolon(token *);
bool imparser_plus(token *);
bool imparser_minus(token *);
bool imparser_asterics(token *);
bool imparser_slash(token *);
bool imparser_comma(token *);
bool imparser_period(token *);
bool imparser_equals(token *);
bool imparser_more(token *);
bool imparser_less(token *);

bool imparser_identifier(token *t);
bool imparser_keyword(token *t);
bool imparser_integer(token *t);


#endif /* PINAPL_IMPARSER_H_ */
