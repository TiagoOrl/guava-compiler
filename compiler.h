#ifndef PEACHCOMPILER_H
#define PEACHCOMPILER_H

#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#define S_EQ(str, str2) \
        (str && str2 && (strcmp(str, str2) == 0))


struct pos
{
    int line;
    int col;
    const char* filename;
};


#define NUMERIC_CASE \
    case '0':       \
    case '1':       \
    case '2':       \
    case '3':       \
    case '4':       \
    case '5':       \
    case '6':       \
    case '7':       \
    case '8':       \
    case '9' 

#define OPERATOR_CASE_EXCLUDING_DIVISION \
    case '+':                           \
    case '-':                           \
    case '*':                           \
    case '>':                           \
    case '<':                           \
    case '^':                           \
    case '%':                           \
    case '!':                           \
    case '=':                           \
    case '~':                           \
    case '|':                           \
    case '&':                           \
    case '(':                           \
    case '[':                           \
    case ',':                           \
    case '.':                           \
    case '?'                          

#define SYMBOL_CASE \
    case '{':       \
    case '}':       \
    case ':':       \
    case ';':       \
    case '#':       \
    case '\\':      \
    case ')':       \
    case ']'      
enum
{
    LEXICAL_ANALYSIS_ALL_OK,
    LEXICAL_ANALYSIS_INPUT_ERROR
};

enum
{
    TOKEN_TYPE_IDENTIFIER,
    TOKEN_TYPE_KEYWORD,
    TOKEN_TYPE_OPERATOR,
    TOKEN_TYPE_SYMBOL,
    TOKEN_TYPE_NUMBER,
    TOKEN_TYPE_STRING,
    TOKEN_TYPE_COMMENT,
    TOKEN_TYPE_NEWLINE
};


enum
{
    NUMBER_TYPE_NORMAL,
    NUMBER_TYPE_LONG,
    NUMBER_TYPE_FLOAT,
    NUMBER_TYPE_DOUBLE
};

struct token
{
    int type;
    int flags;
    struct pos pos;
    union
    {
        char cval;
        const char* sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
        void* any;
    };
    
    struct token_number
    {
        int type;
    } num;

    // True if their is whitespace between the token and the next token
    // i.e * a for operator token * would mean whitespace would be set for token "a"
    bool whitespace;

    // (5+10+20)
    const char* between_brackets;

};

struct lex_process;
typedef char (*LEX_PROCESS_NEXT_CHAR)(struct lex_process* process);
typedef char (*LEX_PROCESS_PEEK_CHAR)(struct lex_process* process);
typedef void (*LEX_PROCESS_PUSH_CHAR)(struct lex_process* process, char c);

struct lex_process_functions
{
    LEX_PROCESS_NEXT_CHAR next_char;
    LEX_PROCESS_PEEK_CHAR peek_char;
    LEX_PROCESS_PUSH_CHAR push_char;
};

struct lex_process
{
    struct pos pos;
    struct vector* token_vec;
    struct compile_process* compiler;

    /**
     * 
     * ((50))
     */
    int current_expression_count;
    struct buffer* parentheses_buffer;
    struct lex_process_functions* function;

    // This will be private data that the lexer does not understand
    // but the person using the lexer does understand.
    void* private;
};

enum
{
    COMPILER_FILE_COMPILED_OK,
    COMPILER_FAILED_WITH_ERRORS
};


struct scope
{
    int flags;
    // void*
    struct vector* entities;

    // total number of bytes this scope uses. Aligned with 16 bytes
    size_t size;

    // NULL if no parent
    struct scope* parent;
};


enum
{
    SYMBOL_TYPE_NODE,
    SYMBOL_TYPE_NATIVE_FUNCTION,
    SYMBOL_TYPE_UNKNOWN
};


struct symbol
{
    const char* name;
    int type;
    void* data;
};


struct compile_process
{
    // The flags in regards to how this file should be compiled
    int flags;

    struct pos pos;
    struct compile_process_input_file
    {
        FILE* fp;
        const char* abs_path;
    } cfile;


    // A vector of tokens from lexical analysis.
    struct vector* token_vec;

