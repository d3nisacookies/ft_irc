#!/usr/bin/env bash
# ft_irc test suite
# Usage: ./tests/irc_test.sh [port]      (run from the repo root)
#
# Each check prints PASS / FAIL. A FAIL is not always a bug in finished code —
# some checks cover features that are still on the to-do list (451 gate, PING, ...).
# Expected replies follow RFC 1459 / 2812 numerics.

PORT=${1:-$((6000 + RANDOM % 2000))}
PASSWORD="pw"
HOST=127.0.0.1
BIN=./ircserv
WAIT=0.3            # seconds to wait for replies
LOGDIR=$(mktemp -d)

GREEN=$'\e[32m'; RED=$'\e[31m'; YELLOW=$'\e[33m'; BOLD=$'\e[1m'; RESET=$'\e[0m'
pass_count=0; fail_count=0; FAILED=()

declare -A FD      # client name -> file descriptor
declare -A LAST    # client name -> output received by the last recv

section() { printf '\n%s== %s ==%s\n' "$BOLD" "$1" "$RESET"; }
ok()      { pass_count=$((pass_count + 1)); printf '  %sPASS%s %s\n' "$GREEN" "$RESET" "$1"; }
ko()      { fail_count=$((fail_count + 1)); FAILED+=("$1"); printf '  %sFAIL%s %s\n' "$RED" "$RESET" "$1";
            [ -n "$2" ] && printf '       %s\n' "$2"; }

# ---------------------------------------------------------------- server control

SERVER_PID=""
start_server() {
    "$BIN" "$PORT" "$PASSWORD" >"$LOGDIR/server.log" 2>&1 &
    SERVER_PID=$!
    for _ in $(seq 1 20); do
        (exec 3<>/dev/tcp/$HOST/$PORT) 2>/dev/null && return 0
        sleep 0.1
    done
    echo "${RED}server did not start on port $PORT${RESET}"; cat "$LOGDIR/server.log"; exit 1
}
server_alive() { kill -0 "$SERVER_PID" 2>/dev/null; }
stop_server()  { [ -n "$SERVER_PID" ] && kill "$SERVER_PID" 2>/dev/null; wait "$SERVER_PID" 2>/dev/null; SERVER_PID=""; }

# Run after each section: a crash takes every client down, so it is always a FAIL.
check_alive() {
    if server_alive; then ok "server still running after: $1"
    else
        wait "$SERVER_PID" 2>/dev/null; local code=$?
        ko "SERVER CRASHED during: $1" "exit status $code (139=segfault, 132=illegal instruction, 134=abort)"
        close_all; start_server
    fi
}

# ---------------------------------------------------------------- client helpers

connect() {   # connect NAME
    local fd
    exec {fd}<>/dev/tcp/$HOST/$PORT || { ko "connect $1"; return 1; }
    FD[$1]=$fd; LAST[$1]=""
}
disconnect() { local fd=${FD[$1]}; [ -n "$fd" ] && exec {fd}>&-; unset "FD[$1]"; }
close_all()  { for n in "${!FD[@]}"; do disconnect "$n"; done; }

send()     { printf '%s\r\n' "$2" >&"${FD[$1]}" 2>/dev/null; }   # full line with CRLF
send_lf()  { printf '%s\n'   "$2" >&"${FD[$1]}" 2>/dev/null; }   # LF only (like plain nc)
send_raw() { printf '%b'     "$2" >&"${FD[$1]}" 2>/dev/null; }   # exact bytes, no newline added

recv() {      # read everything that arrives within $WAIT seconds
    local fd=${FD[$1]} line out=""
    while IFS= read -r -t "$WAIT" line <&"$fd"; do out+="${line%$'\r'}"$'\n'; done
    LAST[$1]=$out
}

# expect NAME REGEX DESCRIPTION  — reads new output, passes if REGEX matches
expect() {
    recv "$1"
    if grep -qE -- "$2" <<<"${LAST[$1]}"; then ok "$3"
    else ko "$3" "expected /$2/, got: $(printf '%s' "${LAST[$1]:-<nothing>}" | tr '\n' '|')"; fi
}
# expect_not NAME REGEX DESCRIPTION — reads new output, passes if REGEX does NOT match
expect_not() {
    recv "$1"
    if grep -qE -- "$2" <<<"${LAST[$1]}"; then
        ko "$3" "did not expect /$2/, got: $(printf '%s' "${LAST[$1]}" | tr '\n' '|')"
    else ok "$3"; fi
}
# expect_silent NAME DESCRIPTION — passes if nothing at all arrives
expect_silent() {
    recv "$1"
    if [ -z "${LAST[$1]}" ]; then ok "$2"
    else ko "$2" "expected no reply, got: $(printf '%s' "${LAST[$1]}" | tr '\n' '|')"; fi
}
# expect_closed NAME DESCRIPTION — passes if the server closed the connection
expect_closed() {
    local fd=${FD[$1]} line
    while IFS= read -r -t 1 line <&"$fd"; do :; done
    if IFS= read -r -t 0.2 line <&"$fd"; [ $? -eq 1 ]; then ok "$2"; else ko "$2" "connection still open"; fi
}

