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
The clients can connect via your hostname so you can give a unique host name
by running the following command as Root:
```
hostname (your custom hostname)
```
However its will restore to defaults if server reboots.
To show your current hostname just run above command with no arguments.

On any PC where you have to open the client,

```
./rvc-tcp-client (hostname/ip address of server) (port no) -reconnect --permit-shell-access --preset-uname (client username)
```
where --permit-shell-access flag is optional enabling any user to execute shell commands on your PC 
and --preset-uname sets username without stdin in commandline itself it is also optional
--reconnect will make server connection automatically after 60 seconds even on exit.
 The order of flags donot matter at all .. application will understand it
but specify the username after --preset-uname flag if given.

For some special options and if you feel the overall command syntax confusing,
We have an intuitive and interactive script that will launch the client with
above parameters automatically. For that run

```
./rvc-tcp-station
```
One can also set the client as a startup service via this script 
to uninstall the client from startup service (if exist) just send -u flag to this script
to do it in single commandline itself
```
./rvc-tcp-station -u
```
If you want to completely and remotely uninstall the client from remote pc, on RSH of victim in your client program you can send command
```
--shell-- @(victim uname) --bash-- init; (path to the script)/rvc-tcp-station -u
```
So 'init' will cause the connection to terminate and immediately the uninstall is carried out
You will never need to fear what if other user shuts down without leaving the group, will the threads 
be damaged? will the network get destroyed? absolutely no as we have caught SIGTERM at the client side and 
have gracefully closed the ongoing connection with proper --exit-- request to server. SIGTERM is sent by OS to 
running applications to give them a chance to execute cleanup and their own exit routines .. we have just made use 
of it. So by these all features of client a hidden remote control of deploy and forget type can be implemented. 

How is hidden client implemented:

In this script all the stdout of the rvc-tcp-client is sent to /dev/null
In Linux, /dev/null is a special device le which writes-off (gets rid of) all data written to it, in the command above, output (stdout) 
is sent to /dev/null. The --no-stdin flag tells the client not to take any input message instead to execute the server
listening process in main thread itself instead of creating a separate thread for that. this fixes the cpu utilization for waiting 
for user input (stdin) and just follows the server... This flag is mandatory else the stdin (tty input) process suspends
and whole system goes in race on exiting the terminal.
after "disown" you can even close the client terminal but the program continues to interact with server at the background 
as the terminal loses the authority over the application it will keep running until server sends --getout-- bash to the client
This enables hidden PC remote control.
Atleast for now, server does not have this feature as it may be serving multiple clients and
exit request is from client side. To do this we have to first close every other client connnected to it that too with proper exit
requests and then leaving the port ... One can contribute in adding this feature however. 

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
7) To view all online users type command '--anyonehere--'. You will get a list
   of all available users with their ip address.
8) To exit the room just send ' --exit-- ' to the server so it will
   terminate your connection.

![commands.png](docs/commands.png)

## Operation Screenshots

Launcher Script
![launcher.png](docs/launcher.png)

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

