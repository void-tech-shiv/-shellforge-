#ifndef ALIASES_H
#define ALIASES_H

#include "parser.h"

/*
 * Convert ShellForge-friendly command names to their
 * Unix command equivalents before execution.
 *
 * Example: list -> ls, show -> cat, copy -> cp
 */
void apply_command_aliases(pipeline_t *pipeline);

#endif
