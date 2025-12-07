# NDS .sav Conversion and Transfer Tool
## Project Goal
Appease my laziness by creating a tool that leverages user level daemon (using systemctl services) + C program to create automatic transfer and conversion of .sav file from modded 3DS to PC. I explored the opensource project 'MelonDS' to understand how they convert to .sav file from other save file types.
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
## Future Improvements
- Add batch conversion/transfers.
- Handle 3DS .sav file conversion/transfers.
- (not included in codebase) Optimise systemctl service script
