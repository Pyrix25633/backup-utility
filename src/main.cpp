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

string getPath(filesystem::directory_entry entry, string from);
bool search(DirectoryEntry entry, vector<DirectoryEntry> vector, Uint32 &pos);

int main(int argc, char* argv[]) {
    ofstream log("log.txt");
    if(argc < 3) return 1;
    string src = argv[1], dst = argv[2];
    vector<DirectoryEntry> allFilesNow, allFilesBefore, toCopyFiles, toRemoveFiles;
    Uint32 size, pos;

    while(true) {
        for(filesystem::directory_entry entry : filesystem::recursive_directory_iterator(src)) {
            DirectoryEntry e;
            e.entry = entry;
            e.path = getPath(entry, src);
            allFilesNow.push_back(e);
            log << "Found source: " << e.path << endl;
        }
        for(filesystem::directory_entry entry : filesystem::recursive_directory_iterator(dst)) {
            DirectoryEntry e;
            e.entry = entry;
            e.path = getPath(entry, dst);
            allFilesBefore.push_back(e);
            log << "Found destination: " << e.path << endl;
        }
        //Files to copy
        size = allFilesNow.size();
        for(Uint32 i = 0; i < size; i++) {
            if(!search(allFilesNow[i], allFilesBefore, pos)) {
                toCopyFiles.push_back(allFilesNow[i]);
                log << "To copy because not there: " << allFilesNow[i].path << endl;
            }
            else if(!allFilesNow[i].entry.is_directory()) {
                if(allFilesNow[i].entry.file_size() != allFilesBefore[pos].entry.file_size()) {
                    toCopyFiles.push_back(allFilesNow[i]);
                    log << "To copy because different size: " << allFilesNow[i].path << endl;
                }
                else if(allFilesNow[i].entry.last_write_time() != allFilesBefore[pos].entry.last_write_time()) {
                    log << "To copy because different date: " << allFilesNow[i].path << endl;
                }
            }
        }
        //Files to delete
        size = allFilesBefore.size();
        for(Uint32 i = 0; i < size; i++) {
            if(!search(allFilesBefore[i], allFilesNow, pos)) {
                toRemoveFiles.push_back(allFilesBefore[i]);
                log << "To remove: " << allFilesBefore[i].path << endl;
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

bool search(DirectoryEntry entry, vector<DirectoryEntry> vector, Uint32 &pos) {
    Uint32 size = vector.size();
    for(Uint32 i = 0; i < size; i++) {
        if(entry.path == vector[i].path) {
            pos = i;
            return true;
        }
    }
    return false;
}

string getPath(filesystem::directory_entry entry, string from) {
    string path;
    path = entry.path();
    path = path.replace(0, from.size() + 1, "");
    return path;
}