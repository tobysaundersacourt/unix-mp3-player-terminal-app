#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <iostream>
#include <thread>
#include <chrono>
using namespace std;

int mod (int a, int b) {
    a = a % b;
    if (a < 0) {
        return a + b;
    }
    return a;
}

void play (
    string dir, 
    string file
) {
    system(("ffplay -hide_banner -loglevel quiet -nodisp -autoexit \"" + dir + "/" + file + "\" 1>/dev/null 2>/dev/null").c_str());
}

void shuffle (vector<string> & files, int* pfile_index) {
    for (int i = 0; i < files.size(); i++) {
        string tmp = files[i];
        int random_index = rand() % files.size();
        files[i] = files[random_index];
        files[random_index] = tmp;
        if (i == *pfile_index) {
            *pfile_index = random_index;
        } 
        else if (random_index == *pfile_index) {
            *pfile_index = i;
        }
    }
}

void listfiles (
    string dir, 
    vector<string> & files
) {
    DIR *dr;
    struct dirent *en;
    dr = opendir(dir.c_str());
    if (dr) {
        while ((en = readdir(dr)) != NULL) {
            string name = en->d_name;
            if (
                name.size() > 3
                && name[name.size() - 4] == '.'
                && name[name.size() - 3] == 'm'
                && name[name.size() - 2] == 'p'
                && name[name.size() - 1] == '3'
            ) 
            {
                files.push_back(en->d_name);
            }
        }
        closedir(dr);
    } else {
        printf(
                "Could not open directory! "
                "Please make sure it exists and you have spelled it correctly.\n"
        );
        exit(0);
    }
    if (files.size() == 0) {
        printf("The directory specified has no mp3 files! Please note this player scans for mp3 files.\n");
        exit(0);
    }
}

void listen ( 
    vector<string>* pfiles, 
    int* pfile_index, 
    bool* pterminate, 
    string* pindex_mode, 
    string* pdir,
    bool* ppause,
    bool* command_control
) {
    string command;
    while (true) {
        cout << "\033[1;34mmusic\033[0m" << " >>> "; 
        getline(cin, command);
        if (command == "stop") {
            system("pkill ffplay");
            *pterminate = true;
            return;
        }
        else if (command[0] == '!') {
            command[0] = ' ';
            system(command.c_str());
        }
        else if (command == "next") {
            system("pkill ffplay");
            (*pfile_index)++;
            *command_control = true;
        }
        else if (command == "previous") {
            system("pkill ffplay");
            (*pfile_index)--;
            *command_control = true;
        }
        else if (command == "random") {
            system("pkill ffplay");
            *pfile_index = rand() % (*pfiles).size();
            *command_control = true;
        }
        else if (command == "shuffle") {
           shuffle ( *pfiles, pfile_index );
           printf("The track list has been shuffled!\n");
        }
        else if (command == "once") {
           if (*pindex_mode != "once") {
               *pindex_mode = "once"; 
                printf("This track will play once more!\n");
           } 
           else {
               *pindex_mode = "next";
                printf("The next track will play once this one has concluded.\n");
           }
        }
        else if (command == "loop") {
             if (*pindex_mode != "loop") {
                   *pindex_mode = "loop"; 
                    printf("Permanent looping mode enabled!\n");
               } 
               else {
                   *pindex_mode = "next";
                    printf("The next track will play once this one has concluded.\n");
               }
        }
        else if (command == "null") {}
        else if (command == "tracks") {
            for (int i = 0; i < (*pfiles).size(); i++) {
                if (i == *pfile_index) {
                    cout << "\033[1;33m" << (*pfiles)[i] << "\033[0m" << "\n";
                }
                else {
                    cout << (*pfiles)[i] << "\n";
                }
            }
        }
        else if (command == "playing") {
            cout << (*pfiles)[*pfile_index] << std::endl;
        }
        else if (command == "mvdir") {
            cout << "Please specify a directory from which to play music.\n" << "\033[1;34mmusic\033[0m" << " >>> ";
            getline(cin, *pdir);
        }
        else if (command == "pause") {
            if (!(*ppause)) {
                cout << "Pausing player. Run the same command to unpause.\n";
                *ppause = true;
                system("pkill ffplay");
                *command_control = true;
            }
            else {
                cout << "Unpausing player.\n";
                *ppause = false;
            }
        }
        else {
            printf(
                    "Unrecognized command input! Here is a list of valid commands:\n\t"
                        "!cmd -- executes a terminal command (call with your command replacing 'cmd')\n\t"
                        "stop -- terminates the program\n\t"
                        "next -- plays the next mp4 file in the file list\n\t"
                        "previous -- plays the previous mp4 in the file list\n\t"
                        "random -- plays a random mp4 file from the file list\n\t"
                        "shuffle -- randomly shuffles the file order without terminating the current ffplay process\n\t"
                        "once -- the program will play the currently playing mp4 file once again after it has concluded\n\t"
                        "loop -- the program will play tracks over and over again\n\t"
                        "tracks -- prints the current track list with the current track highlighted\n\t"
                        "playing -- prints the currently playing track\n\t"
                        "mvdir -- changes the directory from which the tracks are played\n\t"
                        "pause -- pause or unpause the player without retaining the location in the track\n\t"
                        "null -- this command doesn't do anything\n"
            ); 
        }
    }
}

int main() {
    cout << "Please specify a directory from which to play music.\n" << "\033[1;34mmusic\033[0m" << " >>> ";
    string dir;
    getline(cin, dir);
    srand(time(NULL));
    vector<string> files = {};
    listfiles(dir, files);
    string tmpdir = "";
    int file_index = 0;
    bool pause = false;
    int file_index_increment = 1;
    bool command_control = false;
    bool terminate = false;
    string index_mode = "next";
    thread listener (
            listen, 
            &files, 
            &file_index, 
            &terminate, 
            &index_mode, 
            &tmpdir, 
            &pause, 
            &command_control
    );
    while (!terminate) {
        if (!pause) {
            thread player ( play, dir, files[mod (file_index, files.size())] );
            player.join();
            if (!command_control) {
                if (index_mode == "next") {
                    file_index++;
                }
                else if (index_mode == "once") {
                    index_mode = "next";
                }
                else if (index_mode == "loop") {
                    // pass
                }
                else {
                    cerr << "INTERNAL ERROR: INVALID INDEX MODE\n";
                    exit(0);
                }
            }
            else {
                command_control = false;
            }
            if (tmpdir != "") {
                files = {};
                listfiles(tmpdir, files);
                dir = tmpdir;
                tmpdir = "";
            }
        }
        else { this_thread::sleep_for(chrono::nanoseconds(100000000)); }
    }
    listener.join();
}
