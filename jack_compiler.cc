#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <cctype>

#define TRUE 1
#define FALSE 0
#define DEBUG_PRINT printf

// Global Variables
bool g_key_ignore = FALSE;
char* g_class_name = NULL;
char* g_callee_name = NULL;
char* g_caller_name = NULL;
int g_callee_type = 0;
int while_l1_index = 0;
int while_l2_index = 0;
int if_l1_index = 0;
int if_l2_index = 0;
int g_static_index = 0;

void g_class_name_set(char* str)
{
    if(g_class_name == NULL)
    {
        g_class_name = strdup(str);
    }
    else
    {
        free(g_class_name);
        g_class_name = strdup(str);
    }
}

void g_callee_name_set(char* left, char* right)
{
    if(g_callee_name == NULL)
    {
        g_callee_name = (char*)malloc(200 * sizeof(char));
        strcpy(g_callee_name, left);
        strcat(g_callee_name, ".");
        strcat(g_callee_name, right);
    }
    else
    {
        free(g_callee_name);
        g_callee_name = (char*)malloc(200 * sizeof(char));
        strcpy(g_callee_name, left);
        strcat(g_callee_name, ".");
        strcat(g_callee_name, right);
    }
}

void g_caller_name_set(char* left, char* right)
{
    if(g_caller_name == NULL)
    {
        g_caller_name = (char*)malloc(200 * sizeof(char));
        strcpy(g_caller_name, left);
        strcat(g_caller_name, ".");
        strcat(g_caller_name, right);
    }
    else
    {
        free(g_caller_name);
        g_caller_name = (char*)malloc(200 * sizeof(char));
        strcpy(g_caller_name, left);
        strcat(g_caller_name, ".");
        strcat(g_caller_name, right);
    }
}

// Tokenizer
typedef enum {
    KEYWORD, SYMBOL, IDENTIFIER, 
    INT_CONST, STRING_CONST
} token_type;

typedef enum {
    CLASS, METHOD, FUNCTION,
    CONSTRUCTOR, INT, BOOLEAN, CHAR,
    VOID, VAR, STATIC, FIELD, LET, DO,
    IF, ELSE, WHILE, RETURN, TRUE_k, FALSE_k, 
    NULL_k, THIS
} keyword;

typedef struct token_node {
    char* token;
    token_type tokenType;
    struct token_node* next;
    union {
        keyword keyWord;
        char symbol;
        char* identifier;
        int intVal;
        char* stringVal;
    };
} token_node;

bool hasMoreTokens(token_node** token_list)
{
    if(*token_list != NULL)
        return TRUE;
    else
        return FALSE;
}

void advance(token_node** token_list)
{
    token_node* parent = *token_list;
    *token_list = (*token_list)->next;
    if(parent->tokenType == IDENTIFIER)
        free(parent->identifier);
    else if(parent->tokenType == STRING_CONST)
        free(parent->stringVal);
    free(parent->token);
    free(parent); // To prevent memory leaks!
}

bool iskeyword_string(char* new_token)
{
    if (strcmp(new_token, "class") == 0 ||
        strcmp(new_token, "constructor") == 0 ||
        strcmp(new_token, "function") == 0 ||
        strcmp(new_token, "method") == 0 ||
        strcmp(new_token, "field") == 0 ||
        strcmp(new_token, "static") == 0 ||
        strcmp(new_token, "var") == 0 ||
        strcmp(new_token, "int") == 0 ||
        strcmp(new_token, "char") == 0 ||
        strcmp(new_token, "boolean") == 0 ||
        strcmp(new_token, "void") == 0 ||
        strcmp(new_token, "true") == 0 ||
        strcmp(new_token, "false") == 0 ||
        strcmp(new_token, "null") == 0 ||
        strcmp(new_token, "this") == 0 ||
        strcmp(new_token, "let") == 0 ||
        strcmp(new_token, "do") == 0 ||
        strcmp(new_token, "if") == 0 ||
        strcmp(new_token, "else") == 0 ||
        strcmp(new_token, "while") == 0 ||
        strcmp(new_token, "return") == 0)
            return TRUE;
    else 
        return FALSE; 
}

bool issymbol_string(char* new_token)
{
    if (strcmp(new_token, "{") == 0 ||
        strcmp(new_token, "}") == 0 ||
        strcmp(new_token, "(") == 0 ||
        strcmp(new_token, ")") == 0 ||
        strcmp(new_token, "[") == 0 ||
        strcmp(new_token, "]") == 0 ||
        strcmp(new_token, ".") == 0 ||
        strcmp(new_token, ",") == 0 ||
        strcmp(new_token, ";") == 0 ||
        strcmp(new_token, "+") == 0 ||
        strcmp(new_token, "-") == 0 ||
        strcmp(new_token, "*") == 0 ||
        strcmp(new_token, "/") == 0 ||
        strcmp(new_token, "&") == 0 ||
        strcmp(new_token, "|") == 0 ||
        strcmp(new_token, "<") == 0 ||
        strcmp(new_token, ">") == 0 ||
        strcmp(new_token, "=") == 0 ||
        strcmp(new_token, "~") == 0)
            return TRUE;
    else 
        return FALSE; 
}

bool isop(char c)
{
    if (c == '+' || c == '-' ||
        c == '*' || c == '/' ||
        c == '&' || c == '|' ||
        c == '<' || c == '=' || c == '>')
        return TRUE;
    else 
        return FALSE;
}

