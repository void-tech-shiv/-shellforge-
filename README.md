# ShellForge

ShellForge is a Unix-style command shell written in C. It includes a lexer, parser, variable expansion, pipelines, I/O redirection, command history, background processes, and job control.

## Friendly ShellForge commands

ShellForge supports easy-to-read command names while keeping the original Unix names available.

| ShellForge | Unix command | Purpose |
|---|---|---|
| `list` | `ls` | List files/folders |
| `go` | `cd` | Change directory |
| `where` | `pwd` | Show current directory |
| `say` | `echo` | Print text |
| `newfile` | `touch` | Create empty file |
| `show` | `cat` | Display file contents |
| `copy` | `cp` | Copy file/directory |
| `move` | `mv` | Move or rename |
| `delete` | `rm` | Delete a file |
| `makedir` | `mkdir` | Create directory |
| `removedir` | `rmdir` | Remove empty directory |
| `findtext` | `grep` | Search text |
| `processes` | `ps` | Show processes |
| `clean` | `clear` | Clear terminal |
| `me` | `whoami` | Show current user |
| `today` | `date` | Show date/time |
| `past` | `history` | Show command history |
| `tasks` | `jobs` | Show jobs |
| `front` | `fg` | Resume job in foreground |
| `back` | `bg` | Resume job in background |
| `help` | `help` | Show all ShellForge commands and shell features |
| `manual` | `help` | Friendly alias for the help page |
| `quit` | `exit` | Exit ShellForge |

## Help command

Run `help` inside ShellForge to display all friendly commands, their Linux equivalents, usage hints, and supported shell features such as pipelines, redirection, background execution, environment variables, and history.

```text
shellforge$ help
```

## Ubuntu setup

Install the required tools and GNU Readline development library:

```bash
sudo apt update
sudo apt install build-essential libreadline-dev git -y
```

Clone and build:

```bash
git clone https://github.com/void-tech-shiv/-shellforge-.git
cd ./-shellforge-
make
./shellforge
```

## Quick demo

Inside ShellForge:

```text
help
list
where
makedir demo
go demo
newfile hello.txt
say Hello ShellForge > hello.txt
show hello.txt
go ..
list | findtext demo
past
quit
```

## Build cleanup

```bash
make clean
```
