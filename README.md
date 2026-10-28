# hedgemon 🦔

```
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣤⣀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠻⠿⠿⠿⠀⠉⠛⠻⢷⠂⠻⣷⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⢀⣤⡶⠟⠃⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠛⠃⠺⣧⡀⠀⠀⠀⠀⠀⠀
⠀⠀⠀⢠⡄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠙⢿⣆⠀⠀⠀⠀⠀
⠀⠀⣰⡟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⣿⣿⣷⣀⣴⣾⣿⡄⠀⠀⠀⠀
⠀⠰⠟⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⢿⣿⣿⡟⠉⠙⢿⣿⣄⠀⠀⠀
⠀⠀⢸⣇⠀⠀⠀⢀⣀⣀⣀⣀⣀⣀⣀⣠⣤⣴⣿⣿⣿⣿⣶⣴⣿⣿⣿⣃⣀⠀
⠀⠀⠀⢿⣀⣴⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡿⠟⠃⠀
⠀⠀⠀⠘⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠛⠉⠉⠁⠀⠀⠀⠀
⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀
```

> A homemade C system monitor, because `htop` already exists but where would the fun be in that c:

---

## What is it

`hedgemon` runs straight in your terminal and gives you real-time metrics:

- Overall CPU usage of the machine
- RAM and swap usage
- Top resource-hungry processes (sortable by CPU or RAM)
- Active TCP network connections, cleanly grouped by state (`LISTEN` / `ESTABLISHED` / `TIME_WAIT`)

Everything is read directly from `/proc`: zero external libraries, just C and a bit of patience parsing kernel text files >;]

## Why

A personal project to level up in C and systems/networking, with the perfect excuse to build an ASCII hedgehog watching over your PC. Zero regrets.

---

## Build

No external dependencies, just `gcc` and standard C. Compiles out of the box on any Linux distro.

```bash
make          # build
make clean    # remove .o files
make fclean   # remove .o files AND the executable
make re       # fclean + full rebuild
```

## Run

```bash
./hedgemon
```

### Options

| Option   | Values        | Default | Effect                               |
|----------|---------------|---------|--------------------------------------|
| `--sort` | `cpu` / `ram` | `ram`   | Sorts top processes by CPU or RAM    |
| `--top`  | an integer    | `10`    | Number of processes displayed        |

### Examples

```bash
./hedgemon --sort cpu
./hedgemon --sort cpu --top 20
```

`Ctrl+C` to quit, like any civilized tool.

---

## How it actually works

No magic library. Everything is read directly from virtual files exposed by the Linux kernel:

| File                                  | Used for                                                                 |
|---------------------------------------|--------------------------------------------------------------------------|
| `/proc/meminfo`                       | RAM and swap                                                             |
| `/proc/stat`                          | Overall CPU usage (delta between two reads, 1 second apart)              |
| `/proc/[pid]/status` and `/proc/[pid]/stat` | Info and CPU consumption per process                               |
| `/proc/net/tcp`                       | Active network connections (hex parsing, easily the least friendly format of the bunch) |

### Code structure

The codebase is split by responsibility:

```
src/
├── main.c    → main loop, argument parsing
├── mem.c/h   → memory
├── cpu.c/h   → overall CPU + per-process CPU
├── proc.c/h  → process list
├── net.c/h   → network connections
└── ui.c/h    → window-framed UI display
```

---

## Roadmap

Things that might land if hedgehog motivation stays strong:

- [ ] UDP support alongside TCP
- [ ] Map each network connection to its process (like `ss -p`)
- [ ] Real interactive keyboard navigation (scroll, select) instead of a simple top N
- [ ] Sum child process consumption (shoutout to Firefox and its 15 isolated processes)

---

## Mindset

This project is mainly meant for learning properly: reading system files, low-level parsing, process management, a bit of networking, and clean C practices along the way (no buffer overflows, checking return values, modular design).

If you dig into the code and find something messy, **THAT IS IMPOSSIBLE BECAUSE I APPLIED MYSELF** (it's possible, but I did my best ok?! >;C )
