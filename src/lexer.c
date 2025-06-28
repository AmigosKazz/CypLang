#include "../include/lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

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
    if (lexer->source[lexer->position] != '\0') {
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

TokenType check_keyword(const char* identifier) {
    if (strcmp(identifier, "debfonc") == 0) return TOKEN_DEBFONC;
    if (strcmp(identifier, "finfonc") == 0) return TOKEN_FINFONC;
    if (strcmp(identifier, "faire") == 0) return TOKEN_FAIRE;
    if (strcmp(identifier, "finfaire") == 0) return TOKEN_FINFAIRE;
    if (strcmp(identifier, "si") == 0) return TOKEN_SI;
    if (strcmp(identifier, "alors") == 0) return TOKEN_ALORS;
    if (strcmp(identifier, "sinon") == 0) return TOKEN_SINON;
    if (strcmp(identifier, "finsi") == 0) return TOKEN_FINSI;
    if (strcmp(identifier, "tantque") == 0) return TOKEN_TANTQUE;
    if (strcmp(identifier, "pour") == 0) return TOKEN_POUR;
    if (strcmp(identifier, "haut") == 0) return TOKEN_HAUT;
    if (strcmp(identifier, "bas") == 0) return TOKEN_BAS;
    if (strcmp(identifier, "et") == 0) return TOKEN_ET;
    if (strcmp(identifier, "ou") == 0) return TOKEN_OU;
    if (strcmp(identifier, "non") == 0) return TOKEN_NON;
    if (strcmp(identifier, "div") == 0) return TOKEN_DIV;
    if (strcmp(identifier, "mod") == 0) return TOKEN_MOD;
    if (strcmp(identifier, "retourner") == 0) return TOKEN_RETOURNER;
    if (strcmp(identifier, "structure") == 0) return TOKEN_STRUCTURE;
    if (strcmp(identifier, "type") == 0) return TOKEN_TYPE;
    if (strcmp(identifier, "vide") == 0) return TOKEN_VIDE;
    if (strcmp(identifier, "chaine") == 0) return TOKEN_CHAINE;
    if (strcmp(identifier, "caractere") == 0) return TOKEN_CHARACTER;
    if (strcmp(identifier, "entier") == 0) return TOKEN_ENTIER;
    if (strcmp(identifier, "reel") == 0) return TOKEN_REEL;
    if (strcmp(identifier, "booleen") == 0) return TOKEN_BOOLEEN;
    if (strcmp(identifier, "vrai") == 0) return TOKEN_VRAI;
    if (strcmp(identifier, "faux") == 0) return TOKEN_FAUX;
    if (strcmp(identifier, "nil") == 0) return TOKEN_NIL;
    if (strcmp(identifier, "d") == 0) return TOKEN_D;
    if (strcmp(identifier, "r") == 0) return TOKEN_R;
    if (strcmp(identifier, "dr") == 0) return TOKEN_DR;

    return TOKEN_IDENTIFIER;
}

// parse an identifier
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

// parse a string
Token* parse_string(Lexer* lexer) {
    int start_col = lexer->column;
    int start_pos = lexer->position + 1; // skip the opening quote
    advance(lexer); // move past the opening quote

    while (lexer->current_char != '\0' && lexer->current_char != '"') {
        advance(lexer);
    }

    if (lexer->current_char == '\0') {
        fprintf(stderr, "Error: Unterminated string at line %d, column %d\n", lexer->line, lexer->column);
        exit(EXIT_FAILURE);
    }

    int length = lexer->position - start_pos;
    char* str = (char*)malloc(length + 1);
    strncpy(str, lexer->source + start_pos, length);
    str[length] = '\0';

    advance(lexer); // move past the closing quote

    Token* token = (Token*)malloc(sizeof(Token));
    token->type = TOKEN_STRING;
    token->value = str;
    token->line = lexer->line;
    token->column = start_col;

    return token;
}

Token* parse_character(Lexer* lexer) {
    int start_col = lexer->column;
    advance(lexer);

    char value = lexer->current_char;
    advance(lexer);

    if (lexer->current_char != '\'') {
        fprintf(stderr, "Error: Unexpected character '%c' at line %d, column %d\n",
            lexer->line, lexer->column);
        exit(EXIT_FAILURE);
    }

    advance(lexer);

    Token* token = (Token*)malloc(sizeof(Token));
    token->type = TOKEN_CHARACTER;
    token->value = (char*)malloc(2);
    token->value[0] = value;
    token->value[1] = '\0';
    token->line = lexer->line;
    token->column = lexer->column;

    return token;
}

Token* get_the_next_token(Lexer* lexer) {
    while (lexer->current_char != '\0') {
        if (isspace(lexer->current_char)) {
            skip_whitespace(lexer);
            continue;
        }

        if (isalpha(lexer->current_char) || lexer->current_char == '_') {
            return parse_identifier(lexer);
        }

        if (isdigit(lexer->current_char)) {
            return parse_number(lexer);
        }

        if (lexer->current_char == '"') {
            return parse_string(lexer);
        }

        if (lexer->current_char == '\'') {
            return parse_character(lexer);
        }

        // two charcter operators and special tokens
        int start_col = lexer->column;

        if (lexer->current_char == '<') {
            advance(lexer);
            if (lexer->current_char == '=') {
                advance(lexer);
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_LESS_EQUAL;
                token->value = strdup("<=");
                token->line = lexer->line;
                token->column = lexer->column;

                return token;
            } else if (lexer->current_char == '-') {
                advance(lexer);
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_ASSIGN;
                token->value = strdup("<-");
                token->line = lexer->line;
                token->column = lexer->column;

                return token;
            } else {
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_LESS;
                token->value = strdup("<");
                token->line = lexer->line;
                token->column = lexer->column;

                return token;
            }
        }

        if (lexer->current_char == '>') {
            advance(lexer);
            if (lexer->current_char == '=') {
                advance(lexer);
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_GREATER_EQUAL;
                token->value = strdup(">=");
                token->line = lexer->line;
                token->column = start_col;
                return token;
            } else {
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_GREATER;
                token->value = strdup(">");
                token->line = lexer->line;
                token->column = start_col;
                return token;
            }
        }

        if (lexer->current_char == '!') {
            advance(lexer);
            if (lexer->current_char == '=') {
                advance(lexer);
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_BANG_EQUAL;
                token->value = strdup("!=");
                token->line = lexer->line;
                token->column = start_col;
                return token;
            } else {
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_BANG;
                token->value = strdup("!");
                token->line = lexer->line;
                token->column = start_col;
                return token;
            }
        }

        if (lexer->current_char == '-') {
            advance(lexer);
            if (lexer->current_char == '>') {
                advance(lexer);
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_RIGHT_ARROW;
                token->value = strdup("->");
                token->line = lexer->line;
                token->column = start_col;
                return token;
            } else {
                Token* token = (Token*)malloc(sizeof(Token));
                token->type = TOKEN_MINUS;
                token->value = strdup("-");
                token->line = lexer->line;
                token->column = start_col;
                return token;
            }
        }

        // OTHER SINGLE CHARACTER TOKENS
        Token* token = (Token*)malloc(sizeof(Token));
        token->line = lexer->line;
        token->column = lexer->column;
        token->value = (char*) malloc(2);
        token->value[0] = lexer->current_char;
        token->value[1] = '\0';

        switch (lexer->current_char) {
            case '+': token->type = TOKEN_PLUS; break;
            case '*': token->type = TOKEN_ASTERISK; break;
            case '/': token->type = TOKEN_SLASH; break;
            case '=': token->type = TOKEN_EQUAL; break;
            case '.': token->type = TOKEN_DOT; break;
            case ',': token->type = TOKEN_COMMA; break;
            case ';': token->type = TOKEN_SEMICOLON; break;
            case '(': token->type = TOKEN_LPAREN; break;
            case ')': token->type = TOKEN_RPAREN; break;
            case '[': token->type = TOKEN_LBRACK; break;
            case ']': token->type = TOKEN_RBRACK; break;
            default:
                free(token->value);
                free(token);
                fprintf(stderr, "Error: Unexpected character '%c' at line %d, column %d\n",
                    lexer->current_char, lexer->line, lexer->column);
                advance(lexer);
                continue;
        }
        advance(lexer); // move to the next character
        return token;
    }

    // End of file
    Token* token = (Token*)malloc(sizeof(Token));
    token->type = TOKEN_EOF;
    token->value = NULL;
    token->line = lexer->line;
    token->column = lexer->column;

    return token;
}

void free_lexer(Lexer* lexer) {
    if (lexer != NULL) {
        free(lexer);
    }
}
