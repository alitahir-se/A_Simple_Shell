#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/types.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<iostream>
#include <fcntl.h>    
#include<sys/stat.h>


using namespace std;

#define LINE_LEN 80
#define MAX_ARGS 64
#define MAX_ARG_LEN 16
#define MAX_PATHS 64
#define MAX_PATH_LEN 96
#define WHITESPACE " ,\t\n"
#define PARSE "|\n"
#define FILEPARSE ">"
#define Exit "exit"

struct command_t{
char* name;
int argc;
char*argv[MAX_ARGS];

};


void printPrompt();
void readCommand(char* commandLine);
int parseCommand(char *commandLine,command_t* command);
void executePipe(command_t* command,int i,int noOfCommands,int** pipes);
void executeFile(command_t* command,int i,char** parsedfile,int** pipes,int noOfCommands);
int parsePipe(char *commandLine,char** parsedpipe);
void parseSpace(char* file_Name_With_LeadingSpace, char** parsedfile);
int parsePath(char *dirs[]);
char* lookupPath(char**argv,char**dirs);
bool checkfile (char* commandLine);
void parseFile(char *commandLine,char** parsedfile);


int main(int argc,char* argv[])
{
	char* dirs[MAX_ARGS];
	int k=parsePath(dirs);
	char* commandLine=new char[LINE_LEN];
	char** parsedpipe = new char*[LINE_LEN];
	int stdoutCopy = dup(1);
	int stdinCopy = dup(0);
	for(int i=0;i<LINE_LEN;i++)
	{
	parsedpipe[i]=new char[LINE_LEN];
	}
	

	

	while(true)
	{	bool flag=false;
		command_t command;
		command_t fileCommand;
		char** parsedfile = new char* [2];
	
		printPrompt();
		commandLine[0] == '\n';
		readCommand(commandLine);
		
		if(commandLine[0] == '\n')continue;
			
				
		
		for(int i=0;i<commandLine[i]!='\0';i++)
		{
  			if(commandLine[i]=='|')                   
				{
					flag=true;
					break;
				}
		}
	
		if(flag)
		{	
			int p1[2];
			   
			int t1=pipe(p1);
			
			if(t1 == -1)
			{
				 cout << "Cannot Open Anonymous Pipe" <<endl; exit(EXIT_FAILURE);	
			}

			
			int k=0;
			
			int h=parsePipe(commandLine,parsedpipe);
				int** pipes = new int* [h-1];
		for (int j=0;j<h-1;j++)
		{
			pipes[j]=new int [2];
			pipe(pipes[j]);
		}
			for(int i=0;i<h;i++)
			{
					
				bool fileFlag = checkfile(parsedpipe[i]);
			
				if (fileFlag)
				{
					
					parseFile (parsedpipe[i], parsedfile);
					char* file_Name_With_LeadingSpace=parsedfile[1];
				        parseSpace(file_Name_With_LeadingSpace, parsedfile);	
					parseCommand(parsedfile[0], &fileCommand);	//func to break user input
					
					fileCommand.name=lookupPath(fileCommand.argv,dirs);
				}
				else{
					k=parseCommand(parsedpipe[i],&command);
					command.name=lookupPath(command.argv,dirs);
					
				}

		
					if(command.name == NULL)
						continue;
					if(!fileFlag){
						cout << command.name << endl;
						executePipe(&command,i,h,pipes);
		
					}else{

					executeFile(&fileCommand,i,parsedfile,pipes,h);
						dup2(stdoutCopy,1);
						dup2(stdinCopy,0);
					}
			}


				
		}
		else
		{	
			bool fileFlag=checkfile(commandLine);
				


			if(fileFlag){
				
 	
				parseFile (commandLine, parsedfile);
				char* file_Name_With_LeadingSpace=parsedfile[1];
				parseSpace(file_Name_With_LeadingSpace, parsedfile);
				parseCommand(parsedfile[0], &command);
								
				command.name=lookupPath(command.argv,dirs);
			

			}else{

			
				int j=parseCommand(commandLine,&command);
				string s = (string) command.name;
				if(s == "exit")return 0;
				command.name=lookupPath(command.argv,dirs);

			}
			
			pid_t pid=fork();
			if(pid == -1){
				cout<<"Fork Error\n";
				exit(EXIT_FAILURE);
			}
			
			if(pid == 0)

			{	if(fileFlag){

					int fd=open(parsedfile[1],O_WRONLY);
					if(fd == -1){
						fd=open(parsedfile[1],O_CREAT|O_WRONLY);
						chmod(parsedfile[1],0644);
						if(fd < 0){cout << "cannot create file\n";exit(EXIT_FAILURE); }
					}

					dup2(fd,1);
				
				}
				

				execv(command.name,command.argv);

			}else if(pid > 0)
				wait(NULL);
				
				if(fileFlag)
					dup2(stdoutCopy,1);

		}
		
		
	}


			
		

return 0;
}


