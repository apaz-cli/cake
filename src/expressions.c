#include "ownership.h"
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include "expressions.h"
#include "hash.h"
#include <string.h>
#include "console.h"
#include "parser.h"
#include "type.h"

#ifdef _WIN32
#include <Windows.h>
#endif

#if defined _MSC_VER && !defined __POCC__
#include <crtdbg.h>
#include <debugapi.h>
#endif


struct constant_value make_constant_value_double(double d)
{
    struct constant_value r;
    r.dvalue = d;
    r.type = TYPE_DOUBLE;
    return r;
}

struct constant_value make_constant_value_ull(unsigned long long d)
{
    struct constant_value r;
    r.ullvalue = d;
    r.type = TYPE_UNSIGNED_LONG_LONG;
    return r;
}

struct constant_value make_constant_value_ll(long long d)
{
    struct constant_value r;
    r.llvalue = d;
    r.type = TYPE_LONG_LONG;
    return r;
}

double constant_value_to_double(const struct constant_value* a)
{
    switch (a->type)
    {
        case TYPE_LONG_LONG: return (double) a->llvalue;
        case TYPE_DOUBLE: return  a->dvalue;
        case TYPE_UNSIGNED_LONG_LONG: return (double) a->ullvalue;
    }

    return 0;
}

bool constant_value_is_valid(const struct constant_value* a)
{
    return a->type != TYPE_NOT_CONSTANT &&
        a->type != TYPE_EMPTY;
}

void constant_value_to_string(const struct constant_value* a, char buffer[], int sz)
{
    buffer[0] = 0;
    switch (a->type)
    {
        case TYPE_LONG_LONG:
            snprintf(buffer, sz, "%lld", a->llvalue);
            break;
        case TYPE_DOUBLE:
            snprintf(buffer, sz, "%f", a->dvalue);
            break;

        case TYPE_UNSIGNED_LONG_LONG:
            snprintf(buffer, sz, "%llu", a->ullvalue);
            break;
    }
}

unsigned long long constant_value_to_ull(const struct constant_value* a)
{
    switch (a->type)
    {
        case TYPE_LONG_LONG: return (unsigned long long)a->llvalue;
        case TYPE_DOUBLE: return  (unsigned long long)a->dvalue;
        case TYPE_UNSIGNED_LONG_LONG: return (unsigned long long) a->ullvalue;
    }

    return 0;
}
long long constant_value_to_ll(const struct constant_value* a)
{
    switch (a->type)
    {
        case TYPE_LONG_LONG: return (long long) a->llvalue;
        case TYPE_DOUBLE: return  (long long) a->dvalue;
        case TYPE_UNSIGNED_LONG_LONG: return (long long) a->ullvalue;
    }

    return 0;
}
bool constant_value_to_bool(const struct constant_value* a)
{
    switch (a->type)
    {
        case TYPE_LONG_LONG: return a->llvalue != 0;
        case TYPE_DOUBLE: return  a->dvalue != 0;
        case TYPE_UNSIGNED_LONG_LONG: return a->ullvalue != 0;
    }

    return 0;
}

struct constant_value constant_value_cast(const struct constant_value* a, enum constant_value_type type)
{
    struct constant_value r = *a;

    if (a->type == TYPE_EMPTY || a->type == TYPE_NOT_CONSTANT)
    {
        return r;
    }

    switch (type)
    {
        case TYPE_NOT_CONSTANT:
        case TYPE_EMPTY:
            assert(false);
            break;

        case TYPE_LONG_LONG:
            r.type = TYPE_LONG_LONG;
            r.llvalue = constant_value_to_ll(a);
            break;
        case TYPE_DOUBLE:
            r.type = TYPE_DOUBLE;
            r.dvalue = constant_value_to_double(a);
            break;
        case TYPE_UNSIGNED_LONG_LONG:
            r.type = TYPE_UNSIGNED_LONG_LONG;
            r.ullvalue = constant_value_to_ull(a);
            break;
    }
    return r;
}

struct constant_value constant_value_unary_op(const struct constant_value* a, int op)
{
    struct constant_value r = {0};
    if (!constant_value_is_valid(a))
    {
        return r;
    }

    if (a->type == TYPE_DOUBLE)
    {
        r.type = TYPE_DOUBLE;
        switch (op)
        {
            case '!':r.dvalue = !a->dvalue;  break;
                //case '~':r.dvalue = ~ a->dvalue;  break;
            case '+':r.dvalue = +a->dvalue;  break;
            case '-':r.dvalue = -a->dvalue;  break;
            default:
                assert(false);
                break;
        }
        return r;
    }
    else if (a->type == TYPE_UNSIGNED_LONG_LONG)
    {
        r.type = TYPE_UNSIGNED_LONG_LONG;
        switch (op)
        {
            case '!':r.ullvalue = !a->ullvalue;  break;
            case '~':r.ullvalue = ~a->ullvalue;  break;
            case '+':r.ullvalue = a->ullvalue;  break;
                //case '-':r.dvalue = -a->ullvalue;  break;
            case '-':
                r.dvalue = 0;// -a->ullvalue;
                break;
            default:
                assert(false);
                break;
        }
        return r;
    }
    else if (a->type == TYPE_LONG_LONG)
    {
        r.type = TYPE_UNSIGNED_LONG_LONG;
        switch (op)
        {
            case '!':r.llvalue = !((long long) a->llvalue);  break;
            case '~':r.llvalue = ~((long long) a->llvalue);  break;
            case '+':r.llvalue = +((long long) a->llvalue);  break;
            case '-':r.llvalue = -((long long) a->llvalue);  break;
            default:
                assert(false);
                break;
        }
        return r;
    }

    return r;
}


struct constant_value constant_value_op(const struct constant_value* a, const struct constant_value* b, int op)
{
    //TODO https://github.com/thradams/checkedints
    struct constant_value r = {0};
    if (!constant_value_is_valid(a) || !constant_value_is_valid(b))
    {
        return r;
    }

    if (a->type == TYPE_DOUBLE || b->type == TYPE_DOUBLE)
    {
        double va = constant_value_to_double(a);
        double vb = constant_value_to_double(b);
        r.type = TYPE_DOUBLE;

        switch (op)
        {
            //Arithmetic Operators
            case '+':r.dvalue = va + vb;  break;
            case '-':r.dvalue = va - vb;  break;
            case '*':r.dvalue = va * vb;  break;
            case '/':
                if (vb != 0)
                    r.dvalue = va / vb;
                else
                    r.type = TYPE_NOT_CONSTANT;
                break;

                //case '%':r.dvalue = va % vb;  break;

                    //Relational Operators
            case '==':r.dvalue = va == vb;  break;
            case '!=':r.dvalue = va != vb;  break;
            case '>':r.dvalue = va > vb;  break;
            case '<':r.dvalue = va < vb;  break;
            case '<=':r.dvalue = va <= vb;  break;
            case '>=':r.dvalue = va >= vb;  break;

                //Logical Operators
            case '&&':r.dvalue = va && vb;  break;
            case '||':r.dvalue = va || vb;  break;

                //Bitwise Operators
            //case '|':r.dvalue = va | vb;  break;
            //case '&':r.dvalue = va & vb;  break;
            //case '^':r.dvalue = va ^ vb;  break;
            //case '>>':r.dvalue = va >> vb;  break;
            //case '<<':r.dvalue = va << vb;  break;

            default:
                assert(false);
                break;
        }

        return r;
    }

    if (a->type == TYPE_UNSIGNED_LONG_LONG || b->type == TYPE_UNSIGNED_LONG_LONG)
    {
        unsigned long long va = constant_value_to_ull(a);
        unsigned long long vb = constant_value_to_ull(b);
        r.type = TYPE_UNSIGNED_LONG_LONG;

        switch (op)
        {
            //Arithmetic Operators
            case '+':r.ullvalue = va + vb;  break;
            case '-':r.ullvalue = va - vb;  break;
            case '*':r.ullvalue = va * vb;  break;
            case '/':
                if (vb != 0)
                    r.ullvalue = va / vb;
                else
                    r.type = TYPE_NOT_CONSTANT;
                break;

            case '%':
                if (vb != 0)
                    r.ullvalue = va % vb;
                else
                    r.type = TYPE_NOT_CONSTANT;
                break;

                //Relational Operators
            case '==':r.ullvalue = va == vb;  break;
            case '!=':r.ullvalue = va != vb;  break;
            case '>':r.ullvalue = va > vb;  break;
            case '<':r.ullvalue = va < vb;  break;
            case '<=':r.ullvalue = va <= vb;  break;
            case '>=':r.ullvalue = va >= vb;  break;

                //Logical Operators
            case '&&':r.ullvalue = va && vb;  break;
            case '||':r.ullvalue = va || vb;  break;

                //Bitwise Operators
            case '|':r.ullvalue = va | vb;  break;
            case '&':r.ullvalue = va & vb;  break;
            case '^':r.ullvalue = va ^ vb;  break;
            case '>>':r.ullvalue = va >> vb;  break;
            case '<<':r.ullvalue = va << vb;  break;

            default:
                assert(false);
                break;
        }

        return r;
    }

    unsigned long long va = a->llvalue;
    unsigned long long vb = b->llvalue;
    r.type = TYPE_LONG_LONG;
    switch (op)
    {
        //Arithmetic Operators
        case '+':r.llvalue = va + vb;  break;
        case '-':r.llvalue = va - vb;  break;
        case '*':r.llvalue = va * vb;  break;

        case '/':
            if (vb != 0)
                r.llvalue = va / vb;
            else
                r.type = TYPE_NOT_CONSTANT;
            break;

        case '%':
            if (vb != 0)
                r.llvalue = va % vb;
            else
                r.type = TYPE_NOT_CONSTANT;
            break;

            //Relational Operators
        case '==':r.llvalue = va == vb;  break;
        case '!=':r.llvalue = va != vb;  break;
        case '>':r.llvalue = va > vb;  break;
        case '<':r.llvalue = va < vb;  break;
        case '<=':r.llvalue = va <= vb;  break;
        case '>=':r.llvalue = va >= vb;  break;

            //Logical Operators
        case '&&':r.llvalue = va && vb;  break;
        case '||':r.llvalue = va || vb;  break;

            //Bitwise Operators
        case '|':r.llvalue = va | vb;  break;
        case '&':r.llvalue = va & vb;  break;
        case '^':r.llvalue = va ^ vb;  break;
        case '>>':r.llvalue = va >> vb;  break;
        case '<<':r.llvalue = va << vb;  break;

        default:
            assert(false);
            break;
    }


    return r;
}

struct expression* owner postfix_expression(struct parser_ctx* ctx);
struct expression* owner cast_expression(struct parser_ctx* ctx);
struct expression* owner multiplicative_expression(struct parser_ctx* ctx);
struct expression* owner unary_expression(struct parser_ctx* ctx);
struct expression* owner additive_expression(struct parser_ctx* ctx);
struct expression* owner shift_expression(struct parser_ctx* ctx);
struct expression* owner relational_expression(struct parser_ctx* ctx);
struct expression* owner equality_expression(struct parser_ctx* ctx);
struct expression* owner and_expression(struct parser_ctx* ctx);
struct expression* owner exclusive_or_expression(struct parser_ctx* ctx);
struct expression* owner inclusive_or_expression(struct parser_ctx* ctx);
struct expression* owner logical_and_expression(struct parser_ctx* ctx);
struct expression* owner logical_or_expression(struct parser_ctx* ctx);
struct expression* owner conditional_expression(struct parser_ctx* ctx);
struct expression* owner expression(struct parser_ctx* ctx);
struct expression* owner conditional_expression(struct parser_ctx* ctx);



static int compare_function_arguments(struct parser_ctx* ctx,
    struct type* p_type,
    struct argument_expression_list* p_argument_expression_list)
{
    try
    {
        struct param* p_current_parameter_type = NULL;

        const struct param_list* p_param_list = type_get_func_or_func_ptr_params(p_type);

        if (p_param_list)
        {
            p_current_parameter_type = p_param_list->head;
        }

        int param_num = 1;
        struct argument_expression* p_current_argument = p_argument_expression_list->head;

        while (p_current_argument && p_current_parameter_type)
        {
            check_function_argument_and_parameter(ctx, p_current_argument, &p_current_parameter_type->type, param_num);




            p_current_argument = p_current_argument->next;
            p_current_parameter_type = p_current_parameter_type->next;
            param_num++;
        }

        if (p_current_argument != NULL && !p_param_list->is_var_args)
        {
            compiler_set_error_with_token(C_TOO_MANY_ARGUMENTS, ctx,
                p_argument_expression_list->tail->expression->first_token,
                "too many arguments");
            throw;
        }

        if (p_current_parameter_type != NULL && !p_param_list->is_void)
        {
            if (p_argument_expression_list->tail)
            {
                compiler_set_error_with_token(C_TOO_FEW_ARGUMENTS, ctx,
                    p_argument_expression_list->tail->expression->first_token,
                    "too few arguments");
            }
            else
            {
                compiler_set_error_with_token(C_TOO_FEW_ARGUMENTS, ctx, ctx->current, "too few arguments");
            }
            throw;
        }
    }
    catch
    {
        return 1; /*error*/
    }

