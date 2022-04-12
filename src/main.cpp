#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unistd.h>

using namespace std;

typedef unsigned char Uint8;
typedef signed char Int8;
typedef short unsigned int Uint16;
typedef short signed int Int16;
typedef unsigned int Uint32;
typedef signed int Int32;
typedef long unsigned int Uint64;
typedef long signed int Int64;
typedef long long unsigned int Uint128;
typedef long long signed int Int128;

struct DirectoryEntry {
    filesystem::directory_entry entry;
    string path;
};

bool search(filesystem::directory_entry entry, vector<filesystem::directory_entry> vector, Uint32 &pos);

int main(int argc, char* argv[]) {
    ofstream log("log.txt");
    if(argc < 3) return 1;
    string src = argv[1], dst = argv[2];
    vector<filesystem::directory_entry> allFilesNow, allFilesBefore, toCopyFiles, toRemoveFiles;
    Uint32 size, pos;

    while(true) {
        for(filesystem::directory_entry entry : filesystem::recursive_directory_iterator(src)) {
            allFilesNow.push_back(entry);
            log << "Found source: " << entry.path() << endl;
        }
        for(filesystem::directory_entry entry : filesystem::recursive_directory_iterator(dst)) {
            allFilesBefore.push_back(entry);
            log << "Found destination: " << entry.path() << endl;
        }
        //Files to copy
        size = allFilesNow.size();
        for(Uint32 i = 0; i < size; i++) {
            if(!search(allFilesNow[i], allFilesBefore, pos)) {
                toCopyFiles.push_back(allFilesNow[i]);
                log << "To copy because not there: " << allFilesNow[i].path() << endl;
            }
            else if(!allFilesNow[i].is_directory()) {
                if(allFilesNow[i].file_size() != allFilesBefore[pos].file_size()
                    || allFilesNow[i].last_write_time() != allFilesBefore[pos].last_write_time()) {
                    toCopyFiles.push_back(allFilesNow[i]);
                    log << "To copy because different size or date: " << allFilesNow[i].path() << endl;
                }
            }
        }
        //Files to delete
        size = allFilesBefore.size();
        for(Uint32 i = 0; i < size; i++) {
            if(!search(allFilesBefore[i], allFilesNow, pos)) {
                toRemoveFiles.push_back(allFilesBefore[i]);
                log << "To remove: " << allFilesBefore[i].path() << endl;
            }
        }
        cout << "Check is finished" << endl;

        allFilesNow.clear();
        allFilesBefore.clear();
        toCopyFiles.clear();
        toRemoveFiles.clear();
        sleep(60);
    }

    return 0;
}

bool search(filesystem::directory_entry entry, vector<filesystem::directory_entry> vector, Uint32 &pos) {
    Uint32 size = vector.size();
    for(Uint32 i = 0; i < size; i++) {
        if(entry.path() == vector[i].path()) {
            pos = i;
            return true;
        }
    }
    return false;
}