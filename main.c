#include "rom_data.h"
#include <dirent.h>
#include <stdio.h>
#include <string.h>

#define AS_SMALL 64
#define AS_MEDIUM 256
#define AS_LARGE 1024
#define AS_MASSIVE 4096
#define CONFIG_PATH "/path/to/your/dir_configs.txt"

enum MenuStates { MAIN, TRANSFER_SELECT, TRANSFER_EXECUTE, EXIT };

int clear_input_buffer(void);

int read_dir_config(enum MenuStates *current_menu_state, char *nds_sav_dir,
                    char *mel_dir, char *nds_dir);

int main_menu(enum MenuStates *current_menu_state, char *usr_inpt,
              char *from_dir, char *to_dir, char *nds_sav_dir, char *mel_dir);

int select_sav_menu(enum MenuStates *current_menu_state,
                    char *selected_sav_file, char *from_dir);

int save_conversion(enum MenuStates *current_menu_state,
                    char *selected_sav_file, char *nds_dir,
                    size_t *byte_size_type);

int transfer_sav_data(enum MenuStates *current_menu_state, char *from_dir,
                      char *to_dir, char *selected_sav_file,
                      size_t *byte_size_type);

int main(void) {
  char from_dir[AS_LARGE] = {'\0'};
  char to_dir[AS_LARGE] = {'\0'};
  char selected_sav_file[AS_MEDIUM] = {'\0'};
  char usr_inpt = '\0';
  char nds_sav_dir[AS_LARGE];
  char mel_dir[AS_LARGE] = {'\0'};
  char nds_dir[AS_LARGE] = {'\0'};
  size_t byte_size_type = 0;
  int running = 1;
  enum MenuStates current_menu_state = MAIN;

  read_dir_config(&current_menu_state, nds_sav_dir, mel_dir, nds_dir);

  while (running) {
    if (current_menu_state == MAIN) {
      main_menu(&current_menu_state, &usr_inpt, from_dir, to_dir, nds_sav_dir,
                mel_dir);
    }

    if (current_menu_state == TRANSFER_SELECT) {
      select_sav_menu(&current_menu_state, selected_sav_file, from_dir);
    }

    if (current_menu_state == TRANSFER_EXECUTE) {
      if (strcmp(from_dir, nds_sav_dir) == 0) {
        save_conversion(&current_menu_state, selected_sav_file, nds_dir,
                        &byte_size_type);
      }
      transfer_sav_data(&current_menu_state, from_dir, to_dir,
                        selected_sav_file, &byte_size_type);
      running = 0;
    }

    if (current_menu_state == EXIT) {
      break;
    }
  }

  return 0;
}

