/*  newls - A modern POSIX-compliant implementation of the ls utility.
 *  Copyright (C) 2026 Kunaiko
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <string.h>

void list_directory(DIR *dir, const char *path);
void mode_string(mode_t mode, char *str);
void print_long(const char *dir, const char *name);
int alphabetical_sort(const void *pointer_to_a, const void *pointer_to_b);
int size_sort(const void *pointer_to_a, const void *pointer_to_b);
int time_sort(const void *pointer_to_a, const void *pointer_to_b);
int time_comparison(struct timespec t1, struct timespec t2);

int show_all = 0;           /* -a: include entries starting with . */
int long_format = 0;        /* -l: use long listing format */
int reverse = 0;            /* -r: reverse order while sorting */
int size = 0;               /* -S: sort by file size */
int show_all_no_father = 0; /* -A: include entries starting with . except for .. & .*/
int time_sorting = 0;       /* -t: sorting by time */

const char *current_sorting_path = NULL;

int main(int argc, char **argv) {
    int opt = 0;

    while((opt = getopt(argc, argv, "alrSAt")) != -1) {
        switch(opt) {
            case 'a':
                show_all = 1;
                break;
            case 'l':
                long_format = 1;
                break;
            case 'r':
                reverse = 1;
                break;
            case 'S':
                size = 1;
                break;
            case 'A':
                show_all_no_father = 1;
                break;
            case 't':
                time_sorting = 1;
                break;
            default:
                fprintf(stderr, "usage: %s [-alrS] [path]", argv[0]);
                return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";

    DIR *dir = opendir(path);

    if(!dir) {
        perror("opendir");
        return 1;
    }

    list_directory(dir, path);

    closedir(dir);

    return 0;
}

/**
 * list_directory - Read, sort, and output directory contents
 * @dir: Open directory stream to process
 * @path: Filesystem path associated with the directory stream
 */
void list_directory(DIR *dir, const char *path) {
    struct dirent *entry;

    int total = 0;
    int capacity = 10;
    char **names = malloc(capacity * sizeof(char *));

    if(names == NULL) {
        perror("**names malloc");
        return;
    }

    while((entry = readdir(dir)) != NULL) {
        if(!show_all && !show_all_no_father && entry->d_name[0] == '.') continue;
        if(show_all_no_father && !show_all) {
            if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) continue;
        }
        if(total >= capacity) {
            capacity *= 2;
            char **temp = realloc(names, capacity *sizeof(char *));

            if(temp == NULL) {
                perror("names realloc");
                for(int i = 0; i < total; i++) free(names[i]);
                free(names);
                return;
            }
            names = temp;
        }

        names[total] = strdup(entry->d_name);
        total++;
    }

    current_sorting_path = path;
    if(size) {
        qsort(names, total, sizeof(char *), size_sort);
    } else if(time_sorting) {
        qsort(names, total, sizeof(char *), time_sort);
    } else {
        qsort(names, total, sizeof(char *), alphabetical_sort);
    }

    for(int i = 0; i < total; i++) {
        if(long_format) {
            print_long(path, names[i]);
        } else {
            printf("%s\n", names[i]);
        }

        free(names[i]);
    }

    free(names);
}

/**
 * mode_string - Construct a POSIX file permission string
 * @mode: File type and mode bits from stat structure
 * @str: Destination buffer of at least 11 bytes to hold the result
 */
void mode_string(mode_t mode, char *str) {
    if (S_ISDIR(mode))          str[0] = 'd';
    else if (S_ISCHR(mode))     str[0] = 'c';
    else if (S_ISBLK(mode))     str[0] = 'b';
    else if (S_ISFIFO(mode))    str[0] = 'p';
    else if (S_ISLNK(mode))     str[0] = 'l';
    else if (S_ISSOCK(mode))    str[0] = 's';
    else                        str[0] = '-';

    str[1] = (mode & S_IRUSR) ? 'r' : '-';
    str[2] = (mode & S_IWUSR) ? 'w' : '-';
    str[3] = (mode & S_IXUSR) ? 'x' : '-';
    str[4] = (mode & S_IRGRP) ? 'r' : '-';
    str[5] = (mode & S_IWGRP) ? 'w' : '-';
    str[6] = (mode & S_IXGRP) ? 'x' : '-';
    str[7] = (mode & S_IROTH) ? 'r' : '-';
    str[8] = (mode & S_IWOTH) ? 'w' : '-';
    str[9] = (mode & S_IXOTH) ? 'x' : '-';
    str[10] = '\0';
}

/**
 * print_long - Output detailed metadata for a filesystem entry
 * @dir: Parent directory path
 * @name: Entry filename
 */
void print_long(const char *dir, const char *name) {
    char fullpath[4096]; /* Enforce Linux PATH_MAX limit */
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, name);

    struct stat sb;
    if(lstat(fullpath, &sb) < 0) {
        perror(name); /* Handle race conditions where target is removed/inaccessible */
        return;
    }

    char modes[11];
    mode_string(sb.st_mode, modes);

    struct passwd *pw = getpwuid(sb.st_uid);
    struct group *gr = getgrgid(sb.st_gid);

    /* Fallback to '?' if UID/GID cannot be mapped */
    const char *user = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    char time_buf[64];
    struct tm *tm = localtime(&sb.st_mtim.tv_sec);
    strftime(time_buf, sizeof(time_buf), "%e %b %H:%M", tm);

    printf(
            "%s %lu %s %s %ld %s %s\n", 
            modes, 
            (unsigned long)sb.st_nlink, 
            user, 
            group, 
            (long)sb.st_size, 
            time_buf, 
            name
          );
}