register() {  # register NAME NICK  — full registration, drains the replies
    connect "$1"; send "$1" "PASS $PASSWORD"; send "$1" "NICK $2"; send "$1" "USER $2 0 * :$2 real"; recv "$1"
}

# ================================================================== tests

cd "$(dirname "$0")/.." || exit 1

section "Build"
if make >/dev/null 2>&1; then ok "make"; else ko "make"; make; exit 1; fi
STRICT_OUT=$(mktemp -d)
strict_errors=$(for f in srcs/main.cpp srcs/Server.cpp srcs/Client.cpp srcs/Channel.cpp srcs/IRCMessage.cpp srcs/CommandHandler.cpp; do
    c++ -Wall -Wextra -Werror -std=c++98 -Iincludes -c "$f" -o "$STRICT_OUT/x.o" 2>&1; done | grep -E "error" | head -5)
if [ -z "$strict_errors" ]; then ok "compiles with -Wall -Wextra -Werror -std=c++98 (subject requirement)"
else ko "compiles with -Wall -Wextra -Werror -std=c++98 (subject requirement)" "$(echo "$strict_errors" | head -3 | tr '\n' '|')"; fi
grep -qE '^CXXFLAGS.*-Werror' Makefile && ok "Makefile uses -Werror" || ko "Makefile uses -Werror" "CXXFLAGS line has no -Werror"
rm -rf "$STRICT_OUT"

# ---------------------------------------------------------------- arguments
section "Startup arguments"
arg_case() {  # arg_case DESCRIPTION should_start args...
    local desc=$1 should=$2; shift 2
    "$BIN" "$@" >/dev/null 2>&1 & local pid=$!
    sleep 0.3
    if kill -0 $pid 2>/dev/null; then kill $pid; wait $pid 2>/dev/null
        [ "$should" = yes ] && ok "$desc" || ko "$desc" "server started but should have refused"
    else wait $pid; local code=$?
        if [ "$should" = no ]; then
            [ $code -ne 0 ] && ok "$desc" || ko "$desc" "refused but exit status was 0"
        else ko "$desc" "server exited (status $code) but should have started"; fi
    fi
}
APORT=$((PORT + 1))
arg_case "no arguments → refused"              no
arg_case "only port → refused"                 no  $APORT
arg_case "3 arguments → refused"               no  $APORT pw extra
arg_case "port 'abc' → refused"                no  abc pw
arg_case "port '80abc' → refused"              no  80abc pw
arg_case "port '-1' → refused"                 no  -1 pw
arg_case "port '+6667' → refused"              no  +6667 pw
arg_case "port '' → refused"                   no  "" pw
arg_case "port 70000 → refused"                no  70000 pw
arg_case "port 99999999999999999999 → refused (no overflow)" no 99999999999999999999 pw
arg_case "port 0 → refused"                    no  0 pw
arg_case "port 1023 (privileged) → refused"    no  1023 pw
arg_case "empty password → refused"            no  $APORT ""
arg_case "password with space → refused"       no  $APORT "a b"
arg_case "password with tab → refused"         no  $APORT $'a\tb'
arg_case "valid port + password → starts"      yes $APORT pw

start_server

# ---------------------------------------------------------------- line handling
section "Line handling (recv buffering)"
connect a
send_raw a "PA"; sleep 0.3; send_raw a "SS wrong\r\n"
expect a " 464 " "command split across two packets is joined (PA + SS wrong)"
send_raw a "PASS wr"; sleep 0.2; send_raw a "o"; sleep 0.2; send_raw a "ng\r\n"
expect a " 464 " "command split across three packets"
send_raw a "PASS one\r\nPASS two\r\n"
recv a; [ "$(grep -c ' 464 ' <<<"${LAST[a]}")" -eq 2 ] && ok "two commands in one packet → two replies" \
    || ko "two commands in one packet → two replies" "got: $(tr '\n' '|' <<<"${LAST[a]}")"
send_lf a "PASS lfonly"
expect a " 464 " "LF-only line ending (plain nc) accepted"
send_raw a "\r\n\r\n\n"
expect_silent a "empty lines are ignored silently"
send a "     "
expect_silent a "line of only spaces is ignored silently"
disconnect a
check_alive "line handling"

