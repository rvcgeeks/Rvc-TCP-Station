/* ___________________________________________________________
 * ____________<<<___#_R_V_C_G_E_E_K_S___>>>__________________
 * CREATED BY #RVCGEEKS @PUNE for more rvchavadekar@gmail.com
 *
 * #RVCGEEKS TCP SERVER ... only rvcchats-client(s) can connect to it
 * created on 12.05.2019
 * 
*/

const char banner[] = "\n\033[48;2;255;255;255m\033[1;94m\033[38;2;0;128;0m" /* This banner is displayed when a client connects */
                      "                               <Live Chatting (TCP/IP)> <Multi User  (MC/MS)> \n"
                      " ███   █   █   ███             <File transfer    (FTP)> <Shell Control (RSH)> \n"
                      " █  █  █   █  █            ███ ███ ███                                        \n"
                      " ███   █   █  █             █  █   █ █          __________________            \n"
                      " █  █   █ █   █             █  █   ███         |  S T A T I O N   |           \n"
                      " █  █    █     ███          █  ███ █           |______v_1.0_______|           \n"
                      "                                                                              \n"
                      "Welcome to #rvgeeks CLI server ...                                            \n"
                      "Developed by Rajas Chavadekar @Pune                                           \n"
                      "Check out http://www.github.com/rvcgeeks for more cool stuff!!!               \n"
                      "Any queries reverted on rvchavadekar@gmail.com                                \n"
                      "                                                                              \n"
                      "            !!!  PLEASE READ THESE INSTRUCTIONS BEFORE USE !!!                \n"
                      "                                                                              \n"
                      "1) Just type the message and press enter to send your message.                \n"
                      "2) To upload a file first copy it in your 'uploads folder created by app' and \n"
                      "   then in your chats send command '--upload-- yourfilename' to server to send\n"
                      "   it on group.. everyone in that group will be able to download that file.   \n"
                      "3) To download a file in the group send the command '--download-- filename' to\n"
                      "   server and after successful download it will appear in your                \n"
                      "   'downloads' folder. Please accurately mention the filename.                \n"
                      "4) To execute a scell command on a user who permits one's shell access , use  \n"
                      "   '--shell-- @[username |--all--] --bash-- [actual commandline to execute]'  \n"
                      "   in the actual command if you give '--getout--' it will just terminate      \n"
                      "   the client connection. If it fails you will get some error message.        \n"
                      "5) The --pull-- command delibrately uploads a file from client side onto the  \n"
                      "   server. It will command the client program to upload prescribed file.      \n"
                      "   '--pull-- @[username] --file-- [accurate filename]' is its syntax.         \n"
                      "6) Similar is with --push-- command but the opposite action to command the    \n"
                      "   client to download the prescribed file from server.                        \n"
                      "   '--push-- @[username |--all--] --file-- [accurate filename]' is its syntax.\n"
                      "7) To exit the room just send ' --exit-- ' to the server so it will           \n"
                      "   terminate your connection.                                                 \033[0m\n\n"
                      "Please enter your unique username for other group members to identify:";

#include <cstdio>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cstring>
#include <thread>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <ctime> //for maintaining server time
#include <vector>
using namespace std;


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

/* Definitions */
#define PACKET_SIZE 65535

/* Declarations */
struct client_type {
    int id;
    int sockfd;
    string uname;
};

const int INVALID_SOCKET = -1;
const int SOCKET_ERROR = -1;
const int MAX_CLIENTS = 1000;
int shell_sender_client_id;  //unique person can only send shells so global
fstream logfile;

