#include "expressions.h"
#include "type.h"

struct adatype_ctx {
    int pass;
};


enum adatype_fact_kind {
    ADATYPE_ISNUMBER,
};

struct adatype_fact {
    enum adatype_fact_kind kind;
};

// The set of possible values for an expression.
struct adatype {
    enum type_category category;
};
