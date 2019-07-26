/* ___________________________________________________________
 * ____________<<<___#_R_V_C_G_E_E_K_S___>>>__________________
 * CREATED BY #RVCGEEKS @PUNE for more rvchavadekar@gmail.com
 *
 * #RVCGEEKS TCP SERVER ... only rvcchats-client(s) can connect to it
 * created on 12.05.2019
 * 
*/

#define STYLE "\033[0m\n\033[48;2;100;0;0m\033[1;94m\033[38;2;255;255;255m"

const char banner[] = "\n" STYLE /* This banner is displayed when a client connects */
                      "        ________   _     _    _______                                             " STYLE
                      "       / /   / /  | |   / /  / /                                                  " STYLE
                      "      / /   / /   | |  / /  / /        ███ ███ ███                                " STYLE
                      "     /_/___/_/    | | / /  / /          █  █   █ █       __________________       " STYLE
                      "    / /  \\ \\      | |/ /  / /           █  █   ███      |  S T A T I O N   |      " STYLE
                      "   /_/    \\_\\     |___/  /_/_____       █  ███ █        |______v_1.0_______|      " STYLE
                      "__________________________________________________________________________________" STYLE
                      "█ █ █ █ █ █ █ █ █ █ █ █ █ <  Live Chatting  (TCP/IP) >█ █ █ █ █ █ █ █ █ █ █ █ █ █ " STYLE
                      " █ █ █ █ █ █ █ █ █ █ █ █ █<    Multi User   (MC/MS)  > █ █ █ █ █ █ █ █ █ █ █ █ █ █" STYLE
                      "█ █ █ █ █ █ █ █ █ █ █ █ █ <  File transfer  (FTP)    >█ █ █ █ █ █ █ █ █ █ █ █ █ █ " STYLE
                      "_█_█_█_█_█_█_█_█_█_█_█_█_█<____Remote Shell (RSH)____>_█_█_█_█_█_█_█_█_█_█_█_█_█_█" STYLE
                      "                                                                                  " STYLE
                      "    Welcome to #rvgeeks CLI server ...                                            " STYLE
                      "    Developed by Rajas Chavadekar @Pune                                           " STYLE
                      "    Check out http://www.github.com/rvcgeeks for more cool stuff!!!               " STYLE
                      "    Any queries reverted on rvchavadekar@gmail.com                                " STYLE
                      "                                                                                  " STYLE
                      "              !!!  PLEASE READ THESE INSTRUCTIONS BEFORE USE !!!                  " STYLE
                      "                                                                                  " STYLE
                      "  1) Just type the message and press enter to send your message.                  " STYLE
                      "  2) To upload a file first copy it in your 'uploads folder created by app' and   " STYLE
                      "     then in your chats send command '--upload-- yourfilename' to server to send  " STYLE
                      "     it on group.. everyone in that group will be able to download that file.     " STYLE
                      "  3) To download a file in the group send the command '--download-- filename' to  " STYLE
                      "     server and after successful download it will appear in your                  " STYLE
                      "     'downloads' folder. Please accurately mention the filename.                  " STYLE
                      "  4) To execute a scell command on a user who permits one's shell access , use    " STYLE
                      "     '--shell-- @[username |--all--] --bash-- [actual commandline to execute]'    " STYLE
                      "     in the actual command if you give '--getout--' it will just terminate        " STYLE
                      "     the client connection. If it fails you will get some error message.          " STYLE
                      "  5) The --pull-- command delibrately uploads a file from client side onto the    " STYLE
                      "     server. It will command the client program to upload prescribed file.        " STYLE
                      "     '--pull-- @[username] --file-- [accurate filename]' is its syntax.           " STYLE
                      "  6) Similar is with --push-- command but the opposite action to command the      " STYLE
                      "     client to download the prescribed file from server.                          " STYLE
                      "  7) To view all online users type command '--anyonehere--'. You will get a list  " STYLE
                      "     of all available users with their ip address.                                " STYLE
                      "     '--push-- @[username |--all--] --file-- [accurate filename]' is its syntax.  " STYLE
                      "  8) To exit the room just send ' --exit-- ' to the server so it will             " STYLE
                      "     terminate your connection.                                                   " STYLE
                      "                                                                                  \033[0m\n\n"
                      "Please enter your unique username for other group members to identify:";



