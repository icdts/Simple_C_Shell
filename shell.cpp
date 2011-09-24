#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

using namespace std;

vector<string> get_tokens(string);
void execute(vector<string>);

int main(){
    bool should_continue = true;
    string line;
    vector<string> args;

    while(should_continue){
        cout << "(myshell)>";
        getline(cin, line);

        args = get_tokens(line);

        if( args.size() >= 1 ){
            if(args[0] == "exit" || args[0] == "quit"){
                should_continue = false;
            }else{
                execute(args);
            }
        }

        args.clear();
    }
}

vector<string> get_tokens(string data){
    istringstream ss(data);
    string tmp;
    vector<string> tokens;

    ss >> tmp;
    while(ss){
        tokens.push_back(tmp);
        ss >> tmp;
    }

    return tokens;
}

void execute(vector<string> args){
    pid_t pid = fork();
    bool background = false;
    
    if(args[args.size()-1] == "&"){
        background = true;

        args.pop_back();
    }
    

    if( pid == 0 ){
        //child
        vector<char *> c_style;
        for(int i = 0; i < args.size(); i++){
           c_style.push_back(const_cast<char *>(args[i].c_str()));
        }
        c_style.push_back(NULL);

        execvp(c_style[0],&c_style[0]);
        perror("error: ");
        _exit(-1);
    }else{
        //parent
        if( !background ){
            wait(NULL);
        }
    }
}