# ---------------------------------------------------------------- parser
section "Parser"
connect a
send a "pass wrong"
expect a " 464 " "lowercase command works (pass)"
send a "PaSs wrong"
expect a " 464 " "mixed-case command works (PaSs)"
send a ":someprefix PASS wrong"
expect a " 464 " "leading :prefix is skipped"
send a "PASS     wrong"
expect a " 464 " "multiple spaces between command and param"
send a "PASS :$PASSWORD"
expect_not a " 46[14] " "PASS :password (trailing form) accepted"
disconnect a
check_alive "parser"

# ---------------------------------------------------------------- PASS
section "PASS"
connect a
send a "PASS"
expect a " 461 " "PASS without parameter → 461"
send a "PASS wrong"
expect a " 464 " "wrong password → 464"
send a "PASS PW"
expect a " 464 " "password is case-sensitive (PW ≠ pw) → 464"
send a "PASS ${PASSWORD}x"
expect a " 464 " "password with extra char → 464"
send a "PASS $PASSWORD"
expect_not a " 46[14] " "correct password accepted"
disconnect a

connect a
send a "PASS wrong"
expect a "^:[^ ]+ 464 \* " "464 before NICK uses '*' as target"
disconnect a

register a pwreg
send a "PASS $PASSWORD"
expect a " 462 " "PASS after registration → 462"
disconnect a
check_alive "PASS"

# ---------------------------------------------------------------- NICK
section "NICK"
connect a
send a "PASS $PASSWORD"; recv a
send a "NICK"
expect a " 431 " "NICK without parameter → 431"
send a "NICK 1abc"
expect a " 432 " "nick starting with digit → 432"
send a "NICK -abc"
expect a " 432 " "nick starting with '-' → 432"
send a "NICK #chan"
expect a " 432 " "nick starting with '#' → 432"
send a "NICK abcdefghij"
expect a " 432 " "nick longer than 9 chars → 432"
send a "NICK ab:c"
expect a " 432 " "nick containing ':' → 432"
send a "NICK ab,c"
expect a " 432 " "nick containing ',' → 432"
send a "NICK ab@c"
expect a " 432 " "nick containing '@' → 432"
send a "NICK [a]_\`^{|}"
expect_not a " 43[12] " "nick made of allowed special chars accepted ([a]_\`^{|})"
send a "NICK a-b"
expect_not a " 43[12] " "nick with '-' after the first char accepted"
disconnect a

register a taken
connect b; send b "PASS $PASSWORD"; recv b
send b "NICK taken"
expect b " 433 " "nick already in use → 433"
send b "NICK TAKEN"
expect b " 433 " "nick in use, different case (IRC nicks are case-insensitive) → 433"
disconnect a
sleep 0.2
send b "NICK taken"
expect_not b " 433 " "nick becomes free after its owner disconnects"
disconnect b
check_alive "NICK"

# ---------------------------------------------------------------- USER
section "USER"
connect a
send a "PASS $PASSWORD"; recv a
send a "USER"
expect a " 461 " "USER without parameters → 461"
send a "USER a 0 *"
expect a " 461 " "USER with 3 parameters → 461"
disconnect a
register a userreg
send a "USER x 0 * :x"
expect a " 462 " "USER after registration → 462"
disconnect a
check_alive "USER"

# ---------------------------------------------------------------- registration
section "Registration / 001"
connect a
send a "PASS $PASSWORD"; send a "NICK reg1"; send a "USER reg1 0 * :Real Name"
expect a "^:[^ ]+ 001 reg1 " "PASS → NICK → USER gives 001 starting with ':'"
send a "NICK reg1b"
expect_not a " 001 " "no second 001 after a nick change"
disconnect a

connect a
send a "NICK reg2"; send a "USER reg2 0 * :R"; recv a
grep -q " 001 " <<<"${LAST[a]}" && ko "no 001 without PASS" || ok "no 001 without PASS"
send a "PASS $PASSWORD"
expect a " 001 reg2 " "NICK → USER → PASS gives 001 after PASS"
disconnect a

connect a
send a "PASS $PASSWORD"; send a "USER reg3 0 * :R"; send a "NICK reg3"
expect a " 001 reg3 " "PASS → USER → NICK gives 001 after NICK"
disconnect a

connect a
send a "PASS wrong"; send a "NICK reg4"; send a "USER reg4 0 * :R"
expect_not a " 001 " "wrong PASS then NICK/USER → no 001"
disconnect a
check_alive "registration"

