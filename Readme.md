*This project has been created as part of the 42 curriculum by akaung, tswe-zin, ksan.*

# ft_irc

## Description

`ft_irc` is an IRC (Internet Relay Chat) server implemented in **C++98** as part of the 42 curriculum.

The goal of this project is to build a functional IRC server capable of handling multiple clients simultaneously over TCP/IP. The server communicates with IRC clients and implements the essential IRC functionality required by the `ft_irc` project subject.

The server does **not** implement an IRC client or server-to-server communication.

The executable is called `ircserv` and accepts two arguments:

```text
./ircserv <port> <password>
```

* `port` — The TCP port on which the server listens for incoming connections.
* `password` — The password required by clients when connecting to the server.

### Main Features

The server supports:

* Multiple simultaneous clients
* TCP/IP communication
* Client authentication using a server password
* Nickname and username registration
* Joining and leaving IRC channels
* Private messages between users
* Messages sent to channels
* Channel operators and regular users
* Channel topics
* Channel invitations
* Kicking users from channels
* Channel modes
* Ping/Pong communication
* Graceful client disconnection
* Non-blocking I/O
* Single `poll()`-based event loop
* Handling of partial TCP packets and buffered IRC messages

## IRC Commands

The following IRC commands are implemented by the server.

### `PASS`

Authenticates a client using the server connection password.

```text
PASS <password>
```

Example:

```text
PASS mypassword
```

The password must be provided before the client can complete registration.

---

### `NICK`

Sets or changes a client's nickname.

```text
NICK <nickname>
```

Example:

```text
NICK alice
```

The server checks that the nickname is valid and not already being used by another client.

---

### `USER`

Registers the username and real name of a client.

```text
USER <username> 0 * :<realname>
```

Example:

```text
USER alice 0 * :Alice Smith
```

A client must successfully complete the required authentication and registration steps before being considered fully registered.

---

### `JOIN`

Joins a channel.

```text
JOIN <channel>
```

Example:

```text
JOIN #42
```

If the channel does not already exist, it can be created when the client joins it.

For password-protected or invite-only channels, the appropriate requirements must be satisfied.

---

### `PART`

Leaves a channel.

```text
PART <channel>
```

Example:

```text
PART #42
```

A client can use this command to leave a channel they have joined.

---

### `PRIVMSG`

Sends a private message to another client or sends a message to all other members of a channel.

Send a private message:

```text
PRIVMSG <nickname> :<message>
```

Example:

```text
PRIVMSG alice :Hello Alice!
```

Send a channel message:

```text
PRIVMSG <channel> :<message>
```

Example:

```text
PRIVMSG #42 :Hello everyone!
```

Messages sent to a channel are forwarded to the other clients who are members of that channel.

---

### `KICK`

Allows a channel operator to remove a user from a channel.

```text
KICK <channel> <nickname>
```

Example:

```text
KICK #42 alice
```

Only users with channel operator privileges can use this command.

---

### `INVITE`

Allows a channel operator to invite a user to a channel.

```text
INVITE <nickname> <channel>
```

Example:

```text
INVITE alice #42
```

This is particularly relevant for invite-only channels.

---

### `TOPIC`

Views or changes the topic of a channel.

View the current topic:

```text
TOPIC <channel>
```

Change the topic:

```text
TOPIC <channel> :<new topic>
```

Example:

```text
TOPIC #42 :Welcome to our channel!
```

When topic restrictions are enabled, only channel operators can change the topic.

---

### `MODE`

Changes or displays channel modes.

```text
MODE <channel> <mode> [parameter]
```

The following channel modes are supported.

#### Invite-only — `i`

Enable:

```text
MODE #42 +i
```

Disable:

```text
MODE #42 -i
```

When enabled, only invited users can join the channel.

#### Topic restriction — `t`

Enable:

```text
MODE #42 +t
```

Disable:

```text
MODE #42 -t
```

When enabled, only channel operators can change the channel topic.

#### Channel key — `k`

Set a channel password:

```text
MODE #42 +k secret
```

Remove the channel password:

```text
MODE #42 -k
```

A client must provide the correct key when joining a password-protected channel.

#### Operator privilege — `o`

