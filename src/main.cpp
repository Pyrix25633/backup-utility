#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <unistd.h>

using namespace std;

int main(int argc, char* argv[]) {
    ofstream log("log.txt");
    if(argc < 3) return 1;
    string src = argv[1], dst = argv[2];
    vector<filesystem::directory_entry> allFilesNow, allFilesBefore, toCopyFiles, toRemoveFiles;

    while(true) {
        allFilesNow.clear();
        for(filesystem::directory_entry entry : filesystem::directory_iterator(src)) {
            allFilesNow.push_back(entry);
        }

        cout << "Check is finished" << endl;
        sleep(60);
    }

    return 0;
}