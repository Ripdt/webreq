# WebReq

A lightweight command-line tool written in C to make HTTP web requests. No bloated GUIs, no unnecessary features — just simple, fast HTTP requests from your terminal.

## Index

- [WebReq](#webreq)
  - [Index](#index)
  - [Inspiration](#inspiration)
  - [Build](#build)
  - [Usage](#usage)
  - [Options](#options)
  - [Examples](#examples)

## Inspiration

Hate. Hate is the inspiration. I was tired of using complicated GUI interfaces with a billion features that I don't need. Also, I was implementing a MCP server for my job and needed to make HTTP requests to verify the tools. The LLM tools cost tokens and tokens are money. So maybe you can summarize the inspiration to **demand + hate = action**.

## Build

Requires GCC. On Windows, `ws2_32` is linked automatically.

```bash
make build        # Debug build (default)
make debug        # Debug build (same as build)
make release      # Optimized release build
make lib_debug    # Build only the shared library (debug)
make lib_release  # Build only the shared library (release)
make clean        # Remove all compiled files
```

The Makefile auto-detects your OS and outputs to the correct directory:

| OS | Debug output | Release output |
|----|-------------|----------------|
| Windows | `bin/debug/windows/req.exe` | `bin/release/windows/req.exe` |
| Linux | `bin/debug/linux/req` | `bin/release/linux/req` |

## Usage

**Windows:**
```
./bin/debug/windows/req.exe [OPTIONS]
```

**Linux:**
```
./bin/debug/linux/req [OPTIONS]
```

If options are not provided via command-line flags, the tool will prompt you interactively.

## Options

| Flag | Long | Description |
|------|------|-------------|
| `-h` | `--help` | Show help message and exit |
| `-H` | `--host HOST` | Specify the host address (e.g. `example.com`) |
| `-p` | `--port PORT` | Specify the port number (e.g. `80`) |
| `-P` | `--path PATH` | Specify the request path (e.g. `/api/data`) |
| `-m` | `--message MSG` | Specify the request body |
| `-M` | `--method METHOD` | Specify the HTTP method (`GET`, `POST`, etc.) |

## Examples

**Simple GET request:**

```bash
# Windows
./bin/debug/windows/req.exe -H example.com -p 80 -P / -M GET -m ""

# Linux
./bin/debug/linux/req -H example.com -p 80 -P / -M GET -m ""
```

**POST request with a body:**

```bash
# Windows
./bin/debug/windows/req.exe -H example.com -p 80 -P /api/submit -M POST -m '{"key":"value"}'

# Linux
./bin/debug/linux/req -H example.com -p 80 -P /api/submit -M POST -m '{"key":"value"}'
```

**Interactive mode** (just run without arguments):

```bash
./bin/debug/windows/req.exe
```

The tool will prompt you for host, port, path, method, and message.
