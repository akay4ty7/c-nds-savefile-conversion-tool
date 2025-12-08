# NDS .sav Conversion and Transfer Tool
![project8](https://github.com/user-attachments/assets/670ed191-0be6-4ad1-9d9b-be42f920b417)
## Project Goal
This project was born out of a curiosity of the open source project called 'MelonDs and also from my laziness to avoid manual dragging and dropping files. I created a tool that leverages user level daemon (systemctl services) + C programming to create an automatic transfer and conversion of .sav file from modded 3DS to PC. I explored the opensource project 'MelonDS' to understand how they convert to .sav file from other save file types. This meant I had to trawl through a large code base with 0 clue where things were. Despite this, I was able to trace the from the interface all the way back to the conversion code. Based on what I've gathered, the conversion basically traces through a large lookup table based on the NDS's game code and finds 1 of 11 size types (e.g. size type 1 is 512bytes). The size type indicates how many bytes to read and write from the beginning of the original .sav into a the new .sav file. If you are using the melonds emulator, you don't need to actually convert if you are transferring .sav nds to .sav melonds as the emulator handles this (still an option if desired).
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
