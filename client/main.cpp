#include <SFML/Graphics.hpp>
#include <iostream>
#include <utility>

using namespace std;

sf::Texture w_pawn, b_pawn, w_king, b_king, w_queen, b_queen,
        w_rook, b_rook, w_bishop, b_bishop, w_knight, b_knight;

sf::Cursor hand, arrow;

int mousePressed = 0;

struct piece {
    int type;
};

struct player {
    piece pieces[16];
    int leftPieces = 16,
            capturedPieces = 0;
};

struct square {
    int x, y, sx, sy, hasPiece = 0, side, color;
    sf::Sprite sprite;
    sf::Texture texture;
    string type;
};

// Basic board initialization

square board[8][8];
int i, j;

int w = sf::VideoMode::getDesktopMode().width,
        h = sf::VideoMode::getDesktopMode().height;

// calculate board width, height and square width
int bw = w - (0.3 * w);
int bh = h - (0.3 * h);
int sqh = bh / 8;

int grabI = -1, grabJ = -1;

// open window
sf::RenderWindow window(sf::VideoMode(w, h), "Chess");

int validatePawn(int origX, int origY, int destX, int destY, int side) {
    if (!side) {
        if (origX >= destX) return 0;

        if (origX == 1) {
            if (destX == 3 && board[destX - 1][destY].hasPiece) return 0;
            if (destX > 3) return 0;
        } else if (destX > origX + 1) return 0;

        if ((destX == origX + 1 && (destY == origY + 1 || destY == origY - 1))
            && (!board[destX][destY].hasPiece || board[destX][destY].side == board[origX][origY].side))
            return 0;
    } else {
        if (origX <= destX) return 0;

        if (origX == 6) {
            if (destX == 4 && board[destX + 1][destY].hasPiece) return 0;
            if (destX < 4) return 0;
        } else if (destX < origX - 1) return 0;

        if ((destX == origX - 1 && (destY == origY + 1 || destY == origY - 1))
            && (!board[destX][destY].hasPiece || board[destX][destY].side == board[origX][origY].side))
            return 0;
    }

    if (destY > origY + 1 || destY < origY - 1) return 0;

    if (destY == origY) {
        if (board[destX][destY].hasPiece) return 0;
    }

    return 1;
}

int validateQueen(int origX, int origY, int destX, int destY, int side) {

    if (board[destX][destY].side == board[origX][origY].side) return 0;

    if (origX != destX && origY != destY) {
        int cx = origX, cy = origY;

        while (cx != destX && cy != destY) {
            if (cy < destY) cy++;
            else cy--;

            if (cx < destX) cx++;
            else cx--;

            if (board[cx][cy].hasPiece) return 0;
        }

        if (cx != destX || cy != destY) return 0;
    }

    if (origY == destY) {
        int cx = origX;

        while (cx < destX - 1) {
            cx++;

            if (board[cx][destY].hasPiece) return 0;
        }

        while (cx > destX + 1) {
            cx--;

            if (board[cx][destY].hasPiece) return 0;
        }
    }

    if (origX == destX) {
        int cy = origY;

        while (cy > destY + 1) {
            cy--;

            if (board[destX][cy].hasPiece) return 0;
        }

        while (cy < destY - 1) {
            cy++;

            if (board[destX][cy].hasPiece) return 0;
        }
    }

    return 1;
}

