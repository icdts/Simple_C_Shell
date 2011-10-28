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
pid_t output(vector<string>&,int,int,int,int,int);
int find_token(vector<string>&,int,string);

int main(){
    bool should_continue = true;
    string line;
    vector<string> args;

    while(should_continue){
        std::cout << "(myshell)>";
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
	vector<int> to_close;
    bool background = false;
	int pipe_loc;
	int last_pipe_loc = -1;
	int append;
	int new_fd[2];
	int last_fd[2];

	new_fd[0] = -1;
	new_fd[1] = -1;
	last_fd[0] = -1;
	last_fd[1] = -1;

    if(args[args.size()-1] == "&"){
        background = true;
        args.pop_back();
    }
    
	pipe_loc = find_token(args,0,"|");
	append = find_token(args,0,">>");

	if(pipe_loc == -1 && append == -1){
		pid.push_back( make_child(args,0,args.size()-1,-1,-1,-1,-1) );
	}else{
		
		//handle pipes
		while( pipe_loc != -1 ){
			pipe(new_fd);
		
			cout << "making child with pipes:" << endl;
			cout << "	last_fd = [" << last_fd[0] << "," << last_fd[1] << "]" << endl;
			cout << "	new_fd = [" << new_fd[0] << "," << new_fd[1] << "]" << endl;
			pid.push_back( make_child(args,last_pipe_loc+1,pipe_loc-1,last_fd[0],last_fd[1],new_fd[0],new_fd[1]) );
			last_pipe_loc = pipe_loc;
			pipe_loc = find_token(args,last_pipe_loc+1,"|");

			//parent won't use either end
			to_close.push_back(new_fd[0]);
			to_close.push_back(new_fd[1]);

			//new is no longer new, it has something attached to it
			last_fd[0] = new_fd[0];
			last_fd[1] = new_fd[1];
			new_fd[0] = -1;
			new_fd[1] = -1;
		}

		if(append != -1){
			pipe(new_fd);

			pid.push_back(make_child(args,last_pipe_loc+1,append-1,last_fd[0],last_fd[1],new_fd[0],new_fd[1]));
			pid.push_back(output(args,append+1,-1,-1,new_fd[0],new_fd[1]));

			//parent won't use either end
			to_close.push_back(new_fd[0]);
			to_close.push_back(new_fd[1]);
			
			//new is no longer new, it has something attached to it
			last_fd[0] = new_fd[0];
			last_fd[1] = new_fd[1];
			new_fd[0] = -1;
			new_fd[1] = -1;
		}else{
			cout << "making child with pipes:" << endl;
			cout << "	last_fd = [" << last_fd[0] << "," << last_fd[1] << "]" << endl;
			cout << "	new_fd = [" << new_fd[0] << "," << new_fd[1] << "]" << endl;
			pid.push_back(make_child(args,last_pipe_loc+1,args.size()-1,last_fd[0],last_fd[1],new_fd[0],new_fd[1]));
			last_fd[0] = new_fd[0];
			last_fd[1] = new_fd[1];
			new_fd[0] = -1;
			new_fd[1] = -1;
		}
	}

	for(int i=0; i<to_close.size(); i++){
		close(to_close[i]);
	}

	//wait on children
	if( !background ){
		for(int i=0; i<pid.size(); i++){
			waitpid(pid[i],NULL,0);
		}
	}
}

pid_t make_child(vector<string> &args, int start, int end, int read, int close1, int close2, int write){
	pid_t pid = fork();
	if( pid == 0 ){
		if( close1 != -1 ){
			//close(close1);
		}

		if( close2 != -2 ){
			//close(close2);
		}

		if(read != -1){
			dup2(read,STDIN_FILENO);
		}
		
		if(write != -1){
			dup2(write,STDOUT_FILENO);
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

pid_t output(vector<string> &args, int filename_loc, int start, int end, int read_end, int close_end){
	pid_t pid = fork();
	if( pid == 0 ){
		int fd;
		int count;
		char c;

		fd=open(args[filename_loc].c_str(),O_RDWR|O_CREAT,0600);

		dup2(read_end,STDIN_FILENO); 
		close(close_end);

		while ((count=read(0,&c,1))>0) 
			write(fd,&c,1);
		close(fd);

		_exit(1);
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