token_node* create_new_token_node(char* new_token, bool is_string) {
    token_node* new_token_node = (token_node*)malloc(sizeof(token_node));
    new_token_node->token = strdup(new_token);
    if(iskeyword_string(new_token))
    {
        new_token_node->tokenType = KEYWORD;
        if (strcmp(new_token, "class") == 0)
            new_token_node->keyWord = CLASS;
        else if (strcmp(new_token, "constructor") == 0)
            new_token_node->keyWord = CONSTRUCTOR;
        else if (strcmp(new_token, "function") == 0)
            new_token_node->keyWord = FUNCTION;
        else if (strcmp(new_token, "method") == 0)
            new_token_node->keyWord = METHOD; 
        else if (strcmp(new_token, "field") == 0)
            new_token_node->keyWord = FIELD; 
        else if (strcmp(new_token, "static") == 0)
            new_token_node->keyWord = STATIC; 
        else if (strcmp(new_token, "var") == 0)
            new_token_node->keyWord = VAR; 
        else if (strcmp(new_token, "int") == 0) 
            new_token_node->keyWord = INT; 
        else if (strcmp(new_token, "char") == 0)
            new_token_node->keyWord = CHAR;
        else if (strcmp(new_token, "boolean") == 0)
            new_token_node->keyWord = BOOLEAN; 
        else if (strcmp(new_token, "void") == 0)
            new_token_node->keyWord = VOID; 
        else if (strcmp(new_token, "true") == 0)
            new_token_node->keyWord = TRUE_k; 
        else if (strcmp(new_token, "false") == 0)
            new_token_node->keyWord = FALSE_k; 
        else if (strcmp(new_token, "null") == 0)
            new_token_node->keyWord = NULL_k; 
        else if (strcmp(new_token, "this") == 0) 
            new_token_node->keyWord = THIS; 
        else if (strcmp(new_token, "let") == 0)
            new_token_node->keyWord = LET; 
        else if (strcmp(new_token, "do") == 0) 
            new_token_node->keyWord = DO; 
        else if (strcmp(new_token, "if") == 0) 
            new_token_node->keyWord = IF; 
        else if (strcmp(new_token, "else") == 0) 
            new_token_node->keyWord = ELSE; 
        else if (strcmp(new_token, "while") == 0) 
            new_token_node->keyWord = WHILE; 
        else if (strcmp(new_token, "return") == 0)
            new_token_node->keyWord = RETURN; 
        else{}
    }
    else if(issymbol_string(new_token))
    {
        new_token_node->tokenType = SYMBOL;
        new_token_node->symbol = new_token[0];
    }
    else if(isdigit(new_token[0]))
    {
        new_token_node->tokenType = INT_CONST;
        new_token_node->intVal = atoi(new_token);
    }
    else if(is_string)
    {
        new_token_node->tokenType = STRING_CONST;
        new_token_node->stringVal = strdup(new_token);
    }
    else
    {
        new_token_node->tokenType = IDENTIFIER;
        new_token_node->identifier = strdup(new_token);
    }
    new_token_node->next = NULL;
    return new_token_node;
}

void append_to_token_list(char* new_token, token_node** token_list, bool is_string)
{
    token_node* new_token_node = create_new_token_node(new_token, is_string);

    if (*token_list == NULL) {
        *token_list = new_token_node;
        return;
    }

    token_node* current = *token_list;
    while (current->next != NULL) {
        current = current->next;
    }
    current->next = new_token_node;
}

bool issymbol_char(char c)
{
    if (c == '{' || c == '}' || c == '(' ||
        c == ')' || c == '[' || c == ']' ||
        c == '.' || c == ',' || c == ';' ||
        c == '+' || c == '-' || c == '*' ||
        c == '/' || c == '&' || c == '|' ||
        c == '<' || c == '>' || c == '=' ||
        c == '~') 
        return TRUE;
    else 
     return FALSE; 
}

void tokenize_and_add_to_token_list(char* instruction, token_node** token_list)
{
    while(strlen(instruction) != 0)
    {
        char match[1000];

        if (isspace(instruction[0]))
        {
            instruction++;
        }
        else if (issymbol_char(instruction[0]))
        {
            match[0] = instruction[0];
            match[1] = '\0';
            append_to_token_list(match, token_list, FALSE);
            instruction++;
        }
        else if (instruction[0] == '"')
        {
            match[0] = '\0';
            char *ret;
            ret = strchr(instruction+1, '"'); 
            strncat(match, instruction+1, ret - instruction - 1);
            append_to_token_list(match, token_list, TRUE);
            instruction = ret + 1;
        }
        else
        {
            for (int i = 0; i < 1000; i++)
            {
                if (strlen(instruction) != 0 && (isspace(instruction[0]) || issymbol_char(instruction[0])))
                {
                    match[i] = '\0';
                    append_to_token_list(match, token_list, FALSE);
                    break;
                }
                else
                {
                    match[i] = instruction[0];
                    instruction++;
                }
            }
        }
    }
}

void write_tokens_to_file(FILE **tokenize_out, token_node** token_list)
{
    fprintf(*tokenize_out, "<tokens>\n");

    token_node* current = *token_list;
    while (current != NULL) {
        switch(current->tokenType){
            case KEYWORD:
            {
                fprintf(*tokenize_out, "<keyword> ");
                fprintf(*tokenize_out, "%s", current->token);       
                fprintf(*tokenize_out, " </keyword>");
            } break;
            case SYMBOL:
            {
                fprintf(*tokenize_out, "<symbol> ");
                if(strcmp(current->token, ">") == 0)
                    fprintf(*tokenize_out, "&gt;");       
                else if(strcmp(current->token, "<") == 0)
                    fprintf(*tokenize_out, "&lt;");       
                else if(strcmp(current->token, "\"") == 0)
                    fprintf(*tokenize_out, "&quot;");       
                else if(strcmp(current->token, "&") == 0)
                    fprintf(*tokenize_out, "&amp;");       
                else
                    fprintf(*tokenize_out, "%s", current->token);       
                fprintf(*tokenize_out, " </symbol>");               
            } break;
            case IDENTIFIER:
            {
                fprintf(*tokenize_out, "<identifier> ");
                fprintf(*tokenize_out, "%s", current->token);       
                fprintf(*tokenize_out, " </identifier>");               
            } break;
            case INT_CONST:
            {
                fprintf(*tokenize_out, "<integerConstant> ");
                fprintf(*tokenize_out, "%s", current->token);       
                fprintf(*tokenize_out, " </integerConstant>");              
            } break;
            case STRING_CONST:
            {
                fprintf(*tokenize_out, "<stringConstant> ");
                fprintf(*tokenize_out, "%s", current->token);       
                fprintf(*tokenize_out, " </stringConstant>");               
            } break;
        }
        fprintf(*tokenize_out, "\n");
        current = current->next;
    }
    fprintf(*tokenize_out, "</tokens>\n");
}

// Symbol Table
typedef enum {
    STATIC_ki, FIELD_ki, ARG_ki, VAR_ki, NONE_ki
} kind_t;

#define HASH_TABLE_SIZE 101

typedef struct nlist
{
    struct nlist* next;
    char *name;
    char *type;
    kind_t kind;
    int index;
} nlist;

typedef struct hash_table
{
    nlist *arr[HASH_TABLE_SIZE];
} hash_table;

hash_table* create_new_hash_table()
{
    hash_table* ht = (hash_table *)malloc(sizeof(hash_table));

    for (int i = 0; i < HASH_TABLE_SIZE; i++) 
        (ht->arr)[i] = NULL;

    return ht;
}

void destroy_hash_table(hash_table* ht)
{
    for (int i = 0; i < HASH_TABLE_SIZE; i++) 
    {
        nlist* np = (ht->arr)[i];

        nlist* parent = np;
        nlist* temp;

        while(parent != NULL)
        {
            temp = parent->next;
            
            free(parent->name);
            free(parent->type);
            free(parent);

            parent = temp;
        }
    }

    free(ht);
}

unsigned hash(char *s)
{
    unsigned hashval;

    for (hashval = 0; *s != '\0'; s++)
    {
        hashval = *s + 31 * hashval;
    }

    return hashval % HASH_TABLE_SIZE;
}