#include <cstdio>
#include <iostream>
#include <fstream> // for logfile
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <csignal>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/sendfile.h>
#include <ctime> //for maintaining server time
#include <vector>
using namespace std;

/* Declarations */

/* Color identification for different users */
#define COLS 10
const  string cols[COLS] = {
    "\033[38;2;0;255;0m",
    "\033[38;2;0;0;255m",
    "\033[38;2;255;0;0m",
    "\033[38;2;255;255;0m",
    "\033[38;2;0;255;255m",
    "\033[38;2;255;0;255m",
    "\033[38;2;255;128;0m",
    "\033[38;2;128;255;0m",
    "\033[38;2;0;128;255m",
    "\033[38;2;0;255;128m"
};

#define PACKET_SIZE 2048

struct client_type {
    int id;
    int sockfd;
    int master_sockfd;  /* sockfd of client who has sent this client a push pull or shell request */
    string uname;
    string ip_addr_str;
};

/* Time delay by msleep */
int msleep(unsigned long milisec){
    struct timespec req={0};
    time_t sec=(int)(milisec / 1000);
    milisec -= sec*1000;
    req.tv_sec = sec;
    req.tv_nsec = milisec * 1000000L;
    while(nanosleep(&req,&req) == -1);
    return 1;
}

const int EMPTY_SOCKET = -1,
          SOCKET_ERROR = -1,
          MAX_CLIENTS = 5000;
fstream logfile;
bool FATAL_TERMINATE = false; /* flag to indicate a termination signal is caught */

/* This server is cunstructed for star topology .. this array kept global for signal handlers */
vector<client_type> client_array(MAX_CLIENTS);

/* Show online users */
void show_online_users(int sockfd, vector<client_type> &client_array){
    string reply = "\n\n\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m ONLINE USERS: \033[0m\n\n";
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(client_array[i].uname != "")
            reply += to_string(i + 1) + ") " + client_array[i].ip_addr_str + " > @" + client_array[i].uname + "\n";
    send(sockfd, reply.c_str(), PACKET_SIZE, 0);
}

bool is_client_array_empty() {
    for (int i = 0; i < MAX_CLIENTS; i++)
        if(client_array[i].sockfd != EMPTY_SOCKET)
            return false;
    return true;
}

void shutdown_all_connections(int signum) {
    cout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Signal "
         << signum << " Caught ... Terminating all active connections !!  \033[0m\n";
    logfile << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Signal "
         << signum << " Caught ... Terminating all active connections !!  \033[0m\n";
    char request[PACKET_SIZE] = "--shell-- --getout--";
    for(int i = 0; i < MAX_CLIENTS; i++)
        if(client_array[i].sockfd != EMPTY_SOCKET)
            send(client_array[i].sockfd, request, PACKET_SIZE, 0);
    FATAL_TERMINATE = true;
}

void donot_disturb(int signum) {
    if(is_client_array_empty()) {
        cout<<"\n\n TERMINATING SERVER...\n\n";
        exit(0);
    } else {
        cout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Signal "
             << signum << " Caught ... Cant terminate server, Some users are online !!  \033[0m\n";
        logfile << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Signal "
             << signum << " Caught ... Cant terminate server, Some users are online !!  \033[0m\n";
    }
}

/* Handling of OS signals */
void init_signal_handlers() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    
    /* Fatal signals ... need to close connection and exit */
    action.sa_handler = shutdown_all_connections;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGKILL, &action, NULL);
    sigaction(SIGABRT, &action, NULL);
    
    /* User level termination signals ... Ignore them and continue */
    action.sa_handler = donot_disturb;
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGINT , &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
}

