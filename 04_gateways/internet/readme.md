# REPL CLI - Interactive Shell

```
$ ./cli
ACLI 1.0
Type 'help' for available commands.

>> help
Available commands:
  help  - Show this help message
  quit  - Exit the shell
  exit  - Exit the shell

>> quit
Goodbye!
```

## Architecture

```
┌─────────────┐
│  CliShell   │ ── Reads input, shows prompt
└──────┬──────┘
       │
       ├── Built-in: help, quit, exit
       │
       └── Custom commands via Controller
              │
              ▼
       ┌──────────────────────────────┐
       │ RequestResponseInterface     │ (virtual class)
       └──────────────────────────────┘
                    │
                    ▼
            ┌───────────────┐
            │  Controller   │ (your implementation)
            └───────────────┘
```

## Project Structure

```
presenter_cli/
├── CMakeLists.txt
├── inc/
│   ├── cli_shell.h
│   └── request_response_interface.h
├── src/
│   ├── main.cc
│   └── cli_shell.cc
└── test/
    └── cli_shell_test.cc
```

