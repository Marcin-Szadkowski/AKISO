int execArgsPiped(char** parsed, char** parsedpipe) 
{ 
    // 0 is read end, 1 is write end 
    int pipefd[2];  
    pid_t p1, p2, wpid1, wpid2;
    int status1, status2; 
  
    if (pipe(pipefd) < 0) { 
        printf("\nPipe could not be initialized"); 
        perror("lsh");
    } 
    p1 = fork(); 
    if (p1 < 0) { 
        printf("\nCould not fork"); 
        perror("lsh");
    } 
  
    if (p1 == 0) { 
        // Child 1 executing.. 
        // It only needs to write at the write end 
        close(pipefd[0]); 
        dup2(pipefd[1], 1); 
        //close(pipefd[0]); 
        close(pipefd[1]);

        if (execvp(parsed[0], parsed) < 0) { 
            printf("\nCould not execute command 1.."); 
            exit(EXIT_FAILURE);
        } 
    } else { 
        // Parent executing 
        p2 = fork(); 
  
        if (p2 < 0) { 
            printf("\nCould not fork"); 
            perror("lsh"); 
        } 
  
        // Child 2 executing.. 
        // It only needs to read at the read end 
        if (p2 == 0) { 
            close(pipefd[1]); 
            dup2(pipefd[1], STDIN_FILENO); 
            close(pipefd[0]); 
            //close(pipefd[1]);
            
            if (execvp(parsedpipe[0], parsedpipe) < 0) { 
                printf("\nCould not execute command 2.."); 
                exit(EXIT_FAILURE); 
            } 
        } else { 
            // parent executing, waiting for two children
            do {
              wpid1 = waitpid(p1, &status1, WUNTRACED);
              wpid2 = waitpid(p2, &status2, WUNTRACED);
            } while (!WIFEXITED(status1) && !WIFSIGNALED(status1)  && !WIFEXITED(status2) && !WIFSIGNALED(status2)); 
           // wait(NULL); 
           // wait(NULL); 
        } 
    }
    return 1; 
} 