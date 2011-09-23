#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

vector<string> get_tokens(string);
void execute(vector<string> args);

int main(){
    bool should_continue = true;
    string line;
    vector<string> args;

    while(should_continue){
        cout << "(myshell)>";
        getline(cin, line);

        args = get_tokens(line);

        if(args.size() >= 1 && args[0] == "exit"){
            should_continue = false;
        }else{
            execute(args);
        }
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

    if( pid == 0 ){
        //child
        int *array = new int[args.size()];
        copy(args.begin(),args.end(),array);

        exec("/usr/bin/" + args[0], array);
    }else{
        //parent
        if( !(args[args.size()-1] == "&") ){
            wait(NULL);
        }
    }
}
