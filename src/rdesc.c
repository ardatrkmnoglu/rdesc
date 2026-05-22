// SPDX-FileCopyrightText: 2025-2026 Metehan Selvi <me@metehanselvi.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

#include "../include/cst_macros.h"
#include "../include/grammar.h"
#include "../include/rdesc.h"
#include "../include/rule_macros.h"
#include "../include/stack.h"
#include "common.h"
#include "test_instruments.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


/* Additional space for child pointers in nonterminal. */
#define rchild_list_cap(p, nt_id) \
	((p).grammar->child_caps[nt_id] * sizeof(size_t) + sizeof_node(p) - 1) \
		/ sizeof_node(p)

/* Returns the previous node's unwind size (used to navigate backwards). */
#define runwind_size(node) _rdesc_priv_node_deref(node).unwind_size

#define stringify2(s) #s

#define stringify(s) stringify2(s)


/* Constructs nonterminal. Returns non-zero and rolls back to previous valid
 * state if construction fails. */
static int new_nt_node(struct rdesc *p, uint16_t nt_id);
/* Constructs token and returns 0 if the construction succeeded. */
static int new_tk_node(struct rdesc *p, uint16_t tk_id, const void *seminfo);

/* Destroys all tokens in CST and token stacks.
 *
 * Does not call destructor methods for tokens in CST if the parse was
 * complete.
 */
static void destroy_tokens(struct rdesc *p);

/* Adds children to parent's child list using indexes. This function does not
 * fail even if realloc changed the stack pointer. */
static inline void push_child(struct rdesc *p,
			      size_t parent_idx,
			      size_t child_idx);

/* Similar to push child, this does not fail. */
static inline void pop_child(struct rdesc *p,
			     size_t node_idx);

const char *rdesc_version(void)
{
#ifdef RDESC_VERSION_PRE_RELEASE
	return stringify(RDESC_VERSION_MAJOR) "."
		stringify(RDESC_VERSION_MINOR) "."
		stringify(RDESC_VERSION_PATCH) "-"
		RDESC_VERSION_PRE_RELEASE;
#else
	return stringify(RDESC_VERSION_MAJOR) "."
		stringify(RDESC_VERSION_MINOR) "."
		stringify(RDESC_VERSION_PATCH);
#endif
}

int rdesc_init(struct rdesc *p,
	       const struct rdesc_grammar *grammar,
	       size_t seminfo_size,
	       void (*token_destroyer)(uint16_t, void *))
{
	p->grammar = grammar;
	p->seminfo_size = seminfo_size;
	p->token_destroyer = token_destroyer;

	p->cur = SIZE_MAX;
	p->has_saved_tk = false;

	if (seminfo_size > 0) {
		p->saved_seminfo = xmalloc(seminfo_size);

		if (p->saved_seminfo == NULL)
			return 1; /* Could not preallocate extra seminfo space. */
	} else {
		p->saved_seminfo = NULL;
	}

	rdesc_stack_init(&p->token_stack, sizeof_tk(*p), 0, NULL);
	if (p->token_stack == NULL) {
		if (p->saved_seminfo != NULL)
			free(p->saved_seminfo);

		return 1;  /* Could not initialize token stack.  */
	}

	rdesc_stack_init(&p->cst_stack, sizeof_node(*p), 0, NULL);
	if (p->cst_stack == NULL) {
		if (p->saved_seminfo != NULL)
			free(p->saved_seminfo);
		rdesc_stack_destroy(p->token_stack);

		return 1;  /* Could not intialize CST stack. */
	}

	return 0;
}

void rdesc_destroy(struct rdesc *p)
{
	destroy_tokens(p);

	rdesc_stack_destroy(p->token_stack);

	rdesc_stack_destroy(p->cst_stack);

	if (p->saved_seminfo != NULL)
		free(p->saved_seminfo);
}

int rdesc_start(struct rdesc *p, uint16_t start_symbol)
{
	runtime_assertion(p->cur == SIZE_MAX,
			  "cannot start during parse");

	runtime_assertion(p->has_saved_tk == false,
			  "cannot start during error recovery");

	p->top_unwind = 0;

	rdesc_stack_reset(&p->cst_stack);

	if (new_nt_node(p, start_symbol))
		return 1;  /* Start symbol creation failed. */

	return 0;
}

void rdesc_reset(struct rdesc *p)
{
	destroy_tokens(p);

	p->cur = SIZE_MAX;

	rdesc_stack_reset(&p->token_stack);

	rdesc_stack_reset(&p->cst_stack);
}

