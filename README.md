# NDS .sav Conversion and Transfer Tool
## Project Goal
Nintendo DS save files come in various formats depending on the hardware or emulator producing them. **MelonDS**, an open-source NDS emulator, handles conversion between these formats internally. I wanted to understand how that conversion actually works—and also automate my own workflow of transferring saves from a modded 3DS to PC (I was tired of manual drag-and-drop). The tool combines a user-level systemd daemon with a C program to automatically detect, transfer, and convert save files. To understand the conversion logic, I traced through the MelonDS codebase from the UI layer down to the actual conversion routines. The mechanism is straightforward once located: the emulator uses the NDS game code to index into a lookup table, returning one of 11 predefined size types (e.g., type 1 = 512 bytes). The size type determines how many bytes to read from the source save and write into the target format.

Worth noting: if you're transferring native NDS saves into MelonDS, the emulator handles conversion on load meaning explicit conversion is optional but available.

**Interesting Knowledge Gained:**
- Codebase navigation: Tracing functionality through an unfamiliar open-source project with no prior context.
- Save format structure: Understanding that NDS save conversion is essentially a size-mapping problem, not a data transformation.
- Systemd user services: Setting up a daemon to monitor and react to file events automatically.
- Lookup table design: Seeing how a simple table-driven approach handles variation across hundreds of game titles.
## Future Improvements
- Add batch conversion/transfers.
- Handle 3DS .sav file conversion/transfers.
- (not included in codebase) Optimise systemctl service script
## Demo
![project8](https://github.com/user-attachments/assets/670ed191-0be6-4ad1-9d9b-be42f920b417)
## Build
Before compilation, in main.c, replace the value of macro 'CONFIG_PATH' with the absolute path to your dir_config.txt.

dir_config.txt should contain something like the following:
```
NDS_SAV_DIR="/path/to/gvfsmount/roms/nds/saves/"
MEL_DIR="/path/to/melonds/.sav/data/"
NDS_DIR="/path/to/gvfsmount/roms/nds/"
```
Once you're happy with your configs run the following in the root project folder:
```
gcc -o main main.c
```
## Run
```
./main
```