Give operator privileges:

```text
MODE #42 +o alice
```

Remove operator privileges:

```text
MODE #42 -o alice
```

#### User limit — `l`

Set a maximum number of users:

```text
MODE #42 +l 10
```

Remove the user limit:

```text
MODE #42 -l
```

### Supported Mode Summary

| Mode        | Description                         |
| ----------- | ----------------------------------- |
| `+i` / `-i` | Enable/disable invite-only mode     |
| `+t` / `-t` | Restrict topic changes to operators |
| `+k` / `-k` | Set/remove channel password         |
| `+o` / `-o` | Give/remove operator privileges     |
| `+l` / `-l` | Set/remove user limit               |

---

### `PING`

Tests whether the connection to the server is still active.

```text
PING <server>
```

Example:

```text
PING 127.0.0.1
```

The server responds with a `PONG` response.

This command is also important for maintaining compatibility with IRC clients that periodically check server connectivity.

---

### `QUIT`

Disconnects a client from the IRC server.

```text
QUIT
```

A client may optionally provide a quit message:

```text
QUIT :Goodbye!
```

The server removes the client from the channels they belong to and closes their connection cleanly.

---

## Command Handler

IRC commands are processed through the `CommandHandler` class.

The command handler contains dedicated functions for the supported commands:

```cpp
void passCmd(...);
void nickCmd(...);
void userCmd(...);
void joinCmd(...);
void partCmd(...);
void privmsgCmd(...);
void kickCmd(...);
void inviteCmd(...);
void topicCmd(...);
void modeCmd(...);
void pingCmd(...);
void quitCmd(...);
```

Commands are first parsed into an `IRCMessage` and then dispatched to the appropriate command handler.

Responses are represented using the `Response` structure:

```cpp
struct Response
{
    Client* destination;
    std::string message;
};
```

This allows the server to determine which client or clients should receive each IRC response.

## Technical Overview

The project is written entirely using the **C++98 standard**.

The server uses:

* TCP sockets for network communication
* Non-blocking file descriptors
* `poll()` for monitoring server and client sockets
* A command parser for IRC messages
* Client management
* Channel management
* Buffered input handling for partial TCP packets
* IRC command and response processing

### Non-Blocking I/O

All I/O operations are performed using non-blocking file descriptors.

A single `poll()` instance is used to monitor the listening socket and connected clients.

This allows the server to handle multiple clients without blocking on any individual connection.

The server does not use `fork()` or create a process for each client.

### Partial Messages

TCP does not guarantee that a complete IRC command will arrive in a single `recv()` call.

For example, a client may send:

```text
com
man
d\n
```

instead of:

```text
command\n
```

The server therefore buffers received data and reconstructs complete IRC commands before processing them.

This is important because IRC commands must be processed only after a complete message has been received.

### Event Handling

The server uses `poll()` as the main event-monitoring mechanism.

The same polling mechanism is responsible for monitoring:

* The listening socket
* Client read events
* Client write events
* New connections
* Client disconnections

This provides an event-driven architecture capable of handling multiple simultaneous connections.

## Instructions

### Requirements

A Unix-like environment with a C++ compiler supporting C++98 is required.

The project is designed to comply with the 42 `ft_irc` subject and uses the permitted system and networking functions.

### Compilation

Build the project using:

```bash
make
```

The Makefile provides the following targets:

```bash
make
make all
make clean
make fclean
make re
```

* `make` / `make all` — Compile the server.
* `make clean` — Remove object and dependency files.
* `make fclean` — Remove object files, dependency files and the executable.
* `make re` — Clean and rebuild the project.

After compilation, the executable is:

```text
ircserv
```

### Running the Server

Start the server by providing a port and connection password:

```bash
./ircserv 6667 password
```

For example:

```bash
./ircserv 6667 mypassword
```

The server will then listen for incoming TCP connections on port `6667`.

### Connecting With an IRC Client

A compatible IRC client can be used to connect to the server.

Example connection settings:

```text
Server: 127.0.0.1
Port: 6667
Password: mypassword
```

After connecting, the client should be able to authenticate, register a nickname and username, join channels, communicate with other users and use the supported channel operator commands.