    return 0;
}


bool is_enumeration_constant(struct parser_ctx* ctx)
{
    if (ctx->current->type != TK_IDENTIFIER)
    {
        return false;
    }

    if (ctx->current->flags & TK_FLAG_IDENTIFIER_IS_ENUMERATOR)
        return true;

    if (ctx->current->flags & TK_FLAG_IDENTIFIER_IS_NOT_ENUMERATOR)
        return false;

    const bool is_enumerator = find_enumerator(ctx, ctx->current->lexeme, NULL) != NULL;

    if (is_enumerator)
    {
        ctx->current->flags |= TK_FLAG_IDENTIFIER_IS_ENUMERATOR;
    }
    else
    {
        ctx->current->flags |= TK_FLAG_IDENTIFIER_IS_NOT_ENUMERATOR;
    }

    return is_enumerator;
}

bool is_first_of_floating_constant(struct parser_ctx* ctx)
{
    /*
     floating-constant:
      decimal-floating-constant
      hexadecimal-floating-constan
    */
    return ctx->current->type == TK_COMPILER_DECIMAL_FLOATING_CONSTANT ||
        ctx->current->type == TK_COMPILER_HEXADECIMAL_FLOATING_CONSTANT;
}

bool is_first_of_integer_constant(struct parser_ctx* ctx)
{
    /*
     integer-constant:
      decimal-constant integer-suffixopt
      octal-constant integer-suffixopt
      hexadecimal-constant integer-suffixopt
      binary-constant integer-suffixop
    */

    return ctx->current->type == TK_COMPILER_DECIMAL_CONSTANT ||
        ctx->current->type == TK_COMPILER_OCTAL_CONSTANT ||
        ctx->current->type == TK_COMPILER_HEXADECIMAL_CONSTANT ||
        ctx->current->type == TK_COMPILER_BINARY_CONSTANT;
}

bool is_predefined_constant(struct parser_ctx* ctx)
{
    return ctx->current->type == TK_KEYWORD_TRUE ||
        ctx->current->type == TK_KEYWORD_FALSE ||
        ctx->current->type == TK_KEYWORD_NULLPTR;
}

bool is_first_of_constant(struct parser_ctx* ctx)
{
    /*
     constant:
      integer-constant
      floating-constant
      enumeration-constant
      character-constant
      predefined-constant
    */
    return is_first_of_integer_constant(ctx) ||
        is_first_of_floating_constant(ctx) ||
        is_enumeration_constant(ctx) ||
        (ctx->current->type == TK_CHAR_CONSTANT) ||
        is_predefined_constant(ctx);
}

bool is_first_of_primary_expression(struct parser_ctx* ctx)
{
    /*
     primary-expression:
      identifier
      constant
      string-literal
      ( expression )
      generic-selection
      typeid (expression )
    */
    if (ctx->current == NULL)
        return false;

    return ctx->current->type == TK_IDENTIFIER ||
        is_first_of_constant(ctx) ||
        ctx->current->type == TK_STRING_LITERAL ||
        ctx->current->type == '(' ||
        ctx->current->type == TK_KEYWORD__GENERIC;
}

struct generic_association* owner generic_association(struct parser_ctx* ctx)
{
    struct generic_association* owner p_generic_association = NULL;
    try
    {
        p_generic_association = move calloc(1, sizeof * p_generic_association);
        if (p_generic_association == NULL)
            throw;
        p_generic_association->first_token = ctx->current;
        /*generic - association:
            type-name : assignment-expression
            default : assignment-expression
            */
        if (ctx->current && ctx->current->type == TK_KEYWORD_DEFAULT)
        {
            parser_match(ctx);
        }
        else if (first_of_type_name(ctx))
        {
            p_generic_association->p_type_name = move type_name(ctx);
            p_generic_association->type = move make_type_using_declarator(ctx, p_generic_association->p_type_name->declarator);
        }
        else
        {
            compiler_set_error_with_token(C_UNEXPECTED, ctx, ctx->current, "unexpected");
        }
        parser_match_tk(ctx, ':');
        p_generic_association->expression = move assignment_expression(ctx);
    }
    catch{
    }

    return p_generic_association;
}

struct generic_assoc_list generic_association_list(struct parser_ctx* ctx)
{
    struct generic_assoc_list list = {0};
    try
    {
        struct generic_association* owner p_generic_association =
            move generic_association(ctx);

        if (p_generic_association == NULL) throw;

        LIST_ADD(&list, p_generic_association);

        while (ctx->current->type == ',')
        {
            parser_match(ctx);

            struct generic_association* owner p_generic_association2 = generic_association(ctx);
            if (p_generic_association2 == NULL) throw;

            LIST_ADD(&list, p_generic_association2);
        }
    }
    catch
    {
    }
    return list;
}

struct generic_selection* owner generic_selection(struct parser_ctx* ctx)
{
    /*C23
      generic-selection:
        _Generic ( assignment-expression , generic-assoc-ctx )
    */

    /*
      Extension
      generic-selection:
        "_Generic" ( generic-argument, generic-assoc-list )

        generic-argument:
          assignment-expression
          type-name
    */

    struct generic_selection* owner p_generic_selection = NULL;
    try
    {
        p_generic_selection = move calloc(1, sizeof * p_generic_selection);
        if (p_generic_selection == NULL)
            throw;

        p_generic_selection->first_token = ctx->current;



        parser_match_tk(ctx, TK_KEYWORD__GENERIC);
        parser_match_tk(ctx, '(');

        if (first_of_type_name(ctx))
        {
            /*extension*/
            p_generic_selection->type_name = move type_name(ctx);
        }
        else
        {
            p_generic_selection->expression = move assignment_expression(ctx);
        }

        parser_match_tk(ctx, ',');

        p_generic_selection->generic_assoc_list = move generic_association_list(ctx);


        struct type lvalue_type = {0};

        struct type* p_type = NULL;

        if (p_generic_selection->expression)
        {
            p_type = &p_generic_selection->expression->type;


            if (expression_is_subjected_to_lvalue_conversion(p_generic_selection->expression))
            {
                lvalue_type = move type_lvalue_conversion(&p_generic_selection->expression->type);
                p_type = &lvalue_type;
            }

        }
        else
        {
            p_type = &p_generic_selection->type_name->declarator->type;
        }


        struct generic_association* current = p_generic_selection->generic_assoc_list.head;
        while (current)
        {
            if (current->p_type_name)
            {
                if (type_is_same(p_type, &current->type, true))
                {
                    p_generic_selection->p_view_selected_expression = current->expression;
                    break;
                }
            }
            else
            {
                /*default*/
                p_generic_selection->p_view_selected_expression = current->expression;
            }
            current = current->next;
        }

        p_generic_selection->last_token = ctx->current;
        parser_match_tk(ctx, ')');
        type_destroy(&lvalue_type);
    }
    catch
    {
    }
    return p_generic_selection;
}


int char_constant_to_int(const char* text, bool* overflow)
{
    int count = 0;
    const char* p = text + 1;
    int value = 0;
    while (*p != '\'')
    {
        value = value * 256 + *p;
        p++;
        count++;
        if (count > 4)
        {
            *overflow = true;
        }
    }
    //warning: character constant too long for its type
    return value;
}


//enum token_type parse_number(const char* lexeme, enum type_specifier_flags* flags);

int convert_to_number(struct token* token, struct expression* p_expression_node)
{
    /*copia removendo os separadores*/
    //um dos maiores buffer necessarios seria 128 bits binario...
    //0xb1'1'1.... 
    int c = 0;
    char buffer[128 * 2 + 4] = {0};
    const char* s = token->lexeme;
    while (*s)
    {
        if (*s != '\'')
        {
            buffer[c] = *s;
            c++;
        }
        s++;
    }
    enum type_specifier_flags  flags = 0;
    parse_number(buffer, &flags);
    p_expression_node->type.type_specifier_flags = flags;

    switch (token->type)
    {
        case TK_COMPILER_DECIMAL_CONSTANT:

            if (flags & TYPE_SPECIFIER_UNSIGNED)
            {
                p_expression_node->constant_value = make_constant_value_ull(strtoull(buffer, 0, 10));
            }
            else
            {
                p_expression_node->constant_value = make_constant_value_ll(strtoll(buffer, 0, 10));
            }

            break;
        case TK_COMPILER_OCTAL_CONSTANT:
            p_expression_node->constant_value = make_constant_value_ll(strtoll(buffer, 0, 8));

            break;
        case TK_COMPILER_HEXADECIMAL_CONSTANT:
            p_expression_node->constant_value = make_constant_value_ll(strtoll(buffer + 2, 0, 16));

            break;
        case TK_COMPILER_BINARY_CONSTANT:
            p_expression_node->constant_value = make_constant_value_ll(strtoll(buffer + 2, 0, 2));
            break;
        case TK_COMPILER_DECIMAL_FLOATING_CONSTANT:
            p_expression_node->constant_value = make_constant_value_double(strtod(buffer, 0));
            break;
        case TK_COMPILER_HEXADECIMAL_FLOATING_CONSTANT:
            p_expression_node->constant_value = make_constant_value_double(strtod(buffer + 2, 0));
            break;
        default:
            assert(false);
    }

    return 0;
}

static bool is_integer_or_floating_constant(enum token_type type)
{
    return type == TK_COMPILER_DECIMAL_CONSTANT ||
        type == TK_COMPILER_OCTAL_CONSTANT ||
        type == TK_COMPILER_HEXADECIMAL_CONSTANT ||
        type == TK_COMPILER_BINARY_CONSTANT ||
        type == TK_COMPILER_DECIMAL_FLOATING_CONSTANT ||
        type == TK_COMPILER_HEXADECIMAL_FLOATING_CONSTANT;
}

struct object* expression_get_object(struct expression* p_expression, struct type* p_type)
{    
    if (p_expression->expression_type == PRIMARY_EXPRESSION_DECLARATOR)
    {
        *p_type = type_dup(&p_expression->declarator->type);
        return &p_expression->declarator->object;
    }
    else if (p_expression->expression_type == UNARY_EXPRESSION_ADDRESSOF)
    {
        if (p_expression->right->expression_type == PRIMARY_EXPRESSION_DECLARATOR)
        {
            *p_type = type_dup(&p_expression->right->declarator->type);
            return &p_expression->right->declarator->object;
        }
    }
    else if (p_expression->expression_type == CAST_EXPRESSION)
    {
        if (p_expression->left && p_expression->left->expression_type == PRIMARY_EXPRESSION_DECLARATOR)
        {
            *p_type = type_dup(&p_expression->left->declarator->type);
            return &p_expression->left->declarator->object;
        }
    }
    else if (p_expression->expression_type == POSTFIX_DOT)
    {
        //TODO return ROOT object!

        if (p_expression->left->declarator)
        {
            *p_type = type_dup(&p_expression->type);
            return &p_expression->left->declarator->object.members.data[p_expression->member_index];
        }
    }
    else if (p_expression->expression_type == POSTFIX_ARROW)
    {
        //TODO return ROOT object!
        if (p_expression->left->declarator)
        {
            if (p_expression->left->declarator->object.pointed && 
                p_expression->left->declarator->object.pointed->members.size > 0)
            {
                *p_type = type_dup(&p_expression->type);
                return &p_expression->left->declarator->object.pointed->members.data[p_expression->member_index];
            }
        }
    }
    return NULL;
}


