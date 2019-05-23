# Rvc TCP Station

An all in one TCP server client system in Linux
1) Multi user TCP chat
2) RSH (Remote Shell control)
3) FTP (File transfer protocol)

## Internal Architecture

![architecture.png](docs/architecture.png)

## Getting Started

These instructions will get you a copy of the project up and running on your local machine for development and testing purposes. See deployment for notes on how to deploy the project on a live system.

### Prerequisites

A good Linux OS
It is a native C++ application so basic C++ libraries 
and multi threading support (pthread) for compilation, basic g++ compiler

### Compilation

It has a make script so in terminal so just run

```
make
```
This will create a zip with two executables "rvc-tcp-client" and "rvc-tcp-server"
Copy this zip on different computers and use client and server accordingly

## Deployment

On the terminal of PC you have decided to keep as server run rvc-tcp-server

```
./rvc-tcp-server (port no)
```

On any PC where you have to open the client,

```
./rvc-tcp-client (ip address of server) (port no) --permit-shell-access --preset-uname (client username)
```
where --permit-shell-access flag is optional enabling any user to execute shell commands on your PC 
and --preset-uname sets username without stdin in commandline itself it is also optional

If you want to execute client as hidden execute the shell script

```
./rvc-tcp-client-nude (ip address of server) (port no) (client username)
```
on success it wont echo anything so please dont be afraid... any error message will be displayed if error exists
after that you can close the terminal 

INSIDE THE SCRIPT:

In this script all the stdout of the rvc-tcp-client is sent to /dev/null
In Linux, /dev/null is a special device le which writes-off (gets rid of) all data written to it, in the command above, output (stdout) 
is sent to /dev/null. The --no-stdin flag tells the client not to take any input message instead to execute the server
listening process in main thread itself instead of creating a separate thread for that. this fixes the cpu utilization for waiting 
for user input (stdin) and just follows the server... This flag is mandatory else the stdin (tty input) process suspends
and whole system goes in race on exiting the terminal.
after "disown" you can even close the client terminal but the program continues to interact with server at the background 
as the terminal loses the authority over the application it will keep running until server sends --getout-- bash to the client
This enables hidden PC remote control

## Instructions

1) Just type the message and press enter to send your message.
2) To upload a file first copy it in your 'uploads folder created by app' and 
   then in your chats send command '--upload-- yourfilename' to server to send
   it on group.. everyone in that group will be able to download that file.
3) To download a file in the group send the command '--download-- filename' to
   server and after successful download it will appear in your 
   'downloads' folder. Please accurately mention the filename. 
4) To execute a scell command on a user who permits one's shell access , use
   '--shell-- @[username |--all--] --bash-- [actual commandline to execute]'
   in the actual command if you give '--getout--' it will just terminate 
   the client connection. If it fails you will get some error message. 
5) The --pull-- command delibrately uploads a file from client side onto the 
   server. It will command the client program to upload prescribed file. 
   '--pull-- @[username] --file-- [accurate filename]' is its syntax. 
6) Similar is with --push-- command but the opposite action to command the 
   client to download the prescribed file from server. 
   '--push-- @[username |--all--] --file-- [accurate filename]' is its syntax.
7) To exit the room just send ' --exit-- ' to the server so it will
   terminate your connection.

![commands.png](docs/commands.png)

## Operation Screenshots

Normal Chatting
![chats-shell.png](docs/chats-shell.png)

Upload and Download
![upload-download.png](docs/upload-download.png)

Push Pull request
![push-pull.png](docs/push-pull.png)

Hidden Remote control
![hidden-control.png](docs/hidden-control.png)

Remote Shutdown
![remote-shutdown.png](docs/remote-shutdown.png)

## Author

* **Rajas Chavadekar** 

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

