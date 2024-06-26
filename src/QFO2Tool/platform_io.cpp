#include "platform_io.h"
#include "Image2Texture.h"
#include "tinyfiledialogs.h"

int ext_compare_utf8_ascii_case_insensitive(char* str1, char* str2, int num_char)
{
    while (num_char > 0) {
        char c1 = *str1;
        char c2 = *str2;
        if (c1 >= 'A' && c1 <= 'Z') c1 = 'a' + c1 - 'A';
        if (c2 >= 'A' && c2 <= 'Z') c2 = 'a' + c2 - 'A';
        if (c1 < c2) return -1;
        if (c1 > c2) return 1;
        if (c1 == '\0') return 0; // both strings terminate here
        str1++;
        str2++;
        num_char--;
    }
    return 0;
}


#ifdef QFO2_WINDOWS
#include <Windows.h>
#include <string.h>
#include <direct.h>

int ext_compare(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char)
{
    return _wcsnicmp(str1, str2, num_char);
}

//TODO: match this return to the linux version
bool io_isdir(char* dir_path)
{
    struct __stat64 stat_info;
    int error = _wstat64(tinyfd_utf8to16(dir_path), &stat_info);
    if (error == 0 && (stat_info.st_mode & _S_IFDIR) != 0) {
        /* dir_path exists and is a directory */
        return true;
    }
    return false;
}

//another way to check if directory exists?
// #include <stdbool.h>  //bool type ?
//returns true if the file exists, false otherwise
//TODO: needs a windows version?
bool io_file_exists(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        return false;
    }
    return (stat_info.st_mode & S_IFREG);
}


int io_file_size(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        return 0;
    }
    return (stat_info.st_size);
}

//makes a directory from the path provided
//recursively makes leading directories if they don't exist
//returns true on success or directory exists, false on error
//TODO: this also needs a windows version
// TODO this should probably take a wchar
bool io_make_dir(char* dir_path)
{
    int error;
    error = _mkdir(dir_path);
    if (error == 0) {
        return true;
    }
    else {
        if (errno == 2) {
        char* ptr = strrchr(dir_path, '/');
        *ptr = '\0';
        if (io_make_dir(dir_path)) {
            *ptr = '/';
            return io_make_dir(dir_path);
            }
        }
        else {
            printf("You may ask yourself, how did I get here?\n");
        }
    }
    printf("Error making directory, errno:\t%d:\t%s\n", errno, strerror(errno));
    return false;
}


#elif defined(QFO2_LINUX)
#include <strings.h>
#include <sys/stat.h>

int ext_compare(NATIVE_STRING_TYPE* str1, NATIVE_STRING_TYPE* str2, int num_char)
{
    return strncasecmp(str1, str2, num_char);
}

//returns true if path exists and is a directory
//false otherwise
bool io_isdir(char* dir_path)
{
    struct stat stat_info;
    int error = stat(dir_path, &stat_info);
    if (error) {
        printf("Error checking directory? %s\n", strerror(errno));
        return false;
    }
    return (stat_info.st_mode & S_IFDIR);
}

//another way to check if directory exists?
// #include <stdbool.h>  //bool type ?
//returns true if the file exists, false otherwise
bool io_file_exists(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        return false;
    }
    return (stat_info.st_mode & S_IFREG);
}

int io_file_size(const char* filename)
{
    struct stat stat_info;
    int error = stat(filename, &stat_info);
    if (error) {
        return 0;
    }
    return (stat_info.st_size);
}

//makes a directory from the path provided
//recursively makes leading directories if they don't exist
//returns true on success or directory exists, false on error
bool io_make_dir(char* dir_path)
{
    int error;
    error = mkdir(dir_path, (S_IRWXU | S_IRWXG | S_IRWXO));
    if (error == 0) {
        return true;
    }
    else {
        if (errno == 2) {
        char* ptr = strrchr(dir_path, '/');
        *ptr = '\0';
        if (io_make_dir(dir_path)) {
            *ptr = '/';
            return io_make_dir(dir_path);
            }
        }
        else {
            printf("You may ask yourself, how did I get here?\n");
        }
    }
    printf("Error making directory, errno:\t%d:\t%s\n", errno, strerror(errno));
    return false;
}


#endif

//check if path exists
bool io_path_check(char* file_path)
{
    if (io_isdir(file_path) == false) {
        int choice = tinyfd_messageBox(
            "Warning",
            "Directory does not exist.\n"
            "Create directory?\n\n"
            "--YES:    Create directory and new TILES.LST\n"
            "--NO:     Select different folder to create TILES.LST\n"
            "--CANCEL: Cancel...do I need to explain?\n",
            "yesnocancel", "warning", 2);
        if (choice == 0) {      //CANCEL: Cancel
            return false;
        }
        if (choice == 1) {      //YES:    Create directory and new TILES.LST
            bool dir_exists = io_make_dir(file_path);
            if (dir_exists == false) {

            //TODO: do I need to track this choice2?
                int choice2 = tinyfd_messageBox(
                    "Warning",
                    "Unable to create the directory.\n"
                    "probably a file system error?\n",
                    "ok", "warning", 0);

                return false;
            }
        }
        if (choice == 2) {      //NO:     Select different folder to create TILES.LST
            char* new_path = tinyfd_selectFolderDialog("Select save folder...", file_path);
            strncpy(file_path, new_path, MAX_PATH);
            return io_path_check(file_path);
        }
    }
    return true;
}

//create a backup file from file_path
//appends date&time in this format
//      _yyyymmdd_hhmmss
//appends same file extension as source
bool io_backup_file(char* file_path)
{
    char* extension = strrchr(file_path, '\0');
    char time_buff[32];
    char rename_buff[MAX_PATH];
    time_t t = time(NULL);
    tm* tp = localtime(&t);
    strftime(time_buff, 32, "_%Y%m%d_%H%M%S", tp);
    snprintf(rename_buff, MAX_PATH, "%s%s%s", file_path, time_buff, extension-4);
    // snprintf(rename_buff, MAX_PATH, "%s%s%s", file_path, time_buff, ".LST");

    int error = rename(file_path, rename_buff);
    if (error != 0) {
        perror("Error renaming TILES.LST: ");
        return false;
    }
    return true;
}