/* Command Validation for separators */
bool validate_separator_tags(char tempmsg[], int sockfd) {
    char errmsg[PACKET_SIZE];
    if(strstr(tempmsg, "@") == NULL) {
        strcpy(errmsg, " no target specified !!\n");
        send(sockfd, errmsg, PACKET_SIZE, 0); 
        return true;
    }
    if((strstr(tempmsg, "--push--") != NULL || 
        strstr(tempmsg, "--pull--") != NULL) &&
        strstr(tempmsg, "--file--") == NULL) {
        strcpy(errmsg, " '--file--' separator needed!!\n");
        send(sockfd, errmsg, PACKET_SIZE, 0); 
        return true;
    }
    if(strstr(tempmsg, "--shell--") != NULL &&
       strstr(tempmsg, "--bash--") == NULL) {
       strcpy(errmsg, " '--bash--' separator needed!!\n");
       send(sockfd, errmsg, PACKET_SIZE, 0); 
       return true;
    }
    return false;
}

/* Function for Client thread */
int process_client(
    client_type &new_client,
    thread &self_thread     /* Passing its own thread so it can detach itself */
) {
    bool is_uname_set = false;
    
    /* Session */
    for (;;) {
        /* Update server time for every loop */
        time_t timer;
        char timestring[PACKET_SIZE], tempmsg[PACKET_SIZE] = "";
        string msg = ""; 
        struct tm *tm_info;
        time(&timer);
        tm_info = localtime(&timer);
        strftime(timestring, PACKET_SIZE, "[ %d/%m/%Y  %H:%M:%S ] >     ", tm_info);
        
        if (new_client.sockfd != 0) {
            
            /* Show the banner to a fresher */
            if(!is_uname_set) {
                msg = string(banner);
                send(new_client.sockfd, msg.c_str(), strlen(msg.c_str()), 0);
            }
            
            uname_exists:
            memset(tempmsg, 0, PACKET_SIZE);
            int iResult = recv(new_client.sockfd, tempmsg, PACKET_SIZE, 0);
            
            /* Check whether it is shell execution request */
            if(strstr(tempmsg,"--shell--") == tempmsg) {
                
                int i = 0; 
                char *cmd = strstr(tempmsg, "--bash-- ") + 9, errmsg[PACKET_SIZE];  
                if(validate_separator_tags(tempmsg, new_client.sockfd))
                    continue;
                /*Select user to send shell command .. if --all-- tag then send shell to all available users */
                for(; i < MAX_CLIENTS; i++)
                    if((strstr(tempmsg + 11, client_array[i].uname.c_str()) == tempmsg + 11 ||
                        strstr(tempmsg + 11, "--all--") == tempmsg + 11 ) &&  /* shells also can be executed in all users including self (if they permit) */
                       client_array[i].uname != "" && client_array[i].id != new_client.id
                    ) { 
                        char finalcmd[PACKET_SIZE] = "--shell-- ";
                        strcat(finalcmd, cmd);
                        send(client_array[i].sockfd, finalcmd, PACKET_SIZE, 0);
                        client_array[i].master_sockfd = new_client.sockfd;
                        cout<<"Shell '"<<(finalcmd + 10)<<"' sent to user @"<<client_array[i].uname<<"\n";
                        logfile<<"Shell '"<<(finalcmd + 10)<<"' sent to user @"<<client_array[i].uname<<"\n"; 
                        if(strstr(tempmsg + 11, "--all--") != tempmsg + 11 )
                            break; 
                    }
        
                if(strstr(tempmsg + 11, "--all--") != tempmsg + 11 && i == MAX_CLIENTS) {
                    strcpy(errmsg, "The user DOSENT EXIST !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0); 
                }
            }
            
            /* Check whether it is request to show all online users */
            if(strstr(tempmsg,"--anyonehere--") == tempmsg)
                show_online_users(new_client.sockfd, client_array);
            
            /* Check whether it is file upload request ... for server, accept packets and build file in ./share/ */
            if(strstr(tempmsg,"--upload--") == tempmsg) {
                
                char filename[PACKET_SIZE] = "share/", buffer[PACKET_SIZE];
                strcat(filename, tempmsg + 11);
                
                /* Open the out file */
                FILE *file_to_recieve = fopen(filename, "wb");
                if(file_to_recieve == NULL) {
                    char errmsg[PACKET_SIZE] = "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m   FATAL ERROR : opening new file '";
                    strcat(errmsg, filename + 6);
                    strcat(errmsg, "' on server   \033[0m");
                    cout << errmsg << endl; 
                    logfile << errmsg <<endl;
                    send(new_client.master_sockfd, errmsg, PACKET_SIZE, 0);
                    new_client.master_sockfd = EMPTY_SOCKET;
                    continue; 
                } 
                
                /* Recieve filemeta and calculate the chunks */
                long size = 0; 
                recv(new_client.sockfd, reinterpret_cast<char*>(&size), 8, 0); 
                if(size < 0) { /* -ve size indicates file not successfully opened at client side */
                    char errmsg[PACKET_SIZE] = "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m   FATAL ERROR : opening file '";
                    strcat(errmsg, filename + 6);
                    strcat(errmsg, "' on client   \033[0m");
                    cout << errmsg << endl; 
                    logfile << errmsg <<endl;
                    send(new_client.master_sockfd, errmsg, PACKET_SIZE, 0);
                    new_client.master_sockfd = EMPTY_SOCKET;
                    fclose(file_to_recieve);
                    continue;
                }
                cout<<"SIZE = "<<size<<endl;
                
                /* recieving packets from client and building file in 'share' directory */
                long remain_data = size, len;
                while ((remain_data > 0) && ((len = recv(new_client.sockfd, buffer, PACKET_SIZE, 0)) > 0)) {
                    fwrite(buffer, 1, len, file_to_recieve);
                    remain_data -= len;
                }
                fclose(file_to_recieve);
                
                /* Broadcast the upload message to the other clients */
                string success = string("\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m") 
                                 + timestring + " @" +new_client.uname
                                 + " has uploaded the file '" + string(filename + 6) + " ("
                                 + to_string(size) + " Bytes)' on server successfully!!\033[0m\n";
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_array[i].sockfd != EMPTY_SOCKET)
                        iResult = send(client_array[i].sockfd, success.c_str(), PACKET_SIZE, 0);
                }
                cout << success << endl; 
                logfile << success <<endl; 
                new_client.master_sockfd = EMPTY_SOCKET;
                continue;
            }
            
            /* Check whether it is a download or push request .. for server send the filemeta tag, its size and fragmented packets from file to client */ 
            if(strstr(tempmsg ,"--download--") == tempmsg || strstr(tempmsg ,"--push--") == tempmsg) {
                
                client_type selected_client = new_client; int i = 0;
                char filename[PACKET_SIZE] = "share/", errmsg[PACKET_SIZE];
                if(strstr(tempmsg , "--push--") == tempmsg ) {
                    if(validate_separator_tags(tempmsg, new_client.sockfd))
                        continue;
                    for(; i < MAX_CLIENTS; i++)
                        if((strstr(tempmsg + 10, client_array[i].uname.c_str()) == tempmsg + 10 ||
                            strstr(tempmsg + 10, "--all--") == tempmsg + 11 ) &&  /* shells also can be executed in all users (if they permit) */
                            client_array[i].uname != "" && i != new_client.id
                        ) { 
                            /* parsing ther valid username for who will recieve push request */
                            strcat(filename, strstr(tempmsg, "--file-- ") + 9);
                            selected_client = client_array[i];
                            strcpy(errmsg, "Server is pushing you a file '");
                            strcat(errmsg, filename); 
                            strcat(errmsg, "' \n");
                            logfile<<errmsg; 
                            send(selected_client.sockfd, errmsg, PACKET_SIZE, 0);
                            break; 
                        }
                } else {
                    selected_client = new_client;
                    strcat(filename, tempmsg + 13);
                }
                if(i == MAX_CLIENTS) {
                    strcpy(errmsg, "The user DOSENT EXIST !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0); 
                    continue;
                }
                
                if(selected_client.master_sockfd != EMPTY_SOCKET && strstr(tempmsg ,"--push--") == tempmsg) {
                    strcpy(errmsg, "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m   The client is owned by some other master... target is busy... !!!  \033[0m\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0);
                    continue;
                } else if(strstr(tempmsg ,"--push--") == tempmsg)
                    client_array[i].master_sockfd = new_client.sockfd;
                
                if(selected_client.uname != new_client.uname){
                    strcpy(errmsg, "push request acknowledged !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0);
                }
                
                /* Open the in file */
                int fd = open(filename, O_RDONLY);
                if (fd == -1) {
                    char errmsg[PACKET_SIZE] = "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m   FATAL ERROR : opening file '";
                    strcat(errmsg, filename);
                    strcat(errmsg, " on server   \033[0m");
                    cout << errmsg << endl; 
                    logfile << errmsg <<endl;
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0);
                    client_array[i].master_sockfd = EMPTY_SOCKET;
                    continue; 
                }
                
                /* Send the filesize so client accepts only required number of packets under download request */
                struct stat file_stat;
                fstat(fd, &file_stat);
                long size = file_stat.st_size;
                char filemeta[PACKET_SIZE] = "--filemeta-- ";
                memcpy(filemeta + 13, reinterpret_cast<char*>(&size), 8);
                strcat(filemeta + 21, filename + 6);
                send(selected_client.sockfd, filemeta, PACKET_SIZE, 0);
                cout<<"SIZE = "<<size<<endl;
                
                /* Reading file packet by packet and sending it to client under download or push request */
                long sent_bytes = 0, remain_data = size, offset = 0;
                while (((sent_bytes = sendfile(selected_client.sockfd, fd, &offset, PACKET_SIZE)) > 0) && (remain_data > 0)) {
                    remain_data -= sent_bytes;
                }
                
                client_array[i].master_sockfd = EMPTY_SOCKET;
            } 
            
            /* revert the response of shell to the sender */ 
            if (strstr(tempmsg ,"--shellout--") == tempmsg) {
                
                char finalout[PACKET_SIZE] = "shellout @ ";
                strcat(finalout, new_client.uname.c_str());
                strcat(finalout, "@");
                strcat(finalout, new_client.ip_addr_str.c_str());
                strcat(finalout, "   > ");
                strcat(finalout, tempmsg + 12);
                cout<<finalout; 
                logfile<<finalout;
                finalout[strlen(finalout) - 1] = 0;
                send(new_client.master_sockfd, finalout, PACKET_SIZE, 0);
                if(strstr(tempmsg, "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m   Bash ") == tempmsg + 12) { /* If bash ends at client side */
                    cout<<"MASTER HAS SURRENDERED!!\n";
                    new_client.master_sockfd = EMPTY_SOCKET;
                }
            }
            
            /* Check whether it is a pull request so after sending this request, client fires up for its regular upload procedure WITHOUT USERS CONCERN */ 
            if (strstr(tempmsg ,"--pull--") == tempmsg) {  
                
                client_type selected_client; int i = 0;
                char filename[PACKET_SIZE] ,errmsg[PACKET_SIZE];
                if(validate_separator_tags(tempmsg, new_client.sockfd))
                    continue;
                for(; i < MAX_CLIENTS; i++)
                    if(strstr(tempmsg + 10, client_array[i].uname.c_str()) == tempmsg + 10) {
                        strcpy(filename, strstr(tempmsg, "--file-- ") + 9);
                        selected_client = client_array[i];
                        strcpy(errmsg, "Server is pulling from you a file '");
                        strcat(errmsg, filename);
                        strcat(errmsg, "' \n");
                        logfile<<errmsg;
                        send(selected_client.sockfd, errmsg, PACKET_SIZE, 0);
                        break; 
                    }
                    
                if(i == MAX_CLIENTS) {
                    strcpy(errmsg, "The user DOSENT EXIST !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0);
                    continue;
                }
                
                if(selected_client.uname != new_client.uname) {
                    strcpy(errmsg, "pull request acknowledged !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0);
                }
                
                char request[PACKET_SIZE] = "--pull-- ";
                strcat(request, filename);
                client_array[i].master_sockfd = new_client.sockfd;
                send(selected_client.sockfd, request, PACKET_SIZE, 0); /* no actual file operations in this block as it is imposed client side action */
            }
            
            /* Check whether client wants to leave the server ... for server it detaches the thread maintained for that client ... else just broadcast the chat message */ 
            if(iResult != SOCKET_ERROR && strcmp(tempmsg, "--exit--")) {
                
                if(!is_uname_set){
                    char errmsg[PACKET_SIZE];
                    
                    /* check whether the given uname already exists or not to make everyone in group uniquely identified */
                    if(!strcmp(tempmsg, "--all--")){
                        strcpy(errmsg, "--all-- represents everyone online ... this cant be your username!!\n Please enter a different username: ");
                        send(new_client.sockfd, errmsg, strlen(errmsg), 0);     /* --all-- is a username which represents everyone connected to server */
                        goto uname_exists;                                      /* any command w.r.t. --all-- implies on every online user */
                    } 
                    for(int i = 0; i < MAX_CLIENTS; i++)
                        if(client_array[i].uname == string(tempmsg)){
                            strcpy(errmsg, "SORRY this username already exists!!\n Please enter a different username: ");
                            send(new_client.sockfd, errmsg, strlen(errmsg), 0);
                            goto uname_exists; 
                        }
                        
                    new_client.uname = string(tempmsg);
                    strcpy(tempmsg, "\n\n\n          Welcome to group \033[1;94m");
                    strcat(tempmsg, cols[new_client.id % COLS].c_str());
                    strcat(tempmsg, new_client.uname.c_str());
                    strcat(tempmsg, "\033[0m !! You can now send messages...\n");
                    send(new_client.sockfd, tempmsg, strlen(tempmsg), 0);
                    show_online_users(new_client.sockfd, client_array);
                    
                } if (strcmp("", tempmsg)) {
                    if(!is_uname_set){
                        msg = string(timestring) + new_client.uname + " has joined the group!\n";
                        is_uname_set = true;
                    } else
                        msg = "\033[1;94m" + cols[new_client.id % COLS]
                              + timestring + "@"
                              + new_client.uname + ": " 
                              + tempmsg + " \033[0m";
                } 
                
                /* Check if it is a shell response */
                if(strstr(tempmsg ,"--shellout--") != tempmsg) {
                    cout << msg << endl; logfile << msg << endl;
                } 
                /* Broadcast the chat message to the other clients only when it is not any command */
                if( strstr(tempmsg ,"--upload--") != tempmsg && 
                    strstr(tempmsg ,"--download--") != tempmsg && 
                    strstr(tempmsg ,"--shell--") != tempmsg && 
                    strstr(tempmsg ,"--shellout--") != tempmsg && 
                    strstr(tempmsg ,"--pull--") != tempmsg &&
                    strstr(tempmsg ,"--push--") != tempmsg &&
                    strstr(tempmsg ,"--filemeta--") != tempmsg &&
                    strstr(tempmsg ,"--anyonehere--") != tempmsg &&
                    msg != ""
                ) { for(int i = 0; i < MAX_CLIENTS; i++)
                        if (client_array[i].sockfd != EMPTY_SOCKET && new_client.id != i)
                            iResult = send(client_array[i].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
                }
            } else {
                
                msg = string(timestring) + new_client.uname + " has left the group";
                cout << msg << endl; logfile << msg << endl; 
                
                /* Broadcast the disconnection message to the other clients */
                for (int i = 0; i < MAX_CLIENTS; i++)
                    if (client_array[i].sockfd != EMPTY_SOCKET)
                        iResult = send(client_array[i].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
                close(new_client.sockfd);
                close(client_array[new_client.id].sockfd);
                
                /* Securely erasing the username so no conflict arises in upload-download or push-pull or shell commands */
                client_array[new_client.id].uname = "";
                client_array[new_client.id].sockfd = EMPTY_SOCKET;
                
                if(FATAL_TERMINATE)  /* Directly exit if fatal terminate */
                    if(is_client_array_empty()) {
                        cout << "\n\n TERMINATING SERVER ON FATAL REQUEST ...\n\n";
                        exit(0);
                    }
                    
                self_thread.detach();
                return 0;
            }
        }
    } 
    /* Unreachable code here as this loop runs until the client sends exit request */
}

int main(int argc, char** argv) {
    
    int sockfd, port_no, num_clients = 0, temp_id = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    string msg = "";
    thread client_threads[MAX_CLIENTS];
    
    /* Initialize signal handlers */
    init_signal_handlers();
    
    cout << "    RVC TCP STATION SERVER Copyright (c) 2019 Rajas Chavadekar ( @rvcgeeks____ )\n";
    
    /* Arguments  :: -1 on input error */
    if (argc < 2) {
        cerr <<  "ERROR, no port provided\n"; 
        return -1;
    }
    port_no = atoi(argv[1]);
    if (port_no <= 0) {
        cerr <<  "Invalid port -.-\n"; 
        return -1; 
    }
    
    /* Create socket :: -2 on unsuccessful open */
    cout <<  "Creating server socket...\n";
    sockfd =  socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr <<  "ERROR opening socket\n"; 
        return -2;
    }
    
    /* Clear address structure */
    bzero((char *) &server_addr, sizeof(server_addr));
    
    /* Setup the host_addr structure for use in bind call */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);
    
    /* Bind :: -3 on unsuccessful */
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
        cerr <<  "ERROR on binding\n";
        return -3;
    }
    
    /* Make directories and start logging */
    int succ = system(
      " if ! [ -d ./share ]\n "
      "   then \n"
      "   mkdir share\n "
      " fi\n"
      " if ! [ -d ./logs ]\n "
      "   then \n"
      "   mkdir logs\n "
      " fi\n"
    );
    if(succ != 0) cout << "Error creating directories!!!\n";
    logfile.open(
        string("logs/") + (to_string(port_no) + "." 
        + to_string(int(time(NULL))) + ".log").c_str(),
        ios::out | ios::ate );
    
    /* Listen */
    listen(sockfd, 5);
    char local_host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr.sin_addr), local_host, INET_ADDRSTRLEN);
    string s = string(" Listening on ") + local_host + ":" + to_string(port_no) + "\n";
    cout << s; logfile << s;
    cout << " Current ip addresses of server :\n";
    int ret = system("hostname -I");
    
    cout<<" \033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m  Press Ctrl + C to terminate the server ( Will work only if no users are connected... ) \033[0m\n"<<ret;
    
    /* Initialize the client list */
    for (int i = 0; i < MAX_CLIENTS; i++)
        client_array[i] = (client_type) { -1, EMPTY_SOCKET, EMPTY_SOCKET, "", ""};
    client_len = sizeof(client_addr);
    
    /* Serve */
    for (;;) {
        
        /* Accept new connection */
        int incoming = EMPTY_SOCKET;
        incoming = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (incoming == EMPTY_SOCKET)
            continue;
        
        /* Reset the number of clients */
        num_clients = -1;
        
        /* Create a temporary id for the next client */
        temp_id = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client_array[i].sockfd == EMPTY_SOCKET && temp_id == -1) {
                client_array[i].sockfd = incoming;
                client_array[i].id = i; temp_id = i;
            }
            if (client_array[i].sockfd != EMPTY_SOCKET)
                num_clients++;
        }
        if (temp_id != -1) {
            char remote_host[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
            string accmsg = string("Connection accepted from ")
                            + remote_host + ":" + to_string(ntohs(client_addr.sin_port)) 
                            + " (Client #" + to_string(client_array[temp_id].id) 
                            + ") accepted\n";
            cout << accmsg; 
            logfile << accmsg;
            client_array[temp_id].ip_addr_str = string(remote_host) + ":" + to_string(ntohs(client_addr.sin_port));
            
            /* Send the id to that client */
            msg =  to_string(client_array[temp_id].id);
            send(client_array[temp_id].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
            
            /* Create a thread process for that client */
            client_threads[temp_id] =  thread(
                process_client,
                ref(client_array[temp_id]),
                ref(client_threads[temp_id]));
        } else {
            
            /* The server is full :( */
            msg = "Server is full";
            send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
            char remote_host[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
            string rejmsg = string("Connection rejected to ")
                            + remote_host + ":" + to_string(ntohs(client_addr.sin_port)) 
                            + " (Client #" + to_string(client_array[temp_id].id) 
                            + "). The server is full!\n";
            cout << rejmsg;
            logfile << rejmsg;
            
            /* Close connection */
            close(client_array[temp_id].sockfd);
        }
    }
    /* Unreachable code in this block as a server is infinite loop as it serves forever */
}