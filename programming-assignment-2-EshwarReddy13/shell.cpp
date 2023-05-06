#include <iostream>
#include <sstream>
#include <bits/stdc++.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fstream>
#include <sys/stat.h>
#include <fcntl.h>


#include <vector>
#include <string>

#include "Tokenizer.h"

#include <ctime>

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

//gnc read line

using namespace std;

string getMonth(int num){
    string ans;
    switch(num){
        case 1:
            ans = "Jan";
            break;
        case 2:
            ans = "Feb";
            break;
        case 3:
            ans = "Mar";
            break;
        case 4:
            ans = "Apr";
            break;
        case 5:
            ans = "May";
            break;
        case 6:
            ans = "Jun";
            break;
        case 7:
            ans = "July";
            break;
        case 8:
            ans = "Aug";
            break;
        case 9:
            ans = "Sep";
            break;
        case 10:
            ans = "Oct";
            break;
        case 11:
            ans = "Nov";
            break;
        case 12:
            ans = "Dec";
            break;
    }
    return ans;
}

int main () {
    //rl_bind_key('\t', rl_complete);
    // need date/time, username, and absolute path to current dir
    vector<pid_t> bgPids;
    char buff[1024];
    string currentDir = getcwd(buff,sizeof(buff));
    chdir("..");
    string prevDir = getcwd(buff,sizeof(buff));
    chdir(currentDir.c_str());

    for (;;) {
        for(size_t i = 0; i < bgPids.size(); i++){
            if(waitpid(bgPids[i],0,WNOHANG) == bgPids[i]){ 
                bgPids.erase(bgPids.begin() +i);
                i--;
            }
        }
        // need date/time, username, and absolute path to current dir
        bool bg = false;
        bool inputoutput = false;
        bool cdFlag = false;

        int original_stdin = dup(STDIN_FILENO);
        int original_stdout = dup(STDOUT_FILENO);

        time_t now = time(0);
        tm* tim = localtime(&now);

        string mont = getMonth(tim->tm_mon);

        cout << RED << mont << " " << tim->tm_mday << " ";
        cout << tim->tm_hour << ":" << tim->tm_min << ":" << tim->tm_sec << " " << NC;


        string currUser = getenv("USER");
        cout << GREEN << currUser <<  ":" << NC;
        if(getcwd(buff,sizeof(buff))!=NULL){
            cout << BLUE << buff << NC;
        }else{
            cout << "Current directory cannout be found" << " ";
        }
        cout << YELLOW << "$" << NC << " ";
        
        // get user inputted command
        string input;
        getline(cin, input);

        if (input == "exit") {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        // get tokenized commands from user input
        Tokenizer tknr(input);
        if (tknr.hasError()) {  // continue to next prompt if input had an error
            continue;
            cout << "Has error";
        }

        // // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        for (auto cmd : tknr.commands) {
            for (auto str : cmd->args) {
                cerr << "|" << str << "| ";
            }
            if (cmd->hasInput()) {
                cerr << "in< " << cmd->in_file << " ";
            }
            if (cmd->hasOutput()) {
                cerr << "out> " << cmd->out_file << " ";
            }
            cerr << endl;
        }
        

        // fork to create child
        for(size_t i=0;i<tknr.commands.size();i++){
            //add_history(tknr.commands.at(i));
            char** args = new char*[tknr.commands.at(i)->args.size()+1];
            for(size_t j=0;j<tknr.commands.at(i)->args.size()+1;j++){
                if(j==tknr.commands.at(i)->args.size()){
                    args[j] = NULL;
                }else{
                    args[j] = ((char*) tknr.commands.at(i)->args.at(j).c_str());
                    if(tknr.commands.at(i)->args.at(j)==">" || tknr.commands.at(i)->args.at(j)=="<"){
                        args[j] = (char*) "--color=auto";
                    }
                    else{
                        args[j] = ((char*) tknr.commands.at(i)->args.at(j).c_str());
                    }
                    
                }
            }


            int fd[2];
            if(pipe(fd)==-1){
                return 1;
            }

            pid_t pid = fork();
            if (pid < 0) {  // error check
                perror("fork");
                exit(2);
            }
            if(tknr.commands.at(i)->isBackground()){
                bg = true;
            }
    
            if (pid == 0) {  // if child, exec to run command
                // run single commands with no arguments
                // if(bg){
                //     if(raise(SIGSTOP)!=0){
                //         cout << "Failed to send SIGSTOP signal to child process" << endl;
                //     }
                //     int status;
                //     waitpid(pid,&status,WUNTRACED);
                // }


                string commmandToExec = args[0];
                if(commmandToExec == "cd"){
                    cdFlag = true;
                    string direction = args[1];
                    currentDir = getcwd(buff,sizeof(buff));
                    if(direction=="-"){
                        chdir(prevDir.c_str());
                        prevDir = currentDir;
                    }else{
                        string directoryName = args[1];
                        chdir(directoryName.c_str());
                        prevDir = currentDir;
                    }
                }

                if(tknr.commands.at(i)->hasOutput()){
                    //cout << "Came here1" << endl;
                    string filename = tknr.commands.at(i)->out_file;
                    //cout << filename << endl;
                    int fds = open(filename.c_str(),O_WRONLY|O_CREAT,S_IWUSR|S_IRUSR);
                    dup2(fds,STDOUT_FILENO);
                    close(fds);
                }


                if(tknr.commands.at(i)->hasInput()){
                    //cout << "Came here2" << endl;
                    string filename = tknr.commands.at(i)->in_file;
                    //cout << filename << endl;
                    int fds = open(filename.c_str(),O_RDONLY,S_IWUSR|S_IRUSR);
                    dup2(fds,STDIN_FILENO);
                    close(fds);
                }

                if(!cdFlag && !inputoutput){
                    if(i<tknr.commands.size()-1){
                        dup2(fd[1],STDOUT_FILENO);
                        close(fd[0]);
                    }
                    if (execvp(args[0], args) < 0) {  // error check
                        perror("execvp");
                        exit(2);
                    }

                }

                
            }
            else {  // if parent, wait for child to finish

                if(bg){
                    bgPids.push_back(pid);
                }else{
                    if(i==tknr.commands.size()-1){
                        waitpid(pid,0,0);
                    }
                }
                
                dup2(fd[0],STDIN_FILENO);
                close(fd[1]);
            }

            delete[] args;
        }
        dup2(original_stdin, STDIN_FILENO);
        dup2(original_stdout, STDOUT_FILENO);
    }

    // for(auto it = bgPids.rbegin(); it != bgPids.rend(); ++it) {
    //     pid_t pidTemp = *it;
    //     kill(pidTemp, SIGCONT);
    //     int status;
    //     waitpid(pidTemp, &status, WUNTRACED);
    // }
    return 0;
}