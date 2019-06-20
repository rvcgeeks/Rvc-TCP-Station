/* ___________________________________________________________
 * ____________<<<___#_R_V_C_G_E_E_K_S___>>>__________________
 * CREATED BY #RVCGEEKS @PUNE for more rvchavadekar@gmail.com
 *
 * #RVCGEEKS TCP CLIENT ... only connects with rvcchats-server
 * created on 12.05.2019
 * 
*/

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <csignal>
#include <netdb.h>
using namespace std;

/* Declarations */

#define PACKET_SIZE 2048

struct client_type {
    int id;
    int sockfd;
    char received_message[PACKET_SIZE];
};

const int EMPTY_SOCKET = -1,
          SOCKET_ERROR = -1;
char my_uname[PACKET_SIZE];
int SHELL_EXIT = 0,
    MY_SOCKFD = -1;
bool PERMIT_SHELL_ACCESS = false,
     CONSOLE_IO_ENABLED = true,
     RECONNECT_ON_EXIT = false,
     SET_UNAME_PRESET = false;
     
#define mycout if(CONSOLE_IO_ENABLED) cout 

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

/* returns postition of given flags in argv */
int findarg(int argc, char **argv, const char arg[]){
    int i = 0;
    for(; i < argc; i++)
        if(!strcmp(argv[i], arg))
            break;
    return i;
}

/* Progressbar animation */
void progressbar(long current, long total){ mycout << " ";
    current = total - current;
    for(int i = 0; i < 8; i++)
        if(i == (current / 800) % 8) { mycout << "\033[48;2;255;255;255m      \033[0m"; }
        else { mycout << "\033[48;2;255;0;0m      \033[0m"; }
    mycout << "\e[?25l "<< current / double(total) * 100 <<" %      \033[100D";
}

/* Handling for termination signals */
void shutdown_connection(int signum){   /* negative signum indicates everything is fine else it is signal sent by OS */
    
    if(MY_SOCKFD >=0 ) {
        /* if positive signum then first request server an exit request */
        if(signum > 0) {
            mycout << " Signal "<<signum<<" acknowledged !!! \n";
            string msg =  string("\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m      SIGNAL ") 
                          + to_string(signum) + " OCCURED ON CLIENT  !!!      \033[0m\n";
            send(MY_SOCKFD, msg.c_str(), PACKET_SIZE, 0);
            msg = "--exit--";
            send(MY_SOCKFD, msg.c_str(), PACKET_SIZE, 0);
        }
        /* Closing socket */ 
        mycout <<  "Closing connection...\n";
        int ret = shutdown(MY_SOCKFD, SHUT_WR);
        if (ret == SOCKET_ERROR)
            mycout <<  "shutdown() failed with error.\n";
        close(MY_SOCKFD);
    }
    if(signum > 0)
        exit(0);
}

/* Handling of interrupt signals -- donot permit interrupts */
void donot_disturb(int signum) {
    mycout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m    WE DON'T DO THAT HERE ;)    \033[0m\n";
    string msg =  string("\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m      SIGNAL ") 
                  + to_string(signum) + " OCCURED ON CLIENT  !!!   However session will be resumed ...   \033[0m"; /* Inform server about this misbehavior */
    if(MY_SOCKFD >= 0)
        send(MY_SOCKFD, msg.c_str(), PACKET_SIZE, 0);
}

/* Handling of OS signals */
void init_signal_handlers() {
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    
    /* Fatal signals ... need to close connection and exit */
    action.sa_handler = shutdown_connection;
    sigaction(SIGTERM, &action, NULL);
    sigaction(SIGKILL, &action, NULL);
    sigaction(SIGABRT, &action, NULL);
    
    /* User level termination signals ... Ignore them and continue */
    action.sa_handler = donot_disturb;
    sigaction(SIGQUIT, &action, NULL);
    sigaction(SIGINT , &action, NULL);
    sigaction(SIGTSTP, &action, NULL);
}

void exec(char cmdout[],const char finalcmd[], int sockfd) {
    char shellouts[PACKET_SIZE];
    
    /* Create a pipe for that command to enable live responses from shell */
    unique_ptr<FILE, decltype(&pclose)> pipe(popen(finalcmd, "r"), pclose);
    if (!pipe) {
        strcpy(cmdout, "popen() failed!");
        send(sockfd, cmdout, strlen(cmdout), 0);
    } else while (fgets(cmdout, PACKET_SIZE, pipe.get()) != NULL) {
        memset(shellouts, 0, PACKET_SIZE);
        if(sockfd == EMPTY_SOCKET) {
            mycout << "shellout @--self--   > " << cmdout <<endl;
        } else {
            strcpy(shellouts, "--shellout--");
            strcat(shellouts, cmdout); 
            send(sockfd, shellouts, PACKET_SIZE, 0);
        }
    }
    strcpy(shellouts, "--shellout--\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m   Bash ");
    strcat(shellouts, finalcmd);
    strcat(shellouts,"     Executed !!!     \033[0m\n\n");
    if(sockfd == EMPTY_SOCKET) {
        mycout << (shellouts + 12);
    } else
        send(sockfd, shellouts, PACKET_SIZE, 0);
}

