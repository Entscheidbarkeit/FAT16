/*
 * The structures in this file have been adapted from fsck.fat.h
 * from dosfsutils debian package for educational purpose.
 * Credit to original authors and license see below.
 * Feel free to modify or ignore this code to your liking.
 */

/* fsck.fat.h  -  Common data structures and global variables
   Copyright (C) 1993 Werner Almesberger <werner.almesberger@lrc.di.epfl.ch>
   Copyright (C) 1998 Roman Hodek <Roman.Hodek@informatik.uni-erlangen.de>
   Copyright (C) 2008-2014 Daniel Baumann <mail@daniel-baumann.ch>
   Copyright (C) 2015 Andreas Bombe <aeb@debian.org>
   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
   GNU General Public License for more details.
   You should have received a copy of the GNU General Public License
   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef FAT_STRUCTURES_H
#define FAT_STRUCTURES_H

#include <inttypes.h>
#include <stddef.h>

#define ATTR_NONE    0 /* no attribute bits */
#define ATTR_RO      1 /* read-only */
#define ATTR_HIDDEN  2 /* hidden */
#define ATTR_SYS     4 /* system */
#define ATTR_VOLUME  8 /* volume label */
#define ATTR_DIR    16 /* directory */
#define ATTR_ARCH   32 /* archived */

#define MSDOS_NAME 11
#define MSDOS_DOT    ".          " /* "." , padded to MSDOS_NAME chars */
#define MSDOS_DOTDOT "..         " /* "..", padded to MSDOS_NAME chars */

#define GET_UNALIGNED_W(f) \
    ( (uint16_t)f[0] | ((uint16_t)f[1]<<8) )

#define CHECKFLAGS(byte, flag) !!((byte & flag) == flag)

#define MIN(x,y) ((x < y) ?x :y)
#define MAX(x,y) ((x > y) ?x :y)

struct boot_sector {
	uint8_t  ignored[3];     /* Boot strap short or near jump */
	uint8_t  system_id[8];   /* Name - can be used to special case
	                            partition manager volumes */
	uint8_t  sector_size[2]; /* bytes per logical sector */
	uint8_t  cluster_size;   /* sectors/cluster */
	uint16_t reserved;       /* reserved sectors */
	uint8_t  fats;           /* number of FATs */
	uint8_t  dir_entries[2]; /* root directory entries */
	uint8_t  sectors[2];     /* number of sectors */
	uint8_t  media;          /* media code (unused) */
	uint16_t fat_length;     /* sectors/FAT */
	uint16_t secs_track;     /* sectors per track */
	uint16_t heads;          /* number of heads */
	uint32_t hidden;         /* hidden sectors (unused) */
	uint32_t total_sect;     /* number of sectors (if sectors == 0) */
	
	uint8_t  drive_number;   /* Logical Drive Number */
	uint8_t  reserved2;      /* Unused */
	
	uint8_t  extended_sig;   /* Extended Signature (0x29) */
	uint32_t serial;         /* Serial number */
	uint8_t  label[11];      /* FS label */
	uint8_t  fs_type[8];     /* FS Type */
	
	/* fill up to 512 bytes */
	uint8_t junk[450];
} __attribute__ ((packed));


typedef struct {
	uint8_t  name[MSDOS_NAME];  /* name including extension */
	uint8_t  attr;              /* attribute bits */
	uint8_t  lcase;             /* Case for base and extension */
	uint8_t  ctime_ms;          /* Creation time, milliseconds */
	uint16_t ctime;             /* Creation time */
	uint16_t cdate;             /* Creation date */
	uint16_t adate;             /* Last access date */
	uint16_t starthi;           /* High 16 bits of cluster in FAT32 */
	uint16_t time, date, start; /* time, date and first cluster */
	uint32_t size;              /* file size (in bytes) */
} __attribute__ ((packed)) DIR_ENT;

typedef struct {
	uint16_t value;
} FAT_ENTRY;

