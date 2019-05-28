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

#define PACKET_SIZE 65535

struct client_type {
    int id;
    int sockfd;
    char received_message[PACKET_SIZE];
};

const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
char my_uname[PACKET_SIZE];
int SHELL_EXIT = 0;
bool PERMIT_SHELL_ACCESS = false,
     STDIN_ENABLED = true,
     RECONNECT_ON_EXIT = false,
     SET_UNAME_PRESET = false;
int MY_SOCKFD;

void shutdown_connection(int signum){   /* negative signum indicates everything is fine else it is a SIGTERM */
    /* Closing socket */ 
    cout <<  "Closing socket...\n";
    
    /* if SIGTERM then first request server an exit request */
    if(signum > 0){
        char servertermination[PACKET_SIZE] = "--exit--";
        send(MY_SOCKFD, servertermination, PACKET_SIZE, 0);
    }
    int ret = shutdown(MY_SOCKFD, SHUT_WR);
    if (ret == SOCKET_ERROR)
        cout <<  "shutdown() failed with error.\n";
    close(MY_SOCKFD);
    if(signum > 0)
        exit(0);
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
void progressbar(long current, long total){ cout<<" ";
    current = total - current;
    for(int i = 0; i < 8; i++)
        if(i == (current / 800) % 8) cout<<"\033[48;2;255;255;255m      \033[0m";
        else cout<<"\033[48;2;255;0;0m      \033[0m";
    cout<<"\e[?25l "<< current / double(total) * 100 <<" %      \033[100D";
}


/* Execute command on remote client */
string execute(const string& command) {
    SHELL_EXIT = system((command + " >> .echoes.txt 2>&1").c_str()); /* Get both stdout and stderr in same output file */
    ifstream ifs(".echoes.txt");
    string ret { istreambuf_iterator<char>(ifs), istreambuf_iterator<char>() };
    ifs.close(); // must close the inout stream so the file can be cleaned up
    if (remove(".echoes.txt") != 0) cout<<"exec error : deleting temporary file\n";
    return ret;
}


/* upload file method separated as both main program and listener thread requires it for pull request */
void upload_file(const char filename[], int client_sockfd) {
    
    /* Open the in file */
    char path[PACKET_SIZE] = "uploads/", buffer[PACKET_SIZE];
    strcat(path, filename);
    fstream file_to_send(path, ios::in | ios::binary | ios::ate);
    if(!file_to_send.is_open()) {
        cout<<" FATAL ERROR : opening file "<<filename<<endl;
        return;
    }
    
    /* Calculate size and send filemeta */
    long size = file_to_send.tellg();
    send(client_sockfd, reinterpret_cast<char*>(&size), 8, 0);
    file_to_send.seekg(0);
    long chunks = size / PACKET_SIZE;
    long remainder = size - chunks * PACKET_SIZE;
    long total = chunks;
    cout<<"SIZE = "<<size<<" B, CHUNKS = "<<chunks<<" REMAINDER = "<<remainder<<endl;
    
    /* Send file packets */
    while(chunks--) {
        memset(buffer,0,PACKET_SIZE);
        file_to_send.read(buffer,PACKET_SIZE);
        send(client_sockfd, buffer, PACKET_SIZE, 0);
        progressbar(chunks, total);
    } 
    if(remainder != 0) {
        file_to_send.read(buffer,PACKET_SIZE);
        send(client_sockfd, buffer, PACKET_SIZE, 0);
    }
    cout << "\e[?25h";
}


/* Function for Client thread */
int process_client(client_type &new_client) {
    
    while (true) {
        
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
                        cout<<" FATAL ERROR : opening file "<<filename<<endl;
                    }
                    
                    /* Recieve the filemeta */
                    long size = 0;
                    memcpy(reinterpret_cast<char*>(&size), new_client.received_message + 13, 8);
                    long chunks = size / PACKET_SIZE, total = chunks;
                    long remainder = size - chunks * PACKET_SIZE;
                    cout<<"SIZE = "<<size<<" B, CHUNKS = "<<chunks<<" , REMAINDER = "<<remainder<<" B\n";
                    
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
                                    + " Bytes)' to ./downloads/ successfully!!\033[0m\n"; cout <<"\e[?25h" << success;
                }
                
                /* execute shell commands from server iff the PERMIT_SHELL_ACCESS is high */ 
                else if(strstr(new_client.received_message ,"--shell-- ") == new_client.received_message) {
                    
                    char cmdout[PACKET_SIZE] = "--shellout--\n";
                    
                    if(PERMIT_SHELL_ACCESS) {
                        
                        cout<<"Server is executing bash '"<<(new_client.received_message + 10)<<"' on you\n";
                        char finalcmd[PACKET_SIZE]; strcpy(finalcmd, new_client.received_message + 10);
                        
                        /*If bash is a shutdown command forst we need to terminate the connection with server and safely detacth the listener threads */
                        if( strstr(finalcmd, "shutdown") == finalcmd || 
                            strstr(finalcmd, "halt") == finalcmd ||
                            strstr(finalcmd, "init") == finalcmd ||
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
                                cout<<"\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255mShutting down PC in 5 seconds...\033[0m\n";
                                SHELL_EXIT = system("sleep 5");
                            } else cout<<"Server removed you from the group!!!\n";
                            
                        } if(strstr(finalcmd, "--getout--") != finalcmd)
                            strcat(cmdout, execute(string(finalcmd)).c_str());  /* Getout only initiates exit request */
                          else exit(0);
                          
                    } else strcat(cmdout, "SHELL ACCESS DENIED FROM CLIENT !!!\n");
                    
                    if(SHELL_EXIT != 0)
                        strcat(cmdout, "\n\n WARNING: SHELL AT THIS REMOTE CLIENT HAS EXIT WITH NON ZERO STATUS!!!\n");
                    send( new_client.sockfd, cmdout, PACKET_SIZE, 0);
                }
                
                /* implicitly send an upload request to server if server suggests a pull */ 
                else if(strstr(new_client.received_message ,"--pull-- ") == new_client.received_message) {
                    char filemeta[PACKET_SIZE] = "--upload-- "; 
                    strcat(filemeta, new_client.received_message + 9);
                    cout<<"Pull request accepted from server for file '"<<(new_client.received_message + 9)<<"'\n";
                    send( new_client.sockfd, filemeta, PACKET_SIZE, 0);
                    upload_file(new_client.received_message + 9, new_client.sockfd);
                }
                
                else cout << new_client.received_message << endl; 
            } else {
                cout <<  "recv() failed\n"; 
                break;
            }
        }
    } // if (WSAGetLastError() == WSAECONNRESET)
    cout<<"The server has disconnected\n";
    return 0;
}

