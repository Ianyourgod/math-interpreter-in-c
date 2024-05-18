#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#define TOKEN_EOF 1
#define TOKEN_INT 2
#define TOKEN_ADD 3
#define TOKEN_SUB 4
#define TOKEN_MUL 5
#define TOKEN_DIV 6

/********************\
|     * LEXER *      |
\********************/

struct Lexer {
    int position;
    char* input;
};

struct Token {
    int type;
    char value[100];
};

struct Token lexer_get_next_token(struct Lexer* self) {
    struct Token token = {0, ""};

    while (self->input[self->position] != '\0') {
        if (isdigit(self->input[self->position])) {
            token.type = TOKEN_INT;
            int i = 0;
            while (isdigit(self->input[self->position])) {
                token.value[i] = self->input[self->position];
                i++;
                self->position++;
            }
            token.value[i] = '\0';
            break;
        } else if (self->input[self->position] == '+') {
            token.type = TOKEN_ADD;
            token.value[0] = '+';
            token.value[1] = '\0';
            self->position++;
            break;
        } else if (self->input[self->position] == '-') {
            token.type = TOKEN_SUB;
            token.value[0] = '-';
            token.value[1] = '\0';
            self->position++;
            break;
        } else if (self->input[self->position] == '*') {
            token.type = TOKEN_MUL;
            token.value[0] = '*';
            token.value[1] = '\0';
            self->position++;
            break;
        } else if (self->input[self->position] == '/') {
            token.type = TOKEN_DIV;
            token.value[0] = '/';
            token.value[1] = '\0';
            self->position++;
            break;
        } else {
            self->position++;
        }
    }

    if (self->input[self->position] == '\0') {
        token.type = TOKEN_EOF;
    }

    return token;
}

struct Token lexer_peek_next_token(struct Lexer* self) {
    int position = self->position;
    struct Token token = lexer_get_next_token(self);
    self->position = position;
    return token;
}

struct Token lexer_eat_token(struct Lexer* self, int type) {
    struct Token token = lexer_get_next_token(self);
    if (token.type != type) {
        printf("expected: %d, got: %d\n", type, token.type);
        printf("Invalid syntax\n");
        exit(1);
    }
    return token;
}

struct Token lexer_eat_token_value(struct Lexer* self, int type, char* value) {
    struct Token token = lexer_get_next_token(self);
    if (token.type != type || strcmp(token.value, value) != 0) {
        printf("Invalid syntax\n");
        exit(1);
    }
    return token;
}

/********************\
|     * PARSER *     |
\********************/

struct Parser {
    struct Lexer* lexer;
    struct Token current_token;
};

struct Node {
    int type;
    int value;
    struct Node* left;
    struct Node* right;
};

struct Parser parser_init(struct Lexer* lexer) {
    struct Parser parser = {lexer, lexer_get_next_token(lexer)};
    return parser;
}

void parser_eat(struct Parser* self, int type) {
    if (self->current_token.type == type) {
        self->current_token = lexer_get_next_token(self->lexer);
    } else {
        printf("Invalid syntax\n");
        exit(1);
    }
}

struct Node parser_parse_int(struct Parser* self) {
    struct Node node = {TOKEN_INT, atoi(self->current_token.value), NULL, NULL};
    parser_eat(self, TOKEN_INT);
    return node;
}

struct Node* parser_term(struct Parser* self) {
    struct Node* node = (struct Node*)malloc(sizeof(struct Node));
    *node = parser_parse_int(self);

    while ((self->current_token.type == TOKEN_MUL) || (self->current_token.type == TOKEN_DIV)) {
        struct Token token = self->current_token;
        parser_eat(self, token.type);

        struct Node* right = (struct Node*)malloc(sizeof(struct Node));
        *right = parser_parse_int(self);

        struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
        new_node->type = token.type;
        new_node->left = node;
        new_node->right = right;

        node = new_node;
    }

    return node;
}

struct Node* parser_expr(struct Parser* self) {
    struct Node* node = parser_term(self);

    while ((self->current_token.type == TOKEN_ADD) || (self->current_token.type == TOKEN_SUB)) {
        struct Token token = self->current_token;
        parser_eat(self, token.type);

        struct Node* right = parser_term(self);

        struct Node* new_node = (struct Node*)malloc(sizeof(struct Node));
        new_node->type = token.type;
        new_node->left = node;
        new_node->right = right;

        node = new_node;
    }

    return node;
}

/********************\
|     * MAIN *       |
\********************/

void print_node(struct Node node) {
    if (node.type == TOKEN_INT) {
        printf("%d", node.value);
    } else {
        print_node(*node.left);
        if (node.type == TOKEN_ADD) {
            printf("+");
        } else if (node.type == TOKEN_SUB) {
            printf("-");
        } else if (node.type == TOKEN_MUL) {
            printf("*");
        } else if (node.type == TOKEN_DIV) {
            printf("/");
        }
        print_node(*node.right);
    }
}

void free_node(struct Node* node) {
    if (node->type != TOKEN_INT) {
        free_node(node->left);
        free_node(node->right);
    }
    free(node);
}

int interpret(struct Node* node) {
    if (node->type == TOKEN_INT) {
        return node->value;
    } else {
        int left = interpret(node->left);
        int right = interpret(node->right);
        if (node->type == TOKEN_ADD) {
            return left + right;
        } else if (node->type == TOKEN_SUB) {
            return left - right;
        } else if (node->type == TOKEN_MUL) {
            return left * right;
        } else if (node->type == TOKEN_DIV) {
            return left / right;
        }
    }
    printf("Invalid syntax\n");
    exit(1);
}

int main() {
    // get user input

    char input[100];
    fgets(input, 100, stdin);

    // tokenize
    struct Lexer lexer = {0, input};

    struct Parser parser = parser_init(&lexer);

    // parse the input
    struct Node* node = parser_expr(&parser);

    // interpret
    printf("Result: %d\n", interpret(node));

    // free the memory
    free_node(node);

    return 0;
}