struct expression* owner primary_expression(struct parser_ctx* ctx)
{
    /*
     primary-expression:
      identifier
      constant
      string-literal
      ( expression )
      generic-selection
    */
    struct expression* owner p_expression_node = NULL;
    try
    {
        if (ctx->current->type == TK_IDENTIFIER)
        {
            p_expression_node = move calloc(1, sizeof * p_expression_node);

            if (p_expression_node == NULL) throw;

            p_expression_node->first_token = ctx->current;
            p_expression_node->last_token = ctx->current;

            struct map_entry* p_entry = find_variables(ctx, ctx->current->lexeme, NULL);

            if (p_entry && p_entry->type == TAG_TYPE_ENUMERATOR)
            {
                struct enumerator* p_enumerator = p_entry->p;
                p_expression_node->expression_type = PRIMARY_EXPRESSION_ENUMERATOR;
                p_expression_node->constant_value = make_constant_value_ll(p_enumerator->value);

                p_expression_node->type = move type_make_enumerator(p_enumerator->enum_specifier);

            }
            else if (p_entry &&
                (p_entry->type == TAG_TYPE_ONLY_DECLARATOR || p_entry->type == TAG_TYPE_INIT_DECLARATOR))
            {
                struct declarator* p_declarator = NULL;
                struct init_declarator* p_init_declarator = NULL;
                if (p_entry->type == TAG_TYPE_INIT_DECLARATOR)
                {
                    p_init_declarator = p_entry->p;
                    p_declarator = p_init_declarator->p_declarator;
                }
                else
                {
                    p_declarator = p_entry->p;
                }

                if (type_is_deprecated(&p_declarator->type))
                {
                    compiler_set_warning_with_token(W_DEPRECATED, ctx, ctx->current, "'%s' is deprecated", ctx->current->lexeme);
                }

                p_declarator->num_uses++;
                p_expression_node->declarator = p_declarator;
                p_expression_node->expression_type = PRIMARY_EXPRESSION_DECLARATOR;

                p_expression_node->type = move type_dup(&p_declarator->type);
                if (p_init_declarator)
                {
                    if (p_init_declarator->p_declarator &&
                        p_init_declarator->p_declarator->declaration_specifiers &&
                        p_init_declarator->p_declarator->declaration_specifiers->storage_class_specifier_flags & STORAGE_SPECIFIER_CONSTEXPR)
                    {
                        if (p_init_declarator->initializer &&
                            p_init_declarator->initializer->assignment_expression)
                        {
                            p_expression_node->constant_value =
                                p_init_declarator->initializer->assignment_expression->constant_value;
                        }
                    }
                }
            }
            else if (ctx->p_current_function_opt &&
                strcmp(ctx->current->lexeme, "__func__") == 0)
            {
                /*
                   not sure if this is the best way to implement but
                   works for now
                */
                const char* funcname =
                    ctx->p_current_function_opt->init_declarator_list.head->p_declarator->name->lexeme;

                p_expression_node = move calloc(1, sizeof * p_expression_node);
                if (p_expression_node == NULL) throw;

                p_expression_node->expression_type = PRIMARY_EXPRESSION__FUNC__;
                p_expression_node->first_token = ctx->current;
                p_expression_node->last_token = ctx->current;

                p_expression_node->type = move type_make_literal_string(strlen(funcname) + 1, TYPE_SPECIFIER_CHAR);
            }
            else
            {
                compiler_set_error_with_token(C_NOT_FOUND, ctx, ctx->current, "not found '%s'", ctx->current->lexeme);
                throw;
            }
            parser_match(ctx);
        }
        else if (ctx->current->type == TK_STRING_LITERAL)
        {
            p_expression_node = move calloc(1, sizeof * p_expression_node);
            if (p_expression_node == NULL) throw;

            p_expression_node->expression_type = PRIMARY_EXPRESSION_STRING_LITERAL;
            p_expression_node->first_token = ctx->current;
            p_expression_node->last_token = ctx->current;

            enum type_specifier_flags char_type = TYPE_SPECIFIER_CHAR;

            if (get_char_type(ctx->current->lexeme) == 2)
            {
                if (sizeof(wchar_t) == 2)
                    char_type = TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_SHORT;
                else if (sizeof(wchar_t) == 4)
                    char_type = TYPE_SPECIFIER_UNSIGNED | TYPE_SPECIFIER_INT;
            }

            p_expression_node->type = move type_make_literal_string(string_literal_byte_size(ctx->current->lexeme), char_type);

            parser_match(ctx);

            /*
            string concatenation deveria ser em uma phase anterior
            mas como mantemos as forma do fonte aqui foi uma alternativa
            */
            while (ctx->current &&
                ctx->current->type == TK_STRING_LITERAL)
            {
                parser_match(ctx);
            }
        }
        else if (ctx->current->type == TK_CHAR_CONSTANT)
        {
            p_expression_node = move calloc(1, sizeof * p_expression_node);
            if (p_expression_node == NULL) throw;

            bool over = false;
            const int v = char_constant_to_int(ctx->current->lexeme, &over);
            if (over)
            {
                compiler_set_warning_with_token(W_NONE, ctx, ctx->current, "character constant too long for its type");
            }
            p_expression_node->constant_value = make_constant_value_ll(v);

            p_expression_node->expression_type = PRIMARY_EXPRESSION_CHAR_LITERAL;
            p_expression_node->first_token = ctx->current;
            p_expression_node->last_token = ctx->current;

            p_expression_node->type = move type_make_int();
            p_expression_node->type.attributes_flags |= CAKE_HIDDEN_ATTRIBUTE_LIKE_CHAR;

            parser_match(ctx);
        }

        else if (ctx->current->type == TK_KEYWORD_TRUE ||
            ctx->current->type == TK_KEYWORD_FALSE)
        {
            p_expression_node = move calloc(1, sizeof * p_expression_node);
            if (p_expression_node == NULL) throw;

            p_expression_node->expression_type = PRIMARY_EXPRESSION_PREDEFINED_CONSTANT;
            p_expression_node->first_token = ctx->current;
            p_expression_node->last_token = ctx->current;

            p_expression_node->constant_value =
                make_constant_value_ll(ctx->current->type == TK_KEYWORD_TRUE ? 1 : 0);

            p_expression_node->type.type_specifier_flags = TYPE_SPECIFIER_BOOL;
            p_expression_node->type.type_qualifier_flags = 0;



            parser_match(ctx);
        }
        else if (ctx->current->type == TK_KEYWORD_NULLPTR)
        {
            p_expression_node = move calloc(1, sizeof * p_expression_node);
            if (p_expression_node == NULL) throw;

            p_expression_node->expression_type = PRIMARY_EXPRESSION_PREDEFINED_CONSTANT;
            p_expression_node->first_token = ctx->current;
            p_expression_node->last_token = ctx->current;

            p_expression_node->constant_value = make_constant_value_ll(0);

            /*TODO nullptr type*/
            p_expression_node->type.type_specifier_flags = TYPE_SPECIFIER_NULLPTR_T;
            p_expression_node->type.type_qualifier_flags = 0;



            parser_match(ctx);
        }
        else if (is_integer_or_floating_constant(ctx->current->type))
        {
            p_expression_node = move calloc(1, sizeof * p_expression_node);
            if (p_expression_node == NULL) throw;

            p_expression_node->first_token = ctx->current;
            p_expression_node->last_token = ctx->current;
            p_expression_node->expression_type = PRIMARY_EXPRESSION_NUMBER;
            convert_to_number(ctx->current, p_expression_node);
            parser_match(ctx);
        }
        else if (ctx->current->type == TK_KEYWORD__GENERIC)
        {
            p_expression_node = move calloc(1, sizeof * p_expression_node);
            if (p_expression_node == NULL) throw;
            p_expression_node->first_token = ctx->current;

            p_expression_node->expression_type = PRIMARY_EXPRESSION_GENERIC;

            p_expression_node->generic_selection = move generic_selection(ctx);

            p_expression_node->last_token = p_expression_node->generic_selection->last_token;

            if (p_expression_node->generic_selection->p_view_selected_expression)
            {
                p_expression_node->type = move type_dup(&p_expression_node->generic_selection->p_view_selected_expression->type);

                p_expression_node->constant_value = p_expression_node->generic_selection->p_view_selected_expression->constant_value;
            }
            else
            {
                compiler_set_error_with_token(C_NO_MATCH_FOR_GENERIC, ctx, ctx->current, "no match for generic");
            }
        }
        else if (ctx->current->type == '(')
        {
            parser_match(ctx);
            p_expression_node = move expression(ctx);
            if (p_expression_node == NULL) throw;
            parser_match_tk(ctx, ')');
        }
        else
        {
            compiler_set_error_with_token(C_UNEXPECTED, ctx, ctx->current, "unexpected");
        }
    }
    catch
    {
    }

    /*generic - assoc - ctx:
    generic-association
    generic-assoc-ctx , generic-association
    */
    //assert(p_expression_node&& p_expression_node->first);
    //assert(p_expression_node&& p_expression_node->last);

    return p_expression_node;
}



struct argument_expression_list argument_expression_list(struct parser_ctx* ctx)
{
    /*
     argument-expression-list:
      assignment-expression
      argument-expression-ctx , assignment-expression
    */

    /*
     argument-expression-list: (extended)
      assignment-expression
      move assignment-expression
      argument-expression-ctx , move assignment-expression
      argument-expression-ctx , assignment-expression
    */

    struct argument_expression_list list = {0};
    struct argument_expression* owner p_argument_expression = NULL;

    try
    {
        p_argument_expression = move calloc(1, sizeof(struct argument_expression));
        if (p_argument_expression == NULL) throw;


        if (ctx->current->type == TK_KEYWORD__MOVE) /*extension*/
        {
            p_argument_expression->move_token = ctx->current;
            parser_match(ctx);
        }

        p_argument_expression->expression = move assignment_expression(ctx);
        LIST_ADD(&list, p_argument_expression);

        while (ctx->current->type == ',')
        {
            parser_match(ctx);


            struct argument_expression* owner p_argument_expression_2 = calloc(1, sizeof * p_argument_expression_2);
            if (p_argument_expression_2 == NULL) throw;

            if (ctx->current->type == TK_KEYWORD__MOVE) /*extension*/
            {
                p_argument_expression_2->move_token = ctx->current;
                parser_match(ctx);
            }

            p_argument_expression_2->expression = move assignment_expression(ctx);
            if (p_argument_expression_2->expression == NULL) throw;

            LIST_ADD(&list, p_argument_expression_2);
        }
    }
    catch
    {
    }
    return list;
}



bool first_of_postfix_expression(struct parser_ctx* ctx)
{
    //( type-name )  postfix confunde com (expression) primary
    if (first_of_type_name_ahead(ctx))
        return true; //acho q  nao precisa pq primary tb serve p postif
    return is_first_of_primary_expression(ctx);
}

static void fix_member_type(struct type* p_type, const struct type* struct_type, const struct type* member_type)
{
    if (struct_type->type_qualifier_flags & TYPE_QUALIFIER_CONST)
    {
        /*
          struct X { int i; };
          const struct X x;
          x.i ;//x.i is const
        */
        p_type->type_qualifier_flags |= TYPE_QUALIFIER_CONST;
    }

    /*
          struct X { int i; };
          const struct X x;
          x.i ;//x.i is also local, or parameter etc.
    */
    p_type->storage_class_specifier_flags = struct_type->storage_class_specifier_flags;


    if (struct_type->type_qualifier_flags & TYPE_QUALIFIER_VIEW)
    {
        /*
          struct X { _Owner int i; };
          view struct X x;
          x.i ;//is is not _Owner
        */
        p_type->type_qualifier_flags &= ~TYPE_QUALIFIER_OWNER;
    }

}


static void fix_arrow_member_type(struct type* p_type, const struct type* left, const struct type* member_type)
{
    struct type t = type_remove_pointer(left);

    if (t.type_qualifier_flags & TYPE_QUALIFIER_CONST)
    {
        /*
           const struct X * p;
        */

        p_type->type_qualifier_flags |= TYPE_QUALIFIER_CONST;
    }


    if (t.type_qualifier_flags & TYPE_QUALIFIER_VIEW)
    {
        /*
          view struct X * p;
        */
        p_type->type_qualifier_flags &= ~TYPE_QUALIFIER_OWNER;
    }

    type_destroy(&t);
}

