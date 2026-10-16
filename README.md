# CS375 Lab 7 – Thread-Safe Data Structures

C++ implementations of thread-safe data structures using `std::mutex`, `std::lock_guard`
and `std::condition_variable`, plus a LaTeX report.

## 📁 Folder layout

```
CS375-Lab-7-Thread-Safe-DS/
├── src/                              ← ✅ SUBMIT THESE 5 FILES (individually, not zipped)
│   ├── thread_safe_data_structures.cpp   Exercise 1 – Queue + Stack, Producer-Consumer, Undo-Redo
│   ├── priority_queue.cpp                Exercise 3 – Thread-safe priority queue
│   ├── circular_buffer.cpp               Exercise 4 – Bounded circular buffer (condition variables)
│   ├── thread_safe_deque.cpp             Exercise 5 – Double-ended queue
│   └── thread_safe_linked_list.cpp       Exercise 6 – Singly linked list
├── report/
│   ├── report.tex                    LaTeX source (code is pulled in automatically from src/)
│   ├── report.pdf                    ← ✅ SUBMIT THIS (after adding screenshots)
│   └── screenshots/                  ← put your PNG screenshots here
├── Makefile                          shortcuts: make / make run / make report / make clean
└── README.md
```

## ✅ What to submit on Canvas

| File | Location |
|------|----------|
| `thread_safe_data_structures.cpp` | `src/` |
| `priority_queue.cpp` | `src/` |
| `circular_buffer.cpp` | `src/` |
| `thread_safe_deque.cpp` | `src/` |
| `thread_safe_linked_list.cpp` | `src/` |
| `report.pdf` | `report/` |

## 🔧 Setup (Ubuntu / WSL / Google Cloud VM)

```bash
sudo apt update && sudo apt install build-essential
sudo apt install texlive-full          # only needed to build the report (large; texlive-latex-extra also works)
```

## ▶️ Compile and run

Each program is compiled exactly as the assignment specifies:

```bash
cd src
g++ -o thread_safe_data_structures thread_safe_data_structures.cpp -pthread -std=c++11
./thread_safe_data_structures
```

Or build/run everything at once from the repo root:

```bash
make          # builds all 5 programs into bin/
make run      # builds and runs all 5
```

## 📸 Screenshots

Screenshots are in `report/screenshots/`, and `report.tex` uses these file names:

| Exercise | File name | Program |
|----------|-----------|---------|
| 1 | `Lab7exercise1_screenshot.png` | `thread_safe_data_structures` |
| 3 | `Lab7exercise3.png` | `priority_queue` |
| 4 | `Lab7exercise4_screenshot.png` | `circular_buffer` |
| 5 | `Lab7exercise5_screenshot.png` | `thread_safe_deque` |
| 6 | `Lab7exercise6_screenshot.png` | `thread_safe_linked_list` |

If you replace a screenshot, keep the same file name (or update the matching `\screenshot{...}`
line in `report.tex`). A missing file shows up as a "Screenshot missing" box, so the report always compiles.

Each screenshot should show **the `g++` command, the `./program` command, and the output**.

## 📄 Build the report

```bash
cd report
latexmk -pdf report.tex
```

Before submitting, replace `\author{Your Name}` in `report/report.tex` with your name.

## Notes on changes to the starter code

- Added the missing `#include <iostream>`, `<atomic>` and `<random>`.
- **Exercise 1:** the consumer reads the "all produced" counter *before* trying to pop. Otherwise a
  consumer could exit while the last message is still in the queue.
- **Exercise 3:** `rand()` was replaced with a per-thread `std::mt19937`, because `rand()` is not
  thread-safe. The popper now runs until all 20 items are removed.
- **Exercise 4:** `pop()` now **waits** on `not_empty`. The original version returned `false`, so
  items could be left behind and the program could hang. The test now uses 2 producers and
  2 consumers. A non-blocking `try_pop()` is also provided.
- **Exercise 6:** copying the list is disabled (it would double-delete nodes), and `delete`
  happens outside the lock.
- All programs use a `coutMutex` so output lines from different threads don't get mixed together.
