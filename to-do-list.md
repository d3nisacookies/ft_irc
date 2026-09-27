# 26 Sep — ft_irc Notes

**To:** AK

## TODO

* Handle `""` command in parser.
* Clear `_params` before parsing a new message.

  * Currently `_params` keeps using `push_back()`.
* Client status needs to be handled.

  * Currently tracking registration using:

    * `hasUsername()`
    * `hasNickname()`
    * `isPassVerified()`

## Registration Commands

```text
PASS password
NICK nickname
USER username hostname servername :realname
```

### Example

```text
PASS mypassword
NICK bob
USER bob localhost myserver :Bob Smith
```

### Params

* `PASS` → 1 param
* `NICK` → 1 param
* `USER` → 4 params
  * `[0]` → username
  * `[1]` → hostname
  * `[2]` → servername
  * `[3]` → realname


**To:** Ksan

- Sever got functions that havent write in the cpp but declare in hpp