# EXE Bitness in Wine vs Native Windows

## Scope

This document summarizes what is reliable and unreliable when detecting Windows executable bitness in Wine-based environments, with direct comparison to native Windows behavior.

## High-Level Summary

- Native Windows usually provides stable behavior across WSH/COM and process I/O APIs.
- Wine can differ in COM coverage, process pipe behavior, and console/output plumbing.
- For bitness detection, native WinAPI file parsing approaches are consistently safer than script-hosted COM binary read paths.

## Behavior Comparison

| Capability | Native Windows (typical) | Wine (observed) | Impact | Reliable Approach |
|---|---|---|---|---|
| `ADODB.Stream.LoadFromFile` for binary reads | Usually works | Can fail even when object creation works | File load step fails unexpectedly | Avoid as primary path |
| `MSXML2.XMLHTTP` binary `responseBody` | Often usable as byte array | May not be VBArray-compatible | Binary parsing breaks | Avoid for local binary parsing |
| `XMLHTTP responseText` fallback | Returns text | Returns text | Not byte-accurate for PE offsets | Do not use for PE header parsing |
| WSH named arg APIs | Usually stable | Can throw in some prefixes | Script startup/flag parsing issues | Defensive `try/catch` if used |
| `WScript.Shell.Exec` stdout/stderr pipes | Usually usable | Can be partially unimplemented (`E_NOTIMPL`) | Child output capture fails | Prefer exit codes / file handoff |
| Console stdout/stderr visibility | Generally predictable | Can vary by prefix/runtime/shim | Program appears "blank" | Use robust output fallbacks |

## What Works Reliably in Wine

- Native WinAPI-based PE parsing (read file bytes, parse `MZ`, `e_lfanew`, `PE\0\0`, optional header magic).
- `GetBinaryTypeW` as a quick first-pass detector.
- x86-targeted tool binaries for broad prefix compatibility.
- Minimal runtime dependencies and static/minimized linking where possible.
- Data exchange via exit codes or temporary files instead of stdout parsing.

## What Is Fragile in Wine

- COM-based binary data paths originally designed for script-host convenience.
- Assuming stdout is clean text in all environments (shims can inject extra lines).
- Assuming `Exec` stream reads are fully implemented in all Wine builds.
- Assuming behavior parity with native Windows for every WSH/COM component.

## Common Symptoms and Likely Causes

### `HRESULT 0x800A1395 (VBArray object expected)`

- Likely cause: binary body not exposed as expected array type.
- Consequence: byte extraction path fails.
- Best action: stop using that path for binary file parsing.

### `HRESULT 0x80004001 (E_NOTIMPL)` while reading process output

- Likely cause: process stream methods not fully implemented.
- Consequence: `Exec` output capture fails despite process launch.
- Best action: use process exit code contract or file-based output channel.

### Valid EXE reported as unknown / missing PE signature

- Likely cause: text-decoded fallback (`responseText`) used as if binary.
- Consequence: incorrect offsets and false negatives.
- Best action: only parse raw bytes from filesystem APIs.

### Program window opens but shows no text

- Likely cause: runtime/output interaction mismatch in specific Wine stack.
- Consequence: appears to do nothing.
- Best action: simplify runtime dependencies and prefer direct WinAPI writes/fallbacks.

## Practical Guidance

1. Treat Wine as a compatibility layer with partial differences, not a perfect clone of native Windows behavior.
2. Keep bitness detection logic close to the PE format and WinAPI file access.
3. Use script-hosted COM paths only as optional conveniences, not core detection mechanisms.
4. Favor deterministic contracts (exit code or temp file) over parsing command output text.
5. Prioritize resilience over cleverness for tooling expected to run across many Wine prefixes.

## TL;DR

Avoid JScript, VBScript, and Go language. Wine isn't complete enough to handle them.