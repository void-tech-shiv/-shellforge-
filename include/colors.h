#ifndef COLORS_H
#define COLORS_H

/* ANSI terminal colors */
#define C_RESET   "\033[0m"
#define C_BOLD    "\033[1m"
#define C_DIM     "\033[2m"

#define C_RED     "\033[31m"
#define C_GREEN   "\033[32m"
#define C_YELLOW  "\033[33m"
#define C_BLUE    "\033[34m"
#define C_MAGENTA "\033[35m"
#define C_CYAN    "\033[36m"
#define C_WHITE   "\033[37m"

#define C_BRED     "\033[91m"
#define C_BGREEN   "\033[92m"
#define C_BYELLOW  "\033[93m"
#define C_BBLUE    "\033[94m"
#define C_BMAGENTA "\033[95m"
#define C_BCYAN    "\033[96m"
#define C_BWHITE   "\033[97m"

/* Readline needs non-printing escape sequences wrapped in \001 / \002. */
#define RL_GREEN   "\001\033[92m\002"
#define RL_BLUE    "\001\033[94m\002"
#define RL_CYAN    "\001\033[96m\002"
#define RL_MAGENTA "\001\033[95m\002"
#define RL_WHITE   "\001\033[97m\002"
#define RL_RESET   "\001\033[0m\002"
#define RL_BOLD    "\001\033[1m\002"

#endif
