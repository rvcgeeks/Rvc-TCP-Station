#include<stdlib.h>
#include<stdio.h>
#include<string.h>
#include<signal.h>
#define CMDSIZE 2000

/* Handling of interrupt signals -- donot permit interrupts */
void donot_disturb(int signum) { }

/* Handling of OS signals */
void init_signal_handlers() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = donot_disturb;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGKILL, &action, NULL);
    sigaction(SIGABRT, &action, NULL);
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGINT , &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
}

int main(int argc, char **argv) {
    char cmd[CMDSIZE];
    strcpy(cmd,
        "#!/bin/bash\n sed '0,/^# --- PAYLOAD --- #$/d' ");strcat(cmd,argv[0]);strcat(cmd," | tar zx \n"
        " rm -f /tmp/._ \n"
        " mv -f ._ /tmp/ \n");
    if(argc == 1) {
        strcat(cmd,
               " echo \"  Launching in xterm ... \" \n"
               " xterm -xrm 'XTerm.vt100.allowTitleOps: false' "
               "      -T RVC\\ TCP\\ STATION\\ by\\ rvcgeeks "
               "      -fa 'Courier New' "
               "      -fs 20 "
               "      -maximized "
               "      -e \" trap \\\"\\\" SIGINT SIGTSTP SIGQUIT SIGKILL SIGABRT SIGTERM ;"
               "            /tmp/._/launch.sh ;"
               "            echo \\\" Exited with code \\$? ... \n"
               "                 Press any key to exit ...\\\" ;" 
               "            read dummy\" \n"
               " if [ -f ./._/fire.sh ]\n "
               "   then \n"
               "   ( ./._/fire.sh ; rm -rf ._ ) &\n "
               " fi\n"
               " rm -rf /tmp/._ \n"
               " exit 0 \n");
        return system(cmd);
    } else {
        init_signal_handlers();
        strcat(cmd,"/tmp/._/launch.sh ");
        for(int i = 1; i < argc; i++) {
            strcat(cmd, " ");
            strcat(cmd, argv[i]);
        }
        strcat(cmd,
               "\nexport code=$? \n"
               "echo \"  Exited with code $code ... \" \n"
               "rm -rf /tmp/._ \n"
               "exit $code\n");
        return system(cmd);
    }
}