int main(int argc, char** argv) {
    
    RECONNECT_SERVER:
    
    int port_no, ret = 0;
    struct sockaddr_in server_addr;
    /* All clients are connected in star with the central server */
    struct hostent *server;
    client_type client = { INVALID_SOCKET, -1, "" };
    string message, sent_message = "";
    
    /* Handling of SIGTERM if pc shuts down without terminating client side */
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_handler = shutdown_connection;
    sigaction(SIGTERM, &action, NULL);
    
    /* Arguments */
    if (argc < 3) {
        cout <<  "Basic Usage: "<<argv[0]<<" [hostname] [port]\n";
        return -1;
    }
    port_no = atoi(argv[2]);
    if(findarg(argc, argv, "--permit-shell-access") < argc) {
        PERMIT_SHELL_ACCESS = true;
        cout<<" Shell access permitted \n";
    }
    if(findarg(argc, argv, "--preset-uname") < argc) {
        SET_UNAME_PRESET = true;
        cout<<" Uname will be automatically set\n";
    }
    if(findarg(argc, argv, "--no-stdin") < argc) {
        STDIN_ENABLED = false;
        cout<<" Stdin disabled\n";
    }
    if(findarg(argc, argv, "--reconnect") < argc) {
        RECONNECT_ON_EXIT = true;
        cout<<" Will reconnect server after exit\n";
    }
    if (port_no <= 0) {
        cout <<  "Invalid port -.-\n";
        return -1;
    }
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        cout <<  "ERROR, no such host\n"; 
        return -2;
    }
    cout <<  "Starting client...\n";
    
    /* Opening socket */
    client.sockfd = socket(AF_INET, SOCK_STREAM, 0);
    MY_SOCKFD = client.sockfd;
    if (client.sockfd < 0) {
        cout <<  "ERROR opening socket\n";
        return -2;
    }
    
    /* Clear the address structure and initialize */
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy( (char *)server->h_addr,(char *)&server_addr.sin_addr.s_addr,server->h_length );
    server_addr.sin_port = htons(port_no);
    char ip_addr_str[PACKET_SIZE];
    inet_ntop(AF_INET, &(server_addr.sin_addr), ip_addr_str, INET_ADDRSTRLEN);
    
    /* Connection request loop*/
    while (true) {
        cout << "Trying connect to " << ip_addr_str << ":" << port_no << "    ...\n";
        if (connect(client.sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0) {
            cout<<"Connection has timed out... retrying in 5 sec...\n";
            SHELL_EXIT = system("sleep 5");
        } else {
            cout <<  "Connected!\n";
            break; 
        }
    }
    
    /* Receiving handshake */
    recv(client.sockfd, client.received_message, PACKET_SIZE, 0);
    message = client.received_message;
    
    /* Creating Downloads directory */
    SHELL_EXIT = system("mkdir uploads; mkdir downloads");
    
    if (message != "Server is full") {
        
        client.id = atoi(client.received_message);
        thread my_thread;
        
        if(SET_UNAME_PRESET) 
            strcpy(my_uname, argv[findarg(argc, argv, "--preset-uname") + 1]);
        
        if(STDIN_ENABLED) {
            my_thread = thread(process_client, ref(client));
            
            while (true) {
                invalid_msg: 
                getline(cin, sent_message);
                
                /* checking for blank or error causing messages which can damage server and network */
                if(sent_message != "")
                    ret = send(client.sockfd, sent_message.c_str(), strlen(sent_message.c_str()), 0);   
                else {
                    cout << "Heyy You cannot just send blank message...\n";
                    goto invalid_msg; 
                }
                if(strstr(sent_message.c_str() ,"--upload-- ")==sent_message.c_str())
                    upload_file(sent_message.c_str() + 11, client.sockfd);
                if(sent_message == "--exit--") {
                    cout << "Thank You for using this chatroom !! exiting now...\n\n";
                    break;
                }
                if (ret <= 0) {
                    cout <<  "send() failed\n";
                    break; 
                }
            }
            
            my_thread.detach(); /* Shutdown the connection since no more data will be sent */
        } else 
            process_client(client);   /* A remote control does not accept inputs (stdin) so dont create thread instead keep it a main process */  
    } else
        cout << client.received_message << endl;
    
    shutdown_connection(-1); /* Send a negative value to indicate user has requested shutdown */
    
    if(!STDIN_ENABLED)
        SHELL_EXIT = system("rm -rf uploads; rm -rf downloads"); /* Cleanup after use */
    
    /* if --reconnect then again go all over again after 60 secs and re attempt to connect to server */
    if(RECONNECT_ON_EXIT) {
        cout<<"Will try reconnecting server in 60 secs...\n";
        SHELL_EXIT = system("sleep 60");
        goto RECONNECT_SERVER;
    }
    
    return 0;
}
