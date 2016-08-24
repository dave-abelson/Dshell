#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
#define MAX_INPUT 1024

void printHelpScreen();
int buildArgv(char *cmdLine, char **argv); /*CHANGE TO RETURN TRUE OR FALSE IF BG OR FB */
int isBuiltIn(char **argv, char store[]);
int isBuiltIn2(char *argv, char store[]);
int checkForThePipes(char **argv, int numbArgs);
extern char **environ;

//history variables
  int hist_i = 0;
  char ** hist = NULL;
  int hist_length = 0;
  int hist_max = 10;

  void add_history(const char *entry){
      char *line = strdup(entry);
      if(hist_length == hist_max){
        hist_max *= 2;
        hist = (char**) realloc(hist, hist_max * sizeof(char*));
      }
      line[strlen(line)] = '\0';
      hist[hist_length] = line;
      hist_i++;
      hist_length++;

  }

  void start_history(){
    hist = (char**)malloc(hist_max * sizeof(char*));
    const char *llun = "";
    add_history(llun);
  }

int main(int argc, char **argv, char **envp){
  int finished = 0;
  char *prompt = "Dshell>";
  char cmd1[MAX_INPUT];
  char *cmd = malloc(MAX_INPUT);
  char *argvArr[MAX_INPUT];
  char whatUserTyped[MAX_INPUT];
  char *shift = (char*)malloc(MAX_INPUT *sizeof(char));
  char *argvNoSpec[MAX_INPUT];
  char tempStore[MAX_INPUT]; 
  pid_t pid;
  int child_status;
  int numArgs;
  int charCounter;
  // int firstTime = 1;
  int returnCode = 0;
  int hasGreaterThan = 0, posWithGreaterThan;
  int hasLessThan = 0, posWithLessThan;
  int hasSign;
  int hasPipes, pipeError = 0, pipeOrNaw = 0;
  int fd;
  char ch;

    if(hist == NULL){
      start_history();
    }
  while(!finished){
    char *cursor;
    char last_char;
    int count;

    int rv;
    char currentPath[MAX_INPUT];
    getcwd(currentPath, sizeof(currentPath));
    printf("[");
    fflush(stdout);
    rv = write(1, currentPath, strlen(currentPath));
    printf("] ");
    fflush(stdout);
    rv = write(1, prompt, strlen(prompt));
    for(int j = 0; j < MAX_INPUT; j++){
      whatUserTyped[j] = '\0';
    }
    for(int j = 0; j < MAX_INPUT; j++){
      shift[j] = '\0';
    }
    if(!rv){
      finished = 1;
      break;
    }
    // if(!firstTime)
    //   memset((char*)whatUserTyped, '\0', charCounter);
    charCounter = 0;
    int input_length = 0;
    int line_position = 0;
    for(rv = 1, count = 0, cursor = cmd1, last_char = 1;
    rv && (++count < (MAX_INPUT-1)) && (last_char != '\n');
    cursor++) { 
      rv = read(0, cursor, 1);
      last_char = *cursor;
      if(*cursor >= 32 && *cursor < 127){
          if(line_position == input_length){
            if(input_length > MAX_INPUT-1){ break;}
              whatUserTyped[line_position] = last_char;
              input_length++;
              line_position++;
              if(last_char == 3){
                write(1, "^c", 2);
              }else{
                write(1, &last_char, 1);
              }
            }
          else{
            for(int j = 0; j < MAX_INPUT; j++){
              shift[j] = '\0';
            }
            int i;
            for(i = 0; i < MAX_INPUT; i++){
              if(whatUserTyped[line_position + i] == '\0'){
                break;
              }
              shift[i] = whatUserTyped[line_position + i];
            }
              write(1, &last_char, 1);
              if(input_length > MAX_INPUT-1){ break;}
              whatUserTyped[line_position] = last_char;
              input_length++;
              //line_position++;
              charCounter++;
              int replace = 0;
              for(i = 0; i < MAX_INPUT; i++){
                replace += 1;
                write(1, &shift[i], 1);
                whatUserTyped[line_position + i + 1] = shift[i];
                if(whatUserTyped[line_position + i] == '\0'){
                   ch = 8;
                   write(1, &ch, 1);
                   ch = 32;
                   write(1, &ch, 1);
                   // ch = 8;
                   // write(1, &ch, 1);
                   break;
                }

              }
              line_position++;
              for(i = 0; i < replace - 2; i++){
                  ch = 8;
                  write(1, &ch, 1);
              }


          }
        }
      //enter
      else if(*cursor == 10){
        ch = *cursor;
        if(strlen(whatUserTyped) != 0){
          add_history(whatUserTyped);
        }
        write(1, &ch, 1);
        for(int j = 0; j < MAX_INPUT; j++){
          shift[j] = '\0';
        }
        break;
      }
      //delete
      else if(*cursor == 127 && input_length > 0){
        //cursor--;
        if(line_position > 0){
          ch = 8;
          write(1, &ch, 1);
          // ch = 32;
          // write(1, &ch, 1);
          // ch = 8;
          // write(1, &ch, 1);
          // whatUserTyped[line_position] = '\0';
          //printf("INPUTLENGTH: %d\n", input_length);
          if(line_position == input_length){
            ch = 32;
            write(1, &ch, 1);
            ch = 8;
            write(1, &ch, 1);
            whatUserTyped[line_position - 1] = '\0';
          }else{
            for(int j = 0; j < MAX_INPUT; j++){
              shift[j] = '\0';
            }
            char *shift = (char*)malloc(MAX_INPUT * sizeof(char));
            int i;
            for(i = 0; i < MAX_INPUT; i++){
              shift[i] = whatUserTyped[i];
              if(whatUserTyped[line_position + i] == '\0'){
                break;
              }

            }

          int replacement = 0;
            for(int i = 0; i < MAX_INPUT; i++){
                replacement += 1;
                write(1, &shift[i], 1);
                if(whatUserTyped[line_position + i] == '\0'){
                   ch = 32;
                   write(1, &ch, 1);
                   break;
                }
              }
            for(i = line_position; i < MAX_INPUT; i++){
                if(whatUserTyped[i] == '\0'){
                   break;
                }
                whatUserTyped[i-1] = whatUserTyped[i];
              }
            for(i = input_length - 1; i < MAX_INPUT; i++){
              whatUserTyped[i] = '\0';
            }
            // ch = 8;
            // write(1, &ch, 1);
            // ch = 32;
            // write(1, &ch ,1);
            for(i = 0; i < replacement - 1; i++){
                ch = 8;
                write(1, &ch, 1);
            }
            ch = 8;
            write(1, &ch, 1);
          }
          input_length--;
          line_position--;
          charCounter--;
          cursor--;
        }
      }
      //escape characters
      else if(*cursor == 27){
          char char1;
          char char2;
          read(0, &char1, 1);
          read(0, &char2, 1);
          int i =0;
          //up arrow
          if(char1 == 91 && char2 == 65){
            if(input_length > 0){
              for(i = 0; i < input_length; i++){
                ch = 8;
                write(1, &ch, 1);
              }
              for(i = 0; i < input_length; i++){
                ch = 32;
                whatUserTyped[i] = '\0';
                write(1, &ch, 1);
              }
              for(i = 0; i < input_length; i++){
                ch = 8;
                write(1, &ch, 1);
              }
            }
              if(hist_i <= 0){
                  hist_i = hist_length -1;
              }
              if(hist[hist_i] == NULL){
                const char *llun = "";
                strcpy(whatUserTyped, llun);
              }
              else{
                  strcpy(whatUserTyped, hist[hist_i]);
              }
              input_length = strlen(whatUserTyped);
              hist_i = (hist_i + 1) % hist_length;
              write(1, whatUserTyped, input_length);
              //whatUserTyped = "cd /home"
              //printf("WHAT: %s\n", whatUserTyped);
              //memset((char*)whatUserTyped, 0, charCounter);
              line_position = input_length;
          }
          //down
          else if(char1 == 91 && char2 == 66){
              if(input_length > 0){
                int i = 0;
                for(i = 0; i < input_length; i++){
                  ch = 8;
                  write(1, &ch, 1);
                }
                for(i = 0; i < input_length; i++){
                  ch = 32;
                  whatUserTyped[i] = '\0';
                  write(1, &ch, 1);
                }
                for(i = 0; i < input_length; i++){
                  ch = 8;
                  write(1, &ch, 1);
                }
              }
              if(hist_i <= 0){
                hist_i = 0;
              }
              else{
                hist_i++;
                if(hist_i >= hist_length){
                  hist_i = 0;
                }
              }

              if(hist[hist_i] == NULL){
                const char *llun = "";
                strcpy(whatUserTyped, llun);
              }
              else{
                  strcpy(whatUserTyped, hist[hist_i]);
              }
              input_length = strlen(whatUserTyped);
              hist_i = (hist_i + 1) % hist_length;
              write(1, whatUserTyped, input_length);

              // if(hist_i > 0){
              //   strcpy(whatUserTyped, hist[hist_i]);
              //   input_length = strlen(whatUserTyped);
              //   hist_i = (hist_i - 1) % hist_length;

              //   write(1, whatUserTyped, input_length);
              //   line_position = input_length;
              // }
              // else{
              //   strcpy(whatUserTyped, "");
              //   input_length = strlen(whatUserTyped);
              //   write(1, whatUserTyped, input_length);
              //   line_position = input_length;
              // }
          }
          //left
          else if(char1 == 91 && char2 == 68){
              //printf("LEFT ARROW\n");
              if(line_position > 0){
                ch = 8;
                write(1, &ch, 1);
                //charCounter--;
                line_position--;
                cursor--;
              }
          }
          //right
          else if(char1 == 91 && char2 == 67){
              //printf("RIGHT ARROW\n");
              if(line_position < input_length){
                char rig = whatUserTyped[line_position];
                write(1, &rig, 1);
                line_position++;
                cursor++;
                //charCounter++;
              }
          }
          else{
            printf("NOTHING\n");
          }
    }
    } 
    *cursor = '\0';
    whatUserTyped[input_length] = 10;
    input_length++;
    whatUserTyped[input_length] = 0;

    if(input_length != 1){
    numArgs = buildArgv(whatUserTyped, argvArr);
    memset(argvNoSpec, '\0', MAX_INPUT);
    for(int i = 0; i < numArgs; i++){
      if(*argvArr[i] == '>'){
        hasGreaterThan = 1;
        posWithGreaterThan = i;
      }else if(*argvArr[i] == '<'){
        hasLessThan = 1;
        posWithLessThan = i;
      }
    }
    if(*argvArr[0] == '|'){
      printf("%c : command not found.\n", *argvArr[0]);
      goto endOfWhileLoop;
    }
    int i;
    char *pipeArgv[MAX_INPUT];
    for(i = 0; i < numArgs; i++){
       pipeArgv[i] = malloc(strlen(argvArr[i]) * sizeof(char*));
       strcpy(pipeArgv[i], argvArr[i]);
    }
    int numPipes = checkForThePipes(pipeArgv, numArgs);
    checkForThePipes(argvArr, numArgs);
    char pipeStore[MAX_INPUT];
    int supposedNumCommands = numPipes + 1;
    int actualNumCommands = 0;
    int compareCommands = supposedNumCommands;
    int pipeFd[numPipes*2];
    if(numPipes == -1)
      pipeError = 1;
    else if(numPipes == 0){

    }
    else{
      hasPipes = 1;
      supposedNumCommands = numPipes + 1;
      actualNumCommands = 0;
      compareCommands = supposedNumCommands;
      for(i = 0; i < numPipes; i++){
        if(pipe(pipeFd + i*2) < 0){
          perror("Piping Error.\n");
          pipeError = 1;
        }
      }
      if(hasPipes && pipeError){
      }
      // printf("argv[0] %s\n argv[1] %s\n argv[2] %s\n", pipeArgv[6], pipeArgv[7], pipeArgv[8]);
      int j = 0; //i value of where i am traversing through the statement
      while(supposedNumCommands){
        if(isBuiltIn2(pipeArgv[j], pipeStore)){
          actualNumCommands++;
          printf("%s\n", pipeArgv[j]);
        }
        for(i = j; i < numArgs; i++){
          if(pipeArgv[i] == NULL){
            break;
          }
        }
        j = i+1;
        // printf("J: %d\n", j);
        // printf("different array when checking for builtin! %s\n", pipeArgv[j]);
        supposedNumCommands--;
      }
      if(actualNumCommands == compareCommands){
        // printf("sweet! we can pipe! \n");
        pipeOrNaw = 1;
      }
      for(i = 0; i < numArgs; i++){
        free(pipeArgv[i]);
      }
    }

    int firstIndexSign = 0;
    if(posWithLessThan > posWithGreaterThan && posWithGreaterThan != 0)
      firstIndexSign = posWithGreaterThan;
    else if(posWithLessThan > posWithGreaterThan && posWithGreaterThan == 0)
      firstIndexSign = posWithLessThan;
    else if(posWithGreaterThan > posWithLessThan && posWithLessThan != 0)
      firstIndexSign = posWithLessThan;
    else if(posWithGreaterThan > posWithLessThan && posWithLessThan == 0)
      firstIndexSign = posWithGreaterThan;
    // printf("%d\n", firstIndexSign);
    for(int i = 0; i < firstIndexSign; i++){
      argvNoSpec[i] = malloc(100);
    }


    if(hasLessThan || hasGreaterThan)
      hasSign = 1;
    else
      hasSign = 0;
    // printf("has sign? %d", hasSign);
    if(hasSign){
    if(posWithGreaterThan > posWithLessThan){
      // printf("did i go here1\n");
      // char **ptr1 = argvArr;
      // char **ptr2 = argvNoSpec;
      if(posWithLessThan == 0){
        for(int i = 0; i < posWithGreaterThan; i++){
          strcpy(argvNoSpec[i], argvArr[i]);
          // strcpy(*ptr2, *ptr1);
          // printf("argv[%d]; %s\n", i, argvNoSpec[i]);
          // ptr2++;
          // ptr1++;
          // printf("%s\n", argvArr[i]);
        }
      }
      else{ //poswithlessthan != 0
        // printf("poswithlessthan != 0\n");
        for(int i = 0; i < posWithLessThan; i++){
          strcpy(argvNoSpec[i], argvArr[i]);
          // strcpy(*ptr2, *ptr1);
          // ptr2++;
          // ptr1++;
        // printf("%s\n", argvArr[i]);
        }
      }
      // printf("new0 %s new1 %s\n", argvNoSpec[0], argvNoSpec[1]);
    }
    else{ // posWithLessThan > posWithGreaterThan
      // printf("did i go here2\n");
      if(posWithGreaterThan == 0){
        for(int i = 0; i < posWithLessThan; i++){
          strcpy(argvNoSpec[i], argvArr[i]);
          // printf("%s\n", argvArr[i]);
        }
      }else{
        for(int i = 0; i < posWithGreaterThan; i++){
          strcpy(argvNoSpec[i], argvArr[i]);
        }
      }
    }
      }
    // printf("wat i entered? %s %s\n", argvNoSpec[0], argvNoSpec[1]);
    // if(argvNoSpec[0] != NULL){
    //   printf("so i made it here! %s\n", argvNoSpec[0]);
    // }else{
    //   printf("nothing in argv\n");
    // }
    char temp[MAX_INPUT];
    if(pipeOrNaw){
      pid_t pid;
      int k = 0;
      int p = 0;
      char* subArray[MAX_INPUT];
      while(actualNumCommands){
        int f = 0;
        p = 0;
        if((pid = fork()) == 0){
          if(actualNumCommands != 1 && actualNumCommands > 0){ //not the last command, dup.
            if(dup2(pipeFd[k + 1], 1) < 0){
              perror("dup error not last comand\n");
            }
          }
          if(actualNumCommands != compareCommands){ //not the first comand, dup.
            if(dup2(pipeFd[k - 2], 0) < 0){
              perror("dup error not first command\n");
            }
          }
          isBuiltIn2(argvArr[f], pipeStore);
          memset(subArray, '\0', MAX_INPUT);
          while(argvArr[p] != NULL){
            subArray[p] = malloc(strlen(argvArr[p]));
            strcpy(subArray[p], argvArr[p]);
            p++;
          }
          if(execve(pipeStore, subArray, envp) < 0){
            perror("Execute error\n");
          }
          for(int i = 0; i < p; i++){
            free(subArray[i]);
          }
          exit(0);
        }
        while(argvArr[f] != NULL)
          f++;
        f++;
        k+=2;
        actualNumCommands--;
      }
      for(int i = 0; i < 2 * numPipes; i++){
        close(pipeFd[i]);
      }
      for(int i = 0; i < numPipes; i++)
        wait(&child_status);
    }
    else if(*argvArr[0] == '>' || *argvArr[0] == '<')
      printf("%c : command not found!\n", *argvArr[0]);
    // else if(*argvArr[numArgs - 1] == '>' || *argvArr[numArgs - 1] == '<' || *argvArr[numArgs -1] == '|')
    //   printf("%c : command not found!\n", *argvArr[numArgs]);
    else if(!strcmp(argvArr[0], "exit"))
      exit(0);
    else if(!strcmp(argvArr[0], "help")){
      if(hasGreaterThan){
        fd = open(argvArr[posWithGreaterThan + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
        dup2(1, 100);
        dup2(fd, 1);
        close(fd);
        printHelpScreen();
        dup2(100, 1);
        close(100);
      }else{
        printHelpScreen();
        returnCode = 0;
      }
    }
    else if(!strcmp(argvArr[0], "pwd")){
      if(getcwd(temp, sizeof(temp)) != NULL){
        if(hasGreaterThan){
          fd = open(argvArr[posWithGreaterThan + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
          dup2(1, 100);
          dup2(fd, 1);
          close(fd);
          printf("%s\n", temp);
          dup2(100, 1);
          close(100);
        }else{
          printf("%s\n", temp);
          returnCode = 0;
        }
      }else{
        returnCode = 1;
      }
    }else if(!strcmp(argvArr[0], "set")){
      if(argvArr[1] != NULL){
        if(!strcmp(argvArr[2], "=")){
          char *namevar = argvArr[1];
          char *value = argvArr[3];
          setenv(namevar, value, 1);
          returnCode = 0;
        }else{
          printf("Invalid argument!\n");
          returnCode = 1;
        }
      }else{
        printf("Invalid arguments!\n");
        returnCode = 1;
      }
    }
    else if((!strcmp(argvArr[0], "cd"))){
        //printf("HERE\n");
        char* oldpwdpath = getcwd(temp, sizeof(temp));
        
        if(argvArr[1] == NULL){
            char *oldpath = getenv("OLDPWD");
            //printf("OLD: %s\n", oldpath);
            struct stat s;
            int err = stat(oldpath, &s);
            if(err == -1){
                if(errno == ENOENT){
                  printf("Path does not exist\n");
                }
                else{
                  perror("stat");
                  //exit(1);
                }
            }
            else{
                if(S_ISDIR(s.st_mode)){
                   chdir(oldpath); 
                }
                else{
                    printf("Not valid Directory");
                }
            }
          }
        else if(!strcmp(argvArr[1], "-")){
           char *home = getenv("HOME");
           if(home != NULL){
              chdir(home);
              returnCode = 0;
           }else{
            returnCode = 1;
           }
        }
        else if(!strcmp(argvArr[1], "..")){
          chdir("..");
        }
        else if(!strcmp(argvArr[1], ".")){
        }
        else{
            char firstchar = argvArr[1][0];
            struct stat s;
            char *absolutepath;
            //char *newpath;
            char buildpath[MAX_INPUT];
            int i = 0;
            if(firstchar != '/'){
            
              char *temporalpath = getcwd(temp, sizeof(temp));
              strcpy(buildpath, temporalpath);
              strcat(buildpath, "/");
              strcat(buildpath, argvArr[1]);
              //printf("BUILT PATH: %s\n", buildpath);
              
              while(buildpath[i] != 0){
                  i++;
              }
              //printf("BUILT PATH: %s\n", buildpath);
              char *newpath = (char*)malloc(i * sizeof(char));
              //printf("NUM %d\n", i);
              //printf("NEW PATH: %s\n", buildpath);
              strncpy(newpath, buildpath, i);
              //printf("NEW PATH: %s\n", newpath);
              int err = stat(newpath, &s);
              if(err == -1){
                if(errno == ENOENT){
                  printf("Path does not exist\n");
                }
                else{
                  perror("stat");
                  //exit(1);
                }
              }
            
              else{
                if(S_ISDIR(s.st_mode)){
                   chdir(newpath); 
                }
                else{
                    printf("Not valid Directory");
                }
              }
              free(newpath);
            }
          
          else{
              absolutepath = argvArr[1];
              while(absolutepath[i] != 0){
                  i++;
              }
              //printf("LENGTH: %d\n", i);
              char *otherpath = (char*)malloc(i * sizeof(char));
              strncpy(otherpath, absolutepath, i);
              // printf("BUILD PATH: %s\n", absolutepath);
              // printf("BUILD PATH: %s\n", otherpath);
              // for(int j = 0; j < strlen(otherpath); i++){
              //   printf("%c", otherpath[i]);
              // }
              int err = stat(otherpath, &s);
              if(err == -1){
                if(errno == ENOENT){
                  printf("Path does not exist\n");
                }
                else{
                  perror("stat");
                  //exit(1);
                }
            }
            else{
                if(S_ISDIR(s.st_mode)){
                   chdir(otherpath); 
                }
                else{
                    printf("Not valid Directory");
                }
            }
            free(otherpath);
            }
           }
        setenv("OLDPWD", oldpwdpath, 1);
    }
    else if(!strcmp(argvArr[0], "echo")){
      if(argvArr[1] != NULL){ //dsfaasdfdsf
        if(argvArr[1][0] == '$'){
          if(hasGreaterThan){
            fd = open(argvArr[posWithGreaterThan + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
            dup2(1, 100);
            dup2(fd, 1);
            close(fd);
          }
          if((strlen(argvArr[1]) == 2) && argvArr[1][1] == '?'){
            printf("%d\n", returnCode);
            returnCode = 0;
          }else{
            char temporary[strlen(argvArr[1])-1];
            strcpy(temporary, argvArr[1] + 1);
            char *thePath = getenv((char*)temporary);
            if(thePath != NULL){
              printf("%s ", thePath);
              if(argvArr[2] != NULL){
                for(int i = 2; i < numArgs; i++){
                  printf("%s ", argvArr[i]);
                }
              }
              printf("\n");
              returnCode = 0;
            }
            else{
              if(argvArr[2] != NULL){
                for(int i = 2; i < numArgs; i++){
                  printf("%s ", argvArr[i]);
                }
                printf("\n");
              }
              returnCode = 1;
            }
          }
          dup2(100, 1);
          close(100);
        }else{
          if((pid = fork()) == 0){
            if(hasGreaterThan){ //asdf asdfkalsdjf
              fd = open(argvArr[posWithGreaterThan + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
              dup2(fd, 1);
              close(fd);
              for(int i = 1; i < posWithGreaterThan; i++){
                printf("%s ", argvArr[i]);
              }
            }else{
              for(int i = 1; i < numArgs; i++){
                printf("%s ", argvArr[i]);
              }
            }
            printf("\n");
            returnCode = 0;
            exit(0);
          }else{
            // close(fd);
            wait(&child_status);
          }
        }
      } 
    }
    else if(!strcmp(argvArr[0], "printenv")){
      int i = 0;
      if(hasGreaterThan){
        fd = open(argvArr[posWithGreaterThan + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
        dup2(1, 100);
        dup2(fd, 1);
        close(fd);
      }
      while(environ[i]) {
        printf("%s\n", environ[i++]);
      }
      dup2(100, 1);
      close(100);
      returnCode = 0;
    }
    else{
      if(isBuiltIn(argvArr, tempStore)){
        // printf("tempStore? %s\n", tempStore);
        if((pid = fork()) == 0){
          if(hasGreaterThan){
            fd = open(argvArr[posWithGreaterThan + 1], O_RDWR | O_CREAT | O_TRUNC, 0666);
            dup2(fd, 1);
            if(execve(tempStore, argvNoSpec, environ) < 0){
              printf("%s : Command not found in fork.\n", argvArr[0]);
              returnCode = 1;
              exit(1);
            }else{
              returnCode = 0;
              exit(0);
            }
          }
          else if(hasLessThan){
            fd = open(argvArr[posWithLessThan + 1], O_RDONLY, 0666);
            dup2(fd, 0);
            if(execve(tempStore, argvArr, environ) < 0){
              printf("%s : Command not found in fork.\n", argvArr[0]);
              returnCode = 1;
              exit(1);
            }else{
              returnCode = 0;
              exit(0);
            }
          }else{
            if(execve(tempStore, argvArr, environ) < 0){
              printf("%s : Command not found in fork.\n", argvArr[0]);
              returnCode = 1;
              exit(1);
            }else{
              returnCode = 0;
              exit(0);
            }
          }
          
        }else{
          // close(fd);
          wait(&child_status);
        }
      }else{
        char *holdFuncName = argvArr[0];
        char *aptr = malloc(strlen(holdFuncName));
        strncpy(aptr, holdFuncName, strlen(holdFuncName)-1);
        printf("%s : command not found.\n", holdFuncName);    /* ERROR IF I ENTER A LOT OF CHARS AND THEN LESS */
        free(aptr);
      }
    }

    if(!rv){
      finished = 1;
      break;
    }
    for(int i = 0; i < firstIndexSign; i++){
      free(argvNoSpec[i]);
      argvNoSpec[i] = "\0";
    }
    // firstTime = 0;
    hasGreaterThan = 0;
    hasLessThan = 0;
    pipeOrNaw = 0;
    }
  endOfWhileLoop: if(cmd){}
  }
  free(cmd);
  free(hist); 
  free(shift);
}

int checkForThePipes(char **argv, int numbArgs){
  int numPipes = 0;
  int gotOne = 0;
  for(int i = 0; i < numbArgs; i++){
    if(!strcmp(argv[i], "|") && !gotOne){
      numPipes++;
      //argv[i] = '\0';
      gotOne = 1;
    }else if(!strcmp(argv[i], "|") && gotOne){
      return -1;
    }else{
      gotOne = 0;
    }
  }
  return numPipes;
}

/* returns num args */
int buildArgv(char *cmdLine, char **argv){
  // char *temp;
  // int argc = 0;
  // char *xd = malloc(MAX_INPUT);
  // for(temp = strtok(cmdLine, " "); temp; temp = strtok(NULL, " ")){
    // strcpy(xd, temp);
    // strcat(xd, "\0");
    // argv[argc++] = temp;
  // }
  // free(xd);
  char *temp; //points to a space
  int argc = 0;
  int run = 1;
  //int bg; /*wtf*/
  cmdLine[strlen(cmdLine)-1] = ' ';       //replace \n with ' '
  while(*cmdLine && (*cmdLine == ' '))    //ignore leading spaces
    cmdLine++;
  // if(*cmdLine != '\"'){
    while(run){
      if(*cmdLine == '\"'){
        cmdLine++;
        temp = strchr(cmdLine, '\"');
        argv[argc++] = cmdLine;
        *temp = '\0';
        cmdLine = temp + 2;
        while(*cmdLine && (*cmdLine == ' '))
          cmdLine++;
      }else{ /* cmdLine != '\"' */
        temp = strchr(cmdLine, ' ');
        argv[argc++] = cmdLine;
        *temp = '\0';
        cmdLine = temp + 1;
        while(*cmdLine && (*cmdLine == ' '))
          cmdLine++;
      }
      if(*cmdLine == '\0')
        run = 0;
    }
  argv[argc] = NULL;
  return argc;
}

/* returns 1 if true, 0 if false. argv array passed in as param and 
where to store the path*/
int isBuiltIn(char **argv, char store[]){
  char *daTemp = argv[0];
  // char *test = malloc(MAX_INPUT);        THeSE 3 LINES ARE FOR RID OF \n 
  // strncpy(test, daTemp, strlen(daTemp)-1);
  char *path = getenv("PATH");

  // printf("what is the func?: %s\n", test);

  char tempPath[MAX_INPUT];
  strcpy(tempPath, path);
  char *tempArr[MAX_INPUT];
  int argc = 0;
  char *temp;
  char tempCat[MAX_INPUT];
  struct stat fStat;
  // printf("%s", argv[0]);
  for(temp = strtok(tempPath, ":"); temp; temp = strtok(NULL, ":")){
    strcpy(tempCat, temp);
    strcat(tempCat, "/");
    strcat(tempCat, daTemp);
    tempArr[argc] = tempCat;
    // printf("%s\n", tempArr[argc]);
    if(stat(tempArr[argc], &fStat) >= 0){
      break;
    }
    argc++;
  }
  if(tempArr[argc] != NULL){
    strcpy(store, tempArr[argc]);
    // free(test);
    return 1;
  }
  // free(test);
  return 0;
  // if(tempArr[argc] != NULL){ /*contains path of existing*/
  //  if((pid = Fork()) == 0){
  //    execve(tempArr[argc], )
  //  }
  // } else{
  //  printf("%s: Command not found.\n", argv[0]);
  //  return 0;
  // }
}

int isBuiltIn2(char *argv, char store[]){
  char *daTemp = argv;
  char *path = getenv("PATH");

  char tempPath[MAX_INPUT];
  strcpy(tempPath, path);
  char *tempArr[MAX_INPUT];
  int argc = 0;
  char *temp;
  char tempCat[MAX_INPUT];
  struct stat fStat;
  // printf("%s", argv[0]);
  for(temp = strtok(tempPath, ":"); temp; temp = strtok(NULL, ":")){
    strcpy(tempCat, temp);
    strcat(tempCat, "/");
    strcat(tempCat, daTemp);
    tempArr[argc] = tempCat;
    // printf("%s\n", tempArr[argc]);
    if(stat(tempArr[argc], &fStat) >= 0){
      break;
    }
    argc++;
  }
  if(tempArr[argc] != NULL){
    strcpy(store, tempArr[argc]);
    return 1;
  }
  return 0;
}

void printHelpScreen(){
  printf("CSE320 Shell, version 3.2.0(2016)-release (x86_64-pc-linux-gnu)\n");
  printf("These shell commands are defined by me. Type 'help' to see this list.\n");
  printf("Typing 'help name' will not give you more info about the function 'name'.\n\n");
  printf(" job_spec [&]\n cd [-L|[-P [-e]] [-@]] [dir]\t Changes diretory.\n");
  printf(" echo [-nE] [arg ...]\t\t Basic echo support. Print strings and expand environment variables.\n");
  printf(" exit\t\t\t\t Exits the shell.\n pwd\t\t\t\t Prints current absolute path of where you are currently located on the file system.\n");
  printf(" set\t\t\t\t Modify existing environment variables and create new ones.\n\n");
}