#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <math.h>

#include "fat_structures.h"

void die(const char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(1);
}

void fs_read(int fd, size_t offset, void *buffer, size_t len) {
    lseek(fd,offset,SEEK_SET);
    read(fd,buffer,len);
}

int endianess(void){
    uint32_t num = 0x12000078;
    uint8_t *test = (uint8_t * )&num;

    if(test[0] == 0x12){
        return 0; // big endian
    }
    else{
        return 1; // little endian
    }
}
void printLabel(int fd) {
    // Artemis expects this exact format
    char *format_string = "The Label of the given FAT16 file system is: \"%.11s\"\n";
    char* buffer = malloc(12);
    fs_read(fd,43,(void*)buffer,11);
    buffer[11] = '\0';
    printf(format_string,buffer);
    free(buffer);
}

FAT_ENTRY *readFAT(int fd, struct boot_sector *boot) {
    
    uint16_t *s_size = (uint16_t*)boot->sector_size;
    if(endianess()){
        *s_size = GET_UNALIGNED_W(boot->sector_size);
    }
    FAT_ENTRY* fat = malloc(boot->fat_length * (*s_size));
    fs_read(fd,(boot->reserved)*(*s_size),(void*)fat,boot->fat_length*(*s_size));
    return fat;
}

void initFatData(int fd, struct boot_sector *boot, FATData *fatData) {
    fatData->fd = fd;
    fatData->boot = boot;
    fatData->fat = readFAT(fd,boot);
    fatData->sector_size = *((uint16_t*)boot->sector_size);
    fatData->root_entries = *((uint16_t*)boot->dir_entries);
    if(endianess()){
        fatData->sector_size = GET_UNALIGNED_W(boot->sector_size);
        fatData->root_entries = GET_UNALIGNED_W(boot->dir_entries);
    }
    fatData->entry_size = boot->cluster_size*fatData->sector_size;
    fatData->rootdir_offset = (boot->reserved + boot->fat_length*boot->fats)*fatData->sector_size;
    fatData->data_offset = fatData->rootdir_offset + ceil(fatData->root_entries*32/fatData->sector_size)*fatData->sector_size;
}

void readFile(FATData *fatData, DIR_ENT *dir) {
    if (CHECKFLAGS(dir->attr, ATTR_DIR)) return;
    
    // Artemis expects this line
    printf("File: \"%s\"\n",sanitizeName((char*)dir->name));
    puts("Reading file:");

    uint16_t fat = dir->start;
    do{
        char* buffer = malloc(fatData->entry_size+1);
        fs_read(fatData->fd,fatData->data_offset+(fat-2)*fatData->entry_size,(void*)buffer,fatData->entry_size);
        buffer[fatData->entry_size] = '\0';
        printf("%s",buffer);
        fflush(stdout);
        free(buffer);
        fat = fatData->fat[fat].value;
    }while(fat <= 0xFFEF&&fat >= 0x0002);
	print("\n");
}

void iterateDirectory(FATData *fatData, DIR_ENT *dir, int level) {
    if (!CHECKFLAGS(dir->attr, ATTR_DIR)) return;
    uint16_t fat = dir->start;
    do{
        DIR_ENT *cur = malloc(sizeof(DIR_ENT));
        for(uint32_t i = 0; i < fatData->entry_size/sizeof(DIR_ENT);i++){
            fs_read(fatData->fd,fatData->data_offset+fatData->entry_size*(fat-2)+i*sizeof(DIR_ENT),(void*)cur,sizeof(DIR_ENT));
            handleEntry(fatData,cur,level);
        }
        free(cur);
        fat = fatData->fat[fat].value;
    }while(fat <= 0xFFEF&&fat >= 0x0002);
}

void iterateRoot(FATData *fatData) {
    for(uint16_t i = 0; i< fatData->root_entries; i++){
        DIR_ENT *cur = malloc(sizeof(DIR_ENT));
        fs_read(fatData->fd,fatData->rootdir_offset+i*sizeof(DIR_ENT),(void*)cur,sizeof(DIR_ENT));
    
        handleEntry(fatData,cur,0);
        free(cur);
    }
}