static void destroy_tokens(struct rdesc *p)
{
	if (!p->token_destroyer)
		return;

	if (p->has_saved_tk) {
		p->token_destroyer(p->saved_tk, p->saved_seminfo);

		p->has_saved_tk = false;
	}

	/* Destroy tokens in backtrack stack */
	for (size_t i = 0; i < rdesc_stack_len(p->token_stack); i++) {
		tk_t *tk = rdesc_stack_at(p->token_stack, i);
		p->token_destroyer(tk->id, &tk->seminfo);
	}

	if (p->cur != SIZE_MAX  /* do not destroy tokens in CST stack after a successful match */
	    && rdesc_stack_len(p->cst_stack)) {
		/* Walk CST backwards to destroy all embedded tokens */
		uint16_t top_unwind = p->top_unwind;

		for (size_t top_idx = rdesc_stack_len(p->cst_stack) - top_unwind;
		     top_idx > 0;  /* Termination: Cannot be a token */
		     top_idx -= top_unwind) {
			struct rdesc_node top =
				_rdesc_priv_cst_illegal_access(p, top_idx);

			if (rtype(top) == RDESC_TOKEN)
				p->token_destroyer(rid(top), rseminfo(top));

			top_unwind = runwind_size(top);
		}
	}
}

/* - THE PUMP -------------------------------------------------------------- */
#define current_alternative(node) \
	productions(*p->grammar)[rid(node)][ralt_idx(node)]
#define next_alternative(node) \
	productions(*p->grammar)[rid(node)][ralt_idx(node) + 1]

#define next_symbol(node) \
	current_alternative(node)[rchild_count(node)]

#define is_alternative_complete(node) \
	(next_symbol(node).id == EOA && \
	 next_symbol(node).ty == RDESC_SENTINEL)

#define does_production_have_unchecked_alternative(node) \
	!(next_alternative(node)[0].id == EOP && \
	  next_alternative(node)[0].ty == RDESC_SENTINEL)

/* Backtraces to the last nonterminal that is not completed (decision point),
 * or teardowns the entire CST. */
static inline int backtrack_decision_point(struct rdesc *p)
{
	size_t decision_point_idx = rdesc_stack_len(p->cst_stack) - p->top_unwind;
	bool has_decision_point_to_continue_on = false;
	size_t tokens_pushed = 0;

	/* FIRST traversal: Push tokens indexed after the decision point onto
	 * the backtracking stack. */

	/* Initialization: Start from the top. */
	while (true) {
		struct rdesc_node top =
			_rdesc_priv_cst_illegal_access(p, decision_point_idx);

		/* Maintenance: The slice from decision_point_idx (exclusive)
		 * to the end does not contain any nonterminals that have
		 * unchecked alternatives. */
		if (rtype(top) == RDESC_TOKEN) {
			// TODO: abstraction violation, &top.n->n.tk
			if (rdesc_stack_push(&p->token_stack, &top.n->n.tk) == NULL) {
				/* Could not move token to the token stack.
				 * Keep the existing token in the CST and
				 * report an error. */

				rdesc_stack_multipop(&p->token_stack,
						     tokens_pushed);

				return 1;
			}
			tokens_pushed++;
		} else /* RDESC_NONTERMINAL */ {
			/* Non-predictive recursive descent parser decision
			 * point: Continue on the first nonterminal with
			 * remaining unchecked alternatives. */
			if (does_production_have_unchecked_alternative(top)) {
				has_decision_point_to_continue_on = true;
				break;
			}
		}

		/* No decision point was found if decision_point_idx is zero. */
		if (decision_point_idx == 0)
			break;

		decision_point_idx -= runwind_size(top);
	}

	/* Two loops exists to enable a rollback to a valid state in case of
	 * memory allocation failure. The first loop ensure all the tokens
	 * pushed back to the backtracking stack, the second loop removes
	 * elements from the first stack. */

	/* The traversal now changes the parser state. From this point on,
	 * no memory failure can occur. */
	p->cur = rdesc_stack_len(p->cst_stack) - p->top_unwind;

	while (p->cur > decision_point_idx) {
		struct rdesc_node top =
			_rdesc_priv_cst_illegal_access(p, p->cur);

		/* Remove element from parent's child pointer list. */
		size_t parent_idx =  _rdesc_priv_parent_idx(top);
		if (parent_idx != SIZE_MAX)
			pop_child(p, parent_idx);

		p->cur -= runwind_size(top);
	};

	/* Safety: p->cur changed, so p->top_unwind MUST BE UPDATED.
	 * This is guaranteed: Before return we update p->top_unwind. */
	if (has_decision_point_to_continue_on) {
		struct rdesc_node top =
			_rdesc_priv_cst_illegal_access(p, decision_point_idx);

		ralt_idx(top)++;
		rchild_count(top) = 0;

		p->top_unwind = 1 + rchild_list_cap(*p, rid(top));
	} else {
		p->top_unwind = 0;
	}

	/* Removes nodes after the p->cur, which is the top. If there is no
	 * such decision point to continue on, p->top_unwind is set to zero
	 * and the top is also removed. */
	rdesc_stack_multipop(&p->cst_stack,
			     rdesc_stack_len(p->cst_stack) - (p->cur + p->top_unwind));

	return 0;
}