    struct vector* node_vec;
    struct vector* node_tree_vec;
    FILE* ofile;

    struct 
    {
        struct scope* root;
        struct scope* current;
    } scope;

    struct 
    {
        // Current active symbol table. struct symbol*
        struct vector* table;
        // struct vector* multiple symbol tables stored in here..
        struct vector* tables;
    }symbols;
};


enum
{
    PARSE_ALL_OK,
    PARSE_GENERAL_ERROR
};

enum
{
    NODE_TYPE_EXPRESSION,
    NODE_TYPE_EXPRESSION_PARENTHESES,
    NODE_TYPE_NUMBER,
    NODE_TYPE_IDENTIFIER,
    NODE_TYPE_STRING,
    NODE_TYPE_VARIABLE,
    NODE_TYPE_VARIABLE_LIST,
    NODE_TYPE_FUNCTION,
    NODE_TYPE_BODY,
    NODE_TYPE_STATEMENT_RETURN,
    NODE_TYPE_STATEMENT_IF,
    NODE_TYPE_STATEMENT_ELSE,
    NODE_TYPE_STATEMENT_WHILE,
    NODE_TYPE_STATEMENT_DO_WHILE,
    NODE_TYPE_STATEMENT_FOR,
    NODE_TYPE_STATEMENT_BREAK,
    NODE_TYPE_STATEMENT_CONTINUE,
    NODE_TYPE_STATEMENT_SWITCH,
    NODE_TYPE_STATEMENT_CASE,
    NODE_TYPE_STATEMENT_DEFAULT,
    NODE_TYPE_STATEMENT_GOTO,

    NODE_TYPE_UNARY,
    NODE_TYPE_TENARY,
    NODE_TYPE_LABEL,
    NODE_TYPE_STRUCT,
    NODE_TYPE_UNION,
    NODE_TYPE_BRACKET,
    NODE_TYPE_CAST,
    NODE_TYPE_BLANK
};

enum
{
    NODE_FLAG_INSIDE_EXPRESSION      = 0b00000001,
    NODE_FLAG_IS_FORWARD_DECLARATION = 0b00000010,
    NODE_FLAG_HAS_VARIABLE_COMBINED  = 0b00000100
};

struct array_brackets
{
    // vector of struct node*
    struct vector* n_brackets;

};

struct node;
struct datatype
{
    int flags;
    // i.e type of long, int, float ect..
    int type;

    // i.e long int. int being the secondary.
    struct datatype* secondary;
    // long
    const char* type_str;
    // The sizeof the datatype.
    size_t size;
    int pointer_depth;

    union
    {
       struct node* struct_node;
       struct node* union_node;
    };
    
    struct array
    {
        struct array_brackets* brackets;
        // total array size = type * array_size * array_size * ...
        size_t size;
    } array;
};


struct parsed_switch_case
{
    // Index of the parsed case
    int index;
};


struct node 
{
    int type;
    int flags;

    struct pos pos;

    struct node_binded
    {
        // Pointer to our body node
        struct node* owner; 

        // Pointer to the function this node is in.
        struct node* function;
    } binded;
    
    union
    {
        struct exp
        {
            struct node* left;
            struct node* right;
            const char* op;
        } exp;

        struct parenthesis
        {
            // the expression inside the parenthesis node
            struct node* exp;
        } parenthesis;

        struct var
        {
            struct datatype type;
            const char* name;
            struct node* val;
            int padding;
            // aligned offset
            int aoffset;
        } var;

        struct node_ternary
        {
            struct node* true_node;
            struct node* false_node;
        } ternary;

        struct varlist
        {
            // list of struct node* variables
            struct vector* list;
        } var_list;

        struct bracket
        {   
            //  int x[50] : [50] would be the bracket node. The inner would be NODE_TYPE_NUMBER value of 50
            struct node* inner;
        } bracket;

        struct _union
        {
            const char* name;
            struct node* body_n;
            /**
             * struct abc
             * {
             * 
             * } var_name;
             * 
             * NULL if no variable attached to structure
             */
            struct node* var;
        } _union;

