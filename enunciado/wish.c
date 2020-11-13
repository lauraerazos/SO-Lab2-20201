#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <fcntl.h>

char **paths;
int pathLen = 1;
void parseCommand(char *line);
int wordCount(char *line);
void selectCommand(char ** words,int count,int redir);
void changeDir(char **words);
void runCommand(char ** words);
void addPath(char **words);
char **copy_command(int start, int end, char **command);
void redirExecute(char **words, int index);

static char error_message[25] = "An error has occurred\n";

int main(int argc, char ** argv){
	char *bin = "/bin";
	paths = (char**)malloc(3*sizeof(char*));
	paths[pathLen-1] = bin;
	char *line;
	size_t len = 0;
	ssize_t lineSize = 0;
	// char *string,*found;
	// modo de shell interactivo
	if(argc == 1){
		int seguir = 1;
		// Ciclo principal 
		while(seguir == 1){
			printf("wish> ");
			lineSize = getline(&line, &len, stdin);
			parseCommand(line);        	
		}
	}
	else if (argc == 2)// modo batch, recibe un archivo con instrucciones
	{
		FILE *file;
		file = fopen(argv[1], "r");
		if(file == NULL){
			write(STDERR_FILENO,error_message, strlen(error_message)*sizeof(char));
			exit(1);
		}

		lineSize = getline(&line, &len, file);

		while (lineSize >= 0){

			parseCommand(line);        	
			lineSize = getline(&line, &len, file);
		}
		exit(0);
	}
	else
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
	
}

int findRedir(char **words,int len){
	for(int i=0;i<len;i++){
		if(strcmp(words[i], ">\0") == 0){
		return i;
		}
	}
	return 0;
}

void parseCommand(char *line){


	int countWords=wordCount(line);
	char *words[countWords];
	int length= strlen(line);

	line[length-1]='\0';
	for(int i = 0; i < length; i++){
		if(line[i] == '\t' || line[i] == '\n')
			 line[i] = ' ';
	}	
	// Eliminar espacios del principio
	while(*line == ' ')
		line ++;

	char *found;
	int i = 0;
	int aux=0;
	while((found = strsep(&line, " ")) != NULL){
			// words[i++] = found;
			if(strlen(found)>0){
				aux=1;
				words[i++] = found;
			}
					
	}

	// printf("%d\n",!(*words[0]));
	if(aux==1){
		words[i] = NULL;
		int redir=findRedir(words,i);
		
		selectCommand(words,countWords,redir);
	}

	
    //execvp(words[0], words);  // run
}


//Cuenta las palabras en la línea ingresada
int wordCount(char *line){
	int count = 1;
	int length= strlen(line);

	for(int i = 1; i<length; i++){
		if( line[i] != ' ' &&  line[i-1] == ' '){
			count++;
		}
			
	}
	return count;
}

//Selector de comandos
void selectCommand(char ** words,int count,int redir){

	if(strcmp(words[0], "exit") == 0){
		if (count>1){
			write(STDERR_FILENO, error_message, strlen(error_message));
		}
		else exit(0);
	}
	else if(strcmp(words[0], "cd") == 0){

		changeDir(words); //funcionando
	}
	else if(strcmp(words[0], "path") == 0){

		addPath(words);
	}
	else{

		if(redir>0){
			redirExecute(words,redir);
		}

		else runCommand(words);
	}

	// free(items);
	// numItems = 0;
}

void changeDir(char **words){
	if (words[1] != NULL && words[2] == NULL)
	{
		int cdSuccess = chdir(words[1]);
		if (cdSuccess == -1)
		{
			write(STDERR_FILENO, error_message, strlen(error_message));
			return;

		}
	}
	else write(STDERR_FILENO, error_message, strlen(error_message));
	return;
}

void runCommand(char ** words){
		int flag=0;
		if (!(*words[0])) {
			flag=1;
		}
		
		for(int i=0;i<pathLen && flag==0;i++){

			int fullPathLen = strlen(paths[i]) + strlen(words[0])+1;
            char auxpath[fullPathLen];
			strcpy(auxpath,paths[i]);
			strcat(auxpath, "/");
			strcat(auxpath, words[0]);
			flag =1;
			int rc = fork();
			if (rc == 0) {
				
				if(execv(auxpath, words)==-1){
					flag=0;
				} 		
			}else{
			wait(NULL);
			}
		}
		if(flag==0){
			write(STDERR_FILENO,error_message, strlen(error_message));
		}		
}

void addPath(char **words){
	if (paths != NULL) free(paths);
	paths = (char**)malloc(sizeof(char*));
	char *path_name = NULL;
	int index = 0;
	char **p = words;
	while(*(++p)){
		path_name = (char*)malloc(strlen(*p)*sizeof(char));
		stpcpy(path_name, *p);
		paths[index] = path_name;
		index++;
		paths = (char**)realloc(paths, (index+1)*sizeof(char*));
	}
	paths[index] = NULL;
	pathLen=index;
}


void redirExecute(char **words, int index){
	char **args = copy_command(0, index, words);
	if(words[index+1]==NULL || words[index+2] != NULL){
		write(STDERR_FILENO,error_message, strlen(error_message));

	}else
	{
		int fd = open(words[index+1], O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
		int std_out = dup(STDOUT_FILENO);
		int std_err = dup(STDERR_FILENO);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		runCommand(args);
		close(fd);
		dup2(std_out, STDOUT_FILENO);
		dup2(std_err, STDERR_FILENO);	
	}
		
}

char **copy_command(int start, int end, char **command){
	char **new_command = (char**)malloc((end-start+1)*sizeof(char*));
	// char **new_command[end];
	for(int i = 0; i < end; i++)
		new_command[i] = command[i];
	new_command[end] = NULL;
	return new_command;
}