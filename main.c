#include <stdio.h>
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <grp.h>

void list_directory(DIR *dir, int show_all, const char *path);
void mode_string(mode_t mode, char *str);
void print_long(const char *dir, const char *name);

int show_all = 0;

int main(int argc, char **argv) {
    int opt = 0;

    while((opt = getopt(argc, argv, "a")) != -1) {
        switch(opt) {
            case 'a':
                show_all = 1;
                break;
            default:
                fprintf(stderr, "usage: %s [-a] [path]", argv[0]);
                return 1;
        }
    }

    const char *path = (optind < argc) ? argv[optind] : ".";

    DIR *dir = opendir(path);

    if(!dir) {
        perror("opendir");
        return 1;
    }

    list_directory(dir, show_all, path);

    closedir(dir);

    return 0;
}

void list_directory(DIR *dir, int show_all, const char *path) {
    struct dirent *entry;
    while((entry = readdir(dir)) != NULL) {
        if(!show_all && entry->d_name[0] == '.') continue;
        print_long(path, entry->d_name);
        //printf("%s\n", entry->d_name);
    }

}

void mode_string(mode_t mode, char *str) {
    if (S_ISDIR(mode))          str[0] = 'd'; // directory
    else if (S_ISCHR(mode))     str[0] = 'c'; // char device
    else if (S_ISBLK(mode))     str[0] = 'b'; // block device
    else if (S_ISFIFO(mode))    str[0] = 'p'; // FIFO (named pipe)
    else if (S_ISLNK(mode))     str[0] = 'l'; // sim link
    else if (S_ISSOCK(mode))    str[0] = 's'; // socket
    else                        str[0] = '-'; // regular files (& others)

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

void print_long(const char *dir, const char *file) {
    char fullpath[4096]; // max path on Linux 
    snprintf(fullpath, sizeof(fullpath), "%s/%s", dir, file);
    
    struct stat sb;
    if(lstat(fullpath, &sb) < 0) {
        perror(file); // in case the file is deleted or if the user doesn't have permission
        return;
    }

    char modes[11];
    mode_string(sb.st_mode, modes);

    struct passwd *pw = getpwuid(sb.st_uid);
    struct group *gr = getgrgid(sb.st_uid);

    const char *user = pw ? pw->pw_name : "?";
    const char *group = gr ? gr->gr_name : "?";

    printf("%s %s %s %s\n", modes, user, group, file);
}