int validateKnight(int origX, int origY, int destX, int destY, int side) {
    if (board[destX][destY].hasPiece && board[destX][destY].side == board[origX][origY].side) return 0;
    int fit = 0, si, sj;

    int ti = destX, tj = destY;

    tj--;
    ti -= 2;
    if (ti == origX && tj == origY) fit = 1;
    ti = destX, tj = destY;
    tj++;
    ti -= 2;
    if (ti == origX && tj == origY) fit = 1;
    ti = destX, tj = destY;
    tj--;
    ti += 2;
    if (ti == origX && tj == origY) fit = 1;
    ti = destX, tj = destY;
    tj++;
    ti += 2;
    if (ti == origX && tj == origY) fit = 1;
    ti = destX, tj = destY;
    ti--;
    tj -= 2;
    if (ti == origX && tj == origY) fit = 1;
    ti = destX, tj = destY;
    ti++;
    tj -= 2;
    if (ti == origX && tj == origY) fit = 1;
    ti = destX, tj = destY;
    ti--;
    tj += 2;
    if (ti == origX && tj == origY) fit = 1;
    ti = destX, tj = destY;
    ti++;
    tj += 2;
    if (ti == origX && tj == origY) fit = 1;

    return fit;
}

int validateBishop(int origX, int origY, int destX, int destY, int side) {
    if (board[destX][destY].hasPiece && board[destX][destY].side == board[origX][origY].side) return 0;

    if (origX == destX || origY == destY) return 0;

    if (origX != destX && origY != destY) {
        int cx = origX, cy = origY;

        while (cx != destX && cy != destY) {
            if (cy < destY) cy++;
            else cy--;

            if (cx < destX) cx++;
            else cx--;

            if (board[cx][cy].hasPiece) return 0;
        }

        if (cx != destX || cy != destY) return 0;
    }

    return 1;
}

int validateKing(int origX, int origY, int destX, int destY, int side) {
    if (board[destX][destY].hasPiece && board[destX][destY].side == board[origX][origY].side) return 0;

    if (abs(destX - origX) > 1 || abs(destY - origY) > 1) return 0;

    return 1;
}

int validateRook (int origX, int origY, int destX, int destY, int side) {
    if (board[destX][destY].hasPiece && board[destX][destY].side == board[origX][origY].side) return 0;

    if (origX != destX && origY != destY) return 0;

    if (origX == destX) {
        int tj = origY;

        while (tj > destY + 1) {
            tj--;
            if (board[origX][tj].hasPiece) return 0;
        }

        while (tj < destY - 1) {
            tj++;
            if (board[origX][tj].hasPiece) return 0;
        }
    }

    if (origY == destY) {
        int ti = origX;

        while (ti > destX + 1) {
            ti--;
            if (board[ti][origY].hasPiece) return 0;
        }

        while (ti < destX - 1) {
            ti++;
            if (board[ti][origY].hasPiece) return 0;
        }
    }

    return 1;
}

