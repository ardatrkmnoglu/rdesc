#include "../include/cst_macros.h"
#include "../include/rdesc.h"
#include "../include/util.h"

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


void rdesc_flip_left(struct rdesc *p,
		     struct rdesc_node *parent,
		     uint16_t child_index)
{
	struct rdesc_node *initial_root = rchild(p, parent, child_index);
	uint16_t nt_id = rid(initial_root);

	/* Store the parent index. The new rotated subtree will be
	 * rewired back to this node. */
	size_t subtree_parent_idx = _rdesc_priv_parent_idx(initial_root);

	/* The original root becomes the leaf (beta) that terminates
	 * the new left-recursive spine. */
	rvariant(initial_root) = 1;

	size_t prev_idx = _rdesc_priv_child_idx(parent, child_index);
	struct rdesc_node *prev = initial_root;

	size_t this_idx = _rdesc_priv_child_idx(prev, (rchild_count(prev) - 1));
	struct rdesc_node *this = _rdesc_priv_cst_illegal_access(p, this_idx);

	/* Loop over the right-recursive spine.
	 *
	 * Initialization: 'prev' is the initial root, and 'this' is its last
	 * child (the recursive nonterminal). */
	while (rvariant(this) != 1) {
		size_t hold_rest_idx =
			_rdesc_priv_child_idx(this, rchild_count(this) - 1);

		rid(this) = nt_id;

		/* Shift alpha children to the right to make index 0 the
		 * recursive slot. */
		for (uint16_t i = rchild_count(this) - 1; i > 0; i--)
			_rdesc_priv_child_idx(this, i) =
				_rdesc_priv_child_idx(this, i - 1);

		_rdesc_priv_child_idx(this, 0) = prev_idx;
		_rdesc_priv_parent_idx(prev) = this_idx;

		prev = this;
		prev_idx = this_idx;

		this = _rdesc_priv_cst_illegal_access(p, hold_rest_idx);
		this_idx = hold_rest_idx;

		/* Termination: If 'this' is an epsilon node (variant == 1).
		 * The epsilon node is orphaned from the CST and will be
		 * reclaimed automatically by rdesc. */
	}

	_rdesc_priv_parent_idx(prev) = subtree_parent_idx;
	_rdesc_priv_child_idx(parent, child_index) = prev_idx;

	/* Disconnect initial root that became the terminating leaf of
	 * recursion, from its children by decrementing child count. */
	rchild_count(initial_root)--;
}
