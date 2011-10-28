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
#include <fcntl.h>

using namespace std;

vector<string> get_tokens(string);
void execute(vector<string>);
pid_t make_child(vector<string>&,int,int,int,int,int,int);
int find_token(vector<string>&,int,string);

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
    vector<pid_t> pid;
    bool background = false;
	int pipe_loc;
	int append;
	int new_fd[2];

    if(args[args.size()-1] == "&"){
        background = true;
        args.pop_back();
    }
    
	pipe_loc = find_token(args,0,"|");
	append = find_token(args,0,">>");

	if(pipe_loc == -1 && append == -1){
		pid.push_back( make_child(args,0,args.size()-1,-1,-1,-1,-1) );
	}else{
		if(pipe_loc != -1){
			//handle pipes

			if(append != -1){
			
			}
		}else{
			//only append
			//make pipe
			pipe(new_fd);

			pid.push_back(make_child(args,0,append-1,-1,-1,new_fd[0],new_fd[1]));
			pid.push_back(output(args,append+1,-1,-1,new_fd[0],new_fd[1]));

			close(new_fd[0]);
			close(new_fd[1]);
		}
	}
	
	//wait on children
	if( !background ){
		wait(NULL);
	}
}

pid_t make_child(vector<string> &args, int start, int end, int read, int close1, int close2, int write){
	pid_t pid = fork();
	if( pid == 0 ){
		if( close1 != -1 ){
			close(close1);
		}

		if( close2 != -2 ){
			close(close2);
		}

		if(read != -1){
			dup2(read,0);
		}
		
		if(write != -1){
			dup2(write,0);
		}

        vector<char *> c_style;
        for(int i = start; i <= end; i++){
           c_style.push_back(const_cast<char *>(args[i].c_str()));
        }
        c_style.push_back(NULL);

        execvp(c_style[0],&c_style[0]);
        perror("error: ");
        _exit(-1);
    }
	return pid;
}

int find_token(vector<string> &vec, int start, string token){
	for(int i = start; i < vec.size(); i++){
		if(vec[i] == token){
			return i;
		}
	}
	return -1;
}