int isCheck() {
    int ti, tj;

    struct king {
        int i, j, side;
    } k[2];

    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++) {
            if (board[i][j].hasPiece && board[i][j].type == "king") {
                if (board[i][j].side) {
                    k[0].i = i;
                    k[0].j = j;
                    k[0].side = board[i][j].side;
                } else {
                    k[1].i = i;
                    k[1].j = j;
                    k[1].side = board[i][j].side;
                }
            }
        }

    // Horizontally and vertically check for rooks and queens
    for (int x = 0; x < 2; x++) {
        // check north
        ti = k[x].i - 1;
        if (ti >= 0) {
            while (!board[ti][k[x].j].hasPiece && ti > 0) ti--;
            if ((board[ti][k[x].j].type == "queen" || board[ti][k[x].j].type == "rook")
                && board[ti][k[x].j].side != k[x].side) return 1;
        }

        // check south
        ti = k[x].i + 1;
        if (ti <= 7) {
            while (!board[ti][k[x].j].hasPiece && ti < 7) ti++;
            if ((board[ti][k[x].j].type == "queen" || board[ti][k[x].j].type == "rook")
                && board[ti][k[x].j].side != k[x].side) return 1;
        }

        // check east
        tj = k[x].j + 1;
        if (tj <= 7) {
            while (!board[k[x].i][tj].hasPiece && tj < 7) tj++;
            if ((board[k[x].i][tj].type == "queen" || board[k[x].i][tj].type == "rook")
                && board[k[x].i][tj].side != k[x].side) return 1;
        }

        // check west
        tj = k[x].j - 1;
        if (tj >= 0) {
            while (!board[k[x].i][tj].hasPiece && tj > 0) tj--;
            if ((board[k[x].i][tj].type == "queen" || board[k[x].i][tj].type == "rook")
                && board[k[x].i][tj].side != k[x].side) return 1;
        }
    }

    // Check for bishops and queens

    for (int x = 0; x < 2; x++) {
        // check northeast
        ti = k[x].i - 1;
        tj = k[x].j + 1;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            while (ti > 0 && tj < 7 && !board[ti][tj].hasPiece) {
                ti--;
                tj++;
            }

            if (board[ti][tj].hasPiece) {
                if ((board[ti][tj].type == "queen" || board[ti][tj].type == "bishop") && board[ti][tj].side != board[k[x].i][k[x].j].side) return 1;
            }
        }

        // check northwest
        ti = k[x].i - 1;
        tj = k[x].j - 1;

        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            while (ti > 0 && tj > 0 && !board[ti][tj].hasPiece) {
                ti--;
                tj--;
            }

            if (board[ti][tj].hasPiece) {
                if ((board[ti][tj].type == "queen" || board[ti][tj].type == "bishop") && board[ti][tj].side != board[k[x].i][k[x].j].side) return 1;
            }
        }

        // check southeast
        ti = k[x].i + 1;
        tj = k[x].j + 1;

        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            while (ti < 7 && tj < 7 && !board[ti][tj].hasPiece) {
                ti++;
                tj++;
            }

            if (board[ti][tj].hasPiece) {
                if ((board[ti][tj].type == "queen" || board[ti][tj].type == "bishop") && board[ti][tj].side != board[k[x].i][k[x].j].side) return 1;
            }
        }

        // check southwest
        ti = k[x].i + 1;
        tj = k[x].j - 1;

        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            while (ti < 7 && tj > 0 && !board[ti][tj].hasPiece) {
                ti++;
                tj--;
            }

            if (board[ti][tj].hasPiece) {
                if ((board[ti][tj].type == "queen" || board[ti][tj].type == "bishop") && board[ti][tj].side != board[k[x].i][k[x].j].side) return 1;
            }
        }
    }

    // check for pawns
    for (int x = 0; x < 2; x++) {
        ti = k[x].i;
        tj = k[x].j;
        if (k[x].side) {
            if ((board[ti - 1][tj - 1].hasPiece && board[ti - 1][tj - 1].type == "pawn" && board[ti - 1][tj - 1].side != board[ti][tj].side)
                || (board[ti - 1][tj + 1].hasPiece && board[ti - 1][tj + 1].type == "pawn" && board[ti - 1][tj + 1].side != board[ti][tj].side)) return 1;
        }
        else {
            if ((board[ti + 1][tj - 1].hasPiece && board[ti + 1][tj - 1].type == "pawn" && board[ti + 1][tj - 1].side != board[ti][tj].side)
                || (board[ti + 1][tj + 1].hasPiece && board[ti + 1][tj + 1].type == "pawn" && board[ti + 1][tj + 1].side != board[ti][tj].side)) return 1;
        }
    }

    // check for knights

    for (int x = 0; x < 2; x++) {
        ti = k[x].i;
        tj = k[x].j;
        ti--;
        tj -= 2;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            if (board[ti][tj].hasPiece && board[ti][tj].type == "knight"
                && board[ti][tj].side != board[k[x].i][k[x].j].side)
                return 1;
        }

        ti = k[x].i;
        tj = k[x].j;
        ti--;
        tj += 2;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            if (board[ti][tj].hasPiece && board[ti][tj].type == "knight"
                && board[ti][tj].side != board[k[x].i][k[x].j].side)
                return 1;
        }

        ti = k[x].i;
        tj = k[x].j;
        ti++;
        tj -= 2;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            if (board[ti][tj].hasPiece && board[ti][tj].type == "knight"
                && board[ti][tj].side != board[k[x].i][k[x].j].side)
                return 1;
        }

        ti = k[x].i;
        tj = k[x].j;
        ti++;
        tj += 2;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            if (board[ti][tj].hasPiece && board[ti][tj].type == "knight"
                && board[ti][tj].side != board[k[x].i][k[x].j].side)
                return 1;
        }

        ti = k[x].i;
        tj = k[x].j;
        ti -= 2;
        tj--;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            if (board[ti][tj].hasPiece && board[ti][tj].type == "knight"
                && board[ti][tj].side != board[k[x].i][k[x].j].side)
                return 1;
        }

        ti = k[x].i;
        tj = k[x].j;
        ti += 2;
        tj--;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            if (board[ti][tj].hasPiece && board[ti][tj].type == "knight"
                && board[ti][tj].side != board[k[x].i][k[x].j].side)
                return 1;
        }

        ti = k[x].i;
        tj = k[x].j;
        ti -= 2;
        tj++;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            if (board[ti][tj].hasPiece && board[ti][tj].type == "knight"
                && board[ti][tj].side != board[k[x].i][k[x].j].side)
                return 1;
        }

        ti = k[x].i;
        tj = k[x].j;
        ti += 2;
        tj++;
        if (ti >= 0 && ti <= 7 && tj >= 0 && tj <= 7) {
            if (board[ti][tj].hasPiece && board[ti][tj].type == "knight"
                && board[ti][tj].side != board[k[x].i][k[x].j].side)
                return 1;
        }
    }

    // check for kings

    if (abs(k[0].i - k[1].i) <= 1 && abs(k[0].j - k[1].j) <= 1) return 1;

    return 0;
}

