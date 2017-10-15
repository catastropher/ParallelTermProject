#include <string>

enum TokenType
{
    TOK_TITLE,
    TOK_BEGIN,
    TOK_END,
    TOK_VAR,
    TOK_INT,
    TOK_LIST,
    TOK_LSQUARE_BRACKET,
    TOK_RSQUARE_BRACKET,
    TOK_REM,
    TOK_LABEL,
    TOK_LET,
    TOK_IF,
    TOK_THEN,
    TOK_GOTO,
    TOK_INPUT,
    TOK_PRINT,
    TOK_PROMPT,
    TOK_EQ,
    TOK_NE,
    TOK_LT,
    TOK_GT,
    TOK_LE,
    TOK_GE,
    TOK_ADD,
    TOK_SUB,
    TOK_MUL,
    TOK_DIV,
    TOK_MOD,
    TOK_NUMBER,
    TOK_ID
};

struct Token
{
    Token(TokenType type_, std::string value_) : type(type_), value(value_) { }
    
    TokenType type;
    std::string value;
};
