#include "dialog.h"
#include <string.h>
#include <strings.h>
#include <stdio.h>
#include <stdlib.h>

int validate_onoff(DialogRec *d)
{
    return !strcmp(d->param, "on") || !strcmp(d->param, "off");
}

int validate_noparam(DialogRec *d)
{
    return !strcmp(d->param, "");
}

DialogRec *findDialogRec(char *command, DialogRec dialogspec[])
{
    int index = 0;
    DialogRec *res = NULL;

    while (strcmp(dialogspec[index].command, ""))
    {
        if (!strncasecmp(dialogspec[index].command, command, strlen(dialogspec[index].command)))
        {
            res = &dialogspec[index];
            return res;
        }
        ++index;
    }
    return res;
}

ProlResult processLine(char line[LINEMAX], int state, DialogRec dialogspec[])
{
    ProlResult *res = malloc(sizeof(ProlResult));
    DialogRec *dlr = findDialogRec(line, dialogspec);
    char *linep = line;

    if (!dlr)
    {
        res->failed = 1;
        res->dialogrec = dlr;
        strcpy(res->info, "Sorry hat nicht geklappt.(COMMAND_NOT_FOUND_ERRROR)");
    }
    else
    {
        if (state != dlr->state)
        {
            res->failed = 1;
            res->dialogrec = dlr;
            strcpy(res->info, "Ich war noch nicht bereit dafür.(STATE_ERROR)");
        }
        else
        {
            strcpy(dlr->param, linep + strlen(dlr->command));
            if (dlr->validator)
            {
                if (dlr->validator(dlr))
                {
                    res->failed = 0;
                    res->dialogrec = dlr;
                    dlr->is_valid = 1;
                    strcpy(res->info, "Is alles in butter(SUCCESS)");
                }
                else
                {
                    res->failed = 1;
                    res->dialogrec = dlr;
                    dlr->is_valid = 0;
                    strcpy(res->info, "Wasn das jetzt fürn Käse?!(NON_VALID_PARAM)");
                }
            }
            else
            {
                if (dlr->validator(dlr))
                {
                    res->failed = 0;
                    res->dialogrec = dlr;
                    strcpy(res->info, "Is alles in butter(SUCCESS)");
                }
            }
        }
    }
    return *res;
}

/**
int main(int argv, char **argc){
    char line[100] =  "begin";
    ProlResult prol;
    DialogRec dialog[] = {
    {"BEGIN", "",0,1,validate_noparam},
    {"toaster", "", 1,1,validate_onoff},
    {"END","",1,2,validate_noparam},
    {""}
    };
    
    printf("%s\n",findDialogRec("begin tomato", dialog)->command);
    prol = processLine(line,0,dialog);
    printf("%s",prol.info);
    printf("Hallo Du!");
    return 0;
}
 */
