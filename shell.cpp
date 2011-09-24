#include <iostream>
#include <string>
#include <sstream>
#include <vector>

#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>

using namespace std;

vector<string> get_tokens(string);
void execute(vector<string>);
char** getArrOfCharArrs(vector<string>);
void handleErrors();

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
        char** array = getArrOfCharArrs(args);

        execvp(array[0],array);
        handleErrors();
    }else{
        //parent
        if( !background ){
            wait(NULL);
        }
    }
}

char** getArrOfCharArrs(vector<string> args){
    char** arry = new char*[args.size()];

    for(int i = 0; i < args.size(); i++){
        arry[i] = new char[args[i].length()+1];
        strcpy(arry[i],args[i].c_str());
    }

    return arry;
}

void handleErrors(){
    cout << "WAT";
    _exit(-1);
}
