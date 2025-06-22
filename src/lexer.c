#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

typedef enum {
    TOKEN_EOF,
    TOKEN_IDENTIFIER,
    TOKEN_NUMBER,
    TOKEN_STRING,
    TOKEN_ASSIGN,
    TOKEN_PLUS,
    TOKEN_MINUS,
    TOKEN_MULTIPLY,
    TOKEN_DIVIDE,
    TOKEN_MODULO,
    TOKEN_LPAREN,
    TOKEN_RPAREN,
    TOKEN_SEMICOLON,
    TOKEN_COMMA,

    // Keywords
    TOKEN_SI,
    TOKEN_ALORS,
    TOKEN_SINON,
    TOKEN_FINSI,
    TOKEN_TANTQUE,
    TOKEN_FAIRE,
    TOKEN_FINFAIRE,
    TOKEN_POUR,
    TOKEN_HAUT,
    TOKEN_BAS,
    TOKEN_REPETER,
    TOKEN_JUSQUA,
    TOKEN_ENTIER,
    TOKEN_REEL,
    TOKEN_CARACTERE,
    TOKEN_BOOLEEN,
    TOKEN_VIDE,
    TOKEN_ECRIRE,
    TOKEN_LIRE,
    TOKEN_RETOURNER,
    TOKEN_DEBFONC,
    TOKEN_FINFONC,
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


Lexer* init_lexer(char* source) {
    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    lexer->source = source;
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->current_char = source[0];
    return lexer;
}

// move to the next character in the source code
void advance(Lexer* lexer) {
    if (lexer->current_char == '\n') {
        lexer->line++;
        lexer->column = 1;
    } else {
        lexer->column++;
    }

    lexer->position++;
    if (lexer->source[lexer->position] == '\0') {
        lexer->current_char = lexer->source[lexer->position];
    } else {
        lexer->current_char = '\0'; // end of file
    }
}

void skip_whitespace(Lexer* lexer) {
    while (lexer->current_char != '\0' && isspace(lexer->current_char)) {
        advance(lexer);
    }
}

TokenType check_keyword(char* identifier) {
    if (strcmp(identifier, "si") == 0) return TOKEN_SI;
    if (strcmp(identifier, "alors") == 0) return TOKEN_ALORS;
    if (strcmp(identifier, "sinon") == 0) return TOKEN_SINON;
    if (strcmp(identifier, "finsi") == 0) return TOKEN_FINSI;
    if (strcmp(identifier, "tantque") == 0) return TOKEN_TANTQUE;
    if (strcmp(identifier, "faire") == 0) return TOKEN_FAIRE;
    if (strcmp(identifier, "finfaire") == 0) return TOKEN_FINFAIRE;
    if (strcmp(identifier, "pour") == 0) return TOKEN_POUR;
    if (strcmp(identifier, "haut") == 0) return TOKEN_HAUT;
    if (strcmp(identifier, "bas") == 0) return TOKEN_BAS;
    if (strcmp(identifier, "repeter") == 0) return TOKEN_REPETER;
    if (strcmp(identifier, "jusqua") == 0) return TOKEN_JUSQUA;
    if (strcmp(identifier, "entier") == 0) return TOKEN_ENTIER;
    if (strcmp(identifier, "reel") == 0) return TOKEN_REEL;
    if (strcmp(identifier, "caractere") == 0) return TOKEN_CARACTERE;
    if (strcmp(identifier, "booleen") == 0) return TOKEN_BOOLEEN;
    if (strcmp(identifier, "vide") == 0) return TOKEN_VIDE;
    if (strcmp(identifier, "ecrire") == 0) return TOKEN_ECRIRE;
    if (strcmp(identifier, "lire") == 0) return TOKEN_LIRE;
    if (strcmp(identifier, "retourner") == 0) return TOKEN_RETOURNER;
    if (strcmp(identifier, "debfonc") == 0) return TOKEN_DEBFONC;
    if (strcmp(identifier, "finfonc") == 0) return TOKEN_FINFONC;

    return TOKEN_IDENTIFIER; // not a keyword
}

// parse an identifier or keyword
Token* parse_identifier(Lexer* lexer) {
    int start_col = lexer->column;
    int start_pos = lexer->position;

    while (
        lexer->current_char != '\0' &&
        (isalnum(lexer->current_char) || lexer->current_char == '_')) {
        advance(lexer);
    }

    int length = lexer->position - start_pos;
    char* identifier = (char*)malloc(length + 1);
    strncpy(identifier, lexer->source + start_pos, length);
    identifier[length] = '\0';

    TokenType type = check_keyword(identifier);

    Token* token = (Token*)malloc(sizeof(Token));
    token->type = type;
    token->value = identifier;
    token->line = lexer->line;
    token->column = start_col;

    return token;
}

// parse a number
Token* parse_number(Lexer* lexer) {
    int start_col = lexer->column;
    int start_pos = lexer->position;

    while (lexer->current_char != '\0' && isdigit(lexer->current_char)) {
        advance(lexer);
    }

    int length = lexer->position - start_pos;
    char* number = (char*)malloc(length + 1);
    strncpy(number, lexer->source + start_pos, length);
    number[length] = '\0';

    Token* token = (Token*)malloc(sizeof(Token));
    token->type = TOKEN_NUMBER;
    token->value = number;
    token->line = lexer->line;
    token->column = start_col;

    return token;
}