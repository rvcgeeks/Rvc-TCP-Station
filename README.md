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
./rvc-tcp-server <port no>
```

On any PC where you have to open the client,

```
./rvc-tcp-client <ip address of server> <port no> --permit-shell-access
```
where --permit-shell-access flag i soptional enabling any user to execute shell commands on your PC 

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