/* The pumping automaton.
 *
 * - EMEM: Provided token pushed to either CST stack or token stack, but memory
 *   allocation error occurred afterwards.
 *
 * - EMEM_TK_NOT_OWNED: Provided token was not pushed to token stack or CST,
 *   and it still belong to caller.
 *
 * - READY: Parse complete.
 *
 * - CONTINUE: Request the next token.
 *
 * - NOMATCH: Parse failed.
 *
 * - RETRY: Descend into nonterminal, caller should call this function again. */
static inline enum internal_pump_state {
	EMEM,
	EMEM_TK_NOT_OWNED,
	READY,
	CONTINUE,
	NOMATCH,
	RETRY,
} pump(struct rdesc *p, tk_t *tk)
{
	struct rdesc_node n = _rdesc_priv_cst_illegal_access(p, p->cur);

	if (rdesc_stack_len(p->cst_stack) == 0) {
		if (rdesc_stack_push(&p->token_stack, tk) == NULL) {
			/* Token should be stored for next start, but could
			 * not because of push error. */
			return EMEM_TK_NOT_OWNED;
		}

		return NOMATCH;
	}

	struct rdesc_grammar_symbol rule = next_symbol(n);

	switch (rule.ty) {
	case RDESC_TOKEN:
		if (rule.id == tk->id) {
			/* Match! Add the token to nonterminal's children. */
			if (new_tk_node(p, tk->id, &tk->seminfo)) {
				/* Could not add token to the current
				 * nonterminal's children. */
				return EMEM_TK_NOT_OWNED;
			}
		} else {
			/* Push the token back to the token stack and continue
			 * on the next alternative. */
			if (rdesc_stack_push(&p->token_stack, tk) == NULL) {
				/* Could not push token back to backtracking
				 * stack. */
				return EMEM_TK_NOT_OWNED;
			}

			if (backtrack_decision_point(p)) {
				/* Memory error in backtracking. */
				return EMEM;
			}
		}

		/* Climb the tree if to find incomplete nonterminal to continue
		 * parsing on. */
		while (true) {
			n = _rdesc_priv_cst_illegal_access(p, p->cur);
			if (!is_alternative_complete(n))
				break;

			p->cur = _rdesc_priv_parent_idx(n);

			/* Every node, including the root is completed. Return
			 * ready. */
			if (p->cur == SIZE_MAX)
				return READY;
		}

		return CONTINUE;

	case RDESC_NONTERMINAL:
		if (new_nt_node(p, rule.id)) {
			/* An error occured before the token ever used. */
			return EMEM_TK_NOT_OWNED;
		}

		return RETRY;

	default: unreachable(); return 0;  // GCOV_EXCL_LINE
	} // GCOV_EXCL_LINE
}

/* The pumping loop.
 *
 * It continues parsing by repeatedly calling the pump() funciton if there are
 * tokens in token backtracking stack.
 */
static enum rdesc_result pump_loop(struct rdesc *p,
				   uint16_t id,
				   void *seminfo,
				   bool token_provided)
{
	uint8_t tk_[sizeof_tk(*p)];
	tk_t *tk = cast(tk_t *, &tk_);

	if (token_provided) {
		tk->id = id;
		if (seminfo && p->seminfo_size)
			memcpy(&tk->seminfo, seminfo, p->seminfo_size);
	}

	while (true) {
		if (!token_provided) {
			if (rdesc_stack_len(p->token_stack) > 0)
				tk = rdesc_stack_pop(&p->token_stack);
			else
				return RDESC_CONTINUE;
		} else {
			token_provided = false;
		}

		enum internal_pump_state state;
		do {
			state = pump(p, tk);
		} while (state == RETRY);

		switch (state) {
		case EMEM:
			return RDESC_ENOMEM;

		case EMEM_TK_NOT_OWNED:
			p->saved_tk = tk->id;

			if (p->seminfo_size)
				memcpy(p->saved_seminfo,
				       &tk->seminfo,
				       p->seminfo_size);

			p->has_saved_tk = true;

			return RDESC_ENOMEM;

		case CONTINUE:
			break;

		case NOMATCH:
			p->cur = SIZE_MAX;

			return RDESC_NOMATCH;

		case READY:
			return RDESC_READY;

		default: unreachable();  // GCOVR_EXCL_LINE
		}
	}
}