struct expression* owner postfix_expression_tail(struct parser_ctx* ctx, struct expression* owner p_expression_node)
{
    try
    {
        while (ctx->current != NULL)
        {
            if (ctx->current->type == '[')
            {
                struct expression* owner p_expression_node_new = calloc(1, sizeof * p_expression_node_new);
                p_expression_node_new->first_token = ctx->current;
                p_expression_node_new->expression_type = POSTFIX_ARRAY;
                p_expression_node_new->left = move p_expression_node;

                if (!type_is_pointer_or_array(&p_expression_node->type))
                {
                    compiler_set_error_with_token(C_SUBSCRIPTED_VALUE_IS_NEITHER_ARRAY_NOR_POINTER,
                        ctx,
                        ctx->current,
                        "subscripted value is neither array nor pointer");
                }

                if (type_is_pointer(&p_expression_node->type))
                {
                    p_expression_node_new->type = move type_remove_pointer(&p_expression_node->type);

                }
                else if (type_is_array(&p_expression_node->type))
                {
                    p_expression_node_new->type = move get_array_item_type(&p_expression_node->type);
                }

                parser_match(ctx);
                /*contem a expresao de dentro do  [ ] */
                p_expression_node_new->right = move expression(ctx);
                if (p_expression_node_new->right == NULL) throw;

                parser_match_tk(ctx, ']');
                p_expression_node = move p_expression_node_new;
            }
            else if (ctx->current->type == '(')
            {
                struct expression* owner p_expression_node_new = calloc(1, sizeof * p_expression_node_new);
                p_expression_node_new->first_token = ctx->current;
                p_expression_node_new->expression_type = POSTFIX_FUNCTION_CALL;
                p_expression_node_new->left = move p_expression_node;


                if (!type_is_function_or_function_pointer(&p_expression_node->type))
                {
                    compiler_set_error_with_token(C_CALLED_OBJECT_IS_NOT_FUNCTION_OR_FUNCTION_POINTER,
                        ctx,
                        ctx->current,
                        "called object is not attr function or function pointer");
                    throw;
                }

                p_expression_node_new->type = move get_function_return_type(&p_expression_node->type);

                parser_match(ctx);
                if (ctx->current->type != ')')
                {
                    p_expression_node_new->argument_expression_list = move argument_expression_list(ctx);
                }
                parser_match_tk(ctx, ')');
                compare_function_arguments(ctx, &p_expression_node->type, &p_expression_node_new->argument_expression_list);
                p_expression_node_new->last_token = ctx->previous;
                p_expression_node = move p_expression_node_new;
            }
            else if (ctx->current->type == '.')
            {
                struct expression* owner p_expression_node_new = calloc(1, sizeof * p_expression_node_new);
                p_expression_node_new->first_token = ctx->current;
                p_expression_node_new->expression_type = POSTFIX_DOT;
                p_expression_node_new->left = move p_expression_node;

                if (p_expression_node_new->left->expression_type == POSTFIX_DOT)
                {
                    p_expression_node_new->declarator = p_expression_node_new->left->declarator;
                }
                else
                {
                    p_expression_node_new->declarator = p_expression_node_new->left->declarator;
                    //assert(false);
                    //int mindex = 0;
                    //p_expression_node_new->declarator = expression_get_declarator(p_expression_node, &mindex);
                }

                //struct declarator* declarator;

                parser_match(ctx);
                if (p_expression_node->type.type_specifier_flags & TYPE_SPECIFIER_STRUCT_OR_UNION)
                {
                    struct struct_or_union_specifier* p =
                        find_struct_or_union_specifier(ctx, p_expression_node->type.struct_or_union_specifier->tag_name);
                    p = get_complete_struct_or_union_specifier(p);
                    if (p)
                    {
                        int member_index = 0;
                        struct member_declarator* p_member_declarator =
                            find_member_declarator(&p->member_declaration_list, ctx->current->lexeme, &member_index);
                        if (p_member_declarator)
                        {
                            if (p_expression_node_new->left->expression_type == POSTFIX_DOT)
                            {
                                p_expression_node_new->member_index = member_index +
                                    p_expression_node_new->left->member_index;
                            }
                            else
                            {
                                p_expression_node_new->member_index = member_index;
                            }

                            p_expression_node_new->type = move make_type_using_declarator(ctx, p_member_declarator->declarator);

                            fix_member_type(&p_expression_node_new->type,
                                &p_expression_node_new->left->type,
                                &p_member_declarator->declarator->type);
                        }
                        else
                        {
                            compiler_set_error_with_token(C_STRUCT_MEMBER_NOT_FOUND,
                                ctx,
                                ctx->current,
                                "struct member '%s' not found in '%s'",
                                ctx->current->lexeme,
                                p_expression_node->type.struct_or_union_specifier->tag_name);
                        }
                    }
                    else
                    {
                        print_scope(&ctx->scopes);
                    }
                    parser_match_tk(ctx, TK_IDENTIFIER);
                }
                else
                {
                    compiler_set_error_with_token(C_STRUCTURE_OR_UNION_REQUIRED,
                        ctx,
                        ctx->current,
                        "structure or union required");
                }
                //todo apontar pro nome?
                p_expression_node = move p_expression_node_new;
            }
            else if (ctx->current->type == '->')
            {
                struct expression* owner p_expression_node_new = calloc(1, sizeof * p_expression_node_new);
                p_expression_node_new->first_token = ctx->current;
                p_expression_node_new->expression_type = POSTFIX_ARROW;
                p_expression_node_new->left = move p_expression_node;


                parser_match(ctx);

                if (type_is_pointer_or_array(&p_expression_node->type))
                {
                    struct type item_type = {0};
                    if (type_is_array(&p_expression_node->type))
                    {
                        compiler_set_info_with_token(W_STYLE, ctx, ctx->current, "using '->' in array as pointer to struct");
                        item_type = move get_array_item_type(&p_expression_node->type);
                    }
                    else
                    {
                        item_type = move type_remove_pointer(&p_expression_node->type);
                    }

                    if (type_is_struct_or_union(&item_type))
                    {
                        struct struct_or_union_specifier* p_complete =
                            get_complete_struct_or_union_specifier(p_expression_node->type.next->struct_or_union_specifier);

                        if (p_complete)
                        {
                            int member_index = 0;
                            struct member_declarator* p_member_declarator =
                                find_member_declarator(&p_complete->member_declaration_list, ctx->current->lexeme, &member_index);

                            if (p_member_declarator)
                            {
                                p_expression_node_new->type = move make_type_using_declarator(ctx, p_member_declarator->declarator);
                                fix_arrow_member_type(&p_expression_node_new->type, &p_expression_node_new->left->type, &p_expression_node_new->type);
                            }
                            else
                            {
                                compiler_set_error_with_token(C_STRUCT_MEMBER_NOT_FOUND,
                                    ctx,
                                    ctx->current,
                                    "member '%s' not found in struct '%s'",
                                    ctx->current->lexeme,
                                    p_expression_node->type.next->struct_or_union_specifier->tag_name);

                            }
                        }
                        else
                        {
                            compiler_set_error_with_token(C_STRUCT_IS_INCOMPLETE,
                                ctx,
                                ctx->current,
                                "struct '%s' is incomplete.",
                                ctx->current->lexeme);
                        }
                        parser_match_tk(ctx, TK_IDENTIFIER);
                        type_destroy(&item_type);
                    }
                    else
                    {
                        compiler_set_error_with_token(C_STRUCTURE_OR_UNION_REQUIRED,
                            ctx,
                            ctx->current,
                            "structure or union required");
                    }
                }
                else
                {
                    compiler_set_error_with_token(C_STRUCTURE_OR_UNION_REQUIRED,
                        ctx,
                        ctx->current,
                        "structure or union required");
                }

                p_expression_node = move p_expression_node_new;
            }
            else if (ctx->current->type == '++')
            {
                struct expression* owner p_expression_node_new = calloc(1, sizeof * p_expression_node_new);
                p_expression_node_new->first_token = ctx->current;
                p_expression_node_new->expression_type = POSTFIX_INCREMENT;
                p_expression_node_new->left = move p_expression_node;
                p_expression_node_new->type = move type_dup(&p_expression_node->type);
                parser_match(ctx);
                p_expression_node = move p_expression_node_new;
            }
            else if (ctx->current->type == '--')
            {
                struct expression* owner p_expression_node_new = calloc(1, sizeof * p_expression_node_new);
                p_expression_node_new->first_token = ctx->current;
                p_expression_node_new->expression_type = POSTFIX_DECREMENT;
                p_expression_node_new->left = move p_expression_node;
                p_expression_node_new->type = move type_dup(&p_expression_node->type);
                parser_match(ctx);
                p_expression_node = move p_expression_node_new;
            }
            else
            {
                break;
            }
        }
    }
    catch
    {
    }

    return move p_expression_node;
}

struct expression* owner postfix_expression_type_name(struct parser_ctx* ctx, struct type_name* owner p_type_name)
{
    /*
        ( type-name ) { initializer-ctx }
        ( type-name ) { initializer-ctx , }

        //My extension : if type-name is function then follow is compound-statement
        ( type-name ) compound-statement

    */

    struct expression* owner p_expression_node = NULL;

    try
    {
        p_expression_node = move calloc(1, sizeof * p_expression_node);
        if (p_expression_node == NULL) throw;

        assert(p_expression_node->type_name == NULL);

        p_expression_node->first_token = previous_parser_token(p_type_name->first_token);
        assert(p_expression_node->first_token->type == '(');

        p_expression_node->type_name = move p_type_name;
        p_expression_node->type = move make_type_using_declarator(ctx, p_expression_node->type_name->declarator);


        if (type_is_function(&p_type_name->declarator->type))
        {
            p_expression_node->expression_type = POSTFIX_EXPRESSION_FUNCTION_LITERAL;

            struct scope* parameters_scope =
                &p_expression_node->type_name->declarator->direct_declarator->function_declarator->parameters_scope;

            scope_list_push(&ctx->scopes, parameters_scope);
            p_expression_node->compound_statement = move function_body(ctx);
            scope_list_pop(&ctx->scopes);

        }
        else
        {
            p_expression_node->expression_type = POSTFIX_EXPRESSION_COMPOUND_LITERAL;
            p_expression_node->braced_initializer = move braced_initializer(ctx);
        }

        p_expression_node->last_token = ctx->previous;

        p_expression_node = move postfix_expression_tail(ctx, move p_expression_node);
        if (p_expression_node == NULL) throw;

    }
    catch
    {
    }

    return p_expression_node;
}

struct expression* owner postfix_expression(struct parser_ctx* ctx)
{
    /*
      postfix-expression:
        primary-expression
        postfix-expression [ expression ]
        postfix-expression ( argument-expression-list_opt)
        postfix-expression . identifier
        postfix-expression -> identifier
        postfix-expression ++
        postfix-expression --
        ( type-name ) { initializer-ctx }
        ( type-name ) { initializer-ctx , }

        //My extension : if type-name is function then follow is compound-statement
        ( type-name ) compound-statement

        */
    struct expression* owner p_expression_node = NULL;
    try
    {
        if (first_of_type_name_ahead(ctx)) //aqui preciso ver se nao eh primary
        {
            assert(false); //este caso esta pegando lá dentro deo cast expression.
            p_expression_node = move calloc(1, sizeof * p_expression_node);
            if (p_expression_node == NULL) throw;

            p_expression_node->first_token = ctx->current;
            parser_match_tk(ctx, '(');
            p_expression_node->type_name = move type_name(ctx);
            if (p_expression_node->type_name == NULL) throw;

            p_expression_node->type = move make_type_using_declarator(ctx, p_expression_node->type_name->declarator);
            parser_match_tk(ctx, ')');
            //printf("\n");
            //print_type(&p_expression_node->type);
            bool is_function_type = type_is_function(&p_expression_node->type);

            if (is_function_type)
            {
                p_expression_node->expression_type = POSTFIX_EXPRESSION_FUNCTION_LITERAL;
                p_expression_node->compound_statement = move compound_statement(ctx);
                if (p_expression_node->compound_statement == NULL) throw;

                p_expression_node->last_token = p_expression_node->compound_statement->last_token;
            }
            else
            {
                p_expression_node->expression_type = POSTFIX_EXPRESSION_COMPOUND_LITERAL;
                p_expression_node->braced_initializer = move braced_initializer(ctx);
                p_expression_node->last_token = ctx->current;
            }
        }
        else
        {
            p_expression_node = move primary_expression(ctx);
            if (p_expression_node == NULL) throw;
        }

        p_expression_node = move postfix_expression_tail(ctx, move p_expression_node);
        if (p_expression_node == NULL) throw;
    }
    catch
    {
    }
    return p_expression_node;
}

bool is_first_of_compiler_function(struct parser_ctx* ctx)
{
    if (ctx->current == NULL)
        return false;

    return
        //traits
        ctx->current->type == TK_KEYWORD_IS_LVALUE ||
        ctx->current->type == TK_KEYWORD_IS_OWNER ||
        ctx->current->type == TK_KEYWORD_IS_CONST ||
        ctx->current->type == TK_KEYWORD_IS_POINTER ||
        ctx->current->type == TK_KEYWORD_IS_ARRAY ||
        ctx->current->type == TK_KEYWORD_IS_FUNCTION ||

        ctx->current->type == TK_KEYWORD_IS_SCALAR ||
        ctx->current->type == TK_KEYWORD_IS_ARITHMETIC ||
        ctx->current->type == TK_KEYWORD_IS_FLOATING_POINT ||
        ctx->current->type == TK_KEYWORD_IS_INTEGRAL ||
        //
        ctx->current->type == TK_KEYWORD_STATIC_DEBUG ||
        ctx->current->type == TK_KEYWORD_IS_SAME ||
        ctx->current->type == TK_KEYWORD_ATTR_ADD ||
        ctx->current->type == TK_KEYWORD_ATTR_REMOVE ||
        ctx->current->type == TK_KEYWORD_ATTR_HAS;
}

