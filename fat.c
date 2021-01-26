#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>

#include "fat_structures.h"

void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void fs_read(int fd, size_t offset, void *buffer, size_t len) {
    // TODO
}

void printLabel(int fd) {
    // Artemis expects this exact format
    char *format_string = "The Label of the given FAT16 file system is: \"%.11s\"\n";
    // TODO
}

FAT_ENTRY *readFAT(int fd, struct boot_sector *boot) {
    // TODO
}

void initFatData(int fd, struct boot_sector *boot, FATData *fatData) {
    // TODO
}

void readFile(FATData *fatData, DIR_ENT *dir) {
    if (CHECKFLAGS(dir->attr, ATTR_DIR)) return;
    
    // Artemis expects this line
    puts("Reading file:");

    // TODO
}

void iterateDirectory(FATData *fatData, DIR_ENT *dir, int level) {
    if (!CHECKFLAGS(dir->attr, ATTR_DIR)) return;
    // TODO
}

void iterateRoot(FATData *fatData) {
    // TODO
}

void print_help(char *prog_name) {
    printf("Usage: %s FAT16_IMAGE_FILE\n", prog_name);
    fflush(stdout);
}

void print_menu() {
    // Artemis expects these exact lines
    puts("Type your command, then [enter]");
    puts("Available commands:");
    puts("l      Print label of FAT16_IMAGE_FILE");
    puts("i      Iterate file system and print the name of all directories/files");
    puts("f FILE Print content of FILE");
    puts("q      quit the program.");
    fflush(stdout);
}

int main(int argc, char *argv[]) {
    // TODO
    
    char command = ' ';
    do {
        print_menu();

        do {
            command = (char) fgetc(stdin);
        } while (isspace(command) || command == '\n');
        switch (command) {
            case 'l':
            case 'i':
            case 'f':
            case 'q':
            default:
        }
    } while (command != 'q');

    exit(EXIT_SUCCESS);
}
