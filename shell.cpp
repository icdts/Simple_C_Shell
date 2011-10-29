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

#define READ 0
#define WRITE 1

using namespace std;

vector<string> get_tokens(string);
void execute(vector<string>);
void handle_pipes(vector<string> cmd1, vector<string> cmd2);
void execute_cmd(vector<string> &args);
vector<string> sub_vec(vector<string> &vec, int start, int end);
void output(string filename);
int find_token(vector<string>&,string);
void handle_append(vector<string> args);

int main(){
    bool should_continue = true;
    string line;
    vector<string> args;
	vector<string> empty; //:D

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
	int pipe_loc = find_token(args,"|");

	if(pipe_loc == -1){
		pid_t child = fork();
		if( child == 0 ){
			handle_append(args);
		}
		waitpid(child,NULL,0);
	}else{
		handle_pipes(
			sub_vec(args,0,pipe_loc-1),
			sub_vec(args,pipe_loc+1,args.size()-1)
		);
	}
}
void handle_pipes(vector<string> cmd1, vector<string> cmd2){
	int fd[2];
	pipe(fd);
	pid_t child = fork();
	pid_t child2;

	if( child == 0 ){
		dup2(fd[WRITE], STDOUT_FILENO);
		close(fd[READ]);
		execute_cmd(cmd1);
	}else{
		child2 = fork();
		if( child2 == 0 ){
			dup2(fd[READ], STDIN_FILENO);
			close(fd[WRITE]);

			int next_pipe = find_token(cmd2,"|");
			if( -1 != next_pipe ){
				handle_pipes(
					sub_vec(cmd2,0,next_pipe-1),
					sub_vec(cmd2,next_pipe+1,cmd2.size()-1)
				);
				_exit(0);
			}else{
				handle_append(cmd2);
			}
		}
		wait(NULL);
	}
	close(fd[READ]);
	close(fd[WRITE]);
	wait(NULL);
}

void execute_cmd(vector<string> &args){
	if( args.size() > 0 ){
		int append_pos = find_token(args,">>");

		vector<char *> c_style;
		for(int i = 0; i < args.size(); i++){
		   c_style.push_back(const_cast<char *>(args[i].c_str()));
		}
		c_style.push_back(NULL);

		execvp(c_style[0],&c_style[0]);
		perror("error: ");
		_exit(-1);
	}
	_exit(0);
}

void handle_append(vector<string> args){
	int append_pos = find_token(args,">>");

	if( append_pos == -1 ){
		execute_cmd(args);
	}else{
		vector<string> cmd1;
		vector<string> cmd2;

		cmd1 = sub_vec(args,0,append_pos-1);
		cmd2 = sub_vec(args,append_pos+1,args.size()-1);

		int fd[2];
		pipe(fd);
		pid_t child = fork();
		pid_t child2;

		if( child == 0 ){
			dup2(fd[WRITE], STDOUT_FILENO);
			close(fd[READ]);
			execute_cmd(cmd1);
		}else{
			child2 = fork();
			if( child2 == 0 ){
				dup2(fd[READ], STDIN_FILENO);
				close(fd[WRITE]);

				output(cmd2[0]);	
			}
			wait(NULL);
		}
		close(fd[READ]);
		close(fd[WRITE]);
		wait(NULL);
	}	
}
void output(string filename){
	int fd;
	int count;
	char c;

	fd=open(filename.c_str(),O_RDWR|O_CREAT,0600);

	while ((count=read(0,&c,1))>0) 
		write(fd,&c,1);
	close(fd);

	_exit(1);
}

int find_token(vector<string> &vec, string token){
	for(int i = 0; i < vec.size(); i++){
		if(vec[i] == token){
			return i;
		}
	}
	return -1;
}

vector<string> sub_vec(vector<string> &vec, int start, int end){
	vector<string> new_vec;

	for(int i = start; i <= end; i++){
		new_vec.push_back(vec[i]);
	}
	return new_vec;
}