// Folgende Datenstruktur wird für Teilaufgabe 6 benötigt.
typedef struct {
	int fd;                   /* Filedeskriptor zu geöffnetem Dateisystem Image */
	struct boot_sector *boot; /* Pointer auf Boot Sector des Dateisystems */
	FAT_ENTRY *fat;           /* Pointer auf eine Version der FAT Tabelle */
	uint16_t sector_size;     /* Größe eines einzelnen Sektors (in Byte) */
	uint16_t entry_size;      /* Größe eines Clusters (Blocks) im Dateisystem (in Byte) */
	uint16_t root_entries;    /* Anzahl der Einträge im Root Verzeichnis */
	uint16_t rootdir_offset;  /* Offset des Root Verzeichnis innerhalb des Dateisystems */
	uint16_t data_offset;     /* Offset des Beginns der Datenblöcke im Dateisystem */
} FATData;

// Gegebene Funktion (sanitize.c)
/*
 * Funktion bekommt als Argument einen DIR_ENT->name Pointer und gibt
 * einen Pointer auf einen bereinigten Dateinamen zurück.
 */
extern char *sanitizeName(char *string);


// Folgende Funktionen sind in der Hausaufgabe zu Implementieren.

/*
 * Die Funktion soll einen gegebenen Bereich aus einer Datei in den gegebenen
 * Puffer einlesen.
 *
 * @param fd Filedeskriptor
 * @param offset Offset im File
 * @param buffer Puffer in den gelesen werden soll
 * @param len Länge der zu lesenden Daten
 */
extern void fs_read(int fd, size_t offset, void * buffer, size_t len);

/*
 * Die Funktion soll das Label des DAteisystems ausgeben.
 *
 * @param fd Filedeskriptor
 */
extern void printLabel(int fd);

/*
 * Die Funktion soll eine Kopie der Fat Tabelle aus der Datei in den
 * Speicher lesen.
 *
 * @param fd Filedeskriptor
 * @param boot Pointer auf Boot Sector Datenstruktur
 */
extern FAT_ENTRY* readFAT(int fd, struct boot_sector *boot);

/*
 * Die Funktion soll die Hilfsdatenstruktur fatData initialisieren.
 *
 * @param fd Filedeskriptor
 * @param boot Pointer auf Boot Sector Datenstruktur
 * @param fatData Uninitialisierte FATData Datenstruktur
 */
extern void initFatData(int fd, struct boot_sector *boot, FATData* fatData);

/*
 * Die Funktion soll für alle Einträge im Root Dateisystem die Funktion
 * handleEntry() aufrufen.
 *
 * @param fatData Initialisierte FATData Datenstruktur
 */
extern void iterateRoot(FATData *fatData);

/*
 * Die Funktion soll einen Eintrag eines Verzeichnisses behandeln.
 * Zuerst soll der Name des Eintrags ausgegeben werden. Der Name soll
 * entsprechend der Verzeichnistiefe (level) mit '\t' eingerückt werden.
 * Später sollen Verzeichnisse rekursiv iteriert (iterateDirectory) und ggf.
 * Dateien ausgegeben werden (readFile).
 * Achtung:
 *  * Dateien vom Typ ATTR_VOLUME sollen nicht beachtet werden.
 *  * Dateien, deren Name mit einem Nullbyte beginnt, sind ungültig.
 *
 * @param fatData Initialisierte FATData Datenstruktur
 * @param dir Pointer auf den aktuellen Verzeichniseintrag.
 * @param level Aktuelle Tiefe im Verzeichnisbaum.
 */
extern void handleEntry(FATData *fatData, DIR_ENT *dir, int level);

/*
 * In dieser Funktion soll der Inhalt einer Datei aus dem Dateisystem gelesen
 * und auf der Standardausgabe ausgegeben werden.
 * Die von uns vorbereiteten Dateien enthalten ausschließlich druckbare ASCII-Zeichen.
 * Beachten Sie, dass Dateien aus mehr als einem Block bestehen können.
 *
 * @param fatData Initialisierte FATData Datenstruktur
 * @param dir Pointer auf den aktuellen Verzeichniseintrag.
 */
extern void readFile(FATData *fatData, DIR_ENT* dir);

/*
 * Die Funktion soll für alle Einträge im gegebenen Verzeichnis die Funktion
 * handleEntry() aufrufen.
 * Beachten Sie, dass Verzeichnisse aus mehr als einem Block bestehen können.
 *
 * @param fatData Initialisierte FATData Datenstruktur
 * @param dir Pointer auf den aktuellen Verzeichniseintrag.
 * @param level Tiefe im Verzeichnisbaum.
 */
extern void iterateDirectory(FATData *fatData, DIR_ENT *dir, int level);
#endif /* FAT_STRUCTURES_H */
