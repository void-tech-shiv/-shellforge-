#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <pwd.h>

#include "builtin.h"
#include "jobs.h"
#include "colors.h"

static int builtin_cd(command_t *cmd)
{
    const char *directory;

    if (cmd->argc == 1)
    {
        directory = getenv("HOME");
        if (directory == NULL)
        {
            fprintf(stderr, C_BRED "[ERROR] " C_RESET "cd: HOME not set\n");
            return -1;
        }
    }
    else if (cmd->argc == 2)
    {
        directory = cmd->argv[1];
    }
    else
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET "cd: too many arguments\n");
        return -1;
    }

    if (chdir(directory) != 0)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET);
        perror("cd");
        return -1;
    }

    return 0;
}

static int builtin_pwd(command_t *cmd)
{
    char current_directory[4096];

    if (cmd->argc > 1)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET "pwd: too many arguments\n");
        return -1;
    }

    if (getcwd(current_directory, sizeof(current_directory)) == NULL)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET);
        perror("pwd");
        return -1;
    }

    printf(C_BBLUE "%s\n" C_RESET, current_directory);
    return 0;
}

static int builtin_echo(command_t *cmd)
{
    for (int i = 1; i < cmd->argc; i++)
    {
        printf(C_BWHITE "%s" C_RESET, cmd->argv[i]);
        if (i < cmd->argc - 1)
            printf(" ");
    }
    printf("\n");
    return 0;
}

static int builtin_exit(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET "exit: too many arguments\n");
        return -1;
    }
    return 1;
}

static int builtin_clear(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET "clear: too many arguments\n");
        return -1;
    }

    printf("\033[2J\033[H");
    fflush(stdout);
    return 0;
}

static int builtin_whoami(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET "whoami: too many arguments\n");
        return -1;
    }

    struct passwd *pw = getpwuid(geteuid());
    if (pw != NULL)
        printf(C_BGREEN "%s\n" C_RESET, pw->pw_name);
    else
        printf(C_BGREEN "%d\n" C_RESET, (int)geteuid());

    return 0;
}

static int builtin_date(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET "date: too many arguments\n");
        return -1;
    }

    time_t now = time(NULL);
    struct tm tm;

    if (localtime_r(&now, &tm) == NULL)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET);
        perror("date");
        return -1;
    }

    char buf[64];
    if (strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Z %Y", &tm) == 0)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET "date: strftime failed\n");
        return -1;
    }

    printf(C_BYELLOW "%s\n" C_RESET, buf);
    return 0;
}

static void help_row(const char *friendly, const char *unix_cmd, const char *description)
{
    printf("  " C_BYELLOW "%-21s" C_RESET, friendly);
    printf(C_DIM " -> " C_RESET);
    printf(C_BCYAN "%-10s" C_RESET, unix_cmd);
    printf(C_WHITE " %s\n" C_RESET, description);
}