        struct _struct
        {
            const char* name;
            struct node* body_n;
            /**
             * struct abc
             * {
             * 
             * } var_name;
             * 
             * NULL if no variable attached to structure
             */
            struct node* var;
        } _struct;

        struct body
        {
            //struct node* of vector of statements
            struct vector* statements;
            // the sum of the combined variables inside this body
            size_t size;
            // true if the variable size had to be increased due to padding in body
            bool padded;
            // pointer to the largest variable node in the statements vector
            struct node* largest_var_node;
        } body;

        struct function
        {   
            int flags;

            // return type i.e.  void, int, long, etc...
            struct datatype rtype;
            // the function name
            const char* name;

            struct function_arguments
            {
                // vector of struct node*, must be NODE_TYPE_VARIABLE
                struct vector* vector;

                // How much to add to the EBP to find the first argument
                size_t stack_addition;
            } args;

            // Pointer to the function body node, NULL if this is a function prototype
            struct node* body_n;

            // the stack size for all variables inside this function.
            size_t stack_size;
        } func;

        struct statement
        {
            struct return_stmt
            {
                // The expression of the return i.e. return (40 + x - y ...)
                struct node* exp;
            } return_stmt;

            struct if_stmt
            {
                // if (COND)
                struct node* cond_node;
                // {}
                struct node* body_node;
                // else {}
                struct node* next;
            } if_stmt;

            struct else_stmt
            {
                struct node* body_node;
            } else_stmt;

            struct for_stmt
            {
                struct node* init_node;
                struct node* cond_node;
                struct node* loop_node;
                struct node* body_node;
            } for_stmt;

            struct while_stmt
            {
                struct node* exp_node;
                struct node* body_node;
            } while_stmt;


            struct do_while_stmt 
            {
                struct node* exp_node;
                struct node* body_node;
            } do_while_stmt;

            struct switch_stmt
            {
                struct node* exp;
                struct node* body;
                struct vector* cases;
                bool has_default_case;
            } switch_stmt;


            struct _case_stmt
            {
                struct node* exp_node;
            } _case;

            struct _goto_stmt
            {
                struct node* label;
            } _goto;

        } stmt;

        struct node_label
        {
            struct node* name;
        } label;

        struct cast
        {
            struct datatype dtype;
            struct node* operand_node;
        } cast;
    };
    
    union 
    {
        char cval;
        const char* sval;
        unsigned int inum;
        unsigned long lnum;
        unsigned long long llnum;
    };
};

enum
{
    DATATYPE_FLAG_IS_SIGNED = 0b00000001,
    DATATYPE_FLAG_IS_STATIC = 0b00000010,
    DATATYPE_FLAG_IS_CONST = 0b00000100,
    DATATYPE_FLAG_IS_POINTER = 0b00001000,
    DATATYPE_FLAG_IS_ARRAY = 0b00010000,
    DATATYPE_FLAG_IS_EXTERN = 0b00100000,
    DATATYPE_FLAG_IS_RESTRICT = 0b01000000,
    DATATYPE_FLAG_IGNORE_TYPE_CHECKING = 0b10000000,
    DATATYPE_FLAG_IS_SECONDARY = 0b100000000,
    DATATYPE_FLAG_STRUCT_UNION_NO_NAME = 0b1000000000,
    DATATYPE_FLAG_IS_LITERAL = 0b10000000000,
};
enum
{
    DATA_TYPE_VOID,
    DATA_TYPE_CHAR,
    DATA_TYPE_SHORT,
    DATA_TYPE_INTEGER,
    DATA_TYPE_LONG,
    DATA_TYPE_FLOAT,
    DATA_TYPE_DOUBLE,
    DATA_TYPE_STRUCT,
    DATA_TYPE_UNION,
    DATA_TYPE_UNKNOWN
};


enum
{
    DATA_TYPE_EXPECT_PRIMITIVE,
    DATA_TYPE_EXPECT_UNION,
    DATA_TYPE_EXPECT_STRUCT
};

enum
{
    DATA_SIZE_ZERO = 0,
    DATA_SIZE_BYTE = 1,
    DATA_SIZE_WORD = 2,
    DATA_SIZE_DWORD = 4,
    DATA_SIZE_DDWORD = 8
};

