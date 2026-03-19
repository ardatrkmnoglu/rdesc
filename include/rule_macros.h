/**
 * @file rule_macros.h
 * @brief DSL macros for defining grammar production rules.
 *
 * Provides macros to define production rules in a more readable way.
 *
 * @section prefix_sec Name Mapping for Identifiers
 *
 * `POSTFIX_NT_REST`, `PREFIX_TK`, and `PREFIX_NT` macros determine how a raw
 * token name passed to `TK(...)`/`NT(...)` is expanded into a C identifier
 * (typically an enum member).
 *
 * - Default: Identity mapping. `PREFIX_TK(ID)` expands to `TK_ID`,
 *   `POSTFIX_NT(ID)` to `NT_ID`, and `POSTFIX_NT_REST(ID)` to
 *   `ID_REST`.
 * - Override: Define this macro before including `rule_macros.h` to add
 *   namespaces (e.g., `#define PREFIX_TK(x) MY_TK_ ## x`).
 */

#ifndef RDESC_RULE_MACROS_H
#define RDESC_RULE_MACROS_H


#ifndef PREFIX_TK
/** @brief See @ref prefix_sec  */
#define PREFIX_TK(tk) TK_ ## tk
#endif

#ifndef PREFIX_NT
/** @brief See @ref prefix_sec  */
#define PREFIX_NT(nt) NT_ ## nt
#endif

#ifndef POSTFIX_NT_REST
/** @brief See @ref prefix_sec  */
#define POSTFIX_NT_REST(nt) nt ## _REST
#endif

/** @brief Sentinel for end of alternative. */
#define EOA -1
/** @brief Sentinel for end of production body. */
#define EOP -2

/** @cond */
/** sentinel struct for the end of an alternative */
#define SEOA { .ty = RDESC_SENTINEL, .id = EOA }
/** sentinel struct for the end of a production body (alternative list) */
#define SEOP { { .ty = RDESC_SENTINEL, .id = EOP } }
/** @endcond */

/** @brief Macro to create a terminal (token) production symbol. */
#define TK(tk) { .ty = RDESC_TOKEN, .id = PREFIX_TK(tk) }
/** @brief Macro to create a nonterminal production symbol. */
#define NT(nt) { .ty = RDESC_NONTERMINAL, .id = PREFIX_NT(nt) }

/**
 * @brief Epsilon production (empty/null production). Use to represent an
 * empty alternative that matches nothing. This is equivalent to ε in BNF
 * notation.
 */
#define EPSILON SEOA


/**
 * @brief Macro to define a grammar rule. Adds end of alternative and
 * end of production body sentinels to grammar rules.
 */
#define r(...) { { __VA_ARGS__, SEOA }, SEOP }

/**
 * @brief Macro to define an optional grammar rule (epsilon production).
 *
 * This is a shortcut for a rule with two alternative: an alternative with the
 * symbols provided and an empty (epsilon) one.
 *
 * `ropt(A, α)` is equivalent to:
 * ```c
 * A → α / ε
 * ```
 *
 * For example, `ropt(STMT, STMTS)` defines:
 * ```txt
 * <stmts> ::= <stmt> <stmts>
 *           / E
 * ```
 */
#define ropt(...) \
	r(__VA_ARGS__ alt EPSILON)

/**
 * @brief Defines right-recursive list rules. `base` and `suffix` parameters
 * should wrapped with parenthesis (`()`).
 *
 * Defines two nonterminals: the list head and its rest continuation.
 * `rrr(A, (β), (α))` is equivalent to:
 * ```c
 * A  → β A'
 * A' → α A' / ε
 * ```
 *
 * For example, `rrr(EXPR_LS, (NT(EXPR)), (TK(COMMA), NT(EXPR)))` defines:
 * ```txt
 * 1. <expr_ls> ::= <expr> <expr_ls_rest>
 * 2. <expr_ls_rest> ::= "," <expr> <expr_ls_rest>
 *                     / E
 * ```
 *
 * @warning `rrr(X, ...)` expands into *two* production rules defining
 *          nonterminals `NT_X` and `NT_X_REST`. You must explicitly define
 *          `NT_X_REST` right after `NT_X` in your nonterminal enum definition.
 *          See @ref prefix_sec.
 *
 * @param head The base nonterminal.
 * @param base The initial production sequence (beta).
 * @param suffix The repeating production sequence (alpha).
 */
#define rrr(head, base, suffix) \
	r(_rdesc_priv_trim_paren base, NT(POSTFIX_NT_REST(head))), \
	ropt(_rdesc_priv_trim_paren suffix, NT(POSTFIX_NT_REST(head)))

/**
 * @brief Separates grammar alternatives (alternative separator).
 *
 * Expands to end-of-body sentinel and new alternative initialization.
 *
 * Use between alternatives. For example, `r(α alt β alt γ)` is equivalent to
 * `α / β / γ`, where `/` is ordered choice operator.
 */
#define alt , SEOA, }, {

/** @cond */
#define _rdesc_priv_trim_paren(...) __VA_ARGS__
/** @endcond */


#endif