int save_conversion(enum MenuStates *current_menu_state,
                    char *selected_sav_file, char *nds_dir,
                    size_t *byte_size_type) {
  char nds_file[AS_MEDIUM] = {'\0'};
  char nds_file_path[AS_LARGE] = {'\0'};
  unsigned char game_code[5] = {'\0'};
  unsigned char nds_header[AS_MEDIUM] = {'\0'};
  unsigned int game_code_save_type = 0;
  unsigned int game_code_as_int = 0;

  char input = '\0';

  do {
    printf("Convert save file?(Y/n) (still works without)\n");

    input = getchar();
    clear_input_buffer();

    if (input >= 'A' && input <= 'Z') {
      input = input - 'A' + 'a';
    }
  } while (input != 'y' && input != 'n');

  if (input == 'n') {
    printf("Continuing on with regular save file transfer.\n");
    return 0;
  }

  int index = 0;

  for (index = 0;
       selected_sav_file[index] != '\0' && selected_sav_file[index] != '.';
       index++) {
    nds_file[index] = selected_sav_file[index];
  }

  nds_file[index] = '\0';
  strcat(nds_file, ".nds");
  snprintf(nds_file_path, AS_LARGE, "%s%s", nds_dir, nds_file);

  FILE *nds_file_p = fopen(nds_file_path, "rb");

  if (nds_file_p == NULL) {
    printf("NDS file %s does not exist, check the directory %s\n", nds_file,
           nds_dir);
    return 0;
  }

  // CANNOT USE FSEEK() as we are working with FTP mounts, therefore we save
  // first x bytes into buffer to then memcpy to find the game code within that
  // buffer.
  size_t bytes_read = 0;

  if ((bytes_read = fread(nds_header, 1, AS_MEDIUM, nds_file_p)) < 16) {
    printf("Error: Unreadable game code, conitnuing regular file transfer");
    fclose(nds_file_p);
    return 0;
  }

  fclose(nds_file_p);

  memcpy(game_code, &nds_header[12], 4);
  game_code[4] = '\0';

  printf("Game code value: %s\n", game_code);

  game_code_as_int = (game_code[0]) | (game_code[1] << 8) |
                     (game_code[2] << 16) | game_code[3] << 24;

  printf("Game_code as int: %d\n", game_code_as_int);

  for (int i = 0; i < rom_data_count; i++) {
    if (game_code_as_int == rom_data[i].game_code) {
      printf("Found! Save type: %u\n", rom_data[i].savetype);
      game_code_save_type = rom_data[i].savetype;
    }
  }

  printf("Save type: %d, %u\n", game_code_save_type, game_code_save_type);

  *byte_size_type = save_sizes[game_code_save_type];

  printf("%zu\n", *byte_size_type);
  printf("Conversion path %s%s\n", nds_dir, selected_sav_file);

  *current_menu_state = MAIN;

  return 0;
}

int read_dir_config(enum MenuStates *current_menu_state, char *nds_sav_dir,
                    char *mel_dir, char *nds_dir) {
  char line[AS_LARGE] = {'\0'};
  char var_name[AS_LARGE] = {'\0'};
  char dir[AS_LARGE] = {'\0'};

  FILE *dir_config_p = fopen(CONFIG_PATH, "r");

  if (dir_config_p == NULL) {
    printf("Error: Please provide 'dir_config.txt' file in root.\n");
    *current_menu_state = EXIT;
    return 0;
  }

  printf("Successfully opened dir_config.txt\n");

  while (fgets(line, sizeof(line), dir_config_p)) {
    if (sscanf(line, "%[^=]=\"%[^\"]\"", var_name, dir) == 2) {
      if (strcmp(var_name, "NDS_SAV_DIR") == 0) {
        strcpy(nds_sav_dir, dir);
      } else if (strcmp(var_name, "MEL_DIR") == 0) {
        strcpy(mel_dir, dir);
      } else if (strcmp(var_name, "NDS_DIR") == 0) {
        strcpy(nds_dir, dir);
      }
    }
  }

  fclose(dir_config_p);

  printf("NDS_SAV_DIR=%s\nMEL_DIR=%s\nNDS_DIR=%s\n\n", nds_sav_dir, mel_dir,
         nds_dir);

  return 0;
}

