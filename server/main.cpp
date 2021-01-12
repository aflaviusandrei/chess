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

struct response {
    string msg;
    int res;
};

player waitingPlayer;

int ongoing, poll_r;

// create mutually exclusive flag for multithreading
mutex mtx;

response handleRequest(string msg) {
    response res;

    res.msg = "";
    res.res = 0;

    if (msg.substr(0, 4) == "move") {
        res.res = 1;
        res.msg = "move:" + msg.substr(5, 4);
    }
    else if (msg.substr(0, 9) == "checkmate") {
        ongoing = 0;
        res.msg = "checkmate";
        res.res = 1;
    }

    return res;
}

void sendToClient(string msg, int sd) {
    const char *buffer = msg.c_str();

    send(sd, buffer, strlen(buffer), 0);
}

void matchListener(player p1, player p2) {
    ongoing = 1;
    char *buffer;
    response res;

    pollfd pfd;

    pfd.events = POLLIN;

    int p1d = 0, p2d = 0;

    int val;

    while (ongoing) {

        // check if there is something to read from player 1sc
        pfd.fd = p1.sd;

        if (!p1d && !p2d) {
            if ((poll_r = poll(&pfd, 1, 0)) != 0) {
                if (poll_r & POLLIN) {
                    if ((val = read(p1.sd, buffer, 1024)) != -1) {
//                        cout << "Received from p1" << endl;
                        if (val == 0) {
                            cout << "p1 disconnected" << endl;
                            close(p1.sd);
                            p1d = 1;
                            sendToClient("Opponent disconnected", p2.sd);
                            close(p2.sd);
                        } else {
                            res = handleRequest(buffer);
                            if (res.res) {
                                sendToClient(res.msg, p2.sd);
                            }
                        }
                    }
                }
            }

            // check if there is something to read from player 1
            pfd.fd = p2.sd;

            if ((poll_r = poll(&pfd, 1, 0)) != 0) {
                if (poll_r & POLLIN) {
                    if ((val = read(p2.sd, buffer, 1024)) != -1) {
//                        cout << "Received from p2" << endl;
                        if (val == 0) {
                            cout << "p2 disconnected" << endl;
                            close(p2.sd);
                            p2d = 1;
                            sendToClient("Opponent disconnected", p1.sd);
                            close(p1.sd);
                        } else {
                            res = handleRequest(buffer);
                            if (res.res) {
                                sendToClient(res.msg, p1.sd);
                            }
                        }
                    }
                }
            }
        }
    }
}

void startMatch(player p1, player p2) {
    string bfr = "start";
    const char *buffer = bfr.c_str();

    int r = ((double) rand() / (RAND_MAX)) + 1;

    if (r) {
        bfr = "color: white";
        buffer = bfr.c_str();

        cout << "p1 is white" << endl;

        send(p1.sd, buffer, strlen(buffer), 0);

        bfr = "color: black";
        buffer = bfr.c_str();
        send(p2.sd, buffer, strlen(buffer), 0);
    } else {
        bfr = "color: black";
        buffer = bfr.c_str();

        cout << "p2 is white" << endl;

        send(p1.sd, buffer, strlen(buffer), 0);

        bfr = "color: white";
        buffer = bfr.c_str();
        send(p2.sd, buffer, strlen(buffer), 0);
    }

    cout << "Starting match between " << p1.sd << " and " << p2.sd << endl;

    matchListener(p1, p2);
}

void handleClient(int client) {
    cout << "New client connected: " << client << endl;

    // locking mtx to ensure code execution stays in single thread
    mtx.lock();

    if (waitingPlayer.sd == -1) { // no other free player found, adding this one to queue
        waitingPlayer.sd = client;
    } else { // there is already a player added so we match with him
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

int main() {
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
    if (bind(sock, (struct sockaddr *) &server, sizeof(sockaddr)) == -1) {
        perror("Bind to socket failure");
        return 0;
    }

    // listen call
    if (listen(sock, 5) == -1) {
        perror("Failed listen call");
        return 0;
    }

    int current_client;

    // initialize poller to use with poll() function to check for availability of data
    pollfd pfd;

    pfd.events = POLLIN;

    while (true) {
        if ((current_client = accept(sock, nullptr, nullptr)) == -1) {
            perror("Error while accepting clients");
        } else {

            // Check if player from queue disconnected in the meantime
            if (waitingPlayer.sd != -1) {
                int poll_r;
                pfd.fd = waitingPlayer.sd;
                if ((poll_r = poll(&pfd, 1, 0)) != 0) {
                    read(sock, buffer, 1024);

                    if (strlen(buffer) == 0) {
                        waitingPlayer.sd = -1;
                        close(waitingPlayer.sd);
                    }
                }
            }

            handleClient(current_client);
        }
    }

    return 0;
}