nlist *lookup(hash_table *ht, char *s)
{
    nlist *np;

    for (np = ht->arr[hash(s)]; np != NULL; np = np->next)
    {
        if (strcmp(s, np->name) == 0)
            return np;
    }

    return NULL;
}

nlist *install(hash_table *ht, char *name, char *type, kind_t kind, int index)
{
    nlist *np;
    unsigned hashval;

    if ((np = lookup(ht, name)) == NULL)
    {
        np = (nlist *)malloc(sizeof(*np));
        np->name = strdup(name);
        np->type = strdup(type);
        np->kind = kind;
        np->index = index;
        hashval = hash(name);
        np->next = ht->arr[hashval];
        ht->arr[hashval] = np;
    }
    else
    {
        free((void *)np->type);
        np->type = strdup(type);

        np->kind = kind; 

        np->index = index;
    }

    return np;
}

typedef struct symbol_table {
    hash_table* class_scope;
    hash_table* subroutine_scope;
    int field_index;
    int arg_index;
    int var_index;
} symbol_table;

symbol_table* create_new_symbol_table()
{
    symbol_table* st = (symbol_table *)malloc(sizeof(symbol_table));

    st->class_scope = create_new_hash_table();
    st->subroutine_scope = create_new_hash_table();

    st->field_index = 0; 
    st->arg_index = 0; 
    st->var_index = 0; 

    return st;
}

void destroy_symbol_table(symbol_table* st)
{
    destroy_hash_table(st->class_scope);
    destroy_hash_table(st->subroutine_scope);
    free(st);
}

void start_subroutine(symbol_table* st)
{
    destroy_hash_table(st->subroutine_scope);
    st->subroutine_scope = create_new_hash_table();

    st->arg_index = 0; 
    st->var_index = 0; 
}

void define(symbol_table* st, char* name, char* type, kind_t kind)
{
    if(kind == STATIC_ki)     
    {
        install(st->class_scope, name, type, kind, g_static_index);
        g_static_index++;
    }
    else if(kind == FIELD_ki)
    {
        install(st->class_scope, name, type, kind, st->field_index);
        st->field_index++;            
    }
    else if(kind == ARG_ki)
    {
        install(st->subroutine_scope, name, type, kind, st->arg_index);
        st->arg_index++;
    }
    else if(kind == VAR_ki)
    {
        install(st->subroutine_scope, name, type, kind, st->var_index);
        st->var_index++; 
    }
    else{}
}

int var_count(symbol_table* st, kind_t kind)
{
    if(kind == STATIC_ki)     
        return g_static_index;
    else if(kind == FIELD_ki)
        return st->field_index;            
    else if(kind == ARG_ki)
        return st->arg_index;
    else // VAR_ki
        return st->var_index; 
}

kind_t kind_of(symbol_table* st, char* name)
{
    nlist* cs = lookup(st->class_scope, name);
    nlist* ss = lookup(st->subroutine_scope, name);

    if(cs != NULL)
        return cs->kind;
    else if(ss != NULL)
        return ss->kind;
    else
        return NONE_ki;
}

char* type_of(symbol_table* st, char* name)
{
    nlist* cs = lookup(st->class_scope, name);
    nlist* ss = lookup(st->subroutine_scope, name);

    if(cs != NULL)
        return cs->type;
    else
        return ss->type;
}

int index_of(symbol_table* st, char* name)
{
    nlist* cs = lookup(st->class_scope, name);
    nlist* ss = lookup(st->subroutine_scope, name);

    if(cs != NULL)
        return cs->index;
    else
        return ss->index;
}