void executePipe(command_t* command,int i,int noOfCommands,int** pipes)
{
  int status=0;
  pid_t pid = fork();

	if(pid == -1)
	{
		cout<<"Fork Failure"<<endl;
		exit(EXIT_FAILURE);
	}
	
  	if(pid == 0)
	{
		
					
						if (i==0)										//for 1st command
						{
							dup2(pipes[i][1],1);
						}
						else if (i==noOfCommands-1)									//for 2nd command
						{
							dup2(pipes[i-1][0],0);
						}
						else 									//for 3rd command
						{
							dup2(pipes[i-1][0],0);
								dup2(pipes[i][1],1);
						}

		execv(command->name,command->argv);
		cout<< "failed to execute command" << endl;
		exit(EXIT_FAILURE);
	}
	if(pid > 0)
	{	
		wait(NULL);
		if (i!=noOfCommands-1)
					close(pipes[i][1]);
		
	}
}


void printPrompt(){

char cwd[MAX_PATH_LEN];
getcwd(cwd,sizeof(cwd));
string s(cwd);
size_t found=s.find_last_of("/\\");
string sub=s.substr(found) ;

string name=getenv("USER");

string promptString=name+"@ubuntu:~"+sub + "$  ";
cout << promptString;

}

bool checkfile(char* commandLine)
{

	bool flag=false;
	for(int i=0;i<commandLine[i]!='\0';i++)
	{
		if(commandLine[i]=='>')
		{
			flag=true;
			break;
		}
	}
   return flag;
}

void executeFile(command_t* command,int i,char** parsedfile,int** pipes,int noOfCommands){


	int fd=open(parsedfile[1],O_WRONLY);
	if(fd == -1){
		fd=open(parsedfile[1],O_CREAT|O_WRONLY);
		chmod(parsedfile[1],0644);
		if(fd < 0){cout << "cannot create file\n";exit(EXIT_FAILURE); }
	}
	pid_t pid=fork();
	if(pid == 0){

		 if (i==0)										
						{
							dup2(fd,1);
						}
						else									
						{
							dup2(pipes[i-1][0],0);
							dup2(fd,1);
						}

		execv(command->name,command->argv);			
		cout<<"File Command failed\n";
	}

	if(pid > 0){
	wait(NULL);
	if (i!=noOfCommands-1)
					close(pipes[i][1]);
			}
	


}

void readCommand(char* commandLine){

fgets(commandLine,LINE_LEN,stdin);

}

int parsePipe(char *commandLine,char** parsedpipe){
	int  position = 0;
  	char *token;
	
  	token = strtok(commandLine, PARSE);
 	
 	while (token != NULL) 
	{
   		parsedpipe[position]=token;
   
		position++;

   		 token = strtok(NULL, PARSE);
	}

	

	parsedpipe[position]=NULL;
	
	

	return position;
	




}

void parseFile(char *commandLine,char** parsedfile){
	int  position = 0;
  	char *token;
	
  	token = strtok(commandLine, FILEPARSE);
 	
 	while (token != NULL) 
	{
   		parsedfile[position]=token;
   
		position++;

   		 token = strtok(NULL, FILEPARSE);
	}

	

	parsedfile[position]=NULL;
	


}

void parseSpace(char* file_Name_With_LeadingSpace, char** parsedfile){

  	char *token;
	
  	token = strtok(file_Name_With_LeadingSpace, WHITESPACE);
 	parsedfile[1]=token;
   
 	cout << parsedfile[1]<<endl;
	
}


int parseCommand(char *commandLine,command_t* command){

  int  position = 0;
  char *token;

  token = strtok(commandLine, WHITESPACE);
 command->name=token;
  while (token != NULL) 
{
   command->argv[position]=token;
   
	position++;

    token = strtok(NULL, WHITESPACE);
}

command->argc=position;

command->argv[position]=NULL;


return 0;  
}

int parsePath(char *dirs[]){


char* pathEnvVar;
char* thePath;

for(int i=0;i<MAX_ARGS;i++){


dirs[i]=NULL;
}


pathEnvVar =(char*)getenv("PATH");
thePath=(char*)malloc(strlen(pathEnvVar)+1);
strcpy(thePath,pathEnvVar);

int  position = 0;
  char *token;

  token = strtok(thePath, ":");
 
  while (token != NULL) 
{
   dirs[position]=token;
    
	position++;

    token = strtok(NULL, ":");
}


return 0;

}

char* lookupPath(char**argv,char**dirs){

char* result = new char[MAX_PATH_LEN];
char pName[MAX_PATH_LEN];
char* test[MAX_ARGS];
int flag=-1;
string result1;
for(int i=0;i<MAX_ARGS;i++){


test[i]=dirs[i];

}

if(*argv[0] == '/'){

return argv[0];
}


for(int i=0;test[i]!=NULL;i++){


string s(test[i]);
result1=s+"/"+argv[0];

strcpy(result,result1.c_str());
flag=access(result,F_OK);
if(flag==0)
	return result;

}

cout << "Command Not Found" <<endl;
return NULL;
}