bool is_first_of_unary_expression(struct parser_ctx* ctx)
{
    if (ctx->current == NULL)
        return false;
    return  first_of_postfix_expression(ctx) ||
        ctx->current->type == '++' ||
        ctx->current->type == '--' ||
        ctx->current->type == '&' ||
        ctx->current->type == '*' ||
        ctx->current->type == '+' ||
        ctx->current->type == '-' ||
        ctx->current->type == '~' ||
        ctx->current->type == '!' ||
        ctx->current->type == TK_KEYWORD_SIZEOF ||
        ctx->current->type == TK_KEYWORD__ALIGNOF ||
        is_first_of_compiler_function(ctx);
}


struct expression* owner unary_expression(struct parser_ctx* ctx)
{
    /*
    unary-expression:
      postfix-expression
      ++ unary-expression
      -- unary-expression

      one of (& * + - ~ !) cast-expression

      sizeof unary-expression
      sizeof ( type-name )
      _Alignof ( type-name )



      */
    struct expression* owner p_expression_node = NULL;
    try
    {
        if (ctx->current->type == '++' || ctx->current->type == '--')
        {
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);

            if (ctx->current->type == '++')
                new_expression->expression_type = UNARY_EXPRESSION_INCREMENT;
            else
                new_expression->expression_type = UNARY_EXPRESSION_DECREMENT;
            parser_match(ctx);
            new_expression->right = move unary_expression(ctx);
            if (new_expression->right == NULL) throw;

            new_expression->type = move type_dup(&new_expression->right->type);
            p_expression_node = move new_expression;
        }
        else if (ctx->current != NULL &&
            (ctx->current->type == '&'
                || ctx->current->type == '*'
                || ctx->current->type == '+'
                || ctx->current->type == '-'
                || ctx->current->type == '~'
                || ctx->current->type == '!'))
        {

            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            new_expression->first_token = ctx->current;


            struct token* op_position = ctx->current; //marcar posicao
            enum token_type op = ctx->current->type;
            parser_match(ctx);

            if (style_has_space(ctx->current))
            {
                compiler_set_info_with_token(W_STYLE, ctx, ctx->current, "don't use spaces");
            }

            new_expression->right = move cast_expression(ctx);
            if (new_expression->right == NULL) throw;

            new_expression->last_token = new_expression->right->last_token;
            if (op == '!')
            {
                new_expression->expression_type = UNARY_EXPRESSION_NOT;
                new_expression->constant_value = constant_value_unary_op(&new_expression->right->constant_value, op);

                //same as v == 0

                new_expression->type = move type_make_int_bool_like();
            }
            else if (op == '~')
            {
                new_expression->expression_type = UNARY_EXPRESSION_BITNOT;
                new_expression->constant_value = constant_value_unary_op(&new_expression->right->constant_value, op);

                new_expression->type = move type_dup(&new_expression->right->type);
                new_expression->type.storage_class_specifier_flags &= ~STORAGE_SPECIFIER_LVALUE;
            }
            else if (op == '-')
            {
                new_expression->expression_type = UNARY_EXPRESSION_NEG;

                new_expression->constant_value = constant_value_unary_op(&new_expression->right->constant_value, op);

                new_expression->type = move type_dup(&new_expression->right->type);
                new_expression->type.storage_class_specifier_flags &= ~STORAGE_SPECIFIER_LVALUE;
            }
            else if (op == '+')
            {
                new_expression->expression_type = UNARY_EXPRESSION_PLUS;

                new_expression->constant_value = constant_value_unary_op(&new_expression->right->constant_value, op);

                new_expression->type = move type_dup(&new_expression->right->type);
                new_expression->type.storage_class_specifier_flags &= ~STORAGE_SPECIFIER_LVALUE;
            }
            else if (op == '*')
            {
                new_expression->expression_type = UNARY_EXPRESSION_CONTENT;
                if (!type_is_pointer(&new_expression->right->type))
                {
                    compiler_set_error_with_token(C_INDIRECTION_REQUIRES_POINTER_OPERAND,
                        ctx,
                        op_position,
                        "indirection requires pointer operand");
                }
                new_expression->type = move type_remove_pointer(&new_expression->right->type);
            }
            else if (op == '&')
            {
                /*
                  The result of the unary & operator is a pointer to the object referred to by the lvalue-expression.
                  If the type of the lvalue-expression is " . . . ", the type of the result is
                  "pointer to . . ."
                */
                new_expression->expression_type = UNARY_EXPRESSION_ADDRESSOF;
                //TODO nao tem como tirar endereco de uma constante

                if (!(new_expression->right->type.storage_class_specifier_flags & STORAGE_SPECIFIER_LVALUE))
                {
                    //STORAGE_SPECIFIER_LVALUE not worning yet on type system
#if 0
                    compiler_set_error_with_token(C_ADDRESS_OF_REGISTER,
                        ctx,
                        new_expression->right->first_token,
                        "lvalue required as unary '&' operand");
#endif
                }

                if (new_expression->right->type.storage_class_specifier_flags & STORAGE_SPECIFIER_REGISTER)
                {
                    if (new_expression->right->declarator)
                    {
                        compiler_set_error_with_token(C_ADDRESS_OF_REGISTER,
                            ctx,
                            new_expression->right->first_token,
                            "address of register variable 'x' requested",
                            new_expression->right->declarator->name->lexeme);
                    }
                    else
                    {
                        compiler_set_error_with_token(C_ADDRESS_OF_REGISTER,
                            ctx,
                            new_expression->right->first_token,
                            "address of register variable requested - declarator?");
                    }

                }

                new_expression->type = move type_add_pointer(&new_expression->right->type);
            }
            else
            {
                compiler_set_error_with_token(C_INVALID_TOKEN,
                    ctx,
                    ctx->current,
                    "invalid token");
                throw;
            }
            p_expression_node = move new_expression;
        }
        else if (ctx->current->type == TK_KEYWORD_SIZEOF)
        {
            parser_match(ctx);
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            if (first_of_type_name_ahead(ctx))
            {
                new_expression->expression_type = UNARY_EXPRESSION_SIZEOF_TYPE;
                parser_match_tk(ctx, '(');
                new_expression->type_name = move type_name(ctx);

                new_expression->type = move type_make_int();

                parser_match_tk(ctx, ')');
                new_expression->constant_value = make_constant_value_ll(type_get_sizeof(&new_expression->type_name->declarator->type));

            }
            else
            {
                new_expression->right = move unary_expression(ctx);
                if (new_expression->right == NULL) throw;


                new_expression->expression_type = UNARY_EXPRESSION_SIZEOF_EXPRESSION;
                new_expression->constant_value = make_constant_value_ll(type_get_sizeof(&new_expression->right->type));

            }

            new_expression->type = move type_make_size_t();
            p_expression_node = move new_expression;
        }
        else if (
            ctx->current->type == TK_KEYWORD_IS_LVALUE ||
            ctx->current->type == TK_KEYWORD_IS_OWNER ||
            ctx->current->type == TK_KEYWORD_IS_CONST ||
            ctx->current->type == TK_KEYWORD_IS_POINTER ||
            ctx->current->type == TK_KEYWORD_IS_ARRAY ||
            ctx->current->type == TK_KEYWORD_IS_FUNCTION ||
            ctx->current->type == TK_KEYWORD_IS_ARITHMETIC ||
            ctx->current->type == TK_KEYWORD_IS_SCALAR ||
            ctx->current->type == TK_KEYWORD_IS_FLOATING_POINT ||
            ctx->current->type == TK_KEYWORD_IS_INTEGRAL)
        {
            struct token* traits_token = ctx->current;

            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            new_expression->first_token = ctx->current;
            new_expression->expression_type = UNARY_EXPRESSION_TRAITS;

            parser_match(ctx);

            struct type* p_type = NULL;
            if (first_of_type_name_ahead(ctx))
            {
                parser_match_tk(ctx, '(');
                new_expression->type_name = move type_name(ctx);
                new_expression->last_token = ctx->current;
                parser_match_tk(ctx, ')');
                p_type = &new_expression->type_name->declarator->type;
            }
            else
            {


                new_expression->right = move unary_expression(ctx);
                if (new_expression->right == NULL) throw;

                p_type = &new_expression->right->type;
                new_expression->last_token = ctx->previous;
            }
            switch (traits_token->type)
            {
                case TK_KEYWORD_IS_LVALUE:
                    new_expression->constant_value = make_constant_value_ll(type_is_lvalue(p_type));
                    break;

                case TK_KEYWORD_IS_CONST:
                    new_expression->constant_value = make_constant_value_ll(type_is_const(p_type));
                    break;
                case TK_KEYWORD_IS_OWNER:
                    new_expression->constant_value = make_constant_value_ll(type_is_owner(p_type));
                    break;

                case TK_KEYWORD_IS_POINTER:
                    new_expression->constant_value = make_constant_value_ll(type_is_pointer(p_type));

                    break;
                case TK_KEYWORD_IS_FUNCTION:
                    new_expression->constant_value = make_constant_value_ll(type_is_function(p_type));

                    break;
                case TK_KEYWORD_IS_ARRAY:
                    new_expression->constant_value = make_constant_value_ll(type_is_array(p_type));

                    break;
                case TK_KEYWORD_IS_ARITHMETIC:
                    new_expression->constant_value = make_constant_value_ll(type_is_arithmetic(p_type));

                    break;
                case TK_KEYWORD_IS_SCALAR:
                    new_expression->constant_value = make_constant_value_ll(type_is_scalar(p_type));

                    break;
                case TK_KEYWORD_IS_FLOATING_POINT:
                    new_expression->constant_value = make_constant_value_ll(type_is_floating_point(p_type));

                    break;
                case TK_KEYWORD_IS_INTEGRAL:
                    new_expression->constant_value = make_constant_value_ll(type_is_integer(p_type));

                    break;

                default:
                    assert(false);

            }


            new_expression->type = move type_make_int_bool_like();
            p_expression_node = move new_expression;
        }
        else if (ctx->current->type == TK_KEYWORD_IS_SAME)
        {
            struct expression* owner  new_expression = calloc(1, sizeof * new_expression);
            new_expression->first_token = ctx->current;
            parser_match(ctx);
            new_expression->expression_type = UNARY_EXPRESSION_IS_SAME;
            parser_match_tk(ctx, '(');
            new_expression->type_name = move type_name(ctx);
            parser_match_tk(ctx, ',');
            new_expression->type_name2 = move  type_name(ctx);
            parser_match_tk(ctx, ')');

            new_expression->constant_value =
                make_constant_value_ll(
                    type_is_same(&new_expression->type_name->declarator->type,
                        &new_expression->type_name2->declarator->type, true));


            new_expression->type = move type_make_int_bool_like();
            p_expression_node = move new_expression;
        }
        else if (ctx->current->type == TK_KEYWORD_STATIC_DEBUG)
        {
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            new_expression->first_token = ctx->current;
            new_expression->expression_type = UNARY_EXPRESSION_STATIC_DEBUG;

            parser_match(ctx);

            new_expression->right = move unary_expression(ctx);
            if (new_expression->right == NULL) throw;


            //new_expression->declarator = expression_get_declarator(new_expression->right, &new_expression->member_index);

            new_expression->type = move make_void_type();
            p_expression_node = move new_expression;
        }
        else if (ctx->current->type == TK_KEYWORD__ALIGNOF)
        {
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);

            new_expression->expression_type = UNARY_EXPRESSION_ALIGNOF;
            new_expression->first_token = ctx->current;

            parser_match(ctx);
            parser_match_tk(ctx, '(');
            new_expression->type_name = move type_name(ctx);
            new_expression->type = move make_type_using_declarator(ctx, new_expression->type_name->declarator);
            parser_match_tk(ctx, ')');

            new_expression->constant_value = make_constant_value_ll(type_get_alignof(&new_expression->type));

            new_expression->type = move type_make_int();
            p_expression_node = move new_expression;
            new_expression->last_token = ctx->previous;
        }
        else if (ctx->current->type == TK_KEYWORD__ALIGNAS)
        {
        }
        else //if (is_first_of_primary_expression(ctx))
        {
            p_expression_node = move postfix_expression(ctx);
            if (p_expression_node == NULL) throw;
        }
    }
    catch
    {
    }

    return p_expression_node;
}

