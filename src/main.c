#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lexer.h"

/* ─────────────────────────────────────────────
   Read an entire file into a heap buffer.
   Caller is responsible for free().
   Returns NULL on failure.
   ───────────────────────────────────────────── */
static char *read_file(const char *path, size_t *out_len) {
    FILE *f = fopen(path, "rb");
    if (!f) {
        fprintf(stderr, "error: cannot open '%s'\n", path);
        return NULL;
    }

    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    rewind(f);

    char *buf = malloc((size_t)size + 1);
    if (!buf) {
        fprintf(stderr, "error: out of memory\n");
        fclose(f);
        return NULL;
    }

    size_t read = fread(buf, 1, (size_t)size, f);
    buf[read] = '\0';
    fclose(f);

    *out_len = read;
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s <source-file>\n", argv[0]);
        return 1;
    }

    size_t src_len;
    char  *src = read_file(argv[1], &src_len);
    if (!src) return 1;

    Lexer l;
    lexer_init(&l, src, src_len);

    int had_error = 0;
    int tok_count = 0;

    printf("%-14s %-16s %s\n", "TYPE", "LEXEME", "LOC");
    printf("%-14s %-16s %s\n", "────────────", "──────────────", "───────");

    for (;;) {
        Token t = lexer_next_token(&l);
        tok_count++;

        /* Print lexeme (it is NOT null-terminated, so use %.*s) */
        printf("%-14s %-16.*s %d:%d\n",
               token_type_name(t.type),
               (int)t.len, t.start,
               t.line, t.col);

        if (t.type == TOK_ERROR) {
            had_error = 1;
            fprintf(stderr, "error: unrecognised character '%.*s' at %d:%d\n",
                    (int)t.len, t.start, t.line, t.col);
        }

        if (t.type == TOK_EOF) break;
    }

    printf("\n%d token(s) scanned.\n", tok_count);

    free(src);
    return had_error ? 1 : 0;
}
