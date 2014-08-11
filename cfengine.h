#if defined(__FreeBSD__)
#include <sys/stat.h>
#endif

// Stripped down version of cf3.defs.h retrieved from https://github.com/cfengine/core/blob/master/libpromises/cf3.defs.h

typedef struct Writer_ Writer;
typedef struct Policy_ Policy;
typedef struct Bundle_ Bundle;
typedef struct Body_ Body;
typedef struct SubType_ SubType;
typedef struct Rlist_ Rlist;
typedef struct Constraint_ Constraint;
typedef struct Item_ Item;
typedef struct CompressedArray_ CompressedArray;
typedef struct Audit_ Audit;
typedef struct Stat_ Stat;
typedef struct _Promise Promise;

typedef enum
{
    POLICY_ELEMENT_TYPE_BUNDLE,
    POLICY_ELEMENT_TYPE_BODY,
    POLICY_ELEMENT_TYPE_SUBTYPE,
    POLICY_ELEMENT_TYPE_PROMISE,
    POLICY_ELEMENT_TYPE_CONSTRAINT
} PolicyElementType;

enum cfagenttype
{
    cf_common,
    cf_agent,
    cf_server,
    cf_monitor,
    cf_executor,
    cf_runagent,
    cf_know,
    cf_report,
    cf_keygen,
    cf_hub,
    cf_gendoc,
    cf_noagent
};

enum cf_filetype
{
    cf_reg,
    cf_link,
    cf_dir,
    cf_fifo,
    cf_block,
    cf_char,
    cf_sock
};

typedef enum
{
    REPORT_OUTPUT_TYPE_TEXT,
    REPORT_OUTPUT_TYPE_HTML,
    REPORT_OUTPUT_TYPE_KNOWLEDGE,

    REPORT_OUTPUT_TYPE_MAX
} ReportOutputType;

typedef struct
{
    Writer *report_writers[REPORT_OUTPUT_TYPE_MAX];
} ReportContext;



struct CompressedArray_
{
    int key;
    char *value;
    CompressedArray *next;
};

typedef struct
{
    char *filename;
    Item *file_start;
    Item *file_classes;
    int num_edits;
    int empty_first;
#ifdef HAVE_LIBXML2
    xmlDocPtr xmldoc;
#endif

} EditContext;

#define CF_SMALLBUF 128
#define CF_MAX_IP_LEN 64
#define EVP_MAX_MD_SIZE

struct Rlist_
{
    void *item;
    char type;
    Rlist *state_ptr;
    Rlist *next;
};

typedef struct
{
    size_t start;
    size_t end;
    size_t line;
    size_t context;
} SourceOffset;

struct Audit_
{
    char *version;
    char *filename;
    char *date;
    unsigned char digest[100 + 1];
    Audit *next;
};

typedef struct
{
    void *item;
    char rtype;
} Rval;

typedef struct
{
    int sd;
    int trust;
    int authenticated;
    int protoversion;
    int family;
    char username[CF_SMALLBUF];
    char localip[CF_MAX_IP_LEN];
    char remoteip[CF_MAX_IP_LEN];
    unsigned char digest[EVP_MAX_MD_SIZE + 1];
    unsigned char *session_key;
    char encryption_type;
    short error;
} AgentConnection;

struct Stat_
{
    char *cf_filename;          /* What file are we statting? */
    char *cf_server;            /* Which server did this come from? */
    enum cf_filetype cf_type;   /* enum filetype */
    mode_t cf_lmode;            /* Mode of link, if link */
    mode_t cf_mode;             /* Mode of remote file, not link */
    uid_t cf_uid;               /* User ID of the file's owner */
    gid_t cf_gid;               /* Group ID of the file's group */
    off_t cf_size;              /* File size in bytes */
    time_t cf_atime;            /* Time of last access */
    time_t cf_mtime;            /* Time of last data modification */
    time_t cf_ctime;            /* Time of last file status change */
    char cf_makeholes;          /* what we need to know from blksize and blks */
    char *cf_readlink;          /* link value or NULL */
    int cf_failed;              /* stat returned -1 */
    int cf_nlink;               /* Number of hard links */
    int cf_ino;                 /* inode number on server */
    dev_t cf_dev;               /* device number */
    Stat *next;
};

struct _Promise
{
    SubType *parent_subtype;

    char *classes;
    char *ref;                  /* comment */
    char ref_alloc;
    char *promiser;
    Rval promisee;
    char *bundle;
    Audit *audit;
    Constraint *conlist;
    Promise *next;

    /* Runtime bus for private flags and work space */

    char *agentsubtype;         /* cache the promise subtype */
    char *bundletype;           /* cache the agent type */
    char *namespace;            /* cache the namespace */
    int done;                   /* this needs to be preserved across runs */
    int *donep;                 /* used by locks to mark as done */
    int makeholes;
    char *this_server;
    int has_subbundles;
    Stat *cache;
    AgentConnection *conn;
    CompressedArray *inode_cache;
    EditContext *edcontext;
    dev_t rootdevice;           /* for caching during work */
    const Promise *org_pp;            /* A ptr to the unexpanded raw promise */

    SourceOffset offset;
};

struct Bundle_
{
    Policy *parent_policy;

    char *type;
    char *name;
    char *namespace;
    Rlist *args;
    SubType *subtypes;
    struct Bundle_ *next;

    char *source_path;
    SourceOffset offset;
};

struct SubType_
{
    Bundle *parent_bundle;

    char *name;
    Promise *promiselist;
    SubType *next;

    SourceOffset offset;
};

struct Constraint_
{
    PolicyElementType type;
    union {
        Promise *promise;
        Body *body;
    } parent;

    char *lval;
    Rval rval;

    char *classes;              /* only used within bodies */
    bool references_body;
    Audit *audit;

    SourceOffset offset;
    Constraint *next;
};