void repaint () {
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++) {
            if (board[i][j].hasPiece) {
                if (board[i][j].type == "queen") {
                    if (!board[i][j].color) {
                        board[i][j].texture = b_queen;
                        board[i][j].sprite.setTexture(b_queen);
                    }
                    else {
                        board[i][j].texture = w_queen;
                        board[i][j].sprite.setTexture(w_queen);
                    }
                }
                if (board[i][j].type == "knight") {
                    if (!board[i][j].color) {
                        board[i][j].texture = b_knight;
                        board[i][j].sprite.setTexture(b_knight);
                    }
                    else {
                        board[i][j].texture = w_knight;
                        board[i][j].sprite.setTexture(w_knight);
                    }
                }
                if (board[i][j].type == "rook") {
                    if (!board[i][j].color) {
                        board[i][j].texture = b_rook;
                        board[i][j].sprite.setTexture(b_rook);
                    }
                    else {
                        board[i][j].texture = w_rook;
                        board[i][j].sprite.setTexture(w_rook);
                    }
                }
                if (board[i][j].type == "bishop") {
                    if (!board[i][j].color) {
                        board[i][j].texture = b_bishop;
                        board[i][j].sprite.setTexture(b_bishop);
                    }
                    else {
                        board[i][j].texture = w_bishop;
                        board[i][j].sprite.setTexture(w_bishop);
                    }
                }
                if (board[i][j].type == "pawn") {
                    if (!board[i][j].color) {
                        board[i][j].texture = b_pawn;
                        board[i][j].sprite.setTexture(b_pawn);
                    }
                    else {
                        board[i][j].texture = w_pawn;
                        board[i][j].sprite.setTexture(w_pawn);
                    }
                }
                if (board[i][j].type == "king") {
                    if (!board[i][j].color) {
                        board[i][j].texture = b_king;
                        board[i][j].sprite.setTexture(b_king);
                    }
                    else {
                        board[i][j].texture = w_king;
                        board[i][j].sprite.setTexture(w_king);
                    }
                }
            }
        }
}

