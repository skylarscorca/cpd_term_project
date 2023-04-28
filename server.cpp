// C Headers
#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <netinet/in.h>
#include <pthread.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
// C++ Headers
#include <fstream>
#include <iostream>
#include <sstream>
#include <map>
#include <string>
#include <vector>
#include <sstream>
using namespace std;

#define MSGSIZE 1024

vector<pair<string, vector<string> > > lines;
vector<pair<int, string> > users;

//readFile - reads lines in file into the data structure lines
void readFile(string filename){
    // Check to see if a prior user already opened the file
    for(auto fileLine : lines){
        if(fileLine.first == filename) return;
    }

    string line;
	ifstream file(filename);
    lines.push_back(pair<string, vector<string> >(filename, vector<string>()));
	while (getline(file, line)){
		lines.back().second.push_back(line);
	}
}

//sendFile - send file line-by-line to a client at fd
void sendFile(int userIndex){
	send(users[userIndex].first, (void*)"Start Transfer", MSGSIZE, 0);
	
    // Find user file
    unsigned int fileIndex;
    for(fileIndex = 0; fileIndex < lines.size(); ++fileIndex){
        if(lines[fileIndex].first == users[userIndex].second) break;
    }
    
    // Transfer file
    for(string msg : lines[fileIndex].second){
		char buffer[MSGSIZE];
		send(users[userIndex].first, msg.c_str(), MSGSIZE, 0);
		read(users[userIndex].first, buffer, MSGSIZE);
	}

	send(users[userIndex].first, (void*)"End Transfer", MSGSIZE, 0);
}

//threadFunc - thread function to read any messages from a client
void *threadFunc(void *args){
	ssize_t n;
	int clientFd = *(int*)args;
	char buffer[MSGSIZE];
	bool copied = false;

	pthread_detach(pthread_self());

	// Read until disconnection
	while ((n = read(clientFd, buffer, MSGSIZE)) > 0){
		string line(buffer);
		
		if (line == "exit"){
			break;
		}
		else if (line.substr(0,4) == "get "){
			cout << "Get Received" << endl;
            for(unsigned i = 0; i < users.size(); ++i){
                if(users[i].first == clientFd){
                    users[i].second = line.substr(4);
                    readFile(users[i].second);
                    sendFile(i);	
                }
            }
			copied = true;
		}
		else if(copied){
            // Find user
            unsigned int userIndex;
            for(userIndex = 0; userIndex < users.size(); ++userIndex){
                if(users[userIndex].first == clientFd) break;
            }
            // Find user file
            unsigned int fileIndex;
            for(fileIndex = 0; fileIndex < lines.size(); ++fileIndex){
                if(lines[fileIndex].first == users[userIndex].second) break;
            }

			stringstream ss(line);
			cout << line << endl;
			//get update type
            bool validCMD = true;
            string cmd, row, col, text;
			getline(ss, cmd, ':');
			
			// Update lines with changes
			if(cmd == "ir"){
				getline(ss, row, ':');
				getline(ss, text, ':');
				lines[fileIndex].second.insert(lines[fileIndex].second.begin() + stoi(row), text);
			} else if(cmd == "dr"){
				getline(ss, row, ':');
				lines[fileIndex].second.erase(lines[fileIndex].second.begin() + stoi(row));
			} else if(cmd == "ic"){
				getline(ss, row, ':');
				getline(ss, col, ':');
				getline(ss, text, ':');
				if (text == ""){
					// cout << "EMPTY\n";
					lines[fileIndex].second[stoi(row)] = lines[fileIndex].second[stoi(row)].substr(0, stoi(col));
				} else {
					lines[fileIndex].second[stoi(row)].insert(stoi(col), text);
				}
			} else if(cmd == "as"){
				getline(ss, row, ':');
				getline(ss, text, ':');
				lines[fileIndex].second[stoi(row)].append(text);
			} else if(cmd == "dc"){
				getline(ss, row, ':');
				getline(ss, col, ':');
				lines[fileIndex].second[stoi(row)].erase(stoi(col), 1);
			} else{
				cout << "Error: " << cmd << " is not a valid update type\n";
                validCMD = false;
			}

            if(validCMD){
                // Find user for masterfile update
                unsigned int userIndex;
                for(userIndex = 0; userIndex < users.size(); ++userIndex){
                    if(users[userIndex].first == clientFd) break;
                }
				    
                // Update master file
                for(unsigned int fileIndex = 0; fileIndex < lines.size(); ++fileIndex){
                    if(lines[fileIndex].first == users[userIndex].second) break;
                }
				ofstream file(lines[fileIndex].first);
				for (string line : lines[fileIndex].second) file << line << endl;
				file.close();

                // Send update messages to other users editing the same file
                for(unsigned int i = 0; i < users.size(); ++i){
                    if(i == userIndex) continue;
                    else if(users[i].second == users[userIndex].second)
                        write(users[i].first, buffer, MSGSIZE);
                }
            }
		}
	}

    for(unsigned i = 0; i < users.size(); ++i){
        if(users[i].first == clientFd) users.erase(users.begin()+i);
    }
    close(clientFd);
    pthread_exit(NULL);

	return NULL;
}

//handleSigInt - action performed when user types ctrl-C
void handleSigInt(int unused __attribute__((unused))){
	exit(0);
}

//main server program
int main(int argc, char *argv[]){
	if (argc != 2){
		cerr << "Usage: server <port>\n";
		return 1;
	}
	stringstream stream(argv[1]);
	int port;
	stream >> port;
	//setup SIGINT signal handler
	signal(SIGINT, handleSigInt);

	// Create server socket
	int serverFd;
	if ((serverFd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		std::cerr << "Error: Can't create socket." << std::endl;
		return 1;
	}

	// Set options for socket
	int val;
	if (setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(int))){
		std::cerr << "Error: Can't reuse socket." << std::endl;
		return 2;
	}

	// Configure addr and bind
	struct sockaddr_in serverAddr;
	serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(port);
	if (bind(serverFd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1){
		cerr << "Error: Can't bind socket to port." << endl;
		return 3;
	}

	// Listen for client connections
	if (listen(serverFd, 20) < 0){
		cerr << "Error: Can't listen for clients." << endl;
		return 4;
	}

	// Get name and port assigned to server
	char *name = new char[MSGSIZE];
	struct sockaddr_in infoAddr;
	socklen_t len = sizeof(infoAddr);
	gethostname(name, MSGSIZE);
	getsockname(serverFd, (struct sockaddr *)&infoAddr, &len);

	// Report name and port
	cout << name << ":" << ntohs(serverAddr.sin_port)<< endl;
	delete name;

	// Connect a client
	struct sockaddr_in cliAddr;
	len = sizeof(cliAddr);
	while (true){
		users.push_back(pair<int, string>(accept(serverFd, (struct sockaddr *)&serverAddr, &len), ""));
        cout << "# of connected users: " << users.size() << endl;
		// Create thread to deal with client
		pthread_t thread;
		pthread_create(&thread, NULL, threadFunc, (void *)&users[users.size()-1].first);
	}
	return 0;
}
