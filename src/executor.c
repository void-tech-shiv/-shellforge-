#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>

#include "parser.h"
#include "executor.h"
#include "builtin.h"
#include "jobs.h"

static void update_job_status(int status, pid_t pid);
static pid_t foreground_pgid = 0;

void job_set_foreground_pgid(pid_t p) { foreground_pgid = p; }
pid_t job_get_foreground_pgid(void) { return foreground_pgid; }

static void sigchld_handler(int sig)
{
    int saved_errno = errno;
    int status;
    pid_t pid;
    (void)sig;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0)
        update_job_status(status, pid);

    errno = saved_errno;
}

static void sigtstp_handler(int sig)
{
    int saved_errno = errno;
    (void)sig;
    if (foreground_pgid != 0)
        kill(-foreground_pgid, SIGTSTP);
    errno = saved_errno;
}

static void update_job_status(int status, pid_t pid)
{
    if (WIFSTOPPED(status))
        job_stop(pid);
    else if (WIFEXITED(status) || WIFSIGNALED(status))
        job_done(pid);
}

void setup_background_handler(void)
{
    struct sigaction sa;
    struct sigaction sa_stp;

    memset(&sa, 0, sizeof(sa));
    memset(&sa_stp, 0, sizeof(sa_stp));

    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) < 0)
        perror("sigaction SIGCHLD");

    sa_stp.sa_handler = sigtstp_handler;
    sigemptyset(&sa_stp.sa_mask);
    sa_stp.sa_flags = SA_RESTART;
    if (sigaction(SIGTSTP, &sa_stp, NULL) < 0)
        perror("sigaction SIGTSTP");
}

void reap_background_processes(void)
{
}

static int apply_redirection(command_t *cmd)
{
    int fd;

    if (cmd->input[0] != '\0')
    {
        fd = open(cmd->input, O_RDONLY);
        if (fd < 0) { perror(cmd->input); return -1; }
        if (dup2(fd, STDIN_FILENO) < 0)
        {
            perror("dup2 input");
            close(fd);
            return -1;
        }
        close(fd);
    }

    if (cmd->output[0] != '\0')
    {
        if (cmd->append)
            fd = open(cmd->output, O_WRONLY | O_CREAT | O_APPEND, 0644);
        else
            fd = open(cmd->output, O_WRONLY | O_CREAT | O_TRUNC, 0644);

        if (fd < 0) { perror(cmd->output); return -1; }
        if (dup2(fd, STDOUT_FILENO) < 0)
        {
            perror("dup2 output");
            close(fd);
            return -1;
        }
        close(fd);
    }

    return 0;
}

static void build_command_string(const pipeline_t *pipeline,
                                 int background,
                                 char *out,
                                 size_t out_size)
{
    out[0] = '\0';

    for (int i = 0; i < pipeline->command_count; i++)
    {
        const command_t *cmd = &pipeline->commands[i];

        if (i > 0)
            strncat(out, " | ", out_size - strlen(out) - 1);

        for (int j = 0; j < cmd->argc; j++)
        {
            strncat(out, cmd->argv[j], out_size - strlen(out) - 1);
            if (j < cmd->argc - 1)
                strncat(out, " ", out_size - strlen(out) - 1);
        }
    }

    if (background)
        strncat(out, " &", out_size - strlen(out) - 1);
}

static void create_process_group(pid_t *pids, int count)
{
    for (int i = 0; i < count; i++)
    {
        if (i == 0)
            setpgid(pids[i], pids[i]);
        else
            setpgid(pids[i], pids[0]);
    }
}

