# ircserv — Progress Notes

42 project: an IRC server in C++98 (`./ircserv <port> <password>`). No IRC client, no server-to-server communication.

## Required command set (decided)

**Registration:** `PASS`, `NICK`, `USER`
**Messaging:** `PRIVMSG` (optionally `NOTICE`)
**Channel membership:** `JOIN`, `PART`, `INVITE`
**Channel management:** `TOPIC`, `KICK`, `MODE`
**Connection health:** `PING`, `PONG`, `QUIT`

Explicitly out of scope (server-to-server / service-related, not required): `OPER`, `SQUIT`, `SERVICE`, `SQUERY`.

## Channel modes

| Mode | Meaning | Storage |
|------|---------|---------|
| `i`  | invite-only | `bool` |
| `t`  | topic protected (only ops can change topic) | `bool` |
| `k`  | channel key/password | `bool hasKey` + `std::string key` |
| `o`  | channel operator | per-member, not a single channel bool |
| `l`  | user limit | `bool hasLimit` + `int limit` |

- First client to `JOIN` a nonexistent channel becomes its operator automatically.
- Existing ops can grant/revoke op status with `MODE #chan +o/-o <nick>`.
- Invite tracking (`+i`) lives on the `Channel` (not the `Client`), since `JOIN`'s handler already holds the `Channel` object and would otherwise need an extra lookup.

## Class design

- `Client.hpp`, `Channel.hpp`, `Server.hpp` — one class per header (each with a matching `.cpp`).
- `Client` and `Channel` reference each other (bidirectional: each `Channel` holds its member `Client*`s, each `Client` holds the `Channel*`s it's in) — needed for both fast broadcast (channel → clients) and fast cleanup on disconnect (client → channels).
- Circular dependency between the two headers resolved with forward declarations (`class Channel;` in `Client.hpp`, `class Client;` in `Channel.hpp`); full definitions are only `#include`d in the `.cpp` files where methods are actually called.
- `Server` owns the canonical `Client`/`Channel` objects (e.g. in `std::map`s); `Client`/`Channel` hold non-owning pointers to each other.
- IRC message prefix format: `nick!user@host` — host comes from the `sockaddr_in` that `accept()` fills in directly (no `getpeername()`, since it's not on the allowed function list), converted to a string with `inet_ntoa`/`inet_ntop`.

## Makefile

Standard 42-style Makefile: `NAME` = `ircserv`, `-std=c++98 -Wall -Wextra -Werror`, separate `srcs/` and `includes/` directories (`-Iincludes`), object files built into `build/` with automatic header dependency tracking (`-MMD -MP`), `clean`/`fclean`/`re` targets.

## Networking scaffold (working, throwaway single-client version)

Built and compiled clean, confirmed working end-to-end with `nc`:

```
socket(AF_INET, SOCK_STREAM, 0)
  → bind() to INADDR_ANY on a fixed test port, with sockaddr cast to sockaddr*
  → listen(server_fd, 128)
  → accept() into a client_addr/client_len pair, returns a new client_fd
```

Each syscall checked for `-1` and errors reported. This is scaffolding to build comfort with the raw calls — will be rewritten once `poll()` and the real classes come in.

## Next steps

1. Swap the blocking `accept()` for a `poll()` loop watching just the listening socket, then generalize to watch all connected client fds at once.
2. Bring in `Client` with an fd, host string, and a per-client input buffer that splits incoming bytes on `\r\n` (handling partial/combined messages from a single `recv()`).
3. Wire up `PASS` → `NICK` → `USER` registration end to end, tested by hand with `nc`.
4. Bring in `Channel` and implement `JOIN`/`PRIVMSG`/`PART`, then the operator commands (`KICK`, `INVITE`, `TOPIC`, `MODE`), one at a time, tested against a real client (e.g. irssi).

## Environment notes

- Editing over WSL Ubuntu on Windows 11 — needed the WSL Remote extension installed and the folder reopened via "WSL: Reopen Folder in WSL" to get IntelliSense off Win32 mode.
