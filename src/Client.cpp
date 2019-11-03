#include <iostream>
#include <cstring>
#include <thread>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

#include "Command.hpp"

#include "commands/NameCommand.hpp"
#include "commands/WhisperCommand.hpp"
#include "commands/ListUsersCommand.hpp"

typedef struct sockaddr SA;

static std::string current_name;

int send_message(int socket, Command& command) {    
    std::size_t written_bytes = 0;
	std::size_t error = 0;
	Message message = command.serialize();
	Length length = message.length();

	std::cout << "Message about to be sent:\n";
	std::cout << message;
	while ((error = write(socket, message.c_str(), length - written_bytes)) < length) {
		if (error == -1) {
			return error;
		}

		written_bytes += error;
	}
    
	std::cout << "Message sent\n";

    return 0;
}

int init_client(struct sockaddr_in *server, std::string address, int port) {
	int sockfd;

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		std::cerr << "ERROR: unable to create a socket\n";
		return -1;
	}
	
	server->sin_family = AF_INET;
	server->sin_addr.s_addr = inet_addr(address.c_str());
	server->sin_port = htons(port);

	return sockfd;
}

int main(int argc, char **argv) {
	if (argc < 2) {
		std::cerr << "usage: server <port_number>\n";
		return EXIT_FAILURE;
	}

	int sockfd;
	static struct sockaddr_in server_address;
	int port = atoi(argv[1]);
	fd_set m_master;
	auto quit = false;

	sockfd = init_client(&server_address, "127.0.0.1", port);

	if (sockfd == EXIT_FAILURE) {
		return EXIT_FAILURE;
	}

	if (connect(sockfd, (SA*)&server_address, sizeof(server_address)) == -1) { 
		std::cerr << "ERROR: unable to connect to the server\n";
        return EXIT_FAILURE;
    }

	std::cout << "Commands available: quit(0), name(1), whisper(2), list_users(3).\nEnter a number";

	while (!quit) {
		FD_ZERO(&m_master);

		FD_SET(STDIN_FILENO, &m_master);
		FD_SET(sockfd, &m_master);

		auto retval = select(sockfd + 1, &m_master, NULL, NULL, NULL);
		if (retval == -1) {
			std::cerr << "ERROR: select() failed\n";
			quit = true;
			continue;
		}

		if (FD_ISSET(STDIN_FILENO, &m_master)) {
			std::string input, receiver, message;
			std::getline(std::cin, input);

			if (input == "0") {
				quit = true;
				continue;
			} else if (input == "1") {
				std::cout << "Enter your name: ";
				std::getline(std::cin, current_name);

				NameCommand name_command(current_name);
				send_message(sockfd, name_command);
			} else if (input == "2") {
				std::cout << "Enter the receiver: ";
				std::getline(std::cin, receiver);
				std::cout << "\n Enter the message: ";
				std::getline(std::cin, message);

				WhisperCommand whisper_command(current_name, receiver, message);
				send_message(sockfd, whisper_command);
			} else if (input == "3") {
				ListUsersCommand list_users_command;
				send_message(sockfd, list_users_command);
			}
			std::cout << "Commands available: quit(0), name(1), whisper(2), list_users(3).\nEnter a number";

			
		}

		if (FD_ISSET(sockfd, &m_master)) {
			char buffer[128] = {0};
			int retval;
			retval = recv(sockfd, buffer, sizeof(buffer), 0);

			if (retval == -1 || retval == 0) {
				std::cerr << "Server probably disconnected\n";
				quit = true;
				continue;
			}

			auto response = Command::deserialize(buffer);
			if (response->getCommandType() == Commands::LIST_USERS) {
				auto list_users = dynamic_cast<ListUsersCommand*>(response.get());
				auto users = list_users->getUsers();
				auto i = 1;
				
				std::cout << "Users connected: \n";
				for (auto& user : users) {
					std::cout << i << ".- " << user << "\n";
				}
			}
			
			std::cout << "Message recieved: " << buffer << "\n";
		}

	}

	std::cout << "Closing connection!\n";
	close(sockfd);
	return EXIT_SUCCESS;
}
