# NFR-06 — Portability

**Author:** Prathik M Nambiar
**Requirement (SRS 4.6):** the source code should compile on commonly available C/C++ compilers with minimal changes, and should avoid unnecessary platform-specific dependencies.

This branch contains only my NFR-06 work.

## What was done

| Area | Change |
|---|---|
| Source audit | Checked every file under `src/`, `include/` and `tests/`. Only standard C++17 headers are used, with no `windows.h`, `conio.h`, `unistd.h`, `system()` calls or hardcoded path separators. No source changes were needed. |
| Build system | `CMakeLists.txt` now picks warning flags per compiler: `/W4 /permissive-` on MSVC, `-Wall -Wextra -Wpedantic` on GCC/Clang. |
| Portability test | `tests/test_portability.cpp` (registered as `portability_tests`) enforces C++17 at compile time, checks 64-bit integer width and 8-bit bytes (the password digest uses `std::uint64_t`), prints the platform and compiler, and runs login checks. |
| Continuous integration | `.github/workflows/ci.yml` builds and runs all tests on Ubuntu, Windows and macOS for every push and pull request. |

## How to build and test

```sh
cmake -S . -B build
cmake --build build --config Release
ctest --test-dir build -C Release --output-on-failure
```

Expected result: `100% tests passed out of 6`, including `portability_tests`.

## Test results

| Platform | Compiler | Result |
|---|---|---|
| Linux (64-bit) | GCC 16.2.1 | 6 of 6 tests passed |
| Windows | MSVC | See the GitHub Actions run |
| macOS | Apple Clang | See the GitHub Actions run |

## Known portability issue (not fixed in this task)

`prompt()` in `src/main.cpp` reads input with `std::getline`. When input comes from a Windows-style CRLF file on Linux or macOS, a trailing `\r` stays on the string and login fails. Interactive typing is not affected. This is in another team member's file, so it is reported here and not changed.