enum
{
    FUNCTION_NODE_FLAG_IS_NATIVE = 0b00000001,
};

int compile_file(const char* filename, const char* out_filename, int flags);
struct compile_process *compile_process_create(const char *filename, const char *filename_out, int flags);


char compile_process_next_char(struct lex_process* lex_process);
char compile_process_peek_char(struct lex_process* lex_process);
void compile_process_push_char(struct lex_process* lex_process, char c);


void compiler_error(struct compile_process* compiler, const char* msg, ...);
void compiler_warning(struct compile_process* compiler, const char* msg, ...);

struct lex_process* lex_process_create(struct compile_process* compiler, struct lex_process_functions* functions, void* private);
void lex_process_free(struct lex_process* process);
void* lex_process_private(struct lex_process* process);
struct vector* lex_process_tokens(struct lex_process* process);
int lex(struct lex_process* process);
int parse(struct compile_process* process);

/**
 * @brief Builds tokens for the input string.
 * 
 * @param compiler 
 * @param str 
 * @return struct lex_process* 
 */
struct lex_process* tokens_build_for_string(struct compile_process* compiler, const char* str);


bool token_is_identifier(struct token* token);
bool token_is_keyword(struct token* token, const char* value);
bool token_is_symbol(struct token* token, char c);

bool token_is_nl_or_comment_or_newline_seperator(struct token *token);
bool keyword_is_datatype(const char *str);
bool token_is_primitive_keyword(struct token* token);

size_t datatype_size_for_array_access(struct datatype* dtype);
size_t datatype_element_size(struct datatype* dtype);
size_t datatype_size_no_ptr(struct datatype* dtype);
size_t datatype_size(struct datatype* dtype);
bool datatype_is_struct_or_union_for_name(const char* name);
bool datatype_is_struct_or_union(struct datatype* dtype);
bool datatype_is_primitive(struct datatype* dtype);
bool token_is_operator(struct token* token, const char* val);

struct node* variable_struct_or_union_body_node(struct node* node);

// get the var size of the given variable node
size_t variable_size(struct node* var_node);
struct node* variable_node(struct node* node);
bool variable_node_is_primitive(struct node* node);
// sums the variable size of all variable nodes inside the variable list node
size_t variable_size_for_list(struct node* var_list_node);
struct node* variable_node_or_list(struct node* node);
int padding(int val, int to);
int align_value(int val, int to);
int align_value_treat_positive(int val, int to);
int compute_sum_padding(struct vector* vec);

struct scope* scope_new(struct compile_process* process, int flags);
struct scope* scope_create_root(struct compile_process* process);
void scope_free_root(struct compile_process* process);
void scope_iteration_start(struct scope* scope);
void scope_iteration_end(struct scope* scope);
void* scope_iterate_back(struct scope* scope);
void* scope_last_entity_at_scope(struct scope* scope);
void* scope_last_entity_from_scope_stop_at(struct scope* scope, struct scope* stop_scope);
void* scope_last_entity_stop_at(struct compile_process* process, struct scope* stop_scope);
void* scope_last_entity(struct compile_process* process);
void scope_push(struct compile_process* process, void* ptr, size_t elem_size);
void scope_finish(struct compile_process* process);
struct scope* scope_current(struct compile_process* process);

struct symbol* symresolver_get_symbol(struct compile_process* process, const char* name);
struct symbol* symresolver_get_symbol_for_native_function(struct compile_process* process, const char* name);
void symresolver_build_for_node(struct compile_process* process, struct node* node);
void symresolver_initialize(struct compile_process* process);
void symresolver_new_table(struct compile_process* process);
void symresolver_end_table(struct compile_process* process);


