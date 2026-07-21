# Webserv — Plan de test exhaustif

> Serveur HTTP/1.1 en C++98 (42) devant se comporter comme nginx. Plan construit par lecture croisée de **4 sources** : le code (`srcs/`), les tests existants (`test/`), le sujet 42 (`docs/en.subject.pdf`) et les RFC **9112** (framing HTTP/1.1), **9110** (sémantique/status) et **3875** (CGI).

| | |
|---|---|
| Exigences tracées | **43** (sujet) + **90** (RFC) = 133 |
| Cas de test définis | **170** — 78×P0, 79×P1, 13×P2 |
| Risques code identifiés | **72** (avec `fichier:ligne`) |
| Suites existantes | 5 |

**Légende priorité** — `P0` critique jour d'éval (crash / hang / leak / feature obligatoire), `P1` important, `P2` confort. **Types** — `unit`, `integration`, `differential-vs-nginx` (nginx = oracle), `robustness`, `stress`.

## Sommaire

1. [Comment utiliser ce plan](#1-comment-utiliser-ce-plan)
2. [Synthèse & chemin critique P0](#2-synthèse--chemin-critique-p0)
3. [Couverture actuelle](#3-couverture-actuelle)
4. [Matrice de traçabilité](#4-matrice-de-traçabilité)
5. [Registre de risques (analyse du code)](#5-registre-de-risques-analyse-du-code)
6. [Catalogue de tests (170 cas)](#6-catalogue-de-tests)
   - 6.1 HTTP request parsing & message framing (21)
   - 6.2 HTTP Methods & Static File Serving (20)
   - 6.3 POST / PUT / DELETE & file upload (WebDAV) (23)
   - 6.4 CGI execution & robustness (20)
   - 6.5 Configuration parsing & routing semantics (20)
   - 6.6 Connection management, I/O & non-blocking guarantees (20)
   - 6.7 Status Codes & Error Pages (26)
   - 6.8 Robustness, Stress, Leaks & Evaluation-Day Checklist (20)
7. [Lacunes de complétude (critique)](#7-lacunes-de-complétude)
8. [Plan d'automatisation](#8-plan-dautomatisation)
9. [Annexes — exigences détaillées](#9-annexes--exigences-détaillées)

---

## 1. Comment utiliser ce plan

Ce document est une **référence de travail**, pas un script. Ordre d'attaque recommandé :

1. **Lire le §2** (chemin critique P0) — ce sont les tests qui font la différence entre 0 et une note.
2. **Lire le §5** (registre de risques) — chaque risque est un bug *suspecté* dans le code actuel, avec `fichier:ligne`. Les cas de test P0 du §6 les ciblent directement.
3. **Automatiser** en suivant le §8 : étendre les runners bash existants (`test/parser_tests`, `test/nginx_tests`) et ajouter un runner HTTP boîte-noire.
4. **Tracer** avec le §4 et l'annexe §9 : chaque exigence sujet/RFC est reliée aux cas qui la vérifient (ou marquée ⚠️ non couverte).

> Convention d'ID : `HTTP-xx` (framing), `MTH-/GET-` (méthodes), `DAV-` (upload), `CGI-xx`, `CFG-xx`, `CONN-xx`, `STATUS-xx`, `STRESS-xx`. Les IDs sont ceux générés par l'analyse et référencés dans la matrice.

---

## 2. Synthèse & chemin critique P0

Les mandats 42 non-négociables (échec = note 0) et où ils sont testés :

- **Ne jamais crasher / rester dispo même sous stress** → `STRESS-*`, tests garbage/binaire, siege.
- **Ne jamais bloquer / hang** → tests slow-loris, requête partielle, `CONN-*`, CGI boucle infinie.
- **Un seul poll/select/epoll/kqueue pour TOUTE l'I/O, jamais de read/write sans passer par lui, pas de test errno après read/write** → audit `strace`/`dtruss` + `CONN-*`.
- **Codes de statut corrects + pages d'erreur par défaut** → `STATUS-*`.
- **GET/POST/DELETE, upload, CGI, multi-serveurs/ports, client_max_body_size, error_page** → sections dédiées.

### Checklist P0 (78 cas)

| ID | Cas | Section |
|---|---|---|
| `HTTP-01` | Segmented header delivery must be buffered, not rejected (suspected gap #1) | HTTP request parsing & message framing |
| `HTTP-06` | Missing Host header on HTTP/1.1 -> 400 (mandatory) | HTTP request parsing & message framing |
| `HTTP-10` | Content-Length errors: non-digit, negative, duplicate/conflicting -> 400 | HTTP request parsing & message framing |
| `HTTP-11` | CL+TE conflict: POST 400 vs GET smuggling bypass (suspected gap #2) | HTTP request parsing & message framing |
| `HTTP-12` | GET/HEAD body not consumed -> pipelining desync/smuggling (suspected gap #3) | HTTP request parsing & message framing |
| `HTTP-13` | Chunked decoding correctness: hex size, chunk-ext, trailer (roundtrip) | HTTP request parsing & message framing |
| `HTTP-14` | Malformed / lenient / overflowing chunk-size (suspected gap #10, no crash) | HTTP request parsing & message framing |
| `HTTP-16` | Body size limit enforcement -> 413 (Content-Length and chunked) | HTTP request parsing & message framing |
| `HTTP-17` | Incomplete/short body must not hang the server (timeout/close) | HTTP request parsing & message framing |
| `HSF-01` | GET static file returns 200 with byte-exact body and correct framing headers | HTTP Methods & Static File Serving |
| `HSF-04` | HEAD returns GET-identical headers with an empty body | HTTP Methods & Static File Serving |
| `HSF-06` | 404 Not Found for missing resource serves the configured custom error page | HTTP Methods & Static File Serving |
| `HSF-07` | 403 Forbidden on a file with no read permission | HTTP Methods & Static File Serving |
| `HSF-08` | autoindex ON renders an HTML directory listing when no index file exists | HTTP Methods & Static File Serving |
| `HSF-09` | autoindex OFF on a directory with no index file returns 403 | HTTP Methods & Static File Serving |
| `HSF-10` | Directory index file served instead of a listing | HTTP Methods & Static File Serving |
| `HSF-11` | Directory requested without a trailing slash yields 301 to the slash-terminated path | HTTP Methods & Static File Serving |
| `HSF-12` | Configured 'return' redirects: 301 (absolute URI), 302 (relative), 410 (no Location) | HTTP Methods & Static File Serving |
| `HSF-13` | Method not permitted on a route → 405 with an accurate Allow header | HTTP Methods & Static File Serving |
| `HSF-14` | Unrecognized / unimplemented request method → 501 (not 405, not served) | HTTP Methods & Static File Serving |
| `HSF-15` | Large file (> BUFFER_SIZE) streamed byte-exact with correct Content-Length, no truncation/hang | HTTP Methods & Static File Serving |
| `HSF-17` | DIFFERENTIAL: HTTP/1.0 request handling (webserv 505 vs nginx 200) | HTTP Methods & Static File Serving |
| `DAV-DIFF-01` | Automated status diff of the 23-cell DAV matrix vs recorded nginx oracle | POST / PUT / DELETE & file upload (WebDAV) |
| `PUT-01` | PUT creates new file -> 201, Content-Length: 0, bytes verbatim | POST / PUT / DELETE & file upload (WebDAV) |
| `DEL-01` | DELETE existing file -> 204, removed, GET -> 404 | POST / PUT / DELETE & file upload (WebDAV) |
| `CMBS-413-01` | PUT body larger than client_max_body_size -> 413, no file created | POST / PUT / DELETE & file upload (WebDAV) |
| `DAV-ROBUST-LOOP` | Sustained PUT/DELETE churn + concurrent GET: no crash, no fd leak | POST / PUT / DELETE & file upload (WebDAV) |
| `CGI-01` | GET .py happy path returns dynamic output, not source | CGI execution & robustness |
| `CGI-02` | GET query string + required meta-variables via env dump | CGI execution & robustness |
| `CGI-03` | POST body delivered on CGI stdin with correct CONTENT_LENGTH | CGI execution & robustness |
| `CGI-04` | Chunked request body un-chunked before CGI (stdin sees decoded bytes, EOF at true end) | CGI execution & robustness |
| `CGI-05` | Infinite/long-running CGI times out to 5xx and server stays responsive (no hang) | CGI execution & robustness |
| `CGI-06` | CGI killed by signal (segfault) -> 502, no server crash | CGI execution & robustness |
| `CGI-07` | CGI exits non-zero with no output -> 502 | CGI execution & robustness |
| `CGI-08` | Malformed CGI output (no header/body separator) -> 502, not raw passthrough | CGI execution & robustness |
| `CGI-09` | GAP: large CGI output (>8 KB) from a fast-exiting script is truncated | CGI execution & robustness |
| `CGI-10` | No zombie processes accumulate after repeated CGI requests | CGI execution & robustness |
| `CGI-11` | No file-descriptor leak across many CGI requests | CGI execution & robustness |
| `CFG-01` | Config with no http{} wrapper must exit non-zero | Configuration parsing & routing semantics |
| `CFG-02` | Two http{} blocks must exit non-zero | Configuration parsing & routing semantics |
| `CFG-03` | http{} with zero server{} blocks must exit non-zero | Configuration parsing & routing semantics |
| `CFG-04` | Duplicate singleton directive (two root;) must exit non-zero | Configuration parsing & routing semantics |
| `RT-01` | [GAP] Root path mapping: /kapouet -> /tmp/www (REQ-28 strip vs webserv concat) | Configuration parsing & routing semantics |
| `RT-03` | [GAP] Server with no location block must not crash on any request | Configuration parsing & routing semantics |
| `RT-07` | Custom error_page served byte-exact on 404 (REQ-16) | Configuration parsing & routing semantics |
| `CONN-05` | Byte-by-byte / segmented header delivery must NOT trigger a premature 400 | Connection management, I/O & non-blocking guarantees |
| `CONN-09` | One stalled slow reader must NOT starve other clients (blocking-send head-of-line) | Connection management, I/O & non-blocking guarantees |
| `CONN-11` | Many simultaneous keep-alive clients: sustained availability, no crash, bounded latency | Connection management, I/O & non-blocking guarantees |
| `CONN-12` | No file-descriptor leak under repeated connect / partial-request / abrupt-close churn | Connection management, I/O & non-blocking guarantees |
| `CONN-13` | Client resets (RST) mid-response — SIGPIPE must be ignored, server survives | Connection management, I/O & non-blocking guarantees |
| `CONN-14` | Abrupt client disconnect mid-request does not crash or hang the server | Connection management, I/O & non-blocking guarantees |
| `CONN-15` | Single multiplexer for ALL fds; no per-connection thread; no fork for static serving | Connection management, I/O & non-blocking guarantees |
| `SC-01` | 200 OK — static GET returns exact bytes, Content-Type, and byte-exact Content-Length | Status Codes & Error Pages |
| `SC-02` | 201 Created — PUT a new file returns 201 with Content-Length: 0 | Status Codes & Error Pages |
| `SC-03` | 204 No Content — PUT overwrite and DELETE return 204 with an empty body and NO Content-Length | Status Codes & Error Pages |
| `SC-04` | 301 Moved Permanently — configured return with absolute Location | Status Codes & Error Pages |
| `SC-07` | 400 Bad Request — HTTP/1.1 request missing Host header | Status Codes & Error Pages |
| `SC-10` | 404 + custom error_page served (http-level) — byte-exact custom file body | Status Codes & Error Pages |
| `SC-14` | 405 Method Not Allowed — correct Allow header lists the route's permitted methods | Status Codes & Error Pages |
| `SC-16` | 411 Length Required — body-method with neither Content-Length nor Transfer-Encoding | Status Codes & Error Pages |
| `SC-17` | 413 Content Too Large — POST body exceeding client_max_body_size | Status Codes & Error Pages |
| `SC-18` | 414 URI Too Long — request-target beyond URI_MAX_LENGTH (2048) | Status Codes & Error Pages |
| `SC-19` | 500 Internal Server Error — PUT to a DAV-configured route (broken) yields 500 with correct framing | Status Codes & Error Pages |
| `SC-20` | 501 Not Implemented — unknown method and lowercase method token | Status Codes & Error Pages |
| `SC-21` | 505 HTTP Version Not Supported — HTTP/2.0 rejected; HTTP/1.0 WRONGLY rejected | Status Codes & Error Pages |
| `STRESS-01` | siege -b sustained availability must stay ~100% with zero failed transactions | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-02` | RSS must plateau under sustained load (weaponizes unbounded session-store growth) | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-03` | No file-descriptor leak across connect/disconnect churn (lsof over time) | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-04` | Leak checker clean: macOS leaks(1) / Linux valgrind report no lost blocks after load | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-05` | Graceful shutdown on SIGINT/SIGTERM (targets broken g_SignalStatus / no destructor teardown) | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-06` | Never crash on garbage / binary / TLS-handshake input | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-07` | Resource exhaustion: running out of file descriptors (EMFILE) must not crash | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-08` | Slow non-reading client must NOT stall other clients (blocking-socket head-of-line DoS) | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-09` | Incomplete/dripped request body must not hang the connection forever (no request-read timeout) | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-10` | TCP-segmented (dripped) valid request must be reassembled, not answered 400 (gap #1) | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-12` | Official 42 ./tester passes (GET/POST/DELETE, status codes, client_max_body_size, multi-request) | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-13` | Official cgi_tester passes end-to-end CGI | Robustness, Stress, Leaks & Evaluation-Day Checklist |
| `STRESS-18` | Post-adversarial liveness gate: same PID alive and serving 200 after every stress test | Robustness, Stress, Leaks & Evaluation-Day Checklist |

---

## 3. Couverture actuelle

### Parser config tests
`test/parser_tests/run_tests.sh`

**Couvre :**
- Config-file parsing only: runs ./webserv <config>, greps logs/webserv/parser.log, and asserts parsed directive values (no HTTP traffic at all)
- client_max_body_size: 1k, default 1048576, unit variants (plain bytes 4096, 10M=10485760, 2G=2147483648), 512K via comment.conf
- create_full_put_path on/off; dav_put_path; dav_access octal mode parsing (0664, 0600, 0700); dav_methods 'PUT DELETE' and 'off' (empty)
- default_type (application/octet-stream, text/plain); keepalive_timeout (120s, default 75s, 30s); root directive; enable_cgi on/off
- MIME types block parsing (html/htm/jpg/jpeg/mp4) and cgi extension mapping via both directive form and cgi_extensions block (.py .sh .php .rb .pl)
- error_page wildcard resolution: explicit 50x, 5xx expansion, and xxx catch-all expansion; total error_page count assertion (=5)
- Comment stripping (comment.conf); parser rejection with non-zero exit for invalid_syntax.conf, empty.conf, and nonexistent.conf (assert_fail)

**Manques :**
- Verifies only log-string output, never runtime behavior — a directive can parse/log correctly yet be applied wrong at request time and still pass
- Brittle: uses grep -m1 (first match only) and substring matching on human-readable log lines; a log-format change breaks all asserts
- No semantic validation coverage: no negative/overflow sizes, invalid port or host:port, invalid octal dav_access, duplicate/conflicting directives, missing listen, unknown directive names
- No multi-server / multi-location parsing: no server_name, no multiple listen blocks, no location matching precedence, no return/rewrite/redirect directives, no allow_methods/limit_except
- Error cases only assert non-zero exit, not the specific error message or which line failed; nonexistent.conf test relies on file simply being absent

### nginx DAV reference harness
`test/nginx_tests/run-tests.sh`

**Couvre :**
- Generates reference curl -sv transcripts from real nginx (the oracle webserv should match), not assertions against webserv itself
- Write-method status codes across an 8-way matrix: DAV on/off x autoindex on/off x index.html present/absent, plus create_full_put_path on/off variants
- POST to /, /test/, /test, and to files present/absent (403/405/301/404 outcomes)
- PUT to directories (405 vs 409) and to files: create (201), overwrite (204), nested path with create_full_put_path on (201) vs off (500)
- DELETE files present/absent (204/404/405), DELETE root, DELETE empty vs non-empty directory (recursive 204)
- Exercises interplay of DAV method gating, autoindex, index resolution, and trailing-slash 301 redirect; writes 23 transcript files to output/

**Manques :**
- Not a pass/fail runner against webserv — it only records nginx's responses; no automated diff/comparison step confirms webserv matches the oracle
- Only POST/PUT/DELETE — zero GET / static-file / autoindex-listing coverage (autoindex is toggled but never actually rendered for a GET)
- Fixed 4-byte body (-d "test") with curl-generated Content-Length: no chunked transfer, no large-body / client_max_body_size (413) testing
- Single server on localhost:80 only: no multi-server, no server_name virtual hosts, no multiple ports, no CGI
- No malformed requests, custom/oversized headers, keep-alive, timeouts, concurrency; environment-specific (hardcoded homebrew pidfile /opt/homebrew/var/run/nginx.pid, macOS nginx)

### nginx DAV expected-result spec
`test/nginx_tests/test_nginx_dav.md`

**Couvre :**
- Documentation/oracle: full expected status-code matrices for POST, PUT (dirs and files), and DELETE across all 8 config columns, all marked ✅ verified against nginx
- Explains nginx-specific quirks that webserv must replicate: POST / -> 403 (no resolvable resource), 301 trailing-slash redirect, PUT /test -> 409 conflict on existing dir
- Documents create_full_put_path off -> 500 (not 409), recursive DELETE of non-empty dirs -> 204, and the DELETE / sequencing artifact
- Describes the test setup, config table (6 .conf files), config switcher (use-config.sh), and per-column workflow with setup-idx.sh

**Manques :**
- Static documentation, not executable — encodes expectations but nothing enforces them against webserv
- Same scope ceiling as the harness: write methods only, no GET/static/autoindex-render/CGI/keep-alive/chunked/timeout/multi-server coverage
- ✅ marks confirm nginx's behavior was captured, not that webserv reproduces it — no traceability linking each cell to a webserv assertion
- Legend includes ⬜ à faire / ? incertain states but the committed tables show only ✅, so genuinely untested edge cases are not enumerated

### CGI test fixtures
`www/cgi-bin/`

**Couvre :**
- Script fixtures (not a runner): echo.sh emits Content-Type then cats stdin — validates POST body -> CGI stdin passthrough and header emission
- env.sh dumps sorted environment — intended to inspect CGI meta-variables (REQUEST_METHOD, QUERY_STRING, PATH_INFO, CONTENT_LENGTH, etc.)
- hello.py prints a valid Content-Type header + HTML body — basic GET-driven CGI happy path
- malformed.py prints 'not a header' with no valid CGI header/blank line — fixture for server handling of malformed CGI output
- sleep.py sleeps 60s before responding — fixture for CGI execution-timeout handling
- Covers both interpreter families the parser maps: .sh (bash/sh) and .py (python3)

**Manques :**
- Fixtures only — no harness invokes them or asserts response status/body, so timeout (sleep.py) and malformed-output (malformed.py) behaviors are never actually verified
- No crashing/segfaulting CGI and no non-zero-exit CGI script (500 / 502 handling untested)
- No env.sh assertions: nothing confirms specific meta-variables (QUERY_STRING, PATH_INFO, SCRIPT_NAME, CONTENT_LENGTH, SERVER_PROTOCOL) are set correctly
- No large-output / buffered-output script (tests response streaming and buffer limits), no partial-body-read or infinite-output script
- No coverage of CGI with query strings, path info, GET vs POST method dispatch, or working-directory/chdir behavior

### Official 42 tester binaries
`tester/`

**Couvre :**
- Precompiled black-box graders: tester + ubuntu_tester (canonical 42 webserv tester) and cgi_tester + ubuntu_cgi_tester (CGI grader), macOS and Ubuntu variants each
- The 42 tester generally exercises GET/POST/DELETE, correct status codes, unknown/invalid methods, client_max_body_size enforcement (413), and multi-request handling against a running webserv
- cgi_tester validates CGI execution end-to-end (typically via a provided CGI program), including body passthrough and response assembly
- Provides both macOS-native and ubuntu_* builds so the grader can run on either evaluation platform

**Manques :**
- Opaque compiled binaries — actual assertions are not inspectable from source, so real coverage cannot be confirmed from the repo
- No committed wrapper/config showing how they are launched (which config, which port, expected setup); invocation and pass criteria are undocumented here
- 42 tester is known to use GET/POST/DELETE — no PUT/DAV coverage (complementary to nginx_tests) and it does not deeply stress keep-alive, request timeouts, or high concurrency
- No memory-leak or fd-leak checking (no valgrind/leaks integration around the binary), and no long-run stability/stress harness
- Requires manual platform selection (native vs ubuntu_*); running the wrong-arch binary silently invalidates results

### Trous de couverture globaux

- Request parsing edge cases: no test anywhere sends malformed request lines, bad/unsupported HTTP versions (505), missing Host header, oversized or duplicate headers (431), header-injection, percent-encoded or ../ path-traversal URIs, or overlong URIs (414) — the parser suite only parses config files, not HTTP
- Chunked transfer-encoding: never exercised — nginx_tests always uses a fixed 4-byte curl body with Content-Length; no Transfer-Encoding: chunked request or trailer handling is tested
- Keep-alive / persistent connections: keepalive_timeout is only parsed and logged; no test opens one connection for multiple requests or verifies Connection: keep-alive vs close semantics
- Timeouts: request read/send and keep-alive idle timeouts are entirely untested at runtime; CGI timeout has a fixture (sleep.py) but no harness asserts the server kills it or returns 504
- Status codes: write-method codes (201/204/301/403/404/405/409/500) are captured via the nginx oracle, but GET-path codes — 200, 206 range, 304, 400, 413 body-too-large, 414, 431, 505 — are not asserted in any inspectable suite
- Concurrency / stress: no inspectable test issues parallel or high-volume requests; load and simultaneous-connection behavior depends entirely on the opaque 42 tester
- Memory / fd leaks: nothing runs under valgrind/leaks or checks file-descriptor counts before/after load; no resource-exhaustion or slow-loris style test
- GET / static file serving: not covered by any shell suite (nginx_tests is POST/PUT/DELETE only, parser is config-only) — correct static content, Content-Type, Content-Length, and Last-Modified go unverified except by the opaque tester
- Autoindex: the directive is toggled across nginx_tests configs but only POST/PUT/DELETE are issued, so the actual directory-listing HTML rendered for a GET is never generated or checked
- Redirects: only the implicit 301 trailing-slash redirect is observed; configured return/rewrite redirect directives are neither parsed-tested nor request-tested
- Method restrictions: 405 is exercised via the DAV method matrix, but explicit per-location allow_methods/limit_except (e.g. GET-only locations) and unknown-method 501 handling are untested
- Multi-server routing: every runtime test uses a single server on localhost:80 — no server_name virtual-host selection, no multiple listen ports, no default-server resolution, and the parser never parses multiple server blocks
- CGI timeout/crash: sleep.py (hang) and malformed.py (bad output) exist only as fixtures with no assertions; there is no crashing/segfaulting or non-zero-exit CGI, and correct 500/502/504 handling for CGI failures is unverified in any inspectable suite

---

## 4. Matrice de traçabilité

### 4.1 Sections ↔ code ↔ couverture existante

| Section | Cas (P0) | Composants code | Couverture existante |
|---|---|---|---|
| HTTP request parsing & message framing | 21 (9) | client/Request.cpp, headers/*, handler/RequestHandler.cpp | Aucune (tests existants = config + DAV uniquement) |
| HTTP Methods & Static File Serving | 20 (13) | handler/ExecutionHandler.cpp, ResponseHandler.cpp, client/Response.cpp | Partielle (nginx_tests couvre DAV, pas GET/autoindex/redirect) |
| POST / PUT / DELETE & file upload (WebDAV) | 23 (5) | handler/ExecutionHandler.cpp, config/LocationConfig.cpp | Bonne base (test/nginx_tests, matrice différentielle) — à étendre |
| CGI execution & robustness | 20 (11) | handler/CGIHandler.cpp | Scripts de support seulement (www/cgi-bin/*), aucun test assertif |
| Configuration parsing & routing semantics | 20 (7) | config/*, parser/Parser.cpp | Bonne (test/parser_tests) — routage multi-serveur non couvert |
| Connection management, I/O & non-blocking guarantees | 20 (7) | HTTPServer.cpp, handler/ClientHandler.cpp, client/Client.cpp | Aucune |
| Status Codes & Error Pages | 26 (13) | status/*, client/HTTPError.cpp, config/ErrorPage.cpp | Partielle (codes DAV via nginx_tests) |
| Robustness, Stress, Leaks & Evaluation-Day Checklist | 20 (13) | tout le serveur + tester/ | Testeurs 42 officiels présents (tester/), non automatisés dans un runner |

### 4.2 Exigences du sujet ↔ tests

Reliées par les références `REQ-xx` présentes dans les cas de test. ⚠️ = aucune vérification directe trouvée.

| Exigence | Domaine | Niveau | Testée par |
|---|---|---|---|
| **REQ-01** | cli-config | mandatory | `CFG-09` |
| **REQ-02** | cli-config | mandatory | `CFG-09` |
| **REQ-03** | io-model | mandatory | `CONN-15` |
| **REQ-04** | io-model | mandatory | `CGI-05`, `CONN-01`, `CONN-04`, `CONN-05`, `CONN-07`, `CONN-08`, `CONN-09`, `CONN-12`, `CONN-13`, `CONN-14`, `CONN-20`, `DAV-ROBUST-LOOP`, `HTTP-01`, `HTTP-17`, `STRESS-03`, `STRESS-10`, `STRESS-11` |
| **REQ-05** | io-model | mandatory | `CFG-07`, `CGI-18`, `CONN-09`, `CONN-11`, `CONN-15`, `CONN-17`, `CONN-19`, `STRESS-08`, `STRESS-19` |
| **REQ-06** | io-model | mandatory | `CONN-09`, `CONN-10` |
| **REQ-07** | io-model | mandatory | `CONN-16` |
| **REQ-08** | io-model | mandatory | `CONN-16`, `CONN-20` |
| **REQ-09** | io-model | mandatory | `CONN-16` |
| **REQ-10** | resilience | mandatory | `CGI-05`, `CONN-05`, `CONN-06`, `CONN-07`, `CONN-08`, `HTTP-01`, `HTTP-17`, `STRESS-09`, `STRESS-20` |
| **REQ-11** | resilience | mandatory | `CGI-06`, `CGI-10`, `CGI-11`, `CGI-14`, `CGI-18`, `CMBS-CL-OVERFLOW`, `CONN-11`, `CONN-12`, `CONN-13`, `CONN-14`, `CONN-18`, `CONN-19`, `DAV-ROBUST-LOOP`, `RT-03`, `STRESS-02`, `STRESS-04`, `STRESS-05`, `STRESS-06`, `STRESS-07`, `STRESS-18` |
| **REQ-12** | resilience | mandatory | `CGI-11`, `CONN-06`, `CONN-11`, `CONN-12`, `CONN-17`, `CONN-18`, `DAV-ROBUST-LOOP`, `STRESS-01`, `STRESS-17` |
| **REQ-13** | static-serving | mandatory | `CONN-05`, `CT-MULTIPART-GAP`, `STRESS-16` |
| **REQ-14** | error-handling | mandatory | `HSF-06`, `HSF-07`, `HSF-11` |
| **REQ-15** | error-handling | mandatory | `RT-07`, `SC-12`, `SC-13` |
| **REQ-16** | error-handling | mandatory | `CFG-10`, `CFG-11`, `HSF-06`, `RT-07`, `SC-10`, `SC-11`, `SC-13` |
| **REQ-17** | io-model | mandatory | `CGI-10`, `CONN-15`, `STRESS-14` |
| **REQ-18** | static-serving | mandatory | `CONN-10`, `HSF-01`, `HSF-02`, `HSF-15` |
| **REQ-19** | upload | mandatory | `CT-415-GAP`, `CT-MULTIPART-GAP`, `POST-UPLOAD-GAP`, `PUT-01`, `PUT-02`, `PUT-ROUNDTRIP-DIV`, `SC-02`, `SC-19` |
| **REQ-20** | http-methods | mandatory | `HSF-01`, `SC-01` |
| **REQ-21** | http-methods | mandatory | `CGI-03`, `CGI-19`, `POST-STATIC-01` |
| **REQ-22** | http-methods | mandatory | `DAV-DELETE-ROOT`, `DAV-DIFF-01`, `DEL-01`, `DEL-02`, `DEL-03`, `SC-03` |
| **REQ-23** | virtual-host | mandatory | `RT-05` |
| **REQ-24** | virtual-host | mandatory | `RT-04`, `RT-05` |
| **REQ-25** | routing-config | mandatory | `CFG-08`, `CMBS-413-01`, `CMBS-CHUNKED-413`, `CMBS-CL-OVERFLOW`, `HTTP-16`, `SC-17` |
| **REQ-26** | routing-config | mandatory | `CFG-06`, `DAV-DIFF-01`, `DEL-04`, `HSF-05`, `HSF-13`, `PUT-05`, `SC-14` |
| **REQ-27** | routing-config | mandatory | `HSF-12`, `RT-02`, `RT-08`, `SC-04`, `SC-06` |
| **REQ-28** | routing-config | mandatory | `CFG-12`, `HSF-18`, `HTTP-18`, `RT-01`, `RT-02` |
| **REQ-29** | routing-config | mandatory | `HSF-08`, `HSF-09`, `HSF-19`, `SC-09` |
| **REQ-30** | routing-config | mandatory | `HSF-10` |
| **REQ-31** | upload | mandatory | `POST-UPLOAD-GAP`, `PUT-01`, `PUT-ROUNDTRIP-DIV`, `SC-19` |
| **REQ-32** | cgi | mandatory | `CGI-01`, `CGI-19` |
| **REQ-33** | cgi | mandatory | `CGI-02`, `CGI-15`, `CGI-17` |
| **REQ-34** | cgi | mandatory | `CGI-04`, `HTTP-13` |
| **REQ-35** | cgi | mandatory | `CGI-09` |
| **REQ-36** | cgi | mandatory | ⚠️ — |
| **REQ-37** | cgi | mandatory | `CGI-01`, `CGI-20`, `STRESS-13` |
| **REQ-38** | resilience | mandatory | `CONN-03`, `DAV-DIFF-01`, `STRESS-12`, `STRESS-15` |
| **REQ-39** | http-methods | mandatory | `CHUNKED-PUT-STORE`, `CMBS-CHUNKED-413`, `HTTP-13` |
| **REQ-40** | platform | mandatory | ⚠️ — |
| **REQ-41** | virtual-host | bonus | `RT-04`, `RT-06` |
| **REQ-42** | bonus | bonus | ⚠️ — |
| **REQ-43** | bonus | bonus | `CGI-20` |

> **40/43** exigences du sujet ont au moins un test direct référencé. Les non-couvertes (⚠️) sont reprises au §7.

---

## 5. Registre de risques (analyse du code)

Bugs **suspectés** repérés à la lecture du code — chacun est une cible de test prioritaire.

### Connexions & I/O — `connection-io` (9 risques)

**R01. Client sockets are BLOCKING, so a slow/stalled reader blocks the entire single-threaded event loop inside send()**
- 📍 `libs/common/includes/common/core/net/sockets/TcpServer.hpp:89 (accept never sets O_NONBLOCK; ASocket.cpp:86-91 only reads flags) feeding the blocking send at srcs/client/Client.cpp:342`
- ⚠️ accept() wraps the cfd without SOCK_NONBLOCK/fcntl and setIsNonblock is never called on clients. sendData() calls send() with the full response buffer; on a blocking socket, if the peer's receive window is full the send blocks until the kernel buffer drains — one slow reader hangs all other connections. This is a head-of-line/slow-client DoS and directly violates the 'all I/O must be non-blocking' requirement.

**R02. recv()/send() treat ANY -1 return as a fatal disconnect and read errno after the call**
- 📍 `libs/common/srcs/core/net/sockets/ATcpSocket.cpp:90 and 108 (thrown), caught at srcs/client/Client.cpp:303 and 344`
- ⚠️ The wrapper throws on -1 using strerror(errno); the catch sets E_CLI_DISCONNECTED. Reading errno after read/write is forbidden by the 42 subject. Functionally, EINTR (or, if the socket were ever made non-blocking, EAGAIN/EWOULDBLOCK) would be misclassified as a disconnect, dropping live connections.

**R03. No request/read timeout — Slowloris connections are never reaped**
- 📍 `srcs/handler/ClientHandler.cpp:172 (setLastActivityTime on every E_IN) vs the only timeout check at ClientHandler.cpp:159`
- ⚠️ The single timeout is an idle keep-alive timer that is reset on every received byte. A client trickling one byte at a time keeps the timer alive indefinitely, holding a connection/fd slot forever. There is no separate header-read, body-read, or total-request deadline, so slow-request attacks exhaust resources (unbounded with poll; up to FD_SETSIZE with select).

**R04. No max-simultaneous-clients cap with poll; unbounded client and event maps**
- 📍 `srcs/handler/ClientHandler.cpp:95 (insert) and srcs/HTTPServer.cpp:182 (accept)`
- ⚠️ PollEventIO has no fd limit and nothing enforces a worker_connections/max_clients bound, so connections grow until fd/memory exhaustion. With select the cap is the implicit FD_SETSIZE (SelectEventIO.cpp:87 throws), which merely drops connections rather than applying backpressure.

**R05. Permanent E_OUT registration causes a busy-spin: poll/select returns immediately every iteration**
- 📍 `srcs/handler/ClientHandler.cpp:81 (E_IN|E_OUT registered once, never toggled)`
- ⚠️ A connected client socket is almost always writable, so wait(100ms) returns instantly whenever >=1 client is connected; the loop spins at ~100% CPU and the intended 100ms idle cadence (and session-purge/timeout pacing) never applies. E_OUT should be enabled only when there is buffered response data.

**R06. Graceful shutdown is broken: g_SignalStatus is never set and has internal linkage per translation unit**
- 📍 `includes/webserv/signal.hpp:15 (anonymous namespace) vs srcs/HTTPServer.cpp:154 (while (!g_SignalStatus))`
- ⚠️ Only SIGPIPE is handled (main.cpp:12); no SIGINT/SIGTERM handler assigns g_SignalStatus, so the run loop never exits cleanly and the process dies by default signal action with no client/socket cleanup. Additionally, because g_SignalStatus is defined in an anonymous namespace in a header, each .cpp gets its own copy — a handler in one TU could never affect the instance HTTPServer.cpp observes.

**R07. poll/select failure is logged but not skipped, and can spin on stale events**
- 📍 `srcs/HTTPServer.cpp:158-164`
- ⚠️ On wait() throwing, the catch logs but does not continue; 'ready' is left uninitialized and the code proceeds to iterate servers and call getEvents(), which returns the previous _ready map. A persistent poll error (e.g. EBADF from a stale fd) produces a tight error/reprocess loop instead of recovering or shutting down.

**R08. E_EXCEPT (POLLHUP/POLLERR/POLLNVAL) events are never handled**
- 📍 `srcs/handler/ClientHandler.cpp:167-202 (only E_IN and E_OUT checked)`
- ⚠️ getEvents can report E_EXCEPT (PollEventIO.cpp:222), but the loop only acts on E_IN/E_OUT. A half-closed or errored fd that reports only E_EXCEPT relies on a subsequent recv returning 0/-1 to be cleaned up; if neither E_IN nor E_OUT fires, cleanup is deferred to the keep-alive timeout.

**R09. send-error disconnect is not acted on until the next processClients pass**
- 📍 `srcs/client/Client.cpp:353 (sets DISCONNECTED at ClientHandler.cpp:202) vs the disconnect check at ClientHandler.cpp:175`
- ⚠️ sendData() runs after the disconnect check in the same pass, so a client that errors on send lingers in the map for one extra loop iteration (removed next pass). Minor delayed cleanup, not a hang, but couples cleanup timing to loop scheduling.

### Parsing requête & en-têtes — `request-parsing` (11 risques)

**R10. Segmented/slow header delivery yields a spurious 400. RequestHandler::parseHeaders throws HTTPError(400) whenever CRLFCRLF is not yet present, and there is no 'need more data' return path; Client::processHTTPCycle calls parseHeaders whenever the buffer is non-empty, gated only on bufferRequest.size()>0.**
- 📍 `srcs/handler/RequestHandler.cpp:96 (with Client.cpp:372-374, ClientHandler.cpp:186-193)`
- ⚠️ Any request whose header section spans more than one recv (common with TCP segmentation, slow clients, or byte-drip) is rejected with 400 instead of waiting; only requests that fit a single recv succeed reliably.

**R11. Content-Length vs Transfer-Encoding conflict is only checked for POST/PUT. A GET/HEAD/DELETE carrying both Content-Length and Transfer-Encoding: chunked passes validation.**
- 📍 `srcs/handler/RequestHandler.cpp:314`
- ⚠️ Classic request-smuggling vector: the CL/TE ambiguity is not resolved for non-POST/PUT methods, and the chunked path can then be taken while Content-Length is ignored.

**R12. Request bodies on GET/HEAD are never consumed from the buffer. parseBody is only invoked for POST/PUT/DELETE.**
- 📍 `srcs/client/Client.cpp:381`
- ⚠️ A GET/HEAD with a Content-Length or chunked body leaves the body bytes in _bufferRequest; on a keep-alive connection those bytes are interpreted as the next request -> desync / smuggling.

**R13. All header values are lowercased before storage. Case-sensitive values are corrupted.**
- 📍 `srcs/parser/Parser.cpp:266`
- ⚠️ Cookie/session-id values, Authorization credentials, ETag/If-Match validators, and Content-Type boundary parameters are case-sensitive; downstream consumers read the lowercased (broken) values.

**R14. Content-Type MIME/multipart gating uses exact full-value string equality (findHeader), so any Content-Type carrying parameters fails the allowed-MIME check and multipart is never detected.**
- 📍 `srcs/handler/RequestHandler.cpp:331 (with Request::findHeader, Request.cpp:266)`
- ⚠️ `application/json; charset=utf-8` never equals `application/json`, and `multipart/form-data; boundary=...` never equals `multipart/form-data`, so legitimate parameterized POST/PUT bodies (including all real multipart uploads) get 415 and the CGI multipart branch is effectively dead.

**R15. Percent-decoded path is not re-validated after urlDecode before being used to build the filesystem path.**
- 📍 `srcs/parser/Parser.cpp:216`
- ⚠️ Embedded NUL (%00) or control bytes decode into the path used by buildAbsolutPath; NUL can truncate C-string filesystem calls and decoded control bytes bypass the grammar that validated the raw target.

**R16. Content-Length magnitude is read with istringstream (>>), and the ABNF only bounds it to 1*DIGIT.**
- 📍 `srcs/handler/RequestHandler.cpp:168`
- ⚠️ An overflowing digit string yields implementation-defined/wrapped values; a wrapped-small value could pass the client_max_body_size check while the real declared length is enormous, desyncing body framing.

**R17. Transfer-Encoding accepts no duplicate detection and only the exact token 'chunked'.**
- 📍 `srcs/parser/Parser.cpp:418`
- ⚠️ Multiple Transfer-Encoding: chunked headers are silently accepted (ambiguous framing), while legitimate `gzip, chunked` is rejected with 501 (non-conformance).

**R18. No cap on total header-section size or header count for the main request; CLIENT_HEADER_BUFFER_SIZE (1024) is applied only to chunk-size/trailer lines, not request headers.**
- 📍 `srcs/handler/RequestHandler.cpp:88`
- ⚠️ Aside from the 414 URI guard, a well-formed request-line followed by unbounded header bytes has no size limit; if the segmented-header 400 (gap #1) is fixed, this becomes unbounded memory growth / DoS.

**R19. Chunk-size line is parsed by strtoul(base 16) without ABNF validation.**
- 📍 `srcs/handler/RequestHandler.cpp:543`
- ⚠️ strtoul skips leading whitespace and accepts a leading sign, so inputs like ` a` or `+a` are accepted and `-1` wraps to a huge value (only later caught by the 413 size check); the grammar chunk-size=1*HEXDIG is not enforced.

**R20. For origin-form, the return values of extractSubRule for absolute-path/query are not checked.**
- 📍 `srcs/parser/Parser.cpp:185`
- ⚠️ A silent extraction failure leaves path empty, and the request proceeds with an empty path into location matching / filesystem resolution rather than failing explicitly.

### CGI — `cgi` (11 risques)

**R21. Stdout truncation / data loss on fast-exiting CGI: after a single 8KB read, driveIO calls tryReap which sets _eof=true the moment the child is reaped, and executeCGI then immediately parse()s. Any bytes still buffered in the stdout pipe (child can write up to the ~16KB macOS / ~64KB Linux pipe capacity then exit) are never drained, silently truncating the response body (wrong Content-Length) or turning a valid response into a 502. The stdout fd is still open/valid but never read again once E_EXEC_COMPLETE is set.**
- 📍 `CGIHandler.cpp:291-296 (tryReap sets _eof), CGIHandler.cpp:855-878 (single read per call, no drain loop), ExecutionHandler.cpp:198-205 (parse fires on reap)`
- ⚠️ reap (child exited) is conflated with EOF (stdout drained); these differ whenever output <= pipe capacity, causing silent body truncation for any CGI response larger than BUFFER_SIZE=8KB that exits promptly

**R22. Child inherits all server file descriptors: no FD_CLOEXEC is set and the child only dup2/resets the 4 pipe fds. Listening sockets, other clients' sockets, the poll/select structures' fds, and log file fds all remain open in the CGI process. This leaks fds into every CGI, lets a malicious/buggy script read or write server sockets, and can keep a listening port bound after the server closes it.**
- 📍 `CGIHandler.cpp:184-203 (child branch only handles stdin/stdout pipes)`
- ⚠️ forked child does not close inherited descriptors and no O_CLOEXEC is applied at socket/pipe creation, so every open server fd is duplicated into the CGI process

**R23. Unbounded CGI output buffering: readChunkFromCGI appends to _cgiBuffer with no size cap, and the whole response body is held in memory (_cgiBuffer then _cgiResponseBody) before being sent. A CGI that emits large or continuous output can exhaust server memory before the 30s timeout fires.**
- 📍 `CGIHandler.cpp:861 (_cgiBuffer.insert with no limit); CGIHandler.cpp:329-334 (full body copied again)`
- ⚠️ no maximum response/body size is enforced on CGI output, enabling a memory-exhaustion DoS from a single CGI request

**R24. Destructor does not unregister pipe fds from the multiplexer: ~CGIHandler only kills/reaps the child. The UniqueFd closes the OS fd, but the multiplexer's _events map still holds the (now-closed) fd numbers. reset() is the only path that removes them, and removeClient only calls reset() when isCgiRoute() is currently true (ClientHandler.cpp:114-115); on shutdown clients are destroyed directly. Stale/closed fds can linger in the poll set and collide once the fd number is reused (poll returns POLLNVAL / wrong client gets the event).**
- 📍 `CGIHandler.cpp:67-74 (destructor); ClientHandler.cpp:114-115 (reset guarded by isCgiRoute())`
- ⚠️ fd removal from the multiplexer lives only in reset(), not in the destructor, so any Client teardown path that bypasses reset() leaves dangling entries in the poll set

**R25. Hardcoded SERVER_PORT="8080" and SERVER_NAME="webserv" meta-variables. Wrong whenever the server listens on a different or multiple ports, or serves virtual hosts. CGI apps that construct absolute URLs, enforce port checks, or branch on server name will misbehave.**
- 📍 `CGIHandler.cpp:655 (SERVER_NAME), CGIHandler.cpp:658 (SERVER_PORT)`
- ⚠️ values are literals rather than derived from the listening socket / Host header, violating CGI meta-variable correctness

**R26. PATH_INFO and PATH_TRANSLATED are always empty; there is no PATH_INFO extraction from the request target. CGI scripts that route on PATH_INFO (a very common pattern) receive nothing.**
- 📍 `CGIHandler.cpp:652-653`
- ⚠️ no path-info splitting between the script path and trailing path is implemented

**R27. REDIRECT_STATUS is never set. php-cgi refuses to execute without it (anti-direct-invocation safeguard), so PHP CGI will not run.**
- 📍 `CGIHandler.cpp:624-691 (buildEnv, no REDIRECT_STATUS)`
- ⚠️ a commonly-required non-standard env var for php-cgi is omitted

**R28. Abnormal exit discards otherwise-valid output: WIFSIGNALED unconditionally becomes 502 even if the script produced a complete valid response before being signaled, and nonzero exit only 502s when the buffer is empty (inconsistent handling).**
- 📍 `CGIHandler.cpp:760-769`
- ⚠️ exit-status handling ignores already-buffered valid output for the signal case and is asymmetric for the nonzero-exit case

**R29. chdir(scriptDir) return value ignored: if chdir fails the script runs in the server cwd and any relative file access in the script silently breaks.**
- 📍 `CGIHandler.cpp:195`
- ⚠️ unchecked chdir means a silent working-directory mismatch for the CGI process

**R30. Event-loop busy-spin (not CGI-specific but interacts with CGI): client sockets are registered E_IN|E_OUT and a connected socket is almost always writable, so poll returns immediately every iteration and the 100ms IO_TIMEOUT_MS never idles while any client (including one running a CGI) is connected -> ~100% CPU. It does, however, incidentally guarantee checkTimeout runs frequently.**
- 📍 `ClientHandler.cpp:81 (E_IN|E_OUT registration); HTTPServer.cpp:159-161`
- ⚠️ persistently registering client sockets for E_OUT makes poll return without blocking, spinning the loop under any active connection

**R31. Wall-clock timeout: checkTimeout uses time(NULL); a backward system-clock jump could delay or prevent the 504, letting a runaway CGI persist longer than intended.**
- 📍 `CGIHandler.cpp:783-784`
- ⚠️ elapsed time is computed from wall-clock time rather than a monotonic clock

### Méthodes & réponses — `methods-and-responses` (13 risques)

**R32. Response::_shouldCloseConnection is never initialized in the constructor, and is copied by neither the copy-constructor nor operator=; buildHeaders only ever sets it true (on close) and never false, so the keep-alive path reads an indeterminate value that Client::resetAll uses to decide whether to disconnect**
- 📍 `srcs/client/Response.cpp:18 (ctor), 39 (copy), 52 (op=); set at srcs/handler/ResponseHandler.cpp:409; consumed at srcs/client/Client.cpp:520`
- ⚠️ Undefined behavior: keep-alive connections may be closed or kept open nondeterministically, causing flaky connection reuse

**R33. 304 responses hardcode Content-Length: 1 with an empty body**
- 📍 `srcs/handler/ResponseHandler.cpp:280`
- ⚠️ Advertised length exceeds the 0-byte body; on a keep-alive connection the client blocks waiting for one more byte or mis-frames the next response

**R34. Connection defaults to keep-alive unconditionally regardless of HTTP version; only an explicit 'close' token triggers close**
- 📍 `srcs/handler/ResponseHandler.cpp:388`
- ⚠️ HTTP/1.0 clients (default close) receive Connection: keep-alive and the server won't close, risking hung connections and non-conformance

**R35. Error responses (buildErrorResponse) never emit a Connection header and never call setShouldCloseConnection**
- 📍 `srcs/handler/ResponseHandler.cpp:197`
- ⚠️ After an error the keep-alive/close decision is left to the uninitialized flag; error responses also silently omit keep-alive signaling

**R36. Date header formatted with strftime '%Z' over gmtime instead of the literal 'GMT'**
- 📍 `srcs/handler/ResponseHandler.cpp:385 (and 214)`
- ⚠️ RFC 7231 IMF-fixdate must end in 'GMT'; %Z over gmtime is platform-dependent and can produce 'UTC' or an empty string, yielding a non-conformant Date

**R37. getFileSize returns int (truncated from off_t) and feeds Content-Length**
- 📍 `srcs/handler/ExecutionHandler.cpp:781`
- ⚠️ Files >2GB overflow to a wrong/negative Content-Length, corrupting response framing for large files

**R38. MIME lookup uses a case-sensitive, dot-less extension key with no normalization**
- 📍 `srcs/handler/ExecutionHandler.cpp:366 and 797`
- ⚠️ Uppercase or mixed-case extensions (e.g. .HTML, .JPG) miss the types map and are served as default_type (usually application/octet-stream)

**R39. Autoindex HTML inserts directory path and entry filenames without HTML-escaping**
- 📍 `srcs/handler/ExecutionHandler.cpp:945 and 968`
- ⚠️ Filenames containing HTML/quote metacharacters produce broken markup or reflected-content/XSS in the generated listing

**R40. Trailing-character indexing absPath[absPath.size()-1] is done without an empty-string guard**
- 📍 `srcs/handler/ExecutionHandler.cpp:278, 463, 497`
- ⚠️ If absPath is ever empty, size()-1 wraps to a huge index and operator[] is undefined behavior

**R41. HEAD is absent from the default allowed-methods set (GET/POST/DELETE)**
- 📍 `srcs/config/DefaultConfig.cpp:14 (enforced at srcs/handler/RequestHandler.cpp:302)`
- ⚠️ HEAD requests to a location using default allowed_methods are rejected with 405 unless HEAD is explicitly configured, and the Allow header won't advertise it

**R42. POST to any non-CGI static location always errors (no upload/success path)**
- 📍 `srcs/handler/ExecutionHandler.cpp:401`
- ⚠️ Clients performing plain form/file POST to a static route always receive 403/404/405, which may be unintended for an upload endpoint not backed by CGI/PUT

**R43. Response header ordering is reverse-alphabetical due to reverse-iterating the lowercase-keyed map, and each PUT/GET adds Content-Length/Content-Type before Server/Date**
- 📍 `srcs/handler/ResponseHandler.cpp:174 and 336`
- ⚠️ Not a framing bug, but a fragile, non-canonical ordering; combined with the per-key list it makes accidental duplicate headers (e.g. two Content-Length) hard to detect

**R44. Content-Length for static files is captured from stat at open time but the body is streamed later via read()**
- 📍 `srcs/handler/ExecutionHandler.cpp:363 vs 633`
- ⚠️ TOCTOU: a file mutated between stat and full read yields a body length that disagrees with the advertised Content-Length, mis-framing keep-alive responses

### Configuration & parser — `config (config data model + config-file parser)` (11 risques)

**R45. findLocationConfig returns _locationConfigs.front() when nothing matches; if the server has no location blocks the vector is empty and front() is undefined behavior (likely crash) on the first request**
- 📍 `Webserv/srcs/config/ServerConfig.cpp:447`
- ⚠️ A syntactically valid server{} block need not contain any location{} (grammar allows zero); the fallback assumes at least one location exists.

**R46. PREFIX_PRIORITY ('^~') is parsed but matched identically to a plain prefix, so it never suppresses/outranks other locations**
- 📍 `Webserv/srcs/config/ServerConfig.cpp:432-443; modifier set at Webserv/srcs/parser/Parser.cpp:823`
- ⚠️ The modifier only alters a log-message label (line 441); nginx '^~' priority semantics are not implemented, so users relying on it get plain longest-prefix behavior.

**R47. server_name and per-listen default_server are stored but never used to select a server by Host header**
- 📍 `Webserv/srcs/parser/Parser.cpp:784-804 and :745-746; consumer absent in Webserv/srcs/ServerFactory.cpp/ClientHandler.cpp`
- ⚠️ getServerNames() feeds only log formatting and Listen.defaultServer is never read; each ServerConfig maps 1:1 to a Server and the client is pinned to a fixed config, so virtual-host resolution is effectively missing.

**R48. Prefix location matching uses raw string prefix, not path-segment boundaries**
- 📍 `Webserv/srcs/config/ServerConfig.cpp:432`
- ⚠️ location /foo unexpectedly matches /foobar, /foo.txt, etc., which can route requests to the wrong location and bypass intended per-location rules.

**R49. Nested location blocks are flattened into one sibling vector with the child uri stored as written (not prefixed with the parent uri)**
- 📍 `Webserv/includes/webserv/parser/Parser.hpp:355-374`
- ⚠️ Nesting implies containment in nginx, but here a nested 'location /bar' under 'location /foo' becomes a top-level '/bar', changing which paths it matches.

**R50. error_page path templating rewrites ANY literal 'x' in the URI, not just intended placeholders**
- 📍 `Webserv/srcs/parser/Parser.cpp:626-635`
- ⚠️ A legitimate path like /max/error.html would have its 'x' characters replaced by status-code digits, producing an unintended path.

**R51. Size values accept hex/octal because strtoul is called with base 0; overflow guard also contains an odd `&& size != 0` term**
- 📍 `Webserv/includes/webserv/parser/Parser.hpp:151-169`
- ⚠️ client_max_body_size 010 silently means 8 and 0x10 means 16, which is surprising and undocumented; the size!=0 guard could skip a throw if a multiply wrapped to exactly 0.

**R52. Child-scope directives overwrite rather than merge inherited collections (error_page list, types map, cgi_extensions map)**
- 📍 `Webserv/includes/webserv/parser/Parser.hpp:243-245, :283, :305-308`
- ⚠️ Defining any error_page/types/cgi_extension in a location discards all inherited entries of that kind, which is easy to trip over.

**R53. Default allowed_methods = {GET,POST,DELETE} excludes HEAD and PUT even though the grammar accepts them**
- 📍 `Webserv/srcs/config/DefaultConfig.cpp:11-18`
- ⚠️ A location without an explicit allowed_methods cannot serve HEAD, which HTTP/1.1 servers are generally expected to support.

**R54. IPv6 listen address is stored without its surrounding brackets**
- 📍 `Webserv/srcs/parser/Parser.cpp:720-722`
- ⚠️ Downstream bind/getaddrinfo code that expects the bracketed form, or that needs to distinguish host:port from an IPv6 literal, may misparse the address.

**R55. keepalive_timeout at common-directive scope is parsed from the entire directive string (dirs[0]) rather than the extracted value**
- 📍 `Webserv/includes/webserv/parser/Parser.hpp:252 with Webserv/srcs/parser/Parser.cpp:527-541`
- ⚠️ parseTimeoutValue scans first/last digit of the whole 'keepalive_timeout NN ;' text; it works only because the directive name has no digits, which is fragile.

### Status & pages d'erreur — `status-errors` (8 risques)

**R56. 304 response declares Content-Length: 1 but appends a zero-byte body -> framing mismatch; a conformant client blocks waiting for the missing byte. Also 304 (and 204) must not carry Content-Length/body per RFC 9110 sec 15.4.5.**
- 📍 `Webserv/srcs/handler/ResponseHandler.cpp:280-285,346-348`
- ⚠️ Declared length != bytes sent breaks message framing on keep-alive connections.

**R57. 408 Request Timeout is registered but never emitted: an idle/slow client is simply removed and the socket closed with no response.**
- 📍 `Webserv/srcs/handler/ClientHandler.cpp:159-165`
- ⚠️ RFC/subject expect a timed-out in-flight request to receive 408; here it is silently dropped.

**R58. 431 Request Header Fields Too Large is registered but unreachable: oversized header/request lines are reported as 400 instead of 431.**
- 📍 `Webserv/srcs/handler/RequestHandler.cpp:552,567 and Parser.cpp:244-254`
- ⚠️ Header-size overflow returns a generic 400, losing the specific 431 semantics the table advertises.

**R59. Unknown status code -> getStatusCode returns blank message and description, so the status line reason phrase is empty and the default error page has an empty title/heading. Reachable via config-driven redirect codes (redirect.statusCode.getCode()).**
- 📍 `Webserv/srcs/status/StatusCodeRegistry.cpp:126 and Webserv/srcs/handler/ExecutionHandler.cpp:166`
- ⚠️ A config `return 350 ...;` (or any unregistered code) would emit 'HTTP/1.1 350 ' with a blank reason phrase and blank body text.

**R60. Error responses to HEAD requests still append a message body; buildErrorResponse never checks the request method.**
- 📍 `Webserv/srcs/handler/ResponseHandler.cpp:279-348`
- ⚠️ RFC 9110 forbids a body on HEAD responses; clients may mis-frame the next response on keep-alive.

**R61. Custom error page is always served as Content-Type text/html and read with no error/partial-read handling; the path is a raw getRoot()+ep.getPath() concatenation with no normalization or traversal check.**
- 📍 `Webserv/srcs/handler/ResponseHandler.cpp:298-332`
- ⚠️ Non-HTML custom pages get the wrong content type; a read() returning -1 mid-file yields a truncated body still flagged as found; unnormalized join can mis-resolve paths.

**R62. Retry-After is hardcoded to '42' for both 413 and 503, which is semantically meaningless for 413 (payload too large is not a retry-after condition).**
- 📍 `Webserv/srcs/handler/ResponseHandler.cpp:257-258`
- ⚠️ Emits a misleading/placeholder retry hint rather than an accurate or omitted value.

**R63. 3xx redirects are modeled as thrown HTTPErrors and rendered through the error path, so a 301/302 also gets a generated default HTML body; acceptable per RFC but couples redirects to error handling and depends on the Location being present.**
- 📍 `Webserv/srcs/handler/ExecutionHandler.cpp:166,281,418 and ResponseHandler.cpp:216-217`
- ⚠️ If an HTTPError with a redirect code is ever constructed without a Location, a 3xx would be sent with no Location header.

### Sessions & cookies — `session-cookies` (9 risques)

**R64. createSession returns Session BY VALUE (a detached copy) while getSession returns a reference. Any caller that writes session data through the createSession return value silently loses it — the write never reaches the stored copy.**
- 📍 `Webserv/srcs/session/SessionStore.cpp:86-95 (return session; at 94)`
- ⚠️ Inconsistent create-vs-get API is a latent correctness bug the moment login/first-request code tries to persist data on a freshly created session.

**R65. fread() return value is ignored in generateUUID; a short read or read error leaves some of the 16 bytes uninitialized/zero.**
- 📍 `Webserv/srcs/session/SessionStore.cpp:65`
- ⚠️ Partial/failed entropy read yields weak or duplicated UUIDs (session-prediction / collision), and reads uninitialized stack memory — a real security and UB concern; only fopen failure is currently detected.

**R66. No Secure and no SameSite attribute on the sid cookie (defaults secure=false, and ResponseHandler never sets Secure/SameSite).**
- 📍 `Webserv/srcs/session/Cookie.cpp:26-28; Webserv/srcs/handler/ResponseHandler.cpp:426-439`
- ⚠️ Session cookie is transmittable over plaintext and sent cross-site -> session hijacking over HTTP and CSRF exposure. HttpOnly is set, which mitigates only XSS theft.

**R67. serializeCookie() appends _value/_path/_domain verbatim with no validation or escaping.**
- 📍 `Webserv/srcs/session/Cookie.cpp:237-242`
- ⚠️ For sid (hex UUID) it is safe today, but the generic Cookie class allows a value/path containing ';' or CRLF, enabling Set-Cookie attribute smuggling / response header injection if ever populated from user/session data.

**R68. Every cookie-less request creates a new server-side session, with no cap on store size and default TTL of 7 days (604800s); purge only runs every 60s and only removes already-expired entries.**
- 📍 `Webserv/srcs/handler/ResponseHandler.cpp:434-436; Webserv/includes/webserv/config/DefaultConfig.hpp:22`
- ⚠️ Bots/asset fetches/attackers that never echo the cookie cause unbounded memory growth (a session per request accumulating for up to 7 days) — a memory-exhaustion / DoS vector.

**R69. getSession returns a reference into the std::map (SessionStore.cpp:118) and the whole design assumes single-threaded execution (no locking; Meyers singleton init and std::gmtime's shared buffer are not thread-safe).**
- 📍 `Webserv/srcs/session/SessionStore.cpp:37-41,103-119; Webserv/srcs/session/Cookie.cpp:245`
- ⚠️ Safe only because the server is single-threaded (verified: only fork() for CGI, no threads). If concurrency is ever added, purge/erase could dangle the returned reference and gmtime/singleton race — no synchronization exists.

**R70. ttl==0 semantics are contradictory: isExpired() documents expires==0 as 'never expire', but createSession(0) sets expires=now (nonzero) with ttl=0 (no sliding), so the session expires within ~1 second instead of never.**
- 📍 `Webserv/srcs/session/SessionStore.cpp:90-91; Webserv/srcs/session/Session.cpp:145-147`
- ⚠️ A configured session-ttl of 0 yields useless ~1-second sessions rather than the intended infinite/browser-session lifetime; the 'never expires' branch is effectively dead for stored sessions.

**R71. std::gmtime return value is dereferenced without a NULL check.**
- 📍 `Webserv/srcs/session/Cookie.cpp:245`
- ⚠️ gmtime can return NULL for out-of-range time_t; dereference would crash. Low likelihood (expires is near-future) but unguarded.

**R72. Set-Cookie is re-emitted on EVERY response for an existing valid session, and the setLevel(DEBUG) call plus INFO logs fire on every Cookie/Session construction.**
- 📍 `Webserv/srcs/handler/ResponseHandler.cpp:428; Webserv/srcs/session/Cookie.cpp:30-32; Webserv/srcs/session/Session.cpp:26-28`
- ⚠️ Redundant Set-Cookie on every hit is chatty and forces a persistent (Expires-bearing) cookie; per-object logger.setLevel(DEBUG) hard-codes debug logging and adds per-request overhead/log noise.

---

## 6. Catalogue de tests

### 6.1 HTTP request parsing & message framing

_Black-box, raw-byte tests sent via nc/printf against the configured listener (default localhost:8080 across default.conf/blucken.conf; adjust to the running config's listen). Focus is message framing: the request-line ABNF, header syntax, the mandatory Host rule, Content-Length vs Transfer-Encoding, chunked decoding, size limits, and the smuggling/DoS surface a 42 evaluator will attack. Each case cites the 42 subject requirement, RFC 9112/9110/3875 section, and the code-analysis gap it targets. Standout adversarial cases grounded in the code: (1) HTTP-01 segmented header delivery — parseHeaders throws 400 the instant CRLFCRLF is absent (RequestHandler.cpp:108), so any request split across TCP segments is spuriously rejected while nginx buffers; this is the single most likely eval-failing/crash-adjacent defect. (2) HTTP-11/HTTP-12 request smuggling — CL+TE conflict and body consumption are only handled for POST/PUT/DELETE (validateHeaders:314, Client.cpp:381), so a GET carrying both CL+TE or a bare CL body desyncs a keep-alive connection. (3) HTTP-14 chunk-size parsing via strtoul accepts leading space/sign and must not overflow/crash on 16+ hex digits. (4) HTTP-05 version handling — HTTP/1.0 is wrongly 505 (Parser.cpp:166) where nginx serves it. Differential cases specify nginx's reference behavior. Priorities: P0 = crash/hang/leak/smuggling/mandatory-framing; P1 = important status-code correctness; P2 = SHOULD-level/tolerance. All are automatable in a shell/pytest harness; those depending on write-enabled routes (chunked roundtrip) name the config precondition. Every P0 either guards against a crash/hang/desync or exercises a confirmed suspected_gap._

#### `HTTP-01` · P0 · robustness — Segmented header delivery must be buffered, not rejected (suspected gap #1)
**Réf :** REQ-04/REQ-10; RFC 9112 §2.2 (incremental parsing); code gap #1 RequestHandler.cpp:108 · **Outil :** nc / python3 socket · **Auto :** oui

**Pré :** Server running on localhost:8080. A second, unrelated connection is available to confirm liveness.

**Procédure :**

```sh
Send the header section across two TCP segments with a pause:
{ printf 'GET / HTTP/1.1\r\n'; sleep 1; printf 'Host: x\r\n\r\n'; } | nc localhost 8080
Alt (deterministic) python:
python3 -c "import socket,time; s=socket.create_connection(('127.0.0.1',8080)); s.sendall(b'GET / HTTP/1.1\r\n'); time.sleep(1); s.sendall(b'Host: x\r\n\r\n'); print(s.recv(4096))"
```

**Attendu :** nginx: buffers and returns 200/404 (one normal response). webserv (suspected): returns 400 'CRLFCRLF not found' on the first partial recv — a CONFIRMED framing bug. Test PASSES only if a normal 200/404 is returned; a 400 here is a defect. Server MUST NOT crash and MUST still serve a concurrent request.

#### `HTTP-02` · P1 · integration — Malformed request-line (double SP / missing version / empty target) -> 400
**Réf :** RFC 9112 §3 / §2.2 (R01, R29); R-400; Parser.cpp:140 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'GET  / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080   # two SP after method
printf 'GET /\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080              # missing HTTP-version
printf 'GET  HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080       # empty request-target
printf 'GET / HTTP/1.1 \r\nHost: x\r\n\r\n' | nc -w2 localhost 8080     # trailing SP
```

**Attendu :** Each returns 400 Bad Request (ABNF request-line = method SP request-target SP HTTP-version fails on extra/missing SP, missing version, empty target). Never 200. Matches nginx.

#### `HTTP-03` · P1 · integration — Unknown / lowercase / oversized method -> 501 Not Implemented
**Réf :** RFC 9112 §3.4 (R03); RFC 9110 R-501; Parser.cpp:172 · **Outil :** nc / python3 · **Auto :** oui

**Procédure :**

```sh
printf 'BREW / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080         # unknown
printf 'get / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080          # wrong case
printf 'PATCH / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080        # real but unimpl
printf '%s / HTTP/1.1\r\nHost: x\r\n\r\n' "$(python3 -c 'print("A"*5000)')" | nc -w2 localhost 8080  # 5000-octet method
```

**Attendu :** All four return 501 Not Implemented (config::strToMethod exact-match only accepts GET/HEAD/POST/PUT/DELETE). Server must not crash on the 5000-octet method. Distinguish from 405 (which requires a known method disallowed on a route).

#### `HTTP-04` · P1 · integration — Request-target length boundary: 2049 bytes -> 414, ~2000 bytes accepted
**Réf :** RFC 9112 §3.4/§3 (R02, R04); RFC 9110 R-414; Parser.cpp:158, DefaultConfig URI_MAX_LENGTH=2048 · **Outil :** nc / python3 · **Auto :** oui

**Procédure :**

```sh
printf 'GET /%s HTTP/1.1\r\nHost: x\r\n\r\n' "$(python3 -c 'print("a"*2049)')" | nc -w2 localhost 8080   # over URI_MAX_LENGTH(2048)
printf 'GET /%s HTTP/1.1\r\nHost: x\r\n\r\n' "$(python3 -c 'print("a"*2000)')" | nc -w2 localhost 8080   # under limit
printf 'GET /%s HTTP/1.1\r\nHost: x\r\n\r\n' "$(python3 -c 'print("a"*7900)')" | nc -w2 localhost 8080   # RFC 8000-octet recommendation
```

**Attendu :** 2049-byte target -> 414 URI Too Long. 2000-byte target -> NOT 414 (200/404, parsed normally). 7900-byte target -> webserv returns 414 (URI_MAX_LENGTH=2048), diverging from RFC 9112 §3 SHOULD-support of 8000 octets; note this as a SHOULD-level non-conformance (nginx default large_client_header_buffers would 414 too, so acceptable). No crash.

#### `HTTP-05` · P1 · differential-vs-nginx — HTTP version handling: 1.0/2.0/malformed (HTTP/1.0 differential BUG)
**Réf :** RFC 9112 §2.3 (R27); RFC 9110 R-505 / R-VERSION-MINOR; Parser.cpp:166 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'GET / HTTP/1.0\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080     # older minor
printf 'GET / HTTP/2.0\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080     # unsupported major
printf 'GET / HTTP/1.11\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080    # malformed version token
printf 'GET / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080     # control (200/404)
```

**Attendu :** HTTP/2.0 -> 505 HTTP Version Not Supported (correct). HTTP/1.11 -> 400 (malformed HTTP-version = DIGIT '.' DIGIT). HTTP/1.0 -> nginx returns 200 (major version supported regardless of minor); webserv returns 505 because Parser.cpp:166 hard-codes literal 'HTTP/1.1' — CONFIRMED conformance bug (RFC 9110 R-VERSION-MINOR). Flag the HTTP/1.0 divergence.

#### `HTTP-06` · P0 · integration — Missing Host header on HTTP/1.1 -> 400 (mandatory)
**Réf :** RFC 9112 §3.2 (R10); RFC 9110 R-HOST; RequestHandler.cpp:308 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'GET / HTTP/1.1\r\n\r\n' | nc -w2 localhost 8080
```

**Attendu :** 400 Bad Request (E_PARS_HOST unset -> validateHeaders throws 400). Matches nginx. This is an RFC MUST and a common eval probe.

#### `HTTP-07` · P1 · integration — Duplicate Host and invalid Host value -> 400
**Réf :** RFC 9112 §3.2 (R11, R12); Parser.cpp:269 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'GET / HTTP/1.1\r\nHost: a.com\r\nHost: b.com\r\n\r\n' | nc -w2 localhost 8080   # duplicate
printf 'GET / HTTP/1.1\r\nHost: exa mple.com\r\n\r\n' | nc -w2 localhost 8080          # embedded space
printf 'GET / HTTP/1.1\r\nHost: \r\n\r\n' | nc -w2 localhost 8080                       # empty authority
```

**Attendu :** Duplicate Host -> 400 (parser rejects second Host). Space-in-value -> 400 (Host ABNF fails). Empty Host -> 400. All match nginx.

#### `HTTP-08` · P1 · integration — Header syntax defenses: WS-before-colon, obs-fold, bare CR -> 400
**Réf :** RFC 9112 §5.1/§5.2/§2.2 (R06, R07, R08, R09); Parser.cpp:251 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'GET / HTTP/1.1\r\nHost: x\r\nX-Test : v\r\n\r\n' | nc -w2 localhost 8080          # space before colon
printf 'GET / HTTP/1.1\r\nHost: x\r\nX-Fold: a\r\n b\r\n\r\n' | nc -w2 localhost 8080      # obs-fold continuation
printf 'GET / HTTP/1.1\r\nHost: x\rY: z\r\n\r\n' | nc -w2 localhost 8080                    # bare CR in header
printf 'GET / HTTP/1.1\r\n Host: x\r\n\r\n' | nc -w2 localhost 8080                          # WS before first header
```

**Attendu :** All -> 400 Bad Request. Space-before-colon (RFC 9112 §5.1 MUST reject), obs-fold (§5.2: reject or SP-fold — webserv rejects), bare CR (§2.2: invalid), and leading-WS line before first header (§2.2 smuggling defense) must never be interpreted as a valid header. Never 200. The bare-CR line must NOT be silently split into a second header.

#### `HTTP-09` · P2 · differential-vs-nginx — Line-terminator handling: bare-LF and leading empty line (differential)
**Réf :** RFC 9112 §2.2 (R05, R28); RequestHandler.cpp:111 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'GET / HTTP/1.1\nHost: x\r\n\r\n' | nc -w2 localhost 8080         # bare-LF request-line, CRLF headers
printf 'GET / HTTP/1.1\nHost: x\n\n' | nc -w2 localhost 8080               # all bare-LF
printf '\r\nGET / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080     # leading empty CRLF line
```

**Attendu :** Case 1 (bare-LF request-line + CRLF header section): webserv tolerates the request line (strips optional CR) and returns 200/404 (RFC 9112 §2.2 MAY). Case 2 (all bare-LF, no CRLFCRLF): returns 400 deterministically, MUST NOT hang/crash. Case 3 (leading CRLF): nginx ignores one empty line and returns 200; webserv returns 400 because it treats the empty first line as the request-line (RequestHandler.cpp:111) — note this §2.2 SHOULD divergence.

#### `HTTP-10` · P0 · integration — Content-Length errors: non-digit, negative, duplicate/conflicting -> 400
**Réf :** RFC 9112 §6.3 item 5 / §6.2 (R13, R14); Parser.cpp:299 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: abc\r\n\r\n' | nc -w2 localhost 8080
printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: -1\r\n\r\n' | nc -w2 localhost 8080
printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 0x10\r\n\r\n' | nc -w2 localhost 8080
printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\nContent-Length: 6\r\n\r\nhello' | nc -w2 localhost 8080
```

**Attendu :** All -> 400 Bad Request and connection close (Content-Length = 1*DIGIT; negative/non-numeric/hex fail the ABNF; two differing CL values are an unrecoverable framing error). Conflicting CL is a classic smuggling vector and MUST be 400. Matches nginx.

#### `HTTP-11` · P0 · differential-vs-nginx — CL+TE conflict: POST 400 vs GET smuggling bypass (suspected gap #2)
**Réf :** RFC 9112 §6.3 item 3 / §6.1 (R15); code gap #2 RequestHandler.cpp:314 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
# A) POST with both -> handled
printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n' | nc -w2 localhost 8080
# B) SAME on GET -> gap probe
printf 'GET / HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n' | nc -w2 localhost 8080
```

**Attendu :** A) POST -> 400 and close (validateHeaders rejects CL+TE for POST/PUT). B) GET -> webserv does NOT reject (CL/TE conflict only checked for POST/PUT, RequestHandler.cpp:314) and never consumes the body — CONFIRMED smuggling gap. Correct/nginx behavior: reject with 400 or process per TE and close the connection; TE and CL must never both be honored. A 200 on (B) with the connection kept open is a defect: assert either 400/close, or that the connection is closed after the response.

#### `HTTP-12` · P0 · robustness — GET/HEAD body not consumed -> pipelining desync/smuggling (suspected gap #3)
**Réf :** RFC 9112 §6.3 (body framing) / §9.3 (R24, R26); RFC 3875 CGI-BODY-LENGTH-BOUND; code gap #3 Client.cpp:381 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Keep-alive connection (no Connection: close on the first request).

**Procédure :**

```sh
python3 -c "import socket; s=socket.create_connection(('127.0.0.1',8080)); s.sendall(b'GET / HTTP/1.1\r\nHost: x\r\nContent-Length: 44\r\n\r\nGET /admin HTTP/1.1\r\nHost: x\r\nDummy: 1\r\n\r\n'); import time; time.sleep(0.5); print(repr(s.recv(8192)))"
# The 44-byte 'body' is itself a smuggled second request line.
```

**Attendu :** Server must consume exactly the declared body (or reject the GET body) so the smuggled bytes are NOT parsed as a second request. webserv (suspected): parseBody runs only for POST/PUT/DELETE (Client.cpp:381), so the Content-Length body of a GET stays in _bufferRequest and is re-parsed as the next request -> the smuggled 'GET /admin' is answered -> CONFIRMED desync. Assert only ONE response is returned for one logical request, and that '/admin' is not served from the body. No crash.

#### `HTTP-13` · P0 · integration — Chunked decoding correctness: hex size, chunk-ext, trailer (roundtrip)
**Réf :** RFC 9112 §7.1/§7.1.1/§7.1.2 (R16, R17, R20, R21); REQ-34/REQ-39; RequestHandler.cpp:468 · **Outil :** nc · **Auto :** oui

**Pré :** A write-enabled route: PUT allowed with dav_put_path storage (e.g. /uploads or /webdav in blucken.conf). If unavailable, use the CGI echo alternative below against /cgi-bin/echo.sh.

**Procédure :**

```sh
# Upload a chunked body exercising hex size (A=10), ignored ;ext, and a trailer:
printf 'PUT /uploads/chunk.txt HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nA\r\n0123456789\r\n5;ext=1\r\nhello\r\n0\r\nX-Tr: v\r\n\r\n' | nc -w2 localhost 8080
# Read it back:
printf 'GET /uploads/chunk.txt HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w2 localhost 8080
# CGI alt (no write route):
printf 'POST /cgi-bin/echo.sh HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n' | nc -w2 localhost 8080
```

**Attendu :** PUT -> 201/204; readback body is EXACTLY the 15 bytes '0123456789hello' — proving hex chunk-size (A=10 not 10 decimal), that ';ext=1' is ignored and not counted, and that the 'X-Tr: v' trailer is accepted-and-ignored (not merged, not affecting length). CGI echo alt -> stdin/body 'hello' (5 bytes). Matches nginx un-chunking.

#### `HTTP-14` · P0 · robustness — Malformed / lenient / overflowing chunk-size (suspected gap #10, no crash)
**Réf :** RFC 9112 §7.1 (R18, R19); code gap #10 RequestHandler.cpp:543-568 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nXYZ\r\nhello\r\n0\r\n\r\n' | nc -w2 localhost 8080   # non-hex
printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n 5\r\nhello\r\n0\r\n\r\n' | nc -w2 localhost 8080    # leading space
printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n+5\r\nhello\r\n0\r\n\r\n' | nc -w2 localhost 8080    # leading sign
printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nFFFFFFFFFFFFFFFF\r\n' | nc -w2 localhost 8080          # 16 hex (2^64-1)
printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nFFFFFFFFFFFFFFFFF\r\n' | nc -w2 localhost 8080         # 17 hex (ERANGE)
```

**Attendu :** Non-hex 'XYZ' -> 400 and close. ' 5' and '+5' -> nginx returns 400 (chunk-size=1*HEXDIG); webserv leniently ACCEPTS them because strtoul(...,16) skips leading whitespace/sign (RequestHandler.cpp:563) — CONFIRMED gap #10; assert at minimum no crash and deterministic behavior, ideally 400. 16-F size -> 413 (exceeds client_max_body_size) or controlled close; 17-F -> 400 (ERANGE). The server MUST NOT overflow, wrap the size, hang, or crash on any of these.

#### `HTTP-15` · P1 · integration — Transfer-Encoding variants: unknown 501, non-final, double-chunked (gap #8)
**Réf :** RFC 9112 §6.1/§6.3 item 4 (R22, R23); code gap #8 Parser.cpp:418 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: boguscoding\r\n\r\n' | nc -w2 localhost 8080          # unknown coding
printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked, gzip\r\n\r\n' | nc -w2 localhost 8080        # chunked not final
printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n' | nc -w2 localhost 8080  # duplicate TE
```

**Attendu :** Unknown coding -> 501 Not Implemented. 'chunked, gzip' (chunked not final) -> webserv returns 501 (value != exact 'chunked'); RFC 9112 §6.3 item 4 prefers 400+close — note divergence, both are error/close so acceptable. Duplicate 'Transfer-Encoding: chunked' -> webserv accepts silently (no duplicate detection, gap #8); nginx treats repeated TE as ambiguous framing. Assert no crash and the connection is closed after any TE error; never both CL and TE honored.

#### `HTTP-16` · P0 · integration — Body size limit enforcement -> 413 (Content-Length and chunked)
**Réf :** REQ-25; RFC 9110 R-413; RequestHandler.cpp:172/485 · **Outil :** nc / python3 · **Auto :** oui

**Pré :** A route/server with a known small client_max_body_size (default http-level 1k in default.conf). Adjust the over-limit size to exceed the configured value.

**Procédure :**

```sh
# Content-Length over limit (declare 2000000 on a 1k route):
printf 'POST /uploads/big HTTP/1.1\r\nHost: x\r\nContent-Length: 2000000\r\n\r\n' | nc -w2 localhost 8080
# Chunked body whose decoded total exceeds the limit:
{ printf 'POST /uploads/big HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n'; python3 -c 'import sys; [sys.stdout.write("400\r\n"+"A"*1024+"\r\n") for _ in range(4)]; sys.stdout.write("0\r\n\r\n")'; } | nc -w2 localhost 8080
```

**Attendu :** Both -> 413 Content Too Large. For Content-Length, the declared size is rejected before/without storing the body. For chunked, the cumulative decoded size is checked per chunk (RequestHandler.cpp:485) and 413 is raised once it crosses client_max_body_size. A within-limit body is accepted normally. Matches nginx.

#### `HTTP-17` · P0 · robustness — Incomplete/short body must not hang the server (timeout/close)
**Réf :** REQ-10/REQ-04; RFC 9112 §8 (R30); ClientHandler.cpp:159 · **Outil :** python3 / curl · **Auto :** oui

**Pré :** Known keepalive_timeout (default 120s at http level; set a short value e.g. 5s for a fast run). A concurrent connection to confirm the loop is not blocked.

**Procédure :**

```sh
# Declare CL:100 but send only ~15 bytes, then hold the socket open:
python3 -c "import socket,time; s=socket.create_connection(('127.0.0.1',8080)); s.sendall(b'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 100\r\n\r\nonly-a-few-bytes'); s.settimeout(140); \ntry:\n  print('recv:', repr(s.recv(4096)))\nexcept Exception as e:\n  print('closed/err:', e)"
# Meanwhile, from another shell:
curl -s -o /dev/null -w '%{http_code}\n' --max-time 5 http://localhost:8080/
```

**Attendu :** The stalled connection is eventually closed (or answered 408) by the server within keepalive_timeout — it MUST NOT hang indefinitely. The concurrent curl returns 200/404 within its 5s budget, proving the single event loop is not blocked by the slow client. Note: 408 is registered but never emitted (the code just closes on idle timeout) — a silent-drop divergence from RFC 9112 §8 SHOULD; acceptable as long as it does not hang.

#### `HTTP-18` · P1 · robustness — Path traversal and embedded NUL in percent-decoded target (suspected gap #6)
**Réf :** REQ-28 (path mapping/sandbox); RFC 9110 R-403; code gap #6 Parser.cpp:216 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'GET /%%2e%%2e/%%2e%%2e/etc/passwd HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080      # encoded ../../
printf 'GET /..%%2f..%%2f..%%2fetc/passwd HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080       # mixed encoded slash
printf 'GET /foo%%00.txt HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080                        # percent-encoded NUL
```

**Attendu :** Traversal attempts -> 403 (or 404): normalizePath collapses ../ then the root-prefix check rejects escapes; /etc/passwd is never served. NUL case: the percent-decoded path carries an embedded 0x00 (gap #6, no re-validation after urlDecode); the server MUST NOT crash and MUST NOT serve an unintended file — expect 404/400, not a 200 disclosing '/foo'. Assert the process is still alive afterward (fresh curl 200).

#### `HTTP-19` · P2 · integration — Request-target forms: absolute-form authority-override, bad scheme, asterisk
**Réf :** RFC 9110 R-ABSOLUTE-FORM / R-ORIGIN-FORM; RFC 9112 §3.2; Parser.cpp:183 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'GET http://a.example/x HTTP/1.1\r\nHost: b.example\r\n\r\n' | nc -w2 localhost 8080   # absolute-form, Host overridden
printf 'GET ftp://a.example/x HTTP/1.1\r\nHost: a.example\r\n\r\n' | nc -w2 localhost 8080    # non-http scheme
printf 'GET * HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080                             # asterisk-form
```

**Attendu :** absolute-form -> accepted, resolved to /x with the target authority 'a.example' taking precedence over Host 'b.example' (200/404, not 400). ftp scheme -> 400 (only http/https accepted). asterisk-form 'GET *' -> 400 (asterisk-form only valid for OPTIONS, which webserv doesn't implement anyway). Consistent with nginx accepting absolute-form.

#### `HTTP-20` · P1 · integration — Persistent connection & in-order pipelining; Connection: close honored
**Réf :** RFC 9112 §9.3/§9.6 (R25, R26); ResponseHandler.cpp:388 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
# Two pipelined requests on one keep-alive connection, second asks to close:
printf 'GET / HTTP/1.1\r\nHost: x\r\n\r\nGET / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w3 localhost 8080
# Explicit close on a single request:
printf 'GET / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w3 localhost 8080
```

**Attendu :** First case: TWO complete responses in request order, then the server closes after the second (nc reaches EOF/exits). Second case: exactly one response, then the socket is closed by the server (SHOULD carry Connection: close). HTTP/1.1 defaults to persistent; absent 'close' the connection stays open. (Note the code emits Connection: keep-alive unconditionally regardless of version — a separate response-side concern.)

#### `HTTP-21` · P1 · integration — Body-length precedence: 411 for bodyless POST, no-hang for bodyless GET
**Réf :** RFC 9112 §6.3 item 7 (R24); RFC 9110 R-411; RequestHandler.cpp:314 · **Outil :** nc · **Auto :** oui

**Procédure :**

```sh
printf 'POST / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080   # POST, no CL and no TE
printf 'GET / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080    # GET, no body
```

**Attendu :** POST with neither Content-Length nor Transfer-Encoding -> 411 Length Required (validateHeaders:319). GET with no CL/TE -> immediate 200/404 (message body length is zero, no waiting) — MUST NOT block waiting for a body. Both return within the -w2 window.

---

### 6.2 HTTP Methods & Static File Serving

_All 20 cases were black-box-validated against the actual webserv binary (Webserv/webserv) using a canonical test config ("config T"): single server on localhost:8080, root = Webserv/www, default_type text/html, error_page 404 /404.html, with this location/method matrix — location / {autoindex on; index index.html index.htm default.html; methods GET HEAD POST PUT DELETE}; location /api {methods GET POST; default_type application/json; error_page 404 /api/not-found.html}; location /static {methods GET HEAD; autoindex on}; location /uploads {autoindex off; methods GET HEAD}; location = /old-page {return 301 http://example.com/new-page}; location /legacy {return 302 /modern}; location /gone {return 410}. NOTE: the server needs ~3-4s to load its ABNF grammar before it binds — sleep 4 after launch before probing. Path mapping is nginx `root`-style (full request URI is appended to root; the location prefix is NOT stripped), confirmed via /noai→www/noai. Empirically CONFIRMED webserv behaviors baked into the expectations below: GET/test.txt→200 text/plain CL6; file.xyz(unknown ext)→text/html(default_type); autoindex listing works but (bug) leaks the ABSOLUTE filesystem path in <title>/<h1> and does NOT HTML-escape entry names (XSS); HEAD→405 on GET/POST-only /api (HEAD missing from allowed set — differs from nginx); uppercase .CSS→text/html not text/css (case-sensitive dot-less MIME lookup — differs from nginx); Date header ends in "UTC" not "GMT" (RFC/nginx violation); HTTP/1.0→505 (hardcoded HTTP/1.1 — differs from nginx 200); BREW/OPTIONS/lowercase-get→501; hugefile.txt(2.79MB)→md5-exact, correct Content-Length, no truncation; /old-page→301, /legacy→302, /gone→410. The differential-vs-nginx cases (HSF-03/05/11/16/17/19) directly target suspected_gaps in the code analysis (ExecutionHandler MIME/autoindex, Parser version hardcode, ResponseHandler Date, DefaultConfig HEAD omission). Substitute the actual configured port for :8080. For the eval, run each also against the nginx oracle in test/nginx_tests to capture the reference transcript._

#### `HSF-01` · P0 · integration — GET static file returns 200 with byte-exact body and correct framing headers
**Réf :** REQ-20 / REQ-18 / R-METHOD-GET / R-200 · **Outil :** curl · **Auto :** oui

**Pré :** config T running on :8080; www/test.txt exists (6 bytes: 'hellow')

**Procédure :**

```sh
curl -s -D - -o /tmp/g http://localhost:8080/test.txt | tr -d '\r' | grep -iE 'HTTP/|content-type|content-length' ; cmp /tmp/g /Users/blucken/Documents/webserv/Webserv/www/test.txt && echo BYTE_EXACT
```

**Attendu :** Status line 'HTTP/1.1 200 OK'; 'Content-Type: text/plain'; 'Content-Length: 6'; downloaded body is byte-identical to the file (cmp prints BYTE_EXACT, exit 0)

#### `HSF-02` · P1 · integration — Content-Type derived from file extension across the types map
**Réf :** R-CONTENT-TYPE / REQ-18 · **Outil :** curl · **Auto :** oui

**Pré :** config T; files www/test.txt, www/static/style.css, www/static/app.js, www/api/index.json, www/index_test.html, www/file.xyz exist

**Procédure :**

```sh
for pair in 'test.txt|text/plain' 'static/style.css|text/css' 'static/app.js|application/javascript' 'index_test.html|text/html' 'file.xyz|text/html'; do f=${pair%|*}; want=${pair#*|}; got=$(curl -s -D - -o /dev/null http://localhost:8080/$f | tr -d '\r' | awk 'tolower($1)=="content-type:"{print $2}'); echo "$f got=$got want=$want"; done ; curl -s -D - -o /dev/null 'http://localhost:8080/api/index.json' | tr -d '\r' | grep -i content-type
```

**Attendu :** text/plain, text/css, application/javascript, text/html (for .html AND for the unknown .xyz which falls back to default_type text/html). /api/index.json served under a location with default_type application/json AND json in types → 'application/json'

#### `HSF-03` · P1 · differential-vs-nginx — DIFFERENTIAL: uppercase file extension mis-typed (case-sensitive MIME lookup)
**Réf :** R-CONTENT-TYPE / code-gap ExecutionHandler:366 · **Outil :** curl · **Auto :** oui

**Pré :** config T; create www/UP.CSS (contents 'x{color:red}')

**Procédure :**

```sh
printf 'x{color:red}' > /Users/blucken/Documents/webserv/Webserv/www/UP.CSS; curl -s -D - -o /dev/null http://localhost:8080/UP.CSS | tr -d '\r' | grep -i content-type; rm -f /Users/blucken/Documents/webserv/Webserv/www/UP.CSS
```

**Attendu :** webserv returns 'Content-Type: text/html' (default_type — WRONG: dot-less lookup of key 'CSS' misses the lowercase 'css' entry). nginx returns 'Content-Type: text/css'. Flag as a differential defect (ExecutionHandler.cpp:366 case-sensitive types.find)

#### `HSF-04` · P0 · integration — HEAD returns GET-identical headers with an empty body
**Réf :** R-METHOD-HEAD (rfc-MUST) / R-SAFE-GET-HEAD · **Outil :** curl · **Auto :** oui

**Pré :** config T; www/test.txt (6 bytes)

**Procédure :**

```sh
curl -s -I http://localhost:8080/test.txt | tr -d '\r' | grep -iE 'HTTP/|content-length|content-type' ; echo -n 'HEAD body bytes after headers: '; printf 'HEAD /test.txt HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc localhost 8080 | awk 'f{c+=length($0)+1} /^\r?$/{f=1} END{print c+0}'
```

**Attendu :** 'HTTP/1.1 200 OK', 'Content-Type: text/plain', 'Content-Length: 6' (the Content-Length the equivalent GET would produce), and ZERO body bytes after the blank line

#### `HSF-05` · P1 · differential-vs-nginx — DIFFERENTIAL: HEAD rejected 405 on a GET-only route (HEAD absent from allowed set)
**Réf :** REQ-26 / R-METHOD-HEAD / code-gap RequestHandler:302 · **Outil :** curl · **Auto :** oui

**Pré :** config T; location /api allows only {GET, POST}; www/api/index.json exists

**Procédure :**

```sh
curl -s -D - -o /dev/null -I http://localhost:8080/api/index.json | tr -d '\r' | grep -iE 'HTTP/|allow'
```

**Attendu :** webserv returns '405 Method Not Allowed' + 'Allow: GET, POST' (HEAD is treated as a distinct method and is not in the location's allowed_methods). nginx serves HEAD wherever GET is allowed → '200 OK'. Flag as differential (RequestHandler.cpp:302 + DefaultConfig HEAD omission)

#### `HSF-06` · P0 · integration — 404 Not Found for missing resource serves the configured custom error page
**Réf :** R-404 / REQ-14 / REQ-16 · **Outil :** curl · **Auto :** oui

**Pré :** config T with 'error_page 404 /404.html'; www/404.html is 178 bytes

**Procédure :**

```sh
curl -s -D - -o /tmp/e http://localhost:8080/definitely-missing-xyz | tr -d '\r' | grep -iE 'HTTP/|content-length|content-type'; cmp /tmp/e /Users/blucken/Documents/webserv/Webserv/www/404.html && echo BODY_IS_CUSTOM_404
```

**Attendu :** 'HTTP/1.1 404 Not Found'; 'Content-Type: text/html'; 'Content-Length: 178'; response body is byte-identical to www/404.html (cmp exit 0)

#### `HSF-07` · P0 · robustness — 403 Forbidden on a file with no read permission
**Réf :** R-403 / REQ-14 · **Outil :** curl · **Auto :** oui

**Pré :** config T; create www/perm000.txt then chmod 000

**Procédure :**

```sh
printf secret > /Users/blucken/Documents/webserv/Webserv/www/perm000.txt; chmod 000 /Users/blucken/Documents/webserv/Webserv/www/perm000.txt; curl -s -o /dev/null -w '%{http_code}\n' http://localhost:8080/perm000.txt; chmod 644 /Users/blucken/Documents/webserv/Webserv/www/perm000.txt; rm -f /Users/blucken/Documents/webserv/Webserv/www/perm000.txt
```

**Attendu :** 403 (open() EACCES mapped to 403). Matches nginx. Server must not crash and must keep serving other requests

#### `HSF-08` · P0 · integration — autoindex ON renders an HTML directory listing when no index file exists
**Réf :** REQ-29 · **Outil :** curl · **Auto :** oui

**Pré :** config T; location /static {autoindex on}; www/static/ contains style.css, hello.txt, app.js and no index.html

**Procédure :**

```sh
curl -s -D - http://localhost:8080/static/ | tr -d '\r' | grep -iE 'HTTP/|content-type|content-length'; echo '---body---'; curl -s http://localhost:8080/static/
```

**Attendu :** 'HTTP/1.1 200 OK'; 'Content-Type: text/html; charset=utf-8'; a non-empty <ul> listing with an '<a href="../">..</a>' parent link and one <a> per entry (style.css, hello.txt, app.js). Content-Length equals the emitted body length

#### `HSF-09` · P0 · differential-vs-nginx — autoindex OFF on a directory with no index file returns 403
**Réf :** REQ-29 / R-403 · **Outil :** curl · **Auto :** oui

**Pré :** Add to config T: location /uploads {autoindex off; allowed_methods GET HEAD;} (inherits root www, so /uploads/ maps to existing www/uploads/ which has no index.html)

**Procédure :**

```sh
curl -s -o /dev/null -w '%{http_code}\n' http://localhost:8080/uploads/
```

**Attendu :** 403 (directory, no index, autoindex disabled). Matches nginx (nginx also returns 403 for autoindex-off directory). Must NOT emit a listing

#### `HSF-10` · P0 · integration — Directory index file served instead of a listing
**Réf :** REQ-30 / R-200 · **Outil :** curl · **Auto :** oui

**Pré :** config T; index directive 'index.html index.htm default.html'; www/app/index.html exists (242 bytes)

**Procédure :**

```sh
curl -s -D - -o /dev/null http://localhost:8080/app/ | tr -d '\r' | grep -iE 'HTTP/|content-type|content-length'
```

**Attendu :** 'HTTP/1.1 200 OK'; 'Content-Type: text/html'; 'Content-Length: 242' (the index file's bytes), NOT an autoindex listing and NOT 403

#### `HSF-11` · P0 · differential-vs-nginx — Directory requested without a trailing slash yields 301 to the slash-terminated path
**Réf :** R-301 / R-LOCATION-3XX / REQ-14 · **Outil :** curl · **Auto :** oui

**Pré :** config T; www/static/ is a directory

**Procédure :**

```sh
curl -s -D - -o /dev/null http://localhost:8080/static | tr -d '\r' | grep -iE 'HTTP/|location'
```

**Attendu :** 'HTTP/1.1 301 Moved Permanently' and 'Location: /static/'. Matches nginx trailing-slash redirect behavior

#### `HSF-12` · P0 · integration — Configured 'return' redirects: 301 (absolute URI), 302 (relative), 410 (no Location)
**Réf :** REQ-27 / R-301 / R-302 / R-LOCATION-3XX · **Outil :** curl · **Auto :** oui

**Pré :** config T: '= /old-page'→return 301 http://example.com/new-page; '/legacy'→return 302 /modern; '/gone'→return 410

**Procédure :**

```sh
for p in old-page legacy gone; do echo "== /$p =="; curl -s -D - -o /dev/null http://localhost:8080/$p | tr -d '\r' | grep -iE 'HTTP/|location'; done
```

**Attendu :** /old-page → '301 Moved Permanently' + 'Location: http://example.com/new-page'; /legacy → '302 Found' + 'Location: /modern'; /gone → '410 Gone' with NO Location header (and a non-empty error body). curl -L on /old-page follows to the target

#### `HSF-13` · P0 · integration — Method not permitted on a route → 405 with an accurate Allow header
**Réf :** REQ-26 / R-405 / R-ALLOW · **Outil :** curl · **Auto :** oui

**Pré :** config T; location /static allows only {GET, HEAD}; www/static/hello.txt exists

**Procédure :**

```sh
curl -s -D - -o /dev/null -X POST http://localhost:8080/static/hello.txt | tr -d '\r' | grep -iE 'HTTP/|allow'; curl -s -D - -o /dev/null -X DELETE http://localhost:8080/static/hello.txt | tr -d '\r' | grep -iE 'HTTP/|allow'
```

**Attendu :** Both POST and DELETE → 'HTTP/1.1 405 Method Not Allowed' and 'Allow: GET, HEAD' (enumerating exactly the configured methods). The allowed method (GET) on the same URL succeeds with 200

#### `HSF-14` · P0 · robustness — Unrecognized / unimplemented request method → 501 (not 405, not served)
**Réf :** R-501 / R03-method-too-long-501 · **Outil :** nc · **Auto :** oui

**Pré :** config T running

**Procédure :**

```sh
for m in BREW OPTIONS get; do printf '%s / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' "$m" | nc localhost 8080 | head -1; done
```

**Attendu :** Each → 'HTTP/1.1 501 Not Implemented' (method tokens are exact-uppercase; BREW, OPTIONS and lowercase 'get' are all unimplemented). Server does not crash. NB: this diverges from nginx which returns 405 for OPTIONS on a static resource — acceptable per RFC 9112 §3.4 SHOULD 501, but record the difference

#### `HSF-15` · P0 · stress — Large file (> BUFFER_SIZE) streamed byte-exact with correct Content-Length, no truncation/hang
**Réf :** REQ-18 / R-FRAMING / R-METHOD-GET · **Outil :** curl · **Auto :** oui

**Pré :** config T; www/hugefile.txt is 2796205 bytes (far exceeds BUFFER_SIZE=8192)

**Procédure :**

```sh
curl -s --max-time 20 -D /tmp/h.hdr -o /tmp/h.body http://localhost:8080/hugefile.txt; grep -i content-length /tmp/h.hdr; stat -f%z /tmp/h.body; md5 -q /tmp/h.body; md5 -q /Users/blucken/Documents/webserv/Webserv/www/hugefile.txt
```

**Attendu :** 200; 'Content-Length: 2796205'; downloaded size == 2796205; md5 of body == md5 of disk file (multi-chunk read loop delivers the whole body, no truncation, completes well within --max-time)

#### `HSF-16` · P1 · differential-vs-nginx — DIFFERENTIAL: Date response header must end in 'GMT' (webserv emits 'UTC')
**Réf :** R-DATE / code-gap ResponseHandler:385 · **Outil :** curl · **Auto :** oui

**Pré :** config T running

**Procédure :**

```sh
curl -s -D - -o /dev/null http://localhost:8080/test.txt | tr -d '\r' | grep -i '^date:'
```

**Attendu :** RFC 9110 IMF-fixdate requires the value end in ' GMT' (e.g. 'Sun, 06 Nov 1994 08:49:37 GMT'), which nginx emits. webserv currently emits '... UTC' (strftime %Z over gmtime) — a conformance defect (ResponseHandler.cpp:385). Assert the value matches /GMT$/

#### `HSF-17` · P0 · differential-vs-nginx — DIFFERENTIAL: HTTP/1.0 request handling (webserv 505 vs nginx 200)
**Réf :** R-VERSION-MINOR / R27-unsupported-version / code-gap Parser:166 · **Outil :** nc · **Auto :** oui

**Pré :** config T; www/test.txt exists

**Procédure :**

```sh
printf 'GET /test.txt HTTP/1.0\r\nHost: x\r\n\r\n' | nc localhost 8080 | head -1
```

**Attendu :** webserv returns 'HTTP/1.1 505 HTTP Version Not Supported' (version token hardcoded to exactly 'HTTP/1.1', Parser.cpp:166). nginx and RFC 9110 §15.6.6 require processing a supported MAJOR version regardless of minor → '200 OK'. Flag as a conformance/differential defect; 42 testers and telnet clients frequently send HTTP/1.0

#### `HSF-18` · P1 · integration — Percent-decoded request path resolves files with spaces / encoded octets
**Réf :** R-ORIGIN-FORM / REQ-28 · **Outil :** curl · **Auto :** oui

**Pré :** config T; www/'my file.txt' exists (13 bytes: 'hello spaces')

**Procédure :**

```sh
curl -s -D - -o /tmp/s 'http://localhost:8080/my%20file.txt' | tr -d '\r' | grep -iE 'HTTP/|content-length'; cat /tmp/s
```

**Attendu :** 200; 'Content-Length: 13'; body 'hello spaces' (server percent-decodes %20 to a space before filesystem lookup)

#### `HSF-19` · P1 · differential-vs-nginx — ADVERSARIAL: autoindex output is unescaped (XSS) and leaks the absolute filesystem path
**Réf :** REQ-29 / code-gap ExecutionHandler:945 · **Outil :** curl · **Auto :** oui

**Pré :** config T; create a listed dir with a metachar filename: mkdir www/xss && : > 'www/xss/a<script>b.txt' and ensure a location maps /xss/ to it with autoindex on (or reuse /static with the crafted file)

**Procédure :**

```sh
mkdir -p /Users/blucken/Documents/webserv/Webserv/www/static/xss; : > '/Users/blucken/Documents/webserv/Webserv/www/static/xss/a<script>b.txt'; curl -s 'http://localhost:8080/static/xss/'; rm -rf /Users/blucken/Documents/webserv/Webserv/www/static/xss
```

**Attendu :** webserv listing contains the RAW unescaped string '<a href="a<script>b.txt">a<script>b.txt</a>' (reflected-XSS) AND the <title>/<h1> print the ABSOLUTE server path '/Users/.../www/static/xss/' (path disclosure). nginx HTML-escapes entry names (<script>) and shows only the URI path '/static/xss/'. Flag both as differential defects (ExecutionHandler.cpp:945/968)

#### `HSF-20` · P2 · integration — Query string ignored for static file; empty file served as 200 with Content-Length 0
**Réf :** R-ORIGIN-FORM / R-200 · **Outil :** curl · **Auto :** oui

**Pré :** config T; www/test.txt (6 bytes) and www/empty.txt (0 bytes) exist

**Procédure :**

```sh
curl -s -D - -o /dev/null 'http://localhost:8080/test.txt?foo=bar&x=1' | tr -d '\r' | grep -iE 'HTTP/|content-length'; echo '---empty---'; curl -s -D - -o /dev/null http://localhost:8080/empty.txt | tr -d '\r' | grep -iE 'HTTP/|content-length'
```

**Attendu :** /test.txt?foo=bar → 200 with 'Content-Length: 6' (query component stripped from the filesystem path). /empty.txt → 200 with 'Content-Length: 0' and no body bytes

---

### 6.3 POST / PUT / DELETE & file upload (WebDAV)

#### `DAV-DIFF-01` · P0 · differential-vs-nginx — Automated status diff of the 23-cell DAV matrix vs recorded nginx oracle
**Réf :** REQ-38, REQ-22, REQ-26, test_nginx_dav.md · **Outil :** bash+curl · **Auto :** oui

**Pré :** Create test/nginx_tests/configs/webserv-dav.conf mirroring nginx dav-on_ai-on: root==dav_put_path==<ABS>/test/nginx_tests/www (REQUIRED for PUT/GET round-trip, buildAbsolutPath:392); client_max_body_size 1M; /small at 100 bytes; create_full_put_path on; dav_access user:rw group:rw all:r; dav_methods PUT DELETE; locations / /test/ /small /nodav(dav_methods off) /upload(enable_cgi on,.py); every location lists allowed_methods GET HEAD POST PUT DELETE (HEAD/PUT absent from defaults). Also build DAV-off/autoindex/cfpp variants mirroring the 6 nginx configs. nginx oracle transcripts already in test/nginx_tests/output/*.txt.

**Procédure :**

```sh
Extend run-tests.sh into a comparator. Per cell: code=$(curl -s -o /dev/null -w '%{http_code}' -X $METHOD localhost:8080$URL ${BODY:+-d test}); nginx=$(grep -m1 '< HTTP/1.1' output/$NAME.txt|awk '{print $3}'); [ $code = $nginx ]&&echo PASS||echo FAIL. Iterate all 8 columns x the run-tests.sh URL/method set with the same setup/cleanup hooks.
```

**Attendu :** Every webserv cell == nginx oracle EXCEPT PUT-NEST-01/02 and POST-upload. Must match: POST / 403; POST /test/ 403(idx-)/405(idx+); POST /test 301; POST file absent 404/present 405; PUT dir 405(DAV-)/409(DAV+); PUT new 405(DAV-)/201(DAV+); PUT existing 204(DAV+); DELETE file 204/absent 404(DAV+),405(DAV-); DELETE /test/ empty|nonempty 204(DAV+).

#### `PUT-01` · P0 · integration — PUT creates new file -> 201, Content-Length: 0, bytes verbatim
**Réf :** REQ-19, REQ-31, R-METHOD-PUT, R-201 · **Outil :** curl · **Auto :** oui

**Pré :** rm -f www/put_new.txt; DAV-on; dav_put_path==root.

**Procédure :**

```sh
printf 'PUTPAYLOAD-0123456789' >/tmp/pb; curl -sv -X PUT --data-binary @/tmp/pb http://localhost:8080/put_new.txt; xxd www/put_new.txt
```

**Attendu :** 201 Created with 'Content-Length: 0' and empty body (ResponseHandler:152). www/put_new.txt byte-identical to /tmp/pb (21 bytes); subsequent GET returns the same 21 bytes.

#### `PUT-02` · P1 · integration — PUT over existing file -> 204, content replaced, no body
**Réf :** REQ-19, R-METHOD-PUT, R-204 · **Outil :** curl · **Auto :** oui

**Pré :** printf 'OLD-CONTENT' > www/put_ovr.txt

**Procédure :**

```sh
curl -sv -X PUT --data-binary 'NEW' http://localhost:8080/put_ovr.txt -o /dev/null -D -; cat www/put_ovr.txt
```

**Attendu :** 204 No Content, zero body bytes (E_EXEC_NOCONTENT, openFile:601). File now exactly 'NEW' (O_TRUNC).

#### `PUT-03` · P1 · integration — PUT idempotency: twice -> 201 then 204, final state identical
**Réf :** R-IDEMPOTENT-PUT-DELETE, R-METHOD-PUT · **Outil :** curl · **Auto :** oui

**Pré :** rm -f www/idem.txt

**Procédure :**

```sh
curl -s -o /dev/null -w 'a=%{http_code}\n' -X PUT --data-binary ABC http://localhost:8080/idem.txt; curl -s -o /dev/null -w 'b=%{http_code}\n' -X PUT --data-binary ABC http://localhost:8080/idem.txt; cat www/idem.txt
```

**Attendu :** a=201, b=204; file exactly 'ABC' after both (idempotent).

#### `PUT-04` · P1 · differential-vs-nginx — PUT to directory / trailing-slash -> 409
**Réf :** test_nginx_dav.md, R-METHOD-PUT · **Outil :** curl · **Auto :** oui

**Pré :** www/test/ exists as directory; DAV-on.

**Procédure :**

```sh
curl -s -o /dev/null -w '%{http_code}\n' -X PUT --data-binary x http://localhost:8080/test/; curl -s -o /dev/null -w '%{http_code}\n' -X PUT --data-binary x http://localhost:8080/test
```

**Attendu :** Both 409 (executePUT:463 trailing slash, :468 existing dir). Matches nginx PUT-dirs DAV+.

#### `PUT-05` · P1 · differential-vs-nginx — PUT with dav_methods off -> 405 + Allow
**Réf :** REQ-26, R-405, R-ALLOW, test_nginx_dav.md · **Outil :** curl · **Auto :** oui

**Pré :** Target /nodav; rm -f www/nodav_x.txt.

**Procédure :**

```sh
curl -sv -X PUT --data-binary x http://localhost:8080/nodav/nodav_x.txt -o /dev/null -D -
```

**Attendu :** 405 (executePUT:458) with Allow header (buildErrorResponse:197). Matches nginx DAV- column.

#### `PUT-NEST-01` · P1 · differential-vs-nginx — GAP: nested PUT, cfpp ON, missing parent -> webserv 500 (nginx 201)
**Réf :** 42 subject create_full_put_path, R-METHOD-PUT, gap cfpp-never-applied · **Outil :** curl · **Auto :** oui

**Pré :** rm -rf www/test/a; create_full_put_path on; DAV-on.

**Procédure :**

```sh
curl -sv -X PUT --data-binary deep http://localhost:8080/test/a/b/c.txt -o /dev/null -D -; ls -la www/test/a/b/c.txt 2>&1
```

**Attendu :** DIVERGENCE: webserv 500, no file created, vs nginx 201 full-path created. cfpp parsed but never applied; open() ENOENT on missing parent, openFile:595 doesn't throw on ENOENT, writeChunk !_fd.valid() (:662) throws 500. Must NOT crash.

#### `PUT-ROUNDTRIP-DIV` · P1 · integration — GAP: dav_put_path != root -> PUT then GET returns 404
**Réf :** REQ-31, REQ-19, buildAbsolutPath:392-395 · **Outil :** curl · **Auto :** oui

**Pré :** Config with DAV location dav_put_path != root (blucken.conf /webdav: root=www, dav_put_path=www/webdav/uploads).

**Procédure :**

```sh
curl -s -o /dev/null -w 'put=%{http_code}\n' -X PUT --data-binary stored http://localhost:8080/webdav/f.txt; curl -s -o /dev/null -w 'get=%{http_code}\n' http://localhost:8080/webdav/f.txt; find www -name f.txt
```

**Attendu :** put=201 but get=404. PUT uses dav_put_path (buildAbsolutPath:393), GET uses root (:395). Confirms REQ-31 divergence unless dav_put_path==root.

#### `DEL-01` · P0 · integration — DELETE existing file -> 204, removed, GET -> 404
**Réf :** REQ-22, R-METHOD-DELETE, R-204 · **Outil :** curl · **Auto :** oui

**Pré :** printf bye > www/del_me.txt; DAV-on.

**Procédure :**

```sh
curl -sv -X DELETE http://localhost:8080/del_me.txt -o /dev/null -D -; ls www/del_me.txt 2>&1; curl -s -o /dev/null -w 'get=%{http_code}\n' http://localhost:8080/del_me.txt
```

**Attendu :** 204 empty body; www/del_me.txt unlinked (executeDELETE:517); get=404. Matches nginx.

#### `DEL-02` · P1 · integration — DELETE absent file (DAV on) -> 404; idempotency
**Réf :** REQ-22, R-IDEMPOTENT-PUT-DELETE, R-404 · **Outil :** curl · **Auto :** oui

**Pré :** printf x > www/del2.txt; DAV-on.

**Procédure :**

```sh
curl -s -o /dev/null -w 'd1=%{http_code}\n' -X DELETE http://localhost:8080/del2.txt; curl -s -o /dev/null -w 'd2=%{http_code}\n' -X DELETE http://localhost:8080/del2.txt
```

**Attendu :** d1=204, d2=404 (idempotent effect). 404-on-absent matches nginx.

#### `DEL-03` · P1 · differential-vs-nginx — DELETE non-empty directory via trailing slash -> 204 (recursive)
**Réf :** REQ-22, R-METHOD-DELETE, test_nginx_dav.md · **Outil :** curl · **Auto :** oui

**Pré :** mkdir -p www/deldir; printf a>www/deldir/one; printf b>www/deldir/two; DAV-on.

**Procédure :**

```sh
curl -s -o /dev/null -w '%{http_code}\n' -X DELETE http://localhost:8080/deldir/; ls -la www/deldir 2>&1
```

**Attendu :** 204; www/deldir recursively removed (executeDELETE:509). Matches nginx 'DELETE /test/ non vide 204'.

#### `DEL-04` · P1 · differential-vs-nginx — DELETE non-slash directory -> 409; DELETE dav off -> 405
**Réf :** REQ-26, R-405, R-ALLOW, test_nginx_dav.md · **Outil :** curl · **Auto :** oui

**Pré :** mkdir -p www/dir409; have /nodav.

**Procédure :**

```sh
curl -s -o /dev/null -w 'ns=%{http_code}\n' -X DELETE http://localhost:8080/dir409; curl -s -o /dev/null -w 'off=%{http_code}\n' -X DELETE http://localhost:8080/nodav/x.txt
```

**Attendu :** ns=409 (executeDELETE:511), off=405 (:491) with Allow. 405 matches nginx DAV-.

#### `POST-STATIC-01` · P1 · differential-vs-nginx — POST to static location always fails (403/404/405/301), never success
**Réf :** REQ-21, test_nginx_dav.md, R-405/R-404/R-403 · **Outil :** curl · **Auto :** oui

**Pré :** no root index; create www/exists.txt; www/test/index.html present for /test/ sub-case.

**Procédure :**

```sh
for t in / /test /test/ /exists.txt /missing.txt; do printf '%-12s ' $t; curl -s -o /dev/null -w '%{http_code}\n' -X POST -d test http://localhost:8080$t; done
```

**Attendu :** / 403 (:413); /test 301 Location /test/ (:418); /test/ 405 with index / 403 without (:429/:433); /exists.txt 405 (:438); /missing.txt 404 (:443). executePOST has no success branch. Matches nginx POST matrix.

#### `POST-UPLOAD-GAP` · P1 · integration — GAP: POST file upload rejected (no POST upload path)
**Réf :** REQ-19, REQ-31, R-METHOD-POST, executePOST no-success-path · **Outil :** curl · **Auto :** oui

**Pré :** /upload with no matching CGI ext; rm -f www/upload/photo.bin.

**Procédure :**

```sh
head -c 2048 /dev/urandom >/tmp/p.bin; curl -sv -X POST --data-binary @/tmp/p.bin -H 'Content-Type: application/octet-stream' http://localhost:8080/upload/photo.bin -o /dev/null -D -
```

**Attendu :** 404/405 (executePOST), NOT 2xx, no file stored. Flag REQ-19/REQ-31 (POST upload must give 2xx+store). Uploads must use PUT here. Must not crash.

#### `CT-415-GAP` · P1 · differential-vs-nginx — GAP: parameterized Content-Type on PUT -> spurious 415
**Réf :** REQ-19, R-CONTENT-TYPE, gap content-type-exact-equality · **Outil :** curl · **Auto :** oui

**Pré :** DAV-on; types has 'text/plain txt'; rm -f www/param_ct.txt.

**Procédure :**

```sh
curl -sv -X PUT -H 'Content-Type: text/plain; charset=utf-8' --data-binary hi http://localhost:8080/param_ct.txt -o /dev/null -D -
```

**Attendu :** DIVERGENCE: webserv 415 (exact-compares whole 'text/plain; charset=utf-8' vs bare 'text/plain', findHeader:278/RequestHandler:360). nginx accepts ->201/204. Gap #5.

#### `CT-MULTIPART-GAP` · P1 · integration — GAP: multipart/form-data upload -> 415, CGI multipart branch dead
**Réf :** REQ-19, REQ-13, R-METHOD-POST, gap multipart-never-detected · **Outil :** curl · **Auto :** oui

**Pré :** /upload enable_cgi on, .py mapping + www/upload/recv.py.

**Procédure :**

```sh
printf bytes >/tmp/f.dat; curl -sv -X POST -F 'field=@/tmp/f.dat' http://localhost:8080/upload/recv.py -o /dev/null -D -
```

**Attendu :** 415. CT is 'multipart/form-data; boundary=...'; findHeader('Content-Type','multipart/form-data') never matches boundary-bearing value (RequestHandler:333), else 415s (:369). Every real browser upload rejected. Gap #5; impacts REQ-13/REQ-19. Must not crash.

#### `CMBS-413-01` · P0 · integration — PUT body larger than client_max_body_size -> 413, no file created
**Réf :** REQ-25, R-413 · **Outil :** curl · **Auto :** oui

**Pré :** /small (100 bytes), DAV+; rm -f www/small/big.bin.

**Procédure :**

```sh
python3 -c 'import sys;sys.stdout.buffer.write(b"A"*200)' >/tmp/big; curl -sv -X PUT --data-binary @/tmp/big http://localhost:8080/small/big.bin -o /dev/null -D -; find www -name big.bin
```

**Attendu :** 413 with Retry-After. parseBody sees Content-Length 200>100, throws 413 (RequestHandler:173) BEFORE openFile (Client.cpp:385 precedes :396); no file (find empty). A 100-byte body is accepted.

#### `CMBS-CHUNKED-413` · P1 · integration — Chunked PUT whose decoded total exceeds limit -> 413
**Réf :** REQ-25, REQ-39, R-413, RFC9112 7.1 · **Outil :** nc · **Auto :** oui

**Pré :** /small (100 bytes), DAV+.

**Procédure :**

```sh
printf 'PUT /small/c.bin HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n64\r\n%s\r\n64\r\n%s\r\n0\r\n\r\n' "$(python3 -c 'print("A"*100)')" "$(python3 -c 'print("B"*100)')"|nc -w2 localhost 8080|head -1
```

**Attendu :** 413. Two 0x64(=100)-byte chunks=200 decoded; decodeChunkedBody cumulative check (RequestHandler:485) throws 413 when total>100. Server alive, closes conn.

#### `CMBS-CL-OVERFLOW` · P1 · robustness — GAP/adversarial: Content-Length > 2^32 wraps 32-bit size, bypasses 413
**Réf :** REQ-11, REQ-25, R-413, gap content-length-istringstream · **Outil :** nc · **Auto :** oui

**Pré :** /small (100 bytes), DAV+.

**Procédure :**

```sh
printf 'PUT /small/ovf.bin HTTP/1.1\r\nHost: x\r\nContent-Type: application/octet-stream\r\nContent-Length: 4294967297\r\n\r\nAB'|nc -w3 localhost 8080|head -1; curl -s -o /dev/null -w 'alive=%{http_code}\n' --max-time 3 http://localhost:8080/
```

**Attendu :** Declared 4294967297 (2^32+1) read via iss>>unsigned int (RequestHandler:168) WRAPS to 1, 413 guard (:173) bypassed. Observe NO immediate 413, then writer waits ~4GiB -> conn times out/closes. CRITICAL: no crash, keeps serving (alive=200/403/404). Gap #7.

#### `CHUNKED-PUT-STORE` · P1 · integration — Chunked PUT de-chunked and stored byte-identical -> 201
**Réf :** REQ-39, R-METHOD-PUT, RFC9112 7.1 · **Outil :** nc · **Auto :** oui

**Pré :** rm -f www/chunked_put.txt; DAV+; limit>=11.

**Procédure :**

```sh
printf 'PUT /chunked_put.txt HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n6\r\n world\r\n0\r\n\r\n'|nc -w2 localhost 8080|head -1; cat www/chunked_put.txt; echo
```

**Attendu :** 201 Created. www/chunked_put.txt = exactly 'hello world' (11 bytes) no chunk framing (decodeChunkedBody reassembles, synthesizes Content-Length:11 at :523, writeChunkChunked streams). Verifies REQ-39.

#### `FRAME-CL-TE-411` · P1 · integration — PUT framing: both CL+TE -> 400; neither -> 411; empty -> 201
**Réf :** R-411, RFC9112 6.3, R-METHOD-PUT · **Outil :** nc · **Auto :** oui

**Pré :** rm -f www/frame.txt www/empty.txt; DAV+.

**Procédure :**

```sh
printf 'PUT /frame.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n'|nc -w2 localhost 8080|head -1; printf 'PUT /frame.txt HTTP/1.1\r\nHost: x\r\n\r\n'|nc -w2 localhost 8080|head -1; printf 'PUT /empty.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 0\r\n\r\n'|nc -w2 localhost 8080|head -1; ls -l www/empty.txt
```

**Attendu :** both CL+TE -> 400 (validateHeaders:325); neither -> 411 (:319); empty -> 201 with 0-byte www/empty.txt. No desync across the three.

#### `DAV-ROBUST-LOOP` · P0 · stress — Sustained PUT/DELETE churn + concurrent GET: no crash, no fd leak
**Réf :** REQ-11, REQ-12, REQ-04 · **Outil :** bash+curl+lsof · **Auto :** oui

**Pré :** webserv running; PID known; base=$(lsof -p PID|wc -l).

**Procédure :**

```sh
PID=<pid>; base=$(lsof -p $PID|wc -l); for i in $(seq 1 500); do curl -s -o /dev/null -X PUT --data-binary p$i http://localhost:8080/loop_$((i%20)).txt & curl -s -o /dev/null -X DELETE http://localhost:8080/loop_$(((i+7)%20)).txt & curl -s -o /dev/null http://localhost:8080/ & done; wait; sleep 2; curl -s -o /dev/null -w 'alive=%{http_code}\n' --max-time 3 http://localhost:8080/; echo delta=$(expr $(lsof -p $PID|wc -l) - $base); kill -0 $PID&&echo PID-ALIVE
```

**Attendu :** PID-ALIVE, fresh GET 200/403 within max-time, fd delta ~0 (no leak), no 5xx storm, no hang.

#### `DAV-DELETE-ROOT` · P2 · differential-vs-nginx — Destructive: DELETE / on DAV-on root -> 204 (recursively deletes root), run last
**Réf :** REQ-22, test_nginx_dav.md DELETE / row · **Outil :** curl · **Auto :** oui

**Pré :** RUN LAST on THROWAWAY www; cp -a www www.bak; DAV-on; dav_put_path==root.

**Procédure :**

```sh
curl -s -o /dev/null -w '%{http_code}\n' -X DELETE http://localhost:8080/; ls -la www 2>&1; rm -rf www && mv www.bak www
```

**Attendu :** 204: executeDELETE treats '/' as existing trailing-slash dir, recursively removes the whole dav_put_path/root tree (deleteDirectory), like nginx ('DELETE / 204'). Files under www then 404 until restored.

---

### 6.4 CGI execution & robustness

_Grounded in the real runtime config configs/blucken.conf (servers on localhost:8080-8084; location /cgi-bin, root .../Webserv/www, enable_cgi on, cgi_extensions .py=/usr/bin/python3 .sh=/bin/bash) and the fixtures in Webserv/www/cgi-bin/ (hello.py, env.sh, echo.sh, malformed.py, sleep.py). Confirmed from source: CGI_TIMEOUT_S=30, BUFFER_SIZE=8192; driveIO() reads ONE 8 KB chunk then tryReap() forces _eof (truncation gap); buildEnv() hardcodes SERVER_PORT=8080, SERVER_NAME=webserv, PATH_INFO="" (meta-var gaps); tryReap() maps WIFSIGNALED and nonzero-exit-with-empty-buffer to 502; parse() 502s on missing header separator or missing Content-Type/Location/Status; POST validateHeaders() 415s any Content-Type not exactly equal to a configured MIME value (form-urlencoded gap). All tests are black-box HTTP unless a leak/zombie probe requires the server PID. Start the server as ./webserv configs/blucken.conf before running. Tests marked GAP target suspected code-analysis defects; several are differential vs nginx (run the same request against a real nginx CGI route to get the oracle). New fixtures (big.py, crash.py, exit1.py, status404.py, redir.py, statusonly.py, fdcount.sh, hdrcase.py) must be created under Webserv/www/cgi-bin/ and chmod +x._

#### `CGI-01` · P0 · integration — GET .py happy path returns dynamic output, not source
**Réf :** REQ-32 / REQ-37 / CGI-RESP-HEADER-BLOCK · **Outil :** curl · **Auto :** oui

**Pré :** Server running ./webserv configs/blucken.conf; www/cgi-bin/hello.py present and +x; /usr/bin/python3 exists.

**Procédure :**

```sh
curl -sS -D- -o /tmp/b http://localhost:8080/cgi-bin/hello.py; cat /tmp/b
```

**Attendu :** HTTP/1.1 200 OK; Content-Type: text/html; body is exactly '<html><body><h1>Hello from CGI!</h1></body></html>\n' (the script OUTPUT). Body must NOT contain the literal '#!/usr/bin/env python3' source line.

#### `CGI-02` · P0 · integration — GET query string + required meta-variables via env dump
**Réf :** REQ-33 / CGI-META-QUERY_STRING / CGI-META-REQUEST_METHOD / CGI-META-SCRIPT_NAME / CGI-META-GATEWAY_INTERFACE · **Outil :** curl · **Auto :** oui

**Pré :** www/cgi-bin/env.sh present +x (dumps `env|sort`); /bin/bash mapped for .sh.

**Procédure :**

```sh
curl -sS 'http://localhost:8080/cgi-bin/env.sh?a=1&b=hello%20world' | grep -E '^(REQUEST_METHOD|QUERY_STRING|SCRIPT_NAME|GATEWAY_INTERFACE|SERVER_PROTOCOL|REMOTE_ADDR|REQUEST_URI)='
```

**Attendu :** REQUEST_METHOD=GET; QUERY_STRING=a=1&b=hello%20world (percent-encoding preserved, NOT decoded to 'hello world'); SCRIPT_NAME=/cgi-bin/env.sh; GATEWAY_INTERFACE=CGI/1.1; SERVER_PROTOCOL=HTTP/1.1; REMOTE_ADDR=127.0.0.1; REQUEST_URI=/cgi-bin/env.sh?a=1&b=hello%20world.

#### `CGI-03` · P0 · integration — POST body delivered on CGI stdin with correct CONTENT_LENGTH
**Réf :** REQ-21 / CGI-BODY-STDIN / CGI-META-CONTENT_LENGTH / CGI-META-CONTENT_TYPE · **Outil :** curl · **Auto :** oui

**Pré :** www/cgi-bin/echo.sh present +x (emits Content-Type then `cat` of stdin). Use Content-Type: text/plain (in blucken.conf types) so POST passes the MIME gate.

**Procédure :**

```sh
curl -sS -D- -H 'Content-Type: text/plain' --data-binary 'name=webserv&n=42' http://localhost:8080/cgi-bin/echo.sh; echo '---'; curl -sS -H 'Content-Type: text/plain' --data-binary 'name=webserv&n=42' http://localhost:8080/cgi-bin/env.sh | grep -E '^(CONTENT_LENGTH|CONTENT_TYPE|REQUEST_METHOD)='
```

**Attendu :** echo.sh: 200 with body exactly 'name=webserv&n=42' (18 bytes, verbatim stdin). env.sh: CONTENT_LENGTH=18, CONTENT_TYPE=text/plain, REQUEST_METHOD=POST.

#### `CGI-04` · P0 · integration — Chunked request body un-chunked before CGI (stdin sees decoded bytes, EOF at true end)
**Réf :** REQ-34 / CGI-BODY-UNCHUNK / RFC9112 R16 · **Outil :** nc · **Auto :** oui

**Pré :** echo.sh and env.sh present +x. No Content-Type header (defaults to application/octet-stream, avoids 415).

**Procédure :**

```sh
printf 'POST /cgi-bin/echo.sh HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n5\r\nhel\r\n... ' # send in two chunks:  printf 'POST /cgi-bin/echo.sh HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n3\r\nhel\r\n2\r\nlo\r\n0\r\n\r\n' | nc localhost 8080  ;  # then env: printf 'POST /cgi-bin/env.sh HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\nConnection: close\r\n\r\n3\r\nhel\r\n2\r\nlo\r\n0\r\n\r\n' | nc localhost 8080 | grep -E '^CONTENT_LENGTH='
```

**Attendu :** echo.sh response body is exactly 'hello' (5 bytes), with NO chunk-size lines ('3','2','0') and no CRLF framing leaking into the body. env.sh shows CONTENT_LENGTH=5 and no Transfer-Encoding-derived value. (macOS nc: append `-q1` or wrap `( printf ...; sleep 1 )`.)

#### `CGI-05` · P0 · robustness — Infinite/long-running CGI times out to 5xx and server stays responsive (no hang)
**Réf :** CGI-ERR-TIMEOUT-NOHANG / REQ-10 / REQ-04 · **Outil :** curl · **Auto :** oui

**Pré :** www/cgi-bin/sleep.py present +x (sleeps 60s > CGI_TIMEOUT_S=30). Note server socket is registered E_IN|E_OUT so the loop keeps driving checkTimeout.

**Procédure :**

```sh
( time curl -sS -o /dev/null -w '%{http_code}\n' --max-time 40 http://localhost:8080/cgi-bin/sleep.py ) & sleep 2; curl -sS -o /dev/null -w 'concurrent=%{http_code}\n' --max-time 5 http://localhost:8080/index_test.html; wait
```

**Attendu :** The CGI request returns a 5xx status (504 expected per checkTimeout, 502/500 acceptable) in ~30-31s, NOT curl's own 40s max-time abort. The concurrent static request returns 200 within 5s WHILE the CGI is still running — proving the single event loop is not blocked. Server PID still alive afterward.

#### `CGI-06` · P0 · robustness — CGI killed by signal (segfault) -> 502, no server crash
**Réf :** CGI-ERR-CRASH / REQ-11 / R-502 · **Outil :** curl · **Auto :** oui

**Pré :** Create www/cgi-bin/crash.py: `#!/usr/bin/env python3\nimport os,signal\nos.kill(os.getpid(), signal.SIGSEGV)` ; chmod +x.

**Procédure :**

```sh
before=$(pgrep -f 'webserv configs'); curl -sS -o /dev/null -w '%{http_code}\n' http://localhost:8080/cgi-bin/crash.py; after=$(pgrep -f 'webserv configs'); [ "$before" = "$after" ] && echo SAME_PID; curl -sS -o /dev/null -w 'still_up=%{http_code}\n' http://localhost:8080/cgi-bin/hello.py
```

**Attendu :** 502 Bad Gateway (WIFSIGNALED path). Valid HTTP response framing. Server PID unchanged (SAME_PID) and a follow-up CGI request returns 200 (still_up=200). No child left as <defunct>.

#### `CGI-07` · P0 · robustness — CGI exits non-zero with no output -> 502
**Réf :** CGI-ERR-CRASH / R-502 · **Outil :** curl · **Auto :** oui

**Pré :** Create www/cgi-bin/exit1.py: `#!/usr/bin/env python3\nimport sys\nsys.exit(1)` ; chmod +x.

**Procédure :**

```sh
curl -sS -o /dev/null -w '%{http_code}\n' http://localhost:8080/cgi-bin/exit1.py
```

**Attendu :** 502 Bad Gateway (WIFEXITED, status!=0, empty buffer). Not 200, not empty/hung response, no crash.

#### `CGI-08` · P0 · robustness — Malformed CGI output (no header/body separator) -> 502, not raw passthrough
**Réf :** CGI-ERR-CRASH / CGI-RESP-HEADER-BLOCK / R-502 · **Outil :** curl · **Auto :** oui

**Pré :** www/cgi-bin/malformed.py present +x (prints 'not a header' with no blank line).

**Procédure :**

```sh
curl -sS -D- -o /tmp/mb http://localhost:8080/cgi-bin/malformed.py; head -1 /tmp/mb; wc -c /tmp/mb
```

**Attendu :** 502 Bad Gateway (parse() finds no CRLFCRLF/LFLF -> invalid gateway output). The literal string 'not a header' MUST NOT be forwarded as the client body. Server remains up.

#### `CGI-09` · P0 · differential-vs-nginx — GAP: large CGI output (>8 KB) from a fast-exiting script is truncated
**Réf :** REQ-35 / CGI-RESP-HEADER-BLOCK (code-analysis truncation gap) · **Outil :** curl · **Auto :** oui

**Pré :** Create www/cgi-bin/big.py that emits a header then a fixed body that FITS the pipe buffer so the child can exit before the parent drains: `#!/usr/bin/env python3\nimport sys\nsys.stdout.write('Content-Type: text/plain\\r\\n\\r\\n')\nsys.stdout.write('A'*N)\nsys.stdout.flush()` with N=60000 on Linux (64 KB pipe) or N=12000 on macOS (16 KB pipe). chmod +x. Code: driveIO() reads one 8192 chunk then tryReap() forces _eof.

**Procédure :**

```sh
curl -sS -o /tmp/big -w 'code=%{http_code} bytes=%{size_download}\n' http://localhost:8080/cgi-bin/big.py; wc -c /tmp/big
```

**Attendu :** CORRECT (nginx oracle): 200 with exactly N body bytes and Content-Length=N. SUSPECTED BUG: body truncated to ~8164 bytes (first 8 KB read minus the header block) with a wrong/short Content-Length, or a 502. Any size_download != N is a defect.

#### `CGI-10` · P0 · stress — No zombie processes accumulate after repeated CGI requests
**Réf :** REQ-11 / REQ-17 / CGI reaping · **Outil :** ps · **Auto :** oui

**Pré :** Server running; hello.py present. Use ps to inspect defunct children of the server PID.

**Procédure :**

```sh
PID=$(pgrep -f 'webserv configs'); for i in $(seq 1 100); do curl -s -o /dev/null http://localhost:8080/cgi-bin/hello.py; done; ps -o pid,ppid,stat,command -ax | awk -v p=$PID '$2==p' | grep -c -i 'defunct\|Z+\| Z '
```

**Attendu :** 0 zombie/<defunct> children parented to the server after 100 CGI requests (waitpid reaps every child). Count must be 0.

#### `CGI-11` · P0 · stress — No file-descriptor leak across many CGI requests
**Réf :** REQ-11 / REQ-12 · **Outil :** lsof · **Auto :** oui

**Pré :** lsof available; server PID known.

**Procédure :**

```sh
PID=$(pgrep -f 'webserv configs'); b=$(lsof -p $PID 2>/dev/null | wc -l); for i in $(seq 1 200); do curl -s -o /dev/null http://localhost:8080/cgi-bin/hello.py; done; sleep 1; a=$(lsof -p $PID 2>/dev/null | wc -l); echo "before=$b after=$a delta=$((a-b))"
```

**Attendu :** FD count is stable (delta within a small constant, e.g. <=5), NOT growing by ~200 or ~400. Pipe read/write ends and reaped-child fds are all closed. A monotonically climbing count is a leak (fails REQ-11/12).

#### `CGI-12` · P1 · integration — Script not found -> 404; non-executable -> 403
**Réf :** CGI access gates (CGIHandler spawn) / R-404 / R-403 · **Outil :** curl · **Auto :** oui

**Pré :** hello.py exists +x. Make a non-exec copy: cp www/cgi-bin/hello.py www/cgi-bin/noexec.py && chmod -x www/cgi-bin/noexec.py.

**Procédure :**

```sh
curl -sS -o /dev/null -w 'missing=%{http_code}\n' http://localhost:8080/cgi-bin/does_not_exist.py; curl -sS -o /dev/null -w 'noexec=%{http_code}\n' http://localhost:8080/cgi-bin/noexec.py
```

**Attendu :** missing=404 (access X_OK -> ENOENT); noexec=403 (access X_OK -> EACCES). Both are valid framed responses, server stays up.

#### `CGI-13` · P1 · integration — CGI Status: header honored (404) and client redirect (302 + Location)
**Réf :** CGI-RESP-STATUS / CGI-RESP-CLIENT-REDIRECT · **Outil :** curl · **Auto :** oui

**Pré :** Create www/cgi-bin/status404.py: prints 'Status: 404 Not Found\r\nContent-Type: text/plain\r\n\r\nnope'. Create www/cgi-bin/redir.py: prints 'Status: 302 Found\r\nLocation: http://example.com/x\r\nContent-Type: text/html\r\n\r\n'. chmod +x both.

**Procédure :**

```sh
curl -sS -o /dev/null -w 'st=%{http_code}\n' http://localhost:8080/cgi-bin/status404.py; curl -sS -D- -o /dev/null http://localhost:8080/cgi-bin/redir.py
```

**Attendu :** status404.py -> client status 404 (Status line consumed, not forwarded as a body header). redir.py -> 302 Found with 'Location: http://example.com/x' propagated to the client.

#### `CGI-14` · P1 · robustness — CGI body without Content-Type but with Status -> valid response, no crash/hang
**Réf :** CGI-RESP-NO-CONTENT-TYPE / REQ-11 · **Outil :** curl · **Auto :** oui

**Pré :** Create www/cgi-bin/statusonly.py: prints 'Status: 200 OK\r\n\r\nhello-no-ct' (no Content-Type). chmod +x.

**Procédure :**

```sh
curl -sS -D- -o /tmp/nc --max-time 5 http://localhost:8080/cgi-bin/statusonly.py; cat /tmp/nc
```

**Attendu :** A valid HTTP response returned within 5s (200 with body 'hello-no-ct'); server does not crash, does not hang, and does not fabricate a guessed Content-Type. (Note: a script emitting ONLY a body with no Content-Type/Location/Status header at all -> 502 per parse(); statusonly.py has Status so it is accepted.)

#### `CGI-15` · P1 · differential-vs-nginx — GAP: PATH_INFO extraction (extra path after script) is unimplemented
**Réf :** REQ-33 / CGI-META-PATH_INFO / CGI-META-PATH_TRANSLATED (code-analysis gap) · **Outil :** curl · **Auto :** oui

**Pré :** env.sh present. Request adds trailing path segments after the script name.

**Procédure :**

```sh
curl -sS -D- 'http://localhost:8080/cgi-bin/env.sh/extra/path.txt?x=1' | grep -E '^(PATH_INFO|SCRIPT_NAME|PATH_TRANSLATED)=' ; echo '--status--'; curl -sS -o /dev/null -w '%{http_code}\n' 'http://localhost:8080/cgi-bin/env.sh/extra/path.txt'
```

**Attendu :** CORRECT (nginx/RFC3875): PATH_INFO=/extra/path.txt, SCRIPT_NAME=/cgi-bin/env.sh, PATH_TRANSLATED set to the mapped physical path. SUSPECTED BUG: buildEnv hardcodes PATH_INFO="" and PATH_TRANSLATED=""; worse, extension dispatch keys on the basename ('path.txt' -> .txt not in cgi map) so the CGI is never executed and the request falls through to static handling (likely 404). Document the divergence.

#### `CGI-16` · P1 · differential-vs-nginx — GAP: SERVER_PORT / SERVER_NAME are hardcoded (wrong on non-8080 port / other Host)
**Réf :** CGI-META-SERVER_PORT / CGI-META-SERVER_NAME (code-analysis gap) · **Outil :** curl · **Auto :** oui

**Pré :** env.sh present. Hit the SAME server on port 8081 (blucken.conf listens 8080-8084) and send a distinct Host.

**Procédure :**

```sh
curl -sS -H 'Host: myhost.test' 'http://localhost:8081/cgi-bin/env.sh' | grep -E '^(SERVER_PORT|SERVER_NAME|HTTP_HOST)='
```

**Attendu :** CORRECT: SERVER_PORT=8081 and SERVER_NAME reflects the bound address/Host (myhost.test). SUSPECTED BUG: SERVER_PORT=8080 (hardcoded literal) and SERVER_NAME=webserv (hardcoded), regardless of the actual listening port or Host header. HTTP_HOST will show the request Host (lowercased).

#### `CGI-17` · P2 · differential-vs-nginx — GAP: request header values are lowercased, corrupting HTTP_* meta-variables
**Réf :** CGI-META-HTTP_STAR / REQ-33 (code-analysis gap #4) · **Outil :** curl · **Auto :** oui

**Pré :** env.sh present.

**Procédure :**

```sh
curl -sS -H 'X-Test-Header: MixedCase123' -H 'User-Agent: ProbeAgentXYZ' 'http://localhost:8080/cgi-bin/env.sh' | grep -E '^HTTP_(X_TEST_HEADER|USER_AGENT)='
```

**Attendu :** CORRECT (RFC 3875): HTTP_X_TEST_HEADER=MixedCase123, HTTP_USER_AGENT=ProbeAgentXYZ (values verbatim, only the NAME is transformed). SUSPECTED BUG: values arrive lowercased -> HTTP_X_TEST_HEADER=mixedcase123, HTTP_USER_AGENT=probeagentxyz (Parser lowercases every header value). Breaks any CGI relying on case-sensitive header values.

#### `CGI-18` · P1 · robustness — GAP: CGI child inherits server file descriptors (no FD_CLOEXEC)
**Réf :** REQ-05 / REQ-11 (code-analysis FD_CLOEXEC gap) · **Outil :** curl · **Auto :** oui

**Pré :** Create www/cgi-bin/fdcount.sh: `#!/bin/sh\nprintf 'Content-Type: text/plain\\r\\n\\r\\n'\nif [ -d /proc/self/fd ]; then ls -1 /proc/self/fd; else ls -1 /dev/fd; fi` ; chmod +x. Child only dup2/resets the 4 pipe fds; no CLOEXEC set at socket/pipe creation.

**Procédure :**

```sh
curl -sS 'http://localhost:8080/cgi-bin/fdcount.sh'
```

**Attendu :** CORRECT: the child should see only fds 0,1,2 (plus at most the stdout pipe). SUSPECTED BUG: extra inherited descriptors appear — the listening socket(s) 8080-8084, other clients' sockets, and log-file fds — proving server fds leak into every CGI (a script could read/write server sockets). More than ~3-4 entries indicates the leak.

#### `CGI-19` · P1 · differential-vs-nginx — GAP: standard form POST (application/x-www-form-urlencoded) to CGI -> 415
**Réf :** REQ-21 / REQ-32 / R-415 (code-analysis gap #5) · **Outil :** curl · **Auto :** oui

**Pré :** echo.sh present +x. blucken.conf types map does NOT contain application/x-www-form-urlencoded; validateHeaders requires the POST Content-Type to exactly equal a configured MIME value.

**Procédure :**

```sh
curl -sS -o /dev/null -w 'form=%{http_code}\n' -H 'Content-Type: application/x-www-form-urlencoded' --data 'a=1&b=2' http://localhost:8080/cgi-bin/echo.sh; curl -sS -o /dev/null -w 'json_param=%{http_code}\n' -H 'Content-Type: application/json; charset=utf-8' --data '{}' http://localhost:8080/cgi-bin/echo.sh
```

**Attendu :** CORRECT (nginx): the CGI runs (200) — content-type gating should not block a body-bearing POST to an executable CGI. SUSPECTED BUG: form=415 and json_param=415, because the exact-string MIME check rejects application/x-www-form-urlencoded (absent from types) and any parameterized value like 'application/json; charset=utf-8' (never equals bare 'application/json'). Real browser/form uploads to CGI are effectively blocked.

#### `CGI-20` · P2 · differential-vs-nginx — Multiple interpreters: .sh executes; .php requires config + REDIRECT_STATUS (gap)
**Réf :** REQ-43 / REQ-37 (code-analysis REDIRECT_STATUS gap) · **Outil :** curl · **Auto :** oui

**Pré :** .sh mapped to /bin/bash in blucken.conf (echo.sh/env.sh already exercise it). For .php: add `cgi_extensions { /usr/bin/php-cgi .php; }` to the /cgi-bin location, install php-cgi, and drop www/cgi-bin/info.php (`<?php echo 'phpok'; ?>`). buildEnv never sets REDIRECT_STATUS.

**Procédure :**

```sh
# .sh interpreter (baseline, should already pass): curl -sS 'http://localhost:8080/cgi-bin/env.sh' | grep -m1 GATEWAY_INTERFACE ; # .php (with the added config): curl -sS -o /dev/null -w 'php=%{http_code}\n' http://localhost:8080/cgi-bin/info.php
```

**Attendu :** env.sh returns 200 with GATEWAY_INTERFACE=CGI/1.1 (bash interpreter path works end-to-end). SUSPECTED BUG for PHP: php-cgi refuses to run without REDIRECT_STATUS (anti-direct-invocation guard) -> the child produces no valid header block -> 502 (or the interpreter error surfaces). Documents that a third interpreter family is not wired correctly.

---

### 6.5 Configuration parsing & routing semantics

_These cases extend Webserv/test/parser_tests (which today only greps parser.log for parsed directive VALUES and asserts non-zero exit for 3 broken files) with (a) the missing invalid-config exit-code matrix and (b) the entirely-absent RUNTIME routing coverage. All runtime cases assume cwd = /Users/blucken/Documents/webserv/Webserv and the launch idiom `./webserv <cfg> & SRV=$!; sleep 0.3; <probe>; kill $SRV 2>/dev/null; wait 2>/dev/null` on unprivileged ports 8080/8081. Grounded in code reality: bind() failure is caught-and-logged, never fatal (Server.cpp:170), so runtime bind conflicts do NOT exit non-zero; server_name/default_server are parsed but never consumed for selection (client is pinned to the ServerConfig whose socket accept()ed it); findLocationConfig() falls back to _locationConfigs.front() (ServerConfig.cpp:447 — UB on empty vector); buildAbsolutPath concatenates root+path with NO location-prefix strip; the error_page templater rewrites EVERY literal 'x' (Parser.cpp:626-635). Cases explicitly targeting a suspected code gap are flagged [GAP] in the title. Differential-vs-nginx cases state both the subject-required/nginx result and webserv's actual behavior so the evaluator can see the divergence. P0 = crash/hang or a mandatory feature (REQ-11/15/16/24/28, invalid-config-must-exit); P1 = important correctness; P2 = nice-to-have/edge._

#### `CFG-01` · P0 · integration — Config with no http{} wrapper must exit non-zero
**Réf :** 42-subject config-parse (Parser.cpp:466); ABNF ServerConfig.abnf:10-12 · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv, ./webserv built

**Procédure :**

```sh
printf 'server { listen 8080; }\n' > /tmp/no_http.conf; ./webserv /tmp/no_http.conf; echo "exit=$?"
```

**Attendu :** exit != 0 (EXIT_FAILURE). Whole-file ABNF match `config = OWS http-block OWS` fails at Parser.cpp:466 ('Failed to match config'); process must NOT begin listening. nginx `nginx -t` likewise rejects.

#### `CFG-02` · P0 · integration — Two http{} blocks must exit non-zero
**Réf :** ABNF ServerConfig.abnf:10 (config = OWS http-block OWS); Parser.cpp:466 · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
printf 'http { server { listen 8080; } }\nhttp { server { listen 8081; } }\n' > /tmp/two_http.conf; ./webserv /tmp/two_http.conf; echo "exit=$?"
```

**Attendu :** exit != 0. Grammar allows exactly one non-repeating http-block; the second causes the top-level ABNF match to fail. Must not start.

#### `CFG-03` · P0 · integration — http{} with zero server{} blocks must exit non-zero
**Réf :** Parser.cpp:105-109; existing empty.conf gap · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
printf 'http {\n  root /tmp;\n}\n' > /tmp/zero_srv.conf; ./webserv /tmp/zero_srv.conf 2>&1 | tail -1; echo "exit=${PIPESTATUS[0]}"
```

**Attendu :** exit != 0 and a 'No server blocks found in config' diagnostic (Parser.cpp:105-109 throws std::runtime_error). Distinct from empty.conf (which is a 1-byte '\n' file failing the ABNF earlier).

#### `CFG-04` · P0 · integration — Duplicate singleton directive (two root;) must exit non-zero
**Réf :** config duplicate-singleton (Parser.cpp:515-522) · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
printf 'http { server { listen 8080; root /a; root /b; location / { } } }\n' > /tmp/dup_root.conf; ./webserv /tmp/dup_root.conf; echo "exit=$?"
```

**Attendu :** exit != 0. ensureAtMostOne (Parser.cpp:515-522) rejects a second `root`. Repeat for a second `client_max_body_size;` and a second `autoindex;` — all must exit non-zero. (Contrast: error_page/listen/server_name/cgi_extension ARE repeatable and must NOT fail.)

#### `CFG-05` · P1 · integration — Unknown directive / missing semicolon must exit non-zero
**Réf :** ABNF ServerConfig.abnf:18 (server-block); Parser.cpp:466 · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
printf 'http { server { listen 8080; frobnicate yes; location / { } } }\n' > /tmp/unk.conf; ./webserv /tmp/unk.conf; echo "unknown_exit=$?"; printf 'http { server { listen 8080 root /a; location / { } } }\n' > /tmp/nosemi.conf; ./webserv /tmp/nosemi.conf; echo "nosemi_exit=$?"
```

**Attendu :** Both exit != 0. An unrecognized directive token and a missing terminating ';' each break the server-block ABNF match. Must not start.

#### `CFG-06` · P1 · integration — allowed_methods with an invalid token must exit non-zero
**Réf :** REQ-26; ABNF allowed-methods; grammar-token validation · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
printf 'http { server { listen 8080; location / { allowed_methods GET FOO; } } }\n' > /tmp/bad_method.conf; ./webserv /tmp/bad_method.conf; echo "exit=$?"
```

**Attendu :** exit != 0. Grammar restricts method tokens to GET/HEAD/POST/PUT/DELETE; 'FOO' fails the ABNF. (Also verify `io_multiplexer` handled separately in CFG-07.)

#### `CFG-07` · P1 · integration — io_multiplexer with an invalid value must exit non-zero
**Réf :** REQ-05; io-multiplexer-dir (Parser.cpp:82-88) · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv (http-level only directive)

**Procédure :**

```sh
printf 'http { io_multiplexer bogus; server { listen 8080; location / { } } }\n' > /tmp/bad_mux.conf; ./webserv /tmp/bad_mux.conf; echo "exit=$?"
```

**Attendu :** exit != 0. io-multiplexer-val must be one of poll/select/kqueue/epoll; 'bogus' fails the ABNF (Parser.cpp:82-88). Valid values (`select`, `poll`) must NOT fail — assert `io_multiplexer select;` starts.

#### `CFG-08` · P1 · integration — [GAP] client_max_body_size 5G overflows unsigned int and must exit non-zero
**Réf :** REQ-25; parseMultiplier overflow guard (Parser.hpp:151-169) · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv; t_clientMaxBodySize is 32-bit unsigned

**Procédure :**

```sh
printf 'http { server { listen 8080; client_max_body_size 5G; location / { } } }\n' > /tmp/cmbs_of.conf; ./webserv /tmp/cmbs_of.conf; echo "of_exit=$?"; printf 'http { server { listen 8080; client_max_body_size 0x10; location / { } } }\n' > /tmp/cmbs_hex.conf; ./webserv /tmp/cmbs_hex.conf & P=$!; sleep 0.3; grep -m1 'Client max body size' logs/webserv/parser.log; kill $P 2>/dev/null
```

**Attendu :** 5G (5368709120 > UINT_MAX 4294967295) -> parseMultiplier throws 'Invalid numeric value', exit != 0. Existing suite already accepts 2G (2147483648). Also document the strtoul base-0 quirk: `0x10` is silently parsed as 16 bytes (log shows '16 bytes') — surprising but non-fatal; flag as a spec ambiguity.

#### `CFG-09` · P2 · integration — Argument without .conf extension / nonexistent file must exit non-zero
**Réf :** REQ-01/REQ-02 (main.cpp:32-41) · **Outil :** bash + webserv · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
./webserv /etc/hosts; echo "ext_exit=$?"; ./webserv /tmp/does_not_exist_12345.conf; echo "missing_exit=$?"
```

**Attendu :** Both exit != 0. main.cpp:39 rejects any argv[1] not ending in '.conf' ('must have .conf extension'); a missing but .conf-named file throws when opened (HTTPServer.cpp:236-240). Must not start.

#### `CFG-10` · P1 · differential-vs-nginx — [GAP] error_page templater rewrites EVERY literal 'x', mangling real paths
**Réf :** REQ-16; error_page templating gap (Parser.cpp:626-635) · **Outil :** bash + webserv + grep parser.log · **Auto :** oui

**Pré :** cwd=Webserv; LOGFILE=logs/webserv/parser.log

**Procédure :**

```sh
printf 'http { server { listen 8080; error_page 404 /maxpage.html; error_page 500 502 503 504 /50x.html; location / { } } }\n' > /tmp/ep_x.conf; > logs/webserv/parser.log; ./webserv /tmp/ep_x.conf & P=$!; sleep 0.3; grep 'Error page:' logs/webserv/parser.log; kill $P 2>/dev/null
```

**Attendu :** Log shows 'Error page: 404 -> /ma4page.html' (the 'x' in 'maxpage' greedily replaced by code digit '4' at Parser.cpp:626-635), NOT the intended '/maxpage.html'. The intended placeholder case still works: '500 -> /500.html' ... '504 -> /504.html'. nginx treats the URI literally and would map 404 -> /maxpage.html. This is a real config-corruption bug: any legitimate path containing 'x' is rewritten.

#### `CFG-11` · P1 · differential-vs-nginx — Child-scope error_page replaces (does not merge) inherited parent list
**Réf :** REQ-16; nginx error_page inheritance; config gap (Parser.hpp:243-245) · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
mkdir -p /tmp/ir && printf 'PARENT-404-BODY' > /tmp/ir/p404.html && printf 'X' > /tmp/ir/p500.html; printf 'http {\n  error_page 404 /p404.html;\n  server { listen 8080; root /tmp/ir; error_page 500 /p500.html; location / { allowed_methods GET; } }\n}\n' > /tmp/ep_inherit.conf; ./webserv /tmp/ep_inherit.conf & SRV=$!; sleep 0.3; curl -s -o body.txt -w '%{http_code}\n' --max-time 5 http://127.0.0.1:8080/nope.html; cat body.txt; echo; kill $SRV 2>/dev/null
```

**Attendu :** HTTP 404, but body is the built-in generated error page (NOT 'PARENT-404-BODY'): because the server declared its own error_page (for 500), setErrorPage REPLACES the inherited http-level list wholesale (Parser.hpp:243-245), discarding the 404->/p404.html mapping. This MATCHES nginx's rule ('inherited only if no error_page at this level'), so it is correct behavior — the test guards against a future merge regression and confirms the custom page is dropped as nginx would.

#### `CFG-12` · P1 · integration — http-level directive inherits into server/location at request time
**Réf :** REQ-28; applyParentDefaults inheritance (Parser.hpp:326-340) · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
mkdir -p /tmp/inh && printf 'INHERITED-ROOT' > /tmp/inh/index.html; printf 'http {\n  root /tmp/inh;\n  server { listen 8080; location / { index index.html; allowed_methods GET; } }\n}\n' > /tmp/inh.conf; ./webserv /tmp/inh.conf & SRV=$!; sleep 0.3; curl -s -w ' [%{http_code}]\n' --max-time 5 http://127.0.0.1:8080/; kill $SRV 2>/dev/null
```

**Attendu :** 200 with body exactly 'INHERITED-ROOT'. The location omits `root`, so applyParentDefaults must have propagated the http-level root (/tmp/inh) down through the server into the location; the index file resolves to /tmp/inh/index.html. Proves runtime application of inherited defaults (existing suite only checks the parse LOG, never behavior).

#### `RT-01` · P0 · differential-vs-nginx — [GAP] Root path mapping: /kapouet -> /tmp/www (REQ-28 strip vs webserv concat)
**Réf :** REQ-28 (buildAbsolutPath RequestHandler.cpp:404) · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
mkdir -p /tmp/www/pouic/toto /tmp/www/kapouet/pouic/toto; printf 'STRIPPED' > /tmp/www/pouic/toto/pouet; printf 'CONCAT' > /tmp/www/kapouet/pouic/toto/pouet; printf 'http { server { listen 8080; location /kapouet { root /tmp/www; allowed_methods GET; } } }\n' > /tmp/kap.conf; ./webserv /tmp/kap.conf & SRV=$!; sleep 0.3; curl -s -w ' [%{http_code}]\n' --max-time 5 http://127.0.0.1:8080/kapouet/pouic/toto/pouet; kill $SRV 2>/dev/null
```

**Attendu :** The 42 subject REQ-28 example mandates STRIP semantics -> body 'STRIPPED' (serving /tmp/www/pouic/toto/pouet). webserv's buildAbsolutPath concatenates root+full-URI with NO prefix strip, so it will return 'CONCAT' (/tmp/www/kapouet/pouic/toto/pouet) — matching nginx `root` (not `alias`). Flag the divergence: if the evaluator reads REQ-28 as alias-strip, webserv fails; if as nginx-root, it passes. Either way must be 200, never 404/crash.

#### `RT-02` · P1 · integration — Location matching: EXACT '=' precedence + longest-prefix selection
**Réf :** REQ-28; location matching (ServerConfig.cpp:415-448); return REQ-27 · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv; uses return codes so routing is filesystem-independent

**Procédure :**

```sh
printf 'http { server { listen 8080;\n  location = /ping { return 201; }\n  location /ping { return 202; }\n  location /pi { return 203; }\n  location / { return 204; }\n} }\n' > /tmp/loc.conf; ./webserv /tmp/loc.conf & SRV=$!; sleep 0.3; for p in /ping /pinga /pix /zzz; do printf '%s -> ' "$p"; curl -s -o /dev/null -w '%{http_code}\n' --max-time 5 "http://127.0.0.1:8080$p"; done; kill $SRV 2>/dev/null
```

**Attendu :** /ping -> 201 (EXACT '=' wins over the /ping prefix, ServerConfig.cpp:424-430); /pinga -> 202 (longest matching prefix /ping beats /pi and /); /pix -> 203 (/pi beats /); /zzz -> 204 (fallback to /). Confirms EXACT-first then longest-prefix (raw string prefix, matching nginx prefix semantics).

#### `RT-03` · P0 · robustness — [GAP] Server with no location block must not crash on any request
**Réf :** REQ-11 (no crash); findLocationConfig empty-vector gap (ServerConfig.cpp:447) · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv; grammar allows a server{} with zero location{}

**Procédure :**

```sh
printf 'http { server { listen 8080; root /tmp; } }\n' > /tmp/no_loc.conf; ./webserv /tmp/no_loc.conf & SRV=$!; sleep 0.3; curl -s -o /dev/null -w 'first=%{http_code}\n' --max-time 5 http://127.0.0.1:8080/; kill -0 $SRV 2>/dev/null && echo 'ALIVE' || echo 'DEAD'; curl -s -o /dev/null -w 'second=%{http_code}\n' --max-time 5 http://127.0.0.1:8080/; kill $SRV 2>/dev/null
```

**Attendu :** Server MUST stay alive ('ALIVE') and return a controlled status to both requests (e.g. 404/403/500), never crash. Actual risk: findLocationConfig returns _locationConfigs.front() on an EMPTY vector (ServerConfig.cpp:447) -> undefined behavior / SIGSEGV -> connection reset, 'DEAD', second curl fails. Any crash = grade 0.

#### `RT-04` · P1 · differential-vs-nginx — [GAP] Two server{} on the same host:port — server_name/Host routing
**Réf :** REQ-41/REQ-24; server_name unused + silent bind-fail (ServerFactory.cpp / Server.cpp:170) · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
mkdir -p /tmp/sa /tmp/sb; printf 'AAA' > /tmp/sa/index.html; printf 'BBB' > /tmp/sb/index.html; printf 'http {\n  server { listen 8080; server_name a.test; root /tmp/sa; location / { index index.html; allowed_methods GET; } }\n  server { listen 8080; server_name b.test; root /tmp/sb; location / { index index.html; allowed_methods GET; } }\n}\n' > /tmp/vhost.conf; ./webserv /tmp/vhost.conf & SRV=$!; sleep 0.3; curl -s -H 'Host: a.test' --max-time 5 http://127.0.0.1:8080/; echo; curl -s -H 'Host: b.test' --max-time 5 http://127.0.0.1:8080/; echo; kill -0 $SRV 2>/dev/null && echo ALIVE; kill $SRV 2>/dev/null
```

**Attendu :** nginx (subject REQ-41/REQ-24): Host: a.test -> 'AAA', Host: b.test -> 'BBB'. webserv ACTUAL: both return 'AAA' — the 2nd server's bind(8080) fails EADDRINUSE (SO_REUSEADDR set, SO_REUSEPORT not), is caught+logged (Server.cpp:170) and its socket never listens; server_name is never consulted for selection, so block-1 serves every request. Server must stay ALIVE and answer (no hang/crash). Documents that Host-based virtual-host selection is unimplemented.

#### `RT-05` · P1 · integration — Two server{} on different host:port both reachable from one process
**Réf :** REQ-23, REQ-24 (multiple interface:port pairs) · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
mkdir -p /tmp/sa /tmp/sb; printf 'PORT-8080' > /tmp/sa/index.html; printf 'PORT-8081' > /tmp/sb/index.html; printf 'http {\n  server { listen 8080; root /tmp/sa; location / { index index.html; allowed_methods GET; } }\n  server { listen 8081; root /tmp/sb; location / { index index.html; allowed_methods GET; } }\n}\n' > /tmp/multiport.conf; ./webserv /tmp/multiport.conf & SRV=$!; sleep 0.3; curl -s --max-time 5 http://127.0.0.1:8080/; echo; curl -s --max-time 5 http://127.0.0.1:8081/; echo; kill $SRV 2>/dev/null
```

**Attendu :** 8080 -> 'PORT-8080', 8081 -> 'PORT-8081', both served concurrently by the single webserv process with the content configured per listen. This is the MANDATORY multi-port requirement and should PASS (distinct from RT-04's same-port conflict).

#### `RT-06` · P2 · differential-vs-nginx — [GAP] default_server selection for a host:port (unknown Host)
**Réf :** REQ-41; default_server flag never consumed (Parser.cpp:745-746) · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
mkdir -p /tmp/s1 /tmp/s2; printf 'FIRST' > /tmp/s1/index.html; printf 'DEFAULTSRV' > /tmp/s2/index.html; printf 'http {\n  server { listen 8080; server_name one.test; root /tmp/s1; location / { index index.html; allowed_methods GET; } }\n  server { listen 8080 default_server; server_name two.test; root /tmp/s2; location / { index index.html; allowed_methods GET; } }\n}\n' > /tmp/defsrv.conf; ./webserv /tmp/defsrv.conf & SRV=$!; sleep 0.3; curl -s -H 'Host: unknown.test' --max-time 5 http://127.0.0.1:8080/; echo; kill $SRV 2>/dev/null
```

**Attendu :** nginx: an unknown Host on 8080 is served by the block marked `default_server` -> 'DEFAULTSRV'. webserv ACTUAL: 'FIRST' — the first-declared block wins the bind (block 2's bind fails and never listens), and Listen.defaultServer is parsed but never read. Documents that per-host default-server selection is unimplemented (bonus REQ-41).

#### `RT-07` · P0 · integration — Custom error_page served byte-exact on 404 (REQ-16)
**Réf :** REQ-16 (custom error page); REQ-15 (default fallback); R-404 · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
mkdir -p /tmp/ep && printf '<html><body>MY CUSTOM 404 PAGE</body></html>' > /tmp/ep/e404.html; printf 'http { server { listen 8080; root /tmp/ep; error_page 404 /e404.html; location / { allowed_methods GET; } } }\n' > /tmp/errpage.conf; ./webserv /tmp/errpage.conf & SRV=$!; sleep 0.3; curl -s -o got.txt -w '%{http_code}\n' --max-time 5 http://127.0.0.1:8080/missing.html; diff <(printf '<html><body>MY CUSTOM 404 PAGE</body></html>') got.txt && echo BYTE-EXACT || echo MISMATCH; kill $SRV 2>/dev/null
```

**Attendu :** HTTP 404 with body byte-identical to /tmp/ep/e404.html ('BYTE-EXACT'); Content-Type text/html; Content-Length equals the file size. Resolution is root+ep.getPath() = /tmp/ep/e404.html (ResponseHandler.cpp:288-314). Removing the error_page directive must instead yield the built-in default 404 body (guards REQ-15).

#### `RT-08` · P1 · integration — Configured return redirect emits 3xx + Location (REQ-27)
**Réf :** REQ-27; R-301/R-302/R-LOCATION-3XX (ExecutionHandler.cpp:161-166) · **Outil :** bash + curl · **Auto :** oui

**Pré :** cwd=Webserv

**Procédure :**

```sh
printf 'http { server { listen 8080;\n  location = /old { return 301 http://example.com/new; }\n  location /temp { return 302 /modern; }\n  location / { return 204; }\n} }\n' > /tmp/redir.conf; ./webserv /tmp/redir.conf & SRV=$!; sleep 0.3; curl -s -o /dev/null -D - --max-time 5 http://127.0.0.1:8080/old | grep -iE '^HTTP/|^location:'; echo '---'; curl -s -o /dev/null -D - --max-time 5 http://127.0.0.1:8080/temp/x | grep -iE '^HTTP/|^location:'; kill $SRV 2>/dev/null
```

**Attendu :** /old -> status line '301' with 'Location: http://example.com/new'; /temp/x -> '302' with 'Location: /modern'. Matches nginx `return 301|302 <url>;`. The Location header MUST be present on every Location-bearing 3xx (R-LOCATION-3XX).

---

### 6.6 Connection management, I/O & non-blocking guarantees

_Common preconditions for every case below: webserv is running as `./webserv conf/test.conf` bound to 127.0.0.1:8080; `PID=$(pgrep -f '[w]ebserv conf/test.conf')`. The test config must expose: a static doc root serving `/` and `/index.html`; a 50 MiB file `/big.bin` (create once: `dd if=/dev/zero of=<root>/big.bin bs=1m count=50`); a POST-accepting route `/upload`; and at least one server/route whose `keepalive_timeout` is set small (5s) so idle-reap timing is observable in seconds. For differential cases, a stock nginx serving the same doc root runs on 127.0.0.1:8081. These tests are deliberately adversarial and several directly target suspected gaps proven in the code read: BLOCKING client sockets (no `setIsNonblock` on accepted fds) which violate REQ-04/05/06 by letting one slow reader freeze the loop inside `send()`; the premature-400 on any header section split across recv() calls (RequestHandler.cpp:108, gap #1); permanent E_OUT registration causing a busy-spin (ClientHandler.cpp:81); `errno` read after recv/send with any -1 treated as disconnect (REQ-08, ATcpSocket.cpp:90/108); no request-read timeout because last-activity resets per byte (ClientHandler.cpp:172); GET/HEAD bodies never consumed → keep-alive desync (Client.cpp:381, gap #3); and broken graceful shutdown (g_SignalStatus never set). Each such case names the gap in its expected result so a failure is traceable._

#### `CONN-01` · P1 · integration — HTTP/1.1 persistent connection is the default; sequential requests reuse one socket (stability loop)
**Réf :** RFC9112 R26 §9.3; REQ-04 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server up on 127.0.0.1:8080; /index.html exists.

**Procédure :**

```sh
Run 10 times to catch nondeterminism from the uninitialized Response::_shouldCloseConnection flag:
for i in $(seq 1 10); do python3 - <<'PY'
import socket
s=socket.create_connection(("127.0.0.1",8080)); s.settimeout(4)
n=0
for _ in range(3):
    s.sendall(b"GET /index.html HTTP/1.1\r\nHost: x\r\n\r\n")
    r=b""
    while b"\r\n\r\n" not in r: r+=s.recv(4096)
    assert r.startswith(b"HTTP/1.1 200"), r[:40]
    n+=1
print("OK reused",n)
PY
done
```

**Attendu :** All 10 iterations print `OK reused 3`: no Connection header (or `Connection: keep-alive`) keeps the socket open and all three sequential requests on the SAME socket return `HTTP/1.1 200`. Any iteration where the server closes the socket after request #1/#2 (recv returns b'' unexpectedly) exposes the uninitialized `_shouldCloseConnection` flag (Response.cpp ctor never initializes it; ResponseHandler.cpp:409 only ever sets it true).

#### `CONN-02` · P1 · integration — Connection: close is honored — exactly one response then server-initiated close
**Réf :** RFC9112 R25 §9.6 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server up on 127.0.0.1:8080.

**Procédure :**

```sh
python3 - <<'PY'
import socket
s=socket.create_connection(("127.0.0.1",8080)); s.settimeout(4)
s.sendall(b"GET / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
buf=b""
while True:
    d=s.recv(4096)
    if not d: break
    buf+=d
print("resp bytes",len(buf))
print("has_close_hdr", b"connection: close" in buf.lower())
print("eof_seen", True)
PY
```

**Attendu :** Server sends exactly ONE response carrying `Connection: close`, then closes the socket (recv returns b'' → loop ends). `eof_seen` prints True. Matches RFC9112 §9.6 (server MUST initiate closure after responding to a `close` request).

#### `CONN-03` · P1 · differential-vs-nginx — HTTP pipelining: two requests written in one segment answered in order
**Réf :** RFC9112 R26 §9.3; REQ-38 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server on :8080; nginx oracle on :8081; /index.html exists, /missing404 does not.

**Procédure :**

```sh
for P in 8080 8081; do echo "== port $P =="; python3 - $P <<'PY'
import socket,sys
p=int(sys.argv[1])
s=socket.create_connection(("127.0.0.1",p)); s.settimeout(4)
s.sendall(b"GET /index.html HTTP/1.1\r\nHost: x\r\n\r\nGET /missing404 HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
buf=b""
while True:
    d=s.recv(4096)
    if not d: break
    buf+=d
statuses=[l for l in buf.split(b"\r\n") if l.startswith(b"HTTP/1.1 ")]
print(statuses)
PY
done
```

**Attendu :** Both requests are answered on the one connection, IN ORDER: first status line is `HTTP/1.1 200` (index.html), second is `HTTP/1.1 404` (missing404), then close. Two status lines total, same order as nginx on :8081. Server must not answer only one, reorder, or hang.

#### `CONN-04` · P1 · robustness — GET with a Content-Length body must not desync the connection (body-leak smuggling)
**Réf :** RFC9112 §6.3; REQ-04 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server on :8080; /index.html exists. TARGETS GAP #3 (Client.cpp:381 — parseBody only runs for POST/PUT/DELETE, so a GET body is never consumed and leaks into the next request).

**Procédure :**

```sh
python3 - <<'PY'
import socket
inj=b"GET /INJECTED HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n"
head=("GET /index.html HTTP/1.1\r\nHost: x\r\nContent-Length: %d\r\n\r\n"%len(inj)).encode()
s=socket.create_connection(("127.0.0.1",8080)); s.settimeout(4)
s.sendall(head+inj)
buf=b""
try:
    while True:
        d=s.recv(4096)
        if not d: break
        buf+=d
except socket.timeout: pass
statuses=[l for l in buf.split(b"\r\n") if l.startswith(b"HTTP/1.1 ")]
print("num_responses",len(statuses),statuses)
print("injected_processed", b"/INJECTED" in buf or len(statuses)>1)
PY
```

**Attendu :** Correct/nginx behavior: exactly ONE HTTP response (to /index.html); the declared 44-byte GET body is read and discarded, leaving the connection synced. If `num_responses` is 2 (server produced a second status line by parsing the leaked body as a fresh `GET /INJECTED` request), the connection desynced — confirms GET/HEAD bodies are never consumed (request-smuggling primitive).

#### `CONN-05` · P0 · differential-vs-nginx — Byte-by-byte / segmented header delivery must NOT trigger a premature 400
**Réf :** REQ-04, REQ-10; RFC9112 §2.2 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server on :8080; nginx on :8081. TARGETS GAP #1 (RequestHandler.cpp:108 throws 400 whenever CRLFCRLF is not yet in the buffer — any request split across recv() calls is rejected).

**Procédure :**

```sh
for P in 8080 8081; do echo "== port $P =="; python3 - $P <<'PY'
import socket,time,sys
p=int(sys.argv[1])
s=socket.create_connection(("127.0.0.1",p)); s.settimeout(0.6)
for chunk in [b"GET / HTTP/1.1\r\n", b"Host: x\r\n", b"User-Agent: drip\r\n", b"\r\n"]:
    s.sendall(chunk); time.sleep(0.3)
    try:
        early=s.recv(4096)
        print("PREMATURE_RESPONSE:",early[:40])   # ANY data before final CRLF = BUG
    except socket.timeout:
        print("waiting (correct)")
s.settimeout(3)
try: print("FINAL:", s.recv(4096)[:40])
except socket.timeout: print("FINAL: <none>")
PY
done
```

**Attendu :** nginx (:8081): three `waiting (correct)` lines, then `FINAL: HTTP/1.1 200/404` once the terminating CRLF arrives. Correct webserv must match. SUSPECTED webserv failure: a `PREMATURE_RESPONSE: HTTP/1.1 400 Bad Request` appears after the first or second chunk — the server rejected a well-formed request only because its header section spanned multiple recv() calls (gap #1). This breaks REQ-04 (slow clients) and REQ-13/38 (real browsers/TCP segmentation).

#### `CONN-06` · P1 · robustness — Slow-loris trickle (chunked) must be bounded by a request timeout, not held indefinitely
**Réf :** REQ-10, REQ-12; RFC9112 §8 R30 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server on :8080 with a small keepalive_timeout (e.g. 5s) on /upload. TARGETS the no-request-timeout gap (ClientHandler.cpp:172 resets last-activity on every inbound byte; decodeChunkedBody waits across reads).

**Procédure :**

```sh
time python3 - <<'PY'
import socket,time
s=socket.create_connection(("127.0.0.1",8080)); s.settimeout(2)
s.sendall(b"POST /upload HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n")
start=time.time(); closed=False
while time.time()-start < 30:
    try: s.sendall(b"1\r\nA\r\n")           # one 1-byte chunk every 4s (< keepalive)
    except OSError: closed=True; break
    try:
        r=s.recv(256)
        if not r: closed=True; break
    except socket.timeout: pass
    time.sleep(4)
print("closed_by_server" if closed else "STILL_OPEN_AFTER_30s")
PY
```

**Attendu :** A conformant server enforces a request-read/body timeout and closes (or returns 408) the trickling connection within a bounded window even though bytes keep arriving → prints `closed_by_server`. SUSPECTED webserv failure: `STILL_OPEN_AFTER_30s` because keepalive last-activity is refreshed on every byte and there is no separate header/body/total-request deadline — an attacker holds fds forever (unbounded with poll). Server must at minimum never hang the whole process (concurrent curl in CONN-14 still works).

#### `CONN-07` · P1 · robustness — Incomplete body (Content-Length larger than bytes sent) must not hang the process
**Réf :** REQ-10, REQ-04; RFC9112 §8 R30 · **Outil :** python3 socket + curl · **Auto :** oui

**Pré :** Server on :8080; /upload accepts POST.

**Procédure :**

```sh
python3 - <<'PY' &
import socket,time
s=socket.create_connection(("127.0.0.1",8080))
s.sendall(b"POST /upload HTTP/1.1\r\nHost: x\r\nContent-Length: 100000\r\n\r\nonly-a-few-bytes")
time.sleep(20)   # hold the socket, never send the rest
PY
BG=$!
# Other clients must remain served the whole time:
for i in 1 2 3; do curl -s -o /dev/null -w '%{http_code} in %{time_total}s\n' --max-time 3 http://127.0.0.1:8080/; sleep 1; done
kill $BG 2>/dev/null
```

**Attendu :** Every concurrent curl returns 200 within its 3s budget while the stalled client holds its half-sent body — the incomplete request never freezes the single-threaded loop (REQ-04/REQ-10). The stalled connection is eventually closed/408'd by an idle or request timeout, not held forever. `kill -0 $PID` still succeeds afterward.

#### `CONN-08` · P1 · integration — Idle keep-alive connection is reaped after keepalive_timeout
**Réf :** REQ-10, REQ-04; RFC9112 §9.3 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server on :8080; the matched route/server has keepalive_timeout 5.

**Procédure :**

```sh
python3 - <<'PY'
import socket,time
s=socket.create_connection(("127.0.0.1",8080)); s.settimeout(15)
s.sendall(b"GET / HTTP/1.1\r\nHost: x\r\n\r\n")
r=b""
while b"\r\n\r\n" not in r: r+=s.recv(4096)
t0=time.time()
try:
    while True:
        d=s.recv(4096)
        if not d: break   # server closed the idle connection
except socket.timeout: pass
print("server_closed_after_%.1fs"%(time.time()-t0))
PY
```

**Attendu :** After the response, with no further client activity, the server closes the idle connection roughly `keepalive_timeout` seconds later (~5s here; observed value between 5 and ~6s given the 100ms loop and 1s time() granularity). If keepalive_timeout were configured 0, the connection stays open indefinitely (documented behavior). No crash, no leak.

#### `CONN-09` · P0 · robustness — One stalled slow reader must NOT starve other clients (blocking-send head-of-line)
**Réf :** REQ-04, REQ-05, REQ-06 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server on :8080; /big.bin is 50 MiB; /index.html exists. TARGETS the blocking-socket gap: accepted client fds are never set O_NONBLOCK, so Client::sendData() (Client.cpp:342) does a blocking send() of the whole buffer.

**Procédure :**

```sh
python3 - <<'PY'
import socket,time
slow=socket.socket()
slow.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,2048)   # tiny window
slow.connect(("127.0.0.1",8080))
slow.sendall(b"GET /big.bin HTTP/1.1\r\nHost: x\r\n\r\n")
time.sleep(1.0)   # let the server push until kernel send buffer fills, then it blocks in send()
t0=time.time()
fast=socket.create_connection(("127.0.0.1",8080)); fast.settimeout(6)
fast.sendall(b"GET /index.html HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
try:
    d=fast.recv(4096); dt=time.time()-t0
    print("fast_responded_in_%.2fs"%dt, d[:20])
except socket.timeout:
    print("fast_STARVED_timeout (BUG)")
PY
```

**Attendu :** Non-blocking / nginx-like behavior: the fast client gets `HTTP/1.1 200` in well under 1s while the slow reader is still stalled (`fast_responded_in_0.xxs`). SUSPECTED webserv failure: `fast_STARVED_timeout` or a multi-second `dt`, because the event loop is blocked inside a blocking send() to the non-reading slow client — a direct violation of REQ-04/05/06 (never block, monitor read+write simultaneously, one slow client must never freeze others).

#### `CONN-10` · P1 · integration — Large response is delivered completely and byte-identically to a slow reader (partial-write handling)
**Réf :** REQ-06, REQ-18; RFC9110 R-FRAMING · **Outil :** python3 socket + md5 · **Auto :** oui

**Pré :** Server on :8080; /big.bin is 50 MiB. Tests eraseBufferResponseFront partial-write advance (Client.cpp:360) and readChunk streaming.

**Procédure :**

```sh
REF=$(md5 -q <root>/big.bin 2>/dev/null || md5sum <root>/big.bin | cut -d' ' -f1)
python3 - <<'PY'
import socket,time,hashlib
s=socket.create_connection(("127.0.0.1",8080)); s.settimeout(30)
s.sendall(b"GET /big.bin HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
buf=b""
while True:
    try: d=s.recv(4096)
    except socket.timeout: break
    if not d: break
    buf+=d; time.sleep(0.0005)   # read slowly to force many partial writes
hdr,_,body=buf.partition(b"\r\n\r\n")
cl=[l.split(b":",1)[1].strip() for l in hdr.split(b"\r\n") if l.lower().startswith(b"content-length")]
print("content_length_hdr",cl,"body_len",len(body),"md5",hashlib.md5(body).hexdigest())
PY
echo "file md5 = $REF (expect 52428800 bytes)"
```

**Attendu :** `body_len` == 52428800 and its md5 == the on-disk file md5; the `Content-Length` header equals 52428800. No truncation, no duplicated/corrupted bytes across the many partial writes. (Also flags the getFileSize int-truncation risk if a >2 GiB file is substituted — Content-Length would go wrong/negative.)

#### `CONN-11` · P0 · stress — Many simultaneous keep-alive clients: sustained availability, no crash, bounded latency
**Réf :** REQ-12, REQ-11, REQ-05 · **Outil :** siege · **Auto :** oui

**Pré :** Server on :8080 serving /; siege installed.

**Procédure :**

```sh
siege -b -c 150 -t 30S http://127.0.0.1:8080/ ; echo '--- post-stress liveness ---'; kill -0 $PID && echo 'server alive'; curl -s -o /dev/null -w 'recheck=%{http_code}\n' --max-time 3 http://127.0.0.1:8080/
```

**Attendu :** Availability reported ~100.00% with 0 failed transactions; response time stays bounded (no runaway); after the run `server alive` prints and `recheck=200`. No crash, no hung/leaked connections, process remains responsive (REQ-11/REQ-12). If availability drops or the process dies, the single-loop concurrency handling is deficient.

#### `CONN-12` · P0 · robustness — No file-descriptor leak under repeated connect / partial-request / abrupt-close churn
**Réf :** REQ-11, REQ-12, REQ-04 · **Outil :** nc + lsof · **Auto :** oui

**Pré :** Server on :8080; PID known.

**Procédure :**

```sh
BASE=$(lsof -p $PID 2>/dev/null | wc -l); echo "baseline fds=$BASE"
for i in $(seq 1 500); do printf 'GET / HTTP/1.1\r\nHost: x' | nc -w1 127.0.0.1 8080 >/dev/null 2>&1; done
sleep 3
AFTER=$(lsof -p $PID 2>/dev/null | wc -l); echo "after fds=$AFTER"
kill -0 $PID && echo 'server alive'
```

**Attendu :** `after` fd count is within a small constant of `baseline` (no monotonic growth) and `server alive` prints. Each connection sent an incomplete request then closed; the server must fully reclaim every fd (removeClient unregisters from the multiplexer and RAII-closes the socket). A steadily rising fd count is an fd leak → eventual exhaustion (REQ-11/REQ-12).

#### `CONN-13` · P0 · robustness — Client resets (RST) mid-response — SIGPIPE must be ignored, server survives
**Réf :** REQ-04, REQ-11 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server on :8080; /big.bin is 50 MiB. Verifies main.cpp:12 signal(SIGPIPE, SIG_IGN) and disconnect handling on send error.

**Procédure :**

```sh
python3 - <<'PY'
import socket,struct,time
s=socket.socket(); s.connect(("127.0.0.1",8080))
s.sendall(b"GET /big.bin HTTP/1.1\r\nHost: x\r\n\r\n")
time.sleep(0.3); s.recv(4096)                      # read a little of the response
s.setsockopt(socket.SOL_SOCKET,socket.SO_LINGER,struct.pack('ii',1,0))  # RST on close
s.close()                                          # abort mid-stream
PY
sleep 1
kill -0 $PID && echo 'server alive' || echo 'SERVER DIED (BUG)'
curl -s -o /dev/null -w 'recheck=%{http_code}\n' --max-time 3 http://127.0.0.1:8080/
```

**Attendu :** `server alive` and `recheck=200`. The server's next send() to the reset peer returns EPIPE; because SIGPIPE is ignored the process does not die — the wrapper throws, the client is marked disconnected and removed. No crash (REQ-11), other clients unaffected (REQ-04).

#### `CONN-14` · P0 · robustness — Abrupt client disconnect mid-request does not crash or hang the server
**Réf :** REQ-04, REQ-11 · **Outil :** python3 socket + curl · **Auto :** oui

**Pré :** Server on :8080; /upload accepts POST.

**Procédure :**

```sh
python3 - <<'PY'
import socket
s=socket.create_connection(("127.0.0.1",8080))
s.sendall(b"POST /upload HTTP/1.1\r\nHost: x\r\nContent-Length: 1000\r\n\r\nPARTIAL-BODY-ONLY")
s.close()   # drop the connection with body incomplete
PY
curl -s -o /dev/null -w 'concurrent=%{http_code} in %{time_total}s\n' --max-time 3 http://127.0.0.1:8080/
kill -0 $PID && echo 'server alive'
```

**Attendu :** recv()==0 on the dropped socket maps to E_CLI_DISCONNECTED and the client is removed; the concurrent curl returns 200 within 3s and `server alive` prints. The abrupt mid-request disconnect neither crashes the process nor freezes the loop (REQ-04).

#### `CONN-15` · P0 · integration — Single multiplexer for ALL fds; no per-connection thread; no fork for static serving
**Réf :** REQ-05, REQ-17, REQ-03 · **Outil :** dtruss/strace + ps · **Auto :** non

**Pré :** Server on :8080 (io_multiplexer poll). Needs sudo (macOS dtruss requires SIP allowance); Linux strace works unprivileged on own process.

**Procédure :**

```sh
# --- syscall audit while issuing a plain static GET ---
# macOS:  sudo dtruss -f -t poll,select,kevent,fork,vfork -p $PID  &  then: curl -s http://127.0.0.1:8080/ >/dev/null
# Linux:  strace -f -e trace=poll,epoll_wait,select,kqueue,clone,fork,vfork -p $PID  &  then: curl -s http://127.0.0.1:8080/ >/dev/null
# --- thread count (must be single-threaded) ---
# macOS:  ps -M $PID | wc -l        (expect 1 data row)
# Linux:  ls /proc/$PID/task | wc -l  (expect 1)
# --- open a second concurrent connection during the trace to confirm the SAME poll call-site handles it and the listen fds ---
```

**Attendu :** Exactly one polling call-site (poll — the configured multiplexer) is invoked repeatedly and it is the only place fds become ready; listen sockets AND client sockets are all serviced by that single loop. A plain static GET produces NO clone/fork/vfork (fork appears only for CGI). Thread count is 1 (no per-connection thread/process). Violations → grade 0 per REQ-05/REQ-17.

#### `CONN-16` · P1 · integration — No recv/send without prior poll readiness; no errno-based control flow after I/O
**Réf :** REQ-07, REQ-08, REQ-09 · **Outil :** dtruss/strace + source · **Auto :** non

**Pré :** Server on :8080; trace as in CONN-15. Cross-references REQ-08 gap (ATcpSocket.cpp:90/108 read errno via strerror after every recv/send).

**Procédure :**

```sh
# Trace ordering: strace -f -e trace=poll,recvfrom,sendto,read,write -p $PID  during a mix of a static GET and a slow client.
# Assert in the trace: every recvfrom/sendto/read/write on a SOCKET or PIPE fd is immediately preceded (same fd) by that fd being reported ready by poll; regular-file read()/write() may occur without a preceding poll (exempt).
# Assert: after a recv/send returns -1 the server does NOT branch to immediately retry the same fd (no EAGAIN retry loop) — it waits for the next poll.
# Source cross-check: grep -n 'errno' libs/common/srcs/core/net/sockets/ATcpSocket.cpp
```

**Attendu :** Every socket/pipe recv/send is gated by a preceding poll-ready event; no recv/send occurs on an fd poll did not mark ready (REQ-07/09). No retry-on-EAGAIN loop. NOTE the confirmed REQ-08 smell: the recv/send wrappers call strerror(errno) after a -1 return and treat ANY -1 as a fatal disconnect — so an EINTR (or, since client fds are blocking, any transient error) is misclassified; source inspection should flag this even though behavior here is poll-gated.

#### `CONN-17` · P1 · robustness — Idle connected client must not pin the CPU (busy-spin from permanent POLLOUT interest)
**Réf :** REQ-05, REQ-12 · **Outil :** python3 + ps · **Auto :** oui

**Pré :** Server on :8080. TARGETS the busy-spin gap (ClientHandler.cpp:81 registers E_IN|E_OUT once and never toggles E_OUT off).

**Procédure :**

```sh
echo "idle (0 clients) CPU:"; ps -o %cpu= -p $PID; sleep 2
python3 -c 'import socket,time; s=socket.create_connection(("127.0.0.1",8080)); s.sendall(b"GET / HTTP/1.1\r\nHost: x\r\n\r\n"); s.recv(65536); time.sleep(20)' &
BG=$!; sleep 4
echo "with 1 idle keep-alive client CPU:"; ps -o %cpu= -p $PID
kill $BG 2>/dev/null
```

**Attendu :** CPU stays near 0% both with zero clients and with one idle (fully-served, silent) keep-alive client — poll should block for its 100ms timeout when nothing is actionable. SUSPECTED webserv failure: CPU jumps to ~90-100% once a client is connected, because the client socket is permanently registered for POLLOUT and a connected socket is almost always writable, so poll(100ms) returns immediately every iteration (efficiency/DoS surface; interferes with timeout pacing).

#### `CONN-18` · P1 · robustness — Graceful shutdown on SIGTERM/SIGINT: clean exit, sockets/CGI reaped, no orphans
**Réf :** REQ-11, REQ-12 · **Outil :** kill + pgrep · **Auto :** oui

**Pré :** Server on :8080; optionally a CGI request in flight. TARGETS the broken-shutdown gap (g_SignalStatus is in an anonymous namespace in a header and only SIGPIPE is handled; the run-loop condition is never triggered).

**Procédure :**

```sh
CHILDREN_BEFORE=$(pgrep -P $PID | wc -l)
kill -TERM $PID
sleep 1
if kill -0 $PID 2>/dev/null; then echo 'STILL ALIVE after SIGTERM (loop never exits)'; else echo 'process exited'; fi
echo "orphaned children now: $(pgrep -P $PID 2>/dev/null | wc -l)"
# port should free immediately for re-bind:
( ./webserv conf/test.conf & sleep 1; curl -s -o /dev/null -w 'rebind=%{http_code}\n' http://127.0.0.1:8080/ )
```

**Attendu :** Process exits promptly (well under 1s) with no leftover CGI child processes and the listening port frees for an immediate re-bind (`rebind=200`). SUSPECTED failure: because no SIGINT/SIGTERM handler sets g_SignalStatus (only SIGPIPE is installed), the run() while(!g_SignalStatus) loop never breaks — the process is terminated only by the signal's default action with NO cleanup (any in-flight CGI child may be orphaned; a lingering socket may briefly hold the port). Any 'STILL ALIVE' or orphaned-children output flags the gap.

#### `CONN-19` · P2 · robustness — select() FD_SETSIZE boundary: excess connections rejected without UB or crash
**Réf :** REQ-05, REQ-11 · **Outil :** python3 socket · **Auto :** oui

**Pré :** Server launched with a config setting io_multiplexer select. Exercises SelectEventIO::add throwing when an accepted fd >= FD_SETSIZE (1024).

**Procédure :**

```sh
python3 - <<'PY'
import socket
socks=[]
for i in range(1100):
    try:
        s=socket.create_connection(("127.0.0.1",8080)); s.sendall(b"GET / HTTP/1.1\r\nHost: x\r\n\r\n"); socks.append(s)
    except OSError as e:
        print("connect stopped at",i,e); break
print("held_open",len(socks))
PY
kill -0 $PID && echo 'server alive'
curl -s -o /dev/null -w 'recheck=%{http_code}\n' --max-time 3 http://127.0.0.1:8080/
```

**Attendu :** With select, once an accepted fd reaches FD_SETSIZE the server rejects/closes the excess connection via a thrown-and-caught error (no FD_SET out-of-bounds UB), continues serving existing clients, stays alive, and `recheck=200`. With poll there is no such cap — confirm graceful growth instead. Never a crash or silent corruption (REQ-05/REQ-11).

#### `CONN-20` · P2 · robustness — Signal-interrupted (EINTR) I/O must not drop an in-flight connection
**Réf :** REQ-08, REQ-04 · **Outil :** python3 + kill loop · **Auto :** oui

**Pré :** Server on :8080; /big.bin 50 MiB. Probes the REQ-08 risk that any recv/send==-1 (incl. EINTR) is thrown and treated as a disconnect. Flaky by nature (races the signal against a blocked syscall).

**Procédure :**

```sh
python3 - <<'PY' &
import socket,time
s=socket.create_connection(("127.0.0.1",8080)); s.settimeout(40)
s.sendall(b"GET /big.bin HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n")
b=b""
while True:
    try: d=s.recv(4096)
    except socket.timeout: break
    if not d: break
    b+=d; time.sleep(0.002)
print("received_bytes",len(b))
PY
CP=$!
for i in $(seq 1 3000); do kill -URG $PID 2>/dev/null; done   # SIGURG: no default terminate
wait $CP
```

**Attendu :** `received_bytes` == 52428800 — the transfer completes intact despite thousands of signals; an EINTR-interrupted recv/send is retried, not fatal. SUSPECTED failure: a short/`received_bytes` < file size (or the client sees an early EOF) because a signal interrupted a blocking recv/send and the wrapper threw, causing the server to classify the live connection as disconnected (REQ-08 violation).

---

### 6.7 Status Codes & Error Pages

_Every expected result below was empirically verified on 2026-07-20 against the compiled webserv binary running `configs/blucken.conf` on 127.0.0.1:8081 (roots at Webserv/www; listen 8080-8084; http-level `error_page 404 /404.html` and `error_page 500 502 503 504 /50x.html`; `/api` has `error_page 404 /api/not-found.html`; `/webdav` has 404+403 pages; `/static` allows GET HEAD only; `return` on /old-page(301) /legacy(302) /gone(410); client_max_body_size 10M on /cgi-bin, 2G elsewhere). Fixture byte sizes used in assertions: www/404.html=178, www/api/not-found.html=172, www/webdav/not-found.html=196; server-generated bodies observed 400=206, 405=286, 500=304, 301=213, 302=298, 410=475.

SHARED PRECONDITION for all cases: `./webserv configs/blucken.conf` running; wait ~2s after launch for it to bind all sockets (the ABNF grammar loader delays bind). Use 127.0.0.1 explicitly (Docker may hold *:8080; webserv also binds 127.0.0.1:8080-8084). On macOS use `nc -w1`; on the Linux evaluation box use `nc -q1`. Because a connected client socket is almost always writable, expect no hangs on well-formed requests.

Coverage maps every implemented code in the focus list: 200/201/204/301/302/304/400/403/404/405/408/411/413/414/500/501/505, plus custom-error-page-served (file), location-scoped custom page override, default generated fallback, Allow on 405, and byte-exact Content-Length on error bodies.

ADVERSARIAL DEFECTS SURFACED (each has a dedicated case, marked differential/robustness): (1) HTTP/1.0 returns 505 instead of being served 200 — version parser hard-codes "HTTP/1.1" (violates R-VERSION-MINOR). (2) HEAD on an error status returns a full message body (178 bytes on a HEAD/404) — RFC 9110 forbids a body on HEAD (framing/keep-alive desync). (3) Error responses never emit a Connection header and ignore `Connection: close` — the socket is left open, and on a persistent connection the errored response also mis-signals framing. (4) 408 is registered but never emitted; a partial/incomplete request gets an instant spurious 400 (headers split across TCP segments → 400 with no wait) or is processed with a truncated body, and slow-idle requests are silently reaped at keepalive_timeout with no 408 (differential vs nginx). (5) PUT to the DAV-configured routes (/uploads, /webdav, /app/uploads) returns 500 instead of 201/204 — the mandatory upload path is broken there; PUT only succeeds under `location /`. (6) 304 is unimplemented (conditional GET returns 200) and its dead code path advertises Content-Length: 1 with a 0-byte body. (7) Date header is emitted with a "UTC" suffix (strftime %Z) instead of the RFC-mandated "GMT". These are the highest-value findings for a 42 evaluator._

#### `SC-01` · P0 · integration — 200 OK — static GET returns exact bytes, Content-Type, and byte-exact Content-Length
**Réf :** REQ-20 / RFC9110 R-200, R-CONTENT-TYPE, R-FRAMING · **Outil :** nc/printf (or curl) · **Auto :** oui

**Pré :** www/test.txt exists (6 bytes, content 'hellow').

**Procédure :**

```sh
printf 'GET /test.txt HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** Status line 'HTTP/1.1 200 OK'; headers include 'Content-Type: text/plain' and 'Content-Length: 6'; body is exactly the 6 bytes 'hellow'; server closes the socket (Connection: close honored on the success path).

#### `SC-02` · P0 · integration — 201 Created — PUT a new file returns 201 with Content-Length: 0
**Réf :** REQ-19 / RFC9110 R-201, R-METHOD-PUT · **Outil :** nc/printf · **Auto :** oui

**Pré :** location / allows PUT (dav_methods PUT DELETE inherited); target /put_new.txt absent. Send NO Content-Type (defaults to application/octet-stream) to avoid the 415 MIME-gating gap.

**Procédure :**

```sh
printf 'PUT /put_new.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\nConnection: close\r\n\r\nhello' | nc -w1 127.0.0.1 8081 ; ls -l Webserv/www/put_new.txt
```

**Attendu :** 'HTTP/1.1 201 Created' with 'Content-Length: 0' and an empty body; file www/put_new.txt created containing 'hello'. (nginx also returns 201 + Location.)

#### `SC-03` · P0 · integration — 204 No Content — PUT overwrite and DELETE return 204 with an empty body and NO Content-Length
**Réf :** REQ-22 / RFC9110 R-204, R-METHOD-DELETE · **Outil :** nc/printf · **Auto :** oui

**Pré :** Run SC-02 first so /put_new.txt exists.

**Procédure :**

```sh
printf 'PUT /put_new.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\nConnection: close\r\n\r\nWORLD' | nc -w1 127.0.0.1 8081 ; printf 'DELETE /put_new.txt HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** Both responses are 'HTTP/1.1 204 No Content' with zero body bytes after the blank line and NO Content-Length header (verified: 204 omits Content-Length). After DELETE, GET /put_new.txt → 404.

#### `SC-04` · P0 · integration — 301 Moved Permanently — configured return with absolute Location
**Réf :** REQ-27 / RFC9110 R-301, R-LOCATION-3XX · **Outil :** nc/printf · **Auto :** oui

**Pré :** location = /old-page { return 301 http://example.com/new-page; }

**Procédure :**

```sh
printf 'GET /old-page HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** 'HTTP/1.1 301 Moved Permanently' with 'Location: http://example.com/new-page'; a generated HTML body of exactly 213 bytes with matching Content-Length: 213.

#### `SC-05` · P1 · differential-vs-nginx — 301 directory redirect — dir target without trailing slash → Location target+'/'
**Réf :** RFC9110 R-301 / ExecutionHandler dir-no-slash · **Outil :** nc/printf · **Auto :** oui

**Pré :** www/static/ is a directory served by location ^~ /static.

**Procédure :**

```sh
printf 'GET /static HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** 'HTTP/1.1 301 Moved Permanently' with 'Location: /static/' (relative path). nginx emits the same 301 to /static/ (nginx uses an absolute Location; webserv uses the relative form — both acceptable). Must NOT 200-list the directory without the slash.

#### `SC-06` · P1 · integration — 302 Found — configured temporary redirect with Location
**Réf :** REQ-27 / RFC9110 R-302, R-LOCATION-3XX · **Outil :** nc/printf · **Auto :** oui

**Pré :** location /legacy { return 302 /modern; }

**Procédure :**

```sh
printf 'GET /legacy HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** 'HTTP/1.1 302 Found' with 'Location: /modern'; generated body 298 bytes with Content-Length: 298.

#### `SC-07` · P0 · unit — 400 Bad Request — HTTP/1.1 request missing Host header
**Réf :** RFC9112 R10 / RFC9110 R-400, R-HOST · **Outil :** nc/printf · **Auto :** oui

**Pré :** No Host header sent on an HTTP/1.1 request.

**Procédure :**

```sh
printf 'GET / HTTP/1.1\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** 'HTTP/1.1 400 Bad Request'; generated HTML body of 206 bytes with Content-Length: 206. Server stays alive (verify a follow-up GET /test.txt still 200s).

#### `SC-08` · P1 · robustness — 400 Bad Request — duplicate Host and malformed request-line variants
**Réf :** RFC9112 R01,R08,R11,R27,R29 / RFC9110 R-400 · **Outil :** nc/printf · **Auto :** oui

**Pré :** Run each line separately; confirm the process survives all of them.

**Procédure :**

```sh
printf 'GET / HTTP/1.1\r\nHost: a\r\nHost: b\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081 ; printf 'GET  /  HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081 ; printf 'GET / HTTP/1.1\r\nHost: x\r\nX-Test : v\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081 ; printf 'GET / HTTP/1.11\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** All four return 'HTTP/1.1 400 Bad Request' (duplicate Host; two-space request-line; whitespace before colon; malformed version token HTTP/1.11). Process remains alive after each.

#### `SC-09` · P1 · integration — 403 Forbidden — unreadable file (EACCES) and POST to a directory root
**Réf :** RFC9110 R-403 / REQ-29 · **Outil :** nc/printf + chmod · **Auto :** oui

**Pré :** No error_page 403 is configured at http/server/location `/`, so a generated 403 page is expected.

**Procédure :**

```sh
printf 'secret' > Webserv/www/sec403.txt; chmod 000 Webserv/www/sec403.txt; printf 'GET /sec403.txt HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081; chmod 644 Webserv/www/sec403.txt; rm -f Webserv/www/sec403.txt; printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\nConnection: close\r\n\r\nabc' | nc -w1 127.0.0.1 8081
```

**Attendu :** Both requests return 'HTTP/1.1 403 Forbidden' with a server-generated HTML body and a Content-Length equal to that body's byte count (verified 403 on both the chmod-000 GET and the POST-to-dir-root path).

#### `SC-10` · P0 · integration — 404 + custom error_page served (http-level) — byte-exact custom file body
**Réf :** REQ-16 / RFC9110 R-404, R-FRAMING · **Outil :** nc/printf · **Auto :** oui

**Pré :** http { error_page 404 /404.html; } and www/404.html is 178 bytes.

**Procédure :**

```sh
printf 'GET /no-such-resource HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** 'HTTP/1.1 404 Not Found'; Content-Type: text/html; Content-Length: 178; body is byte-identical to www/404.html ('<h1>404 Not Found</h1>...'). This proves REQ-16 custom error_page serving with exact file bytes.

#### `SC-11` · P1 · integration — 404 — location-scoped custom error_page overrides the http-level page
**Réf :** REQ-16 / RFC9110 R-404 · **Outil :** nc/printf · **Auto :** oui

**Pré :** location /api { error_page 404 /api/not-found.html; } and www/api/not-found.html is 172 bytes; contrast with the 178-byte http-level page from SC-10.

**Procédure :**

```sh
printf 'GET /api/does-not-exist HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** 'HTTP/1.1 404 Not Found' with Content-Length: 172 and body byte-identical to www/api/not-found.html ('The requested API endpoint does not exist.'), NOT the 178-byte root page — confirming location-scope error_page selection.

#### `SC-12` · P1 · integration — Default (generated) error page fallback when no error_page is configured for the status
**Réf :** REQ-15 / RFC9110 R-405 · **Outil :** nc/printf · **Auto :** oui

**Pré :** No error_page is configured for 405 anywhere; the server must synthesize a page (REQ-15).

**Procédure :**

```sh
printf 'DELETE /static/style.css HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** 'HTTP/1.1 405 Method Not Allowed' with a non-empty server-generated HTML body (observed 286 bytes) and Content-Type: text/html — a valid built-in default page, not an empty body or reset. (Also see SC-13 for configured-but-unreadable fallback.)

#### `SC-13` · P2 · integration — Fallback to generated page when a configured error_page FILE is missing/unreadable
**Réf :** REQ-15, REQ-16 / ResponseHandler.cpp:288-333 fallback · **Outil :** curl + temp config · **Auto :** oui

**Pré :** Author a minimal config test_missing_ep.conf: http{ root <abs>/Webserv/www; error_page 404 /this-file-absent.html; server{ listen 127.0.0.1:8091; location / { allowed_methods GET; } } }. Launch it on 8091.

**Procédure :**

```sh
curl -sv http://127.0.0.1:8091/nope 2>&1 | sed -n '1,20p'
```

**Attendu :** 404 with a non-empty server-generated HTML body (the configured /this-file-absent.html cannot be opened, so ResponseHandler degrades to the built-in page) and a Content-Length matching that generated body. Must not send an empty/truncated body or crash.

#### `SC-14` · P0 · integration — 405 Method Not Allowed — correct Allow header lists the route's permitted methods
**Réf :** REQ-26 / RFC9110 R-405, R-ALLOW · **Outil :** nc/printf · **Auto :** oui

**Pré :** location ^~ /static { allowed_methods GET HEAD; }

**Procédure :**

```sh
printf 'DELETE /static/style.css HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081 ; printf 'PUT /static/style.css HTTP/1.1\r\nHost: x\r\nContent-Length: 1\r\nConnection: close\r\n\r\nx' | nc -w1 127.0.0.1 8081
```

**Attendu :** Both return 'HTTP/1.1 405 Method Not Allowed' with header 'Allow: GET, HEAD' (verified exact value) and a generated body with correct Content-Length (286). The Allow header MUST be present on every 405. Cross-check: on /api (GET POST) a DELETE returns Allow listing GET and POST.

#### `SC-15` · P1 · differential-vs-nginx — 408 never emitted — incomplete request gets instant 400 or premature processing, not 408
**Réf :** RFC9112 R30 / RFC9110 R-408 (registered, never emitted) / suspected gap #1 · **Outil :** python3 (raw socket, timed) · **Auto :** oui

**Pré :** Demonstrates that 408 is registered but unreachable, and that headers split across segments are rejected without waiting.

**Procédure :**

```sh
python3 - <<'PY'
import socket,time
s=socket.socket();s.settimeout(4);s.connect(('127.0.0.1',8081))
s.sendall(b'GET / HTTP/1.1\r\nHost: x\r\n')   # header section NOT terminated
t=time.time()
try: print('%.2fs'%(time.time()-t), s.recv(80))
except socket.timeout: print('no response (waiting)')
PY
```

**Attendu :** webserv responds 'HTTP/1.1 400 Bad Request' almost immediately (~0.1s) instead of waiting for the rest of the headers — a spurious 400 on TCP-segmented headers, and NO 408 is ever produced. (nginx waits and, on client_header_timeout, returns 408.) Companion check: a POST with 'Content-Length: 100' but only 10 body bytes is answered immediately (server does not block on the missing body and never sends 408). Flag both as defects; server must at least not hang/crash (it does not).

#### `SC-16` · P0 · unit — 411 Length Required — body-method with neither Content-Length nor Transfer-Encoding
**Réf :** RFC9110 R-411 / RFC9112 R24 · **Outil :** nc/printf · **Auto :** oui

**Pré :** POST/PUT with no framing headers.

**Procédure :**

```sh
printf 'POST /uploads HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** 'HTTP/1.1 411 Length Required' with a generated HTML body and matching Content-Length (verified 411 on this exact request).

#### `SC-17` · P0 · integration — 413 Content Too Large — POST body exceeding client_max_body_size
**Réf :** REQ-25 / RFC9110 R-413 · **Outil :** python3 / dd · **Auto :** oui

**Pré :** location /cgi-bin has client_max_body_size 10M; send 11 MB.

**Procédure :**

```sh
head -c 11000000 /dev/zero | tr '\0' A > /tmp/big.bin; python3 - <<'PY'
import socket
b=open('/tmp/big.bin','rb').read()
s=socket.socket();s.settimeout(6);s.connect(('127.0.0.1',8081))
s.sendall(b'POST /cgi-bin/x HTTP/1.1\r\nHost: x\r\nContent-Type: application/octet-stream\r\nContent-Length: %d\r\nConnection: close\r\n\r\n'%len(b))
try: s.sendall(b)
except Exception: pass
print(s.recv(120).split(b'\r\n')[0])
PY
```

**Attendu :** 'HTTP/1.1 413 Payload Too Large' (verified). The server rejects before storing/processing the body (it may close the receive side early → a broken-pipe on the client's send, which is acceptable). A body within the limit (e.g. 1 MB) to the same route is accepted normally.

#### `SC-18` · P0 · unit — 414 URI Too Long — request-target beyond URI_MAX_LENGTH (2048)
**Réf :** RFC9110 R-414 / RFC9112 R02 · **Outil :** python3 (raw socket) · **Auto :** oui

**Pré :** URI_MAX_LENGTH=2048; send ~2100-byte target.

**Procédure :**

```sh
python3 - <<'PY'
import socket
s=socket.socket();s.settimeout(4);s.connect(('127.0.0.1',8081))
s.sendall(b'GET /'+b'a'*2100+b' HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n')
print(s.recv(120).split(b'\r\n')[0])
PY
```

**Attendu :** 'HTTP/1.1 414 URI Too Long' (verified). Companion boundary check: a request-line of ~7900 octets (RFC-recommended 8000 support) should NOT be 414 in principle, but note webserv's 2048 cap will 414 it — record as a stricter-than-recommended limit (RFC9112 R04 SHOULD).

#### `SC-19` · P0 · differential-vs-nginx — 500 Internal Server Error — PUT to a DAV-configured route (broken) yields 500 with correct framing
**Réf :** RFC9110 R-500 / REQ-19, REQ-31 / ExecutionHandler openFile · **Outil :** nc/printf · **Auto :** oui

**Pré :** location /uploads, /webdav, /app/uploads all declare dav_methods+dav_put_path; PUT under `location /` works (SC-02) but these routes fail.

**Procédure :**

```sh
printf 'PUT /uploads/putz.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\nConnection: close\r\n\r\nabc' | nc -w1 127.0.0.1 8081 ; printf 'PUT /webdav/putz.txt HTTP/1.1\r\nHost: x\r\nContent-Length: 3\r\nConnection: close\r\n\r\nabc' | nc -w1 127.0.0.1 8081
```

**Attendu :** Both return 'HTTP/1.1 500 Internal Server Error' with a generated HTML body of 304 bytes and Content-Length: 304, connection correctly framed, server stays alive. DEFECT: nginx creates the file and returns 201/204; here the mandatory upload path is broken on every DAV route (openFile 500s). This case doubles as the 500 status/framing verification (R-500) and a broken-upload finding (REQ-19/REQ-31).

#### `SC-20` · P0 · unit — 501 Not Implemented — unknown method and lowercase method token
**Réf :** RFC9110 R-501 / RFC9112 R03 · **Outil :** nc/printf · **Auto :** oui

**Pré :** strToMethod is exact-uppercase-match; anything else → 501.

**Procédure :**

```sh
printf 'BREW / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081 ; printf 'get / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081 ; printf 'PATCH / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** All three return 'HTTP/1.1 501 Not Implemented' (verified BREW and lowercase 'get'; PATCH/OPTIONS/TRACE likewise). This is distinct from 405 (recognized-but-disallowed method on a route).

#### `SC-21` · P0 · differential-vs-nginx — 505 HTTP Version Not Supported — HTTP/2.0 rejected; HTTP/1.0 WRONGLY rejected
**Réf :** RFC9110 R-505, R-VERSION-MINOR / RFC9112 R27 · **Outil :** nc/printf · **Auto :** oui

**Pré :** Version parser hard-codes the literal 'HTTP/1.1'.

**Procédure :**

```sh
printf 'GET / HTTP/2.0\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081 ; printf 'GET /test.txt HTTP/1.0\r\nHost: x\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** HTTP/2.0 → 'HTTP/1.1 505 HTTP Version Not Supported' (correct). HTTP/1.0 → 505 as well (DEFECT): major version 1 is supported, so per R-VERSION-MINOR the request MUST be processed (nginx serves it 200 with the body). Flag HTTP/1.0→505 as non-conformant. Server stays alive for both.

#### `SC-22` · P1 · robustness — HEAD on an error status wrongly returns a message body (framing defect)
**Réf :** RFC9110 R-METHOD-HEAD / ResponseHandler.cpp:279-348 · **Outil :** python3 (raw socket, byte count) · **Auto :** oui

**Pré :** buildErrorResponse never checks the request method.

**Procédure :**

```sh
python3 - <<'PY'
import socket
s=socket.socket();s.settimeout(4);s.connect(('127.0.0.1',8081))
s.sendall(b'HEAD /no-such HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n')
d=b''
try:
  while True:
    c=s.recv(4096)
    if not c: break
    d+=c
except socket.timeout: pass
h,_,b=d.partition(b'\r\n\r\n')
print('status',h.split(b'\r\n')[0],'| body_bytes',len(b))
PY
```

**Attendu :** Status 404 with 'Content-Length: 178' AND 178 body bytes present after the blank line (verified). RFC 9110 forbids message content on any HEAD response — the body MUST be empty. This mis-frames the next response on a persistent connection. DEFECT (nginx sends headers only, zero body).

#### `SC-23` · P1 · robustness — Error responses ignore Connection: close and omit the Connection header (leak/desync)
**Réf :** RFC9112 R25 / ResponseHandler buildErrorResponse omits Connection · **Outil :** python3 (raw socket, timed) · **Auto :** oui

**Pré :** Compare a success response (honors close) with an error response (does not).

**Procédure :**

```sh
python3 - <<'PY'
import socket,time
def probe(req):
  s=socket.socket();s.settimeout(2.5);s.connect(('127.0.0.1',8081));s.sendall(req)
  d=b''
  try:
    while True:
      c=s.recv(4096)
      if not c: break
      d+=c
  except socket.timeout: return 'LEFT-OPEN', d.split(b'\r\n')[0]
  return 'CLOSED', d.split(b'\r\n')[0]
print('success:',probe(b'GET /test.txt HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n'))
print('error  :',probe(b'GET /no-such HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n'))
PY
```

**Attendu :** Success case → 'CLOSED' (server honors Connection: close). Error case → 'LEFT-OPEN' (verified: the 404 response carries no Connection header and the socket is NOT closed despite the client's Connection: close), so the client blocks until its own timeout. DEFECT: error responses never signal keep-alive/close and don't honor close, risking hung/leaked connections.

#### `SC-24` · P2 · differential-vs-nginx — 304 Not Modified — conditional GET unimplemented (returns 200); latent CL:1-with-empty-body bug
**Réf :** RFC9110 R-304 / ResponseHandler.cpp:280-285 · **Outil :** nc/printf · **Auto :** oui

**Pré :** www/test.txt exists; send a conditional request whose precondition would let a conformant server answer 304.

**Procédure :**

```sh
printf 'GET /test.txt HTTP/1.1\r\nHost: x\r\nIf-Modified-Since: Thu, 01 Jan 2099 00:00:00 GMT\r\nConnection: close\r\n\r\n' | nc -w1 127.0.0.1 8081
```

**Attendu :** webserv returns 'HTTP/1.1 200 OK' with the full body (verified: conditional requests are not implemented; If-Modified-Since/If-None-Match are ignored). nginx returns '304 Not Modified' with no body. Record as differential. NOTE: the code's dead 304 path advertises 'Content-Length: 1' with a 0-byte body — if ever reached it would desync framing; ensure no throw site can emit 304.

#### `SC-25` · P1 · integration — Byte-exact Content-Length on every error body (custom file vs generated)
**Réf :** RFC9110 R-FRAMING / focus: correct Content-Length on error bodies · **Outil :** python3 (raw socket, byte count) · **Auto :** oui

**Pré :** Cross-checks Content-Length == actual body byte count across custom-file and generated error pages.

**Procédure :**

```sh
python3 - <<'PY'
import socket,re
def cl_vs_body(req):
  s=socket.socket();s.settimeout(3);s.connect(('127.0.0.1',8081));s.sendall(req)
  d=b''
  try:
    while True:
      c=s.recv(4096)
      if not c: break
      d+=c
  except socket.timeout: pass
  h,_,b=d.partition(b'\r\n\r\n')
  m=re.search(rb'Content-Length: (\d+)',h)
  return d.split(b'\r\n')[0], (int(m.group(1)) if m else None), len(b)
for req in [b'GET /no-such HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n',b'GET / HTTP/1.1\r\nConnection: close\r\n\r\n',b'BREW / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n']:
  print(cl_vs_body(req))
PY
```

**Attendu :** For each error the advertised Content-Length equals the actual body bytes: 404 custom file → (404,178,178); 400 generated → (400,206,206); 501 generated → length matches its body. No error advertises a length that differs from the bytes sent (contrast the 304 dead-path and HEAD-body defects). This is the framing-correctness gate for error bodies.

#### `SC-26` · P2 · differential-vs-nginx — Date header format — must end in 'GMT' (currently 'UTC')
**Réf :** RFC9110 R-DATE / ResponseHandler.cpp:385 · **Outil :** curl · **Auto :** oui

**Pré :** strftime('%Z') over gmtime yields the platform zone name.

**Procédure :**

```sh
curl -sD - -o /dev/null http://127.0.0.1:8081/test.txt | grep -i '^Date:'
```

**Attendu :** webserv emits e.g. 'Date: Mon, 20 Jul 2026 21:41:05 UTC' (verified). RFC 9110 IMF-fixdate REQUIRES the literal 'GMT' suffix. DEFECT: value ends in 'UTC' (platform %Z), so the Date header is non-conformant; a strict client/parser may reject it. All 2xx/3xx/4xx must carry a GMT-terminated Date.

---

### 6.8 Robustness, Stress, Leaks & Evaluation-Day Checklist

_All tests assume the eval config: from Webserv/, launch `./webserv configs/blucken.conf` (poll multiplexer, ports 8080-8084, root=Webserv/www, client_max_body_size 2G, keepalive 60s), capture PID with `PID=$(pgrep -f 'webserv.*blucken')`. macOS host => use `leaks <pid>`; Linux/Docker => use `valgrind`. Tests are ordered to weaponize the specific defects surfaced by the code analysis: (gap#1) parseHeaders throws 400 the instant CRLFCRLF is not yet in the buffer -> any TCP-segmented request breaks (STRESS-10); accepted client sockets are BLOCKING, so a single non-reading client stalls the whole single-threaded event loop inside send() -> head-of-line DoS (STRESS-08); the only timeout is an idle keep-alive timer refreshed on every inbound byte, so a body-drip client holds an fd forever with no 408 (STRESS-09); every cookie-less request mints a new 7-day session with no store cap -> unbounded RSS growth under siege (STRESS-02); the CGI child inherits every server fd (no FD_CLOEXEC) (STRESS-14); g_SignalStatus lives in an anonymous namespace and no SIGINT/SIGTERM handler exists, so there is no graceful shutdown and no destructor teardown at exit -> a clean valgrind at-exit report is impossible (STRESS-04/05). CRITICAL EVAL GOTCHA: tester/cgi_tester/ubuntu_* in Webserv/tester/ are ALL Linux x86-64 ELF (verified via file(1)); they will not execute on the macOS eval box and must be run inside the 42 Linux VM/Docker (STRESS-12/13). P0 = crash/hang/leak/mandatory-feature; the pipeline ends with STRESS-18, a mandatory liveness gate that re-asserts the same PID is alive and serving after every adversarial test — a crash there is an automatic grade 0 per REQ-11._

#### `STRESS-01` · P0 · stress — siege -b sustained availability must stay ~100% with zero failed transactions
**Réf :** REQ-12 · **Outil :** siege · **Auto :** oui

**Pré :** Server running on :8080 with configs/blucken.conf; siege installed; www/index.html present.

**Procédure :**

```sh
siege -b -c 25 -t 30S 'http://localhost:8080/' ; then a mixed run: siege -b -c 25 -t 30S -f <(printf 'http://localhost:8080/\nhttp://localhost:8080/static/\nhttp://localhost:8080/hugefile.txt\nhttp://localhost:8080/does-not-exist\n')
```

**Attendu :** siege report shows Availability: 100.00 % (>=99.5% acceptable) and Failed transactions: 0. No 'socket: connection reset' / 'read error' lines. Longest transaction stays bounded (< 1s locally). Server PID unchanged after the run.

#### `STRESS-02` · P0 · stress — RSS must plateau under sustained load (weaponizes unbounded session-store growth)
**Réf :** REQ-11 · **Outil :** siege + ps · **Auto :** oui

**Pré :** Server running on :8080. siege does NOT echo Set-Cookie, so every request creates a fresh 7-day session (SessionStore has no size cap; purge only removes already-expired entries every 60s).

**Procédure :**

```sh
PID=$(pgrep -f 'webserv.*blucken'); ( for i in $(seq 1 120); do ps -o rss= -p $PID; sleep 5; done ) > /tmp/rss.log & siege -b -c 40 -t 600S 'http://localhost:8080/'; echo 'baseline vs final RSS:'; head -1 /tmp/rss.log; tail -1 /tmp/rss.log
```

**Attendu :** RSS rises during warm-up then PLATEAUS; final RSS within ~1.5x of the early-steady value. A monotonic climb over 10 min (e.g. 5 MB -> 200 MB+) that never plateaus confirms the session-store memory-exhaustion DoS (one Session accumulated per cookie-less request, TTL 7 days). Server must not be OOM-killed; PID unchanged.

#### `STRESS-03` · P0 · robustness — No file-descriptor leak across connect/disconnect churn (lsof over time)
**Réf :** REQ-04 · **Outil :** lsof + curl · **Auto :** oui

**Pré :** Server running on :8080.

**Procédure :**

```sh
PID=$(pgrep -f 'webserv.*blucken'); B=$(lsof -p $PID 2>/dev/null | wc -l); for i in $(seq 1 5000); do curl -s -o /dev/null http://localhost:8080/ ; done; sleep 3; A=$(lsof -p $PID 2>/dev/null | wc -l); echo "baseline=$B after=$A"; # half-open churn: for i in $(seq 1 2000); do (exec 3<>/dev/tcp/localhost/8080; printf 'GET / HTTP/1.1\r\n' >&3; exec 3>&-); done; sleep 3; lsof -p $PID | wc -l
```

**Attendu :** Open-fd count after the churn returns to within a few of baseline (delta <= ~5). A steadily rising fd count is a descriptor leak. After the abrupt half-open closes, recv()==0/-1 must remove each client and close its fd (no CLOSE_WAIT pile-up in lsof).

#### `STRESS-04` · P0 · robustness — Leak checker clean: macOS leaks(1) / Linux valgrind report no lost blocks after load
**Réf :** REQ-11 · **Outil :** leaks (macOS) / valgrind (Linux) · **Auto :** oui

**Pré :** Server running. On macOS run leaks against the LIVE pid (reports unreachable blocks; works without shutdown). On Linux run the server under valgrind.

**Procédure :**

```sh
macOS: PID=$(pgrep -f 'webserv.*blucken'); siege -b -c 20 -t 30S http://localhost:8080/ >/dev/null 2>&1; leaks $PID ;  Linux/Docker: valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes --error-exitcode=42 ./webserv configs/blucken_linux.conf & sleep 2; siege -b -c 20 -t 30S http://localhost:8080/ >/dev/null 2>&1; kill -INT %1; wait
```

**Attendu :** leaks(1): '0 leaks for 0 total leaked bytes'. valgrind: 'definitely lost: 0 bytes' AND 'indirectly lost: 0 bytes'; --track-fds shows no leaked descriptors beyond 0,1,2. NOTE: valgrind 'still reachable' will be large because there is no graceful shutdown (see STRESS-05) — that is a separate finding, but definitely/indirectly-lost must be zero.

#### `STRESS-05` · P0 · robustness — Graceful shutdown on SIGINT/SIGTERM (targets broken g_SignalStatus / no destructor teardown)
**Réf :** REQ-11 · **Outil :** kill + valgrind/leaks · **Auto :** oui

**Pré :** Code analysis: g_SignalStatus is defined in an anonymous namespace in a header (per-TU copy) and no SIGINT/SIGTERM handler is installed; only SIGPIPE is caught. The run loop `while(!g_SignalStatus)` can therefore never observe a set flag.

**Procédure :**

```sh
PID=$(pgrep -f 'webserv.*blucken'); kill -INT $PID; sleep 1; ps -p $PID >/dev/null && echo 'STILL ALIVE (ignored SIGINT?)' || echo 'exited'; # under valgrind observe the leak summary printed at termination after kill -INT
```

**Attendu :** The process should exit cleanly and run destructors (sockets closed, CGI children reaped, no zombies via `ps -o stat | grep Z`). Because graceful shutdown is unimplemented, expect the process to die by the kernel's default SIGINT action with NO cleanup: valgrind reports all allocations as 'still reachable' and any in-flight CGI child may be orphaned. Flag as a defect: evaluators expecting Ctrl-C to trigger clean teardown/leak-free at-exit will not get it.

#### `STRESS-06` · P0 · robustness — Never crash on garbage / binary / TLS-handshake input
**Réf :** REQ-11 · **Outil :** nc + head + printf · **Auto :** oui

**Pré :** Server running on :8080.

**Procédure :**

```sh
head -c 200000 /dev/urandom | nc -w2 localhost 8080 | head -c 200; printf '\x16\x03\x01\x00\xdc\x01\x00\x00\xd8\x03\x03RANDOM_TLS_CLIENT_HELLO_BYTES' | nc -w2 localhost 8080; printf '\x00\x00\x00\x00\r\n\r\n' | nc -w2 localhost 8080; yes 'AAAA' | head -c 5000000 | nc -w2 localhost 8080; PID=$(pgrep -f 'webserv.*blucken'); ps -p $PID >/dev/null && echo ALIVE || echo DEAD; curl -s -o /dev/null -w '%{http_code}\n' http://localhost:8080/
```

**Attendu :** For each garbage input the server responds 400 (Bad Request) or closes the connection; it MUST NOT segfault, abort, or hang. PID unchanged after all four probes; final curl returns 200. A crash here = grade 0.

#### `STRESS-07` · P0 · robustness — Resource exhaustion: running out of file descriptors (EMFILE) must not crash
**Réf :** REQ-11 · **Outil :** ulimit + siege/curl · **Auto :** oui

**Pré :** Start a dedicated instance under a low fd limit so accept() hits EMFILE.

**Procédure :**

```sh
( ulimit -n 64; ./webserv configs/blucken.conf > /tmp/lowfd.log 2>&1 & echo $! > /tmp/lowfd.pid ); sleep 1; siege -b -c 200 -t 20S http://localhost:8080/ ; PID=$(cat /tmp/lowfd.pid); ps -p $PID >/dev/null && echo ALIVE || echo DEAD; sleep 2; curl -s -o /dev/null -w 'recovered:%{http_code}\n' http://localhost:8080/
```

**Attendu :** accept()/socket() returning -1 with EMFILE/ENFILE is handled without crashing: excess clients get connection-refused or are dropped, but the process stays ALIVE and keeps serving the fds it has. After load subsides the recovery curl returns 200. No unhandled exception abort, no infinite error-spin in /tmp/lowfd.log.

#### `STRESS-08` · P0 · differential-vs-nginx — Slow non-reading client must NOT stall other clients (blocking-socket head-of-line DoS)
**Réf :** REQ-05 · **Outil :** python3 + curl · **Auto :** oui

**Pré :** Accepted client sockets are never set O_NONBLOCK (blocking) and sendData() send()s the whole buffer; a peer that stops reading can block the single event loop inside send(). hugefile.txt (2.8 MB) forces the socket buffer to fill.

**Procédure :**

```sh
python3 -c "import socket,time; s=socket.socket(); s.setsockopt(socket.SOL_SOCKET,socket.SO_RCVBUF,512); s.connect(('127.0.0.1',8080)); s.send(b'GET /hugefile.txt HTTP/1.1\r\nHost: x\r\n\r\n'); time.sleep(60)" & sleep 1; time curl -s -o /dev/null -w 'other-client:%{http_code} t=%{time_total}\n' --max-time 10 http://localhost:8080/
```

**Attendu :** nginx serves the second (fast) client immediately: 200 in well under 1s while the stalled downloader is parked. webserv is EXPECTED TO FAIL here: the second curl hangs and hits --max-time 10 (exit 28) because the event loop is blocked in a blocking send() to the stalled reader. This is a direct violation of the non-blocking-at-all-times requirement and a single-slow-client DoS.

#### `STRESS-09` · P0 · robustness — Incomplete/dripped request body must not hang the connection forever (no request-read timeout)
**Réf :** REQ-10 · **Outil :** python3 + lsof · **Auto :** oui

**Pré :** The only timeout is the idle keep-alive timer, and it is refreshed on EVERY inbound byte (ClientHandler setLastActivityTime on each E_IN). A client that drips body bytes slower than keepalive_timeout keeps the timer alive indefinitely; no 408 path exists.

**Procédure :**

```sh
printf 'POST /uploads/slow.bin HTTP/1.1\r\nHost: x\r\nContent-Length: 100000\r\n\r\nshort'; # sent then held: python3 -c "import socket,time; s=socket.socket(); s.connect(('127.0.0.1',8080)); s.send(b'POST /uploads/slow.bin HTTP/1.1\r\nHost: x\r\nContent-Length: 100000\r\n\r\n');\nwhile True:\n time.sleep(30); s.send(b'A')" & CLIENTPID=$!; PID=$(pgrep -f 'webserv.*blucken'); sleep 150; lsof -p $PID | grep -c ':http\|8080'; kill $CLIENTPID
```

**Attendu :** RFC/subject require a bounded read: the server should return 408 (Request Timeout) or close within a bounded window and never block other clients. EXPECTED FAILURE: the dripping connection is held open indefinitely (fd still present in lsof after 150s, no response, no 408) because last-activity is refreshed by each drip byte — a slow-request resource-hold. Must at minimum never crash and never freeze concurrent clients (verify a parallel curl / still succeeds).

#### `STRESS-10` · P0 · differential-vs-nginx — TCP-segmented (dripped) valid request must be reassembled, not answered 400 (gap #1)
**Réf :** REQ-04 · **Outil :** nc + printf · **Auto :** oui

**Pré :** RequestHandler::parseHeaders throws HTTPError(400) whenever CRLFCRLF is not yet in the buffer, and processHTTPCycle calls it on any non-empty buffer — so a header section split across recv() calls is rejected before completion.

**Procédure :**

```sh
( printf 'GET / HTTP/1.1\r\n'; sleep 1; printf 'Host: x\r\n'; sleep 1; printf '\r\n' ) | nc -w4 localhost 8080 | head -1 ; echo '--- single-write control ---'; printf 'GET / HTTP/1.1\r\nHost: x\r\n\r\n' | nc -w2 localhost 8080 | head -1
```

**Attendu :** nginx returns '200 OK' (or 404) for both — the dripped headers are buffered until CRLFCRLF. EXPECTED FAILURE on webserv: the dripped case returns 'HTTP/1.1 400 Bad Request' on the first segment while the single-write control returns 200. This spurious 400 breaks any client that segments the request (slow links, Nagle, browsers under load) and undermines siege reliability. Server must not crash regardless.

#### `STRESS-11` · P1 · robustness — Abrupt client disconnect mid-response leaves other clients unaffected
**Réf :** REQ-04 · **Outil :** curl + bash · **Auto :** oui

**Pré :** Server running; www/hugefile.txt present.

**Procédure :**

```sh
curl -s --limit-rate 20k http://localhost:8080/hugefile.txt -o /dev/null & C=$!; sleep 0.3; kill -9 $C; for i in 1 2 3; do curl -s -o /dev/null -w 'live:%{http_code}\n' --max-time 5 http://localhost:8080/; done; PID=$(pgrep -f 'webserv.*blucken'); ps -p $PID >/dev/null && echo ALIVE || echo DEAD
```

**Attendu :** Killing the downloader mid-transfer triggers recv==0 / EPIPE on send (SIGPIPE ignored) and the client is cleaned up; the three follow-up curls each return 200 within the deadline; PID unchanged; no fd/CLOSE_WAIT leak (cross-check with STRESS-03).

#### `STRESS-12` · P0 · integration — Official 42 ./tester passes (GET/POST/DELETE, status codes, client_max_body_size, multi-request)
**Réf :** REQ-38 · **Outil :** tester (42 grader binary) · **Auto :** oui

**Pré :** CRITICAL: Webserv/tester/tester and ubuntu_tester are Linux x86-64 ELF (verified via file(1)) — they CANNOT run on the macOS eval host natively. Run inside the 42 Linux VM/Docker with configs/blucken_linux.conf, or the macOS box must provide a native tester.

**Procédure :**

```sh
# Linux VM/Docker, from Webserv/: ./webserv configs/blucken_linux.conf & sleep 1; cd tester && ./ubuntu_tester http://localhost:8080 ; # (consult ./ubuntu_tester --help for exact args; some builds read a hostname:port). Re-run 3x to check stability.
```

**Attendu :** Every tester assertion reports OK/PASS (status codes, 413 on client_max_body_size overflow, unknown-method handling, repeated/multi requests on one connection). No failed cases, no server crash, exit code 0. Running the wrong-arch binary (e.g. on macOS) silently invalidates the result — must be run on Linux.

#### `STRESS-13` · P0 · integration — Official cgi_tester passes end-to-end CGI
**Réf :** REQ-37 · **Outil :** cgi_tester (42 grader binary) · **Auto :** oui

**Pré :** Webserv/tester/cgi_tester and ubuntu_cgi_tester are Linux ELF — run in the Linux VM/Docker. Configure a CGI extension in the running config to invoke the grader's expected CGI, or point the tester at the server per its usage.

**Procédure :**

```sh
# Linux VM/Docker: ./webserv configs/blucken_linux.conf & sleep 1; cd tester && ./ubuntu_cgi_tester http://localhost:8080 ; # verify body passthrough + response assembly reported OK
```

**Attendu :** cgi_tester reports all CGI checks passing: request body correctly piped to CGI stdin, CGI stdout parsed into a well-framed HTTP response, correct status. No 502/500 on the grader's valid script, no truncation, no hang. NOTE the code-analysis stdout-truncation risk (single 8KB read then reap sets EOF): if the grader posts a body >8KB and the CGI echoes it and exits promptly, watch for a truncated body / 502.

#### `STRESS-14` · P1 · robustness — CGI child must NOT inherit server file descriptors (missing FD_CLOEXEC)
**Réf :** REQ-17 · **Outil :** curl + custom CGI + lsof · **Auto :** oui

**Pré :** Fork child only dup2/resets the 4 pipe fds and sets no FD_CLOEXEC; listen sockets and other clients' sockets remain open in the CGI process.

**Procédure :**

```sh
cat > www/cgi-bin/fds.py <<'EOF'
#!/usr/bin/env python3
import os
print('Content-Type: text/plain\r\n\r\n', end='')
try:
    fds=sorted(int(x) for x in os.listdir('/proc/self/fd'))
except FileNotFoundError:
    fds='no /proc (use lsof on child)'
print('OPEN_FDS', fds)
EOF
chmod +x www/cgi-bin/fds.py; curl -s http://localhost:8080/cgi-bin/fds.py
```

**Attendu :** The CGI should see only stdin/stdout/stderr (0,1,2) plus at most the transient listdir fd. EXPECTED FAILURE: extra inherited fds (the listen sockets on 8080-8084 and any concurrent client sockets) appear, proving descriptors leak into every CGI — a malicious/buggy script could read/write server sockets and it keeps ports bound. On macOS (no /proc) instead: fire a long CGI and run `lsof -p <cgi_child_pid>` to confirm inherited server sockets.

#### `STRESS-15` · P1 · robustness — Raw telnet session: hand-typed request + keep-alive reuse
**Réf :** REQ-38 · **Outil :** telnet · **Auto :** non

**Pré :** Server running on :8080.

**Procédure :**

```sh
telnet localhost 8080  # then type exactly (each line + Enter):\nGET / HTTP/1.1\nHost: localhost\n<blank line>\n# observe first response, then on the SAME session type a second:\nGET /404.html HTTP/1.1\nHost: localhost\nConnection: close\n<blank line>
```

**Attendu :** First request yields a valid status line 'HTTP/1.1 200 OK' with Server, Date, Content-Type, Content-Length headers, blank line, then the body; the connection stays open (keep-alive default). The second request is answered in order on the same connection, and because Connection: close was sent the server closes afterward (telnet prints 'Connection closed by foreign host'). No hang, no garbage. (Telnet sends CRLF per line, satisfying framing.)

#### `STRESS-16` · P1 · integration — Real-browser acceptance (Chrome + Firefox): page loads, wrong URL -> error, autoindex, upload form
**Réf :** REQ-13 · **Outil :** Chrome/Firefox (manual) · **Auto :** non

**Pré :** Server running on :8080; www/index.html, www/static/ (autoindex on), an upload route (/uploads or /app/uploads with an HTML form) present.

**Procédure :**

```sh
In Chrome AND Firefox: (1) open http://localhost:8080/ — assets (css/js/img) load; (2) open http://localhost:8080/definitely-missing — see the 404 page; (3) open http://localhost:8080/static/ — directory listing renders and links work; (4) open an upload form and POST a file, then GET it back. Keep DevTools Network open; also test http://localhost:8080/old-page (301 to example.com) follows.
```

**Attendu :** Every page renders with no browser framing error (no ERR_INVALID_HTTP_RESPONSE / ERR_CONTENT_LENGTH_MISMATCH / infinite spinner). Wrong URL shows the served 404 body. Autoindex lists entries and navigation works. Uploaded file is stored and retrievable byte-identical. Redirect is followed. Watch for the busy-spin (STRESS-19) causing sluggishness; note any keep-alive mis-framing on large assets.

#### `STRESS-17` · P1 · stress — Many concurrent idle keep-alive connections: survive, and select-config FD_SETSIZE cap is graceful
**Réf :** REQ-12 · **Outil :** python3 + lsof · **Auto :** oui

**Pré :** poll config (blucken.conf) has no fd cap -> unbounded growth risk; select config (default.conf) throws in SelectEventIO::add at fd>=1024 and must drop the connection without crashing.

**Procédure :**

```sh
python3 -c "import socket,time; ss=[]; \nfor i in range(1100):\n  try:\n    s=socket.socket(); s.connect(('127.0.0.1',8080)); s.send(b'GET / HTTP/1.1\r\nHost: x\r\n'); ss.append(s)\n  except Exception as e:\n    print('stopped at',i,e); break\nprint('open',len(ss)); time.sleep(90)" & PID=$(pgrep -f 'webserv.*blucken'); sleep 5; lsof -p $PID | wc -l; ps -p $PID >/dev/null && echo ALIVE || echo DEAD
```

**Attendu :** With poll (blucken.conf): all connections accepted, server stays ALIVE, and after keepalive_timeout (60s) the idle connections are reaped (lsof drops back down) — verify no crash and eventual reap. With select (default.conf): connections beyond ~FD_SETSIZE(1024) are refused/dropped via a caught throw, server stays ALIVE (graceful backpressure), no UB/crash.

#### `STRESS-18` · P0 · robustness — Post-adversarial liveness gate: same PID alive and serving 200 after every stress test
**Réf :** REQ-11 · **Outil :** curl + ps · **Auto :** oui

**Pré :** Run LAST, after STRESS-06/07/08/09/10 and any crash-seeking probe, using the ORIGINAL server PID captured at launch.

**Procédure :**

```sh
PID0=$(cat /tmp/webserv.pid 2>/dev/null || pgrep -f 'webserv.*blucken'); ps -p $PID0 >/dev/null && echo "same-pid-alive:$PID0" || echo 'CRASHED/RESPAWNED'; for p in 8080 8081 8082 8083 8084; do curl -s -o /dev/null -w "port$p:%{http_code}\n" --max-time 5 http://localhost:$p/; done; ps -o stat= -p $PID0 | grep -q Z && echo 'ZOMBIE?' ; pgrep -P $PID0 | xargs -I{} sh -c 'ps -o stat= -p {}' | grep -c Z
```

**Attendu :** The process is still the SAME pid (never crashed/aborted/restarted), each of the 5 ports returns 200, and there are no leftover zombie CGI children. Any crash, abort, or unexpected exit across the adversarial suite is grade 0 per REQ-11.

#### `STRESS-19` · P1 · stress — CPU busy-spin observation with a single idle keep-alive client (E_OUT always registered)
**Réf :** REQ-05 · **Outil :** python3 + ps/top · **Auto :** oui

**Pré :** Client sockets are registered E_IN|E_OUT permanently and never toggled; a connected socket is almost always writable, so poll()/select() returns immediately every iteration.

**Procédure :**

```sh
python3 -c "import socket,time; s=socket.socket(); s.connect(('127.0.0.1',8080)); s.send(b'GET / HTTP/1.1\r\nHost: x\r\n\r\n'); time.sleep(30)" & sleep 1; PID=$(pgrep -f 'webserv.*blucken'); for i in 1 2 3 4 5; do ps -o %cpu= -p $PID; sleep 1; done
```

**Attendu :** A well-behaved event loop is near 0% CPU while idle (blocks in poll up to IO_TIMEOUT_MS=100). EXPECTED FINDING: CPU pegs near 100% of a core while even one client is connected, because E_OUT keeps the socket 'ready' and poll never idles. Not a crash, but an evaluator-visible resilience/quality defect (heat/fan, wasted cycles under load).

#### `STRESS-20` · P1 · robustness — Runaway CGI hits the 30s timeout (504) and does not freeze the server (no-hang under load)
**Réf :** REQ-10 · **Outil :** curl + www/cgi-bin/sleep.py · **Auto :** oui

**Pré :** sleep.py sleeps 60s; CGI_TIMEOUT_S=30. /cgi-bin allows GET/POST and enable_cgi on.

**Procédure :**

```sh
( time curl -s -o /dev/null -w 'cgi:%{http_code} t=%{time_total}\n' --max-time 40 http://localhost:8080/cgi-bin/sleep.py ) & sleep 2; curl -s -o /dev/null -w 'concurrent:%{http_code} t=%{time_total}\n' --max-time 5 http://localhost:8080/ ; wait; PID=$(pgrep -f 'webserv.*blucken'); pgrep -P $PID | xargs -r -I{} ps -o pid=,stat= -p {} ; echo 'check no lingering sleep.py / zombie'
```

**Attendu :** The CGI request returns a 5xx (504 preferred, 502/500 acceptable) at ~30s — NOT at 60s and NOT never. The concurrent GET / returns 200 within its 5s deadline (server not frozen by the hung CGI, since the client E_OUT keeps the loop driving checkTimeout). After completion no orphaned sleep.py process and no zombie child remains (killAndReap ran).

---

## 7. Lacunes de complétude

_Résultat de l'agent-critique : ce qu'un évaluateur 42 tenterait et qui manque encore._

### 7.1 Exigences sans test

- **REQ-02 (cli-config): server must run from a config given as arg OR a default path — the no-argument invocation branch is never exercised.**
  - _Pourquoi :_ Evaluators routinely launch the binary bare to see what happens; a crash or hang on argc handling is an immediate red flag and REQ-02 is mandatory. Every authored CFG case passes an explicit path, so the default/no-arg code path is completely uncovered.
  - _Test suggéré :_ Run `./webserv` with ZERO arguments from the project dir. Assert it either (a) loads a documented default config and serves on that config's ports (curl returns a response), or (b) exits cleanly (non-crash, no segfault, deterministic exit code) with a usage/diagnostic message. Also run `./webserv a.conf b.conf` (too many args) and assert clean rejection.
- **REQ-36 (cgi): the CGI must be executed in the script's own directory so relative-path file opens resolve (server MUST chdir into the script dir before execve).**
  - _Pourquoi :_ Explicit mandatory subject requirement with a dedicated bullet; there is currently no test that the child chdir's. If the server execve's without chdir, real PHP/Python apps that read relative includes silently break — a classic eval probe.
  - _Test suggéré :_ Create www/cgi-bin/relread.py that does `open('sibling.txt').read()` (no path prefix) and prints it, plus www/cgi-bin/sibling.txt='RELOK'. GET /cgi-bin/relread.py and assert body contains 'RELOK'. Negative control: place sibling.txt only in the CWD of the server (not in cgi-bin) and confirm it is NOT found — proving the chdir happened.
- **REQ-40 / non-blocking sockets (mandatory): accepted client fds must be non-blocking, and on macOS fcntl may use only F_SETFL with O_NONBLOCK/FD_CLOEXEC. The code analysis states accepted sockets are left BLOCKING — but no test positively asserts non-blocking mode or audits fcntl flags.**
  - _Pourquoi :_ This is THE top mandatory constraint (never block at all times) and the analysis flags it as the single biggest defect. The plan proves the symptom (head-of-line stall) but never states the required positive property or the macOS fcntl-flag restriction as a pass/fail gate.
  - _Test suggéré :_ (a) Source/strace/dtruss audit: grep the accept path for a setIsNonblock/fcntl(F_SETFL,O_NONBLOCK) call on the accepted fd; assert every fcntl call uses only F_SETFL with O_NONBLOCK/FD_CLOEXEC (no F_GETFL-only misuse or other flags). (b) Behavioral proxy: a recv on an empty just-connected socket must return immediately (server never blocks) — this is what STRESS-08/CONN-09 exploit, so pair those with an explicit 'sockets are O_NONBLOCK' assertion.
- **RFC3875 CGI-BODY-LENGTH-BOUND (rfc-MUST): the CGI must receive EXACTLY CONTENT_LENGTH bytes on stdin and no pipelined/following-request bytes may leak into the script.**
  - _Pourquoi :_ HTTP-12 tests server-level desync but never checks the CGI-stdin delimiting boundary; a CGI that reads until EOF would swallow the next request. This is a smuggling/correctness gap distinct from the tested cases.
  - _Test suggéré :_ On one keep-alive connection, POST a known N-byte body to a CGI that echoes stdin, immediately followed (same segment) by a second pipelined valid request. Assert the CGI's echoed stdin is exactly the N body bytes (not N+the next request line) and that the following request is answered as its own response.
- **RFC3875 CGI-RESP-LOCAL-REDIRECT (rfc-MUST): a CGI 'Location:' with an absolute PATH (no scheme) must trigger an internal re-process and return that local resource's content, NOT a 3xx to the client.**
  - _Pourquoi :_ CGI-13 only covers the client-redirect (absolute-URI→302) branch. The local-redirect branch is a separate MUST with opposite behavior and is entirely uncovered.
  - _Test suggéré :_ Create www/cgi-bin/localredir.py emitting only `Location: /index_test.html\r\n\r\n`. GET it and assert the client receives 200 with the BODY of index_test.html (not a 302, not an empty redirect). Contrast with CGI-13's absolute-URI case which must stay a 302.
- **RFC3875 CGI-RESP-FORWARD-HEADERS (rfc-MUST): extension/protocol headers the script emits must be forwarded to the client.**
  - _Pourquoi :_ No case verifies pass-through of arbitrary CGI headers; this is how CGI apps set cookies/caching. Without it, session/auth CGIs silently lose headers.
  - _Test suggéré :_ Create a CGI emitting `Content-Type: text/plain\r\nX-Custom: yes\r\nSet-Cookie: sid=1\r\n\r\nbody`. Assert the client HTTP response contains `X-Custom: yes` and `Set-Cookie: sid=1` verbatim.
- **RFC3875 CGI-META-HTTP_STAR (rfc-MUST): duplicate request headers of the same name must be merged into one comma-separated HTTP_* value, and Content-Length/Content-Type/Authorization must NOT appear as HTTP_* duplicates.**
  - _Pourquoi :_ CGI-17 only checks value casing of single headers; the merge rule and the CONTENT_* exclusion rule (both MUST) are untested and are common CGI-conformance gotchas.
  - _Test suggéré :_ Send two `X-Dup: a` and `X-Dup: b` headers plus Content-Length and Content-Type on a POST to env.sh. Assert HTTP_X_DUP=='a, b' (single merged var), and assert HTTP_CONTENT_LENGTH / HTTP_CONTENT_TYPE are ABSENT (present only as CONTENT_LENGTH/CONTENT_TYPE).
- **RFC9110 R-METHOD-POST / R-201: a POST that creates a resource should return 201 with a Location header (POST success path).**
  - _Pourquoi :_ Every authored POST case asserts a FAILURE status (403/404/405/415/301). There is no positive POST-success assertion anywhere, so REQ-21's happy path and R-201-via-POST are unverified — a hole an evaluator will immediately probe with a form.
  - _Test suggéré :_ Configure/point at a route where POST is meant to create/upload a resource; POST a body and assert 2xx (201 with Location if created) and that a subsequent GET returns the stored bytes. If the implementation genuinely has no POST-success path (per POST-UPLOAD-GAP), record it explicitly as a REQ-19/REQ-21 conformance finding rather than leaving 201-via-POST untested.
- **REQ-03: the Server response header must not identify a stock third-party server (no nginx/apache), and no web-server binary is execve'd.**
  - _Pourquoi :_ CONN-15 checks fork but not the Server-header identity / execve-of-webserver aspect of REQ-03. Cheap to add and directly named in the subject.
  - _Test suggéré :_ curl -sD - and assert the `Server:` header (if any) is the project's own token, never 'nginx'/'Apache'/'httpd'. Combine with the CONN-15 strace to confirm no execve of a web-server binary (execve only for CGI interpreters).
- **REQ-42 (bonus): cookie/session round-trip — first request issues Set-Cookie, a follow-up carrying that cookie yields stateful behavior, absent/different cookie gets a fresh session.**
  - _Pourquoi :_ The codebase clearly HAS a SessionStore (STRESS-02 weaponizes it for a memory-leak DoS) yet the plan never verifies the feature actually works. Since the machinery exists, the bonus is cheaply claimable and the memory finding needs a functional baseline to compare against.
  - _Test suggéré :_ Hit a session-enabled route; capture Set-Cookie; replay with the cookie and assert a stateful change (counter increments / remembered value); replay without it and assert an independent/fresh session.
- **REQ-35 (cgi): CGI output with NO Content-Length must be fully delivered, terminated at the CGI's EOF, with no truncation or hang (the general no-CL-body case, not just Status-only or the >8KB truncation bug).**
  - _Pourquoi :_ CGI-09 targets the >8KB truncation gap and CGI-14 is Status-only; the plain no-Content-Length small-body delivery (the core REQ-35 assertion) is not directly asserted for completeness/framing.
  - _Test suggéré :_ CGI that emits `Content-Type: text/plain\r\n\r\n` then writes a body of a known middling size (e.g. 3000 bytes) WITHOUT Content-Length and exits. Assert the client receives all 3000 bytes and the response is framed/closed correctly (server supplies its own framing).
- **REQ-23 (simultaneous, distinct content per port): the subject asks that two ports serve DIFFERENT content CONCURRENTLY from one process.**
  - _Pourquoi :_ RT-05 issues the two GETs sequentially; the 'simultaneously / multiple websites by one program' aspect (and that one slow port doesn't block the other listener) is not exercised.
  - _Test suggéré :_ Extend RT-05 to fire both `curl :8080/` and `curl :8081/` truly in parallel (backgrounded) and assert each returns its own distinct configured body while the other request is in flight, confirming the single event loop multiplexes both listeners at once.

### 7.2 Catégories manquantes

- Cookie / session management functional tests (REQ-42) — the SessionStore is only ever used as a DoS vector in STRESS-02; there is no Set-Cookie / stateful round-trip / fresh-session category at all.
- CGI working-directory (chdir) behavior (REQ-36) — no case verifies relative-path file access from the script's own directory.
- CGI response-mode coverage beyond client-redirect: local-redirect (absolute-path Location → internal reprocess), client-redirect-with-document (Location+Status+body), and extension-header forwarding (X-Custom/Set-Cookie) are all absent.
- Positive POST-success path — every POST case asserts a failure code; there is no test that any POST is accepted (2xx) and its effect stored/reflected, leaving REQ-21's happy path and R-201-via-POST uncovered.
- No-argument / default-config invocation (REQ-02) and too-many-args handling — all config tests pass an explicit path.
- Positive non-blocking-socket / fcntl-flag assertion (REQ-40) — the blocking-socket defect is only demonstrated behaviorally; no test states the required property (accepted fds O_NONBLOCK; macOS fcntl limited to F_SETFL O_NONBLOCK/FD_CLOEXEC) as a pass/fail gate.
- Server-identity / anti-third-party checks (REQ-03) — Server header value and absence of a web-server execve are not asserted.
- CGI stdin length-bounding (CGI-BODY-LENGTH-BOUND) and duplicate-header merge / CONTENT_* exclusion (CGI-META-HTTP_STAR) — meta-variable edge rules are only partially covered.
- A real browser file-upload round-trip that SUCCEEDS (byte-identical retrieval) — only the multipart/urlencoded FAILURE gaps are authored; browser-accepted upload (REQ-13/REQ-19) has no green-path case.
- Config robustness edges: comments (#) in config, tab/whitespace/CRLF tolerance, relative vs absolute config path, and duplicate-listen / same host:port dedupe semantics beyond the vhost bind-conflict case.

### 7.3 Redondances à fusionner

- HTTP/1.0 → 505 divergence is authored three times: HTTP-05, HSF-17, SC-21. Keep one canonical differential case and cross-reference.
- Date header 'UTC' vs 'GMT' defect appears twice: HSF-16 and SC-26.
- Unknown/lowercase method → 501 is triplicated: HTTP-03, HSF-14, SC-20.
- Missing-Host → 400 appears in HTTP-06, SC-07, and again inside SC-08.
- Segmented/dripped header delivery → spurious 400 (gap #1) is authored three times: HTTP-01, CONN-05, STRESS-10. Consolidate to one differential-vs-nginx case plus the liveness assertion.
- GET-with-body pipelining desync (gap #3) is duplicated: HTTP-12 and CONN-04.
- Slow non-reading-client head-of-line stall (blocking socket) is duplicated: CONN-09 and STRESS-08.
- fd-leak-under-churn is covered four times: CGI-11, CONN-12, STRESS-03, and DAV-ROBUST-LOOP.
- Incomplete/short body must-not-hang is triplicated: HTTP-17, CONN-07, STRESS-09.
- CGI timeout → 5xx no-hang is duplicated: CGI-05 and STRESS-20.
- Idle-client CPU busy-spin (permanent E_OUT) is duplicated: CONN-17 and STRESS-19.
- Graceful-shutdown / broken g_SignalStatus is duplicated: CONN-18 and STRESS-05.
- 405 + Allow header is exercised in many places (HSF-13, SC-14, PUT-05, DEL-04, DAV-DIFF-01); fine as route-specific checks but the generic 'Allow present on 405' assertion is repeated.
- 413 body-limit is covered by HTTP-16, CMBS-413-01, and SC-17; 414 by HTTP-04 and SC-18; CL+TE-both by HTTP-11 and FRAME-CL-TE-411; FD_CLOEXEC-CGI by CGI-18 and STRESS-14 — dedupe or explicitly mark the survivor per axis.

### 7.4 Notes de priorisation

- The two highest-weight mandatory 42 constraints — single-poll-for-all-I/O (REQ-05/07/09/17) and no-errno-after-read/write (REQ-08) — live ONLY in CONN-15 and CONN-16, both marked automatable:false (require strace/dtruss). These are pass/grade-0 gates; make absolutely sure they are actually run on the Linux eval box (dtruss on macOS needs SIP allowance) and add a behavioral proxy where possible. Do not let the manual-only status cause them to be skipped.
- The BLOCKING accepted-socket defect (CONN-09/STRESS-08) is the most consequential finding: it violates the mandatory 'never block' + 'monitor read and write simultaneously' rules and is a trivial single-slow-client DoS. Run it FIRST and treat any failure as project-fatal, not a cosmetic diff.
- Segmented-header → spurious 400 (gap #1) directly breaks REQ-13 (browsers) and REQ-12 (siege reliability) — it is far more than a diff. Elevate to a P0 must-fix and verify against a real browser (STRESS-16), not just nc.
- Because the whole plan is written for a macOS host but the official tester/cgi_tester binaries are Linux x86-64 ELF (STRESS-12/13), the mandatory grader cannot run natively — schedule a Linux VM/Docker pass early; a plan that only runs on macOS silently omits the single most authoritative signal (REQ-38).
- Several 'differential-vs-nginx' P0/P1 cases assume a recorded nginx oracle exists (test/nginx_tests/output/*.txt). Confirm those transcripts are actually captured before eval; otherwise these cases have no baseline and silently pass/fail.
- Cheap, high-value additions to close mandatory gaps: REQ-36 (CGI chdir), CGI-BODY-LENGTH-BOUND, CGI-RESP-LOCAL-REDIRECT, CGI-RESP-FORWARD-HEADERS, and a positive POST-success case — all are small scripts/requests yet cover MUST-level items with zero current coverage.
- STRESS-18 (post-adversarial liveness gate) is correctly P0 and last; ensure every crash-seeking case (STRESS-06/07, HTTP-14, HTTP-18, CMBS-CL-OVERFLOW, CGI-06) runs BEFORE it and shares the original captured PID, so a silent respawn cannot mask a crash.
- Given the confirmed broken PUT-on-DAV-routes (SC-19) and broken POST/multipart uploads, REQ-19/REQ-31 (mandatory 'clients must upload files') is at real risk of scoring 0. Prioritize a single end-to-end upload round-trip that WORKS on some route as the make-or-break mandatory check, above the many redundant robustness loops.

---

## 8. Plan d'automatisation

### Outils requis

`curl`, `nc` (netcat), `python3` (sockets pour framing déterministe), `siege` ou `ab`/`wrk` (stress), `valgrind`/`leaks` (fuites), `lsof` (fuites de fd), `strace`/`dtruss` (audit poll/errno/execve), un navigateur.

### Runners à écrire (par ordre de valeur)

1. **`test/http_tests/run.sh`** — runner boîte-noire HTTP inexistant aujourd'hui. Lance le serveur avec une config de test, exécute les cas `HTTP-*`, `MTH-*`, `STATUS-*` via `curl`/`nc`, compare code de statut + en-têtes attendus, agrège PASS/FAIL. C'est le plus gros trou (§3).
2. **`test/cgi_tests/run.sh`** — automatise les cas `CGI-*` : GET/POST, timeout (script `sleep.py`), crash (`malformed.py`), variables d'env (`env.sh`), plus vérif absence de zombies (`ps`) et de fuite de fd (`lsof`).
3. **`test/robustness/`** — slow-loris, requête partielle, garbage binaire, déconnexion mi-requête, siege soutenu + surveillance RSS/fd. Chaque test vérifie que le PID survit (`REQ-11`).
4. **Audit `strace`/`dtruss`** (semi-manuel, `CONN-*`) — un seul site d'appel poll, read/write toujours après readiness, aucun `errno` branché après read/write, `fork` seulement pour CGI, aucun `execve` de serveur tiers.
5. **Étendre** `test/parser_tests` (cas `CFG-*` : no-arg, trop d'args, `#` commentaires, chemins relatifs) et `test/nginx_tests` (ajouter le chemin POST **succès** et les cas manquants du §7).

### Harnais suggéré

Un `test/run_all.sh` qui build (`make`), lance chaque runner, et sort un code ≠ 0 si un seul FAIL — prêt pour un hook pre-commit ou une CI. Réutiliser le style d'assertions colorées de `test/parser_tests/run_tests.sh`.

---

## 9. Annexes — exigences détaillées

### 9.1 Sujet 42 (43)

**cli-config**

- **REQ-01** `mandatory` — The program is named 'webserv' and is executed as './webserv [configuration file]', taking a configuration file path as its command-line argument.
  - _Test :_ Launch './webserv conf/test.conf'; the process starts and begins accepting TCP connections on the interface:port pairs declared in that file (curl to a configured port returns a response). Changing the port in the passed config and relaunching makes the server listen on the new port.
- **REQ-02** `mandatory` — The server must use a configuration file provided as a command-line argument OR available in a default path.
  - _Test :_ Run './webserv' with no argument: it either loads a documented default config (and serves on that config's ports) or exits cleanly without crashing. Run with an explicit path: the settings in THAT file (ports, roots) take effect, observable via HTTP responses on the configured ports.

**io-model**

- **REQ-03** `mandatory` — You cannot execve another web server; all HTTP functionality must be implemented by webserv itself.
  - _Test :_ ps/pstree shows no nginx/apache/httpd child process while serving; strace/dtruss of the server shows no execve of a web-server binary (execve appears only for CGI interpreters). The Server response header does not identify a stock third-party server.
- **REQ-04** `mandatory` — The server must remain non-blocking at all times and properly handle client disconnections.
  - _Test :_ Open a connection, send a partial request, then abruptly close the socket: the server does not hang or crash and continues serving other clients (a concurrent curl still succeeds). A slow client never freezes responses to other clients.
- **REQ-05** `mandatory` — The server must be non-blocking and use only ONE poll() (or equivalent: select/epoll/kqueue) for ALL I/O operations between clients and server, listening sockets included.
  - _Test :_ strace/dtruss shows a single multiplexing loop (one epoll_wait/kqueue/poll/select call site) handling every file descriptor including the listen sockets; no per-connection thread or process. Many simultaneous connections are all serviced by that one event loop.
- **REQ-06** `mandatory` — poll() (or equivalent) must monitor reading and writing simultaneously.
  - _Test :_ strace shows both read (POLLIN) and write (POLLOUT) interest registered in the same poll/epoll/kqueue set. Behaviorally: a client that reads a large response slowly and a client sending a request both make progress without the server blocking on either.
- **REQ-07** `mandatory` — You must never do a read or a write operation without going through poll() (or equivalent) first.
  - _Test :_ strace shows every recv/read and send/write on a socket/pipe is preceded by that fd being returned ready by the poll/epoll/kqueue call; no recv/send occurs on a descriptor not reported ready.
- **REQ-08** `mandatory` — Checking the value of errno to adjust server behaviour is strictly forbidden after performing a read or write operation.
  - _Test :_ Source/strace inspection: after a recv/send/read/write returns -1, the server does not branch on errno (e.g., EAGAIN/EWOULDBLOCK) to decide behaviour; it relies solely on poll readiness. Behaviorally, it never immediately retries recv/send on an fd that poll did not mark ready.
- **REQ-09** `mandatory` — Calling read/recv or write/send on a socket/pipe/FIFO without prior poll-reported readiness results in a grade of 0 (regular disk files are exempt and may be read/written directly).
  - _Test :_ strace confirms recv/send on sockets and CGI pipes happen only after a corresponding readiness event; at most the natural one recv (or send) per client per readiness event. Reads/writes on regular files may occur directly without a preceding poll on them.
- **REQ-17** `mandatory` — fork may not be used for anything other than CGI.
  - _Test :_ strace during a plain static GET shows no fork/clone; strace during a CGI request shows fork only to spawn the CGI interpreter. No fork occurs for accept/serve of static content.

**resilience**

- **REQ-10** `mandatory` — A request to the server should never hang indefinitely.
  - _Test :_ Send an incomplete request (headers without terminating CRLF, or a Content-Length larger than the body actually sent): the server eventually closes/times out the connection instead of hanging forever, and other clients remain served. A well-formed request always returns within a bounded time (curl --max-time succeeds).
- **REQ-11** `mandatory` — The program must not crash under any circumstances (even if it runs out of memory) or terminate unexpectedly; otherwise the project is non-functional (grade 0).
  - _Test :_ After sending malformed, oversized, garbage, or memory-pressuring requests, the process is still alive and continues to accept new connections and return responses (verify PID still running and a fresh curl succeeds).
- **REQ-12** `mandatory` — The server must remain available/operational at all times under stress (resilience is key).
  - _Test :_ Run a load/stress tool (siege/ab/wrk) with many concurrent connections over a sustained period: availability stays ~100%, no hung or leaked connections, no crash, and response times stay bounded throughout.
- **REQ-38** `mandatory` — Do not test with only one program/language; the server must behave correctly against varied clients (browsers, telnet, custom scripts in Python/Go/C, NGINX comparison).
  - _Test :_ The same endpoints produce consistent, correct responses whether hit by a browser, telnet (raw request), curl, and a custom test client; header/behavior comparison against NGINX shows no protocol-breaking differences (accounting for HTTP version differences).

**static-serving**

- **REQ-13** `mandatory` — The server must be compatible with standard web browsers of your choice.
  - _Test :_ Open a served page in a real browser (Chrome/Firefox): the page and its linked assets (CSS/JS/images) load and render correctly, with headers a browser accepts (valid status line, Content-Type, Content-Length/chunked).
- **REQ-18** `mandatory` — The server must be able to serve a fully static website.
  - _Test :_ GET an HTML page and its referenced static assets: each returns the exact file bytes with a correct Content-Type (text/html, text/css, image/png, etc.) and correct length; the complete multi-file site is served.

**error-handling**

- **REQ-14** `mandatory` — HTTP response status codes must be accurate.
  - _Test :_ Each situation yields the correct status: existing resource GET -> 200; missing -> 404; method not allowed on a route -> 405; body over client_max_body_size -> 413; configured redirect -> 301/302; malformed request -> 400. The returned status line matches the condition.
- **REQ-15** `mandatory` — The server must have default error pages if none are provided in the configuration.
  - _Test :_ Trigger an error status (e.g., 404) on a route with no error_page configured: the response has a non-empty, server-generated HTML error body whose status matches (not an empty body or a connection reset).
- **REQ-16** `mandatory` — The configuration file must allow setting up custom (default) error pages (error_page directive).
  - _Test :_ Configure a custom error page for a status (e.g., 404 -> /custom_404.html); requesting a missing resource returns 404 with the exact bytes of custom_404.html as the body.

**upload**

- **REQ-19** `mandatory` — Clients must be able to upload files.
  - _Test :_ POST a file to an upload-enabled route (multipart/form-data or raw body): the server accepts it (2xx) and the file is stored; a subsequent GET of the uploaded resource returns byte-identical content.
- **REQ-31** `mandatory` — The configuration must allow authorizing client uploads for a route and specifying the storage location where uploaded files are saved.
  - _Test :_ Upload a file via POST to an upload-enabled route; the file appears in the configured storage directory on disk (and/or is retrievable via a subsequent GET) with identical content, confirming the configured upload store path is honored.

**http-methods**

- **REQ-20** `mandatory` — The server must support the GET method.
  - _Test :_ GET an existing resource returns 200 with the resource body and correct headers.
- **REQ-21** `mandatory` — The server must support the POST method.
  - _Test :_ POST a request body to a route that accepts it (upload or CGI): the server reads the full body and processes it, returning an appropriate 2xx and (for CGI) output reflecting the posted data.
- **REQ-22** `mandatory` — The server must support the DELETE method.
  - _Test :_ DELETE an existing resource returns 200/204 and removes it; a subsequent GET of the same URL returns 404.
- **REQ-39** `mandatory` — The server must handle chunked (Transfer-Encoding: chunked) request bodies by reassembling them, for normal routes as well as CGI.
  - _Test :_ Send a POST with Transfer-Encoding: chunked to an upload or echo route; the server correctly reconstructs the full body (stored/echoed content matches the original un-chunked bytes) and returns success.

**virtual-host**

- **REQ-23** `mandatory` — The server must be able to listen on multiple ports and deliver different content per port.
  - _Test :_ With two listen ports configured to different roots/content, GET http://host:PORT1/ and GET http://host:PORT2/ simultaneously return the distinct content configured for each port.
- **REQ-24** `mandatory` — The configuration must let you define all the interface:port pairs on which the server listens (multiple servers/websites served by one program).
  - _Test :_ Configure several server blocks on different host:port pairs; all of them are reachable concurrently from a single running webserv process, each returning its own configured content.
- **REQ-41** `bonus` — Server-name based virtual hosting and selection of a default server for a given host:port (via the Host header / server_name). The subject deems virtual hosts out of scope but allows implementing them.
  - _Test :_ With multiple server blocks sharing a host:port distinguished by server_name, a request with Host: nameA returns server A's content and Host: nameB returns server B's content; a request with an unknown/absent Host falls back to the default (first) server block for that host:port.

**routing-config**

- **REQ-25** `mandatory` — The configuration must let you set the maximum allowed size for client request bodies (client_max_body_size).
  - _Test :_ With a small configured limit, POST a body larger than the limit -> server responds 413 (Payload/Content Too Large) and does not store/process it; a body within the limit is accepted normally.
- **REQ-26** `mandatory` — The configuration must specify the list of accepted HTTP methods per route.
  - _Test :_ On a route that allows only GET, a POST or DELETE returns 405 Method Not Allowed (ideally with an Allow header listing permitted methods); the allowed method succeeds on the same route.
- **REQ-27** `mandatory` — The configuration must support HTTP redirection on a route.
  - _Test :_ Request a route configured as a redirect: the response is a 3xx (e.g., 301/302, or the configured code) with a Location header pointing at the configured target URL; a browser/curl -L follows it to the target.
- **REQ-28** `mandatory` — The configuration must specify the root directory where requested files are located, with correct path mapping (e.g., URL /kapouet rooted to /tmp/www -> /kapouet/pouic/toto/pouet serves /tmp/www/pouic/toto/pouet).
  - _Test :_ Place a file at /tmp/www/pouic/toto/pouet with route /kapouet rooted to /tmp/www; GET /kapouet/pouic/toto/pouet returns exactly that file's contents, confirming the URL-to-filesystem mapping strips the route prefix and joins the root.
- **REQ-29** `mandatory` — The configuration must allow enabling or disabling directory listing (autoindex).
  - _Test :_ For a directory route with autoindex on and no index file, GET the directory returns an auto-generated HTML listing of its entries; with autoindex off and no index file, the same GET returns 403 (or 404) with no listing.
- **REQ-30** `mandatory` — The configuration must specify a default file to serve when the requested resource is a directory (index).
  - _Test :_ With index configured (e.g., index.html) for a directory route, GET the directory path (e.g., '/') returns the contents of that default file with 200, rather than a listing or 403.

**cgi**

- **REQ-32** `mandatory` — The configuration must trigger CGI execution based on file extension (e.g., .php).
  - _Test :_ Request a resource whose extension is mapped to a CGI (e.g., test.php or test.py): the response body is the dynamic OUTPUT of the executed script, not the raw script source, confirming extension-based CGI dispatch.
- **REQ-33** `mandatory` — The CGI must receive the correct environment variables; the full request and arguments provided by the client must be available to the CGI.
  - _Test :_ Use a CGI script that echoes its environment; request it with a query string and a specific method/path: the response reflects REQUEST_METHOD, QUERY_STRING, PATH_INFO, CONTENT_LENGTH, etc. matching what was sent (e.g., ?a=1&b=2 appears in QUERY_STRING).
- **REQ-34** `mandatory` — For chunked requests, the server must un-chunk the body before passing it to the CGI (the CGI expects EOF as the end of the body).
  - _Test :_ Send a POST with Transfer-Encoding: chunked to a CGI route; a CGI that reports the length/content of its stdin shows the fully re-assembled (de-chunked) body, and it sees EOF at the true end of the body.
- **REQ-35** `mandatory` — For CGI output, if no Content-Length is returned by the CGI, EOF marks the end of the returned data.
  - _Test :_ Use a CGI script that emits a body without a Content-Length header; the server still returns the complete output to the client (correct full body) and closes the response at the CGI's EOF, with no truncation or hang.
- **REQ-36** `mandatory` — The CGI must be run in the correct directory for relative-path file access.
  - _Test :_ Use a CGI that opens a file by relative path (a sibling in the script's directory) and returns its content; the request succeeds, proving the server chdir'd into the script's directory before executing it.
- **REQ-37** `mandatory` — The server must support at least one CGI (e.g., php-CGI or Python).
  - _Test :_ At least one CGI interpreter works end-to-end: requesting a script of that type returns correct dynamically generated output for both GET (query string) and POST (request body) cases.

**platform**

- **REQ-40** `mandatory` — On macOS, fcntl() may be used only with F_SETFL, O_NONBLOCK and FD_CLOEXEC to put descriptors in non-blocking mode; any other flag is forbidden.
  - _Test :_ Source/strace inspection on macOS shows fcntl calls limited to F_SETFL with O_NONBLOCK/FD_CLOEXEC only; sockets/pipes are set non-blocking (a recv on an empty non-blocking socket returns immediately rather than blocking), matching non-blocking behaviour of other Unix OSes.

**bonus**

- **REQ-42** `bonus` — Support cookies and session management (provide simple examples).
  - _Test :_ A first request returns a Set-Cookie header establishing a session id; a subsequent request carrying that Cookie yields stateful, session-specific behaviour (e.g., a counter or remembered value), while a request without/with a different cookie gets a fresh/independent session.
- **REQ-43** `bonus` — Handle multiple CGI types.
  - _Test :_ At least two distinct CGI types are handled (e.g., .php and .py): requesting a script of each extension executes the corresponding interpreter and returns that script's correct dynamic output.

### 9.2 RFC 9112 — Framing HTTP/1.1 (30)

- **R01-request-line-format** `rfc-MUST` (request-line) — Section 3: a request-line is `method SP request-target SP HTTP-version CRLF` with exactly one SP as delimiter. Section 2.2 requires recipients to reject malformed framing. A request-line with the wrong number of components, extra/missing SP, or missing version is invalid and must be rejected (400).
  - _Test :_ Send `GET  / HTTP/1.1\r\nHost: x\r\n\r\n` (two spaces after method) via `printf ... | nc localhost 8080` → Expect 400 Bad Request. Also send `GET /\r\nHost: x\r\n\r\n` (no HTTP-version) → Expect 400.
- **R02-request-target-too-long-414** `rfc-MUST` (request-line) — Section 3.4: 'A server that receives a request-target longer than any URI it wishes to parse MUST respond with a 414 (URI Too Long) status code.'
  - _Test :_ Send `printf 'GET /%0.s' {1..1}` style long target, e.g. `printf 'GET /%s HTTP/1.1\r\nHost: x\r\n\r\n' "$(python3 -c 'print("a"*20000)')" | nc localhost 8080` → Expect 414 URI Too Long.
- **R03-method-too-long-501** `rfc-SHOULD` (request-line) — Section 3.4: 'A server that receives a method longer than any that it implements SHOULD respond with a 501 (Not Implemented) status code.' Unrecognized methods likewise map to 501 (or 405 if the target exists but method is disallowed).
  - _Test :_ Send `printf '%s / HTTP/1.1\r\nHost: x\r\n\r\n' "$(python3 -c 'print("A"*5000)')" | nc localhost 8080` → Expect 501 Not Implemented. Also send `BREW / HTTP/1.1\r\nHost: x\r\n\r\n` → Expect 501 (or 405).
- **R04-support-8000-octet-request-line** `rfc-SHOULD` (request-line) — Section 3: 'It is RECOMMENDED that all HTTP senders and recipients support, at a minimum, request-line lengths of 8000 octets.' A request-line at/below 8000 octets should not be rejected as too long.
  - _Test :_ Send a request whose request-line is ~8000 octets: `printf 'GET /%s HTTP/1.1\r\nHost: x\r\n\r\n' "$(python3 -c 'print("a"*7900)')" | nc localhost 8080` → Expect NOT 414 (should be 200/404, i.e. parsed normally).
- **R05-bare-lf-line-terminator-tolerance** `bonus` (line-terminators) — Section 2.2: 'a recipient MAY recognize a single LF as a line terminator and ignore any preceding CR.' This is optional; a conformant server may accept bare-LF-terminated lines or reject them, but must be deterministic.
  - _Test :_ Send LF-only terminators `printf 'GET / HTTP/1.1\nHost: x\n\n' | nc localhost 8080` → Expect either a normal response (200/404, LF tolerated) OR 400 (LF rejected). Must not hang or crash.
- **R06-bare-cr-invalid** `rfc-MUST` (line-terminators) — Section 2.2: 'A recipient of such a bare CR MUST consider that element to be invalid or replace each bare CR with SP before processing.' A bare CR (CR not immediately followed by LF) inside the start-line or header section is not a valid line terminator.
  - _Test :_ Send a header with an embedded bare CR: `printf 'GET / HTTP/1.1\r\nHost: x\rY: z\r\n\r\n' | nc localhost 8080` → Expect 400 (element treated as invalid) or the CR sanitized to SP (never interpreted as a line break creating a second header).
- **R07-whitespace-before-first-header-400** `rfc-MUST` (whitespace) — Section 2.2: 'A recipient that receives whitespace between the start-line and the first header field MUST either reject the message as invalid or consume each whitespace-preceded line without further processing.' This is an explicit request-smuggling/response-splitting defense.
  - _Test :_ Send a space-indented line right after the request-line: `printf 'GET / HTTP/1.1\r\n Host: x\r\n\r\n' | nc localhost 8080` → Expect 400 (rejected) OR the indented line ignored and then 400 for missing Host. Must never treat ` Host: x` as a valid Host header.
- **R08-no-ws-before-colon-400** `rfc-MUST` (header-syntax) — Section 5.1: 'A server MUST reject, with a response status code of 400 (Bad Request), any received request message that contains whitespace between a header field name and colon.'
  - _Test :_ Send `printf 'GET / HTTP/1.1\r\nHost: x\r\nX-Test : v\r\n\r\n' | nc localhost 8080` → Expect 400 Bad Request (space before the colon must be rejected).
- **R09-obs-fold-rejected** `rfc-MUST` (obs-fold) — Section 5.2: 'A server that receives an obs-fold in a request message ... MUST either reject the message by sending a 400 (Bad Request) ... or replace each received obs-fold with one or more SP octets prior to interpreting the field value.' Obsolete line folding must not be silently interpreted as separate headers.
  - _Test :_ Send a folded value (continuation line starts with SP/HTAB): `printf 'GET / HTTP/1.1\r\nHost: x\r\nX-Fold: a\r\n b\r\n\r\n' | nc localhost 8080` → Expect 400, OR value folded to `a b` (SP-joined). Must never treat ` b` as its own header line.
- **R10-host-missing-400** `rfc-MUST` (host) — Section 3.2: 'A server MUST respond with a 400 (Bad Request) status code to any HTTP/1.1 request message that lacks a Host header field.'
  - _Test :_ Send `printf 'GET / HTTP/1.1\r\n\r\n' | nc localhost 8080` → Expect 400 Bad Request.
- **R11-host-duplicate-400** `rfc-MUST` (host) — Section 3.2: A server MUST respond with 400 to 'any request message that contains more than one Host header field line'.
  - _Test :_ Send `printf 'GET / HTTP/1.1\r\nHost: a.com\r\nHost: b.com\r\n\r\n' | nc localhost 8080` → Expect 400 Bad Request.
- **R12-host-invalid-value-400** `rfc-MUST` (host) — Section 3.2: A server MUST respond with 400 to a request with 'a Host header field with an invalid field value' (e.g. embedded whitespace/illegal characters in the authority).
  - _Test :_ Send `printf 'GET / HTTP/1.1\r\nHost: exa mple.com\r\n\r\n' | nc localhost 8080` → Expect 400 Bad Request (space inside the host value is invalid).
- **R13-content-length-duplicate-400** `rfc-MUST` (content-length) — Section 6.3 (item 5): if multiple Content-Length fields (or a comma list) have differing values, 'the recipient MUST treat it as an unrecoverable error' → for a request that means 400 and close. Conflicting Content-Length is a classic smuggling vector.
  - _Test :_ Send `printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\nContent-Length: 6\r\n\r\nhello' | nc localhost 8080` → Expect 400 Bad Request and connection close.
- **R14-content-length-nondigit-400** `rfc-MUST` (content-length) — Section 6.3 (item 5) + Section 6.2: Content-Length must be one or more DIGIT. A non-numeric, negative, or otherwise invalid value is an unrecoverable framing error → 400.
  - _Test :_ Send `printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: abc\r\n\r\n' | nc localhost 8080` → Expect 400. Also `Content-Length: -1` and `Content-Length: 0x10` → Expect 400.
- **R15-te-and-cl-both-present-smuggling** `rfc-MUST` (smuggling) — Section 6.3 (item 3): 'If a message is received with both a Transfer-Encoding and a Content-Length header field, the Transfer-Encoding overrides the Content-Length. Such a message might indicate an attempt to perform request smuggling ... and ought to be handled as an error.' Section 6.1: server MAY reject or process per Transfer-Encoding alone, but MUST close the connection after responding.
  - _Test :_ Send `printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 5\r\nTransfer-Encoding: chunked\r\n\r\n0\r\n\r\n' | nc localhost 8080` → Expect 400 (recommended) and connection close; at minimum the Content-Length must be ignored (TE wins) and the connection closed — never both interpreted.
- **R16-chunked-decoding** `rfc-MUST` (chunked) — Section 7.1: 'A recipient MUST be able to parse and decode the chunked transfer coding.' The body is the concatenation of chunk-data; decoding completes on a zero-size chunk followed by an optional trailer and final CRLF.
  - _Test :_ Send `printf 'POST /echo HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\n\r\n' | nc localhost 8080` → Expect 200 (or the route's normal status) with a decoded body of exactly `hello` (5 bytes).
- **R17-chunk-size-hex** `rfc-MUST` (chunked) — Section 7.1: 'The chunk-size field is a string of hex digits indicating the size of the chunk-data in octets.' Chunk sizes are hexadecimal, not decimal (e.g. `A` = 10 bytes, `10` = 16 bytes).
  - _Test :_ Send `printf 'POST /echo HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nA\r\n0123456789\r\n0\r\n\r\n' | nc localhost 8080` → Expect 200 with a decoded body of exactly the 10 bytes `0123456789` (proves hex, not decimal, size parsing).
- **R18-chunk-size-nonhex-400** `rfc-MUST` (chunked) — Section 7.1 / Section 8: a chunk-size that is not valid hexadecimal (or otherwise malformed chunk framing) is an invalid/incomplete message and must be rejected with 400 and the connection closed.
  - _Test :_ Send `printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nXYZ\r\nhello\r\n0\r\n\r\n' | nc localhost 8080` → Expect 400 Bad Request and connection close (non-hex chunk-size).
- **R19-chunk-size-overflow-guard** `rfc-MUST` (chunked) — Section 7.1: 'recipients MUST anticipate potentially large hexadecimal numerals and prevent parsing errors due to integer conversion overflows or precision loss.' An oversized chunk-size must be handled safely (reject) rather than overflowing/crashing.
  - _Test :_ Send `printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\nFFFFFFFFFFFFFFFF\r\n' | nc localhost 8080` → Expect 400 (or a controlled error/close); server MUST NOT crash, hang indefinitely, or wrap the size.
- **R20-chunk-extensions-ignored** `rfc-MUST` (chunked) — Section 7.1.1: 'A recipient MUST ignore unrecognized chunk extensions.' Chunk extensions (`;name=value` after chunk-size) must not affect decoding of chunk-data.
  - _Test :_ Send `printf 'POST /echo HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n5;foo=bar\r\nhello\r\n0\r\n\r\n' | nc localhost 8080` → Expect 200 with decoded body exactly `hello` (extension ignored, not counted toward size).
- **R21-trailers-not-merged** `rfc-MUST` (trailers) — Section 7.1.2: a recipient 'MUST NOT merge a received trailer field into the header section unless its corresponding header field definition explicitly permits.' Trailer fields after the last chunk must not retroactively change request framing/routing headers.
  - _Test :_ Send `printf 'POST /echo HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked\r\n\r\n5\r\nhello\r\n0\r\nX-Trailer: v\r\n\r\n' | nc localhost 8080` → Expect 200 with body `hello`; the trailer must be accepted-and-ignored and MUST NOT be treated as a Host/Content-Length override.
- **R22-unknown-transfer-encoding-501** `rfc-SHOULD` (transfer-encoding) — Section 6.1: 'A server that receives a request message with a transfer coding it does not understand SHOULD respond with 501 (Not Implemented).'
  - _Test :_ Send `printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: boguscoding\r\n\r\n' | nc localhost 8080` → Expect 501 Not Implemented.
- **R23-chunked-not-final-400** `rfc-MUST` (transfer-encoding) — Section 6.3 (item 4): 'If a Transfer-Encoding header field is present in a request and the chunked transfer coding is not the final encoding, the message body length cannot be determined reliably; the server MUST respond with the 400 (Bad Request) status code and then close the connection.'
  - _Test :_ Send `printf 'POST / HTTP/1.1\r\nHost: x\r\nTransfer-Encoding: chunked, gzip\r\n\r\n' | nc localhost 8080` → Expect 400 Bad Request and connection close (chunked is not the last coding).
- **R24-body-length-precedence-neither** `rfc-MUST` (body-length) — Section 6.3 (item 7): 'If this is a request message and none of the above are true, then the message body length is zero (no message body is present).' A request without Content-Length or Transfer-Encoding has an empty body and must not block waiting for one.
  - _Test :_ Send `printf 'POST / HTTP/1.1\r\nHost: x\r\n\r\n' | nc localhost 8080` → Expect an immediate response (200/404/405 etc.), NOT a hang waiting for a body.
- **R25-connection-close-server-closes** `rfc-MUST` (connection) — Section 9.6: 'A server that sends a close connection option MUST initiate closure of the connection ... after it sends the response.' When the client requests `Connection: close`, the server must honor it and close after the response.
  - _Test :_ Send `printf 'GET / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc localhost 8080` → Expect one response then the socket is closed by the server (nc exits/EOF). Response SHOULD carry `Connection: close`.
- **R26-persistent-connection-default** `rfc-MUST` (connection) — Section 9.3: 'HTTP/1.1 defaults to the use of persistent connections.' Absent a `close` option, the server must keep the connection open and answer multiple sequential/pipelined requests on it, responding in request order.
  - _Test :_ Send two requests on one connection: `printf 'GET / HTTP/1.1\r\nHost: x\r\n\r\nGET / HTTP/1.1\r\nHost: x\r\nConnection: close\r\n\r\n' | nc localhost 8080` → Expect TWO responses (in order) before the connection closes after the second.
- **R27-unsupported-version** `rfc-SHOULD` (version) — Section 2.3 defines `HTTP-version = HTTP-name "/" DIGIT "." DIGIT` (case-sensitive). Per HTTP semantics (RFC 9110 §15.6.6), a server SHOULD respond with 505 (HTTP Version Not Supported) to a request whose major version it does not support; a malformed version token is a framing error → 400.
  - _Test :_ Send `printf 'GET / HTTP/2.0\r\nHost: x\r\n\r\n' | nc localhost 8080` → Expect 505 (HTTP Version Not Supported). Send `printf 'GET / HTTP/1.11\r\nHost: x\r\n\r\n' | nc localhost 8080` (malformed version) → Expect 400.
- **R28-leading-empty-line-tolerated** `bonus` (request-line) — Section 2.2: 'a server that is expecting to receive and parse a request-line SHOULD ignore at least one empty line (CRLF) received prior to the request-line.' A single leading CRLF before the request should be skipped.
  - _Test :_ Send `printf '\r\nGET / HTTP/1.1\r\nHost: x\r\n\r\n' | nc localhost 8080` → Expect a normal response (200/404), i.e. the leading empty line is ignored, not treated as a malformed request-line.
- **R29-malformed-request-line-400** `rfc-MUST` (request-line) — Section 2.2: recipients must reject invalid message framing. A start-line containing control characters, an empty request-target, or otherwise unparseable syntax is invalid → 400.
  - _Test :_ Send garbage first line `printf 'GET  HTTP/1.1\r\nHost: x\r\n\r\n' | nc localhost 8080` (empty request-target) → Expect 400. Also send `printf '\x16\x03garbage\r\n\r\n' | nc localhost 8080` → Expect 400 (or immediate close), never a 200.
- **R30-incomplete-body-not-hang** `rfc-SHOULD` (body-length) — Section 8: an incomplete message (fewer body octets than declared Content-Length, or a truncated chunked stream) is treated as a failure. A server should apply a read timeout and respond 408 (Request Timeout) / close rather than block forever.
  - _Test :_ Send a declared-but-short body and hold the socket open: `printf 'POST / HTTP/1.1\r\nHost: x\r\nContent-Length: 100\r\n\r\nonly-a-few-bytes' | nc localhost 8080` → Expect the server to eventually respond 408 or close the connection, NOT hang indefinitely.

### 9.3 RFC 9110 — Sémantique HTTP (33)

- **R-METHOD-GET** `mandatory` (method-semantics) — GET requests transfer of a current representation of the target resource; a successful GET returns 200 (OK) with the representation. GET is safe and idempotent (no intended side effects on the origin server).
  - _Test :_ GET /index.html HTTP/1.1 with Host header, file exists & readable -> 200 OK; body equals file bytes; Content-Type and Content-Length present. Repeating the request leaves server state unchanged.
- **R-METHOD-HEAD** `rfc-MUST` (method-semantics) — HEAD is identical to GET except the server MUST NOT send message content in the response; the header fields SHOULD be identical to those of an equivalent GET (including the Content-Length that GET would have produced).
  - _Test :_ HEAD /index.html HTTP/1.1 -> 200 OK with the same headers as the equivalent GET (e.g., Content-Length reflecting the would-be body size) but a zero-length response body.
- **R-METHOD-POST** `mandatory` (method-semantics) — POST requests that the target resource process the enclosed representation according to the resource's own semantics. Success is 200/204 when processed, or 201 when one or more new resources are created. POST is neither safe nor idempotent.
  - _Test :_ POST /upload with a body that creates a resource -> 201 Created (with Location); otherwise 200 or 204. Two identical POSTs may produce two distinct results/resources.
- **R-METHOD-PUT** `bonus` (method-semantics) — PUT requests that the state of the target resource be created or replaced with the enclosed representation. 201 (Created) if a new resource is created at the target URI, 200/204 if an existing resource is replaced. PUT is idempotent but not safe.
  - _Test :_ PUT /new.txt (target absent) -> 201 Created + Location. Sending the identical PUT again -> 200 or 204 with an identical final resource state (idempotent).
- **R-METHOD-DELETE** `mandatory` (method-semantics) — DELETE requests deletion of the target resource. A successful DELETE returns 204 (No Content, no body) or 200 (with a representation describing the status). DELETE is idempotent but not safe.
  - _Test :_ DELETE /file.txt (exists) -> 200 or 204 and the file is removed; a subsequent GET /file.txt -> 404.
- **R-SAFE-GET-HEAD** `mandatory` (safe-idempotent) — GET and HEAD are safe (SHOULD NOT cause side effects / state change on the server) and idempotent.
  - _Test :_ Issuing GET or HEAD N>1 times against a static resource leaves the resource and server-side state unchanged; each returns the same status.
- **R-IDEMPOTENT-PUT-DELETE** `mandatory` (safe-idempotent) — PUT and DELETE are idempotent: the intended effect of multiple identical requests equals that of a single request, even though returned status codes may differ.
  - _Test :_ DELETE /x followed by a second DELETE /x -> resource absent after both; the server does not error abnormally; the second request may return 404 (still idempotent in effect).
- **R-NONIDEMPOTENT-POST** `mandatory` (safe-idempotent) — POST is neither safe nor idempotent; clients must not assume automatic replay-safety.
  - _Test :_ Two identical POST /upload requests may create two distinct resources / two distinct results.
- **R-200** `mandatory` (status-codes) — 200 (OK) indicates the request succeeded; the response payload reflects the result of the method's semantics.
  - _Test :_ GET /index.html (exists, readable) -> 200 OK with the representation in the body.
- **R-201** `rfc-SHOULD` (status-codes) — 201 (Created) indicates the request succeeded and one or more new resources were created. The primary new resource is identified by a Location header field or, if none is sent, by the target URI.
  - _Test :_ POST /upload that creates a resource -> 201 Created; when the new resource's URI differs from the request target, a Location header points to it.
- **R-204** `rfc-MUST` (status-codes) — 204 (No Content) indicates success with no representation data to return; the response carries no message body.
  - _Test :_ DELETE /file (success) or a successful update -> 204 No Content with an empty body (no body bytes).
- **R-301** `rfc-MUST` (status-codes) — 301 (Moved Permanently) indicates the target resource has been assigned a new permanent URI; the response includes a Location header with the new URI.
  - _Test :_ GET /old (configured as a permanent redirect) -> 301 + Location: /new (or an absolute URI).
- **R-302** `rfc-MUST` (status-codes) — 302 (Found) indicates the target resource resides temporarily under a different URI given by the Location header field.
  - _Test :_ GET /temp (configured temporary redirect) -> 302 + Location header pointing to the temporary URI.
- **R-304** `rfc-MUST` (status-codes) — 304 (Not Modified) is sent for a conditional GET/HEAD whose precondition (If-None-Match / If-Modified-Since) indicates the cached representation is still current; it MUST NOT include message body content.
  - _Test :_ GET /x with If-Modified-Since >= the resource's Last-Modified -> 304 Not Modified with an empty body.
- **R-400** `rfc-MUST` (status-codes) — 400 (Bad Request) is sent when the server cannot or will not process the request due to a client error such as malformed syntax or invalid request framing.
  - _Test :_ A malformed request line, a bad/garbled header, or an HTTP/1.1 request missing or duplicating the Host field -> 400 Bad Request.
- **R-403** `mandatory` (status-codes) — 403 (Forbidden) indicates the server understood the request but refuses to authorize it (e.g., filesystem permission denied, autoindex disabled on a directory).
  - _Test :_ GET /secret where read access is forbidden by configuration/permissions -> 403 Forbidden.
- **R-404** `mandatory` (status-codes) — 404 (Not Found) indicates the origin server did not find a current representation for the target resource (or is unwilling to disclose one).
  - _Test :_ GET /does-not-exist -> 404 Not Found.
- **R-405** `rfc-MUST` (status-codes) — 405 (Method Not Allowed) is sent when the server knows the request method but it is not supported by the target resource; the server MUST generate an Allow header field listing the target resource's currently supported methods.
  - _Test :_ POST /static.html on a route that permits only GET/HEAD -> 405 Method Not Allowed and an Allow: GET, HEAD header enumerating the permitted methods.
- **R-411** `rfc-MUST` (status-codes) — 411 (Length Required) is sent when the server refuses to accept a request that lacks a defined Content-Length and whose length is not otherwise determined (e.g., no Transfer-Encoding).
  - _Test :_ POST /upload carrying a body but with no Content-Length and no Transfer-Encoding, where the server requires a declared length -> 411 Length Required.
- **R-413** `rfc-MUST` (status-codes) — 413 (Content Too Large) is sent when the request content is larger than the server is willing or able to process (e.g., exceeds the configured client_max_body_size).
  - _Test :_ POST /upload with a body larger than the configured body-size limit -> 413 Content Too Large.
- **R-414** `rfc-MUST` (status-codes) — 414 (URI Too Long) is sent when the request-target URI is longer than the server is willing to interpret.
  - _Test :_ GET with a request-target/path exceeding the server's URI length limit -> 414 URI Too Long.
- **R-500** `mandatory` (status-codes) — 500 (Internal Server Error) indicates the server encountered an unexpected condition that prevented it from fulfilling the request (e.g., a CGI process crashing).
  - _Test :_ A request that triggers a server-side failure (e.g., a CGI script that errors/crashes) -> 500 Internal Server Error, with the connection still correctly framed.
- **R-501** `rfc-MUST` (status-codes) — 501 (Not Implemented) is sent when the server does not support the functionality required to fulfill the request — in particular, a request method that the server does not recognize or implement at all. This is distinct from 405 (a known method disallowed on a specific route).
  - _Test :_ BREW /path HTTP/1.1 (an unrecognized/unimplemented method) -> 501 Not Implemented, whereas a recognized-but-disallowed method on a route -> 405 with Allow.
- **R-505** `rfc-MUST` (version-handling) — 505 (HTTP Version Not Supported) is sent when the server does not support, or refuses to support, the major HTTP version used in the request message.
  - _Test :_ A request line advertising an unsupported major version (e.g., GET / HTTP/2.0 delivered over an HTTP/1.x connection) -> 505 HTTP Version Not Supported.
- **R-VERSION-MINOR** `mandatory` (version-handling) — A server processes a request whose HTTP major version it supports regardless of minor version; only an unsupported major version triggers 505.
  - _Test :_ GET / HTTP/1.0 and GET / HTTP/1.1 -> processed normally (e.g., 200); a request labelled with an unsupported major version -> 505.
- **R-HOST** `rfc-MUST` (request-parsing) — An HTTP/1.1 request MUST include exactly one Host header field; a server MUST respond with 400 (Bad Request) to any HTTP/1.1 request that lacks a Host field or contains more than one (RFC 9112 supplement to RFC 9110 host semantics).
  - _Test :_ GET / HTTP/1.1 with no Host header, or with two Host headers -> 400 Bad Request.
- **R-DATE** `rfc-MUST` (response-headers) — An origin server with a clock MUST generate a Date header field in all 2xx, 3xx, and 4xx responses (MAY in 1xx and 5xx). A server without a reasonably accurate clock MUST NOT send Date. The value uses IMF-fixdate (GMT).
  - _Test :_ Any 200/301/404 response -> includes a Date header formatted as an IMF-fixdate in GMT (e.g., 'Sun, 06 Nov 1994 08:49:37 GMT').
- **R-FRAMING** `rfc-MUST` (response-headers) — A response body must be framed by exactly one mechanism: a Content-Length header field OR Transfer-Encoding (chunked). A message MUST NOT include both a Content-Length and a Transfer-Encoding, and when present Content-Length must equal the actual body length.
  - _Test :_ GET /file -> response has Content-Length equal to the body byte count and no Transfer-Encoding; a chunked response has Transfer-Encoding: chunked and no Content-Length; no response sends both.
- **R-CONTENT-TYPE** `rfc-SHOULD` (response-headers) — A response containing representation data SHOULD include a Content-Type header field naming the media type; recipients treat a missing Content-Type as application/octet-stream.
  - _Test :_ GET /page.html -> Content-Type: text/html; GET /img.png -> Content-Type: image/png (type matching the served representation).
- **R-ALLOW** `rfc-MUST` (response-headers) — The Allow header field lists the set of methods supported by the target resource and MUST be generated on every 405 (Method Not Allowed) response (an empty value indicates no methods are currently allowed).
  - _Test :_ Any 405 response -> contains an Allow header enumerating exactly the methods permitted on that route (e.g., Allow: GET, HEAD, DELETE).
- **R-LOCATION-3XX** `rfc-MUST` (response-headers) — Redirect responses (301, 302, and other Location-bearing 3xx) include a Location header field giving the target URI of the redirect.
  - _Test :_ A 301 or 302 response -> contains a Location header holding a valid URI reference to the redirect target.
- **R-ORIGIN-FORM** `mandatory` (request-target) — For requests sent directly to an origin server, the request target is in origin-form: an absolute-path optionally followed by '?' and a query; the server resolves it against the Host header to identify the target resource.
  - _Test :_ GET /dir/file?x=1 HTTP/1.1 with Host: example -> resolved to that path and query, returning 200 for an existing resource.
- **R-ABSOLUTE-FORM** `rfc-MUST` (request-target) — A server MUST accept the absolute-form (absolute-URI) in a request target even though clients normally send it only to a proxy; when present, the request-target authority takes precedence over any Host header field.
  - _Test :_ GET http://host/path HTTP/1.1 with Host: host -> accepted and resolved to /path (returns a normal response, not 400).

### 9.4 RFC 3875 — CGI/1.1 (27)

- **CGI-META-REQUEST_METHOD** `rfc-MUST` (meta-variables) — The server MUST set REQUEST_METHOD to the HTTP method of the request (e.g. GET, POST, HEAD). The value is case-sensitive and passed verbatim.
  - _Test :_ Run a CGI that prints its environment. Send a POST request; REQUEST_METHOD in the CGI env equals "POST" (exact case). Send GET; it equals "GET".
- **CGI-META-QUERY_STRING** `rfc-MUST` (meta-variables) — The server MUST set QUERY_STRING to the URL query component (the part after '?'), URL-encoded and NOT decoded by the server. If the request URI has no query component, QUERY_STRING MUST be the empty string (still set).
  - _Test :_ Request /cgi/env.py?a=1&b=hello%20world -> CGI env QUERY_STRING == "a=1&b=hello%20world" (percent-encoding preserved, not decoded). Request /cgi/env.py with no '?' -> QUERY_STRING is present and equals "".
- **CGI-META-CONTENT_LENGTH** `rfc-MUST` (meta-variables) — The server MUST set CONTENT_LENGTH if and only if a request message-body is present, and its value MUST reflect the length of the body AFTER the server has removed any transfer-codings or content-codings. With no body it is NULL/unset.
  - _Test :_ POST a body of exactly 11 bytes -> CGI env CONTENT_LENGTH == "11". GET with no body -> CONTENT_LENGTH is empty/unset (not "0" required, must not falsely claim a body).
- **CGI-META-CONTENT_TYPE** `rfc-MUST` (meta-variables) — The server MUST set CONTENT_TYPE when the client request includes a Content-Type header field, copying its value (media-type plus any parameters). If a body is present but no Content-Type header, the server MAY guess or SHOULD omit the variable.
  - _Test :_ POST with header 'Content-Type: application/x-www-form-urlencoded' -> CGI env CONTENT_TYPE == "application/x-www-form-urlencoded". Include a charset param and confirm it is preserved.
- **CGI-META-PATH_INFO** `rfc-SHOULD` (meta-variables) — When the request URI contains extra path segments after the script identifier, the server SHOULD set PATH_INFO to that trailing path. PATH_INFO is URL-DECODED (not percent-encoded), must not contain path parameters, and the server MUST preserve case.
  - _Test :_ Request /cgi/script.py/extra/path.txt -> CGI env PATH_INFO == "/extra/path.txt". A percent-encoded segment like /cgi/script.py/a%2Fb decodes in PATH_INFO; case is preserved (Extra != extra).
- **CGI-META-PATH_TRANSLATED** `rfc-SHOULD` (meta-variables) — The server SHOULD set PATH_TRANSLATED to the PATH_INFO value mapped (virtual-to-physical) onto the server's document tree. If PATH_INFO is NULL, PATH_TRANSLATED MUST be NULL/unset.
  - _Test :_ With document root /var/www and request /cgi/script.py/extra -> PATH_TRANSLATED ends with the physical path for /extra (e.g. /var/www/extra). With no path-info, PATH_TRANSLATED is empty/unset.
- **CGI-META-SCRIPT_NAME** `rfc-MUST` (meta-variables) — The server MUST set SCRIPT_NAME to the URI path (not URL-encoded) that identifies the CGI script, i.e. the leading portion of the path up to but excluding PATH_INFO.
  - _Test :_ Request /cgi/script.py/extra?x=1 -> CGI env SCRIPT_NAME == "/cgi/script.py" (does not include /extra and does not include the query).
- **CGI-META-SERVER_PROTOCOL** `rfc-MUST` (meta-variables) — The server MUST set SERVER_PROTOCOL to the name/version of the protocol used for the request, e.g. "HTTP/1.1".
  - _Test :_ Send an HTTP/1.1 request -> CGI env SERVER_PROTOCOL == "HTTP/1.1".
- **CGI-META-SERVER_NAME** `rfc-MUST` (meta-variables) — The server MUST set SERVER_NAME to the host name or address to which the client request is directed (typically derived from the Host header or configured server_name).
  - _Test :_ Request with 'Host: example.test' to a matching server block -> CGI env SERVER_NAME == "example.test" (or the configured server name / bound address), and is non-empty.
- **CGI-META-SERVER_PORT** `rfc-MUST` (meta-variables) — The server MUST set SERVER_PORT to the TCP port on which the request was received, even when it is the default port.
  - _Test :_ Connect to the server on port 8080 -> CGI env SERVER_PORT == "8080". Even on port 80 the variable is present and equals "80".
- **CGI-META-GATEWAY_INTERFACE** `rfc-MUST` (meta-variables) — The server MUST set GATEWAY_INTERFACE to the CGI dialect, formatted "CGI/1.1".
  - _Test :_ Any CGI invocation -> CGI env GATEWAY_INTERFACE == "CGI/1.1".
- **CGI-META-REMOTE_ADDR** `rfc-MUST` (meta-variables) — The server MUST set REMOTE_ADDR to the network (IP) address of the client sending the request.
  - _Test :_ Connect from 127.0.0.1 -> CGI env REMOTE_ADDR == "127.0.0.1" (matches the connecting peer address).
- **CGI-META-HTTP_STAR** `rfc-MUST` (meta-variables) — For each client request header field, the server sets an HTTP_* meta-variable: the field name is upper-cased, '-' replaced with '_', and prefixed with 'HTTP_'. Multiple headers of the same name MUST be merged into one comma-separated value. The server SHOULD exclude Authorization and headers already exposed as their own variables (Content-Length -> CONTENT_LENGTH, Content-Type -> CONTENT_TYPE).
  - _Test :_ Send 'User-Agent: probe' and 'X-Test-Header: v' -> CGI env has HTTP_USER_AGENT == "probe" and HTTP_X_TEST_HEADER == "v". Content-Length header does NOT appear as HTTP_CONTENT_LENGTH (only as CONTENT_LENGTH). Sending the same header name twice yields a single merged value.
- **CGI-META-NAMING** `rfc-MUST` (meta-variables) — Meta-variable names are case-insensitive with no two differing only in case; a zero-length (NULL) value is not distinguished from a missing value (an optional variable may be omitted when NULL).
  - _Test :_ The environment passed to the CGI contains each variable name exactly once, in the canonical upper-case spelling used by the RFC (e.g. exactly one QUERY_STRING entry).
- **CGI-BODY-STDIN** `rfc-MUST` (request-body) — The request message-body MUST be delivered to the CGI script on its standard input (fd 0). The server MUST make at least CONTENT_LENGTH bytes available to read.
  - _Test :_ POST a known body to a CGI that echoes stdin -> the CGI receives exactly the request body bytes on stdin, and the echoed bytes match what was sent.
- **CGI-BODY-LENGTH-BOUND** `rfc-MUST` (request-body) — The boundary of readable request data is CONTENT_LENGTH: the script must not read more than CONTENT_LENGTH bytes. The server therefore must delimit the body it feeds to CGI at exactly CONTENT_LENGTH and not leak pipelined/extra bytes into the script's stdin.
  - _Test :_ POST body of N bytes immediately followed (same connection) by another pipelined request; the CGI reading CONTENT_LENGTH bytes receives exactly the N body bytes and not the following request's bytes.
- **CGI-BODY-UNCHUNK** `rfc-MUST` (request-body) — A chunked (Transfer-Encoding: chunked) request body MUST be de-chunked by the server before invoking the CGI: the script sees the decoded body on stdin and a CONTENT_LENGTH equal to the decoded length. The script is never exposed to chunk framing, and Transfer-Encoding is not passed as a length source.
  - _Test :_ Send a POST with 'Transfer-Encoding: chunked' whose decoded body is 'hello' (5 bytes) split across chunks -> the CGI's stdin contains exactly 'hello' with no chunk-size lines, and CONTENT_LENGTH == "5".
- **CGI-RESP-HEADER-BLOCK** `rfc-MUST` (response-parsing) — CGI output comprises a header block and an optional body separated by a single blank line. The server MUST parse the CGI-emitted header fields up to the first blank line and generate a client HTTP response from them (i.e. it does not pass the raw script header lines through verbatim for parsed-header CGI).
  - _Test :_ A CGI that prints 'Content-Type: text/plain' + CRLF + CRLF + 'body' -> the client receives a well-formed HTTP response whose body is 'body' and whose headers include a status line and Content-Type; the literal 'Content-Type:' line is not duplicated in the body.
- **CGI-RESP-CONTENT-TYPE** `rfc-MUST` (response-parsing) — For a document response, the script MUST supply a Content-Type field; the server forwards it to the client. The server passes through the script-provided Content-Type value.
  - _Test :_ A CGI emitting 'Content-Type: application/json' -> the client HTTP response carries 'Content-Type: application/json'.
- **CGI-RESP-NO-CONTENT-TYPE** `rfc-SHOULD` (response-parsing) — If a script returns a body without a Content-Type, the server SHOULD NOT attempt to determine the content type. In practice the server must handle this gracefully (either forward with no/omitted Content-Type or return a controlled error) and MUST NOT crash or hang.
  - _Test :_ A CGI that prints only 'Status: 200 OK' + blank line + body (no Content-Type) -> the server returns a valid HTTP response and does not crash/hang; it does not fabricate an incorrect content type by guessing.
- **CGI-RESP-STATUS** `rfc-MUST` (response-parsing) — The server MUST honor a 'Status:' header line from the CGI, formatted 'Status: <code> SP <reason-phrase>', using that code/reason as the client HTTP response status. If no Status is provided for a document response, the default is 200 OK.
  - _Test :_ A CGI emitting 'Status: 404 Not Found' -> the client receives HTTP status 404. A CGI emitting no Status with a Content-Type + body -> the client receives HTTP status 200. The 'Status:' line itself is consumed, not forwarded as a client header.
- **CGI-RESP-LOCAL-REDIRECT** `rfc-MUST` (response-parsing) — A local redirect response is a CGI 'Location:' header whose value is an absolute path (local-pathquery), with no other headers and no body. The server MUST reprocess the request internally against that local path and return the resulting response (not a 3xx to the client).
  - _Test :_ A CGI emitting only 'Location: /index.html' + blank line -> the client receives the contents of /index.html (e.g. HTTP 200 with that document), not a 302 redirect.
- **CGI-RESP-CLIENT-REDIRECT** `rfc-MUST` (response-parsing) — A client redirect response is a CGI 'Location:' header whose value is an absolute URI. For an HTTP client the server MUST generate a 302 'Found' response with that Location.
  - _Test :_ A CGI emitting 'Location: http://example.com/foo' + blank line -> the client receives HTTP status 302 with 'Location: http://example.com/foo'.
- **CGI-RESP-CLIENT-REDIRECT-DOC** `rfc-MUST` (response-parsing) — A client redirect with document combines an absolute-URI Location, an explicit Status (302 or extension code), a Content-Type, and a body. The server forwards the redirect status, Location, and the accompanying document to the client.
  - _Test :_ A CGI emitting 'Location: http://example.com/', 'Status: 302 Found', 'Content-Type: text/html', blank line, then HTML -> client receives HTTP 302 with that Location and the HTML body.
- **CGI-RESP-FORWARD-HEADERS** `rfc-MUST` (response-parsing) — The server MUST forward other (extension/protocol) header fields the script emits to the client, translating CGI header syntax to HTTP syntax where they differ.
  - _Test :_ A CGI emitting 'Content-Type: text/plain' plus 'X-Custom: yes' -> the client HTTP response includes 'X-Custom: yes'.
- **CGI-ERR-TIMEOUT-NOHANG** `mandatory` (error-handling) — The server may interrupt or terminate script execution at any time. A CGI that never terminates (infinite loop / never closes output) MUST NOT hang the server: it must be bounded by a timeout, after which the server terminates the script and returns an error status to the client (commonly 504 Gateway Timeout, or 500/502).
  - _Test :_ Request a CGI that sleeps forever / infinite-loops -> within a bounded time the server responds with a 5xx error (e.g. 504 or 502) and remains responsive to concurrent/subsequent requests (it does not deadlock).
- **CGI-ERR-CRASH** `mandatory` (error-handling) — If the CGI process crashes, exits abnormally (non-zero / killed by signal), or produces malformed/no header output, the server MUST return a controlled HTTP error response (typically 502 Bad Gateway for invalid gateway output, or 500 Internal Server Error) rather than crashing, leaking raw output, or leaving the connection hung.
  - _Test :_ Request a CGI that exits immediately with no output, and one that segfaults -> the client receives a 5xx status (500 or 502) with a valid HTTP response, and the server continues serving other requests.


---

_Plan généré à partir d'une analyse multi-agents (21 agents : lecture sujet+RFC+code+tests → 8 sections → critique de complétude). Les cas de test sont des points de départ à adapter aux ports/chemins de vos configs._
