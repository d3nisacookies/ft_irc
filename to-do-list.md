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



Here’s a clean **ft_irc evaluation checklist**, organized so you can work through it systematically.

## 1. Build / Compilation — Required

- [x]  Enable `Werror` in the Makefile.
- [ ]  Fix `removeChannel(Channel &channel)` unused parameter warning in `Server.cpp:260`.
    - [ ]  Either implement the function so `channel` is used, or explicitly handle the unused parameter appropriately.
- [x]  Fix `Wreorder` in `Channel.hpp:29`.
    - [x]  Make the constructor initializer-list order exactly match the order of member declarations.
- [x]  Run a clean build:
    - [x]  `make fclean`
    - [x]  `make`
    - [x]  Confirm compilation succeeds with `Wall -Wextra -Werror`.

---

## 2. Registration / **451 Gate** — Critical

Every command that requires registration must be rejected until the client is properly registered.

### Before registration

Test each command from a fresh connection:

- [x]  `JOIN #pre` → **451**
- [x]  `PART #pre` → **451**
- [x]  `PRIVMSG ...` → **451**
- [ ]  `NOTICE ...` → **451**
- [x]  `MODE ...` → **451**
- [x]  `TOPIC ...` → **451**
- [x]  `INVITE ...` → **451**
- [x]  `JOIN #pre` must **not create the channel** before registration.
- [x]  A client without a nickname must never become a channel member.

### Partial registration

- [x]  `PASS password`
- [x]  `NICK test`
- [x]  Without `USER`, `JOIN #room` → **451**

Likewise:

- [x]  `PASS password`
- [x]  `USER ...`
- [x]  Without `NICK`, registration-dependent commands → **451**

### PASS after registration

- [x]  Register successfully with `PASS + NICK + USER`.
- [x]  Send `PASS` again.
- [x]  Expected numeric: **462 ERR_ALREADYREGISTRED**
- [x]  Must **not** return `202 Password accepted`.

### PASS before NICK

- [x]  Send invalid `PASS`.
- [x]  Expected **464 ERR_PASSWDMISMATCH**
- [x]  Expected format should include  as the target:
    - [x]  `464 * :...`
- [x]  Must **not** produce:
    - [x]  `464 :...`

### Registration state

Verify your server distinguishes:

- [ ]  Connected
- [ ]  PASS received
- [ ]  NICK received
- [ ]  USER received
- [ ]  Fully registered

And only allows registered users through the **451 gate**.

---

## 3. Missing Commands — Required

### Unknown command

- [x]  Send something like:
    - `FOOBAR`
- [x]  Server responds with **421 ERR_UNKNOWNCOMMAND**
- [x]  Connection remains usable afterward.

### PING

- [ ]  `PING :hello`
- [ ]  Server responds with:
    - [ ]  `PONG`
    - [ ]  Correct server/source/argument format as required by your subject/tester.
- [ ]  Connection stays alive.
- [ ]  Test with `irssi`.
- [ ]  Confirm irssi does not disconnect because of missing PONG.

### QUIT

- [ ]  `QUIT`
- [ ]  Server sends any required quit/error response according to your implementation/spec.
- [ ]  Connection is actually closed.
- [ ]  Client is removed from the server's client list.
- [ ]  Client is removed from any channels.
- [ ]  Other channel members receive the appropriate `QUIT` message.
- [ ]  No dangling channel membership remains.

---

## 4. Numeric Codes — Required Corrections

### PRIVMSG

#### Missing target

- [x]  `PRIVMSG`
- [x]  Expected **411 ERR_NORECIPIENT**
- [x]  Not 461.

#### Missing message text

Example:

- [x]  `PRIVMSG nick`
- [x]  Expected **412 ERR_NOTEXT**
- [x]  Not 461.

#### User not in channel

Example:

- [x]  `PRIVMSG #room :hello`
- [x]  If sender is not a member of `#room`:
- [x]  Expected **404 ERR_CANNOTSENDTOCHAN**
- [x]  Not 442.

---

### JOIN

#### Invalid channel name

Example:

- [ ]  `JOIN room`
- [ ]  Do not return generic **461**.
- [ ]  Use the appropriate channel-name error expected by your subject/tester:
    - [ ]  **403 ERR_NOSUCHCHANNEL**, or
    - [ ]  **476 ERR_BADCHANMASK**, depending on your implementation/spec.

Also test:

- [ ]  `JOIN #room`
- [ ]  `JOIN ##room`
- [ ]  `JOIN #`
- [ ]  `JOIN #room, #other`
- [ ]  `JOIN` with no parameters → **461**

---

## 5. Nickname Case Sensitivity

IRC nicknames should be treated case-insensitively for uniqueness.

Test:

1. Client A:
    - [ ]  `NICK taken`
    - [ ]  succeeds
2. Client B:
    - [ ]  `NICK TAKEN`
    - [ ]  must be rejected as already in use.

Also test:

- [ ]  `taken`
- [ ]  `TAKEN`
- [ ]  `Taken`
- [ ]  `tAkEn`

All should collide.

---

# Final Regression Checklist

After fixing everything, run this sequence from **fresh connections**:

### Build

- [ ]  `make fclean`
- [ ]  `make`
- [ ]  `Wall -Wextra -Werror`
- [ ]  Zero warnings/errors.

### Registration

- [ ]  `JOIN` before registration → **451**
- [ ]  `PART` before registration → **451**
- [ ]  `PRIVMSG` before registration → **451**
- [ ]  `NOTICE` before registration → **451**
- [ ]  `MODE` before registration → **451**
- [ ]  `TOPIC` before registration → **451**
- [ ]  `INVITE` before registration → **451**
- [ ]  `PASS + NICK`, no USER → commands still → **451**
- [ ]  `PASS + USER`, no NICK → commands still → **451**
- [ ]  `PASS` after registration → **462**
- [ ]  Wrong PASS → **464 * :...**
- [ ]  No pre-registration channel creation.

### Core protocol

- [ ]  Unknown command → **421**
- [ ]  `PING` → **PONG**
- [ ]  `QUIT` → connection closes
- [ ]  QUIT removes user from channels.

### PRIVMSG

- [ ]  No target → **411**
- [ ]  No text → **412**
- [ ]  Not in channel → **404**
- [ ]  Valid user-to-user message works.
- [ ]  Valid channel message works.

### JOIN

- [ ]  `JOIN #room` works.
- [ ]  Invalid channel name → **403/476**
- [ ]  Missing parameter → **461**
- [ ]  Joining creates membership only after successful registration.
- [ ]  JOIN broadcast is correct.

### Nicknames

- [ ]  `taken` + `TAKEN` → collision.
- [ ]  Case-insensitive nickname lookup.
- [ ]  Nick change updates all relevant state.
- [ ]  Nick change is broadcast correctly.
- [ ]  Nickname validation rejects invalid names.

### Final sanity

- [ ]  Test with `nc`.
- [ ]  Test with `irssi`.
- [ ]  Test multiple simultaneous clients.
- [ ]  Test disconnects unexpectedly.
- [ ]  Test JOIN/PART/QUIT cleanup.
- [ ]  Test commands after errors—the connection should remain usable.
- [ ]  Check for crashes, leaks, dangling clients, and stale channel members.

**Priority order:** fix **Build → 451 registration gate → PING/QUIT/421 → numeric codes → case-insensitive NICK → full regression**.