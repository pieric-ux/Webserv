*This project has been created as part of the 42 curriculum by blucken, pdemont.*

# Webserv

A non-blocking **HTTP/1.1 web server written in C++98**, configured through an
nginx-inspired configuration file. Webserv accepts and serves multiple clients
simultaneously on multiple ports using a single I/O multiplexing loop
(`poll`/`select`), and never blocks on a single connection.

---

## Description

The goal of *webserv* is to understand how an HTTP server works by writing one
from scratch, using only low-level system calls. The server reads a
configuration file describing virtual servers and their routes, then listens for
incoming TCP connections, parses each HTTP request, builds the matching response,
and writes it back — all driven by a single event loop so that one slow client
can never stall the others.

What makes this implementation stand out is that the **configuration file and the
HTTP messages are validated against formal grammars**. Instead of a hand-rolled
parser, Webserv relies on a custom ABNF engine that checks input against
[RFC 5234](https://www.rfc-editor.org/rfc/rfc5234) grammars
(`assets/ServerConfig.abnf`, `assets/HTTP.abnf`, `assets/URI.abnf`, ...).

The project is split into the server itself and three of our own libraries,
pulled in as git submodules under `libs/`:

| Library         | Library file   | Responsibility                                                                            |
| --------------- | -------------- | ----------------------------------------------------------------------------------------- |
| **Cpp-common**  | `libcommon.a`  | TCP sockets, the `poll`/`select` event-I/O abstraction, RAII smart pointers, file/string utilities |
| **Log42**       | `liblog42.a`   | Hierarchical logging system inspired by Python's `logging` (Manager → Logger → Handler → Formatter) |
| **ParserAbnf**  | `libabnf.a`    | RFC 5234 ABNF parser used to validate the config file and HTTP messages                    |

### Features

- **Core HTTP/1.1** — `GET`, `HEAD`, `POST`, `PUT`, `DELETE`; static file
  serving, directory listing (`autoindex`), custom error pages (`error_page`),
  HTTP redirects (`return`), MIME-type resolution, and `client_max_body_size`
  limits.
- **Multiple servers & routes** — several `server` blocks, virtual hosts via
  `server_name`, multiple `listen` endpoints, and per-route `location` blocks
  with prefix / exact (`=`) / `^~` matching.
- **CGI execution** — runs external scripts (e.g. `python3`, `bash`, `php`) via
  the `cgi_extensions` / `enable_cgi` directives.
- **Sessions & cookies** — cookie parsing and a server-side session store for
  stateful clients.
- **WebDAV-style uploads** — `dav_methods` (`PUT`/`DELETE`), `dav_access`
  permissions, and `dav_put_path` for file uploads.

---

## Instructions

### Requirements

- A C++98-capable compiler (`c++` / `g++` / `clang++`)
- `make`
- A Unix-like system (Linux or macOS)
- `git` (the libraries are git submodules)

### Installation

The three libraries live in `libs/` as git submodules, so they **must** be
fetched before building:

```sh
# Clone with submodules in one step...
git clone --recursive <repository-url>

# ...or, if you already cloned without --recursive:
git submodule update --init --recursive
```

### Compilation

```sh
make            # build the libraries, then the webserv binary
make re         # full rebuild from scratch
make clean      # remove object files
make fclean     # remove objects, libraries and the binary
```

`make` automatically builds `libcommon.a`, `liblog42.a` and `libabnf.a` before
linking `webserv`. Compilation uses `-Wall -Wextra -Werror -Wshadow -std=c++98`.

Additional targets:

```sh
make debug      # rebuild with debug symbols (-g3, stack protector, ...)
make sanitize   # rebuild with AddressSanitizer enabled
make leaks      # run under leaks (macOS) / valgrind (Linux)
DEV_LOG=1 make  # build with verbose DEBUG-level logging
```

### Execution

Run the server **from the `Webserv/` directory** (it resolves `configs/`,
`assets/`, `www/` and `logs/` as relative paths):

```sh
./webserv                      # uses the default config: configs/default.conf
./webserv configs/blucken.conf # uses a custom config (must end in .conf)
```

Once running, open the served site in a browser or query it with `curl`:

```sh
curl -v http://localhost:8080/
```

Ready-to-use configuration files are provided in `configs/` and
`example_configs/`, and a sample web root is provided in `www/`.

### Configuration example

```nginx
http {
    types {
        text/html  html htm;
        text/css   css;
        image/png  png;
    }

    cgi_extensions {
        /usr/bin/python3  .py;
    }

    server {
        listen localhost:8080 default_server;
        server_name example.com www.example.com;
        root /path/to/www;

        location / {
            allowed_methods GET HEAD POST;
            index index.html;
            autoindex on;
            client_max_body_size 2M;
            error_page 404 /404.html;
        }

        location /cgi-bin {
            allowed_methods GET POST;
            enable_cgi on;
        }

        location /old {
            return 301 http://example.com/new;
        }
    }
}
```

---

## Resources

References used while building the project:

- [RFC 9110 — HTTP Semantics](https://www.rfc-editor.org/rfc/rfc9110)
- [RFC 9112 — HTTP/1.1](https://www.rfc-editor.org/rfc/rfc9112)
- [RFC 7230 — HTTP/1.1: Message Syntax and Routing](https://www.rfc-editor.org/rfc/rfc7230)
- [RFC 5234 — Augmented BNF for Syntax Specifications: ABNF](https://www.rfc-editor.org/rfc/rfc5234)
- [RFC 3986 — Uniform Resource Identifier (URI): Generic Syntax](https://www.rfc-editor.org/rfc/rfc3986)
- [RFC 3875 — The Common Gateway Interface (CGI) Version 1.1](https://www.rfc-editor.org/rfc/rfc3875)
- [RFC 6265 — HTTP State Management Mechanism (Cookies)](https://www.rfc-editor.org/rfc/rfc6265)
- [nginx documentation](https://nginx.org/en/docs/) — configuration model and directive behavior
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/) — sockets and `poll`/`select`
- [MDN Web Docs — HTTP](https://developer.mozilla.org/en-US/docs/Web/HTTP)

### Use of AI

AI (an LLM-based assistant) was used **only for documentation tasks**: drafting
and structuring this `README.md`, writing the documentation of the companion
libraries (Log42, ParserAbnf), and producing in-code documentation comments. The
design, architecture, and implementation of the server and its libraries were
written by the authors; AI did not generate the project's source logic.
