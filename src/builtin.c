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

static int builtin_cd(command_t *cmd)
{
    const char *directory;

    if (cmd->argc == 1)
    {
        directory = getenv("HOME");
        if (directory == NULL)
        {
            fprintf(stderr, "cd: HOME not set\n");
            return -1;
        }
    }
    else if (cmd->argc == 2)
    {
        directory = cmd->argv[1];
    }
    else
    {
        fprintf(stderr, "cd: too many arguments\n");
        return -1;
    }

    if (chdir(directory) != 0)
    {
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
        fprintf(stderr, "pwd: too many arguments\n");
        return -1;
    }

    if (getcwd(current_directory, sizeof(current_directory)) == NULL)
    {
        perror("pwd");
        return -1;
    }

    printf("%s\n", current_directory);
    return 0;
}

static int builtin_echo(command_t *cmd)
{
    for (int i = 1; i < cmd->argc; i++)
    {
        printf("%s", cmd->argv[i]);
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
        fprintf(stderr, "exit: too many arguments\n");
        return -1;
    }
    return 1;
}

static int builtin_clear(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, "clear: too many arguments\n");
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
        fprintf(stderr, "whoami: too many arguments\n");
        return -1;
    }

    struct passwd *pw = getpwuid(geteuid());
    if (pw != NULL)
        printf("%s\n", pw->pw_name);
    else
        printf("%d\n", (int)geteuid());

    return 0;
}

static int builtin_date(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, "date: too many arguments\n");
        return -1;
    }

    time_t now = time(NULL);
    struct tm tm;

    if (localtime_r(&now, &tm) == NULL)
    {
        perror("date");
        return -1;
    }

    char buf[64];
    if (strftime(buf, sizeof(buf), "%a %b %d %H:%M:%S %Z %Y", &tm) == 0)
    {
        fprintf(stderr, "date: strftime failed\n");
        return -1;
    }

    printf("%s\n", buf);
    return 0;
}

static int builtin_help(command_t *cmd)
{
    if (cmd->argc > 1)
    {
        fprintf(stderr, "help: too many arguments\n");
        return -1;
    }

    printf("\n");
    printf("=========================================================\n");
    printf("              ShellForge Command Help\n");
    printf("=========================================================\n");
    printf("\n");
    printf("ShellForge friendly commands:\n");
    printf("  list [path]        - list files and folders (ls)\n");
    printf("  go <dir>           - change directory (cd)\n");
    printf("  where              - show current directory (pwd)\n");
    printf("  say <text...>      - print text (echo)\n");
    printf("  newfile <name>     - create an empty file (touch)\n");
    printf("  show <file>        - display file contents (cat)\n");
    printf("  copy <src> <dst>   - copy files/directories (cp)\n");
    printf("  move <src> <dst>   - move/rename a file (mv)\n");
    printf("  delete <path>      - remove a file (rm)\n");
    printf("  makedir <name>     - create directory (mkdir)\n");
    printf("  removedir <name>   - remove empty directory (rmdir)\n");
    printf("  findtext <p> <f>   - search text (grep)\n");
    printf("  processes          - show running processes (ps)\n");
    printf("  clean              - clear the terminal (clear)\n");
    printf("  me                 - show current user (whoami)\n");
    printf("  today              - show current date/time (date)\n");
    printf("  past               - show ShellForge command history\n");
    printf("  tasks              - list background/stopped jobs (jobs)\n");
    printf("  front <id>         - bring a job to foreground (fg)\n");
    printf("  back <id>          - resume a job in background (bg)\n");
    printf("  manual             - show this help page (help)\n");
    printf("  quit               - exit ShellForge (exit)\n");
    printf("\n");
    printf("Original Unix command names are also supported.\n");
    printf("\n");
    printf("Shell features:\n");
    printf("  < > >>            - input/output redirection\n");
    printf("  |                 - pipelines\n");
    printf("  &                 - run command in background\n");
    printf("  $VAR              - environment variable expansion\n");
    printf("  '...'  \"...\"      - quoted strings\n");
    printf("  Up / Down         - browse command history\n");
    printf("\n");
    printf("=========================================================\n");
    printf("\n");

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
