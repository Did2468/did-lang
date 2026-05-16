#include "lexer.h"

#include <string.h>   
#include <ctype.h>   

typedef struct {
    const char *word;
    TokenType   type;
} Keyword;

static const Keyword KEYWORDS[] = {
    { "fn",     TOK_FN     },
    { "let",    TOK_LET    },
    { "if",     TOK_IF     },
    { "else",   TOK_ELSE   },
    { "while",  TOK_WHILE  },
    { "return", TOK_RETURN },
    { "true",   TOK_BOOL_LIT },
    { "false",  TOK_BOOL_LIT },
    { "i64",    TOK_I64    },
    { "bool",   TOK_BOOL   },
};

#define KEYWORD_COUNT (sizeof(KEYWORDS) / sizeof(KEYWORDS[0]))


/* Is the lexer at end of input? */
static int is_at_end(const Lexer *l) {
    return l->pos >= l->src_len;
}

/* Current character without advancing. */
static char peek(const Lexer *l) {
    if (is_at_end(l)) return '\0';
    return l->src[l->pos];
}

/* One character ahead without advancing. */
static char peek_next(const Lexer *l) {
    if (l->pos + 1 >= l->src_len) return '\0';
    return l->src[l->pos + 1];
}

/*
  Advance pos by one, maintaining line/col.
  Returns the character that was consumed.
 */
static char advance(Lexer *l) {
    char c = l->src[l->pos++];
    if (c == '\n') {
        l->line++;
        l->col = 1;
    } else {
        l->col++;
    }
    return c;
}

/*
  Skip all whitespace and // comments.
  Called at the top of lexer_next_token.
 */
static void skip_whitespace_and_comments(Lexer *l) {
    for (;;) {
        while (!is_at_end(l) && isspace((unsigned char)peek(l))) {
            advance(l);
        }

        if (!is_at_end(l) && peek(l) == '/' && peek_next(l) == '/') {
            while (!is_at_end(l) && peek(l) != '\n') {
                advance(l);
            }
            continue;  /* loop back to skip the newline itself */
        }

        break;
    }
}

/*
  Build a token of the given type whose lexeme
  spans [start_pos, l->pos).
  `start_line` and `start_col` are the position
  of the first character of the lexeme.
 */
static Token make_token(const Lexer *l, TokenType type,
                        size_t start_pos,
                        int start_line, int start_col)
{
    Token t;
    t.type  = type;
    t.start = l->src + start_pos;
    t.len   = l->pos - start_pos;
    t.line  = start_line;
    t.col   = start_col;
    return t;
}

/*
  Scan an identifier or keyword.
  Called when peek() is alpha or '_'.
 */
static Token scan_ident_or_keyword(Lexer *l) {
    size_t start_pos  = l->pos;
    int    start_line = l->line;
    int    start_col  = l->col;

    while (!is_at_end(l) && (isalnum((unsigned char)peek(l)) || peek(l) == '_')) {
        advance(l);
    }

    /* Build a preliminary IDENT token. */
    Token t = make_token(l, TOK_IDENT, start_pos, start_line, start_col);

    /* Check against the keyword table. */
    for (size_t i = 0; i < KEYWORD_COUNT; i++) {
        size_t klen = strlen(KEYWORDS[i].word);
        if (klen == t.len && strncmp(KEYWORDS[i].word, t.start, t.len) == 0) {
            t.type = KEYWORDS[i].type;
            break;
        }
    }

    return t;
}

/*
  Scan an integer literal.
  Called when peek() is a digit.
  Only decimal integers for now.
 */
static Token scan_integer(Lexer *l) {
    size_t start_pos  = l->pos;
    int    start_line = l->line;
    int    start_col  = l->col;

    while (!is_at_end(l) && isdigit((unsigned char)peek(l))) {
        advance(l);
    }

    return make_token(l, TOK_INT_LIT, start_pos, start_line, start_col);
}



void lexer_init(Lexer *l, const char *src, size_t src_len) {
    l->src     = src;
    l->src_len = src_len;
    l->pos     = 0;
    l->line    = 1;
    l->col     = 1;
}

