#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "expand.h"

void expand_variables(pipeline_t *pipeline)
{
    for(int i = 0; i < pipeline->command_count; i++)
    {
        command_t *cmd = &pipeline->commands[i];

        for(int j = 0; j < cmd->argc; j++)
        {
            char *arg = cmd->argv[j];
            if(arg == NULL)
                continue;

            if(arg[0] == '$')
            {
                char *name = arg + 1;
                char *value = getenv(name);

                if(value != NULL)
                    cmd->argv[j] = value;
                else
                    cmd->argv[j] = "";
            }
        }
    }
}