### Testing With Netcat

A basic TCP connection can be tested using:

```bash
nc -C 127.0.0.1 6667
```

Commands can then be entered manually.

For example:

```text
PASS mypassword
NICK alice
USER alice 0 * :Alice
JOIN #42
PRIVMSG #42 :Hello everyone!
```

Testing partial TCP packets is also important.

For example:

```text
com
man
d\n
```

The server must aggregate the received data and reconstruct:

```text
command
```

before attempting to process it.

## Project Structure

The project is organized into header files under `includes/`, source files under `srcs/`, and compiled object/dependency files under `build/`.

```text
.
├── Makefile
├── README.md
├── includes/
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   ├── IRCMessage.hpp
│   └── CommandHandler.hpp
├── srcs/
│   ├── main.cpp
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── IRCMessage.cpp
│   └── CommandHandler.cpp
└── build/
    ├── main.o
    ├── Server.o
    ├── Client.o
    ├── Channel.o
    ├── IRCMessage.o
    ├── CommandHandler.o
    └── ...
```

### Main Components

#### `Server`

Responsible for:

* Creating the listening socket
* Configuring the server socket
* Binding and listening
* Accepting new clients
* Managing connected clients
* Running the `poll()` event loop
* Receiving and sending data
* Managing server-wide state

#### `Client`

Responsible for storing information about an individual connection, including:

* Socket file descriptor
* Nickname
* Username
* Authentication state
* Registration state
* Input buffer
* Output data
* Channel memberships

#### `Channel`

Responsible for channel-related information, including:

* Channel name
* Topic
* Channel members
* Channel operators
* Invite-only state
* Topic restrictions
* Channel key
* User limit

#### `IRCMessage`

Responsible for representing and parsing IRC messages into their relevant components, such as:

* Command
* Parameters
* Trailing message
* Parsed IRC data

#### `CommandHandler`

Responsible for dispatching and executing IRC commands.

Supported handlers include:

```text
PASS
NICK
USER
JOIN
PART
PRIVMSG
KICK
INVITE
TOPIC
MODE
PING
QUIT
```

## C++ Header Protection

The project uses `#pragma once` in header files to prevent the same header from being included multiple times during compilation.

For example:

```cpp
#pragma once

#include "Server.hpp"
#include "Client.hpp"
#include "IRCMessage.hpp"
```

This prevents duplicate declarations and reduces unnecessary repeated header processing.

## Technical Choices

### C++98

The project is implemented using C++98 because this is the required standard for the 42 `ft_irc` project.

Modern C++ features introduced in C++11 and later are therefore not relied upon.

### TCP/IP

The server communicates with IRC clients using TCP/IP sockets.

TCP provides reliable and ordered delivery, which is appropriate for IRC communication.

### `poll()`

`poll()` is used as the main event mechanism.

It allows the server to monitor:

* The listening socket
* Client sockets ready for reading
* Client sockets ready for writing
* Connection and disconnection events

A single polling mechanism is used to handle all network I/O.

### Non-Blocking Sockets

Client and server sockets are configured for non-blocking operation.

This prevents one client from blocking the entire server while waiting for network input or output.

### Client and Channel Management

Clients and channels are represented as separate objects.

A client stores information such as:

* Socket file descriptor
* Nickname
* Username
* Authentication state
* Registration state
* Input buffer
* Output buffer
* Channel memberships

A channel stores information such as:

* Channel name
* Topic
* Members
* Operators
* Channel modes
* Channel key
* User limit
* Invitation information

This separation makes it easier to manage relationships between users and channels.

## Error Handling

The server handles errors and invalid situations including:

* Invalid command arguments
* Invalid passwords
* Duplicate nicknames
* Invalid nicknames
* Invalid channels
* Unauthorized channel operations
* Invalid channel modes
* Missing command parameters
* Failed socket operations
* Client disconnections
* Partial network messages
* Unexpected client input
* Attempts to access channels without the required permissions

The server should remain operational when an individual client disconnects or sends invalid input.

## Resources

The following resources were useful for understanding the concepts required for this project.

### IRC Protocol