struct expression* owner cast_expression(struct parser_ctx* ctx)
{
    /*
     cast-expression:
      unary-expression
      ( type-name ) cast-expression


      ( type-name ) //<- extension void value
    */
    struct expression* owner p_expression_node = NULL;
    try
    {
        if (first_of_type_name_ahead(ctx))
        {
            p_expression_node = move calloc(1, sizeof * p_expression_node);
            if (p_expression_node == NULL) throw;

            p_expression_node->first_token = ctx->current;
            p_expression_node->expression_type = CAST_EXPRESSION;
            parser_match_tk(ctx, '(');
            p_expression_node->type_name = move type_name(ctx);
            if (p_expression_node->type_name == NULL)
                throw;


            p_expression_node->type = move type_dup(&p_expression_node->type_name->type);

            //type_set_int(&ctx->result_type);
            //print_type_name(p_cast_expression->type_name);
            parser_match_tk(ctx, ')');
            //struct token_list r = copy_replacement_list(&l);
            //pop_f
            if (ctx->current->type == '{')
            {
                // Achar que era um cast_expression foi um engano...
                // porque apareceu o { então é compound literal que eh postfix.
                struct expression* owner new_expression = postfix_expression_type_name(ctx, move p_expression_node->type_name);


                free(p_expression_node);
                p_expression_node = move new_expression;
            }
            else if (is_first_of_unary_expression(ctx))
            {
                p_expression_node->left = move cast_expression(ctx);
                if (p_expression_node->left == NULL) throw;
                p_expression_node->constant_value = p_expression_node->left->constant_value;
                p_expression_node->type = move make_type_using_declarator(ctx, p_expression_node->type_name->declarator);
                if (type_is_floating_point(&p_expression_node->type))
                {
                    p_expression_node->constant_value =
                        constant_value_cast(&p_expression_node->constant_value, TYPE_DOUBLE);
                }
                else if (type_is_integer(&p_expression_node->type))
                {
                    if (type_is_unsigned_integer(&p_expression_node->type))
                    {
                        p_expression_node->constant_value =
                            constant_value_cast(&p_expression_node->constant_value, TYPE_UNSIGNED_LONG_LONG);
                    }
                    else
                    {
                        p_expression_node->constant_value =
                            constant_value_cast(&p_expression_node->constant_value, TYPE_LONG_LONG);
                    }
                }
            }
            else
            {
                p_expression_node->type = move make_type_using_declarator(ctx, p_expression_node->type_name->declarator);
                p_expression_node->constant_value.type = TYPE_EMPTY;
            }
        }
        else if (is_first_of_unary_expression(ctx))
        {
            p_expression_node = move unary_expression(ctx);
            if (p_expression_node == NULL) throw;
        }
        else
        {
            compiler_set_error_with_token(C_UNEXPECTED, ctx, ctx->current, "unexpected");
        }
    }
    catch
    {
    }
    if (p_expression_node && ctx->current)
    {
        p_expression_node->last_token = ctx->previous;
    }
    return p_expression_node;
}

struct expression* owner multiplicative_expression(struct parser_ctx* ctx)
{


    /*
     multiplicative-expression:
    cast-expression
    multiplicative-expression * cast-expression
    multiplicative-expression / cast-expression
    multiplicative-expression % cast-expression
    */
    struct expression* owner p_expression_node = NULL;
    try
    {
        p_expression_node = move cast_expression(ctx);
        if (p_expression_node == NULL) throw;

        while (ctx->current != NULL &&
            (ctx->current->type == '*' ||
                ctx->current->type == '/' ||
                ctx->current->type == '%'))
        {
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            enum token_type op = ctx->current->type;
            parser_match(ctx);
            new_expression->left = move p_expression_node;
            new_expression->right = move cast_expression(ctx);

            if (new_expression->left == NULL ||
                new_expression->right == NULL)
            {
                throw;
            }

            if (op == '*')
            {
                new_expression->expression_type = MULTIPLICATIVE_EXPRESSION_MULT;


                if (!type_is_arithmetic(&new_expression->left->type))
                {
                    compiler_set_error_with_token(C_LEFT_IS_NOT_ARITHMETIC, ctx, ctx->current, "left * is not arithmetic");

                }
                if (!type_is_arithmetic(&new_expression->right->type))
                {
                    compiler_set_error_with_token(C_RIGHT_IS_NOT_ARITHMETIC, ctx, ctx->current, "right * is not arithmetic");
                }

            }
            else if (op == '/')
            {

                new_expression->expression_type = MULTIPLICATIVE_EXPRESSION_DIV;

                new_expression->constant_value =
                    constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, '/');

                if (constant_value_is_valid(&new_expression->right->constant_value) &&
                    constant_value_to_ll(&new_expression->right->constant_value) == 0)
                {
                    compiler_set_error_with_token(C_DIVIZION_BY_ZERO, ctx, ctx->current, "divizion by zero");
                }

                if (!type_is_arithmetic(&new_expression->left->type))
                {
                    compiler_set_error_with_token(C_LEFT_IS_NOT_ARITHMETIC, ctx, ctx->current, "left / is not arithmetic");

                }
                if (!type_is_arithmetic(&new_expression->right->type))
                {
                    compiler_set_error_with_token(C_RIGHT_IS_NOT_ARITHMETIC, ctx, ctx->current, "right / is not arithmetic");
                }
            }
            else if (op == '%')
            {
                new_expression->expression_type = MULTIPLICATIVE_EXPRESSION_MOD;


                if (!type_is_integer(&new_expression->left->type))
                {
                    compiler_set_error_with_token(C_LEFT_IS_NOT_INTEGER, ctx, ctx->current, "left is not integer");

                }
                if (!type_is_integer(&new_expression->right->type))
                {
                    compiler_set_error_with_token(C_RIGHT_IS_NOT_INTEGER, ctx, ctx->current, "right is not integer");
                }

                if (constant_value_is_valid(&new_expression->right->constant_value) &&
                    constant_value_to_ll(&new_expression->right->constant_value) == 0)
                {
                    compiler_set_error_with_token(C_DIVIZION_BY_ZERO, ctx, ctx->current, "divizion by zero");
                }
            }

            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, op);


            int code = type_common(&new_expression->left->type, &new_expression->right->type, &new_expression->type);
            if (code != 0)
            {
                compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "invalid type multiplicative expression");
                throw;
            }

            p_expression_node = move  new_expression;
        }
    }
    catch
    {
    }
    return p_expression_node;
}

struct expression* owner additive_expression(struct parser_ctx* ctx)
{
    /*
     additive-expression:
    multiplicative-expression
    additive-expression + multiplicative-expression
    additive-expression - multiplicative-expression
    */

    struct expression* owner p_expression_node = NULL;
    struct expression* owner new_expression = NULL;

    try
    {
        p_expression_node = move multiplicative_expression(ctx);
        if (p_expression_node == NULL) throw;


        while (ctx->current != NULL &&
            (ctx->current->type == '+' ||
                ctx->current->type == '-'))
        {
            struct token* operator_position = ctx->current;

            assert(new_expression == NULL);
            new_expression = move calloc(1, sizeof * new_expression);
            enum token_type op = ctx->current->type;
            parser_match(ctx);
            new_expression->left = move p_expression_node;

            static int count = 0;
            count++;
            new_expression->right = move multiplicative_expression(ctx);
            if (new_expression->right == NULL) throw;

            if (!type_is_scalar(&new_expression->left->type))
            {
                compiler_set_error_with_token(C_LEFT_IS_NOT_SCALAR, ctx, operator_position, "left operator is not scalar");
            }
            if (!type_is_scalar(&new_expression->right->type))
            {
                compiler_set_error_with_token(C_RIGHT_IS_NOT_SCALAR, ctx, operator_position, "right operator is not scalar");
            }


            const bool b_left_is_arithmetic = type_is_arithmetic(&new_expression->left->type);
            const bool b_right_is_arithmetic = type_is_arithmetic(&new_expression->right->type);

            const enum type_category left_category = type_get_category(&new_expression->left->type);
            const enum type_category right_category = type_get_category(&new_expression->right->type);

            if (op == '+')
            {
                new_expression->expression_type = ADDITIVE_EXPRESSION_PLUS;


                /*
                 For addition, either both operands shall have arithmetic type,
                 or one operand shall be a pointer to a complete object type and
                 the other shall have integer type. (Incrementing is equivalent to adding 1.)
                */
                if (b_left_is_arithmetic && b_right_is_arithmetic)
                {
                    int code = type_common(&new_expression->left->type, &new_expression->right->type, &new_expression->type);
                    if (code != 0)
                    {
                        compiler_set_error_with_token(C_UNEXPECTED, ctx, ctx->current, "internal error");
                        throw;
                    }
                }
                else
                {
                    if (left_category == TYPE_CATEGORY_POINTER || left_category == TYPE_CATEGORY_ARRAY  /* || left_category == TYPE_CATEGORY_FUNCTION*/)
                    {
                        if (type_is_integer(&new_expression->right->type))
                        {
                            if (left_category == TYPE_CATEGORY_ARRAY)
                            {
                                struct type t = get_array_item_type(&new_expression->left->type);
                                new_expression->type = move type_add_pointer(&t);
                                type_destroy(&t);
                            }
                            else
                            {
                                new_expression->type = move type_dup(&new_expression->left->type);
                            }
                        }
                        else
                        {
                            compiler_set_error_with_token(C_RIGHT_IS_NOT_INTEGER, ctx, ctx->current, "expected integer type on right");
                        }
                    }
                    else if (right_category == TYPE_CATEGORY_POINTER || right_category == TYPE_CATEGORY_ARRAY /*|| right_category == TYPE_CATEGORY_FUNCTION*/)
                    {
                        if (type_is_integer(&new_expression->left->type))
                        {
                            if (right_category == TYPE_CATEGORY_ARRAY)
                            {
                                new_expression->type = move get_array_item_type(&new_expression->right->type);
                            }
                            else
                            {
                                new_expression->type = move type_dup(&new_expression->right->type);
                            }
                        }
                        else
                        {
                            compiler_set_error_with_token(C_LEFT_IS_NOT_INTEGER, ctx, ctx->current, "expected integer type on left");
                        }
                    }
                    else
                    {
                        compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "invalid types additive expression");
                    }
                }
            }
            else if (op == '-')
            {
                new_expression->expression_type = ADDITIVE_EXPRESSION_MINUS;


                /*
                 For subtraction, one of the following shall hold:
                    — both operands have arithmetic type;
                    — both operands are pointers to qualified or unqualified versions of compatible complete object
                    types; or
                    — the left operand is a pointer to a complete object type and the right operand has integer type.
                    (Decrementing is equivalent to subtracting 1.)
                */
                if (b_left_is_arithmetic && b_right_is_arithmetic)
                {
                    //— both operands have arithmetic type;
                    int code = type_common(&new_expression->left->type, &new_expression->right->type, &new_expression->type);
                    if (code != 0)
                    {
                        compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "internal error type_common");
                        throw;
                    }
                }
                else
                {
                    if (left_category == TYPE_CATEGORY_POINTER || left_category == TYPE_CATEGORY_ARRAY)
                    {
                        if (right_category == TYPE_CATEGORY_POINTER || right_category == TYPE_CATEGORY_ARRAY)
                        {
                            //— both operands are pointers to qualified or unqualified versions of compatible complete object
                            // types;
                            struct type t1 = type_lvalue_conversion(&new_expression->left->type);
                            struct type t2 = type_lvalue_conversion(&new_expression->right->type);

                            if (!type_is_same(&t1, &t2, false))
                            {
                                compiler_set_error_with_token(C_INCOMPATIBLE_POINTER_TYPES, ctx, ctx->current, "incompatible pointer types");
                            }

                            new_expression->type = move type_make_int();
                            type_destroy(&t1);
                            type_destroy(&t2);
                        }
                        else
                        {
                            if (type_is_integer(&new_expression->right->type))
                            {
                                //- the left operand is a pointer to a complete object typeand the right operand has integer type.
                                new_expression->type = move type_dup(&new_expression->left->type);
                            }
                            else
                            {
                                compiler_set_error_with_token(C_RIGHT_IS_NOT_INTEGER, ctx, ctx->current, "right must be integer type");
                            }
                        }
                    }
                    else
                    {
                        compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "invalid types for operator -");
                    }
                }
            }

            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, op);


            p_expression_node = move new_expression;
            new_expression = NULL; /*MOVED*/
        }
    }
    catch
    {
        if (p_expression_node)
        {
            //expression_node_delete(p_expression_node);
        }
        if (new_expression)
        {
            //expression_node_delete(p_expression_node);
        }
    }



    return p_expression_node;
}

