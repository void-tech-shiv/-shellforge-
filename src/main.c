#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <readline/history.h>
#include <readline/readline.h>

#include "history.h"
#include "token.h"
#include "lexer.h"
#include "parser.h"
#include "expand.h"
#include "builtin.h"
#include "executor.h"
#include "jobs.h"
#include "aliases.h"
#include "colors.h"

static void print_banner(void)
{
    printf("\n");
    printf(C_BCYAN "  +--------------------------------------------------+\n" C_RESET);
    printf(C_BCYAN "  |" C_RESET C_BOLD C_BMAGENTA "              S H E L L F O R G E               " C_RESET C_BCYAN "|\n" C_RESET);
    printf(C_BCYAN "  |" C_RESET C_BWHITE "        A colorful Unix-style shell in C          " C_RESET C_BCYAN "|\n" C_RESET);
    printf(C_BCYAN "  +--------------------------------------------------+\n" C_RESET);
    printf(C_GREEN "    [OK] " C_RESET "Pipelines     " C_GREEN "[OK] " C_RESET "Redirection\n");
    printf(C_GREEN "    [OK] " C_RESET "Job Control   " C_GREEN "[OK] " C_RESET "Command History\n");
    printf(C_GREEN "    [OK] " C_RESET "Friendly CMD  " C_GREEN "[OK] " C_RESET "Variable Expansion\n");
    printf("\n");
    printf(C_DIM "    Type " C_RESET C_BYELLOW "help" C_RESET C_DIM " to view all ShellForge commands.\n" C_RESET);
    printf(C_DIM "    Example: " C_RESET C_BGREEN "list" C_RESET C_DIM " instead of " C_RESET C_CYAN "ls" C_RESET C_DIM ", " C_RESET
           C_BGREEN "go" C_RESET C_DIM " instead of " C_RESET C_CYAN "cd" C_RESET "\n\n");
}

static void build_prompt(char *prompt, size_t size)
{
    char cwd[1024];
    const char *user = getenv("USER");

    if (user == NULL || user[0] == '\0')
        user = "user";

    if (getcwd(cwd, sizeof(cwd)) == NULL)
        strcpy(cwd, "?");

    snprintf(prompt, size,
             RL_BOLD RL_GREEN "%s" RL_RESET
             RL_WHITE "@" RL_RESET
             RL_BOLD RL_CYAN "shellforge" RL_RESET
             RL_WHITE ":" RL_RESET
             RL_BLUE "%s" RL_RESET
             RL_MAGENTA " > " RL_RESET,
             user, cwd);
}

int main(void)
{
    print_banner();

    /* The shell runs in its own process group so that
       ^C / ^Z sent at the prompt stay on the shell. */
    job_set_shell_pgid(getpgrp());

    setup_background_handler();
    jobs_init();

    using_history();

    token_list_t tokens;
    pipeline_t pipeline;
    char *line;
    char prompt[1400];

    while (1)
    {
        /* Clean up finished children before showing a prompt
           so they don't sit in the jobs table forever. */
        reap_jobs();

        build_prompt(prompt, sizeof(prompt));
        line = readline(prompt);

        if (line == NULL)
        {
            printf("\n" C_BMAGENTA "Goodbye from ShellForge!" C_RESET "\n");
            break;
        }

        if (strlen(line) == 0)
        {
            free(line);
            continue;
        }

        if (strcmp(line, "history") == 0 || strcmp(line, "past") == 0)
        {
            print_history();
            free(line);
            continue;
        }

        add_history(line);

        lexer(line, &tokens);

        if (!parser(&tokens, &pipeline))
        {
            free(line);
            continue;
        }

        expand_variables(&pipeline);
        apply_command_aliases(&pipeline);

        /* 'exit' handled as builtin in the executor */
        if (pipeline.command_count == 1 &&
            pipeline.commands[0].argc > 0 &&
            strcmp(pipeline.commands[0].argv[0], "exit") == 0)
        {
            printf(C_BMAGENTA "Goodbye from ShellForge!" C_RESET "\n");
            free(line);
            break;
        }

        execute_pipeline(&pipeline);

        free(line);
    }

    return 0;
}
