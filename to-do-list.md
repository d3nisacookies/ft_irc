# ft_irc Notes — 26 Sep

**To:** AK

## TODO

* Handle an empty `""` command in the parser.
* Clear `_params` before parsing a new message.

  * Currently `_params` keeps accumulating because `push_back()` is used.
* Client registration/status still needs to be handled.

  * Current registration information is tracked using:

    * `hasUsername()`
    * `hasNickname()`
    * `isPassVerified()`
  * Eventually this should be coordinated with `Client::AuthStatus`.
* Some functions are declared in `Server.hpp` but have not been implemented in `Server.cpp` yet.

  * Ksan needs to add those implementations.

## Registration Commands

```text
PASS password
NICK nickname
USER username hostname servername :realname
```

### Parameters

* `PASS` → 1 parameter
* `NICK` → 1 parameter
* `USER` → 4 parameters

  * `[0]` → username
  * `[1]` → hostname
  * `[2]` → servername
  * `[3]` → realname

## IRC Commands

### JOIN

```text
JOIN #channel
```

* `[0]` → channel name
* Creates the channel if it doesn't exist.
* Adds the client to the channel.
* First client becomes channel operator.

### PART

```text
PART #channel
PART #channel :reason
```

* `[0]` → channel name
* `[1]` → optional reason
* Client leaves the channel.

### PRIVMSG

```text
PRIVMSG nickname :message
PRIVMSG #channel :message
```

* `[0]` → target nickname or channel
* `[1]` → message
* Sends a private message to a client or a message to a channel.

### KICK

```text
KICK #channel nickname
KICK #channel nickname :reason
```

* `[0]` → channel name
* `[1]` → nickname of client to remove
* `[2]` → optional reason
* Removes a client from a channel.
* Requires appropriate channel privileges.

### INVITE

```text
INVITE nickname #channel
```

* `[0]` → nickname to invite
* `[1]` → channel name
* Invites a client to a channel.
* Used with invite-only channels.

### TOPIC

```text
TOPIC #channel
TOPIC #channel :new topic
```

* `[0]` → channel name
* `[1]` → optional new topic
* Without a topic parameter → view current topic.
* With a topic parameter → change the topic.

### MODE

```text
MODE #channel
MODE #channel +i
MODE #channel -i
MODE #channel +t
MODE #channel -t
MODE #channel +k password
MODE #channel -k
MODE #channel +o nickname
MODE #channel -o nickname
MODE #channel +l limit
MODE #channel -l
```

Common channel modes for `ft_irc`:

* `i` → invite-only
* `t` → only operators can change the topic
* `k` → channel password/key
* `o` → give/remove operator privileges
* `l` → user limit

## Responsibility Between Parser and CommandHandler

The parser's main responsibility is to turn the raw IRC message into:

* command
* parameters

The `CommandHandler` validates the command-specific parameters.

For example:

```text
PASS
```

can be parsed first, and then `passCmd()` checks that it has exactly one non-empty parameter.

The parser does not need to perform every command-specific validation.

**Note:** `USER`'s fourth parameter is the realname. Since it can contain spaces, the parser still needs to correctly handle the IRC trailing-parameter syntax (`:`).