void grabPiece(int mx, int my) {
    for (i = 0; i < 8; i++)
        for (j = 0; j < 8; j++) {
            if (mx > board[i][j].x && mx < board[i][j].x + sqh
                && my > board[i][j].y && my < board[i][j].y + sqh) {
                grabI = i;
                grabJ = j;
            }
        }

    if (!board[grabI][grabJ].hasPiece) {
        grabI = -1;
        grabJ = -1;
        mousePressed = 0;
    }
    else {
        window.setMouseCursor(hand);
    }
}

void movePiece(int mx, int my) {
    if (mousePressed && grabI != -1 && grabJ != -1) {
        int pi = grabI, pj = grabJ;

        if (board[pi][pj].hasPiece) {
            board[pi][pj].sx = mx - 30;
            board[pi][pj].sy = my - 30;
        }
    }
}

void printBoard () {
    for (i = 0; i < 8; i++) {
        for (j = 0; j < 8; j++) {
            cout << board[i][j].type << " ";
        }
        cout << endl;
    }
    cout << endl << endl;
}

void dropPiece(int mx, int my) {
    if (mousePressed && grabI != -1 && grabJ != -1) {
        int fi = -1, fj = -1;

        window.setMouseCursor(arrow);

        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++) {
                if (mx > board[i][j].x && mx < board[i][j].x + sqh
                    && my > board[i][j].y && my < board[i][j].y + sqh) {
                    fi = i;
                    fj = j;
                }
            }

        int valid = 1;

        if (board[grabI][grabJ].type == "pawn") {
            valid = validatePawn(grabI, grabJ, fi, fj, board[grabI][grabJ].side);
        }
        if (board[grabI][grabJ].type == "queen") {
            valid = validateQueen(grabI, grabJ, fi, fj, board[grabI][grabJ].side);
        }
        if (board[grabI][grabJ].type == "knight") {
            valid = validateKnight(grabI, grabJ, fi, fj, board[grabI][grabJ].side);
        }
        if (board[grabI][grabJ].type == "rook") {
            valid = validateRook(grabI, grabJ, fi, fj, board[grabI][grabJ].side);
        }
        if (board[grabI][grabJ].type == "bishop") {
            valid = validateBishop(grabI, grabJ, fi, fj, board[grabI][grabJ].side);
        }
        if (board[grabI][grabJ].type == "king") {
            valid = validateKing(grabI, grabJ, fi, fj, board[grabI][grabJ].side);
        }

        if (valid) {
            string iniType = board[fi][fj].type;
            int initHasPiece = board[fi][fj].hasPiece, initSide = board[fi][fj].side;
            board[fi][fj].hasPiece = 1;
            board[fi][fj].type = board[grabI][grabJ].type;
            board[fi][fj].side = board[grabI][grabJ].side;
            board[grabI][grabJ].hasPiece = 0;
            board[grabI][grabJ].side = -1;
            board[grabI][grabJ].type = "";

            if (isCheck()) {
                valid = 0;
            }

            board[grabI][grabJ].hasPiece = 1;
            board[grabI][grabJ].side = board[fi][fj].side;
            board[grabI][grabJ].type = board[fi][fj].type;
            board[fi][fj].hasPiece = initHasPiece;
            board[fi][fj].type = iniType;
            board[fi][fj].side = initSide;
        }

        if (fi == -1 || fj == -1 || !valid) {
            board[grabI][grabJ].sx = board[grabI][grabJ].x + ((sqh - 60) / 2);
            board[grabI][grabJ].sy = board[grabI][grabJ].y + ((sqh - 60) / 2);
        } else {
            board[fi][fj].sprite.setTexture(board[grabI][grabJ].texture);
            board[fi][fj].texture = board[grabI][grabJ].texture;
            board[fi][fj].hasPiece = 1;
            board[fi][fj].sx = board[fi][fj].x + ((sqh - 60) / 2);
            board[fi][fj].sy = board[fi][fj].y + ((sqh - 60) / 2);
            board[fi][fj].type = board[grabI][grabJ].type;
            board[fi][fj].side = board[grabI][grabJ].side;
            board[fi][fj].color = board[grabI][grabJ].color;
            board[grabI][grabJ].hasPiece = 0;
            board[grabI][grabJ].side = -1;
            board[grabI][grabJ].type = "";
        }

        repaint();
    }
}

