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
./rvc-tcp-client (ip address of server) (port no) --permit-shell-access ---preset-uname (your username)
```
where --permit-shell-access flag is optional enabling any user to execute shell commands on your PC 
and --preset-uname sets username without stdin in commandline itself it is also optional

If you want to execute client as hidden execute two commands

```
./rvc-tcp-client (ip address of server) (port no) --permit-shell-access ---preset-uname (your username) < /dev/null & > /dev/null &
disown
```
In Linux, /dev/null is a special device le which writes-off (gets rid of) all data written to it, in the command above, input is read
from, and output is sent to /dev/null.
after "disown" you can even close the client terminal but the program continues to interact with server at the background 
as the terminal loses the authority over the application it will keep running until server sends --getout-- bash to the client
This enables hidden PC remote control

## Operation Screenshots

Normal Chatting
![chats-shell.png](docs/chats-shell.png)

Upload and Download
![upload-download.png](docs/upload-download.png)

Push Pull request
![push-pull.png](docs/push-pull.png)

Remote Shutdown
![remote-shutdown.png](docs/remote-shutdown.png)

## Author

* **Rajas Chavadekar** 

## License

This project is licensed under the MIT License - see the [LICENSE](LICENSE) file for details