/**
 * alphabetical_sort - qsort comparator for lexical ordering
 * @pointer_to_a: Pointer to first entry string
 * @pointer_to_b: Pointer to second entry string
 *
 * Returns a negative value if a < b, positive if a > b, or 0 if equal.
 * Respects the global 'reverse' flag.
 */
int alphabetical_sort(const void *pointer_to_a, const void *pointer_to_b) {
    const char *a = *(const char **)pointer_to_a;
    const char *b = *(const char **)pointer_to_b;

    if(reverse) {
        return strcmp(b, a);
    }
    return strcmp(a, b);
}

/**
 * size_sort - qsort comparator for file size ordering
 * @pointer_to_a: Pointer to first entry string
 * @pointer_to_b: Pointer to second entry string
 *
 * Returns a negative value if a < b, positive if a > b, or 0 if equal.
 * Respects the global 'reverse' flag.
 */
int size_sort(const void *pointer_to_a, const void *pointer_to_b) {
    const char *name_a = *(const char **)pointer_to_a;
    const char *name_b = *(const char **)pointer_to_b;

    char path_a[4096];
    char path_b[4096];

    snprintf(path_a, sizeof(path_a), "%s/%s", current_sorting_path, name_a);
    snprintf(path_b, sizeof(path_b), "%s/%s", current_sorting_path, name_b);

    struct stat sb_a, sb_b;

    if(lstat(path_a, &sb_a) < 0) {
        perror(name_a);
        return 0;
    } else if(lstat(path_b, &sb_b) < 0) {
        perror(name_b);
        return 0;
    }

    if(reverse) {
        if(sb_a.st_size > sb_b.st_size) return 1;
        if(sb_a.st_size < sb_b.st_size) return -1;
        return strcmp(name_a, name_b);
    }

    if(sb_a.st_size > sb_b.st_size) return -1;
    if(sb_a.st_size < sb_b.st_size) return 1;
    return strcmp(name_a, name_b);
}

/**
 * time_sort - qsort comparator for file tinme ordering
 * @pointer_to_a: Pointer to first entry string 
 * @pointer_to_b: Pointer to second entry string
 * 
 * Returns a negative value if a < b, positive if a > b, or 0 if equal.
 * Respects the global 'reverse' flag.
 */ 
int time_sort(const void *pointer_to_a, const void *pointer_to_b) {
    const char *name_a = *(const char **)pointer_to_a;
    const char *name_b = *(const char **)pointer_to_b;

    char path_a[4096];
    char path_b[4096];

    snprintf(path_a, sizeof(path_a), "%s/%s", current_sorting_path, name_a);
    snprintf(path_b, sizeof(path_b), "%s/%s", current_sorting_path, name_b);

    struct stat sb_a, sb_b;

    if(lstat(path_a, &sb_a) < 0) {
        perror(name_a);
        return 0;
    } else if(lstat(path_b, &sb_b) < 0) {
        perror(name_b);
        return 0;
    }

    int time_comp = time_comparison(sb_b.st_ctim, sb_a.st_ctim);

    if(time_comp != 0) {
        return reverse ? -time_comp : time_comp;
    } else {
        return reverse ? strcmp(name_b, name_a) : strcmp(name_a, name_b);
    }
}

/**
 * time_comparison - Helper function to compare two timespec structures
 * @t1: First time struct (seconds and nanoseconds)
 * @t2: Second time struct (seconds and nanoseconds)
 * 
 * Compares tv_sec first. If they are equal, falls back to comparing 
 * tv_nsec for higher precision. Returns -1 if t1 < t2, 1 if t1 > t2, 
 * or 0 if they are identical.
 */
int time_comparison(struct timespec t1, struct timespec t2) {
    if(t1.tv_sec != t2.tv_sec) {
        return t1.tv_sec < t2.tv_sec ? -1 : 1;
    }

    if(t1.tv_nsec != t2.tv_nsec) {
        return t1.tv_nsec < t2.tv_nsec ? -1 : 1;
    }
    return 0;
}