// Function Definitions
void compile(FILE **parse_out, FILE **code_generate_out, token_node **token_list, symbol_table** sym_table);
void compile_class(FILE **parse_out, FILE **code_generate_out, token_node **token_list, symbol_table** sym_table);
void compile_class_var_dec(FILE **parse_out, FILE **code_generate_out, token_node **token_list, symbol_table** sym_table);
void compile_subroutine_dec(FILE **parse_out, FILE **code_generate_out, token_node **token_list, symbol_table** sym_table);
void compile_parameters_list(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_subroutine_body(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_var_dec(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_statements(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_let(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_if(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_while(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_do(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_return(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_expression(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
void compile_term(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);
int compile_expression_list(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table);

// VM Writer
typedef enum {
    CONST_s, ARG_s, LOCAL_s, STATIC_s,
    THIS_s, THAT_s, POINTER_s, TEMP_s
} segment_t;

void write_push(FILE** code_generate_out, segment_t segment, int index);
void write_pop(FILE** code_generate_out, segment_t segment, int index);
void write_arithmetic(FILE** code_generate_out, char command);
void write_unary(FILE **code_generate_out, char command);
void write_label(FILE** code_generate_out, char* label);
void write_goto(FILE** code_generate_out, char* label);
void write_if(FILE** code_generate_out, char* label);
void write_call(FILE** code_generate_out, char* name, int nArgs);
void write_function(FILE** code_generate_out, char* name, int nLocals);
void write_return(FILE** code_generate_out);

// Parser - Code Generator
void compile_class(FILE **parse_out, FILE **code_generate_out, token_node **token_list, symbol_table** sym_table)
{
    fprintf(*parse_out, "<class>\n");

    fprintf(*parse_out, "<keyword> class </keyword>\n");
    advance(token_list);
    
    fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
    g_class_name_set((*token_list)->token);
    advance(token_list);

    fprintf(*parse_out, "<symbol> { </symbol>\n");
    advance(token_list);

    while((*token_list)->symbol != '}')
    {
        if((*token_list)->keyWord == STATIC || (*token_list)->keyWord == FIELD)
        {
            compile_class_var_dec(parse_out, code_generate_out, token_list, sym_table);
        }
        else if((*token_list)->keyWord == CONSTRUCTOR || 
                (*token_list)->keyWord == FUNCTION || 
                (*token_list)->keyWord == METHOD)
                {
                    compile_subroutine_dec(parse_out, code_generate_out, token_list, sym_table);
                }
        else{break;}
    }

    fprintf(*parse_out, "<symbol> } </symbol>\n");
    advance(token_list);

    fprintf(*parse_out, "</class>\n");
}

void compile_class_var_dec(FILE **parse_out, FILE **code_generate_out, token_node **token_list, symbol_table** sym_table)
{
    char* class_var_name = NULL;
    char* class_var_type = NULL;
    kind_t class_var_kind;

    fprintf(*parse_out, "<classVarDec>\n");

    if((*token_list)->keyWord == STATIC)
    {
        fprintf(*parse_out, "<keyword> static </keyword>\n");
        class_var_kind = STATIC_ki;
    }
    else if((*token_list)->keyWord == FIELD)
    {
        fprintf(*parse_out, "<keyword> field </keyword>\n");
        class_var_kind = FIELD_ki;
    }
    else{}
    advance(token_list);
    
    if((*token_list)->keyWord == INT)
        fprintf(*parse_out, "<keyword> int </keyword>\n");
    else if((*token_list)->keyWord == CHAR)
        fprintf(*parse_out, "<keyword> char </keyword>\n");
    else if((*token_list)->keyWord == BOOLEAN)
        fprintf(*parse_out, "<keyword> boolean </keyword>\n");
    else
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
    class_var_type = strdup((*token_list)->token); 
    advance(token_list);
    
    fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
    class_var_name = strdup((*token_list)->token);
    define(*sym_table, class_var_name, class_var_type, class_var_kind);
    advance(token_list);

    while((*token_list)->symbol == ',')
    {
        fprintf(*parse_out, "<symbol> , </symbol>\n");
        advance(token_list);

        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        free(class_var_name);
        class_var_name = strdup((*token_list)->token);
        define(*sym_table, class_var_name, class_var_type, class_var_kind);
        advance(token_list);
    }

    fprintf(*parse_out, "<symbol> ; </symbol>\n");
    advance(token_list);

    fprintf(*parse_out, "</classVarDec>\n");

    if(class_var_name != NULL) free(class_var_name);
    if(class_var_type != NULL) free(class_var_type);
}

void compile_subroutine_dec(FILE **parse_out, FILE **code_generate_out, token_node **token_list, symbol_table** sym_table)
{
    start_subroutine(*sym_table);

    fprintf(*parse_out, "<subroutineDec>\n");

    if((*token_list)->keyWord == CONSTRUCTOR)
        fprintf(*parse_out, "<keyword> constructor </keyword>\n");
    else if((*token_list)->keyWord == FUNCTION)
        fprintf(*parse_out, "<keyword> function </keyword>\n");
    else if((*token_list)->keyWord == METHOD)
        fprintf(*parse_out, "<keyword> method </keyword>\n");
    else{}
    g_callee_type = (*token_list)->keyWord;
    advance(token_list);

    if((*token_list)->keyWord == VOID)
        fprintf(*parse_out, "<keyword> void </keyword>\n");
    else if((*token_list)->keyWord == INT)
        fprintf(*parse_out, "<keyword> int </keyword>\n");
    else if((*token_list)->keyWord == CHAR)
        fprintf(*parse_out, "<keyword> char </keyword>\n");
    else if((*token_list)->keyWord == BOOLEAN)
        fprintf(*parse_out, "<keyword> boolean </keyword>\n");
    else
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
    advance(token_list);

    fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
    g_callee_name_set(g_class_name, (*token_list)->token);
    advance(token_list);

    fprintf(*parse_out, "<symbol> ( </symbol>\n");
    advance(token_list);

    if(g_callee_type == METHOD) // this for argument 0
        (*sym_table)->arg_index++;

    compile_parameters_list(parse_out, code_generate_out, token_list, sym_table);

    fprintf(*parse_out, "<symbol> ) </symbol>\n");
    advance(token_list);

    compile_subroutine_body(parse_out, code_generate_out, token_list, sym_table);

    fprintf(*parse_out, "</subroutineDec>\n");
}

void compile_parameters_list(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    char* parameter_name = NULL;
    char* parameter_type = NULL;
    kind_t parameter_kind = ARG_ki;

    fprintf(*parse_out, "<parameterList>\n");

    if((*token_list)->symbol != ')')
    {
        if((*token_list)->keyWord == INT)
            fprintf(*parse_out, "<keyword> int </keyword>\n");
        else if((*token_list)->keyWord == CHAR)
            fprintf(*parse_out, "<keyword> char </keyword>\n");
        else if((*token_list)->keyWord == BOOLEAN)
            fprintf(*parse_out, "<keyword> boolean </keyword>\n");
        else
            fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        parameter_type = strdup((*token_list)->token);
        advance(token_list);

        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        parameter_name = strdup((*token_list)->token);
        define(*sym_table, parameter_name, parameter_type, parameter_kind);
        advance(token_list);
    }

    while((*token_list)->symbol == ',')
    {
        fprintf(*parse_out, "<symbol> , </symbol>\n");
        advance(token_list);

        if((*token_list)->keyWord == INT)
            fprintf(*parse_out, "<keyword> int </keyword>\n");
        else if((*token_list)->keyWord == CHAR)
            fprintf(*parse_out, "<keyword> char </keyword>\n");
        else if((*token_list)->keyWord == BOOLEAN)
            fprintf(*parse_out, "<keyword> boolean </keyword>\n");
        else
            fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        free(parameter_type);
        parameter_type = strdup((*token_list)->token);
        advance(token_list);

        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        free(parameter_name);
        parameter_name = strdup((*token_list)->token);
        define(*sym_table, parameter_name, parameter_type, parameter_kind);
        advance(token_list);
    }

    fprintf(*parse_out, "</parameterList>\n");

    if(parameter_name != NULL) free(parameter_name);
    if(parameter_type != NULL) free(parameter_type);
}

void compile_subroutine_body(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    fprintf(*parse_out, "<subroutineBody>\n");
    
    fprintf(*parse_out, "<symbol> { </symbol>\n");
    advance(token_list);

    while((*token_list)->keyWord == VAR) 
        compile_var_dec(parse_out, code_generate_out, token_list, sym_table);

    if(g_callee_type == CONSTRUCTOR || g_callee_type == FUNCTION) 
        write_function(code_generate_out, g_callee_name, var_count(*sym_table, VAR_ki));
    else if(g_callee_type == METHOD)
    {
        write_function(code_generate_out, g_callee_name, var_count(*sym_table, VAR_ki));
        write_push(code_generate_out, ARG_s, 0);
        write_pop(code_generate_out, POINTER_s, 0);
    }
    
    if (g_callee_type == CONSTRUCTOR)
    {
        write_push(code_generate_out, CONST_s, var_count(*sym_table, FIELD_ki));
        write_call(code_generate_out, "Memory.alloc", 1);
        write_pop(code_generate_out, POINTER_s, 0);
    }
    if (g_callee_type == METHOD)
    {

    }

    compile_statements(parse_out, code_generate_out, token_list, sym_table);

    fprintf(*parse_out, "<symbol> } </symbol>\n");
    advance(token_list);

    fprintf(*parse_out, "</subroutineBody>\n");
}

void compile_var_dec(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    char* var_name = NULL;
    char* var_type = NULL;
    kind_t var_kind = VAR_ki;

    fprintf(*parse_out, "<varDec>\n");

    fprintf(*parse_out, "<keyword> var </keyword>\n");
    advance(token_list);
    
    if((*token_list)->keyWord == INT)
        fprintf(*parse_out, "<keyword> int </keyword>\n");
    else if((*token_list)->keyWord == CHAR)
        fprintf(*parse_out, "<keyword> char </keyword>\n");
    else if((*token_list)->keyWord == BOOLEAN)
        fprintf(*parse_out, "<keyword> boolean </keyword>\n");
    else
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
    var_type = strdup((*token_list)->token);
    advance(token_list);
    
    fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
    var_name = strdup((*token_list)->token);
    define(*sym_table, var_name, var_type, var_kind);    
    advance(token_list);

    while((*token_list)->symbol == ',')
    {
        fprintf(*parse_out, "<symbol> , </symbol>\n");
        advance(token_list);
        free(var_name); 
        var_name = strdup((*token_list)->token);
        define(*sym_table, var_name, var_type, var_kind);    
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        advance(token_list);
    }

    fprintf(*parse_out, "<symbol> ; </symbol>\n");
    advance(token_list);

    fprintf(*parse_out, "</varDec>\n"); 

    if(var_name != NULL) free(var_name);
    if(var_type != NULL) free(var_type);
}

void compile_statements(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    fprintf(*parse_out, "<statements>\n"); 

    while((*token_list)->symbol != '}')
    {
        if((*token_list)->keyWord == LET)
            compile_let(parse_out, code_generate_out, token_list, sym_table);
        else if((*token_list)->keyWord == IF) 
            compile_if(parse_out, code_generate_out, token_list, sym_table);
        else if((*token_list)->keyWord == WHILE) 
            compile_while(parse_out, code_generate_out, token_list, sym_table);
        else if((*token_list)->keyWord == DO)
            compile_do(parse_out, code_generate_out, token_list, sym_table);
        else if((*token_list)->keyWord == RETURN)
            compile_return(parse_out, code_generate_out, token_list, sym_table);
        else{}
    }

    fprintf(*parse_out, "</statements>\n"); 
}

void compile_let(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    bool array_assign = FALSE;

    char* variable;

    fprintf(*parse_out, "<letStatement>\n"); 

    fprintf(*parse_out, "<keyword> let </keyword>\n");
    advance(token_list);

    fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
    variable = strdup((*token_list)->token);
    advance(token_list);

    if((*token_list)->symbol == '[')
    {
        array_assign = TRUE;

        if(kind_of(*sym_table, variable) == STATIC_ki) 
            write_push(code_generate_out, STATIC_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == VAR_ki) 
            write_push(code_generate_out, LOCAL_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == ARG_ki) 
            write_push(code_generate_out, ARG_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == FIELD_ki) 
            write_push(code_generate_out, THIS_s, index_of(*sym_table, variable));
 
        fprintf(*parse_out, "<symbol> [ </symbol>\n");
        advance(token_list);

        compile_expression(parse_out, code_generate_out, token_list, sym_table); 

        fprintf(*parse_out, "<symbol> ] </symbol>\n");
        advance(token_list);

        write_arithmetic(code_generate_out, '+');
    }

    fprintf(*parse_out, "<symbol> = </symbol>\n");
    advance(token_list);

    compile_expression(parse_out, code_generate_out, token_list, sym_table);

    fprintf(*parse_out, "<symbol> ; </symbol>\n");
    advance(token_list);
    
    fprintf(*parse_out, "</letStatement>\n"); 

    if(array_assign == TRUE)
    {
        write_pop(code_generate_out, TEMP_s, 0);
        write_pop(code_generate_out, POINTER_s, 1);
        write_push(code_generate_out, TEMP_s, 0);
        write_pop(code_generate_out, THAT_s, 0);
    }
    else
    {
        if(kind_of(*sym_table, variable) == STATIC_ki) 
            write_pop(code_generate_out, STATIC_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == VAR_ki) 
            write_pop(code_generate_out, LOCAL_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == ARG_ki) 
            write_pop(code_generate_out, ARG_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == FIELD_ki) 
            write_pop(code_generate_out, THIS_s, index_of(*sym_table, variable));
    }

    free(variable);
}

void compile_if(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    char if_l1[200];
    strcpy(if_l1, "if_l1_");
    char s_if_l1_index[200];
    sprintf(s_if_l1_index, "%d", if_l1_index);
    strcat(if_l1, s_if_l1_index);
    if_l1_index++;

    char if_l2[200];
    strcpy(if_l2, "if_l2_");
    char s_if_l2_index[200];
    sprintf(s_if_l2_index, "%d", if_l2_index);
    strcat(if_l2, s_if_l2_index);
    if_l2_index++;

    fprintf(*parse_out, "<ifStatement>\n");

    fprintf(*parse_out, "<keyword> if </keyword>\n");
    advance(token_list);   

    fprintf(*parse_out, "<symbol> ( </symbol>\n");
    advance(token_list);

    compile_expression(parse_out, code_generate_out, token_list, sym_table);

    fprintf(*parse_out, "<symbol> ) </symbol>\n");
    advance(token_list);

    write_unary(code_generate_out, '~');
    write_if(code_generate_out, if_l1);

    fprintf(*parse_out, "<symbol> { </symbol>\n");
    advance(token_list);

    compile_statements(parse_out, code_generate_out, token_list, sym_table);

    fprintf(*parse_out, "<symbol> } </symbol>\n");
    advance(token_list);

    write_goto(code_generate_out, if_l2);

    write_label(code_generate_out, if_l1);

    if((*token_list)->keyWord == ELSE)
    {
        fprintf(*parse_out, "<keyword> else </keyword>\n");
        advance(token_list);

        fprintf(*parse_out, "<symbol> { </symbol>\n");
        advance(token_list);

        compile_statements(parse_out, code_generate_out, token_list, sym_table);

        fprintf(*parse_out, "<symbol> } </symbol>\n");
        advance(token_list);  
    }

    write_label(code_generate_out, if_l2);
    
    fprintf(*parse_out, "</ifStatement>\n");
}

void compile_while(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    char while_l1[200];
    strcpy(while_l1, "while_l1_");
    char s_while_l1_index[200];
    sprintf(s_while_l1_index, "%d", while_l1_index);
    strcat(while_l1, s_while_l1_index);
    while_l1_index++;

    char while_l2[200];
    strcpy(while_l2, "while_l2_");
    char s_while_l2_index[200];
    sprintf(s_while_l2_index, "%d", while_l2_index);
    strcat(while_l2, s_while_l2_index);
    while_l2_index++;

    fprintf(*parse_out, "<whileStatement>\n");

    fprintf(*parse_out, "<keyword> while </keyword>\n");
    advance(token_list); 

    write_label(code_generate_out, while_l1);

    fprintf(*parse_out, "<symbol> ( </symbol>\n");
    advance(token_list);

    compile_expression(parse_out, code_generate_out, token_list, sym_table);

    fprintf(*parse_out, "<symbol> ) </symbol>\n");
    advance(token_list);

    write_unary(code_generate_out, '~');
    write_if(code_generate_out, while_l2);

    fprintf(*parse_out, "<symbol> { </symbol>\n");
    advance(token_list);

    compile_statements(parse_out, code_generate_out, token_list, sym_table);

    fprintf(*parse_out, "<symbol> } </symbol>\n");
    advance(token_list);    

    write_goto(code_generate_out, while_l1);
    
    fprintf(*parse_out, "</whileStatement>\n");

    write_label(code_generate_out, while_l2);
}

void compile_do(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    int nArgs = 0;
    
    fprintf(*parse_out, "<doStatement>\n");

    fprintf(*parse_out, "<keyword> do </keyword>\n");
    advance(token_list);    

    if((*token_list)->tokenType == IDENTIFIER && ((*token_list)->next)->symbol == '(')
    {
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        g_caller_name_set(g_class_name,
                    (*token_list)->token);
        advance(token_list);

        fprintf(*parse_out, "<symbol> ( </symbol>\n");
        advance(token_list);

        write_push(code_generate_out, POINTER_s, 0);

        nArgs = compile_expression_list(parse_out, code_generate_out, token_list, sym_table) + 1;

        fprintf(*parse_out, "<symbol> ) </symbol>\n");
        advance(token_list);
    }
    else if((*token_list)->tokenType == IDENTIFIER && ((*token_list)->next)->symbol == '.')
    {
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        if(kind_of(*sym_table, (*token_list)->token) == NONE_ki)
            g_caller_name_set((*token_list)->token,
                                (*token_list)->next->next->token);
        else
        {
            g_caller_name_set(type_of(*sym_table, (*token_list)->token),
                                (*token_list)->next->next->token);
            
            char* variable = (*token_list)->token;

            if(kind_of(*sym_table, variable) == STATIC_ki) 
                write_push(code_generate_out, STATIC_s, index_of(*sym_table, variable));
            else if(kind_of(*sym_table, variable) == VAR_ki) 
                write_push(code_generate_out, LOCAL_s, index_of(*sym_table, variable));
            else if(kind_of(*sym_table, variable) == ARG_ki) 
                write_push(code_generate_out, ARG_s, index_of(*sym_table, variable));
            else if(kind_of(*sym_table, variable) == FIELD_ki) 
                write_push(code_generate_out, THIS_s, index_of(*sym_table, variable));

            nArgs++;
        }

        advance(token_list);

        fprintf(*parse_out, "<symbol> . </symbol>\n");
        advance(token_list);
        
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        advance(token_list);
        
        fprintf(*parse_out, "<symbol> ( </symbol>\n");
        advance(token_list);

        nArgs += compile_expression_list(parse_out, code_generate_out, token_list, sym_table);

        fprintf(*parse_out, "<symbol> ) </symbol>\n");
        advance(token_list);
    }
    else{}

    fprintf(*parse_out, "<symbol> ; </symbol>\n");
    advance(token_list);

    fprintf(*parse_out, "</doStatement>\n");

    write_call(code_generate_out, g_caller_name, nArgs);
    write_pop(code_generate_out, TEMP_s, 0);
}

void compile_return(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    fprintf(*parse_out, "<returnStatement>\n");

    fprintf(*parse_out, "<keyword> return </keyword>\n");
    advance(token_list);

    if((*token_list)->symbol != ';')
        compile_expression(parse_out, code_generate_out, token_list, sym_table);
    else
    {
        write_push(code_generate_out, CONST_s, 0);
    }
    
    fprintf(*parse_out, "<symbol> ; </symbol>\n");
    advance(token_list);

    fprintf(*parse_out, "</returnStatement>\n");

    write_return(code_generate_out);
}

void compile_expression(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    fprintf(*parse_out, "<expression>\n"); 

    compile_term(parse_out, code_generate_out, token_list, sym_table);

    while(isop((*token_list)->symbol))
    {
        char op = (*token_list)->symbol;
        fprintf(*parse_out, "<symbol> ");
        if((*token_list)->symbol == '>')
            fprintf(*parse_out, "&gt;");       
        else if((*token_list)->symbol == '<')
            fprintf(*parse_out, "&lt;");       
        else if((*token_list)->symbol == '"')
            fprintf(*parse_out, "&quot;");       
        else if((*token_list)->symbol == '&')
            fprintf(*parse_out, "&amp;");       
        else
            fprintf(*parse_out, "%s", (*token_list)->token);       
        fprintf(*parse_out, " </symbol>\n");       
        advance(token_list);

        compile_term(parse_out, code_generate_out, token_list, sym_table);

        write_arithmetic(code_generate_out, op);
    }

    fprintf(*parse_out, "</expression>\n"); 
}

void compile_term(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    fprintf(*parse_out, "<term>\n"); 

    if((*token_list)->tokenType == INT_CONST)
    {
        fprintf(*parse_out, "<integerConstant> %s </integerConstant>\n", (*token_list)->token);
        write_push(code_generate_out, CONST_s, (*token_list)->intVal);
        advance(token_list);
    }
    else if((*token_list)->tokenType == STRING_CONST)
    {
        fprintf(*parse_out, "<stringConstant> %s </stringConstant>\n", (*token_list)->token);
        int len = strlen((*token_list)->token);
        write_push(code_generate_out, CONST_s, len);
        write_call(code_generate_out, "String.new", 1);
        for(int i = 0; i < len; i++)
        {
            write_push(code_generate_out, CONST_s, (int)(((*token_list)->token)[i]));
            write_call(code_generate_out, "String.appendChar", 2);
        }

        advance(token_list);
    }
    else if((*token_list)->tokenType == KEYWORD)
    {
        fprintf(*parse_out, "<keyword> %s </keyword>\n", (*token_list)->token);
        if((*token_list)->keyWord == TRUE_k)
        {
            write_push(code_generate_out, CONST_s, 1);
            write_unary(code_generate_out, '-');
        }
        else if((*token_list)->keyWord == FALSE_k)
            write_push(code_generate_out, CONST_s, 0);
        else if((*token_list)->keyWord == NULL_k)
            write_push(code_generate_out, CONST_s, 0);
        else if((*token_list)->keyWord == THIS)
            write_push(code_generate_out, POINTER_s, 0);
        else{}
        advance(token_list);
    }
    else if((*token_list)->tokenType == SYMBOL &&
            ((*token_list)->symbol == '-' || (*token_list)->symbol == '~'))
            {
                char unaryop = (*token_list)->symbol;

                fprintf(*parse_out, "<symbol> %s </symbol>\n", (*token_list)->token);
                advance(token_list);

                compile_term(parse_out, code_generate_out, token_list, sym_table);

                write_unary(code_generate_out, unaryop);
            }
    else if((*token_list)->tokenType == SYMBOL && (*token_list)->symbol == '(')
    {
        fprintf(*parse_out, "<symbol> ( </symbol>\n");
        advance(token_list);

        compile_expression(parse_out, code_generate_out, token_list, sym_table);

        fprintf(*parse_out, "<symbol> ) </symbol>\n");
        advance(token_list);
    }
    else if((*token_list)->tokenType == IDENTIFIER && ((*token_list)->next)->symbol == '[')
    {
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);

        char* variable = (*token_list)->token;
        if(kind_of(*sym_table, variable) == STATIC_ki) 
            write_push(code_generate_out, STATIC_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == VAR_ki) 
            write_push(code_generate_out, LOCAL_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == ARG_ki) 
            write_push(code_generate_out, ARG_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == FIELD_ki) 
            write_push(code_generate_out, THIS_s, index_of(*sym_table, variable));

        advance(token_list);

        fprintf(*parse_out, "<symbol> [ </symbol>\n");
        advance(token_list);

        compile_expression(parse_out, code_generate_out, token_list, sym_table);

        write_arithmetic(code_generate_out, '+');

        fprintf(*parse_out, "<symbol> ] </symbol>\n");
        advance(token_list);

        write_pop(code_generate_out, POINTER_s, 1);
        write_push(code_generate_out, THAT_s, 0);
    }
    else if((*token_list)->tokenType == IDENTIFIER && ((*token_list)->next)->symbol == '(')
    {
        int nArgs = 0;

        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        g_caller_name_set(g_class_name, (*token_list)->token);
        advance(token_list);

        fprintf(*parse_out, "<symbol> ( </symbol>\n");
        advance(token_list);

        write_push(code_generate_out, POINTER_s, 0);

        nArgs = compile_expression_list(parse_out, code_generate_out, token_list, sym_table) + 1;

        fprintf(*parse_out, "<symbol> ) </symbol>\n");
        advance(token_list);

        write_call(code_generate_out, g_caller_name, nArgs); 
    }
    else if((*token_list)->tokenType == IDENTIFIER && ((*token_list)->next)->symbol == '.')
    {
        int nArgs = 0;

        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        if(kind_of(*sym_table, (*token_list)->token) == NONE_ki)
            g_caller_name_set((*token_list)->token,
                                (*token_list)->next->next->token);
        else
        {
            g_caller_name_set(type_of(*sym_table, (*token_list)->token),
                                (*token_list)->next->next->token);

            char* variable = (*token_list)->token;
            if(kind_of(*sym_table, variable) == STATIC_ki) 
                write_push(code_generate_out, STATIC_s, index_of(*sym_table, variable));
            else if(kind_of(*sym_table, variable) == VAR_ki) 
                write_push(code_generate_out, LOCAL_s, index_of(*sym_table, variable));
            else if(kind_of(*sym_table, variable) == ARG_ki) 
                write_push(code_generate_out, ARG_s, index_of(*sym_table, variable));
            else if(kind_of(*sym_table, variable) == FIELD_ki) 
                write_push(code_generate_out, THIS_s, index_of(*sym_table, variable));

            nArgs++;
        }

        advance(token_list);

        fprintf(*parse_out, "<symbol> . </symbol>\n");
        advance(token_list);
        
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        advance(token_list);
        
        fprintf(*parse_out, "<symbol> ( </symbol>\n");
        advance(token_list);

        nArgs += compile_expression_list(parse_out, code_generate_out, token_list, sym_table);

        fprintf(*parse_out, "<symbol> ) </symbol>\n");
        advance(token_list);

        write_call(code_generate_out, g_caller_name, nArgs); 
    }
    else if((*token_list)->tokenType == IDENTIFIER)
    {
        char* variable = (*token_list)->token;
        fprintf(*parse_out, "<identifier> %s </identifier>\n", (*token_list)->token);
        if(kind_of(*sym_table, variable) == STATIC_ki) 
            write_push(code_generate_out, STATIC_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == VAR_ki) 
            write_push(code_generate_out, LOCAL_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == ARG_ki) 
            write_push(code_generate_out, ARG_s, index_of(*sym_table, variable));
        else if(kind_of(*sym_table, variable) == FIELD_ki) 
            write_push(code_generate_out, THIS_s, index_of(*sym_table, variable));

        advance(token_list);
    }
    else{}

    fprintf(*parse_out, "</term>\n"); 
}

int compile_expression_list(FILE** parse_out, FILE **code_generate_out, token_node** token_list, symbol_table** sym_table)
{
    int nArgs = 0;
    
    fprintf(*parse_out, "<expressionList>\n");

    if((*token_list)->symbol != ')')
    {
        compile_expression(parse_out, code_generate_out, token_list, sym_table);
        nArgs++;
    }

    while((*token_list)->symbol == ',')
    {
        fprintf(*parse_out, "<symbol> , </symbol>\n");
        advance(token_list);

        compile_expression(parse_out, code_generate_out, token_list, sym_table);
        nArgs++;
    }

    fprintf(*parse_out, "</expressionList>\n");

    return nArgs;
}

void compile(FILE **parse_out, FILE **code_generate_out, token_node **token_list, symbol_table** sym_table)
{
    if((*token_list)->keyWord == CLASS)
    {
        compile_class(parse_out, code_generate_out, token_list, sym_table);
    }
    else
    {
        printf("Something is very wrong.\n");
    }
}
void write_push(FILE **code_generate_out, segment_t segment, int index)
{
    fprintf(*code_generate_out, "push");

    if(segment == CONST_s)
        fprintf(*code_generate_out, " constant ");
    else if(segment == ARG_s)
        fprintf(*code_generate_out, " argument ");
    else if(segment == LOCAL_s)
        fprintf(*code_generate_out, " local ");
    else if(segment == STATIC_s)
        fprintf(*code_generate_out, " static ");
    else if(segment == THIS_s)
        fprintf(*code_generate_out, " this ");
    else if(segment == THAT_s)
        fprintf(*code_generate_out, " that ");
    else if(segment == POINTER_s)
        fprintf(*code_generate_out, " pointer ");
    else if(segment == TEMP_s)
        fprintf(*code_generate_out, " temp ");
    else{}

    fprintf(*code_generate_out, "%d\n", index);
}

void write_pop(FILE **code_generate_out, segment_t segment, int index)
{
    fprintf(*code_generate_out, "pop");

    if(segment == CONST_s)
        fprintf(*code_generate_out, " constant ");
    else if(segment == ARG_s)
        fprintf(*code_generate_out, " argument ");
    else if(segment == LOCAL_s)
        fprintf(*code_generate_out, " local ");
    else if(segment == STATIC_s)
        fprintf(*code_generate_out, " static ");
    else if(segment == THIS_s)
        fprintf(*code_generate_out, " this ");
    else if(segment == THAT_s)
        fprintf(*code_generate_out, " that ");
    else if(segment == POINTER_s)
        fprintf(*code_generate_out, " pointer ");
    else if(segment == TEMP_s)
        fprintf(*code_generate_out, " temp ");
    else{}

    fprintf(*code_generate_out, "%d\n", index);
}

void write_arithmetic(FILE **code_generate_out, char command)
{
    if(command == '+')
        fprintf(*code_generate_out, "add");
    else if(command == '-')
        fprintf(*code_generate_out, "sub");
    else if(command == '=')
        fprintf(*code_generate_out, "eq");
    else if(command == '>')
        fprintf(*code_generate_out, "gt");
    else if(command == '<')
        fprintf(*code_generate_out, "lt");
    else if(command == '&')
        fprintf(*code_generate_out, "and");
    else if(command == '|')
        fprintf(*code_generate_out, "or");
    else if(command == '*')
        fprintf(*code_generate_out, "call Math.multiply 2");
    else if(command == '/')
        fprintf(*code_generate_out, "call Math.divide 2");
    else{}
    fprintf(*code_generate_out, "\n");
}

void write_unary(FILE **code_generate_out, char command)
{
    if(command == '-')
        fprintf(*code_generate_out, "neg");
    else if(command == '~')
        fprintf(*code_generate_out, "not");
    else{}
    fprintf(*code_generate_out, "\n");
}

void write_label(FILE **code_generate_out, char* label)
{
    fprintf(*code_generate_out, "label %s\n", label);
}

void write_goto(FILE **code_generate_out, char* label)
{
    fprintf(*code_generate_out, "goto %s\n", label);
}

void write_if(FILE **code_generate_out, char* label)
{
    fprintf(*code_generate_out, "if-goto %s\n", label);
}

void write_call(FILE **code_generate_out, char* name, int nArgs)
{
    fprintf(*code_generate_out, "call %s %d\n", name, nArgs);
}

void write_function(FILE **code_generate_out, char* name, int nLocals)
{
    fprintf(*code_generate_out, "function %s %d\n", name, nLocals);
}

void write_return(FILE **code_generate_out)
{
    fprintf(*code_generate_out, "return\n");
}

void translate(FILE **in, FILE **tokenize_out, FILE **parse_out, FILE **code_generate_out)
{
    char buffer_in[1000];

    token_node* token_list = NULL;

    symbol_table* sym_table = create_new_symbol_table();

    while (fgets(buffer_in, 1000, *in) != NULL)
    {
        char instruction[1000];
        strcpy(instruction, buffer_in); 

        if(instruction[strlen(instruction) - 1] == '\n') 
            instruction[strlen(instruction) - 1] = '\0'; // -1 in mac/linux, -2 in windows

        char *ret;
        ret = strstr(instruction, "//");
        if (ret != NULL) *ret = '\0';

        ret = strstr(instruction, "/*");
        if (ret != NULL) g_key_ignore = TRUE;

        ret = strstr(instruction, "*/");
        if (ret != NULL)
        {
            g_key_ignore = FALSE;
            strcpy(instruction, ret + 2);
        }

        if(!g_key_ignore)
        {
            tokenize_and_add_to_token_list(instruction, &token_list);

            // parse(&tt, &pp);
        }
    }

    write_tokens_to_file(tokenize_out, &token_list);
    compile(parse_out, code_generate_out, &token_list, &sym_table);

    destroy_symbol_table(sym_table);
}

int main(int argc, char *argv[])
{
    FILE *in;
    FILE *tokenize_out;
    FILE *parse_out;
    FILE *code_generate_out;
    DIR *dir;

    char* arg1 = *(argv+1);
    if(arg1[strlen(arg1) - 1] == '/')
        arg1[strlen(arg1) - 1] = '\0';

    if(strstr(arg1, ".jack") != NULL)
    {
        // jack file
        char file_without_extension[501];
        sscanf(arg1, "%[^.]", file_without_extension);

        char* slash = strrchr(file_without_extension, '/');

        char jack_in[501];
        strcpy(jack_in, file_without_extension);
        strcat(jack_in, ".jack");
        in = fopen(jack_in, "r");
        if(in == NULL){printf("jack file not found\n"); exit(1);}

        char tokenize_xml_out[501];
        strcpy(tokenize_xml_out, file_without_extension);
        strcat(tokenize_xml_out, "T");
        strcat(tokenize_xml_out, ".xml");
        tokenize_out = fopen(tokenize_xml_out, "w");
        if(tokenize_out == NULL){printf("failed to create xml file for tokens\n"); exit(1);}
        
        char parse_xml_out[501];
        strcpy(parse_xml_out, file_without_extension);
        strcat(parse_xml_out, ".xml");
        parse_out = fopen(parse_xml_out, "w");
        if(parse_out == NULL){printf("failed to create xml file for parse tree\n"); exit(1);}

        char code_generate_vm_out[501];
        strcpy(code_generate_vm_out, file_without_extension);
        strcat(code_generate_vm_out, ".vm");
        code_generate_out = fopen(code_generate_vm_out, "w");
        if(code_generate_out == NULL){printf("failed to create vm file\n"); exit(1);}

        translate(&in, &tokenize_out, &parse_out, &code_generate_out); 

        fclose(in);
        fclose(tokenize_out);
        fclose(parse_out);
        fclose(code_generate_out);
    }
    else
    {
        // directory

        dir = opendir(arg1);
        if (dir == NULL) {printf("directory not found\n");exit(1);}

        struct dirent* entity;
        entity = readdir(dir);
        while (entity != NULL) {
            if (strstr(entity->d_name, ".jack") != NULL)
            {
                // printf("translating %s\n", entity->d_name);

                char file_without_extension[501];
                sscanf(entity->d_name, "%[^.]", file_without_extension);

                char jack_in[501];
                strcpy(jack_in, arg1);
                strcat(jack_in, "/");
                strcat(jack_in, entity->d_name);
                in = fopen(jack_in, "r");
                if(in == NULL){printf("jack file not found\n"); exit(1);} 

                char tokenize_xml_out[501];
                strcpy(tokenize_xml_out, arg1);
                strcat(tokenize_xml_out, "/");
                strcat(tokenize_xml_out, file_without_extension);
                strcat(tokenize_xml_out, "T");
                strcat(tokenize_xml_out, ".xml");
                tokenize_out = fopen(tokenize_xml_out, "w");
                if(tokenize_out == NULL){printf("failed to create xml file for tokens\n"); exit(1);}
                
                char parse_xml_out[501];
                strcpy(parse_xml_out, arg1);
                strcat(parse_xml_out, "/");
                strcat(parse_xml_out, file_without_extension);
                strcat(parse_xml_out, ".xml");
                parse_out = fopen(parse_xml_out, "w");
                if(parse_out == NULL){printf("failed to create xml file\n"); exit(1);}

                char code_generate_vm_out[501];
                strcpy(code_generate_vm_out, arg1);
                strcat(code_generate_vm_out, "/");
                strcat(code_generate_vm_out, file_without_extension);
                strcat(code_generate_vm_out, ".vm");
                code_generate_out = fopen(code_generate_vm_out, "w");
                if(code_generate_out == NULL){printf("failed to create vm file\n"); exit(1);}

                translate(&in, &tokenize_out, &parse_out, &code_generate_out);

                fclose(in);
                fclose(tokenize_out);
                fclose(parse_out);
                fclose(code_generate_out);
            }
            entity = readdir(dir);
        }
        closedir(dir);
    }

    return 0;
}