/* upload file method separated as both main program and listener thread requires it for pull request */
void upload_file(const char filename[], int client_sockfd) {
    
    /* Open the in file */
    char path[PACKET_SIZE] = "uploads/", buffer[PACKET_SIZE];
    strcat(path, filename);
    long size = -1;
    fstream file_to_send(path, ios::in | ios::binary | ios::ate);
    if(!file_to_send.is_open()) {
        mycout << " FATAL ERROR : opening file "<<filename<<endl;
        send(client_sockfd, reinterpret_cast<char*>(&size), 8, 0); /* -ve size indicates file not successfully opened at client side */
        return;
    }
    
    /* Calculate size and send filemeta */
    size = file_to_send.tellg();
    send(client_sockfd, reinterpret_cast<char*>(&size), 8, 0);
    file_to_send.seekg(0);
    long chunks = size / PACKET_SIZE;
    long remainder = size - chunks * PACKET_SIZE;
    long total = chunks;
    mycout << "SIZE = "<<size<<" B, CHUNKS = "<<chunks<<" REMAINDER = "<<remainder<<endl;
    chunks++;
    /* Send file packets */
    while(chunks--) {
        memset(buffer,0,PACKET_SIZE);
        file_to_send.read(buffer,PACKET_SIZE);
        send(client_sockfd, buffer, PACKET_SIZE, 0);
        progressbar(chunks, total);
    } mycout << "\e[?25h";
}