enum rdesc_result rdesc_pump(struct rdesc *p, uint16_t id, void *seminfo)
{
	runtime_assertion(p->cur != SIZE_MAX, "parser is not started");

	runtime_assertion(p->has_saved_tk == false,
			  "shall not provide new token during OOM Recovery");

	runtime_assertion(rdesc_stack_len(p->token_stack) == 0,
			  "cannot pump new token if token backtracking "
			  "stack is not empty (dirty Running)");

	return pump_loop(p, id, seminfo, true);
}

enum rdesc_result rdesc_resume(struct rdesc *p)
{
	runtime_assertion(p->cur != SIZE_MAX, "parser is not started");

	if (p->has_saved_tk) {
		p->has_saved_tk = false;

		return pump_loop(p, p->saved_tk, p->saved_seminfo, true);
	} else {
		return pump_loop(p, 0, NULL, false);
	}

}
/* ------------------------------------------------------------------------- */

struct rdesc_node rdesc_get_root(struct rdesc *p)
{
	runtime_assertion(rdesc_has_cst(p), "no tree is initialized");

	return _rdesc_priv_cst_illegal_access(p, 0);
}

bool rdesc_has_cst(const struct rdesc *p)
{
	return rdesc_stack_len(p->cst_stack) != 0;
}

struct _rdesc_priv_node *_rdesc_priv_cst_illegal_access2(const struct rdesc *p,
						   size_t index)
{
	return index == SIZE_MAX ?
		NULL : rdesc_stack_at(p->cst_stack, index);
}

/* Makes the connection between parent and child, by adding `child_index` to
 * parent's children index list. */
static inline void push_child(struct rdesc *p, size_t parent_idx, size_t child_idx)
{
	struct rdesc_node parent = _rdesc_priv_cst_illegal_access(p, parent_idx);

	_rdesc_priv_child_idx(parent, rchild_count(parent)) = child_idx;

	rchild_count(parent)++;
}

/* Removes the last child from node. */
static inline void pop_child(struct rdesc *p, size_t node_idx)
{
	struct rdesc_node parent = _rdesc_priv_cst_illegal_access(p, node_idx);

	rchild_count(parent)--;
}

/* Pushes a new nonterminal to parser's CST stack and reserves space for its
 * children. */
static int new_nt_node(struct rdesc *p, uint16_t nt_id)
{
	/* allocate node pointer */
	struct rdesc_node n = {
		.p = p,
		.n = rdesc_stack_push(&p->cst_stack, NULL)
	};

	if (n.n == NULL)
		return 1;  /* node allocation failed */

	/* the new node will be the p->cur, so that we need to hold parent_idx
	 * in order to add it to its parent */
	size_t parent_idx = p->cur;
	p->cur = rdesc_stack_len(p->cst_stack) - 1;  /* index of the new node */

	_rdesc_priv_parent_idx(n) = parent_idx;
	runwind_size(n) = p->top_unwind;
	rtype(n) = RDESC_NONTERMINAL;

	rid(n) = nt_id;
	ralt_idx(n) = 0;
	rchild_count(n) = 0;

	uint16_t child_list_cap = rchild_list_cap(*p, nt_id);
	if (rdesc_stack_multipush(&p->cst_stack, NULL, child_list_cap) == NULL) {
		/* Rollback changes if nonterminal is partially constructed. */

		rdesc_stack_pop(&p->cst_stack);  /* Pop the node. */
		p->cur = parent_idx;  /* Rollback parent. */

		return 1;  /* child list allocation failed */
	} else {
		p->top_unwind = 1 + child_list_cap;

		if (parent_idx != SIZE_MAX)
			push_child(p, parent_idx, p->cur);

		return 0;
	}


}

/* Creates a new node in parser's CST stack and copies `seminfo` into it. */
static int new_tk_node(struct rdesc *p, uint16_t tk_id, const void *seminfo)
{
	struct rdesc_node n = {
		.p = p,
		.n = rdesc_stack_push(&p->cst_stack, NULL)
	};

	if (n.n == NULL)
		return 1;  /* node allocation failed */

	size_t node_id = rdesc_stack_len(p->cst_stack) - 1;

	push_child(p, p->cur, node_id);

	_rdesc_priv_parent_idx(n) = p->cur;
	runwind_size(n) = p->top_unwind;
	rtype(n) = RDESC_TOKEN;

	rid(n) = tk_id;

	if (seminfo && p->seminfo_size)
		memcpy(rseminfo(n), seminfo, p->seminfo_size);

	p->top_unwind = 1;

	return 0;
}
