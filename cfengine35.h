
typedef struct Policy_ Policy;
typedef struct Bundle_ Bundle;
typedef struct PromiseType_ PromiseType;
typedef struct Promise_ Promise;
typedef struct EvalContext_ EvalContext;
typedef void PromiseActuator(EvalContext *ctx, Promise *pp, void *param);
typedef struct Rlist_ Rlist;
typedef struct Map_ Map;
typedef Map Set;
typedef bool (*MapKeyEqualFn)(const void *key1, const void *key2);
typedef void (*MapDestroyDataFn)(void *key);
typedef unsigned int (*MapHashFn) (const void *p, unsigned int max);

typedef struct
{
    void* key;
    void *value;
} MapKeyValue;

typedef struct BucketListItem_
{
    MapKeyValue value;
    struct BucketListItem_ *next;
} BucketListItem;

typedef struct
{
    MapKeyEqualFn equal_fn;
    MapDestroyDataFn destroy_key_fn;
    MapDestroyDataFn destroy_value_fn;
    MapKeyValue *values;
    short size;
} ArrayMap;

typedef struct
{
    ArrayMap *map;
    int pos;
} ArrayMapIterator;


typedef struct
{
    MapHashFn hash_fn;
    MapKeyEqualFn equal_fn;
    MapDestroyDataFn destroy_key_fn;
    MapDestroyDataFn destroy_value_fn;
    BucketListItem **buckets;
} HashMap;

typedef struct
{
    HashMap *map;
    BucketListItem *cur;
    int bucket;
} HashMapIterator;

typedef struct
{
    bool is_array;
    union
    {   
        ArrayMapIterator arraymap_iter;
        HashMapIterator hashmap_iter;
    };      
} MapIterator;
typedef MapIterator SetIterator;


#define TYPED_SET_DECLARE(Prefix, ElementType)                          \
    typedef struct                                                      \
    {                                                                   \
        Set *impl;                                                      \
    } Prefix##Set;                                                      \
                                                                        \
    typedef SetIterator Prefix##SetIterator;                            \
                                                                        \
    Prefix##Set *Prefix##SetNew(void);                                  \
    void Prefix##SetAdd(const Prefix##Set *set, ElementType element);   \
    bool Prefix##SetContains(const Prefix##Set *Set, const ElementType element);  \
    bool Prefix##SetRemove(const Prefix##Set *Set, const ElementType element);  \
    void Prefix##SetClear(Prefix##Set *set);                            \
    size_t Prefix##SetSize(const Prefix##Set *set);                     \
    void Prefix##SetDestroy(Prefix##Set *set);                          \
    Prefix##SetIterator Prefix##SetIteratorInit(Prefix##Set *set);      \
    ElementType Prefix##SetIteratorNext(Prefix##SetIterator *iter);     \

#define TYPED_SET_DEFINE(Prefix, ElementType, hash_fn, equal_fn, destroy_fn) \
                                                                        \
    Prefix##Set *Prefix##SetNew(void)                                   \
    {                                                                   \
        Prefix##Set *set = xcalloc(1, sizeof(Prefix##Set));             \
        set->impl = SetNew(hash_fn, equal_fn, destroy_fn);              \
        return set;                                                     \
    }                                                                   \
                                                                        \
    void Prefix##SetAdd(const Prefix##Set *set, ElementType element)    \
    {                                                                   \
        SetAdd(set->impl, (void *)element);                             \
    }                                                                   \
                                                                        \
    bool Prefix##SetContains(const Prefix##Set *set, const ElementType element)   \
    {                                                                   \
        return SetContains(set->impl, element);                         \
    }                                                                   \
                                                                        \
    bool Prefix##SetRemove(const Prefix##Set *set, const ElementType element)   \
    {                                                                   \
        return SetRemove(set->impl, element);                           \
    }                                                                   \
                                                                        \
    void Prefix##SetClear(Prefix##Set *set)                             \
    {                                                                   \
        return SetClear(set->impl);                                     \
    }                                                                   \
                                                                        \
    size_t Prefix##SetSize(const Prefix##Set *set)                      \
    {                                                                   \
        return SetSize(set->impl);                                      \
    }                                                                   \
                                                                        \
    void Prefix##SetDestroy(Prefix##Set *set)                           \
    {                                                                   \
        if (set)                                                        \
        {                                                               \
            SetDestroy(set->impl);                                      \
            free(set);                                                  \
        }                                                               \
    }                                                                   \
                                                                        \
    Prefix##SetIterator Prefix##SetIteratorInit(Prefix##Set *set)       \
    {                                                                   \
        return SetIteratorInit(set->impl);                              \
    }                                                                   \
                                                                        \
    ElementType Prefix##SetIteratorNext(Prefix##SetIterator *iter)      \
    {                                                                   \
        return SetIteratorNext(iter);                                   \
    }                                                                   \

