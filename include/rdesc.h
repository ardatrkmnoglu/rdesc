// SPDX-FileCopyrightText: 2025-2026 Metehan Selvi <me@metehanselvi.com>
//
// SPDX-License-Identifier: LGPL-2.1-or-later

/**
 * @file rdesc.h
 * @brief The deterministic recursive descent parser.
 */

#ifndef RDESC_H
#define RDESC_H

#include "detail.h"

#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

/** @brief Major version */
#define RDESC_VERSION_MAJOR 0
/** @brief Minor version */
#define RDESC_VERSION_MINOR 3
/** @brief Patch version */
#define RDESC_VERSION_PATCH 0
/** @brief Prerelease identifier */
#define RDESC_VERSION_PRE_RELEASE "preview"


/** @brief Parse operation result codes. */
enum rdesc_result {
	/** Memory allocation failed. */
	RDESC_ENOMEM = -1,
	/** A CST is ready for consumption. */
	RDESC_READY = 0,
	/** New tokens should be provided. */
	RDESC_CONTINUE = 1,
	/** No grammar rule matches the provided tokens. */
	RDESC_NOMATCH = 2,
};

/** @brief Recursive descent parser state. */
struct rdesc;

/** @brief Opaque CST (Concrete Syntax Tree) node. */
struct rdesc_node;


#ifdef __cplusplus
extern "C" {
#endif

/** @brief rdesc version */
const char *rdesc_version(void);

/**
 * @brief Initializes a new parser.
 *
 * @param parser Parser instance to initialize.
 * @param grammar Grammar defining production rules (must outlive parser).
 * @param seminfo_size Size in bytes of token semantic information.
 * @param token_destroyer Optional callback to free token seminfo (can be NULL).
 *
 * @return Non-zero value if memory allocation fails.
 */
int rdesc_init(struct rdesc *parser,
	       const struct rdesc_grammar *grammar,
	       size_t seminfo_size,
	       void (*token_destroyer)(uint16_t id, void *seminfo)) _rdesc_wur;

/**
 * @brief Frees memory allocated by the parser and destroys the parser instance.
 */
void rdesc_destroy(struct rdesc *parser);

/**
 * @brief Sets start symbol for the next match.
 *
 * @return Non-zero value if memory allocation fails.
 */
int rdesc_start(struct rdesc *parser, uint16_t start_symbol) _rdesc_wur;

/**
 * @brief Resets the parser to its initial state.
 */
void rdesc_reset(struct rdesc *parser);

/**
 * @brief Drives the parsing process, the pump.
 *
 * As the central engine of the parser, it consumes tokens from either the
 * internal backtracking stack or the provided id.
 *
 * @param parser Pointer to the parser instance.
 * @param id Identifier of the next token to consume.
 * @param seminfo Extra semantic information for the token.
 *        - Semantic information pointer. The parser copies this data
 *          internally, so passing a pointer to stack-allocated data is valid.
 *          NULL is acceptable.
 *
 * @return The current status of the parse operation.
 *
 * @warning Raises an error if the parser is not started.
 */
enum rdesc_result rdesc_pump(struct rdesc *parser,
			     uint16_t id,
			     void *seminfo) _rdesc_wur;

/**
 * @brief Resume parsing without providing a new token.
 *
 * Resumes using either:
 * - The saved token from a previous ENOMEM error, or
 * - A token from the backtrack stack
 */
 enum rdesc_result rdesc_resume(struct rdesc *parser) _rdesc_wur;

/**
 * @brief Returns the root of the CST.
 *
 * @warning Raises an error if no CST has been created yet.
 */
struct rdesc_node rdesc_get_root(struct rdesc *parser);

/** @brief Returns true if a CST is present in the parser. */
bool rdesc_has_cst(const struct rdesc *parser);

#ifdef __cplusplus
}
#endif


#endif
