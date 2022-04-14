#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <iterator>
#include <algorithm>
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

char dirSep = '\\';

bool search(DirectoryEntry &entry, vector<DirectoryEntry> &vector, Uint32 &pos);
string getPath(filesystem::path entryPath, string &from);
string getDirectory(string path);
bool compareFiles(filesystem::path path1, filesystem::path path2);

int main(int argc, char* argv[]) {
    ofstream log("log.txt");
    if(argc < 3) return 1;
    string src = "", dst = "", rmv = "";
    vector<DirectoryEntry> allFilesNow, allFilesBefore, toCopyFiles, toRemoveFiles;
    Uint32 size, pos, time = 0, cpdFiles = 0, rmvdFiles = 0;
    filesystem::copy_options copyOptions = filesystem::copy_options::overwrite_existing
        | filesystem::copy_options::recursive | filesystem::copy_options::directories_only;

    for(Uint8 i = 1; i < argc; i++) {
        string s = argv[i];
        if(s == "-s") {
            src = argv[i + 1];
            cout << "Source: " << src << endl;
        }
        else if(s == "-d") {
            dst = argv[i + 1];
            cout << "Destination: " << dst << endl;
        }
        else if(s == "-r") {
            rmv = argv[i + 1];
            cout << "Folder for removed files: " << rmv << endl;
        }
        else if(s == "-t") {
            time = stoi(argv[i + 1]);
            cout << "Scan time: " << time << endl;
        }
        else if(s == "--unix") {
            dirSep = '/';
            cout << "Unix directory separator" << endl;
        }
    }

    if(src == "") {
        cout << "ERROR: Missing source (option -s)! Exiting..." << endl;
        return 0;
    }
    else if(dst == "") {
        cout << "ERROR: Missing destination (option -d)! Exiting..." << endl;
        return 0;
    }
    else if(rmv == "") {
        cout << "ERROR: Missing folder for removed files (option -r)! Exiting..." << endl;
        return 0;
    }
    else if(time == 0) {
        cout << "ERROR: Missing scan time (option -t)! Exiting..." << endl;
        return 0;
    }

    while(true) {
        log << "Files in source folder:" << endl;
        for(filesystem::directory_entry entry : filesystem::recursive_directory_iterator(src)) {
            DirectoryEntry e;
            e.entry = entry;
            e.path = getPath(entry.path(), src);
            allFilesNow.push_back(e);
            log << "\t" << e.path << endl;
        }
        log << "Files in destination folder:" << endl;
        for(filesystem::directory_entry entry : filesystem::recursive_directory_iterator(dst)) {
            DirectoryEntry e;
            e.entry = entry;
            e.path = getPath(entry.path(), dst);
            allFilesBefore.push_back(e);
            log << "\t" << e.path << endl;
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
                else if(!compareFiles(allFilesNow[i].entry.path(), allFilesBefore[pos].entry.path())) {
                    toCopyFiles.push_back(allFilesNow[i]);
                    log << "To copy because different content: " << allFilesNow[i].path << endl;
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
        //Copy files
        size = toCopyFiles.size();
        for(Uint32 i = 0; i < size; i++) {
            if(toCopyFiles[i].entry.is_directory()) { //Copy directory
                filesystem::path dstPath(dst + dirSep + toCopyFiles[i].path);
                try {
                    filesystem::copy(toCopyFiles[i].entry.path(), dstPath, copyOptions);
                    log << "Copied folder: " << toCopyFiles[i].path << endl;
                    cpdFiles++;
                }
                catch(exception e) {}
            }
            else { //Copy file
                filesystem::path dstPath(dst + dirSep + toCopyFiles[i].path);
                try {
                    filesystem::copy_file(toCopyFiles[i].entry.path(), dstPath, copyOptions);
                    log << "Copied file: " << toCopyFiles[i].path << endl;
                    cpdFiles++;
                }
                catch(exception e) {
                    filesystem::remove(dstPath);
                    filesystem::copy_file(toCopyFiles[i].entry.path(), dstPath, copyOptions);
                    log << "Copied file: " << toCopyFiles[i].path << endl;
                    cpdFiles++;
                }
            }
        }
        //Remove files
        size = toRemoveFiles.size();
        for(Uint32 i = 0; i < size; i++) {
            filesystem::path rmvPath(rmv + dirSep + toRemoveFiles[i].path);
            try {
                filesystem::rename(toRemoveFiles[i].entry.path(), rmvPath);
                rmvdFiles++;
            }
            catch(exception e) {
                try {
                    filesystem::remove(rmvPath);
                    filesystem::rename(toRemoveFiles[i].entry.path(), rmvPath);
                    rmvdFiles++;
                }
                catch(exception e) {
                    filesystem::path dirPath(getDirectory(rmv + dirSep + toRemoveFiles[i].path));
                    filesystem::create_directories(dirPath);
                    filesystem::rename(toRemoveFiles[i].entry.path(), rmvPath);
                    rmvdFiles++;
                }
            }
        }

        cout << "Check is finished, " << cpdFiles << " files copied, " << rmvdFiles << " files removed."<< endl;

        allFilesNow.clear();
        allFilesBefore.clear();
        toCopyFiles.clear();
        toRemoveFiles.clear();
        cpdFiles = 0;
        rmvdFiles = 0;
        sleep(time);
    }

    log.close();

    return 0;
}

bool search(DirectoryEntry &entry, vector<DirectoryEntry> &vector, Uint32 &pos) {
    Uint32 size = vector.size();
    for(Uint32 i = 0; i < size; i++) {
        if(entry.path == vector[i].path) {
            pos = i;
            return true;
        }
    }
    return false;
}

string getPath(filesystem::path entryPath, string &from) {
    string path;
    path = entryPath.string();
    path = path.replace(0, from.size() + 1, "");
    return path;
}

string getDirectory(string path) {
    Int32 pos0 = 0, pos1 = 0;
    while(pos1 != string::npos) {
        pos1 = path.find(dirSep, pos0 + 1);
        if(pos1 != string::npos) {
            pos0 = pos1;
        }
    }
    return path.substr(0, pos0);
}

bool compareFiles(filesystem::path path1, filesystem::path path2) {
    ifstream file1(path1.string(), ifstream::binary | ifstream::ate);
    ifstream file2(path2.string(), ifstream::binary | ifstream::ate);
    if(file1.fail() || file2.fail()) return false;
    if(file1.tellg() != file2.tellg()) return false;
    file1.seekg(0, ifstream::beg);
    file2.seekg(0, ifstream::beg);
    return equal(istreambuf_iterator<char>(file1.rdbuf()),
        istreambuf_iterator<char>(),
        istreambuf_iterator<char>(file2.rdbuf()));
}