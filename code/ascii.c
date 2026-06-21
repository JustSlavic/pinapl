#include "ascii.h"


bool ascii_is_newline(char c) { return (c == '\n'); }
bool ascii_is_crlf(char c) { return (c == '\r') || (c == '\n'); }
bool ascii_is_space(char c) { return (c == ' '); }
bool ascii_is_whitespace(char c) { return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'); }
bool ascii_is_lowercase_alpha(char c) { return (c >= 'a') && (c <= 'z'); }
bool ascii_is_capital_alpha(char c) { return (c >= 'A') && (c <= 'Z'); }
bool ascii_is_alpha(char c) { return ascii_is_capital_alpha(c) || ascii_is_lowercase_alpha(c); }
bool ascii_is_digit(char c) { return (c >= '0') && (c <= '9'); }
bool ascii_is_hex(char c) { return ascii_is_digit(c) || (('a' <= c) && (c <= 'f')) || (('A' <= c) && (c <= 'F')); }
bool ascii_is_double_quote(char c) { return (c == '"'); }
