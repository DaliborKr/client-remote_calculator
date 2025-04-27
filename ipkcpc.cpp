/**
  IPK - Projekt 1: IPK Calculator Protocol
  
  Client for the IPK Calculator Protocol, which is able to communicate with
  any server using IPKCP via TCP (textual variant) or UDP (binary variant).


  Copyright (C) 2023  Dalibor Kříčka (xkrick01)

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/


#include <iostream>
#include <string.h>
#include <regex>
#include <sys/socket.h>     //avaiable only on UNIX systems (more in CHANGELOG.md in section Limitations)
#include <arpa/inet.h>      //avaiable only on UNIX systems (more in CHANGELOG.md in section Limitations)
#include <signal.h>
#include <unistd.h>

using namespace std;

#define RET_CODE_ERR 1
#define RET_CODE_OK 0

#define REQUIRED_NUM_ARGS 7   //ipkcpc -h <host> -p <port> -m <mode>
#define BUFFER_SIZE 1024
#define UDP_DATA_PAYLOAD_OFFSET 3

bool was_interrupted = false; //global variable for handling interrupt signal

/**
 * @brief Prints help for the program
 */
void print_help(){
  cout << "NAME:\n"
      << "  ipkcpc - Client communicating via TCP/UDP\n"
      << "\n"
      << "USAGE:\n"
      << "  ipkcpc [options] [arguments ...]\n"
      << "\n"
      << "OPTIONS:\n"
      << "  --help\tdisplay how to use program and exit\n"
      << "  -h <VALUE>\thost IPv4 address to connect to\n"
      << "  -p <MODE>\tcommunication mode to use [possible values: tcp, udp]\n"
      << "  -m <INT>\thost port number to connect to\n"
      << "\n"
      << "AUTHOR:\n"
      << "  Dalibor Kříčka (xkrick01)\n\n"
      ;

  exit(RET_CODE_OK);
}


/**
 * @brief Validates given program arguments
 * 
 * @param argc number of arguments
 * @param argv array of given arguments
 */
void check_program_args(int argc, char *argv[], char *mode[], char *address[], int *port){
  if (argc == 2 && !strcmp(argv[1],"--help")){
    print_help();
  }

  if (argc != REQUIRED_NUM_ARGS){
    fprintf(stderr, "ERR: invalid number of program arguments\n");
    exit(RET_CODE_ERR);
  }

  bool hostChecked = false;
  bool portChecked = false;
  bool modeChecked = false;

  for (int i = 1; i < (REQUIRED_NUM_ARGS - 1); i++){
    //check -h argument
    if ((strcmp(argv[i],"-h") == 0) && !hostChecked){
      hostChecked = true;
      i++;

      //check IPv4 addres format
      if (!(regex_match(argv[i], regex("^(\\d{1,3}\\.){3}\\d{1,3}$")))){
        fprintf(stderr, "ERR: invalid format of IPv4 address\n");
        exit(RET_CODE_ERR);
      }
      *(address) = argv[i];
    }
    //check -p argument
    else if ((strcmp(argv[i],"-p") == 0) && !portChecked){
      portChecked = true;
      i++;

      //check port format
      if (!(regex_match(argv[i], regex("^\\d+$")))){
        fprintf(stderr, "ERR: invalid format of port\n");
        exit(RET_CODE_ERR);
      }
      *(port) = atoi(argv[i]);
    }
    //check -m argument
    else if ((strcmp(argv[i],"-m") == 0) && !modeChecked){
      modeChecked = true;
      i++;

      //check mode format
      if (!(regex_match(argv[i], regex("^udp|tcp$")))){
        fprintf(stderr, "ERR: invalid mode (tcp or udp)\n");
        exit(RET_CODE_ERR);
      }
      *(mode) = argv[i];
    }
    else{
      fprintf(stderr, "ERR: invalid arguments (the client is started using: 'ipkcpc -h <host> -p <port> -m <mode>')\n");
      exit(RET_CODE_ERR);
    }
  }

}


/**
 * @brief Creates socket
 * 
 * @param mode mode, that will be used for communication (UDP/TCP)
 * @return file descriptor for the new socket
 */
int create_socket(char *mode){
  int type;

  if (strcmp(mode, "udp") == 0){
    type = SOCK_DGRAM;    //datagram socket (UDP)
  }
  else{
    type = SOCK_STREAM;   //stream socket (TCP)
  }
  
  int new_socket = socket(AF_INET, type, 0);

  if (new_socket < 0){
    fprintf(stderr, "ERR: socket completion has failed\n");
    exit(RET_CODE_ERR);
  }

  return new_socket;
}


