#include <cstdio>
#include <cstring>
#include <iostream>
#include <list>
#include <mutex>
#include <netinet/in.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <thread>
#include <unistd.h>

using namespace std;

#define PORT 8888

struct player {
    int sd = -1; // store player's socket descriptor
    int matched = 0; // store whether player is in a match, by default 0
    int adv_sd; // store adversary's sd
} dummy;

player waitingPlayer;

// create mutually exclusive flag for multithreading
mutex mtx;

void startMatch (player p1, player p2) {
    string bfr = "You start";
    const char * buffer = bfr.c_str();

    send(p1.sd, buffer, strlen(buffer), 0);

    cout << "Starting match between " << p1.sd << " and " << p2.sd << endl;


}

void handleClient (int client) {
    cout << "New client connected: " << client << endl;

    // locking mtx to ensure code execution stays in single thread
    mtx.lock();

    if (waitingPlayer.sd == -1) { // no other free player found, adding this one to queue
        waitingPlayer.sd = client;
    }
    else { // there is already a player added so we match with him
        player newPlayer;
        newPlayer.sd = client;
        newPlayer.matched = 1;
        newPlayer.adv_sd = waitingPlayer.sd;
        waitingPlayer.matched = 1;
        waitingPlayer.adv_sd = newPlayer.sd;

        // spawn new thread and tell it to start new match with two players
        thread new_thread(startMatch, waitingPlayer, newPlayer);

        // detach thread from main thread to allow parallel execution
        new_thread.detach();

        // clear queue
        waitingPlayer.sd = -1;
    }

    // unlock mtx
    mtx.unlock();
}

int main () {
    sockaddr_in server;
    int sock, i, val;
    char buffer[1025];

    bzero(&server, sizeof server);
    server.sin_family = AF_INET;
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    server.sin_port = htons(PORT);

    // initialize socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Failed to initialize socket");
        return 0;
    }

    // set sockopt
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &i, sizeof(i)) == -1) {
        perror("Error setting sockopt");
        return 0;
    }

    // bind to socket
    if (bind(sock, (struct sockaddr*)&server, sizeof(sockaddr)) == -1) {
        perror("Bind to socket failure");
        return 0;
    }

    // listen call
    if (listen(sock, 5) == -1) {
        perror("Failed listen call");
        return 0;
    }

    int current_client;

    while (true) {
        if((current_client = accept(sock, nullptr, nullptr)) == -1) {
            perror("Error while accepting clients");
        }
        else {
            handleClient(current_client);
        }

        // check if client in queue is still connected

//        if ((val = read(waitingPlayer.sd, buffer, 1024)) == 0) { // player in queue disconnected, remove from queue
//            cout << "Removing player " << waitingPlayer.sd << " from queue" << endl;
//            close(waitingPlayer.sd);
//            waitingPlayer.sd = -1;
//        }
    }

    return 0;
}