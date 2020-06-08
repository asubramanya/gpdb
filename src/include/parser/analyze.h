/*-------------------------------------------------------------------------
 *
 * analyze.h
 *		parse analysis for optimizable statements
 *
 *
 * Portions Copyright (c) 1996-2016, PostgreSQL Global Development Group
 * Portions Copyright (c) 1994, Regents of the University of California
 *
 * src/include/parser/analyze.h
 *
 *-------------------------------------------------------------------------
 */
#ifndef ANALYZE_H
#define ANALYZE_H

#include "parser/parse_node.h"

/* Hook for plugins to get control at end of parse analysis */
typedef void (*post_parse_analyze_hook_type) (ParseState *pstate,
														  Query *query);
extern PGDLLIMPORT post_parse_analyze_hook_type post_parse_analyze_hook;


extern Query *parse_analyze(Node *parseTree, const char *sourceText,
			  Oid *paramTypes, int numParams);
extern Query *parse_analyze_varparams(Node *parseTree, const char *sourceText,
						Oid **paramTypes, int *numParams);

extern Query *parse_sub_analyze(Node *parseTree, ParseState *parentParseState,
				  CommonTableExpr *parentCTE,
				  LockingClause *lockclause_from_parent);

extern Query *transformTopLevelStmt(ParseState *pstate, Node *parseTree);
extern Query *transformStmt(ParseState *pstate, Node *parseTree);

extern bool analyze_requires_snapshot(Node *parseTree);

extern const char *LCS_asString(LockClauseStrength strength);
extern void CheckSelectLocking(Query *qry, LockClauseStrength strength);
extern void applyLockingClause(Query *qry, Index rtindex,
				   LockClauseStrength strength,
				   LockWaitPolicy waitPolicy, bool pushedDown);


/* Context for transformGroupedWindows() which mutates components
 * of a query that mixes windowing and aggregation or grouping.  It
 * accumulates context for eventual construction of a subquery (the
 * grouping query) during mutation of components of the outer query
 * (the windowing query).
 */
typedef struct
{
	List *subtlist; /* target list for subquery */
	List *subgroupClause; /* group clause for subquery */
	List *subgroupingSets; /* grouping sets for subquery */
	List *windowClause; /* window clause for outer query*/

	/* Scratch area for init_grouped_window context and map_sgr_mutator.
	 */
	Index  *sgr_map;
	int		sgr_map_size;

	/* Scratch area for grouped_window_mutator and var_for_gw_expr.
	 */
	List *subrtable;
	int call_depth;
	TargetEntry *tle;
} grouped_window_ctx;

extern Query *transformGroupedWindows(ParseState *pstate, Query *qry);
extern void init_grouped_window_context(grouped_window_ctx *ctx, Query *qry);
extern Var *var_for_gw_expr(grouped_window_ctx *ctx, Node *expr, bool force);
extern void discard_grouped_window_context(grouped_window_ctx *ctx);
extern Node *map_sgr_mutator(Node *node, void *context);
extern Node *grouped_window_mutator(Node *node, void *context);
extern Alias *make_replacement_alias(Query *qry, const char *aname);
extern char *generate_positional_name(AttrNumber attrno);
extern List*generate_alternate_vars(Var *var, grouped_window_ctx *ctx);

/* State shared by transformCreateStmt and its subroutines */
typedef struct
{
	ParseState *pstate;			/* overall parser state */
	const char *stmtType;		/* "CREATE [FOREIGN] TABLE" or "ALTER TABLE" */
	RangeVar   *relation;		/* relation to create */
	Relation	rel;			/* opened/locked rel, if ALTER */
	List	   *inhRelations;	/* relations to inherit from */
	bool		hasoids;		/* does relation have an OID column? */
	bool		isforeign;		/* true if CREATE/ALTER FOREIGN TABLE */
	bool		isalter;		/* true if altering existing table */
	bool		iscreatepart;	/* true if create in service of creating a part */
	bool		issplitpart;
	List	   *columns;		/* ColumnDef items */
	List	   *ckconstraints;	/* CHECK constraints */
	List	   *fkconstraints;	/* FOREIGN KEY constraints */
	List	   *ixconstraints;	/* index-creating constraints */
	List	   *inh_indexes;	/* cloned indexes from INCLUDING INDEXES */
	List	   *attr_encodings; /* List of ColumnReferenceStorageDirectives */
	List	   *blist;			/* "before list" of things to do before
								 * creating the table */
	List	   *alist;			/* "after list" of things to do after creating
								 * the table */
	List	   *dlist;			/* "deferred list" of utility statements to 
								 * transfer to the list CreateStmt->deferredStmts
								 * for later parse_analyze and dispatch */
	IndexStmt  *pkey;			/* PRIMARY KEY index, if any */

	MemoryContext tempCtx;
} CreateStmtContext;

extern int validate_partition_spec(CreateStmtContext *cxt, 
							CreateStmt 			*stmt, 
							PartitionBy 		*partitionBy, 	
							char	   			*at_depth,
							int					 partNumber);

extern bool is_aocs(List *opts);

List *transformStorageEncodingClause(List *options);
List *TypeNameGetStorageDirective(TypeName *typname);
extern List * form_default_storage_directive(List *enc);

#endif   /* ANALYZE_H */