/**
 * @brief Sets informations about host to communicate with
 * 
 * @param ip_address IPv4 address of host
 * @param port port of host
 * @return structure describing internet socket address
 */
struct sockaddr_in set_host_informations(char *ip_address, int port){
  struct sockaddr_in server_address;
  memset(&server_address, 0, sizeof(server_address));     //initialization of the memory
  server_address.sin_family = AF_INET;
  server_address.sin_port = htons(port);
  server_address.sin_addr.s_addr = inet_addr(ip_address);

  return server_address;
}


/**
 * @brief Properly closes the tcp communication and release allocated resources
 * 
 * @param socket_client socket of the client
 */
void close_tcp_connection(int socket_client){
  shutdown(socket_client, SHUT_RD);
  shutdown(socket_client, SHUT_WR);
  shutdown(socket_client, SHUT_RDWR);
  close(socket_client);
}


/**
 * @brief Handles interrupt signal (ctrl+c)
 * 
 * @param signum type of the signal
 */
void interrupt_signal_handler(int signum){
  was_interrupted = true;
  fclose(stdin);
}


/**
 * @brief Handles UDP communication with host via STDIN
 * 
 * @param socket_client socket
 * @param address address of host
 * @param address_size size of the address of host
 */
void communicate_via_udp(int socket_client, struct sockaddr *address, socklen_t address_size){
  string message;
  char buffer[BUFFER_SIZE];

  while(true){
    getline(cin, message);                        //set message payload data

    if (was_interrupted){
      break;
    }

    message = (char) message.length() + message;  //set message opcode
    message = '\x00' + message;                   //set payload length

    bzero(buffer, BUFFER_SIZE);

    int bytes_tx = sendto(socket_client, message.c_str(), message.size(), 0, address, address_size);
    if (bytes_tx < 0) perror("ERROR: sendto");

    int bytes_rx = recvfrom(socket_client, buffer, BUFFER_SIZE, 0, address, &address_size);
    if (bytes_rx < 0) perror("ERROR: recvfrom");

    string response_data = "";

    for (int i = UDP_DATA_PAYLOAD_OFFSET; i < buffer[2] + UDP_DATA_PAYLOAD_OFFSET; i++){
      response_data += buffer[i];
    }

    if (buffer[1] == '\x00'){
      cout << "OK:" << response_data << "\n";
    }
    else{
      cout << "ERR:" << response_data << "\n";
    }

    if (cin.eof()){
      break;
    }
  }
}


/**
 * @brief Handles TCP communication with host via STDIN
 * 
 * @param address address of host
 * @param address_size size of the address of host
 */
void communicate_via_tcp(int socket_client, struct sockaddr *address, socklen_t address_size){
  string message;
  char buffer[BUFFER_SIZE];

  if (connect(socket_client, address, address_size) != 0){
    perror("ERROR: connect");
    exit(EXIT_FAILURE);
  }

  while(true){
    
    getline(cin, message);
    message += "\n";

    if (cin.eof()){
      message = "BYE\n";
    }

    bzero(buffer, BUFFER_SIZE);

    int bytes_tx = send(socket_client, message.c_str(), message.size(), 0);
    if (bytes_tx < 0) perror("ERROR: send");
    int bytes_rx = recv(socket_client, buffer, BUFFER_SIZE, 0);
    if (bytes_rx < 0) perror("ERROR: recv");
    cout << buffer;

    if (strncmp(buffer, "BYE", 3) == 0){
      break;
    }
  }

  close_tcp_connection(socket_client);
}

int main(int argc, char *argv[]) {
  signal(SIGINT, interrupt_signal_handler);

  char *mode;
  char *host_ip_address;
  int host_port;

  check_program_args(argc, argv, &mode, &host_ip_address, &host_port);

  int socket_client = create_socket(mode);

  struct sockaddr_in server_address = set_host_informations(host_ip_address, host_port);

  struct sockaddr *address = (struct sockaddr *) &server_address;
  socklen_t address_size = sizeof(server_address);

  if (strcmp(mode, "udp") == 0){
    communicate_via_udp(socket_client, address, address_size);
  }
  else{
    communicate_via_tcp(socket_client, address, address_size);
  } 

  return RET_CODE_OK;
} 