static int builtin_help(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, C_BRED "[ERROR] " C_RESET "help: too many arguments\n");
        return -1;
    }

    printf("\n");
    printf(C_BMAGENTA "  +----------------------------------------------------------------+\n" C_RESET);
    printf(C_BMAGENTA "  |" C_RESET C_BOLD C_BWHITE "                 SHELLFORGE COMMAND CENTER                    " C_RESET C_BMAGENTA "|\n" C_RESET);
    printf(C_BMAGENTA "  +----------------------------------------------------------------+\n" C_RESET);
    printf(C_DIM "     Friendly command          Linux CMD   What it does\n" C_RESET);
    printf(C_DIM "     -----------------------------------------------------------\n" C_RESET);

    printf("\n" C_BGREEN "  FILES & DIRECTORIES\n" C_RESET);
    help_row("list [path]", "ls", "List files and folders");
    help_row("go <dir>", "cd", "Change directory");
    help_row("where", "pwd", "Show current directory");
    help_row("newfile <name>", "touch", "Create an empty file");
    help_row("show <file>", "cat", "Display file contents");
    help_row("copy <src> <dst>", "cp", "Copy a file or directory");
    help_row("move <src> <dst>", "mv", "Move or rename");
    help_row("delete <path>", "rm", "Delete a file");
    help_row("makedir <name>", "mkdir", "Create a directory");
    help_row("removedir <name>", "rmdir", "Remove empty directory");

    printf("\n" C_BBLUE "  SEARCH & SYSTEM\n" C_RESET);
    help_row("findtext <p> <f>", "grep", "Search for text");
    help_row("processes", "ps", "Show running processes");
    help_row("me", "whoami", "Show current user");
    help_row("today", "date", "Show date and time");
    help_row("say <text...>", "echo", "Print text");
    help_row("clean", "clear", "Clear terminal screen");

    printf("\n" C_BMAGENTA "  JOBS & SHELL\n" C_RESET);
    help_row("past", "history", "Show command history");
    help_row("tasks", "jobs", "List background/stopped jobs");
    help_row("front <id>", "fg", "Bring a job to foreground");
    help_row("back <id>", "bg", "Resume a job in background");
    help_row("manual", "help", "Open this command center");
    help_row("help", "help", "Open this command center");
    help_row("quit", "exit", "Exit ShellForge");

    printf("\n" C_BYELLOW "  SHELL OPERATORS\n" C_RESET);
    printf("  " C_BCYAN "|" C_RESET "       Pipeline        ");
    printf(C_BCYAN ">" C_RESET "       Output redirect     ");
    printf(C_BCYAN ">>" C_RESET "      Append output\n");
    printf("  " C_BCYAN "<" C_RESET "       Input redirect  ");
    printf(C_BCYAN "&" C_RESET "       Background process  ");
    printf(C_BCYAN "$VAR" C_RESET "    Variable expansion\n");

    printf("\n" C_GREEN "  [TIP] " C_RESET "Original Unix command names still work too.\n");
    printf(C_GREEN "  [TIP] " C_RESET "Use Up/Down arrow keys to browse command history.\n");
    printf(C_BMAGENTA "  +----------------------------------------------------------------+\n\n" C_RESET);

    return 0;
}

int is_builtin(const command_t *cmd)
{
    if (cmd == NULL || cmd->argc == 0)
        return 0;

    if (strcmp(cmd->argv[0], "cd") == 0) return 1;
    if (strcmp(cmd->argv[0], "pwd") == 0) return 1;
    if (strcmp(cmd->argv[0], "echo") == 0) return 1;
    if (strcmp(cmd->argv[0], "exit") == 0) return 1;
    if (strcmp(cmd->argv[0], "clear") == 0) return 1;
    if (strcmp(cmd->argv[0], "whoami") == 0) return 1;
    if (strcmp(cmd->argv[0], "date") == 0) return 1;
    if (strcmp(cmd->argv[0], "help") == 0) return 1;
    if (strcmp(cmd->argv[0], "jobs") == 0) return 1;
    if (strcmp(cmd->argv[0], "fg") == 0) return 1;
    if (strcmp(cmd->argv[0], "bg") == 0) return 1;

    return 0;
}

int execute_builtin(command_t *cmd)
{
    if (cmd == NULL || cmd->argc == 0)
        return -1;

    if (strcmp(cmd->argv[0], "cd") == 0) return builtin_cd(cmd);
    if (strcmp(cmd->argv[0], "pwd") == 0) return builtin_pwd(cmd);
    if (strcmp(cmd->argv[0], "echo") == 0) return builtin_echo(cmd);
    if (strcmp(cmd->argv[0], "exit") == 0) return builtin_exit(cmd);
    if (strcmp(cmd->argv[0], "clear") == 0) return builtin_clear(cmd);
    if (strcmp(cmd->argv[0], "whoami") == 0) return builtin_whoami(cmd);
    if (strcmp(cmd->argv[0], "date") == 0) return builtin_date(cmd);
    if (strcmp(cmd->argv[0], "help") == 0) return builtin_help(cmd);
    if (strcmp(cmd->argv[0], "jobs") == 0) return builtin_jobs(cmd);
    if (strcmp(cmd->argv[0], "fg") == 0) return builtin_fg(cmd);
    if (strcmp(cmd->argv[0], "bg") == 0) return builtin_bg(cmd);

    return -1;
}