/* Function for Client thread */
int process_client(
    client_type &new_client,
     vector<client_type> &client_array,
     thread &thread
) {
    bool is_uname_set = false;
    /* Session */
    while (true) {
        /* Update server time for every loop */
        time_t timer; char timestring[PACKET_SIZE], tempmsg[PACKET_SIZE] = "";
        string msg = ""; struct tm *tm_info;
        time(&timer); tm_info = localtime(&timer);
        strftime(timestring, PACKET_SIZE, "[ %d/%m/%Y  %H:%M:%S ] >     ", tm_info);
        if (new_client.sockfd != 0) {
            /* Show the banner to a fresher */
            if(!is_uname_set) {
                msg = string(banner);
                send(new_client.sockfd, msg.c_str(), strlen(msg.c_str()), 0);
            } uname_exists:
            memset(tempmsg, 0, PACKET_SIZE);
            int iResult = recv(new_client.sockfd, tempmsg, PACKET_SIZE, 0);
            /* Check whether it is shell execution request */
            if(strstr(tempmsg,"--shell-- @") == tempmsg) {
                shell_sender_client_id = new_client.id;
                char *cmd = strstr(tempmsg, "--bash-- ") + 9, errmsg[PACKET_SIZE];  
                int i = 0;  /*Select user to send shell command .. if --all-- tag then send shell to all available users */
                for(; i < MAX_CLIENTS; i++)
                    if((strstr(tempmsg + 11, client_array[i].uname.c_str()) == tempmsg + 11 ||
                        strstr(tempmsg + 11, "--all--") == tempmsg + 11 ) &&  /* shells also can be executed in all users (if they permit) */
                       client_array[i].uname != "" && i != shell_sender_client_id
                    ) { char finalcmd[PACKET_SIZE] = "--shell-- ";
                        strcat(finalcmd, cmd);
                        send(client_array[i].sockfd, finalcmd, PACKET_SIZE, 0);
                        cout<<"Shell '"<<finalcmd<<"' sent to user @"<<client_array[i].uname<<"\n";
                        logfile<<"Shell '"<<finalcmd<<"' sent to user @"<<client_array[i].uname<<"\n"; 
                        if(strstr(tempmsg + 11, "--all--") != tempmsg + 11 ) break; }
                if(strstr(tempmsg + 11, "--all--") != tempmsg + 11 && i == MAX_CLIENTS) {
                    strcpy(errmsg, "The user DOSENT EXIST !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0); }
            } /* Check whether it is file upload request ... for server, accept packets and build file in ./share/ */
            if(strstr(tempmsg,"--upload-- ") == tempmsg) {
                char filename[PACKET_SIZE] = "share/", buffer[PACKET_SIZE];
                strcat(filename, tempmsg + 11);
                fstream file_to_recieve(filename, ios::out | ios::binary);
                if(!file_to_recieve.is_open()) {
                    cout<<" FATAL ERROR : opening file "<<filename<<endl; 
                    logfile<<" FATAL ERROR : opening file "<<filename<<endl; continue; 
                } long size = 0; recv(new_client.sockfd, reinterpret_cast<char*>(&size), 8, 0);
                long chunks = size / PACKET_SIZE;
                long remainder = size - chunks * PACKET_SIZE; /* As size of file is not always the exact multiple of PACKET_SIZE */
                cout<<"SIZE = "<<size<<" B, CHUNKS = "<<chunks<<" , REMAINDER = "<<remainder<<" B\n";
                /* recieving packets from client and building file in 'share' directory */
                while(chunks--) {
                    memset(buffer,0,PACKET_SIZE);
                    recv(new_client.sockfd, buffer, PACKET_SIZE, 0);
                    file_to_recieve.write(buffer,PACKET_SIZE);
                } memset(buffer,0,PACKET_SIZE);
                if(remainder !=0 ) {
                    recv(new_client.sockfd, buffer, PACKET_SIZE, 0);
                    file_to_recieve.write(buffer, remainder);
                } string success = string("\033[48;2;255;0;0m\033[1;94m\033[38;2;255;255;255m") 
                                 + timestring + " @" +new_client.uname
                                 + " has uploaded the file '" + string(filename + 6) + " ("
                                 + to_string(size) + " Bytes)' on server successfully!!\033[0m\n";
                /* Broadcast the upload message to the other clients */
                for (int i = 0; i < MAX_CLIENTS; i++) {
                    if (client_array[i].sockfd != INVALID_SOCKET)
                        iResult = send(client_array[i].sockfd, success.c_str(), PACKET_SIZE, 0);
                } cout << success << endl; logfile << success <<endl; continue;
            } /* Check whether it is a download or push request .. for server send the filemeta tag, its size and fragmented packets from file to client */ 
            if(strstr(tempmsg ,"--download-- ") == tempmsg || strstr(tempmsg ,"--push-- @") == tempmsg) {
                client_type selected_client = new_client; int i = 0;
                char filename[PACKET_SIZE] = "share/", buffer[PACKET_SIZE], errmsg[PACKET_SIZE];
                if(strstr(tempmsg , "--push-- @") == tempmsg ) {
                    for(; i < MAX_CLIENTS; i++)
                        if((strstr(tempmsg + 10, client_array[i].uname.c_str()) == tempmsg + 10 ||
                            strstr(tempmsg + 10, "--all--") == tempmsg + 11 ) &&  /* shells also can be executed in all users (if they permit) */
                            client_array[i].uname != "" && i != new_client.id
                        ) { /* parsing ther valid username for who will recieve push request */
                            strcat(filename, strstr(tempmsg, "--file-- ") + 9);
                            selected_client = client_array[i];
                            strcpy(errmsg, "Server is pushing you a file '");
                            strcat(errmsg, filename); strcat(errmsg, "' \n");
                            logfile<<errmsg; send(selected_client.sockfd, errmsg, PACKET_SIZE, 0);
                            break; }
                } else {
                    selected_client = new_client;
                    strcat(filename, tempmsg + 13);
                } if(i == MAX_CLIENTS) {
                    strcpy(errmsg, "The user DOSENT EXIST !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0); continue;
                } if(selected_client.uname != new_client.uname){
                    strcpy(errmsg, "push request acknowledged !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0);
                } fstream file_to_send(filename, ios::in | ios::binary | ios::ate);
                if(!file_to_send.is_open()) {
                    logfile<<" FATAL ERROR : opening file "<<filename<<endl;
                    cout<<" FATAL ERROR : opening file "<<filename<<endl; continue;
                } long size = file_to_send.tellg();
                char filemeta[PACKET_SIZE] = "--filemeta-- ";
                memcpy(filemeta + 13, reinterpret_cast<char*>(&size), 8);
                strcat(filemeta + 21, filename + 6);
                /* Send the filesize so client accepts only required number of packets under download request */
                send(selected_client.sockfd, filemeta, PACKET_SIZE, 0);
                file_to_send.seekg(0);
                long chunks = size / PACKET_SIZE;
                long remainder = size - chunks * PACKET_SIZE;
                if(remainder != 0) chunks++;
                cout<<"SIZE = "<<size<<" B, CHUNKS = "<<chunks<<"\n";
                /* Reading file packet by packet and sending it to client under download or push request */
                while(chunks--){
                    memset(buffer,0,PACKET_SIZE);
                    file_to_send.read(buffer,PACKET_SIZE);
                    send(selected_client.sockfd, buffer, PACKET_SIZE, 0);
                }
            } /* Send the response of shell to the sender */ 
            if (strstr(tempmsg ,"--shellout--") == tempmsg) {
                 char finalout[PACKET_SIZE] = " Shell response from @";
                 strcat(finalout, new_client.uname.c_str());
                 strcat(finalout, " : \n"); strcat(finalout, tempmsg + 12);
                 cout<<finalout<<endl; logfile<<finalout<<endl;
                 send(client_array[shell_sender_client_id].sockfd, finalout, PACKET_SIZE, 0);
            } /* Check whether it is a pull request so after sending this request, client fires up for its regular upload procedure WITHOUT USERS CONCERN */ 
            if (strstr(tempmsg ,"--pull-- @") == tempmsg) {             
                client_type selected_client; int i = 0;
                char filename[PACKET_SIZE] ,errmsg[PACKET_SIZE];
                for(; i < MAX_CLIENTS; i++)
                    if(strstr(tempmsg + 10, client_array[i].uname.c_str()) == tempmsg + 10) {
                        strcpy(filename, strstr(tempmsg, "--file-- ") + 9);
                        selected_client = client_array[i];
                        strcpy(errmsg, "Server is pulling from you a file '");
                        strcat(errmsg, filename); strcat(errmsg, "' \n");
                        logfile<<errmsg; send(selected_client.sockfd, errmsg, PACKET_SIZE, 0);
                        break; }
                if(i == MAX_CLIENTS) {
                    strcpy(errmsg, "The user DOSENT EXIST !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0); continue;
                } if(selected_client.uname != new_client.uname){
                    strcpy(errmsg, "pull request acknowledged !!!\n");
                    send(new_client.sockfd, errmsg, PACKET_SIZE, 0);
                } char request[PACKET_SIZE] = "--pull-- "; strcat(request, filename);
                send(selected_client.sockfd, request, PACKET_SIZE, 0); /* no actual file operations in this block as it is imposed client side action */
            } /* Check whether client wants to leave the server ... for server it detaches the thread maintained for that client ... else just broadcast the chat message */ 
            if(iResult != SOCKET_ERROR && strcmp(tempmsg, "--exit--")) {
                if(!is_uname_set){ char errmsg[PACKET_SIZE];
                    /* check whether the given uname already exists or not to make everyone in group uniquely identified */
                    if(!strcmp(tempmsg, "--all--")){
                        strcpy(errmsg, "--all-- represents everyone online ... this cant be your username!!\n Please enter a different username: ");
                        send(new_client.sockfd, errmsg, strlen(errmsg), 0);     /* --all-- is a username which represents everyone connected to server */
                        goto uname_exists;                                      /* any command w.r.t. --all-- implies on every online user */
                    } for(int i = 0; i < MAX_CLIENTS; i++)
                        if(client_array[i].uname == string(tempmsg)){
                            strcpy(errmsg, "SORRY this username already exists!!\n Please enter a different username: ");
                            send(new_client.sockfd, errmsg, strlen(errmsg), 0);
                            goto uname_exists; }
                    new_client.uname = string(tempmsg);
                    strcpy(tempmsg, "\n\n\n          Welcome to group \033[1;94m");
                    strcat(tempmsg, cols[new_client.id % COLS].c_str());
                    strcat(tempmsg, new_client.uname.c_str());
                    strcat(tempmsg, "\033[0m !! You can now send messages...\n");
                    send(new_client.sockfd, tempmsg, strlen(tempmsg), 0);
                } if (strcmp("", tempmsg)) {
                    if(!is_uname_set){
                        msg = string(timestring) + new_client.uname + " has joined the group!\n";
                        is_uname_set = true;
                    } else msg = "\033[1;94m" + cols[new_client.id % COLS] + 
                               + timestring + "@"
                               + new_client.uname + ": " 
                               + tempmsg + " \033[0m";
                } if(strstr(tempmsg ,"--shellout--") != tempmsg) {
                    cout << msg << endl; logfile << msg << endl;
                } /* Broadcast the chat message to the other clients only when it is not any command */
                if( strstr(tempmsg ,"--upload--") != tempmsg && 
                    strstr(tempmsg ,"--download--") != tempmsg && 
                    strstr(tempmsg ,"--shell--") != tempmsg && 
                    strstr(tempmsg ,"--shellout--") != tempmsg && 
                    strstr(tempmsg ,"--pull--") != tempmsg &&
                    strstr(tempmsg ,"--push--") != tempmsg &&
                    strstr(tempmsg ,"--filemeta--") != tempmsg &&
                    msg != ""
                ) { for(int i = 0; i < MAX_CLIENTS; i++)
                        if (client_array[i].sockfd != INVALID_SOCKET && new_client.id != i)
                            iResult = send(client_array[i].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
                }
            } else {
                msg = string(timestring) + new_client.uname + " has left the group";
                cout << msg << endl; logfile << msg << endl; 
                /* Broadcast the disconnection message to the other clients */
                for (int i = 0; i < MAX_CLIENTS; i++)
                    if (client_array[i].sockfd != INVALID_SOCKET)
                        iResult = send(client_array[i].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
                close(new_client.sockfd);
                close(client_array[new_client.id].sockfd);
                /* Securely erasing the username so no conflict arises in upload-download or push-pull or shell commands */
                client_array[new_client.id].uname = "";
                client_array[new_client.id].sockfd = INVALID_SOCKET;
                thread.detach();
                return 0;
            }
        }
    } /* Loop in client thread runs until the client sends exit request */
}

int main(int argc, char** argv) {
    int sockfd, port_no, num_clients = 0, temp_id = -1;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    /* This server is cunstructed for star topology */
    vector<client_type> client(MAX_CLIENTS);
    string msg = "";
    thread client_threads[MAX_CLIENTS];
    if (argc < 2) { cerr <<  "ERROR, no port provided\n"; return -1; }
    port_no = atoi(argv[1]);
    if (port_no <= 0) { cerr <<  "Invalid port -.-\n"; return -1; }
    /* Create socket */
    cout <<  "Creating server socket...\n";
    sockfd =  socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) { cerr <<  "ERROR opening socket\n"; return -2; }
    /* Clear address structure */
    bzero((char *) &server_addr, sizeof(server_addr));
    /* Setup the host_addr structure for use in bind call */
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port_no);
    /* Bind */
    if (bind(sockfd, (struct sockaddr *) &server_addr, sizeof(server_addr)) < 0)
        { cerr <<  "ERROR on binding\n"; return -3; }
    /* Make directories and start logging */
    system("mkdir share;mkdir logs");
    logfile.open(
        string("logs/") + (to_string(port_no) + "." 
        + to_string(int(time(NULL))) + ".log").c_str(),
        ios::out | ios::ate );
    /* Listen */
    listen(sockfd, 5);
    char local_host[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(server_addr.sin_addr), local_host, INET_ADDRSTRLEN);
    cout << "Listening on "<<local_host<<":"<< port_no << endl;
    logfile << "Listening on "<<local_host<<":"<< port_no << endl;
    /* Initialize the client list */
    for (int i = 0; i < MAX_CLIENTS; i++)
        client[i] = (client_type) { -1, INVALID_SOCKET };
    client_len = sizeof(client_addr);
    /* Serve */
    while (true) {
        /* Accept new connection */
        int incoming = INVALID_SOCKET;
        incoming = accept(sockfd, (struct sockaddr *) &client_addr, &client_len);
        if (incoming == INVALID_SOCKET)
            continue;
        /* Reset the number of clients */
        num_clients = -1;
        /* Create a temporary id for the next client */
        temp_id = -1;
        for (int i = 0; i < MAX_CLIENTS; i++) {
            if (client[i].sockfd == INVALID_SOCKET && temp_id == -1) {
                client[i].sockfd = incoming;
                client[i].id = i; temp_id = i;
            } if (client[i].sockfd != INVALID_SOCKET) num_clients++;
        } if (temp_id != -1) {
            char remote_host[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
            string accmsg = string("Connection accepted from ")
                            + remote_host + ":" + to_string(ntohs(client_addr.sin_port)) 
                            + " (Client #" + to_string(client[temp_id].id) 
                            + ") accepted\n";
            cout << accmsg; logfile << accmsg;
            /* Send the id to that client */
            msg =  to_string(client[temp_id].id);
            send(client[temp_id].sockfd, msg.c_str(), strlen(msg.c_str()), 0);
            /* Create a thread process for that client */
            client_threads[temp_id] =  thread(
                process_client,
                ref(client[temp_id]),
                ref(client),
                ref(client_threads[temp_id]) );
        } else {
            /* The server is full :( */
            msg = "Server is full";
            send(incoming, msg.c_str(), strlen(msg.c_str()), 0);
            char remote_host[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(client_addr.sin_addr), remote_host, INET_ADDRSTRLEN);
            string rejmsg = string("Connection rejected to ")
                            + remote_host + ":" + to_string(ntohs(client_addr.sin_port)) 
                            + " (Client #" + to_string(client[temp_id].id) 
                            + "). The server is full!\n";
            cout << rejmsg; logfile << rejmsg;
            /* Close connection */
            close(client[temp_id].sockfd);
        }
    }
    /* Unreachable code here as a server is infinite loop as it serves forever */
}