# ---------------------------------------------------------------- before registration
section "Commands before registration"
for cmd in "JOIN #pre" "PRIVMSG x :hi" "PART #pre" "TOPIC #pre" "MODE #pre +i" "KICK #pre x" "INVITE x #pre"; do
    connect a
    send a "$cmd"
    expect a " 451 " "'${cmd%% *}' before registering → 451"
    disconnect a
    check_alive "${cmd%% *} before registration"
done
connect a
send a "CAP LS 302"
expect_not a " 451 | 421 " "CAP LS (sent first by irssi) is not rejected"
disconnect a

connect a
send a "PASS $PASSWORD"; send a "NICK half"; recv a
send a "JOIN #half"
expect a " 451 " "PASS+NICK but no USER → JOIN gives 451"
disconnect a
check_alive "pre-registration"

# ---------------------------------------------------------------- unknown / misc
section "Unknown command, PING, QUIT"
register a misc
send a "FOOBAR"
expect a " 421 " "unknown command → 421"
send a "PING :token123"
expect a "PONG.*token123" "PING → PONG with the same token"
send a "QUIT :bye"
expect_closed a "QUIT closes the connection"
disconnect a
check_alive "misc"

# ---------------------------------------------------------------- JOIN / PRIVMSG / PART
section "JOIN / PRIVMSG / PART"
register a alice
register b bob
send a "JOIN #room"
expect a ":alice!.* JOIN :?#room" "JOIN: joiner gets the JOIN message"
send a "JOIN"
expect a " 461 " "JOIN without parameter → 461"
send a "JOIN room"
expect a " 403 | 476 " "JOIN channel without '#' → 403/476"
send b "JOIN #room"
expect b ":bob!.* JOIN :?#room" "second user joins"
expect a ":bob!.* JOIN :?#room" "existing member sees the new JOIN"

send a "PRIVMSG #room :hello room"
expect b ":alice!.* PRIVMSG #room :hello room" "channel message reaches other member"
expect_not a "PRIVMSG #room :hello room" "sender does not get their own channel message"
send a "PRIVMSG #room :multi word  message: with colon"
expect b "PRIVMSG #room :multi word  message: with colon" "trailing text kept exactly (spaces + colons)"
send b "PRIVMSG alice :hi alice"
expect a ":bob!.* PRIVMSG alice :hi alice" "private message to a nick"
send a "PRIVMSG nobody :hi"
expect a " 401 " "PRIVMSG to unknown nick → 401"
send a "PRIVMSG #nochan :hi"
expect a " 40[13] " "PRIVMSG to unknown channel → 401/403"
send a "PRIVMSG"
expect a " 411 " "PRIVMSG without target → 411"
send a "PRIVMSG bob"
expect a " 412 " "PRIVMSG without text → 412"
register c carol
send c "PRIVMSG #room :let me in"
expect c " 404 " "PRIVMSG to a channel you're not in → 404"

send b "PART #room :see ya"
expect a ":bob!.* PART #room" "PART is seen by other members"
send b "PART #room"
expect b " 442 " "PART a channel you're not in → 442"
send b "PART #ghost"
expect b " 403 " "PART a channel that does not exist → 403"
close_all
check_alive "JOIN/PRIVMSG/PART"

# ---------------------------------------------------------------- operator commands
section "KICK / INVITE / TOPIC"
register op oper1
register u user1
register v user2
send op "JOIN #ops"; recv op
send u "JOIN #ops"; recv u; recv op

send u "KICK #ops oper1"
expect u " 482 " "KICK by non-operator → 482"
send op "KICK #ops nobody"
expect op " 401 | 441 " "KICK unknown nick → 401/441"
send op "KICK #ops user1 :bye"
expect u "KICK #ops user1" "KICK by operator: victim is told"
send u "PRIVMSG #ops :still here?"
expect u " 404 | 442 " "kicked user can no longer talk in the channel"

send op "TOPIC #ops :new topic here"
expect op "TOPIC #ops :new topic here| 332 " "operator sets topic"
send v "JOIN #ops"; recv v; recv op
send v "TOPIC #ops"
expect v "new topic here" "TOPIC without text shows the current topic"
send v "TOPIC #ops :hijack"
expect v "TOPIC #ops :hijack" "without +t, a non-operator may change the topic"

send op "INVITE user1 #ops"
expect u "INVITE user1 :?#ops" "INVITE: target receives the invite"
expect op " 341 " "INVITE: inviter gets 341"
send op "INVITE nobody #ops"
expect op " 401 " "INVITE unknown nick → 401"
close_all
check_alive "KICK/INVITE/TOPIC"

