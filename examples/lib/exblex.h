/**
 * @file exblex.h
 * @brief Basic lexer, used for testing.
 */

#ifndef EXBLEX_H
#define EXBLEX_H

#include "../../src/common.h"

#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>


/**
 * @brief EXtremely Basic LEXer
 *
 * A lightweight lexer designed for internal tests. It ignores whitespace and
 * tokenizes the input based on three categories:
 *
 * 1. Words (`w`): Matches identifiers consisting of alphanumeric characters
 *    and underscores (`[a-zA-Z0-9_]`).
 * 2. Digits (`d`): Matches integer literals consisting of numeric digits
 *    (`[0-9]`).
 * 3. One-Character Punctuation: Matches single-character symbols provided in
 *    the lookup table (e.g., `+`, `*`, `;`).
 *
 * @note The lexer gives precedence to character classes (`w`, `d`). If a
 *       character does not fit into these classes, it is checked against the
 *       `tokens` table.
 *
 * @note Identifier 0 is reserved in exblex.
 */
struct exblex {
	/** @brief Underlying null-terminated input buffer. */
	const char *buf;

	/** @brief Array of token characters. */
	const char *tokens;

	/** @brief Number of tokens in provided array. */
	size_t token_count;

	/* --- Fields from now on should be zero-intialized. --- */

	/** @brief (current) Position in the buffer. */
	size_t cur /* (current) Index lexing continues on. */;

	/** @cond */
	char pushback_char /* A character that was part of the input stream but
			    * not consumed by the previous token (e.g., the `+`
			    * in `abc+def`). */;

	char *current_seminfo /* Holds semantic information pointer of the last
			       * token and returns it in
			       * `exblex_current_seminfo`. */;
	/** @endcond */
};

/** @cond */
static inline uint16_t _exblex_priv_tokenid(const struct exblex *l, char tk)
{
	for (uint16_t i = 1; i <= l->token_count; i++)
		if (l->tokens[i] == tk)
			return i;

	return 0;
}
/** @endcond */

/**
 * @brief Initializes the basic lexer with null-terminated list of chars.
 *
 * @warning Identifier 0 is reserved in exblex. First element of tokens (index
 *          0) should be null-character, as it is reserved.
 *
 * @see `struct exblex` for details.
 */
static inline void exblex_init(struct exblex *l,
			       const char *buf,
			       const char *tokens)
{
	rdesc_assert(tokens[0] == '\0', "first element of tokens (tokens[0]) "
		     "should be '\\0'");

	*l = (struct exblex) {
		.buf = buf,
		.tokens = tokens,
		.token_count = strlen(tokens + 1),
		.cur = 0, .current_seminfo = NULL,
		.pushback_char = '\0',
	};
}

/**
 * @brief Retrieves the semantic information for the last token.
 *
 * @return Pointer to token text (for 'w' and 'd' tokens), or NULL for
 *         punctuation tokens.
 *
 * @note Caller takes ownership of returned pointer, and must `free()` after
 *       use.
 */
static inline char *exblex_current_seminfo(struct exblex *l)
{
	return l->current_seminfo;
}

/**
 * @brief Fetches the next token.
 *
 * @return Token ID:
 *         - 0 for EOF/end of input
 *         - Index into tokens[] array for the matched character or class
 *
 * @note For 'w' (word) and 'd' (digit) tokens, retrieve the matched text
 *       using exblex_current_seminfo.
 */
static inline uint16_t exblex_next(struct exblex *l)
{
	while (l->buf[l->cur] && isspace(l->buf[l->cur]))
		l->cur++;

	char c = l->buf[l->cur++];
	char *seminfo = NULL;
	size_t seminfo_len = 0;

	bool is_num = true;
	while (isalnum(c) || c == '_') {
		if (seminfo == NULL) {
			rdesc_assert(seminfo = malloc(sizeof(char) * 2),
				     "malloc failed");
			seminfo_len++;
		} else {
			rdesc_assert(seminfo = realloc(seminfo, sizeof(char) * (++seminfo_len + 1)),
				     "realloc failed");
		}

		if (!isdigit(c))
			is_num = false;

		seminfo[seminfo_len - 1] = c;

		c = l->buf[l->cur++];
	}

	if (seminfo) {
		seminfo[seminfo_len] = '\0';

		l->cur--;
		int id = 0;

		if (is_num)
			id = _exblex_priv_tokenid(l, 'd');
		else if (!isdigit(seminfo[0]))
			id = _exblex_priv_tokenid(l, 'w');

		if (id) {
			l->current_seminfo = seminfo;

			return id;
		} else {
			if (seminfo_len > 1)
				c = '\0';
			else
				c = seminfo[0];

			free(seminfo);
		}
	}

	return _exblex_priv_tokenid(l, c);
}


#endif