void initSquare(int x, int y, const sf::Texture &sprite, string type, int side, int color) {
    board[x][y].sprite.setTexture(sprite);
    board[x][y].hasPiece = 1;
    board[x][y].sx = board[x][y].x + ((sqh - 60) / 2);
    board[x][y].sy = board[x][y].y + ((sqh - 60) / 2);
    board[x][y].texture = sprite;
    board[x][y].type = std::move(type);
    board[x][y].side = side;
    board[x][y].color = color;
}

int main() {

    hand.loadFromSystem(sf::Cursor::Hand);
    arrow.loadFromSystem(sf::Cursor::Arrow);

    // initalize x, y coordinates of first square
    board[0][0].x = (w - bh) / 2;
    board[0][0].y = 0.3 * h / 2;

    w_pawn.loadFromFile("/home/flav/Documents/Chess/client/assets/w_pawn.png");
    w_king.loadFromFile("/home/flav/Documents/Chess/client/assets/w_king.png");
    w_queen.loadFromFile("/home/flav/Documents/Chess/client/assets/w_queen.png");
    w_rook.loadFromFile("/home/flav/Documents/Chess/client/assets/w_rook.png");
    w_bishop.loadFromFile("/home/flav/Documents/Chess/client/assets/w_bishop.png");
    w_knight.loadFromFile("/home/flav/Documents/Chess/client/assets/w_knight.png");
    b_pawn.loadFromFile("/home/flav/Documents/Chess/client/assets/b_pawn.png");
    b_king.loadFromFile("/home/flav/Documents/Chess/client/assets/b_king.png");
    b_queen.loadFromFile("/home/flav/Documents/Chess/client/assets/b_queen.png");
    b_rook.loadFromFile("/home/flav/Documents/Chess/client/assets/b_rook.png");
    b_bishop.loadFromFile("/home/flav/Documents/Chess/client/assets/b_bishop.png");
    b_knight.loadFromFile("/home/flav/Documents/Chess/client/assets/knight.png");

    // set up rectangle shape to draw board squares
    sf::RectangleShape rectangle(sf::Vector2f(sqh, sqh));
    rectangle.setFillColor(sf::Color::Black);
    rectangle.setOutlineColor(sf::Color::Black);
    rectangle.setOutlineThickness(2);

    for (j = 1; j < 8; j++) {
        board[0][j].x = board[0][j - 1].x + sqh;
        board[0][j].y = board[0][j - 1].y;
    }

    for (i = 1; i < 8; i++) {
        board[i][0].y = board[i - 1][0].y + sqh;
        board[i][0].x = board[i - 1][0].x;

        for (j = 1; j < 8; j++) {
            board[i][j].x = board[i][j - 1].x + sqh;
            board[i][j].y = board[i][j - 1].y;
        }
    }

    int fc, sc;

    fc = rand() % 2;
    if (fc) sc = 0;
    else sc = 1;

    if (fc) {
        initSquare(0, 0, w_rook, "rook", 0, fc);
        initSquare(0, 1, w_knight, "knight", 0, fc);
        initSquare(0, 2, w_bishop, "bishop", 0, fc);
        initSquare(0, 3, w_queen, "queen", 0, fc);
        initSquare(0, 4, w_king, "king", 0, fc);
        initSquare(0, 5, w_bishop, "bishop", 0, fc);
        initSquare(0, 6, w_knight, "knight", 0, fc);
        initSquare(0, 7, w_rook, "rook", 0, fc);

        for (j = 0; j < 8; j++) {
            initSquare(1, j, w_pawn, "pawn", 0, fc);
        }
    }
    else {
        initSquare(0, 0, b_rook, "rook", 0, fc);
        initSquare(0, 1, b_knight, "knight", 0, fc);
        initSquare(0, 2, b_bishop, "bishop", 0, fc);
        initSquare(0, 3, b_queen, "queen", 0, fc);
        initSquare(0, 4, b_king, "king", 0, fc);
        initSquare(0, 5, b_bishop, "bishop", 0, fc);
        initSquare(0, 6, b_knight, "knight", 0, fc);
        initSquare(0, 7, b_rook, "rook", 0, fc);

        for (j = 0; j < 8; j++) {
            initSquare(1, j, b_pawn, "pawn", 0, fc);
        }
    }

    if (sc) {
        initSquare(7, 0, w_rook, "rook", 1, sc);
        initSquare(7, 1, w_knight, "knight", 1, sc);
        initSquare(7, 2, w_bishop, "bishop", 1, sc);
        initSquare(7, 3, w_queen, "queen", 1, sc);
        initSquare(7, 4, w_king, "king", 1, sc);
        initSquare(7, 5, w_bishop, "bishop", 1, sc);
        initSquare(7, 6, w_knight, "knight", 1, sc);
        initSquare(7, 7, w_rook, "rook", 1, sc);

        for (j = 0; j < 8; j++) {
            initSquare(6, j, w_pawn, "pawn", 1, sc);
        }
    }
    else {
        initSquare(7, 0, b_rook, "rook", 1, sc);
        initSquare(7, 1, b_knight, "knight", 1, sc);
        initSquare(7, 2, b_bishop, "bishop", 1, sc);
        initSquare(7, 3, b_queen, "queen", 1, sc);
        initSquare(7, 4, b_king, "king", 1, sc);
        initSquare(7, 5, b_bishop, "bishop", 1, sc);
        initSquare(7, 6, b_knight, "knight", 1, sc);
        initSquare(7, 7, b_rook, "rook", 1, sc);

        for (j = 0; j < 8; j++) {
            initSquare(6, j, b_pawn, "pawn", 1, sc);
        }
    }

    while (window.isOpen()) {

        window.clear();

        int borw = 0;

        // draw entire board
        rectangle.setPosition(board[0][0].x, board[0][0].y);

        window.draw(rectangle);

        for (j = 1; j < 8; j++) {
            rectangle.setPosition(board[0][j].x, board[0][j].y);

            if (borw) rectangle.setFillColor(sf::Color(118, 150, 86));
            else rectangle.setFillColor(sf::Color(238, 238, 210));

            borw = !borw;

            window.draw(rectangle);
        }

        for (i = 1; i < 8; i++) {
            rectangle.setPosition(board[i][0].x, board[i][0].y);

            window.draw(rectangle);
            for (j = 1; j < 8; j++) {
                rectangle.setPosition(board[i][j].x, board[i][j].y);

                if (borw) rectangle.setFillColor(sf::Color(118, 150, 86));
                else rectangle.setFillColor(sf::Color(238, 238, 210));

                borw = !borw;

                window.draw(rectangle);
            }
        }

        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++) {
                board[i][j].sprite.setPosition(
                        board[i][j].sx,
                        board[i][j].sy
                );
            }

        for (i = 0; i < 8; i++)
            for (j = 0; j < 8; j++) {
                if (board[i][j].hasPiece) {
                    window.draw(board[i][j].sprite);
                }
            }

        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::MouseButtonPressed:
                    repaint();
                    mousePressed = 1;
                    grabPiece(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
                    break;
                case sf::Event::MouseButtonReleased:
                    dropPiece(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
                    mousePressed = 0;
                    break;
                case sf::Event::MouseMoved:
                    movePiece(sf::Mouse::getPosition(window).x, sf::Mouse::getPosition(window).y);
                    break;
                case sf::Event::Closed:
                    window.close();
            }
        }

        window.display();
    }

    return 0;
}