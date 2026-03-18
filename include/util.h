/**
 * @file util.h
 * @brief Development and debugging utilities.
 *
 * This header provides tools for visualizing data structures in librdesc.
 */

#ifndef RDESC_UTIL_H
#define RDESC_UTIL_H

#include <stdint.h>
#include <stdio.h>

struct rdesc; /* defined in rdesc.h */
struct rdesc_node;

struct rdesc_grammar;  /* defined in grammar.h */


#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Dumps the concrete syntax tree (CST) as a graphviz DOT graph.
 *
 * @param out Output file stream.
 * @param parser Parser to dump its root.
 * @param node_printer Callback to print token and nonterminal names.
 */
void rdesc_dump_cst(FILE *out,
		    const struct rdesc *parser,
		    void (*node_printer)(FILE *out, const struct rdesc_node *));

/**
 * @brief Dumps the grammar in BNF format.
 *
 * Prints all production rules in human-readable BNF format. (e.g.`A ::= B / C`)
 *
 * @param out Output file stream.
 * @param grammar Underlying grammar struct.
 * @param tk_names The token name or literal representation (e.g., `IDENT` or
 *        `+`). Prefix with '@' to suppress automatic quote wrapping.
 * @param nt_names The raw name of the nonterminal (e.g., `expr`). Angle
 *        brackets `<>` are added automatically.
 */
void rdesc_dump_bnf(FILE *out,
		    const struct rdesc_grammar *grammar,
		    const char *const tk_names[],
                    const char *const nt_names[]);

/**
 * @brief Rotates a right-recursive concrete syntax tree into a left-recursive
 * form.
 *
 * Performs an in-place transformation of a subtree to convert
 * right-associative nodes into left-associative ones. This is typically used
 * as a post-processing step for grammars transformed by the `rrr`
 * (right-recursive-rewrite) macro.
 *
 * The function converts the structure represented by:
 * ```c
 * A  → β A'
 * A' → α A' / ε
 * ```
 * into the left-recursive equivalent:
 * ```c
 * A  → A α
 *    / β
 * ```
 * α and β may consist of multiple tokens or nonterminal nodes. CST connections
 * are maintained to ensure valid destruction and traversal.
 *
 * The node to be rotated cannot be the root of the entire parse tree, so
 * `parent` cannot be `NULL`. This is because a match is considered complete if
 * and only if all nonterminals complete their bodies; this condition cannot be
 * met if the start symbol (the root) is a recursive nonterminal.
 *
 * @param parser Pointer to the rdesc parser instance.
 * @param parent The parent node of the subtree root being rotated.
 * @param child_index The index of the target node (A) within the parent's
 *        child list.
 */
void rdesc_flip_left(struct rdesc *parser,
		     struct rdesc_node *parent,
		     uint16_t child_index);

#ifdef __cplusplus
}
#endif


#endif