TYPED_SET_DECLARE(String, char *)

typedef enum 
{
    RVAL_TYPE_SCALAR = 's', 
    RVAL_TYPE_LIST = 'l', 
    RVAL_TYPE_FNCALL = 'f', 
    RVAL_TYPE_NOPROMISEE = 'X'
} RvalType;

typedef enum 
{
    AGENT_TYPE_COMMON,
    AGENT_TYPE_AGENT,
    AGENT_TYPE_SERVER,
    AGENT_TYPE_MONITOR,
    AGENT_TYPE_EXECUTOR,
    AGENT_TYPE_RUNAGENT,
    AGENT_TYPE_KEYGEN,
    AGENT_TYPE_HUB,
    AGENT_TYPE_GENDOC,
    AGENT_TYPE_NOAGENT
} AgentType;


struct Map_
{
    MapHashFn hash_fn;

    union
    {   
        ArrayMap *arraymap;
        HashMap *hashmap;
    };      
};

struct Rlist_
{
    void *item;
    RvalType type;
    Rlist *state_ptr;           /* Points to "current" state/element of sub-list */
    Rlist *next;
};

typedef struct
{
    AgentType agent_type;

    Rlist *bundlesequence;

    char *original_input_file;
    char *input_file;
    char *input_dir;

    bool check_not_writable_by_others;
    bool check_runnable;

    StringSet *heap_soft;
    StringSet *heap_negated;

    bool tty_interactive; // agent is running interactively, via tty/terminal interface

    // change to evaluation behavior from the policy itself
    bool ignore_missing_bundles;
    bool ignore_missing_inputs;

    struct
    {   
        struct
        {   
            enum
            {   
                GENERIC_AGENT_CONFIG_COMMON_POLICY_OUTPUT_FORMAT_NONE,
                GENERIC_AGENT_CONFIG_COMMON_POLICY_OUTPUT_FORMAT_CF,
                GENERIC_AGENT_CONFIG_COMMON_POLICY_OUTPUT_FORMAT_JSON
            } policy_output_format;
            unsigned int parser_warnings;
            unsigned int parser_warnings_error;
        } common;
        struct
        {   
            char *bootstrap_policy_server;
        } agent;
    } agent_specific;

} GenericAgentConfig;

typedef struct
{
    void **data;
    size_t length;
    size_t capacity;
    void (*ItemDestroy) (void *item);
} Seq;

typedef struct
{
    size_t start;
    size_t end;
    size_t line;
    size_t context;
} SourceOffset;

typedef struct
{
    void *item;
    RvalType type;
} Rval;

struct Policy_
{
    Seq *bundles;
    Seq *bodies;
};

struct Bundle_
{
    Policy *parent_policy;

    char *type;
    char *name;
    char *ns;
    Rlist *args;

    Seq *promise_types;

    char *source_path;
    SourceOffset offset;
};

struct PromiseType_
{
    Bundle *parent_bundle;

    char *name;
    Seq *promises;

    SourceOffset offset;
};

struct Promise_
{
    PromiseType *parent_promise_type;

    char *classes;
    char *comment;
    char *promiser;
    Rval promisee;
    Seq *conlist;
    bool has_subbundles;

    const Promise *org_pp;            /* A ptr to the unexpanded raw promise */

    SourceOffset offset;
};