void handleEntry(FATData *fatData, DIR_ENT *dir, int level){

    if(CHECKFLAGS(dir->attr, ATTR_VOLUME)){
        return;
    }
    if(dir->name[0] == '\0'){
        return; 
    }
    for(int i = 0; i< level; i++){
        printf("\t");
    }
    if (CHECKFLAGS(dir->attr, ATTR_DIR)){
        printf("Dir : \"%s\"\n",sanitizeName((char*)dir->name));
        fflush(stdout);
        if(dir->name[0] != '.')
        	iterateDirectory(fatData,dir,level+1);
    }
    else{
        printf("File: \"%s\"\n",sanitizeName((char*)dir->name));
        fflush(stdout);
    }
}

void search_print_dir(FATData *fatData, DIR_ENT *dir,const char* name);
int stringCmp(char* a, const char* b){
	char aa = a[0];
	char bb = b[0];
	int index = 0;
	while(aa != '\0'){
		if(a[index] != b[index]){
			return 1;
		}
		index++;
		aa = a[index];
		bb = b[index];
	}
	if(aa != bb)
		return 1;
	return 0;
}
void search_handleEntry(FATData *fatData, DIR_ENT *dir,const char* name){
    
    if(CHECKFLAGS(dir->attr, ATTR_VOLUME)){
        return;
    }
    if(dir->name[0] == '\0'){
        return; 
    }
    if (CHECKFLAGS(dir->attr, ATTR_DIR)){
    	if(dir->name[0] !='.')
        	search_print_dir(fatData,dir,name);
    }
    else{
    	char* thisName = sanitizeName((char*)dir->name);
        if(stringCmp(thisName,name) == 0){
            readFile(fatData,dir);
        }
    }
}

void search_print_root(FATData *fatData, const char* name){
    for(uint16_t i = 0; i< fatData->root_entries; i++){
        DIR_ENT *cur = malloc(sizeof(DIR_ENT));
        fs_read(fatData->fd,fatData->rootdir_offset+i*sizeof(DIR_ENT),(void*)cur,sizeof(DIR_ENT));
        search_handleEntry(fatData,cur,name);
        free(cur);
    }
}


void search_print_dir(FATData *fatData, DIR_ENT *dir,const char* name){
    if (!CHECKFLAGS(dir->attr, ATTR_DIR)) return;
    uint16_t fat = dir->start;
    do{
        DIR_ENT *cur = malloc(sizeof(DIR_ENT));
        for(uint32_t i = 0; i < fatData->entry_size/sizeof(DIR_ENT);i++){
            fs_read(fatData->fd,fatData->data_offset+fatData->entry_size*(fat-2)+i*sizeof(DIR_ENT),(void*)cur,sizeof(DIR_ENT));
            search_handleEntry(fatData,cur,name);
        }
        free(cur);
        fat = fatData->fat[fat].value;
    }while(fat <= 0xFFEF&&fat >= 0x0002);
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

    if(argc == 1){
        print_help(argv[0]);
        die("");
    }
    int fd = open(argv[1],O_RDONLY);
    if(fd == -1){
        die("unable to open file" );
    }

    char command = ' ';

    struct boot_sector * boot = malloc(sizeof(struct boot_sector));
    if(boot == NULL)
    	die("");
    fs_read(fd,0,(void*)boot,512);
    FATData* fat = malloc(sizeof(FATData));
    if(fat == NULL)
    	die("");
    initFatData(fd,boot,fat);
    do {
        print_menu();

        do {
            command = (char) fgetc(stdin);
        } while (isspace(command) || command == '\n');
        switch (command) {
            case 'l': 
                printLabel(fd);
                printf("\n");
                break;
            case 'i': 
                iterateRoot(fat);
                printf("\n");
                break;
            case 'f': {
                char* name = malloc(12);
                fgetc(stdin);
                for(int i = 0; i< 12; i++){
                	name[i] = (char)fgetc(stdin);
                	if(name[i] == '\n'){
                		name[i] = '\0';
                		break;
                	}
                }
             
                search_print_root(fat,name);
                free(name);
                printf("\n");
                break;
            }
            case 'q': 
                die("");
                free(boot);
                free(fat->fat);
                free(fat);
                break;
            default: break;
        }
    } while (command != 'q');

    exit(EXIT_SUCCESS);
}

