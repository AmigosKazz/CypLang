#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>
#include <stdbool.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_CHARACTER,

    //Operators and punctuation
    TOKEN_COMMA,
    TOKEN_SEMICOLON,
    TOKEN_DOT,
    TOKEN_MINUS,
    TOKEN_PLUS,
    TOKEN_ASTERISK,
    TOKEN_SLASH,
    TOKEN_EQUAL,
    TOKEN_GREATER,
    TOKEN_GREATER_EQUAL,
    TOKEN_LESS,
    TOKEN_LESS_EQUAL,
    TOKEN_BANG,
    TOKEN_BANG_EQUAL,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_LBRACK,
    TOKEN_RBRACK,
    TOKEN_ASSIGN,
    TOKEN_RIGHT_ARROW,

    // Keywords
    TOKEN_DEBFONC,
    TOKEN_FINFONC,
    TOKEN_FAIRE,
    TOKEN_FINFAIRE,
    TOKEN_SI,
    TOKEN_FINSI,
    TOKEN_ALORS,
    TOKEN_SINON,
    TOKEN_TANTQUE,
    TOKEN_POUR,
    TOKEN_HAUT,
    TOKEN_BAS,
    TOKEN_ET,
    TOKEN_OU,
    TOKEN_NON,
    TOKEN_DIV,
    TOKEN_MOD,
    TOKEN_RETOURNER,
    TOKEN_STRUCTURE,
    TOKEN_TYPE,
    TOKEN_VIDE,
    TOKEN_CHAINE,
    TOKEN_ENTIER,
    TOKEN_REEL,
    TOKEN_BOOLEEN,
    TOKEN_VRAI,
    TOKEN_FAUX,
    TOKEN_NIL,
    TOKEN_D,               // d(for data parameter)
    TOKEN_R,               // r(for result parameter)
    TOKEN_DR               // dr(for data-result parameter)
} TokenType;

typedef struct {
    TokenType type;
    char *value;
    int line;
    int column;
} Token;

typedef struct {
    char* source;
    int position;
    int line;
    int column;
    char current_char;
} Lexer;

Lexer* init_lexer(char* source);
void advance(Lexer* lexer);
void skip_whitespace(Lexer* lexer);
TokenType check_keyword(const char* identifier);
Token* parse_identifier(Lexer* lexer);
Token* parse_number(Lexer* lexer);
Token* parse_string(Lexer* lexer);
Token* parse_char(Lexer* lexer);
Token* get_the_next_token(Lexer* lexer);
void free_lexer(Lexer* lexer);

#endif //LEXER_H