struct expression* owner shift_expression(struct parser_ctx* ctx)
{
    /*
     shift-expression:
       additive-expression
       shift-expression << additive-expression
       shift-expression >> additive-expression
    */
    struct expression* owner p_expression_node = NULL;
    try
    {
        p_expression_node = move additive_expression(ctx);
        if (p_expression_node == NULL) throw;

        while (ctx->current != NULL &&
            (ctx->current->type == '>>' ||
                ctx->current->type == '<<'))
        {
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            enum token_type op = ctx->current->type;
            parser_match(ctx);
            new_expression->left = move p_expression_node;
            new_expression->right = move multiplicative_expression(ctx);
            if (new_expression->left == NULL || new_expression->right == NULL)
            {
                throw;
            }

            if (op == '>>')
            {
                new_expression->expression_type = SHIFT_EXPRESSION_RIGHT;
            }
            else if (op == '<<')
            {
                new_expression->expression_type = SHIFT_EXPRESSION_LEFT;

            }

            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, op);


            int code = type_common(&new_expression->left->type, &new_expression->right->type, &new_expression->type);
            if (code != 0)
            {
                compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "invalid type shift expression");
                throw;
            }

            p_expression_node = move new_expression;
        }
    }
    catch
    {
    }

    return p_expression_node;
}

struct expression* owner relational_expression(struct parser_ctx* ctx)
{
    /*
    relational-expression:
        shift-expression
        relational-expression < shift-expression
        relational-expression > shift-expression
        relational-expression <= shift-expression
        relational-expression >= shift-expression
    */

    struct expression* owner p_expression_node = NULL;
    struct expression* owner new_expression = NULL;
    try
    {
        p_expression_node = move shift_expression(ctx);
        if (p_expression_node == NULL) throw;


        while (ctx->current != NULL &&
            (ctx->current->type == '>' ||
                ctx->current->type == '<' ||
                ctx->current->type == '>=' ||
                ctx->current->type == '<='))
        {
            assert(new_expression == NULL);
            new_expression = move calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            enum token_type op = ctx->current->type;
            parser_match(ctx);
            new_expression->left = move p_expression_node;

            new_expression->right = move shift_expression(ctx);
            if (new_expression->right == NULL) throw;

            if (op == '>')
            {
                new_expression->expression_type = RELATIONAL_EXPRESSION_BIGGER_THAN;

            }
            else if (op == '<')
            {
                new_expression->expression_type = RELATIONAL_EXPRESSION_LESS_THAN;
            }
            else if (op == '>=')
            {
                new_expression->expression_type = RELATIONAL_EXPRESSION_BIGGER_OR_EQUAL_THAN;
            }
            else if (op == '<=')
            {
                new_expression->expression_type = RELATIONAL_EXPRESSION_LESS_OR_EQUAL_THAN;
            }

            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, op);

            new_expression->type = move type_make_int_bool_like();

            p_expression_node = move new_expression;
            new_expression = NULL;/*MOVED*/
        }
    }
    catch
    {
    }

    return p_expression_node;
}

static void check_diferent_enuns(struct parser_ctx* ctx,
    const struct token* operator_token,
    struct expression* left,
    struct expression* right)
{
    if (left->type.type_specifier_flags & TYPE_SPECIFIER_ENUM &&
        right->type.type_specifier_flags & TYPE_SPECIFIER_ENUM)
    {
        if (left->type.enum_specifier->complete_enum_specifier !=
            right->type.enum_specifier->complete_enum_specifier)
        {
            const char* lefttag = "";
            if (left->type.enum_specifier->tag_token)
                lefttag = left->type.enum_specifier->tag_token->lexeme;

            const char* righttag = "";
            if (right->type.enum_specifier->tag_token)
                righttag = right->type.enum_specifier->tag_token->lexeme;

            /*
             * This comparison by name is not 100% correct because they be from
             * diferent scopes.
            */

            compiler_set_warning_with_token(W_ENUN_COMPARE,
                ctx,
                operator_token,
                "comparison between 'enum %s' and 'enum %s'",
                lefttag,
                righttag);
        }
    }

}

struct expression* owner equality_expression(struct parser_ctx* ctx)
{
    /*
      equality-expression:
       relational-expression
       equality-expression == relational-expression
       equality-expression != relational-expression
    */
    /*
    * Equality operators
    One of the following shall hold:
    — both operands have arithmetic type;
    — both operands are pointers to qualified or unqualified versions of compatible types;
    — one operand is a pointer to an object type and the other is a pointer to a qualified or unqualified
    version of void; or
    — one operand is a pointer and the other is a null pointer constant.
    */
    struct expression* owner p_expression_node = NULL;
    struct expression* owner new_expression = NULL;

    try
    {
        p_expression_node = move relational_expression(ctx);
        if (p_expression_node == NULL) throw;


        while (ctx->current != NULL &&
            (ctx->current->type == '==' ||
                ctx->current->type == '!='))
        {
            assert(new_expression == NULL);
            new_expression = move calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            struct  token* operator_token = ctx->current;
            parser_match(ctx);
            new_expression->left = move p_expression_node;
            new_expression->right = move relational_expression(ctx);
            if (new_expression->right == NULL) throw;

            check_diferent_enuns(ctx, operator_token, new_expression->left, new_expression->right);


            new_expression->first_token = operator_token;

            if (operator_token->type == '==')
            {
                new_expression->expression_type = EQUALITY_EXPRESSION_EQUAL;
                new_expression->constant_value =
                    constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, '==');

                if (new_expression->left->constant_value.type == TYPE_EMPTY ||
                    new_expression->right->constant_value.type == TYPE_EMPTY)
                {
                    new_expression->constant_value =
                        make_constant_value_ll(type_is_same(&new_expression->left->type, &new_expression->right->type, true));
                }
            }
            else if (operator_token->type == '!=')
            {
                new_expression->expression_type = EQUALITY_EXPRESSION_EQUAL;

                new_expression->constant_value =
                    constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, '!=');


                if (new_expression->left->constant_value.type == TYPE_EMPTY ||
                    new_expression->right->constant_value.type == TYPE_EMPTY)
                {
                    new_expression->constant_value = make_constant_value_ll
                    (!type_is_same(&new_expression->left->type, &new_expression->right->type, true));

                }
            }
            else
            {
                assert(false);
            }
            new_expression->type = move type_make_int_bool_like();
            p_expression_node = move new_expression;
            new_expression = NULL; /*MOVED*/
        }
    }
    catch
    {
    }

    return p_expression_node;
}

struct expression* owner and_expression(struct parser_ctx* ctx)
{
    /*
     AND-expression:
      equality-expression
      AND-expression & equality-expression
    */
    struct expression* owner p_expression_node = NULL;
    struct expression* owner new_expression = NULL;

    try
    {
        p_expression_node = move equality_expression(ctx);
        if (p_expression_node == NULL)
            throw;

        while (ctx->current != NULL && ctx->current->type == '&')
        {
            parser_match(ctx);

            assert(new_expression == NULL);
            new_expression = move calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            new_expression->expression_type = AND_EXPRESSION;
            new_expression->left = move p_expression_node;
            new_expression->right = move equality_expression(ctx);
            if (new_expression->right == NULL) throw;

            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, '&');


            int code = type_common(&new_expression->left->type, &new_expression->right->type, &new_expression->type);
            if (code != 0)
            {
                compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "invalid types and expression");
                throw;
            }

            p_expression_node = move new_expression;
            new_expression = NULL; /*MOVED*/
        }
    }
    catch
    {
    }

    return p_expression_node;
}

struct expression* owner exclusive_or_expression(struct parser_ctx* ctx)
{
    /*
     exclusive-OR-expression:
      AND-expression
      exclusive-OR-expression ^ AND-expression
    */
    struct expression* owner p_expression_node = NULL;
    struct expression* owner new_expression = NULL;

    try
    {
        p_expression_node = move and_expression(ctx);
        if (p_expression_node == NULL) throw;

        while (ctx->current != NULL &&
            (ctx->current->type == '^'))
        {
            parser_match(ctx);

            assert(new_expression == NULL);
            new_expression = move calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            new_expression->expression_type = EXCLUSIVE_OR_EXPRESSION;
            new_expression->left = move p_expression_node;
            new_expression->right = move and_expression(ctx);
            if (new_expression->right == NULL) throw;

            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, '^');


            int code = type_common(&new_expression->left->type, &new_expression->right->type, &new_expression->type);
            if (code != 0)
            {
                compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "invalid types or expression");
                throw;
            }

            p_expression_node = move new_expression;
            new_expression = NULL;
        }
    }
    catch
    {
    }

    return p_expression_node;
}

struct expression* owner inclusive_or_expression(struct parser_ctx* ctx)
{

    /*
    inclusive-OR-expression:
    exclusive-OR-expression
    inclusive-OR-expression | exclusive-OR-expression
    */
    struct expression* owner p_expression_node = NULL;
    try
    {
        p_expression_node = move exclusive_or_expression(ctx);
        if (p_expression_node == NULL) throw;

        while (ctx->current != NULL &&
            (ctx->current->type == '|'))
        {
            parser_match(ctx);
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            new_expression->expression_type = INCLUSIVE_OR_EXPRESSION;
            new_expression->left = move p_expression_node;
            new_expression->right = move exclusive_or_expression(ctx);
            if (new_expression->right == NULL) throw;

            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, '|');



            int code = type_common(&new_expression->left->type, &new_expression->right->type, &new_expression->type);
            if (code != 0)
            {
                compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "invalid types inclusive or expression");
                throw;
            }
            p_expression_node = move new_expression;
        }
    }
    catch
    {
    }

    return p_expression_node;
}

struct expression* owner logical_and_expression(struct parser_ctx* ctx)
{
    /*
    logical-AND-expression:
     inclusive-OR-expression
     logical-AND-expression && inclusive-OR-expression
    */
    struct expression* owner p_expression_node = NULL;
    try
    {
        p_expression_node = move inclusive_or_expression(ctx);
        if (p_expression_node == NULL) throw;

        while (ctx->current != NULL &&
            (ctx->current->type == '&&'))
        {
            parser_match(ctx);
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            new_expression->expression_type = INCLUSIVE_AND_EXPRESSION;
            new_expression->left = move p_expression_node;
            new_expression->right = move inclusive_or_expression(ctx);
            if (new_expression->right == NULL) throw;

            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, '&&');


            int code = type_common(&new_expression->left->type, &new_expression->right->type, &new_expression->type);
            if (code != 0)
            {
                type_print(&new_expression->left->type);
                type_print(&new_expression->right->type);
                compiler_set_error_with_token(C_INVALID_TYPE, ctx, ctx->current, "invalid types logicl and expression");
                throw;
            }
            p_expression_node = move new_expression;
        }
    }
    catch
    {
    }

    return p_expression_node;
}

struct expression* owner logical_or_expression(struct parser_ctx* ctx)
{
    /*
      logical-OR-expression:
       logical-AND-expression
       logical-OR-expression || logical-AND-expression
    */
    struct expression* owner p_expression_node = NULL;
    try
    {
        p_expression_node = move logical_and_expression(ctx);
        if (p_expression_node == NULL) throw;

        while (ctx->current != NULL &&
            (ctx->current->type == '||'))
        {
            parser_match(ctx);
            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            new_expression->expression_type = LOGICAL_OR_EXPRESSION;
            new_expression->left = move p_expression_node;
            new_expression->right = move logical_and_expression(ctx);
            if (new_expression->right == NULL) throw;


            new_expression->constant_value =
                constant_value_op(&new_expression->left->constant_value, &new_expression->right->constant_value, '||');


            if (!type_is_scalar(&new_expression->left->type))
            {
                compiler_set_error_with_token(C_LEFT_IS_NOT_SCALAR, ctx, ctx->current, "left type is not scalar for or expression");
                throw;
            }

            if (!type_is_scalar(&new_expression->right->type))
            {
                compiler_set_error_with_token(C_RIGHT_IS_NOT_SCALAR, ctx, ctx->current, "right type is not scalar for or expression");
                throw;
            }

            new_expression->type = move type_make_int_bool_like();

            p_expression_node = move new_expression;
        }
    }
    catch
    {
    }

    return p_expression_node;
}



