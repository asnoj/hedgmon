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

un moniteur système en C, fait maison, parce que `htop` existe déjà mais où serait le fun dans ça c:

## c'est quoi

`hedgemon` tourne direct dans ton terminal et te donne en temps réel :
- l'usage CPU global de la machine
- la RAM et le swap utilisés
- le top des process qui bouffent le plus de ressources (triable par CPU ou RAM)
- les connexions réseau TCP actives, groupées proprement par état (LISTEN / ESTABLISHED / TIME_WAIT)

tout ça lu directement depuis `/proc`, sans libs externes, juste du C et un peu de patience à parser des fichiers texte du kernel >;]

## pourquoi

projet perso pour progresser en C et en système/réseau, avec l'excuse parfaite de se faire un hérisson en ASCII art qui surveille ton pc. zero regret.

## compiler

```bash
make
```

et pour tout nettoyer / repartir de zéro :

```bash
make clean   # vire les .o
make fclean  # vire les .o ET l'exécutable
make re      # fclean + rebuild complet
```

aucune dépendance externe, juste `gcc` et une base de C standard. ça devrait compiler tel quel sur n'importe quelle distro Linux.

## lancer

```bash
./hedgemon
```

### options

| option | valeurs | défaut | effet |
|---|---|---|---|
| `--sort` | `cpu` / `ram` | `ram` | trie le top process par CPU ou par RAM |
| `--top` | un entier | `10` | nombre de process affichés |

exemples :

```bash
./hedgemon --sort cpu
./hedgemon --sort cpu --top 20
```

Ctrl+C pour quitter, comme n'importe quel outil civilisé.

## comment ça marche en vrai

pas de lib magique, tout est lu directement dans les fichiers virtuels que le kernel Linux expose :

- `/proc/meminfo` → RAM et swap
- `/proc/stat` → usage CPU global (calculé via un delta entre deux lectures à 1 seconde d'écart)
- `/proc/[pid]/status` et `/proc/[pid]/stat` → infos et conso CPU de chaque process
- `/proc/net/tcp` → connexions réseau actives (parsing hexadécimal, le format le moins sympa du lot)

le code est découpé par responsabilité :

```
src/
├── main.c    → boucle principale, parsing des arguments
├── mem.c/h   → mémoire
├── cpu.c/h   → CPU global + CPU par process
├── proc.c/h  → liste des process
├── net.c/h   → connexions réseau
└── ui.c/h    → affichage encadré façon fenêtre
```

## roadmap

des trucs qui pourraient arriver si la motivation hérisson reste au rendez-vous :

- [ ] support UDP en plus de TCP
- [ ] associer chaque connexion réseau à son process (comme `ss -p`)
- [ ] vraie navigation interactive au clavier (scroll, sélection) plutôt qu'un simple top N
- [ ] sommer la conso des process enfants (coucou Firefox et ses 15 `Isolated`)

## état d'esprit

ce projet sert surtout à apprendre proprement : lecture de fichiers système, parsing bas niveau, gestion de process, un peu de réseau, et les bonnes pratiques C qui vont avec (pas de buffer overflow, vérification des retours, découpage en modules). si tu fouilles le code et que tu trouves un truc crade, c IMPOSSIBLE CAR JE ME SUIS APPLIQUER (c possible mais j'ai fais de mon mieux ok?! >;C )