* **RFC 2812** — Internet Relay Chat: Client Protocol
* **RFC 2811** — Internet Relay Chat: Channel Management
* **RFC 1459** — Internet Relay Chat Protocol

These RFCs provide information about IRC commands, messages, channels, users and client/server communication.

### C++ Documentation

* C++98 language and standard library documentation
* C++ socket programming references
* Unix/Linux system call documentation

### Network Programming

The following system calls and concepts were particularly relevant:

```text
socket()
setsockopt()
getsockname()
getprotobyname()
gethostbyname()
getaddrinfo()
freeaddrinfo()
bind()
connect()
listen()
accept()
htons()
htonl()
ntohs()
ntohl()
inet_addr()
inet_ntoa()
inet_ntop()
send()
recv()
close()
signal()
sigaction()
sigemptyset()
sigfillset()
sigaddset()
sigdelset()
sigismember()
lseek()
fstat()
fcntl()
poll()
```

Understanding TCP connections, file descriptors, non-blocking I/O, socket programming and event-driven programming was essential to implementing the server.

### Testing

`netcat` (`nc`) was useful for manually testing TCP connections and sending raw IRC commands to the server.

Testing with an actual IRC client was also important because IRC clients expect the server to follow the IRC protocol and provide the appropriate responses.

Testing should also include:

* Multiple simultaneous clients
* Client disconnections
* Invalid commands
* Invalid passwords
* Duplicate nicknames
* Partial TCP packets
* Large messages
* Low-bandwidth situations
* Channel permissions
* Channel passwords
* Invite-only channels
* User limits
* Operator privileges
* Topic restrictions

## IRC Commands & Usage

The server supports the following IRC commands:

### Client Commands

```text
PASS <password>
NICK <nickname>
USER <username> 0 * :<realname>
JOIN <#channel> [key]
PART <#channel>
PRIVMSG <nickname|#channel> :<message>
PING <server>
QUIT [:message]
```

Examples:

```text
PASS password
NICK alice
USER alice 0 * :Alice
JOIN #42
PRIVMSG #42 :Hello everyone!
PRIVMSG bob :Hello Bob!
PART #42
PING 127.0.0.1
QUIT :Goodbye!
```

### Operator Commands

```text
KICK <#channel> <nickname>
INVITE <nickname> <#channel>
TOPIC <#channel> [:topic]
```

Examples:

```text
KICK #42 bob
INVITE bob #42
TOPIC #42 :Welcome to our channel!
```

### Channel Modes

```text
MODE <#channel> +i
MODE <#channel> -i
MODE <#channel> +t
MODE <#channel> -t
MODE <#channel> +k <key>
MODE <#channel> -k
MODE <#channel> +o <nickname>
MODE <#channel> -o <nickname>
MODE <#channel> +l <limit>
MODE <#channel> -l
```

| Mode | Description                 |
| ---- | --------------------------- |
| `i`  | Invite-only channel         |
| `t`  | Operator-only topic changes |
| `k`  | Channel password            |
| `o`  | Operator privilege          |
| `l`  | User limit                  |

### Quick Session

```text
PASS password
NICK alice
USER alice 0 * :Alice
JOIN #42
TOPIC #42 :42 IRC
PRIVMSG #42 :Hello!
MODE #42 +i
INVITE bob #42
QUIT
```

## AI Usage

AI tools were used as a **development assistance and learning resource**, not as a replacement for understanding or testing the project.

AI assistance was used for tasks such as:

* Explaining C++98 concepts and syntax
* Clarifying socket programming and TCP/IP concepts
* Understanding `poll()` and non-blocking I/O
* Understanding IRC protocol concepts and command formats
* Helping identify potential edge cases
* Reviewing code structure and suggesting improvements
* Helping debug compilation errors and runtime issues
* Improving documentation and README organization
* Suggesting testing scenarios for partial messages, client disconnections and invalid commands
* Explaining compiler and Makefile options when needed

The final implementation was reviewed, adapted and tested by the project members. AI-generated suggestions were not blindly copied into the project; they were used to support understanding, debugging and development.

## Team

This project was developed as part of the 42 curriculum by:

* **akaung**
* **tswe-zin**
* **ksan**

## License

This project was created for educational purposes as part of the 42 curriculum.
