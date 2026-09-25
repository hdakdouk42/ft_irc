# ft_irc

An IRC server written in C++ 98, as specified by the 42 `ft_irc` subject
(mandatory part only, no bonus).

## Build and run

```bash
make
./ircserv <port> <password>
```

Example:

```bash
./ircserv 6667 hunter2
```

Makefile rules: `all`, `clean`, `fclean`, `re`.
Compiled with `c++ -Wall -Wextra -Werror -std=c++98`.

## Connecting

The reference client for this project is **irssi**. It was used to verify the
whole flow: connection, registration, joining, channel messages, private
messages, topic and the operator commands.

```
irssi
/connect 127.0.0.1 6667 hunter2
/join #test
```

Or in a single command:

```bash
irssi --connect=127.0.0.1 --port=6667 --password=hunter2
```

With netcat, useful to test raw protocol behaviour:

```bash
nc -C 127.0.0.1 6667
PASS hunter2
NICK bob
USER bob 0 * :Bob
JOIN #test
```

## Directory layout

```
.
├── Makefile
├── include/
│   ├── Channel.hpp     channel state and the i/t/k/o/l modes
│   ├── Client.hpp      one socket: identity, state, read and write buffers
│   ├── Message.hpp     parsed IRC line + the parser prototype
│   ├── Replies.hpp     every numeric reply, as macros
│   ├── Server.hpp      the server itself
│   └── Utils.hpp       split, IRC case mapping, validation
└── src/
    ├── main.cpp            argument checking, starts the server
    ├── Server.cpp          socket setup, the poll() loop, read and write
    ├── ServerHelpers.cpp   lookups, channels, disconnection, dispatch
    ├── Client.cpp
    ├── Channel.cpp
    ├── Message.cpp
    ├── Utils.cpp
    └── commands/           one file per command
        ├── Cap.cpp     Invite.cpp   Join.cpp   Kick.cpp
        ├── Mode.cpp    Nick.cpp     Part.cpp   Pass.cpp
        ├── Ping.cpp    Privmsg.cpp  Quit.cpp   Topic.cpp
        └── User.cpp
```

## How the I/O works

There is exactly one `poll()` in the whole program, and it drives everything:
accepting, reading, and writing.

* Every file descriptor is set to `O_NONBLOCK` right after `socket()` or
  `accept()`.
* `recv()` is called **only** when `poll()` reported `POLLIN` on that fd.
* `send()` is called **only** when `poll()` reported `POLLOUT` on that fd.
* `POLLOUT` is requested for a client only while it still has bytes waiting,
  so the loop never spins on writable sockets that have nothing to send.

Each client owns two buffers:

* **read buffer** — bytes are appended as they arrive and only complete
  `\n` terminated lines are executed. A command split over several packets
  (`com`, then `man`, then `d\n`) is rebuilt correctly, and several commands
  arriving in a single packet are all executed.
* **write buffer** — replies are appended here, never written directly.
  A partial `send()` is normal: what is left stays in the buffer and is
  finished on the next `POLLOUT`.

A client is closed through one of two states, which is why a client that
sends a wrong password still receives the `464` before the link drops:

* `CLOSING` — flush what is left, then close.
* `DEAD` — the peer is already gone, close immediately.

## Registration

`PASS` must come first, then `NICK` and `USER` in any order. Once all three
are present the client receives `001` to `004`. If `NICK` and `USER` are
complete but no valid password was given, the client is rejected with `464`.

`CAP LS` is answered with an empty capability list and `CAP REQ` with a NAK.
This server supports no capability, but modern clients wait for that answer
before sending `PASS`, so the handshake would otherwise stall.

## Implemented commands

| Command | Notes |
|---|---|
| `PASS` | connection password |
| `NICK` | validated, unique (IRC case mapping: `Bob` == `bob`) |
| `USER` | username and real name |
| `QUIT` | announced to every channel the client was on |
| `PING` / `PONG` | keeps the link alive |
| `CAP` | negotiation stub, answers with no capability |
| `JOIN` | honours `+i`, `+k`, `+l`; first joiner becomes operator |
| `PART` | empty channels are destroyed |
| `PRIVMSG` | to a channel or to a nickname |
| `NOTICE` | same as `PRIVMSG` but never generates an error reply |
| `KICK` | operator only |
| `INVITE` | operator only |
| `TOPIC` | view always, change restricted when `+t` |
| `MODE` | `i`, `t`, `k`, `o`, `l` |