size_t function_node_argument_stack_addition(struct node* node);
struct node* node_create(struct node* _node);
struct node* node_from_sym(struct symbol* sym);
struct node* node_from_symbol(struct compile_process* current_process, const char* name);
struct node* union_node_for_name(struct compile_process* current_process, const char* name);
struct node* struct_node_for_name(struct compile_process* current_process, const char* name);
void make_cast_node(struct datatype* dtype, struct node* operand_node);
void make_label_node(struct node* label_name_node);
void make_ternary_node(struct node* true_result_node, struct node* false_result_node);
void make_goto_node(struct node* label_node);
void make_continue_node();
void make_case_node(struct node* exp_node);
void make_break_node();
void make_if_node(struct node* cond_node, struct node* body_node, struct node* next_node);
void make_else_node(struct node* body_node);
void make_union_node(const char* name, struct node* body_node);
void make_struct_node(const char* name, struct node* body_node);
void make_exp_node(struct node* left_node, struct node* right_node, const char* op);
void make_exp_parentheses_node(struct node* exp_node);
void make_bracket_node(struct node* node);
void make_body_node(struct vector* body_vec, size_t size, bool padded, struct node* largest_var_node);
void make_function_node(
    struct datatype* return_type, 
    const char* name, 
    struct vector* arguments,
    struct node* body_node);
void make_for_node(
    struct node* init_node, 
    struct node* cond_node, 
    struct node* loop_node, 
    struct node* body_node
);
void make_switch_node(
    struct node* exp_node, 
    struct node* body_node, 
    struct vector* cases, 
    bool has_default_case
);
void make_while_node(struct node* exp_node, struct node* body_node);
void make_do_while_node(struct node* exp_node, struct node* body_node);
void make_return_node(struct node* exp_node);

struct node* node_pop();
struct node* node_peek();
struct node* node_peek_or_null();
void node_push(struct node* node);
void node_set_vector(struct vector* vec, struct vector* root_vec);

bool node_is_expressionable(struct node* node);
bool node_is_struct_or_union_variable(struct node* node);
struct node* node_peek_expressionable_or_null();
bool node_is_expression_or_parentheses(struct node* node);
bool node_is_value_type(struct node* node);
bool node_is_expression(struct node* node, const char* op);
bool is_array_node(struct node* node);
bool is_node_assignment(struct node* node);


struct array_brackets* array_brackets_new();
void array_brackets_free(struct array_brackets* brackets);
void array_brackets_add(struct array_brackets* brackets, struct node* bracket_node);
struct vector* array_brackets_node_vector(struct array_brackets* brackets);
size_t array_brackets_calculate_size_from_index(
    struct datatype* dtype, 
    struct array_brackets* brackets,
    int index);
size_t array_brackets_calculate_size(struct datatype* dtype, struct array_brackets* brackets);
int array_total_indexes(struct datatype *dtype);

#define TOTAL_OPERATOR_GROUPS 14
#define MAX_OPERATORS_IN_GROUP 12

enum
{
    ASSOCIATIVITY_LEFT_TO_RIGHT,
    ASSOCIATIVITY_RIGHT_TO_LEFT
};

struct expressionable_op_precedence_group
{
    char* operators[MAX_OPERATORS_IN_GROUP];
    int associtivity;
};


struct fixup;

// returns true if the fixup was successful
typedef bool (*FIXUP_FIX)(struct fixup* fixup);

// removes the fixup from memory, the implementation of this function should
// free any memory related to the fixup
typedef void (*FIXUP_END)(struct fixup* fixup);

struct fixup_config
{
    FIXUP_FIX fix;
    FIXUP_END end;
    void* private;
};

struct fixup_system
{
    struct vector* vector_fixups;
};

enum
{
    FIXUP_FLAG_RESOLVED = 0b00000001
};

struct fixup
{
    int flags;
    struct fixup_system* system;
    struct fixup_config config;
};

struct fixup;
struct fixup_system* fixup_sys_new();
struct fixup_config* fixup_config(struct fixup* fixup);
void fixup_free(struct fixup* fixup);
void fixup_start_iteration(struct fixup_system* system);
struct fixup* fixup_next(struct fixup_system* system);
void fixup_sys_fixups_free(struct fixup_system* system);
void fixup_sys_free(struct fixup_system* system);
int fixup_sys_unresolved_fixups_count(struct fixup_system* system);
struct fixup* fixup_register(struct fixup_system* system, struct fixup_config* config);
bool fixup_resolve(struct fixup* fixup);
void* fixup_private(struct fixup* fixup);
bool fixups_resolve(struct fixup_system* system);



#endif