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
    const string & dir, 
    const string &  file,
    const bool & display
) {
    if (display) { system(("ffplay -hide_banner -loglevel quiet -autoexit \"" + dir + file + "\" 1>/dev/null 2>/dev/null").c_str()); }
    else { system(("ffplay -hide_banner -loglevel quiet -nodisp -autoexit \"" + dir + file + "\" 1>/dev/null 2>/dev/null").c_str()); }
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
    string* pdir, 
    vector<string>* pfiles
) {
    *pfiles = {};
    cout << "Please specify a directory from which to play music.\n" << "\033[1;34mmusic\033[0m" << " >>> ";
    getline(cin, *pdir);
    if ((*pdir)[(*pdir).size() - 1] != '/') { *pdir += '/'; }
    DIR *dr;
    struct dirent *en;
    dr = opendir((*pdir).c_str());
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
                (*pfiles).push_back(en->d_name);
            }
        }
        closedir(dr);
    } 
    else {
        printf(
                "Could not open directory! "
                "Please make sure it exists and enter it again.\n"
        );
        listfiles(pdir, pfiles);
    }
    if ((*pfiles).size() == 0) {
        printf("The directory specified has no mp3 files! Please enter a directory with mp3 files.\n");
        listfiles(pdir, pfiles);
    }
}

void listen ( 
    vector<string>* pfiles, 
    int* pfile_index, 
    bool* pterminate, 
    string* pindex_mode, 
    string* pdir,
    bool* ppause,
    bool* command_control,
    bool* display
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
        else if (command == "display") {
            if (*display) {
                *display = false;
                cout << "Display mode has been turned off.\n";
            }
            else {
                *display = true;
                cout << "Display mode has been turned on!\n";
            }
        }
        else if (command == "once") {
           if (*pindex_mode != "once") {
               *pindex_mode = "once"; 
                printf("This track will play once more!\n");
           } 
           else {
               *pindex_mode = "next";
                printf("Default mode enabled.\n");
           }
        }
        else if (command == "forward") {
            *pindex_mode = "next";
            printf("Default mode enabled.");
        }
        else if (command == "reverse") {
            if (*pindex_mode != "reverse") {
                *pindex_mode = "reverse";
                printf("Reverse mode enabled!\n");
            }
            else {
                *pindex_mode = "next";
                printf("Default mode enabled.\n");
            }
        }
        else if (command == "loop") {
             if (*pindex_mode != "loop") {
                   *pindex_mode = "loop"; 
                    printf("Permanent looping mode enabled!\n");
               } 
               else {
                   *pindex_mode = "next";
                    printf("Default mode enabled.\n");
               }
        }
        else if (command == "null") {}
        else if (command == "tracks") {
            for (int i = 0; i < (*pfiles).size(); i++) {
                if (i == *pfile_index) {
                    cout << "\033[1;33m" << (*pfiles)[mod(i, (*pfiles).size())] << "\033[0m" << "\n";
                }
                else {
                    cout << (*pfiles)[mod(i, (*pfiles).size())] << "\n";
                }
            }
        }
        else if (command == "track") {
            cout << (*pfiles)[mod(*pfile_index, (*pfiles).size())] << std::endl;
        }
        else if (command == "mvdir") {
            listfiles(pdir, pfiles);
            cout << "Directory successfully changed to '"<< *pdir << "'.\n";
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
                        "forward -- the program will traverse through tracks in the default manner\n\t"
                        "reverse -- the program will play tracks in reverse order\n\t"
                        "loop -- the program will play tracks over and over again\n\t"
                        "tracks -- prints the current track list with the currently playing track highlighted\n\t"
                        "track -- prints the currently playing track\n\t"
                        "mvdir -- changes the directory from which the tracks are played\n\t"
                        "pause -- pause or unpause the player without retaining the location in the track, as opposed to the pause in the display\n\t"
                        "display -- toggle the ffplay display\n\t"
                        "null -- this command doesn't do anything\n"
            ); 
        }
    }
}

int main() {
    srand(time(NULL));
    string dir;
    vector<string> files;
    listfiles(&dir, &files);
    int file_index = 0;
    bool pause = false;
    bool command_control = false;
    bool terminate = false;
    bool display = false;
    string index_mode = "next";
    thread listener (
        listen, 
        &files, 
        &file_index, 
        &terminate, 
        &index_mode, 
        &dir, 
        &pause, 
        &command_control,
        &display
    );
    while (!terminate) {
        if (!pause) {
            thread player ( play, dir, files[mod (file_index, files.size())], display );
            player.join();
            if (!command_control) {
                if (index_mode == "next") {
                    file_index++;
                }
                else if (index_mode == "once") {
                    index_mode = "next";
                }
                else if (index_mode == "reverse") {
                    file_index--;
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
        }
        else { this_thread::sleep_for(chrono::nanoseconds(100000000)); }
    }
    listener.join();
}

/* 
 * TODO:
 *  - Make the ffplay display stay on the window of the terminal instead of following users from window to window. You might want to see if you can "embed" the ffplay window, as there are
 *      probably some applications that use ffplay as a player within their own graphical interfaces. If you can control the window, I'd choose a small window in the upper or bottom right
 *      corner of the screen.
 *  - Add the ability to add a space and then a number to the 'tracks' command, the number specifying how many next tracks to show. Use error handling to prevent non-numbers. Out of ranges
 *      should be taken care of by the modulus, and negative numbers should show the reverse order. 'tracks 0' should have the same functionality as 'track', but that's okay because
 *      'track' is more intuitive because the word isn't plural.
 *  - Consider making your own mp3 player so you have more control over the video controls and the interfacing with your application.
 *  - Format the unrecognized command print statement to also show the invalid command.
 *  - Should echoing the playing track each time a new one is played be a thing? If so, add an option to toggle the prints that tell you which track is playing automatically.
 *  - Should we make certain commands, like 'stop', work during directory selection prompts?
 *  - Implement the abilities to search for tracks and play them. Also, potentially add a directory search option.
 *  - Maybe make it so mvdir can also operate with spaces. So, "mvdir {dir}".
 */