### Channel modes

| Mode | Effect |
|---|---|
| `+i` / `-i` | invite only |
| `+t` / `-t` | only operators may change the topic |
| `+k <key>` / `-k` | channel key (password) |
| `+o <nick>` / `-o <nick>` | give or take operator privilege |
| `+l <n>` / `-l` | user limit |

Note on `INVITE`: the subject lists it under the commands *specific to
channel operators*, so this server requires operator privileges for it.
RFC 2812 is looser and only requires them on a `+i` channel.

## Evaluation checklist

The points the evaluation sheet fails a project on outright, and where they
are satisfied:

| Requirement | Where |
|---|---|
| Exactly **one** `poll()` in the whole project | `src/Server.cpp`, in `Server::run()` — it is the only call site |
| `poll()` called before every `accept` / `recv` / `send` | `accept` only under `POLLIN` on the listening socket, `recv` only under `POLLIN`, `send` only under `POLLOUT` |
| **`errno` never used** to trigger an action | `errno` does not appear anywhere in the code. Nothing ever retries a read or a write outside of `poll()` |
| `fcntl()` only as `fcntl(fd, F_SETFL, O_NONBLOCK)` | two call sites, `Server::setupSocket()` and `Server::acceptClient()`, both exactly that form |
| Compiles with `-Wall -Wextra -Werror` in C++ 98 | also clean with `-pedantic` |
| No fork, no threads | none used |
| No memory leaks | valgrind: all heap blocks freed, 0 errors, no descriptor left open |
| No unexpected termination | the only `throw`s are in `setupSocket()`, at start-up, before any client exists (e.g. the port is already taken). Nothing throws while serving |

If an evaluator asks why `poll()` returning `-1` is handled with a plain
`continue`: with a valid descriptor set the only realistic cause is a signal
interrupting the call, so the set is rebuilt and `poll()` is called again.
Checking `errno` there was avoided on purpose, and a shutdown signal is
picked up by the loop condition.

### Behaviour under the "network specials" tests

* **Partial command** — held in the client's read buffer until a `\n`
  arrives; other connections are untouched in the meantime.
* **Client killed unexpectedly** — `recv` returning 0, or `POLLHUP`/`POLLERR`,
  is treated as a `QUIT`, announced to the client's channels, and the
  descriptor is closed.
* **`nc` killed with half a command sent** — the incomplete buffer dies with
  the client, nothing is executed, no state is left behind.
* **Client stopped with `^Z`, channel flooded, client resumed** — the server
  never blocks: what cannot be written stays in that client's write buffer,
  `POLLOUT` is requested for it, and the backlog is flushed in order once the
  client is resumed. Every stored message arrives intact.

## Commands that are not implemented

The subject lists the features to implement, and only those are here. A
command outside that list (`WHO`, `WHOIS`, `LIST`, `NAMES`, ...) is answered
with `421 :Unknown command`, which is the normal reply for a command a
server does not provide. Some graphical clients send `WHO` automatically
after a `JOIN`, so that reply may show up once in the client window; it has
no effect on joining, messaging or any other feature.

## Error handling

* Bad arguments, bad port, empty password → the program exits with a message.
* A failed `socket`/`bind`/`listen` throws and is reported, nothing leaks.
* Command execution is wrapped in a `try/catch`, so even a failed allocation
  cannot take the server down.
* A client sending more than 4096 bytes without a newline is dropped instead
  of being allowed to grow the buffer forever.
* An abrupt disconnection (`POLLHUP`, `recv` returning 0) is treated like a
  `QUIT` and announced to the client's channels.
* `SIGPIPE` is ignored; `SIGINT`/`SIGTERM`/`SIGQUIT` shut the server down
  cleanly, freeing every client and channel.
