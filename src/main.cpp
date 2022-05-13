#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <iterator>
#include <algorithm>
#include <unistd.h>
#include <cmath>

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
bool isInExtensionsList(string path, vector<string> extensionsList);
string getHumanReadableSize(Uint64 size);
string getProgressBar(Uint64 now, Uint64 total, string totalString);

int main(int argc, char* argv[]) {
    ofstream log("log.txt");
    if(argc < 3) return 1;
    string src = "", dst = "", rmv = "", ext = "";
    vector<DirectoryEntry> allFilesNow, allFilesBefore, toCopyFiles, toRemoveFiles;
    vector<string> extensionsList;
    Uint32 size, pos, time = 0, cpdFiles = 0, rmvdFiles = 0;
    filesystem::copy_options copyOptions = filesystem::copy_options::overwrite_existing
        | filesystem::copy_options::recursive | filesystem::copy_options::directories_only;
    string version = "1.2.0";
    bool fError = false;
    Uint64 sizeNow, sizeToCopy, sizeToRemove;
    string sizeToCopyString, sizeToRemoveString;

    cout << "Backup Utility version " << version << endl;

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
        else if(s == "-e") {
            ext = argv[i + 1];
            cout << "File with list of extensions to check for content changes: " << ext << endl;
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

    if(ext != "") {
        ifstream extensionsFile(ext);
        char cs[20];
        while(!extensionsFile.eof()) {
            extensionsFile.getline(cs, 20);
            extensionsList.push_back(cs);
        }
    }

    while(true) {
        cout << "Starting scan..." << endl;
        log << "Starting scan..." << endl;
        for(filesystem::directory_entry entry : filesystem::recursive_directory_iterator(src)) {
            DirectoryEntry e;
            e.entry = entry;
            e.path = getPath(entry.path(), src);
            allFilesNow.push_back(e);
        }
        size = allFilesNow.size();
        cout << "Source directory scanned: " << size << " file" << ((size == 1) ? "" : "s") << " found" << endl;
        log << "Source directory scanned: " << size << " file" << ((size == 1) ? "" : "s") << " found" << endl;
        for(filesystem::directory_entry entry : filesystem::recursive_directory_iterator(dst)) {
            DirectoryEntry e;
            e.entry = entry;
            e.path = getPath(entry.path(), dst);
            allFilesBefore.push_back(e);
        }
        size = allFilesBefore.size();
        cout << "Destination directory scanned: " << size << " file" << ((size == 1) ? "" : "s") << " found; comparing the two lists..." << endl;
        log << "Destination directory scanned: " << size << " file" << ((size == 1) ? "" : "s") << " found; comparing the two lists..." << endl;
        //Files to copy
        size = allFilesNow.size();
        sizeToCopy = 0;
        for(Uint32 i = 0; i < size; i++) {
            bool toCopy = false;
            DirectoryEntry e = allFilesNow[i];
            if(!search(e, allFilesBefore, pos)) {
                toCopyFiles.push_back(e);
                cout << "\tTo copy because not there: " << e.path << endl;
                log << "\tTo copy because not there: " << e.path << endl;
                toCopy = true;
            }
            else if(!e.entry.is_directory()) {
                if(e.entry.file_size() != allFilesBefore[pos].entry.file_size()) {
                    toCopyFiles.push_back(e);
                    cout << "\tTo copy because different size: " << e.path << endl;
                    log << "\tTo copy because different size: " << e.path << endl;
                    toCopy = true;
                }
                else if(isInExtensionsList(e.path, extensionsList)) {
                    if(!compareFiles(e.entry.path(), allFilesBefore[pos].entry.path())) {
                        toCopyFiles.push_back(e);
                        cout << "\tTo copy because different content: " << e.path << endl;
                        log << "\tTo copy because different content: " << e.path << endl;
                        toCopy = true;
                    }
                }
            }
            if(toCopy) {
                if(!e.entry.is_directory()) {
                    sizeToCopy += e.entry.file_size();
                }
            }
        }
        //Files to delete
        size = allFilesBefore.size();
        sizeToRemove = 0;
        for(Uint32 i = 0; i < size; i++) {
            DirectoryEntry e = allFilesBefore[i];
            if(!search(e, allFilesNow, pos)) {
                toRemoveFiles.push_back(e);
                cout << "\tTo remove: " << e.path << endl;
                log << "\tTo remove: " << e.path << endl;
                if(!e.entry.is_directory()) sizeToRemove += e.entry.file_size();
            }
        }
        cout << "Lists compared, starting copy-delete operation..." << endl;
        log << "Lists compared, starting copy-delete operation..." << endl;
        //Copy files
        size = toCopyFiles.size();
        sizeNow = 0;
        sizeToCopyString = getHumanReadableSize(sizeToCopy);
        cout << size << " file" << ((size == 1) ? "" : "s") << " to copy (" << sizeToCopyString << ")" << endl;
        log << size << " file" << ((size == 1) ? "" : "s") << " to copy (" << sizeToCopyString << ")" << endl;
        if(size > 0) cout << endl;
        for(Uint32 i = 0; i < size; i++) {
            DirectoryEntry e = toCopyFiles[i];
            if(e.entry.is_directory()) { //Copy directory
                filesystem::path dstPath(dst + dirSep + e.path);
                try {
                    filesystem::copy(e.entry.path(), dstPath, copyOptions);
                    fError = false;
                }
                catch(exception ex) {fError = true;}
                if(!fError) {
                    cout << "\x1b[1A" << "\x1b[2K";
                    log << "\tCopied folder: " << e.path << endl;
                    cout << "\tCopied folder: " << e.path << endl;
                    cout << getProgressBar(sizeNow, sizeToCopy, sizeToCopyString) << endl;
                    cpdFiles++;
                }
            }
            else { //Copy file
                filesystem::path dstPath(dst + dirSep + e.path);
                try {
                    filesystem::copy_file(e.entry.path(), dstPath, copyOptions);
                    fError = false;
                }
                catch(exception ex) {
                    try {
                        filesystem::remove(dstPath);
                        filesystem::copy_file(e.entry.path(), dstPath, copyOptions);
                        fError = false;
                    }
                    catch(exception ex) {fError = true;}
                    
                }
                if(!fError) {
                    sizeNow += e.entry.file_size();
                    cout << "\x1b[1A" << "\x1b[2K";
                    log << "\tCopied file: " << e.path << endl;
                    cout << "\tCopied file: " << e.path << endl;
                    cout << getProgressBar(sizeNow, sizeToCopy, sizeToCopyString) << endl;
                    cpdFiles++;
                }
            }
        }
        //Remove files
        size = toRemoveFiles.size();
        sizeToRemoveString = getHumanReadableSize(sizeToRemove);
        cout << size << " file" << ((size == 1) ? "" : "s") << " to remove (" << sizeToRemoveString << ")" << endl;
        log << size << " file" << ((size == 1) ? "" : "s") << " to remove (" << sizeToRemoveString << ")" << endl;
        for(Uint32 i = 0; i < size; i++) {
            DirectoryEntry e = toRemoveFiles[i];
            if(!e.entry.is_directory()) {
                filesystem::path rmvPath(rmv + dirSep + e.path);
                try {
                    filesystem::rename(e.entry.path(), rmvPath);
                    fError = false;
                }
                catch(exception ex) {
                    try {
                        filesystem::remove(rmvPath);
                        filesystem::rename(e.entry.path(), rmvPath);
                        fError = false;
                    }
                    catch(exception ex) {
                        try {
                            filesystem::path dirPath(getDirectory(rmv + dirSep + e.path));
                            filesystem::create_directories(dirPath);
                            filesystem::rename(e.entry.path(), rmvPath);
                            fError = false;
                        }
                        catch(exception ex) {fError = true;}
                    }
                }
                if(!fError) {
                    log << "\tRemoved file: " << e.path << endl;
                    cout << "\tRemoved file: " << e.path << endl;
                    rmvdFiles++;
                }
            }
        }
        //Remove folders
        size = toRemoveFiles.size();
        for(Uint32 i = 0; i < size; i++) {
            DirectoryEntry e = toRemoveFiles[i];
            if(e.entry.is_directory()) {
                filesystem::path rmvPath(rmv + dirSep + e.path);
                try {
                    filesystem::rename(e.entry.path(), rmvPath);
                    fError = false;
                }
                catch(exception ex) {
                    try {
                        filesystem::path dirPath(getDirectory(rmv + dirSep + e.path));
                        filesystem::create_directories(dirPath);
                        filesystem::rename(e.entry.path(), rmvPath);
                        fError = false;
                    }
                    catch(exception ex) {
                        try {
                            filesystem::remove(e.entry.path());
                            fError = false;
                        }
                        catch(exception ex) {fError = true;}
                    }
                }
                if(!fError) {
                    log << "\tRemoved folder: " << e.path << endl;
                    cout << "\tRemoved folder: " << e.path << endl;
                    rmvdFiles++;
                }
            }
        }

        cout << "Operation completed, " << cpdFiles << " file" << ((cpdFiles == 1) ? "" : "s") << " copied (" << sizeToCopyString << "), "
            << rmvdFiles << " file" << ((rmvdFiles == 1) ? "" : "s") << " removed (" << sizeToRemoveString << ")"<< endl;
        cout << "Waiting " << time << " seconds from now, process can be terminated with 'Ctrl + C' before the next scan" << endl;
        log << "Operation completed, " << cpdFiles << " file" << ((cpdFiles == 1) ? "" : "s") << " copied (" << sizeToCopyString << "), "
            << rmvdFiles << " file" << ((rmvdFiles == 1) ? "" : "s") << " removed (" << sizeToRemoveString << ")"<< endl;

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

bool isInExtensionsList(string path, vector<string> extensionsList) {
    Int32 pos0 = -1, pos1 = 0;
    Uint8 size = extensionsList.size();
    string extension;
    while(pos1 != string::npos) {
        pos1 = path.find(dirSep, pos0 + 1);
        if(pos1 != string::npos) {
            pos0 = pos1;
        }
    }
    path = path.substr(pos0 + 1);
    pos0 = -1, pos1 = 0;
    while(pos1 != string::npos) {
        pos1 = path.find('.', pos0 + 1);
        if(pos1 != string::npos) {
            pos0 = pos1;
        }
    }
    extension = path.substr(pos0 + 1);
    for(Uint8 i = 0; i < size; i++) {
        if(extensionsList[i] == extension) {
            return true;
        }
    }
    return false;
}

string getHumanReadableSize(Uint64 size) {
    Uint16 unit = 1024;
    if(size < unit) {
        return to_string(size) + "B";
    }
    Uint64 KiBytes = floor(float(size) / unit);
    Uint16 Bytes = size % unit;
    if(KiBytes < unit) {
        return to_string(KiBytes) + "." + to_string(Bytes) + "KiB";
    }
    Uint32 MiBytes = floor(float(KiBytes) / unit);
    KiBytes %= unit;
    if(MiBytes < unit) {
        return to_string(MiBytes) + "." + to_string(KiBytes) + "MiB";
    }
    Uint16 GiBytes = floor(float(MiBytes) / unit);
    MiBytes %= unit;
    return to_string(GiBytes) + "." + to_string(MiBytes) + "GiB";
}

string getProgressBar(Uint64 now, Uint64 total, string totalString) {
    string s = "[";
    int percent = float(now) / total * 100;

    for(unsigned int i = 1; i <= 100; i++) {
        if(i <= percent) {
            s += "█";
        }
        else {
            s += "░";
        }
    }

    s += "] " + to_string(percent) + "% (" + getHumanReadableSize(now) + "/" + totalString + ")";

    return s;
}