/* Function for Client thread */
int process_client(client_type &new_client) {
    
    for (;;) {
        
        memset(new_client.received_message, 0, PACKET_SIZE);
        
        if (new_client.sockfd != 0) {
            
            int iResult = recv(new_client.sockfd, new_client.received_message,PACKET_SIZE, 0);
            if(SET_UNAME_PRESET) {
                send(new_client.sockfd, my_uname, PACKET_SIZE, 0); /* Send the preset uname to client if option enabled */ 
                SET_UNAME_PRESET = false;
            } 
            
            if (iResult != SOCKET_ERROR) {
                
                /* Server wants to send file download packets ... for client, accept them and build the file */
                if(strstr(new_client.received_message ,"--filemeta-- ") == new_client.received_message) {
                    
                    /* Open the out file */
                    char filename[PACKET_SIZE] = "downloads/", buffer[PACKET_SIZE];
                    strcat(filename, new_client.received_message + 21);
                    fstream file_to_recieve(filename, ios::out | ios::binary);
                    if(!file_to_recieve.is_open()) { 
                        mycout << " FATAL ERROR : opening file "<<filename<<endl;
                        continue;
                    }
                    
                    /* Recieve the filemeta */
                    long size = 0;
                    memcpy(reinterpret_cast<char*>(&size), new_client.received_message + 13, 8);
                    long chunks = size / PACKET_SIZE, total = chunks;
                    long remainder = size - chunks * PACKET_SIZE;
                    mycout << "SIZE = "<<size<<" B, CHUNKS = "<<chunks<<" , REMAINDER = "<<remainder<<" B\n";
                    
                    /* Receive file packets */
                    while(chunks--) {
                        memset(buffer, 0, PACKET_SIZE);
                        recv(new_client.sockfd, buffer, PACKET_SIZE, 0);
                        file_to_recieve.write(buffer, PACKET_SIZE);
                        progressbar(chunks, total);
                    } 
                    memset(buffer, 0, PACKET_SIZE);
                    if(remainder != 0) {
                        recv(new_client.sockfd, buffer, PACKET_SIZE, 0);
                        file_to_recieve.write(buffer, remainder);
                    }
                    
                    string success = string("\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m") 
                                    + " You have successfully downloaded '" + string(filename + 10) + " (" + to_string(size)
                                    + " Bytes)' to ./downloads/ successfully!!\033[0m\n"; mycout <<"\e[?25h" << success;
                }
                
                /* execute shell commands from server iff the PERMIT_SHELL_ACCESS is high */ 
                else if(strstr(new_client.received_message ,"--shell-- ") == new_client.received_message) {
                    
                    char cmdout[PACKET_SIZE] = "--shellout--\n";
                    
                    if(PERMIT_SHELL_ACCESS || !strcmp(new_client.received_message + 10, "--getout--")) {
                        
                        mycout << "Server is executing bash '"<<(new_client.received_message + 10)<<"' on you\n";
                        char finalcmd[PACKET_SIZE]; strcpy(finalcmd, new_client.received_message + 10);
                        
                        /*If bash is a shutdown command forst we need to terminate the connection with server and safely detacth the listener threads */
                        if( strstr(finalcmd, "shutdown") == finalcmd || 
                            strstr(finalcmd, "halt") == finalcmd ||
                            strstr(finalcmd, "init") == finalcmd ||
                            strstr(finalcmd, "reboot") == finalcmd ||
                            strstr(finalcmd, "--getout--") == finalcmd
                        ) { 
                            char servertermination[PACKET_SIZE] = "--exit--";
                            strcat(cmdout, "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255mCOMPLETE PC POWEROFF REQUEST INITIATED...\nTERMINATING SOCKETS...\033[0m\n");
                            
                            if(strstr(finalcmd, "--getout--") != finalcmd)
                                send(new_client.sockfd, cmdout, PACKET_SIZE, 0);
                            
                            send(new_client.sockfd, servertermination, PACKET_SIZE, 0);
                            int ret = shutdown(new_client.sockfd, SHUT_WR);
                            
                            if(RECONNECT_ON_EXIT && strstr(finalcmd, "--getout--") == finalcmd && ret != SOCKET_ERROR)
                                return 0; /* Donot fiddle with sockets here as thery are managed in main for reconnection */
                            
                            shutdown_connection(-1);
                            
                            if(strstr(finalcmd, "--getout--") != finalcmd) {
                                mycout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255mShutting down PC in 5 seconds...\033[0m\n";
                                msleep(5000);
                            } else mycout << "Server removed you from the group!!!\n";
                            
                        } if(strstr(finalcmd, "--getout--") != finalcmd) {  /* Getout only initiates exit request */
                            char ack[PACKET_SIZE];
                            thread shell_thread(exec, cmdout, finalcmd, new_client.sockfd);
                            shell_thread.detach();   /* Immediately detach as exec will handle on its own ... freed it from parent thread */
                            strcpy(ack, "--shellout--           << Thread for bash on RSH started >>  ");
                            send(new_client.sockfd, ack, PACKET_SIZE, 0);
                        }
                        else exit(0);
                    } else {
                        strcat(cmdout, "SHELL ACCESS DENIED FROM CLIENT !!!\n");
                        send(new_client.sockfd, cmdout, PACKET_SIZE, 0);
                    }
                }
                
                /* execute shell commands from server iff the PERMIT_SHELL_ACCESS is high */ 
                else if(strstr(new_client.received_message ,"--shellout--") == new_client.received_message) {
                    mycout << (new_client.received_message + 12);
                }
                
                /* implicitly send an upload request to server if server suggests a pull */ 
                else if(strstr(new_client.received_message ,"--pull--") == new_client.received_message) {
                    char filemeta[PACKET_SIZE] = "--upload-- "; 
                    strcat(filemeta, new_client.received_message + 9);
                    mycout << "Pull request accepted from server for file '"<<(new_client.received_message + 9)<<"'\n";
                    send( new_client.sockfd, filemeta, PACKET_SIZE, 0);
                    upload_file(new_client.received_message + 9, new_client.sockfd);
                }
                
                else mycout << new_client.received_message << endl; 
            } else {
                mycout <<  "recv() failed\n"; 
                break;
            }
        }
    } // if (WSAGetLastError() == WSAECONNRESET)
    mycout << "The server has disconnected\n";
    return 0;
}

int main(int argc, char** argv) {
    
    RECONNECT_SERVER:
    
    int port_no, ret = 0;
    struct sockaddr_in server_addr;
    /* All clients are connected in star with the central server */
    struct hostent *server;
    client_type client = { EMPTY_SOCKET, -1, "" };
    string message, sent_message = "";
    
    mycout << "         RVC TCP STATION CLIENT Copyright (c) 2019 Rajas Chavadekar ( @rvcgeeks____ )\n";
    
    /* Arguments :: -1 on user error */
    if (argc < 3) {
        cout <<  "Basic Usage: "<<argv[0]<<" [server ip address] [port]\n";
        return -1;
    }
    port_no = atoi(argv[2]);
    if(findarg(argc, argv, "--no-console") < argc) {
        CONSOLE_IO_ENABLED = false;
    }
    if(findarg(argc, argv, "--permit-shell-access") < argc) {
        PERMIT_SHELL_ACCESS = true;
        mycout << " Shell access permitted \n";
    }
    if(findarg(argc, argv, "--preset-uname") < argc) {
        SET_UNAME_PRESET = true;
        mycout << " Uname will be automatically set\n";
    }
    if(findarg(argc, argv, "--reconnect") < argc) {
        RECONNECT_ON_EXIT = true;
        mycout << " Will reconnect server after exit\n";
    }
    if (port_no <= 0) {
        mycout <<  "Invalid port -.-\n";
        return -1;
    }
    
    /* get server address :: -2 on unsuccessful */
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        mycout <<  "ERROR, no such host\n"; 
        return -2;
    }
    
    mycout <<  "Starting client...\n";
    
    /* Opening socket :: -3 on error */
    client.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    
    if (client.sockfd < 0) {
        mycout <<  "ERROR opening socket\n";
        return -3;
    }
    
    /* Clear the address structure and initialize */
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr,(char *)&server_addr.sin_addr.s_addr,server->h_length );
    server_addr.sin_port = htons(port_no);
   /* if(inet_pton(AF_INET, argv[1], &server_addr.sin_addr)<=0) {
        printf("\n inet_pton error occured\n");
        return -2;
    }*/ 
    char ip_addr_str[PACKET_SIZE];
    inet_ntop(AF_INET, &(server_addr.sin_addr), ip_addr_str, INET_ADDRSTRLEN); // again conversion for validating that inet_pton is working
    
    /* Connection request loop*/
    for (int i = 1; ; i++) {
        mycout << " [ TRY " << i << " ] : Trying CONNECT server @ " << ip_addr_str << ":" << port_no << " ...\n";
        if (connect(client.sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            mycout << "\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m      Couldn't connect :( ... retrying in 5 sec.     \033[0m\n";
            msleep(5000);
        } else {
            mycout <<  "Connected!\n";
            break; 
        }
    }
    MY_SOCKFD = client.sockfd;
    
    /* Initialize signal handlers */
    init_signal_handlers();
    
    /* Receiving handshake */
    recv(client.sockfd, client.received_message, PACKET_SIZE, 0);
    message = client.received_message;
    
    /* Creating Downloads directory */
    SHELL_EXIT = system(
      " if ! [ -d ./uploads ]\n "
      "   then \n"
      "   mkdir uploads\n "
      " fi\n"
      " if ! [ -d ./downloads ]\n "
      "   then \n"
      "   mkdir downloads\n "
      " fi\n"
    );
    if (message != "Server is full") {
        
        client.id = atoi(client.received_message);
        thread my_thread;
        
        if(SET_UNAME_PRESET) 
            strcpy(my_uname, argv[findarg(argc, argv, "--preset-uname") + 1]);
        
        if(CONSOLE_IO_ENABLED) {
            my_thread = thread(process_client, ref(client));
            
            for (;;) {
                getline(cin, sent_message);
                cin.clear();  /* Clean buffer for next input (if interrupt , clear it off )*/
                if(strstr(sent_message.c_str(),"--shell-- @--self-- --bash-- ") == sent_message.c_str()){ /* Execute a shell on localhost (self) */
                    char cmdout[PACKET_SIZE];
                    exec(cmdout, sent_message.c_str() + 29, EMPTY_SOCKET);
                    continue;
                }
                if(sent_message != "")  /* checking for blank or error causing messages which can damage server and network */
                    ret = send(client.sockfd, sent_message.c_str(), strlen(sent_message.c_str()), 0);   
                if(strstr(sent_message.c_str() ,"--upload-- ")==sent_message.c_str())
                    upload_file(sent_message.c_str() + 11, client.sockfd);
                if(sent_message == "--exit--") {
                    mycout << "Thank You for using this chatroom !! exiting now...\n\n";
                    break;
                }
                if (ret <= 0) {
                    mycout <<  "send() failed\n";
                    break; 
                }
            }
            
            my_thread.detach(); /* Shutdown the connection since no more data will be sent */
        } else 
            process_client(client);   /* A remote control does not accept inputs (stdin) so dont create thread instead keep it a main process */  
    } else
        mycout << client.received_message << endl;
    
    shutdown_connection(-1); /* Send a negative value to indicate user has requested shutdown */
    
    if(!CONSOLE_IO_ENABLED)
        SHELL_EXIT = system("rm -rf uploads; rm -rf downloads"); /* Cleanup after use */
    
    /* if --reconnect then again go all over again after 60 secs and re attempt to connect to server */
    if(RECONNECT_ON_EXIT) {
        mycout << "Will try reconnecting server in 60 secs...\n";
        msleep(60000);
        goto RECONNECT_SERVER;
    }
    
    return 0;
}