Token lexer_next_token(Lexer *l) {
    skip_whitespace_and_comments(l);

    /* Snapshot position before consuming anything. */
    size_t start_pos  = l->pos;
    int    start_line = l->line;
    int    start_col  = l->col;

    if (is_at_end(l)) {
        return make_token(l, TOK_EOF, start_pos, start_line, start_col);
    }

    char c = advance(l);

    /* ── Identifiers and keywords ── */
    if (isalpha((unsigned char)c) || c == '_') {
        l->pos--;
        l->col--;
        return scan_ident_or_keyword(l);
    }

    /* ── Integer literals ── */
    if (isdigit((unsigned char)c)) {
        l->pos--;
        l->col--;
        return scan_integer(l);
    }

    switch (c) {

        /* Single-character: unambiguous */
        case '(': return make_token(l, TOK_LPAREN,    start_pos, start_line, start_col);
        case ')': return make_token(l, TOK_RPAREN,    start_pos, start_line, start_col);
        case '{': return make_token(l, TOK_LBRACE,    start_pos, start_line, start_col);
        case '}': return make_token(l, TOK_RBRACE,    start_pos, start_line, start_col);
        case ',': return make_token(l, TOK_COMMA,     start_pos, start_line, start_col);
        case ';': return make_token(l, TOK_SEMICOLON, start_pos, start_line, start_col);
        case ':': return make_token(l, TOK_COLON,     start_pos, start_line, start_col);
        case '*': return make_token(l, TOK_STAR,      start_pos, start_line, start_col);
        case '+': return make_token(l, TOK_PLUS,      start_pos, start_line, start_col);
        case '/': return make_token(l, TOK_SLASH,     start_pos, start_line, start_col);

        /* '!' or '!=' */
        case '!':
            if (peek(l) == '=') { advance(l); return make_token(l, TOK_NEQ,  start_pos, start_line, start_col); }
            return make_token(l, TOK_BANG, start_pos, start_line, start_col);

        /* '=' or '==' */
        case '=':
            if (peek(l) == '=') { advance(l); return make_token(l, TOK_EQ,     start_pos, start_line, start_col); }
            return make_token(l, TOK_ASSIGN, start_pos, start_line, start_col);

        /* '<' or '<=' */
        case '<':
            if (peek(l) == '=') { advance(l); return make_token(l, TOK_LEQ, start_pos, start_line, start_col); }
            return make_token(l, TOK_LT, start_pos, start_line, start_col);

        /* '>' or '>=' */
        case '>':
            if (peek(l) == '=') { advance(l); return make_token(l, TOK_GEQ, start_pos, start_line, start_col); }
            return make_token(l, TOK_GT, start_pos, start_line, start_col);

        /* '-' or '->' */
        case '-':
            if (peek(l) == '>') { advance(l); return make_token(l, TOK_ARROW, start_pos, start_line, start_col); }
            return make_token(l, TOK_MINUS, start_pos, start_line, start_col);

        /* '&&' — bare '&' is an error in this language */
        case '&':
            if (peek(l) == '&') { advance(l); return make_token(l, TOK_AND, start_pos, start_line, start_col); }
            return make_token(l, TOK_ERROR, start_pos, start_line, start_col);

        /* '||' — bare '|' is an error */
        case '|':
            if (peek(l) == '|') { advance(l); return make_token(l, TOK_OR, start_pos, start_line, start_col); }
            return make_token(l, TOK_ERROR, start_pos, start_line, start_col);

        /* Anything else is an error token — never crash. */
        default:
            return make_token(l, TOK_ERROR, start_pos, start_line, start_col);
    }
}

const char *token_type_name(TokenType t) {
    switch (t) {
        case TOK_INT_LIT:   return "INT_LIT";
        case TOK_BOOL_LIT:  return "BOOL_LIT";
        case TOK_IDENT:     return "IDENT";
        case TOK_FN:        return "FN";
        case TOK_LET:       return "LET";
        case TOK_IF:        return "IF";
        case TOK_ELSE:      return "ELSE";
        case TOK_WHILE:     return "WHILE";
        case TOK_RETURN:    return "RETURN";
        case TOK_I64:       return "I64";
        case TOK_BOOL:      return "BOOL";
        case TOK_PLUS:      return "PLUS";
        case TOK_MINUS:     return "MINUS";
        case TOK_STAR:      return "STAR";
        case TOK_SLASH:     return "SLASH";
        case TOK_EQ:        return "EQ";
        case TOK_NEQ:       return "NEQ";
        case TOK_LT:        return "LT";
        case TOK_GT:        return "GT";
        case TOK_LEQ:       return "LEQ";
        case TOK_GEQ:       return "GEQ";
        case TOK_ASSIGN:    return "ASSIGN";
        case TOK_AND:       return "AND";
        case TOK_OR:        return "OR";
        case TOK_BANG:      return "BANG";
        case TOK_LPAREN:    return "LPAREN";
        case TOK_RPAREN:    return "RPAREN";
        case TOK_LBRACE:    return "LBRACE";
        case TOK_RBRACE:    return "RBRACE";
        case TOK_COMMA:     return "COMMA";
        case TOK_SEMICOLON: return "SEMICOLON";
        case TOK_COLON:     return "COLON";
        case TOK_ARROW:     return "ARROW";
        case TOK_EOF:       return "EOF";
        case TOK_ERROR:     return "ERROR";
        default:            return "UNKNOWN";
    }
}
