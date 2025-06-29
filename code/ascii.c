#include "ascii.h"


bool32 is_newline(char c) { return (c == '\n'); }
bool32 is_crlf(char c) { return (c == '\r') || (c == '\n'); }
bool32 is_ascii_space(char c) { return (c == ' '); }
bool32 is_ascii_whitespace(char c) { return (c == ' ') || (c == '\t') || (c == '\r') || (c == '\n'); }
bool32 is_ascii_lowercase_alpha(char c) { return (c >= 'a') && (c <= 'z'); }
bool32 is_ascii_capital_alpha(char c) { return (c >= 'A') && (c <= 'Z'); }
bool32 is_ascii_alpha(char c) { return is_ascii_capital_alpha(c) || is_ascii_lowercase_alpha(c); }
bool32 is_ascii_digit(char c) { return (c >= '0') && (c <= '9'); }
bool32 is_ascii_hex(char c) { return is_ascii_digit(c) || (('a' <= c) && (c <= 'f')) || (('A' <= c) && (c <= 'F')); }
bool32 is_ascii_double_quote(char c) { return (c == '"'); }
