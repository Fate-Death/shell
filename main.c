#include <stdio.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>

#define LSH_RL_BUFSIZE 1024
#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
#define PATH_MAX 1024

//searching on google
int lsh_search(char **args){
	char google[256]="https://www.google.com/search?q=";
	char command[300];
	strcat(google, args[1]);
	int i=2;
	while(args[i]!=NULL){
		strcat(google, "+");
		strcat(google, args[i]);
		i++;
	}
	snprintf(command,sizeof(command),"xdg-open \"%s\"",google);
	system(command);
	return 1;
}

//Functions declarations for builtin shell commands
int lsh_cd(char **args);
int lsh_help(char **args);
int lsh_exit(char **args);

//List of builtin commands, followed by their corresponding functions.

char *builtin_str[]={
	"cd",
	"help",
	"exit",
	"google"
};
int (*builtin_func[])(char **)={
	&lsh_cd,
	&lsh_help,
	&lsh_exit,
	&lsh_search,
};
int lsh_num_builtins(){
	return sizeof(builtin_str)/sizeof(char *);
}
//Builtin functions implementations.
int lsh_cd(char **args){
	if(args[1]==NULL){
		fprintf(stderr,"lsh: expected arguments to \"cd\"\n");
	}
	else{
		if(chdir(args[1])!=0){
			perror("lsh");
		}
	}
	return 1;
}
int lsh_help(char **args){
	int i;
	printf("LSH\n");
	printf("Type program names and arguments, and hit enter\n");
	printf("The following are built in:\n");
	
	for(i=0;i<lsh_num_builtins();i++){
		printf("%s\n",builtin_str[i]);
	}
	printf("Use man command for more information on other programs\n");
	return 1;
}
int lsh_exit(char **args){
	return 0;
}

int lsh_launch(char **args){
	pid_t pid,wpid;
	int status;
	pid=fork();
	if(pid==0){
		//Child process
		if(execvp(args[0],args)==-1){
			perror("lsh");
		}
		exit(EXIT_FAILURE);
	}
	else if(pid<0){
		//Error forking
		perror("lsh");
	}
	else{
		//Parent process
		do{
			wpid=waitpid(pid,&status,WUNTRACED);
		}while(!WIFEXITED(status)&&!WIFSIGNALED(status));
	}
	return 1;
}
int lsh_execute(char **args){
	int i;
	if (args[0]==NULL){
		//An empty command was entered.
		return 1;
	}
	for(i=0;i<lsh_num_builtins();i++){
		if(strcmp(args[0],builtin_str[i])==0){
			return (*builtin_func[i])(args);
		}
	}
	return lsh_launch(args);
}

char **lsh_split_line(char *line){
	int bufsize=LSH_TOK_BUFSIZE, position=0;
	char **tokens=malloc(bufsize*sizeof(char*));
	char *token;

	if(!tokens){
		fprintf(stderr,"lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	token=strtok(line,LSH_TOK_DELIM);
	while(token!=NULL){
		tokens[position]=token;
		position++;

		if(position>=bufsize){
			bufsize+=LSH_TOK_BUFSIZE;
			tokens=realloc(tokens,bufsize*sizeof(char*));
			if(!tokens){
				fprintf(stderr,"lsh: allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
		token =strtok(NULL,LSH_TOK_DELIM);
	}
	tokens[position]=NULL;
	return tokens;
}

char* lsh_read_line(void){
	char *line=NULL;
	ssize_t bufsize=0; //have geline allocate a buffer for us

	if(getline(&line,&bufsize,stdin)==-1){
		if(feof(stdin)){
			exit(EXIT_SUCCESS);//we recieved a EOF
			}
		else{
			perror("Readline");
			exit(EXIT_FAILURE);
		}
	}
	return line;
}

void pwd(void){
	char cwd[PATH_MAX];
	if(getcwd(cwd, sizeof(cwd))!=NULL){
			printf("%s",cwd);
	}
}

void lsh_loop(void){
	char *line;
	char **args;
	int status;

	do{
		pwd();
		printf("> ");
		line = lsh_read_line();
		args = lsh_split_line(line);
		status = lsh_execute(args);

		free(line);
		free(args);
	}while(status);
}

int main(int argc, char **argv) {


	lsh_loop();

	return 0;

}
//Function that reads characters

/*char *lsh_read_line(void){
	int bufsize = LSH_RL_BUFSIZE;
	int position = 0;
	char *buffer = malloc(sizeof(char)*bufsize);
	int c;

	if(!buffer){
		fprintf(stderr, "lsh: allocation error\n");
		exit(EXIT_FAILURE);
	}
	while(1){
		//Read a character
		c = getchar();

		//If we hit EOF, replace it with a null character and return.
		if(c==EOF||c=='\n'){
			buffer[position]='\0';
			return buffer;
		}
		else{
			buffer[position]=c;
		}
		position++;

		//If we have exceeded the buffer, reallocate.
		if(position>=bufsize){
			bufsize+=LSH_RL_BUFSIZE;
			buffer=realloc(buffer,bufsize);
			if(!buffer){
				fprintf(stderr,"lsh:allocation error\n");
				exit(EXIT_FAILURE);
			}
		}
	}
}*/


