// Standard C++ headers
#include <iostream>
#include <string>
#include <cstring>
#include <cstdlib>
#include<fstream>
#include <stdio.h>
// Socket headers
#include <netdb.h>
#include <arpa/inet.h>

// UNIX system headers
#include <errno.h>
#include<unistd.h>
using namespace std;

const int SERVER_PORT=5432;
const int MAX_LINE=256;





void handle_error(int eno, char const *msg)
{
    if(eno == 0)
        cerr << msg << endl;
    else
        cerr << msg << ": "<< strerror(eno) << endl;
    exit(errno);
}

int main(int argc , char *argv[])
{
    if(argc != 2)
        handle_error(0,"usage: simplex_client host");

    char *host = argv[1];
    char *port = argv[2];
 struct addrinfo hints;
           struct addrinfo *res, *p;
    // Convert hostname to IP address
     memset(&hints, 0, sizeof(hints));
     hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
     hints.ai_socktype = SOCK_DGRAM; /* Datagram socket */
     hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
             hints.ai_protocol = 0;
    int stat = getaddrinfo(host, port , &hints , &res);



    if(stat != 0) 
        handle_error(0, "simplex_client - getaddrinfo (lookup error)");
    if(res->ai_family  != AF_INET) 
        handle_error(0, "simplex_client - gethostname (not IPv4)");

    // Translate hostname and port number to a remote socket address
    sockaddr_in sin;
    sockaddr *sin_p = (sockaddr *)&sin;
    // sin.sin_family = AF_INET;                   // IP version 4
    // sin.sin_port = htons(SERVER_PORT);
    sin_p = hints.ai_addr;
    // h_addr contains the memory address of the IP address (?!?)
    // The IP addresses are in network byte order
    //sin.sin_addr.s_addr = *(uint32_t *)hp->h_addr;
    char *dst = new char[INET_ADDRSTRLEN];
    const char *rmem = inet_ntop(AF_INET, sin_p->sa_data, dst, INET_ADDRSTRLEN);
    if(rmem == NULL) 
        handle_error(errno, "simplex_client - inet_ntop");
    cout << "server IP address: " << dst << endl;

    // Create a socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if(s == -1) 
        handle_error(errno, "simplex_client - socket");

    // Connect to the server
    int rstat = connect(s, sin_p, sizeof(sin_p));
    if(rstat == -1) 
        handle_error(errno, "simplex_client - connect");
 
  string buf;
    while (getline(cin, buf))
    {
        // getline strips the newline, we want to pass it to the server
        buf.push_back('\n');
        // send takes a C string as an argument, NOT a C++ string
        // The string and the terminating 0 is sent, hence length()+1,
        // not length()
        send(s, buf.c_str(), buf.length() + 1, 0);

        if (buf == "send\n"||buf == "Send\n")
        {

            // define buffer
            char *buff = new char[BUFSIZ];

            //Read Picture Size
            printf("Reading Picture Size\n");
            recv(s, buff, BUFSIZ, 0);
            int file_size = atoi(buff);
            cout << "Picture size:";
            cout << file_size << endl;

            // create new file for recive image
            std::ofstream imageFile;
            imageFile.open("client_image.png", std::ios::binary);

            //Read Picture Byte Array and Copy in file
            printf("Reading Picture Byte Array\n");
            ssize_t len;
            int remain_data = file_size;
            int reciveddata = 0;
            printf("Recive Started:\n");
            while ((remain_data > 0) && ((len = recv(s, buff, BUFSIZ, 0)) > 0))
            {
                imageFile.write(buff, len);
                reciveddata += len;
                remain_data -= len;
                int percent = (reciveddata*1.0 / file_size*1.0) * 100;
                cout << "Recive:";
                cout << percent;
                cout << "%" << endl;
            }

            // close stream
            imageFile.close();
            cout << "Recived Size:";
            cout << reciveddata << endl;
           // system("vlc client_image.jpg");
        }
    } 
    close(s);

    return 0;
}

