#include <stdlib.h>
#include <string.h>

#include "ast.h"
#include "log.h"
#include "symbol_table.h"
#include "validate.h"

int ni_validate_top_level(ni_ast_node_list *ast);
int ni_validate_node_list(ni_ast_node_list *ast);
int ni_validate_node(ni_ast_node *ast_node);
int ni_validate_node_fun(ni_ast_node *ast_node);
int ni_validate_node_const(ni_ast_node *ast_node);
int ni_validate_node_var(ni_ast_node *ast_node);
int ni_validate_node_arg(ni_ast_node *ast_node);
int ni_validate_node_var_assign(ni_ast_node *ast_node);
int ni_validate_node_var_lookup(ni_ast_node *ast_node);
int ni_validate_node_bin_op(ni_ast_node *ast_node);
int ni_validate_node_call(ni_ast_node *ast_node);
int ni_validate_node_ret(ni_ast_node *ast_node);
int ni_check_type_assignment(ni_ast_node *ast_node,
                             const ni_ast_type *ast_type);
void ni_set_type_on_const(ni_ast_node *ast_node,
                          ni_ast_internal_type internal_type);

symbol_table *validate_symbol_table = NULL;
ni_ast_node *fun = NULL;

// clang-format off
ni_ast_internal_type convert_arithmetic_op[19][19] = { // + - *
/*                                       NI_AST_INTERNAL_TYPE_CONST_INT     NI_AST_INTERNAL_TYPE_INT           NI_AST_INTERNAL_TYPE_INT8          NI_AST_INTERNAL_TYPE_INT16         NI_AST_INTERNAL_TYPE_INT32         NI_AST_INTERNAL_TYPE_INT64         NI_AST_INTERNAL_TYPE_INT128        NI_AST_INTERNAL_TYPE_CONST_UINT    NI_AST_INTERNAL_TYPE_UINT          NI_AST_INTERNAL_TYPE_UINT8         NI_AST_INTERNAL_TYPE_UINT16        NI_AST_INTERNAL_TYPE_UINT32        NI_AST_INTERNAL_TYPE_UINT64        NI_AST_INTERNAL_TYPE_UINT128       NI_AST_INTERNAL_TYPE_CONST_FLOAT   NI_AST_INTERNAL_TYPE_FLOAT16       NI_AST_INTERNAL_TYPE_FLOAT32       NI_AST_INTERNAL_TYPE_FLOAT64       NI_AST_INTERNAL_TYPE_BOOL          */
/* NI_AST_INTERNAL_TYPE_CONST_INT   */ { NI_AST_INTERNAL_TYPE_CONST_INT   , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_INT8        , NI_AST_INTERNAL_TYPE_INT16       , NI_AST_INTERNAL_TYPE_INT32       , NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT128     ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_INT         */ { NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_INT8        */ { NI_AST_INTERNAL_TYPE_INT8        , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT8        , NI_AST_INTERNAL_TYPE_INT16       , NI_AST_INTERNAL_TYPE_INT32       , NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT128     ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_INT16       */ { NI_AST_INTERNAL_TYPE_INT16       , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT16       , NI_AST_INTERNAL_TYPE_INT16       , NI_AST_INTERNAL_TYPE_INT32       , NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT128     ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_INT32       */ { NI_AST_INTERNAL_TYPE_INT32       , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT32       , NI_AST_INTERNAL_TYPE_INT32       , NI_AST_INTERNAL_TYPE_INT32       , NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT128     ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_INT64       */ { NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT         , NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT64       , NI_AST_INTERNAL_TYPE_INT128     ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_INT128      */ { NI_AST_INTERNAL_TYPE_INT128      , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_INT128      , NI_AST_INTERNAL_TYPE_INT128      , NI_AST_INTERNAL_TYPE_INT128      , NI_AST_INTERNAL_TYPE_INT128      , NI_AST_INTERNAL_TYPE_INT128     ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_CONST_UINT  */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_CONST_UINT  , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UINT8       , NI_AST_INTERNAL_TYPE_UINT16      , NI_AST_INTERNAL_TYPE_UINT32      , NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_UINT        */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_UINT8       */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UINT8       , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT8       , NI_AST_INTERNAL_TYPE_UINT16      , NI_AST_INTERNAL_TYPE_UINT32      , NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_UINT16      */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UINT16      , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT16      , NI_AST_INTERNAL_TYPE_UINT16      , NI_AST_INTERNAL_TYPE_UINT32      , NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_UINT32      */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UINT32      , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT32      , NI_AST_INTERNAL_TYPE_UINT32      , NI_AST_INTERNAL_TYPE_UINT32      , NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_UINT64      */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT        , NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT64      , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_UINT128     */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UINT128     , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_CONST_FLOAT */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_CONST_FLOAT , NI_AST_INTERNAL_TYPE_FLOAT16     , NI_AST_INTERNAL_TYPE_FLOAT32     , NI_AST_INTERNAL_TYPE_FLOAT64     , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_FLOAT16     */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_FLOAT16     , NI_AST_INTERNAL_TYPE_FLOAT16     , NI_AST_INTERNAL_TYPE_FLOAT32     , NI_AST_INTERNAL_TYPE_FLOAT64     , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_FLOAT32     */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_FLOAT32     , NI_AST_INTERNAL_TYPE_FLOAT32     , NI_AST_INTERNAL_TYPE_FLOAT32     , NI_AST_INTERNAL_TYPE_FLOAT64     , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_FLOAT64     */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_FLOAT64     , NI_AST_INTERNAL_TYPE_FLOAT64     , NI_AST_INTERNAL_TYPE_FLOAT64     , NI_AST_INTERNAL_TYPE_FLOAT64     , NI_AST_INTERNAL_TYPE_UNDEF         }, 
/* NI_AST_INTERNAL_TYPE_BOOL        */ { NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF      ,  NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_UNDEF       , NI_AST_INTERNAL_TYPE_BOOL          }, 
};

int convert_assign[19][19] = { // a = b
/*                               INTO -> NI_AST_INTERNAL_TYPE_CONST_INT     NI_AST_INTERNAL_TYPE_INT           NI_AST_INTERNAL_TYPE_INT8          NI_AST_INTERNAL_TYPE_INT16         NI_AST_INTERNAL_TYPE_INT32         NI_AST_INTERNAL_TYPE_INT64         NI_AST_INTERNAL_TYPE_INT128        NI_AST_INTERNAL_TYPE_CONST_UINT    NI_AST_INTERNAL_TYPE_UINT          NI_AST_INTERNAL_TYPE_UINT8         NI_AST_INTERNAL_TYPE_UINT16        NI_AST_INTERNAL_TYPE_UINT32        NI_AST_INTERNAL_TYPE_UINT64        NI_AST_INTERNAL_TYPE_UINT128       NI_AST_INTERNAL_TYPE_CONST_FLOAT   NI_AST_INTERNAL_TYPE_FLOAT16       NI_AST_INTERNAL_TYPE_FLOAT32       NI_AST_INTERNAL_TYPE_FLOAT64      NI_AST_INTERNAL_TYPE_BOOL     */
/*              FROM                */
/* NI_AST_INTERNAL_TYPE_CONST_INT   */ { 1,                                 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT         */ { 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT8        */ { 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT16       */ { 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT32       */ { 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT64       */ { 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT128      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_CONST_UINT  */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT8       */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT16      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT32      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT64      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT128     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_CONST_FLOAT */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 0,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT16     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT32     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT64     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                1,                                },
/* NI_AST_INTERNAL_TYPE_BOOL        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                0,                                },
};

int convert_num_comparison_op[19][19] = { // <= < > =>
/*                                       NI_AST_INTERNAL_TYPE_CONST_INT     NI_AST_INTERNAL_TYPE_INT           NI_AST_INTERNAL_TYPE_INT8          NI_AST_INTERNAL_TYPE_INT16         NI_AST_INTERNAL_TYPE_INT32         NI_AST_INTERNAL_TYPE_INT64         NI_AST_INTERNAL_TYPE_INT128        NI_AST_INTERNAL_TYPE_CONST_UINT    NI_AST_INTERNAL_TYPE_UINT          NI_AST_INTERNAL_TYPE_UINT8         NI_AST_INTERNAL_TYPE_UINT16        NI_AST_INTERNAL_TYPE_UINT32        NI_AST_INTERNAL_TYPE_UINT64        NI_AST_INTERNAL_TYPE_UINT128       NI_AST_INTERNAL_TYPE_CONST_FLOAT   NI_AST_INTERNAL_TYPE_FLOAT16       NI_AST_INTERNAL_TYPE_FLOAT32       NI_AST_INTERNAL_TYPE_FLOAT64      NI_AST_INTERNAL_TYPE_BOOL     */
/* NI_AST_INTERNAL_TYPE_CONST_INT   */ { 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT         */ { 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT8        */ { 0,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT16       */ { 0,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT32       */ { 0,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT64       */ { 0,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT128      */ { 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_CONST_UINT  */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT8       */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT16      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT32      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT64      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT128     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_CONST_FLOAT */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 0,                                 0,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT16     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT32     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT64     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 0,                                1,                                },
/* NI_AST_INTERNAL_TYPE_BOOL        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
};

int convert_equals_comparison_op[19][19] = { // == !=
/*                                       NI_AST_INTERNAL_TYPE_CONST_INT     NI_AST_INTERNAL_TYPE_INT           NI_AST_INTERNAL_TYPE_INT8          NI_AST_INTERNAL_TYPE_INT16         NI_AST_INTERNAL_TYPE_INT32         NI_AST_INTERNAL_TYPE_INT64         NI_AST_INTERNAL_TYPE_INT128        NI_AST_INTERNAL_TYPE_CONST_UINT    NI_AST_INTERNAL_TYPE_UINT          NI_AST_INTERNAL_TYPE_UINT8         NI_AST_INTERNAL_TYPE_UINT16        NI_AST_INTERNAL_TYPE_UINT32        NI_AST_INTERNAL_TYPE_UINT64        NI_AST_INTERNAL_TYPE_UINT128       NI_AST_INTERNAL_TYPE_CONST_FLOAT   NI_AST_INTERNAL_TYPE_FLOAT16       NI_AST_INTERNAL_TYPE_FLOAT32       NI_AST_INTERNAL_TYPE_FLOAT64      NI_AST_INTERNAL_TYPE_BOOL     */
/* NI_AST_INTERNAL_TYPE_CONST_INT   */ { 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT         */ { 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT8        */ { 0,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT16       */ { 0,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT32       */ { 0,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT64       */ { 0,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT128      */ { 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_CONST_UINT  */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT8       */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT16      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT32      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT64      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT128     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_CONST_FLOAT */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 0,                                 0,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT16     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 0,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT32     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT64     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 0,                                 1,                                 1,                                 0,                                1,                                },
/* NI_AST_INTERNAL_TYPE_BOOL        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                0,                                },
};

int convert_bool_comparison_op[19][19] = { // && ||
/*                                       NI_AST_INTERNAL_TYPE_CONST_INT     NI_AST_INTERNAL_TYPE_INT           NI_AST_INTERNAL_TYPE_INT8          NI_AST_INTERNAL_TYPE_INT16         NI_AST_INTERNAL_TYPE_INT32         NI_AST_INTERNAL_TYPE_INT64         NI_AST_INTERNAL_TYPE_INT128        NI_AST_INTERNAL_TYPE_CONST_UINT    NI_AST_INTERNAL_TYPE_UINT          NI_AST_INTERNAL_TYPE_UINT8         NI_AST_INTERNAL_TYPE_UINT16        NI_AST_INTERNAL_TYPE_UINT32        NI_AST_INTERNAL_TYPE_UINT64        NI_AST_INTERNAL_TYPE_UINT128       NI_AST_INTERNAL_TYPE_CONST_FLOAT   NI_AST_INTERNAL_TYPE_FLOAT16       NI_AST_INTERNAL_TYPE_FLOAT32       NI_AST_INTERNAL_TYPE_FLOAT64      NI_AST_INTERNAL_TYPE_BOOL     */
/* NI_AST_INTERNAL_TYPE_CONST_INT   */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT         */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT8        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT16       */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT32       */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT64       */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_INT128      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_CONST_UINT  */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT8       */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT16      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT32      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT64      */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_UINT128     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_CONST_FLOAT */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT16     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT32     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_FLOAT64     */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                1,                                },
/* NI_AST_INTERNAL_TYPE_BOOL        */ { 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                 1,                                0,                                },
};

// clang-format on

int ni_validate(ni_ast_node_list *ast) {
  validate_symbol_table = make_symbol_table(SYMBOL_TABLE_SIZE, NULL);

  int ret = ni_validate_top_level(ast);

  free_all_symbol_tables(validate_symbol_table);
  return ret;
}

int ni_validate_top_level(ni_ast_node_list *ast) {
  int ret = 0;
  ni_ast_node_list_entry *ast_current_node = ast->head;
  while (ast_current_node != NULL) {
    switch (ast_current_node->node->node_type) {
    case NI_AST_NODE_TYPE_FUNCTION_DECLARATION:
      if (insert_symbol_table(
              validate_symbol_table,
              ast_current_node->node->function_declaration.identifier,
              ast_current_node->node, NULL, NULL) != 0) {
        ret = 1;
        log_validation_error(
            ast_current_node->node->location, "'%s' already defined",
            ast_current_node->node->function_declaration.identifier);
      }
      break;
    case NI_AST_NODE_TYPE_CONSTANT_DECLARATION:
      break; // validate later
    default:
      ret = 1;
      log_validation_error(ast_current_node->node->location,
                           "unexpected token");
      break;
    }

    ast_current_node = ast_current_node->next;
  }

  ast_current_node = ast->head;
  while (ast_current_node != NULL) {
    if (ni_validate_node(ast_current_node->node) != 0) {
      ret = 1;
    }
    ast_current_node = ast_current_node->next;
  }
  return ret;
}

int ni_validate_node_list(ni_ast_node_list *ast) {
  if (ast == NULL) {
    return 0;
  }

  int ret = 0;
  ni_ast_node_list_entry *ast_current_node = ast->head;
  while (ast_current_node != NULL) {
    if (ni_validate_node(ast_current_node->node) != 0) {
      ret = 1;
    }
    ast_current_node = ast_current_node->next;
  }
  return ret;
}

int ni_validate_node(ni_ast_node *ast_node) {
  if (ast_node == NULL) {
    return 0;
  }

  switch (ast_node->node_type) {
  case NI_AST_NODE_TYPE_FUNCTION_DECLARATION:
    return ni_validate_node_fun(ast_node);
  case NI_AST_NODE_TYPE_CONSTANT_DECLARATION:
    return ni_validate_node_const(ast_node);
  case NI_AST_NODE_TYPE_BLOCK:
    return ni_validate_node_list(ast_node->block);
  case NI_AST_NODE_TYPE_VARIABLE_DECLARATION:
    return ni_validate_node_var(ast_node);
  case NI_AST_NODE_TYPE_BIN_OP:
    return ni_validate_node_bin_op(ast_node);
  case NI_AST_NODE_TYPE_VARIABLE_ASSIGNMENT:
    return ni_validate_node_var_assign(ast_node);
  case NI_AST_NODE_TYPE_VARIABLE_LOOKUP:
    return ni_validate_node_var_lookup(ast_node);
  case NI_AST_NODE_TYPE_FUNCTION_CALL:
    return ni_validate_node_call(ast_node);
  case NI_AST_NODE_TYPE_FUNCTION_RETURN:
    return ni_validate_node_ret(ast_node);
  case NI_AST_NODE_TYPE_ARGUMENT:
    return ni_validate_node_arg(ast_node);
  default:
    return 0; // no validation
  }
}

int ni_validate_node_fun(ni_ast_node *ast_node) {
  symbol_table *new_table = make_symbol_table(SYMBOL_TABLE_SIZE, validate_symbol_table);
  validate_symbol_table = new_table;
  fun = ast_node;

  int ret = ni_validate_node_list(ast_node->function_declaration.argument_list);

  if (ni_validate_node_list(ast_node->function_declaration.block) != 0) {
    ret = 1;
  }

  fun = NULL;

  validate_symbol_table = validate_symbol_table->parent;
  free_symbol_table(new_table);
  return ret;
}

int ni_validate_node_var(ni_ast_node *ast_node) {
  if (ni_validate_node(ast_node->variable_declaration.value) != 0) {
    return 1;
  }
  if (insert_symbol_table(validate_symbol_table, ast_node->variable_declaration.identifier,
                          ast_node, NULL, NULL) != 0) {
    log_validation_error(ast_node->location, "variable '%s' already defined",
                         ast_node->variable_declaration.identifier);
    return 1;
  }
  return 0;
}

int ni_validate_node_const(ni_ast_node *ast_node) {
  if (insert_symbol_table(validate_symbol_table,
                          ast_node->constant_declaration.identifier, ast_node,
                          NULL, NULL) != 0) {
    log_validation_error(ast_node->location, "constant '%s' already defined",
                         ast_node->constant_declaration.identifier);
    return 1;
  }
  return 0;
}

int ni_validate_node_arg(ni_ast_node *ast_node) {
  if (insert_symbol_table(validate_symbol_table, ast_node->argument.identifier, ast_node,
                          NULL, NULL) != 0) {
    log_validation_error(ast_node->location, "argument '%s' already defined",
                         ast_node->argument.identifier);
    return 1;
  }
  return 0;
}

int ni_validate_node_var_assign(ni_ast_node *ast_node) {
  symbol_value *v =
      get_symbol_table(validate_symbol_table, ast_node->variable_assignment.identifier);
  if (v == NULL) {
    log_validation_error(ast_node->location, "variable '%s' not found",
                         ast_node->variable_assignment.identifier);
    return 1;
  }

  if (ni_validate_node(ast_node->variable_assignment.value) != 0) {
    return 1;
  }

  if (ni_check_type_assignment(ast_node->variable_assignment.value,
                               v->node->variable_declaration.type) != 0) {
    char expected_type[TYPE_STR_LEN];
    char got_type[TYPE_STR_LEN];
    type_str(expected_type, v->node->variable_declaration.type);
    type_str(got_type, ast_node->variable_assignment.value->_result_type);
    log_validation_error(ast_node->location,
                         "type mismatch expects '%s' passed '%s'",
                         expected_type, got_type);
    return 1;
  }

  return 0;
}

int ni_validate_node_var_lookup(ni_ast_node *ast_node) {
  symbol_value *v =
      get_symbol_table(validate_symbol_table, ast_node->variable_lookup.identifier);
  if (v == NULL) {
    log_validation_error(ast_node->location, "'%s' not found",
                          ast_node->variable_lookup.identifier);
    return 1;
  }
  switch (v->node->node_type) {
  case NI_AST_NODE_TYPE_CONSTANT_DECLARATION: // replace lookup with value
    free(ast_node->variable_lookup.identifier);
    ast_node->node_type = v->node->constant_declaration.value->node_type;
    ast_node->_result_type = make_ni_ast_type(
        ast_node->location,
        v->node->constant_declaration.value->_result_type->internal_type);
    switch (ast_node->node_type) {
    case NI_AST_NODE_TYPE_CONSTANT_UINT:
    case NI_AST_NODE_TYPE_CONSTANT_INT:
    case NI_AST_NODE_TYPE_CONSTANT_FLOAT:
      ast_node->const_value =
          strdup(v->node->constant_declaration.value->const_value);
      break;
    case NI_AST_NODE_TYPE_CONSTANT_BOOL:
      ast_node->const_bool = v->node->constant_declaration.value->const_bool;
      break;
    default:
      // other types are irrelevant
      break;
    }
    break;
  case NI_AST_NODE_TYPE_VARIABLE_DECLARATION: // copy type
    ast_node->_result_type = make_ni_ast_type(
        ast_node->location, v->node->variable_declaration.type->internal_type);
    break;
  default:
    log_validation_error(ast_node->location, "incompatible element found");
    return 1;
  }
  return 0;
}

int ni_validate_node_bin_op(ni_ast_node *ast_node) {
  int ret_left = ni_validate_node(ast_node->bin_op.left);
  int ret_right = ni_validate_node(ast_node->bin_op.right);
  if (ret_left != 0 || ret_right != 0) {
    return 1;
  }

  switch (ast_node->bin_op.op) {
  case NI_AST_OPERATOR_ADD:      // +
  case NI_AST_OPERATOR_SUBTRACT: // -
  case NI_AST_OPERATOR_MULTIPLY: // *
    ast_node->_result_type = make_ni_ast_type(
        ast_node->location,
        convert_arithmetic_op
            [ast_node->bin_op.left->_result_type->internal_type]
            [ast_node->bin_op.right->_result_type->internal_type]);
  case NI_AST_OPERATOR_GREATER_THAN:           // >
  case NI_AST_OPERATOR_GREATER_THAN_OR_EQUALS: // >=
  case NI_AST_OPERATOR_LESS_THAN:              // <
  case NI_AST_OPERATOR_LESS_THAN_OR_EQUALS:    // <=
    if (convert_num_comparison_op
            [ast_node->bin_op.left->_result_type->internal_type]
            [ast_node->bin_op.right->_result_type->internal_type] == 1) {
      ast_node->_result_type =
          make_ni_ast_type(ast_node->location, NI_AST_INTERNAL_TYPE_UNDEF);
    } else {
      ast_node->_result_type =
          make_ni_ast_type(ast_node->location, NI_AST_INTERNAL_TYPE_BOOL);
    }
    break;
  case NI_AST_OPERATOR_EQUALS_EQUALS: // ==
  case NI_AST_OPERATOR_NOT_EQUALS:    // !=
    if (convert_equals_comparison_op
            [ast_node->bin_op.left->_result_type->internal_type]
            [ast_node->bin_op.right->_result_type->internal_type] == 1) {
      ast_node->_result_type =
          make_ni_ast_type(ast_node->location, NI_AST_INTERNAL_TYPE_UNDEF);
    } else {
      ast_node->_result_type =
          make_ni_ast_type(ast_node->location, NI_AST_INTERNAL_TYPE_BOOL);
    }
    break;
  case NI_AST_OPERATOR_AND: // &&
  case NI_AST_OPERATOR_OR:  // ||
    if (convert_bool_comparison_op
            [ast_node->bin_op.left->_result_type->internal_type]
            [ast_node->bin_op.right->_result_type->internal_type] == 1) {
      ast_node->_result_type =
          make_ni_ast_type(ast_node->location, NI_AST_INTERNAL_TYPE_UNDEF);
    } else {
      ast_node->_result_type =
          make_ni_ast_type(ast_node->location, NI_AST_INTERNAL_TYPE_BOOL);
    }
    break;
  }
  if (ast_node->_result_type->internal_type == NI_AST_INTERNAL_TYPE_UNDEF) {
    char ltype[TYPE_STR_LEN];
    char rtype[TYPE_STR_LEN];
    type_str(ltype, ast_node->bin_op.left->_result_type);
    type_str(rtype, ast_node->bin_op.right->_result_type);

    log_validation_error(ast_node->location,
                         "incompatible types found for operation: '%s', '%s'",
                         ltype, rtype);

    return 1;
  }

  ni_set_type_on_const(ast_node->bin_op.left,
                       ast_node->_result_type->internal_type);
  ni_set_type_on_const(ast_node->bin_op.right,
                       ast_node->_result_type->internal_type);

  return 0;
}

int ni_validate_node_call(ni_ast_node *ast_node) {
  symbol_value *callee_fun =
      get_symbol_table(validate_symbol_table, ast_node->function_call.identifier);

  if (callee_fun == NULL) {
    log_validation_error(ast_node->location, "function '%s' not found",
                         ast_node->function_call.identifier);
    return 1;
  }

  if (callee_fun->node->function_declaration.return_type != NULL) {
    ast_node->_result_type = make_ni_ast_type(
        ast_node->location,
        callee_fun->node->function_declaration.return_type->internal_type);
  }

  if ((ast_node->function_call.argument_list == NULL ||
       ast_node->function_call.argument_list->count == 0) &&
      callee_fun->node->function_declaration.argument_list != NULL &&
      callee_fun->node->function_declaration.argument_list->count > 0) {
    log_validation_error(ast_node->location, "function '%s' expects arguments",
                         ast_node->function_call.identifier);
    return 1;
  }

  if (ast_node->function_call.argument_list == NULL ||
      ast_node->function_call.argument_list->count == 0) {
    return 0;
  }

  if (callee_fun->node->function_declaration.argument_list == NULL) {
    log_validation_error(ast_node->location,
                         "function '%s' expects no arguments",
                         ast_node->function_call.identifier);
    return 1;
  }

  if (ast_node->function_call.argument_list->count !=
      callee_fun->node->function_declaration.argument_list->count) {
    log_validation_error(
        ast_node->location,
        "function '%s' expects %d arguments but %d were passed",
        ast_node->function_call.identifier,
        fun->function_declaration.argument_list->count,
        ast_node->function_call.argument_list->count);
    return 1;
  }

  ni_ast_node_list_entry *arg = ast_node->function_call.argument_list->head;
  ni_ast_node_list_entry *def_arg =
      callee_fun->node->function_declaration.argument_list->head;
  int ret = 0;
  while (arg != NULL) {
    if (ni_validate_node(arg->node) != 0) {
      ret = 1;
    }
    if (ni_check_type_assignment(arg->node, def_arg->node->argument.type) !=
        0) {
      char expected_type[TYPE_STR_LEN];
      char got_type[TYPE_STR_LEN];
      type_str(expected_type, def_arg->node->argument.type);
      type_str(got_type, arg->node->_result_type);
      log_validation_error(arg->node->location,
                           "type mismatch expects '%s' passed '%s'",
                           expected_type, got_type);
      ret = 1;
    }

    arg = arg->next;
    def_arg = def_arg->next;
  }
  return ret;
}

int ni_validate_node_ret(ni_ast_node *ast_node) {
  if (ni_validate_node(ast_node->function_return.value) != 0) {
    return 1;
  }

  if (ast_node->function_return.value == NULL &&
      fun->function_declaration.return_type == NULL) {
    return 0;
  }

  if (ast_node->function_return.value == NULL &&
      fun->function_declaration.return_type != NULL) {
    log_validation_error(ast_node->location, "return value expected");
    return 1;
  }

  if (ast_node->function_return.value != NULL &&
      fun->function_declaration.return_type == NULL) {
    log_validation_error(ast_node->location, "return value not expected");
    return 1;
  }

  if (ni_check_type_assignment(ast_node->function_return.value,
                               fun->function_declaration.return_type) != 0) {
    char expected_type[TYPE_STR_LEN];
    char got_type[TYPE_STR_LEN];
    type_str(expected_type, fun->function_declaration.return_type);
    type_str(got_type, ast_node->function_return.value->_result_type);
    log_validation_error(ast_node->location,
                         "return type mismatch expects '%s' passed '%s'",
                         expected_type, got_type);
    return 1;
  }

  return 0;
}

void type_str(char *out, const ni_ast_type *ast_type) {
  if (out == NULL) {
    return;
  }

  if (ast_type == NULL) {
    sprintf(out, "void");
  } else {
    switch (ast_type->internal_type) {
    case NI_AST_INTERNAL_TYPE_UNDEF:
      sprintf(out, "undefined");
      break;
    case NI_AST_INTERNAL_TYPE_CONST_INT:
      sprintf(out, "c_int");
      break;
    case NI_AST_INTERNAL_TYPE_CONST_UINT:
      sprintf(out, "c_uint");
      break;
    case NI_AST_INTERNAL_TYPE_INT8:
      sprintf(out, "int8");
      break;
    case NI_AST_INTERNAL_TYPE_UINT8:
      sprintf(out, "uint8");
      break;
    case NI_AST_INTERNAL_TYPE_INT16:
      sprintf(out, "int16");
      break;
    case NI_AST_INTERNAL_TYPE_UINT16:
      sprintf(out, "uint16");
      break;
    case NI_AST_INTERNAL_TYPE_INT32:
      sprintf(out, "int32");
      break;
    case NI_AST_INTERNAL_TYPE_UINT32:
      sprintf(out, "uint32");
      break;
    case NI_AST_INTERNAL_TYPE_INT64:
      sprintf(out, "int64");
      break;
    case NI_AST_INTERNAL_TYPE_UINT64:
      sprintf(out, "uint64");
      break;
    case NI_AST_INTERNAL_TYPE_INT128:
      sprintf(out, "int128");
      break;
    case NI_AST_INTERNAL_TYPE_UINT128:
      sprintf(out, "uint128");
      break;
    case NI_AST_INTERNAL_TYPE_CONST_FLOAT:
      sprintf(out, "float");
      break;
    case NI_AST_INTERNAL_TYPE_FLOAT16:
      sprintf(out, "float16");
      break;
    case NI_AST_INTERNAL_TYPE_FLOAT32:
      sprintf(out, "float32");
      break;
    case NI_AST_INTERNAL_TYPE_FLOAT64:
      sprintf(out, "float64");
      break;
    case NI_AST_INTERNAL_TYPE_INT:
      sprintf(out, "int");
      break;
    case NI_AST_INTERNAL_TYPE_UINT:
      sprintf(out, "uint");
      break;
    case NI_AST_INTERNAL_TYPE_BOOL:
      sprintf(out, "bool");
      break;
    }
  }
}

int ni_check_type_assignment(ni_ast_node *ast_node,
                             const ni_ast_type *ast_type) {

  if (convert_assign[ast_node->_result_type->internal_type]
                    [ast_type->internal_type] != 0) {
    return 1;
  }
  ni_set_type_on_const(ast_node, ast_type->internal_type);

  return 0;
}

void ni_set_type_on_const(ni_ast_node *ast_node,
                          ni_ast_internal_type internal_type) {
  if (ast_node->_result_type->internal_type == NI_AST_INTERNAL_TYPE_CONST_INT ||
      ast_node->_result_type->internal_type ==
          NI_AST_INTERNAL_TYPE_CONST_UINT ||
      ast_node->_result_type->internal_type ==
          NI_AST_INTERNAL_TYPE_CONST_FLOAT) {
    ast_node->_result_type =
        make_ni_ast_type(ast_node->location, internal_type);
  }
}