struct expression* owner assignment_expression(struct parser_ctx* ctx)
{
    /*
    assignment-expression:
       conditional-expression
       unary-expression assignment-operator assignment-expression
       */
       /*
          assignment-operator: one of
          = *= /= %= += -= <<= >>= &= ^= |=
       */
       //aqui eh duvidoso mas conditional faz a unary tb.
       //a diferenca q nao eh qualquer expressao
       //que pode ser de atribuicao
    struct expression* owner p_expression_node = NULL;
    try
    {
        p_expression_node = move conditional_expression(ctx);
        if (p_expression_node == NULL) throw;

        while (ctx->current != NULL &&
            (ctx->current->type == '=' ||
                ctx->current->type == '*=' ||
                ctx->current->type == '/=' ||
                ctx->current->type == '%=' ||
                ctx->current->type == '+=' ||
                ctx->current->type == '-=' ||
                ctx->current->type == '<<=' ||
                ctx->current->type == '>>=' ||
                ctx->current->type == '&=' ||
                ctx->current->type == '^=' ||
                ctx->current->type == '|='))
        {
            const struct token* const op_token = ctx->current;
            parser_match(ctx);


            struct expression* owner new_expression = calloc(1, sizeof * new_expression);
            if (new_expression == NULL) throw;

            new_expression->first_token = ctx->current;
            new_expression->expression_type = ASSIGNMENT_EXPRESSION;
            new_expression->left = move p_expression_node;

            if (ctx->current->type == TK_KEYWORD__MOVE)
            {
                new_expression->move_assignment = ctx->current;
                parser_match(ctx);
            }

            enum type_category category =
                type_get_category(&new_expression->left->type);

            if (category == TYPE_CATEGORY_FUNCTION)
            {
                compiler_set_error_with_token(C_ASSIGNMENT_OF_FUNCTION, ctx, ctx->current, "assignment of function");
            }
            else if (category == TYPE_CATEGORY_ARRAY)
            {
                compiler_set_error_with_token(C_ASSIGNMENT_TO_EXPRESSION_WITH_ARRAY_TYPE, ctx, ctx->current, "assignment to expression with array type");
            }
            else
            {
                if (type_is_const(&new_expression->left->type))
                {
                    compiler_set_error_with_token(C_ASSIGNMENT_OF_READ_ONLY_OBJECT, ctx, ctx->current, "assignment of read-only object");
                }
            }

            if (type_is_lvalue(&new_expression->left->type))
            {
                //TODO
                //compiler_set_error_with_token(C_LVALUE_ASSIGNMENT, ctx, ctx->current, "lvalue assignment");
            }

            new_expression->right = move assignment_expression(ctx);
            if (new_expression->right == NULL) throw;

            if (op_token->type == '=')
            {
                check_assigment(ctx, &new_expression->left->type, new_expression->right, new_expression->move_assignment != NULL, false);
            }

            new_expression->type = move type_dup(&new_expression->right->type);
            new_expression->type.storage_class_specifier_flags |= STORAGE_SPECIFIER_LVALUE;

            new_expression->type.storage_class_specifier_flags &= ~STORAGE_SPECIFIER_FUNCTION_RETURN;
            new_expression->type.storage_class_specifier_flags &= ~STORAGE_SPECIFIER_FUNCTION_RETURN_NODISCARD;


            check_diferent_enuns(ctx, op_token, new_expression->left, new_expression->right);

            p_expression_node = move new_expression;
        }
    }
    catch
    {
    }
    return p_expression_node;
}

struct expression* owner expression(struct parser_ctx* ctx)
{
    /*expression:
      assignment-expression
      expression, assignment-expression
    */
    struct expression* owner p_expression_node = NULL;
    try
    {
        p_expression_node = move assignment_expression(ctx);
        if (p_expression_node == NULL) throw;

        if (ctx->current->type == ',')
        {
            while (ctx->current->type == ',')
            {
                parser_match(ctx);
                struct expression* owner p_expression_node_new = calloc(1, sizeof * p_expression_node_new);
                if (p_expression_node_new == NULL) throw;

                p_expression_node_new->first_token = ctx->current;
                p_expression_node_new->expression_type = ASSIGNMENT_EXPRESSION;
                p_expression_node_new->left = move p_expression_node;

                p_expression_node_new->right = move expression(ctx);
                if (p_expression_node_new->right == NULL) throw;

                p_expression_node = move p_expression_node_new;
            }

            if (p_expression_node->right == NULL) throw;

            /*same type of the last expression*/
            p_expression_node->type = move type_dup(&p_expression_node->right->type);
        }
    }
    catch
    {
    }
    return p_expression_node;
}

bool is_first_of_conditional_expression(struct parser_ctx* ctx)
{
    return is_first_of_unary_expression(ctx) ||
        is_first_of_primary_expression(ctx);
}

bool is_null_pointer_constant(const struct expression* expression)
{
    if (expression)
    {
        return constant_value_is_valid(&expression->constant_value) &&
            constant_value_to_bool(&expression->constant_value) == 0;
    }
    return false;
}

struct expression* owner conditional_expression(struct parser_ctx* ctx)
{
    /*
      conditional-expression:
      logical-OR-expression
      logical-OR-expression ? expression : conditional-expression
    */
    struct expression* owner p_expression_node = NULL;
    struct type left_type = {0};
    struct type right_type = {0};
    try
    {
        p_expression_node = move logical_or_expression(ctx);
        if (p_expression_node == NULL) throw;


        if (ctx->current && ctx->current->type == '?')
        {
            struct expression* owner p_conditional_expression = calloc(1, sizeof * p_conditional_expression);
            p_conditional_expression->first_token = ctx->current;
            p_conditional_expression->expression_type = CONDITIONAL_EXPRESSION;
            p_conditional_expression->condition_expr = move p_expression_node;
            p_expression_node = move p_conditional_expression;

            parser_match(ctx); //?

            p_conditional_expression->left = move expression(ctx);
            if (p_conditional_expression->left == NULL) throw;

            parser_match(ctx); //:
            p_conditional_expression->right = move conditional_expression(ctx);
            if (p_conditional_expression->right == NULL) throw;


            if (constant_value_is_valid(&p_conditional_expression->condition_expr->constant_value))
            {
                if (constant_value_to_bool(&p_conditional_expression->condition_expr->constant_value))
                {
                    /*this is an extensions.. in constant expression we can mix types!*/
                    p_conditional_expression->type = move type_dup(&p_conditional_expression->left->type);
                    p_conditional_expression->constant_value = p_conditional_expression->left->constant_value;
                    goto continuation;
                }
                else
                {
                    /*this is an extensions.. in constant expression we can mix types!*/
                    p_conditional_expression->type = move type_dup(&p_conditional_expression->right->type);
                    p_conditional_expression->constant_value = p_conditional_expression->right->constant_value;
                    goto continuation;
                }
            }

            if (expression_is_subjected_to_lvalue_conversion(p_conditional_expression->left))
            {
                left_type = move type_lvalue_conversion(&p_conditional_expression->left->type);
            }
            else
            {
                left_type = move type_dup(&p_conditional_expression->left->type);
            }

            if (expression_is_subjected_to_lvalue_conversion(p_conditional_expression->right))
            {
                right_type = move type_lvalue_conversion(&p_conditional_expression->right->type);
            }
            else
            {
                right_type = move type_dup(&p_conditional_expression->right->type);
            }

            /*The first operand shall have scalar type*/
            if (!type_is_scalar(&p_conditional_expression->condition_expr->type))
            {
                compiler_set_error_with_token(C_CONDITION_MUST_HAVE_SCALAR_TYPE, ctx, ctx->current, "condition must have scalar type");
                goto continuation;
            }

            /*
            *  both operands have arithmetic type;
            */
            if (type_is_arithmetic(&left_type) &&
                type_is_arithmetic(&right_type))
            {
                type_common(&left_type, &right_type, &p_conditional_expression->type);
                goto continuation;
            }

            /*
            *  both operands have compatible structure or union type;
            */
            if (type_is_struct_or_union(&left_type) !=
                type_is_struct_or_union(&right_type))
            {
                if (!type_is_same(&left_type, &right_type, true))
                {
                    compiler_set_error_with_token(C_INCOMPATIBLE_TYPES, ctx, p_conditional_expression->condition_expr->first_token, "incompatible types");
                }
                type_swap(&p_conditional_expression->type, &right_type);
                goto continuation;
            }

            /*
             *  both operands have void type;
            */
            if (type_is_void(&left_type) && type_is_void(&right_type))
            {
                type_swap(&p_conditional_expression->type, &right_type);
                goto continuation;
            }


            /* both operands have nullptr_t type;*/
            if (type_is_nullptr_t(&left_type) && type_is_nullptr_t(&right_type))
            {
                type_swap(&p_conditional_expression->type, &right_type);
                goto continuation; //ok
            }

            /*
                — one operand is a pointer and the other is a null pointer constant or has type nullptr_t; or
                    — one operand is a pointer to an object type and the other is a pointer to a qualified or unqualified
                    version of void
            */
            if (type_is_pointer(&left_type))
            {
                if (is_null_pointer_constant(p_conditional_expression->right) ||
                    type_is_nullptr_t(&right_type) ||
                    type_is_void_ptr(&right_type))
                {
                    type_swap(&p_conditional_expression->type, &left_type);
                    goto continuation; //ok
                }

                if (type_is_pointer(&right_type))
                {
                    if (type_is_nullptr_t(&left_type) || type_is_void_ptr(&left_type))
                    {
                        type_swap(&p_conditional_expression->type, &left_type);
                        goto continuation; //ok
                    }

                    if (!type_is_same(&left_type, &right_type, false))
                    {
                        type_print(&left_type);
                        type_print(&right_type);
                        compiler_set_error_with_token(C_INCOMPATIBLE_TYPES, ctx, ctx->current, "incompatible types");
                        goto continuation;
                    }
                    type_swap(&p_conditional_expression->type, &right_type);
                    goto continuation;
                }

                compiler_set_error_with_token(C_INCOMPATIBLE_TYPES, ctx, p_conditional_expression->condition_expr->first_token, "incompatible types");
                goto continuation;
            }

            if (type_is_pointer(&right_type))
            {
                if (is_null_pointer_constant(p_conditional_expression->left) ||
                    type_is_nullptr_t(&left_type) ||
                    type_is_void_ptr(&left_type))
                {
                    type_swap(&p_conditional_expression->type, &right_type);
                    goto continuation; //ok
                }

                if (type_is_pointer(&left_type))
                {
                    if (type_is_nullptr_t(&left_type) || type_is_void_ptr(&left_type))
                    {
                        type_swap(&p_conditional_expression->type, &right_type);
                        goto continuation; //ok
                    }

                    if (!type_is_same(&left_type, &right_type, false))
                    {
                        compiler_set_error_with_token(C_INCOMPATIBLE_TYPES, ctx, p_conditional_expression->condition_expr->first_token, "incompatible types");
                        goto continuation;
                    }
                    type_swap(&p_conditional_expression->type, &right_type);
                    goto continuation;
                }

                compiler_set_error_with_token(C_INCOMPATIBLE_TYPES, ctx, p_conditional_expression->condition_expr->first_token, "incompatible types");
                goto continuation;
            }
        continuation:;
        }
    }
    catch
    {
    }

    type_destroy(&left_type);
    type_destroy(&right_type);

    return p_expression_node;
}

struct expression* owner constant_expression(struct parser_ctx* ctx)
{
    struct expression* owner p_expression = conditional_expression(ctx);


    if (p_expression && !constant_value_is_valid(&p_expression->constant_value))
    {
        compiler_set_error_with_token(C_EXPECTED_CONSTANT_EXPRESSION, ctx, ctx->current, "expected constant expression");
    }

    return p_expression;
}

bool expression_is_zero(struct expression* p_expression)
{
    bool is_zero = false;

    if (type_is_nullptr_t(&p_expression->type) ||
        (constant_value_is_valid(&p_expression->constant_value) &&
            constant_value_to_ull(&p_expression->constant_value) == 0))
    {
        is_zero = true;
    }
    return is_zero;
}

/*
* Returns true if the type of expression is subjected to type_lvalue_conversion
*/
bool expression_is_subjected_to_lvalue_conversion(struct expression* expression)
{

    switch (expression->expression_type)
    {
        case UNARY_EXPRESSION_ADDRESSOF:
        case UNARY_EXPRESSION_INCREMENT:
        case UNARY_EXPRESSION_DECREMENT:
        case POSTFIX_INCREMENT:
        case POSTFIX_DECREMENT:
            return false;
    }

    if (expression->type.storage_class_specifier_flags & STORAGE_SPECIFIER_PARAMETER)
        return true;

    return true;
}
