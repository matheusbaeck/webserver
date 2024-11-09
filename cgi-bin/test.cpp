/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   test.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: glacroix <PGCL>                            +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2024/11/07 16:02:23 by glacroix          #+#    #+#             */
/*   Updated: 2024/11/09 17:46:12 by glacroix         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

int main(int argc, char **argv, char **envp)
{
    int pipefd[2];
    char *const args[] = {const_cast<char *>("hello.py"), nullptr};

    if (pipe(pipefd) == -1)
    {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    int pid = fork();
    if (pid == -1)
    {
        perror("fork");
        exit(EXIT_FAILURE);
    }

    if (pid == 0)
    {
        close(pipefd[0]); //close unused write end
        dup2(pipefd[1], STDOUT_FILENO); //redirect STDOUT to pipe
        execve("/home/glacroix/Documents/pushed-webserv/cgi-bin", args, envp); 
        close(pipefd[1]);
    }
    else 
    {
        close(pipefd[1]); // Close write end

        char buffer[1024];
        ssize_t bytesRead;
        while ((bytesRead = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytesRead] = '\0';
            std::cout << buffer; // Send this output to the client
        }
        close(pipefd[0]);
        waitpid(pid, nullptr, 0);  
        
    }
    
    return 0;
}