int transfer_sav_data(enum MenuStates *current_menu_state, char *from_dir,
                      char *to_dir, char *selected_sav_file,
                      size_t *byte_size_type) {
  char from_file[AS_LARGE] = {'\0'};
  char to_file[AS_LARGE] = {'\0'};

  snprintf(from_file, AS_LARGE, "%s%s", from_dir, selected_sav_file);
  snprintf(to_file, AS_LARGE, "%s%s", to_dir, selected_sav_file);

  printf("Initiated Transfer\nFROM: %s\nTO: %s\n", from_file, to_file);

  FILE *from_file_p = fopen(from_file, "rb");

  if (from_file_p == NULL) {
    printf("Error: Cannot open source %s\n", from_file);
    *current_menu_state = MAIN;
    return 0;
  }

  FILE *to_file_p = fopen(to_file, "wb");

  if (to_file_p == NULL) {
    printf("Error: Cannot open destination %s\n", to_file);
    *current_menu_state = MAIN;
    fclose(from_file_p);
    return 0;
  }

  int char_byte = 0;

  if (*byte_size_type == 0) {
    while ((char_byte = fgetc(from_file_p)) != EOF) {
      fputc(char_byte, to_file_p);
    }
  } else {

    char buffer[AS_MASSIVE];
    size_t remainder = *byte_size_type;

    while (remainder > 0) {
      size_t to_read;

      if (remainder < AS_MASSIVE) {
        to_read = remainder;
      } else {
        to_read = AS_MASSIVE;
      }

      size_t bytes_read = fread(buffer, 1, to_read, from_file_p);

      fwrite(buffer, 1, bytes_read, to_file_p);
      remainder -= bytes_read;

      if (bytes_read < to_read) {
        break;
      }
    }
  }

  fclose(from_file_p);
  fclose(to_file_p);

  printf("Transfer Complete\n");

  return 0;
}

int select_sav_menu(enum MenuStates *current_menu_state,
                    char *selected_sav_file, char *from_dir) {
  char file_names[AS_MEDIUM][AS_LARGE] = {'\0'};
  int file_count = 0;

  DIR *from_dir_p = opendir(from_dir);
  struct dirent *entry = NULL;

  if (from_dir_p == NULL) {
    printf("Error: Cannot open directory %s\n", from_dir);
    *current_menu_state = MAIN;
    return 0;
  }

  while ((entry = readdir(from_dir_p)) != NULL && file_count < AS_MEDIUM) {
    size_t len = strlen(entry->d_name);

    /**
     * filename+len-4 from length because we are checking for if .sav is
     * present in string. if strcmp produces 0 if strings compared are
     * identical
     * */

    if (len > 4 && strcmp(entry->d_name + len - 4, ".sav") == 0) {
      strcpy(file_names[file_count], entry->d_name);
      file_count++;
    }
  }

  closedir(from_dir_p);

  if (file_count == 0) {
    printf("Error: No .sav files found %s\n", from_dir);
    *current_menu_state = MAIN;
    return 0;
  }

  printf("Please select a .sav file for transfer: \n");
  for (int i = 0; i < file_count; i++) {
    printf("%d)\t%s\n", i + 1, file_names[i]);
  }
  printf("0)\tExit\n> ");

  int choice = 0;

  if (scanf("%d", &choice) < 0) {
    printf("Invalid Input");
    *current_menu_state = MAIN;
    return 0;
  }

  clear_input_buffer();

  if (choice > 0 && choice <= file_count) {
    strcpy(selected_sav_file, file_names[choice - 1]);
    printf("\nSelected: %s\n", selected_sav_file);
    *current_menu_state = TRANSFER_EXECUTE;
    return 0;
  }

  *current_menu_state = MAIN;
  return 0;
}

int main_menu(enum MenuStates *current_menu_state, char *usr_inpt,
              char *from_dir, char *to_dir, char *nds_sav_dir, char *mel_dir) {
  printf("Save File Transferer\n");
  printf("1)\tNDS -> Melon\n2)\tMelon -> NDS\n0)\tExit\n> ");

  *usr_inpt = getchar();
  clear_input_buffer();

  switch (*usr_inpt) {
  case '1':
    *current_menu_state = TRANSFER_SELECT;
    strcpy(to_dir, mel_dir);
    strcpy(from_dir, nds_sav_dir);

    break;

  case '2':
    *current_menu_state = TRANSFER_SELECT;
    strcpy(from_dir, mel_dir);
    strcpy(to_dir, nds_sav_dir);

    break;

  case '0':
    *current_menu_state = EXIT;

    break;

  default:
    printf("Error: Invalid input\n");

    break;
  }

  return 0;
}

int clear_input_buffer(void) {
  while (getchar() != '\n') {
  }

  return 1;
}
