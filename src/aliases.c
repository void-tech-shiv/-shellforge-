#include <string.h>
#include "aliases.h"

typedef struct
{
    const char *friendly;
    const char *unix_name;
} command_alias_t;

static const command_alias_t aliases[] = {
    {"go",       "cd"},
    {"where",    "pwd"},
    {"say",      "echo"},
    {"clean",    "clear"},
    {"me",       "whoami"},
    {"today",    "date"},
    {"manual",   "help"},
    {"quit",     "exit"},
    {"tasks",    "jobs"},
    {"front",    "fg"},
    {"back",     "bg"},
    {"list",     "ls"},
    {"newfile",  "touch"},
    {"show",     "cat"},
    {"copy",     "cp"},
    {"move",     "mv"},
    {"delete",   "rm"},
    {"makedir",  "mkdir"},
    {"removedir","rmdir"},
    {"findtext", "grep"},
    {"processes","ps"}
};

void apply_command_aliases(pipeline_t *pipeline)
{
    if (pipeline == NULL)
        return;

    const int alias_count = (int)(sizeof(aliases) / sizeof(aliases[0]));

    for (int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];
        if (cmd->argc == 0 || cmd->argv[0] == NULL)
            continue;

        for (int j = 0; j < alias_count; j++)
        {
            if (strcmp(cmd->argv[0], aliases[j].friendly) == 0)
            {
                cmd->argv[0] = (char *)aliases[j].unix_name;
                break;
            }
        }
    }
}