int execute_command(command_t *cmd)
{
    pid_t pid;
    int status;

    if (cmd == NULL) return -1;
    if (cmd->argc == 0) return 0;

    if (is_builtin(cmd) && !cmd->background)
        return execute_builtin(cmd);

    pid = fork();
    if (pid < 0)
    {
        perror("fork");
        return -1;
    }

    if (pid == 0)
    {
        char *args[MAX_ARGS + 1];
        setpgid(0, 0);

        if (apply_redirection(cmd) < 0)
            _exit(EXIT_FAILURE);

        for (int i = 0; i < cmd->argc; i++)
            args[i] = cmd->argv[i];
        args[cmd->argc] = NULL;

        if (is_builtin(cmd))
        {
            int result = execute_builtin(cmd);
            _exit(result);
        }

        execvp(args[0], args);
        perror(args[0]);
        _exit(127);
    }

    setpgid(pid, pid);

    if (cmd->background)
    {
        char command_string[MAX_JOB_COMMAND];
        snprintf(command_string, sizeof(command_string), "%s", cmd->argv[0]);

        int job_id = job_add(pid, command_string, JOB_RUNNING);
        if (job_id > 0)
            printf("[%d] %d\n", job_id, pid);
        return 0;
    }

    job_set_foreground_pgid(pid);

    sigset_t block_mask, old_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block_mask, &old_mask);

    if (waitpid(pid, &status, 0) < 0)
    {
        if (errno != ECHILD)
            perror("waitpid");
        sigprocmask(SIG_SETMASK, &old_mask, NULL);
        job_set_foreground_pgid(0);
        return -1;
    }

    sigprocmask(SIG_SETMASK, &old_mask, NULL);
    job_set_foreground_pgid(0);

    if (WIFEXITED(status))
        return WEXITSTATUS(status);
    return -1;
}

int execute_pipeline(pipeline_t *pipeline)
{
    if (pipeline == NULL)
        return -1;

    int command_count = pipeline->command_count;
    if (command_count <= 0)
        return 0;

    if (command_count == 1)
        return execute_command(&pipeline->commands[0]);

    int previous_read = -1;
    pid_t pids[MAX_COMMANDS];
    int status;
    int final_status = 0;
    int background = pipeline->commands[command_count - 1].background;

    for (int i = 0; i < command_count; i++)
    {
        int pipefd[2];

        if (i < command_count - 1 && pipe(pipefd) < 0)
        {
            perror("pipe");
            return -1;
        }

        pids[i] = fork();
        if (pids[i] < 0)
        {
            perror("fork");
            return -1;
        }

        if (pids[i] == 0)
        {
            command_t *cmd = &pipeline->commands[i];
            char *args[MAX_ARGS + 1];

            if (i == 0)
                setpgid(0, 0);
            else
                setpgid(0, pids[0]);

            if (previous_read != -1)
            {
                if (dup2(previous_read, STDIN_FILENO) < 0)
                {
                    perror("dup2 previous pipe");
                    _exit(EXIT_FAILURE);
                }
            }

            if (i < command_count - 1)
            {
                if (dup2(pipefd[1], STDOUT_FILENO) < 0)
                {
                    perror("dup2 next pipe");
                    _exit(EXIT_FAILURE);
                }
            }

            if (previous_read != -1)
                close(previous_read);

            if (i < command_count - 1)
            {
                close(pipefd[0]);
                close(pipefd[1]);
            }

            if (apply_redirection(cmd) < 0)
                _exit(EXIT_FAILURE);

            for (int j = 0; j < cmd->argc; j++)
                args[j] = cmd->argv[j];
            args[cmd->argc] = NULL;

            if (is_builtin(cmd))
            {
                int result = execute_builtin(cmd);
                _exit(result);
            }

            execvp(args[0], args);
            perror(args[0]);
            _exit(127);
        }

        if (previous_read != -1)
            close(previous_read);

        if (i < command_count - 1)
        {
            close(pipefd[1]);
            previous_read = pipefd[0];
        }
        else
        {
            previous_read = -1;
        }
    }

    create_process_group(pids, command_count);

    if (background)
    {
        char command_string[MAX_JOB_COMMAND];
        build_command_string(pipeline, 1, command_string, sizeof(command_string));

        int job_id = job_add(pids[0], command_string, JOB_RUNNING);
        if (job_id > 0)
            printf("[%d] %d\n", job_id, pids[0]);
        return 0;
    }

    job_set_foreground_pgid(pids[0]);

    sigset_t block_mask, old_mask;
    sigemptyset(&block_mask);
    sigaddset(&block_mask, SIGCHLD);
    sigprocmask(SIG_BLOCK, &block_mask, &old_mask);

    for (int i = 0; i < command_count; i++)
    {
        status = 0;
        if (waitpid(pids[i], &status, 0) < 0)
        {
            if (errno != ECHILD)
                perror("waitpid");
            continue;
        }

        if (i == command_count - 1)
        {
            if (WIFEXITED(status))
                final_status = WEXITSTATUS(status);
            else
                final_status = -1;
        }
    }

    sigprocmask(SIG_SETMASK, &old_mask, NULL);
    job_set_foreground_pgid(0);
    return final_status;
}
