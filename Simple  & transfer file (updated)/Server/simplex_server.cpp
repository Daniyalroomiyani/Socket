// Standard C++ headers
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <fstream>
// Socket headers
#include <sys/socket.h>
#include <netinet/ip.h>

// UNIX system headers
#include <errno.h>
#include<unistd.h>
using namespace std;

const int SERVER_PORT = 5432;
const int MAX_PENDING = 5;
const int MAX_LINE = 256;

void handle_error(int eno, char const *msg)
{
    if (eno == 0)
        cerr << msg << endl;
    else
        cerr << msg << ": " << strerror(eno) << endl;
    exit(errno);
}

int main(int argc, char *argv[])
{
    int nread;
    socklen_t len;
    sockaddr_in sin;
    sockaddr *sin_p = (sockaddr *)&sin;
    sin.sin_family = AF_INET; // IP version 4
    sin.sin_port = htons(SERVER_PORT);
    sin.sin_addr.s_addr =   (INADDR_ANY); // Listen on all IP addresses

    // Create socket
    int s = socket(AF_INET, SOCK_STREAM, 0);
    if (s == -1)
        handle_error(errno, "simplex_server - socket");

    // Bind socket to local address
    int rstat = bind(s, sin_p, sizeof(sin));
    if (rstat == -1)
        handle_error(errno, "simplex_server - bind");

    rstat = listen(s, MAX_PENDING);
    if (rstat == -1)
        handle_error(errno, "simplex_server - listen");

    // Allocate a memory buffer for received messages
    char *buf = new char[MAX_LINE];

    // Wait for connections
    while (true)
    {
        // Upon return the sin structure will contain the address of the
        // connecting socket.  Note that a new socket is returned.
        len = sizeof(sin);
        int new_s = accept(s, sin_p, &len);
        if (new_s == -1)
            handle_error(errno, "simplex_server - accept");

        //Receive and print messages as long as client is connected.
        while (true)
        {
            nread = recv(new_s, buf, MAX_LINE, 0);
            if (nread == -1)
                handle_error(0, "simplex_server - recv");
            if (nread == 0)
                break; // client has disconnected

            string duf = buf;
            if (duf == "Send\n" || duf == "send\n")
            {

                printf("Getting Picture Size\n");

                // make file stream
                FILE *picture;
                picture = fopen("arch.png", "rb");

                // get size of file
                int size;
                fseek(picture, 0, SEEK_END);
                size = ftell(picture);
                fseek(picture, 0, SEEK_SET);

                //Send Picture Size
                printf("Sending Picture Size\n");
                char file_size[256];
                sprintf(file_size, "%d", size);
                cout << "Picture size:";
                cout << file_size << endl;
                send(new_s, file_size, sizeof(file_size), 0);

                // Send Picture as Byte Array(without need of a buffer as large as the image file)
                printf("Sending Picture as Byte Array\n");
                char send_buffer[BUFSIZ]; // no link between BUFSIZE and the file size
                cout << sizeof(send_buffer) << endl;
                printf("Send Start :\n");

                int counter = 0;
                while (!feof(picture))
                {
                    int nb = fread(send_buffer, 1, sizeof(send_buffer), picture);
                    send(new_s, send_buffer, nb, 0);
                    cout << "Buffer Send ... " << endl;
                    cout << "byte ";
                    cout << nb << endl;
                    counter += nb;
                    // no need to bzero
                }
                cout << "Sent Size:";
                cout << counter << endl;
            }

            cout << buf << flush;
        }

        close(new_s);
    }
    close(s);
    delete[] buf;

    return 0;
}
