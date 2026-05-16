#ifndef LEXER_H
#define LEXER_H

#include <stddef.h>  /* size_t */

typedef enum {

    /* Literals */
    TOK_INT_LIT,        /* 42, 0, 1000        */
    TOK_BOOL_LIT,       /* true, false         */
    TOK_IDENT,          /* foo, myVar, x       */

    /* Keywords */
    TOK_FN,             /* fn                  */
    TOK_LET,            /* let                 */
    TOK_IF,             /* if                  */
    TOK_ELSE,           /* else                */
    TOK_WHILE,          /* while               */
    TOK_RETURN,         /* return              */

    /* Type keywords */
    TOK_I64,            /* i64                 */
    TOK_BOOL,           /* bool                */

    /* Arithmetic operators */
    TOK_PLUS,           /* +                   */
    TOK_MINUS,          /* -                   */
    TOK_STAR,           /* *                   */
    TOK_SLASH,          /* /                   */

    /* Comparison operators */
    TOK_EQ,             /* ==                  */
    TOK_NEQ,            /* !=                  */
    TOK_LT,             /* <                   */
    TOK_GT,             /* >                   */
    TOK_LEQ,            /* <=                  */
    TOK_GEQ,            /* >=                  */

    /* Assignment */
    TOK_ASSIGN,         /* =                   */

    /* Logical operators */
    TOK_AND,            /* &&                  */
    TOK_OR,             /* ||                  */
    TOK_BANG,           /* !                   */

    /* Delimiters */
    TOK_LPAREN,         /* (                   */
    TOK_RPAREN,         /* )                   */
    TOK_LBRACE,         /* {                   */
    TOK_RBRACE,         /* }                   */
    TOK_COMMA,          /* ,                   */
    TOK_SEMICOLON,      /* ;                   */
    TOK_COLON,          /* :                   */
    TOK_ARROW,          /* ->                  */

    /* Special */
    TOK_EOF,            /* end of input        */
    TOK_ERROR           /* unrecognised char   */

} TokenType;


typedef struct {
    TokenType   type;
    const char *start;  /* pointer into source buffer */
    size_t      len;    /* byte length of lexeme      */
    int         line;
    int         col;
} Token;


typedef struct {
    const char *src;        /* full source text (not owned)  */
    size_t      src_len;    /* byte length of source         */
    size_t      pos;        /* current scan position         */
    int         line;       /* 1-based current line          */
    int         col;        /* 1-based current column        */
} Lexer;


//PUBLIC API

void  lexer_init(Lexer *l, const char *src, size_t src_len);

/*
 * Scan and return the next Token.
 * Whitespace and line comments (//) are skipped silently.
 * Returns TOK_EOF once the source is exhausted.
 * Returns TOK_ERROR for unrecognised input.
 */
Token lexer_next_token(Lexer *l);

/*
 * Return a human-readable name for a TokenType.
 * Useful for error messages and debug dumps.
 * The returned string is a string literal — do not free it.
 */
const char *token_type_name(TokenType t);

#endif 