# ---------------------------------------------------------------- MODE
section "MODE i / t / k / o / l"
register op oper2
register x xuser
register y yuser
send op "JOIN #m"; recv op

send x "JOIN #m"; recv x; recv op
send x "MODE #m +i"
expect x " 482 " "MODE by non-operator → 482"
send x "PART #m"; recv x; recv op

send op "MODE #m +i"; recv op
send x "JOIN #m"
expect x " 473 " "+i: JOIN without invite → 473"
send op "INVITE xuser #m"; recv op; recv x
send x "JOIN #m"
expect x "JOIN :?#m" "+i: JOIN after INVITE works"
send op "MODE #m -i"; recv op; recv x

send op "MODE #m +k secret"; recv op
send y "JOIN #m"
expect y " 475 " "+k: JOIN without key → 475"
send y "JOIN #m wrongkey"
expect y " 475 " "+k: JOIN with wrong key → 475"
send y "JOIN #m secret"
expect y "JOIN :?#m" "+k: JOIN with right key works"
send op "MODE #m -k"; recv op; recv x; recv y
send y "PART #m"; recv y; recv op; recv x

send op "MODE #m +l 2"; recv op; recv x
send y "JOIN #m"
expect y " 471 " "+l 2 with 2 members: third JOIN → 471"
send op "MODE #m +l abc"
expect_not op " 471 " "+l with non-numeric limit is rejected or ignored (no crash)"
send op "MODE #m -l"; recv op; recv x

send op "MODE #m +t"; recv op; recv x
send x "TOPIC #m :not allowed"
expect x " 482 " "+t: non-op TOPIC → 482"

send op "MODE #m +o xuser"; recv op; recv x
send x "TOPIC #m :now allowed"
expect x "TOPIC #m :now allowed" "+o: new operator may change topic under +t"
send op "MODE #m +o nobody"
expect op " 401 | 441 " "+o unknown nick → 401/441"
send op "MODE #m +z"
expect op " 472 " "unknown mode char → 472"
send op "MODE #nochan +i"
expect op " 403 | 401 " "MODE on unknown channel → 403"
close_all
check_alive "MODE"

# ---------------------------------------------------------------- robustness
section "Robustness"
connect a
send_raw a "PASS $PASSWORD\r\nNICK half"     # unfinished line, then vanish
disconnect a
sleep 0.2
check_alive "client disconnects mid-line"

connect a
long=$(head -c 2000 /dev/zero | tr '\0' 'A')
send a "PRIVMSG x :$long"
recv a
disconnect a
check_alive "2000-byte line (RFC max is 512)"

connect a
send_raw a "\x01\x02\xff\xfe garbage \x00 bytes\r\n"
recv a
disconnect a
check_alive "binary garbage"

connect a
send_raw a "$(head -c 100000 /dev/zero | tr '\0' 'B')"   # 100 KB without any newline
recv a
disconnect a
check_alive "100 KB without newline"

for i in $(seq 1 50); do connect "m$i"; done
for i in $(seq 1 50); do send "m$i" "PASS $PASSWORD"; send "m$i" "NICK many$i"; send "m$i" "USER u 0 * :u"; done
sleep 0.5
recv m50
grep -q " 001 many50 " <<<"${LAST[m50]}" && ok "50 simultaneous clients all register (checked #50)" \
    || ko "50 simultaneous clients all register (checked #50)" "got: $(tr '\n' '|' <<<"${LAST[m50]:-<nothing>}")"
for i in $(seq 1 50); do disconnect "m$i"; done
check_alive "50 clients"

register a stayer
register b leaver
send a "JOIN #crash"; recv a
send b "JOIN #crash"; recv b; recv a
disconnect b                               # leave without QUIT/PART
sleep 0.2
send a "PRIVMSG #crash :anyone?"
recv a
check_alive "message to channel after a member vanished (dangling Client*?)"
send a "NICK leaver"
expect_not a " 433 " "nick of a vanished client is released"
close_all
check_alive "robustness"

# ================================================================== summary
stop_server
total=$((pass_count + fail_count))
printf '\n%s== Summary ==%s\n' "$BOLD" "$RESET"
printf '  %s%d passed%s, %s%d failed%s, %d total\n' "$GREEN" "$pass_count" "$RESET" "$RED" "$fail_count" "$RESET" "$total"
if [ $fail_count -gt 0 ]; then
    printf '\n%sFailed:%s\n' "$YELLOW" "$RESET"
    for f in "${FAILED[@]}"; do printf '  - %s\n' "$f"; done
fi
printf '\nServer log: %s/server.log\n' "$LOGDIR"
[ $fail_count -eq 0 ]
