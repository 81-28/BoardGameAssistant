#ifndef   Othello_hpp
#define   Othello_hpp
#include <bits/stdc++.h>
using namespace std;

class Othello {
    // 0: White, 1: Black
public:
    Othello() : exist(8, vector<bool>(8, false)), board(8, vector<int>(8, -1)) {
        // 初期状態のセット
        exist[3][3] = exist[4][4] = true;
        exist[3][4] = exist[4][3] = true;
        board[3][3] = 0; // White
        board[4][4] = 0; // White
        board[3][4] = 1; // Black
        board[4][3] = 1; // Black
    }

    // ボードの表示
    void displayBoard() {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (!exist[i][j]) cout << "- ";
                else if (board[i][j] == 0) cout << "W ";
                else cout << "B ";
            }
            cout << endl;
        }
    }

    // 合法手を探す
    vector<pair<int, int>> findValidMoves(int color) {
        vector<pair<int, int>> validMoves;

        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                if (board[x][y] == -1 && canFlip(x, y, color)) {
                    validMoves.push_back({x, y});
                }
            }
        }
        return validMoves;
    }

    // 駒を置く
    bool put(int x, int y, int color) {
        if (!canFlip(x, y, color)) return false;

        exist[x][y] = true;
        board[x][y] = color;

        // 駒を反転する
        for (int i = 0; i < 8; i++) {
            flipDirection(x, y, color, dx[i], dy[i]);
        }

        return true;
    }

private:
    vector<vector<bool>> exist; // 駒が存在するか
    vector<vector<int>> board;  // 駒の色（0: White, 1: Black）

    // 8方向のベクトル
    int dx[8] = {0, 0, 1, -1, 1, -1, 1, -1};
    int dy[8] = {1, -1, 0, 0, 1, -1, -1, 1};

    // 駒を挟めるか判定
    bool canFlip(int x, int y, int color) {
        for (int i = 0; i < 8; i++) {
            if (canFlipDirection(x, y, color, dx[i], dy[i])) {
                return true;
            }
        }
        return false;
    }

    // 特定の方向で駒を挟めるか判定
    bool canFlipDirection(int x, int y, int color, int dirX, int dirY) {
        int nx = x + dirX, ny = y + dirY;
        bool foundOpponent = false;

        while (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
            if (board[nx][ny] == -1 || !exist[nx][ny]) {
                return false; // 空きマスまたは駒が存在しない場合
            }
            if (board[nx][ny] == color) {
                return foundOpponent; // 自分の駒が見つかった
            }
            foundOpponent = true; // 相手の駒を発見
            nx += dirX;
            ny += dirY;
        }
        return false;
    }

    // 駒を反転する
    void flipDirection(int x, int y, int color, int dirX, int dirY) {
        int nx = x + dirX, ny = y + dirY;

        if (!canFlipDirection(x, y, color, dirX, dirY)) return;

        while (board[nx][ny] != color) {
            board[nx][ny] = color;
            nx += dirX;
            ny += dirY;
        }
    }
};

int main() {
    Othello othello;
    othello.displayBoard();

    while (true) {
        int x, y, color;
        cout << "Enter x y color (0: White, 1: Black): ";
        cin >> x >> y >> color;

        if (!othello.put(x, y, color)) {
            cout << "Can't put there. Try again." << endl;
        } else {
            cout << "Move accepted!" << endl;
        }

        cout << "Current Board:" << endl;
        othello.displayBoard();

        vector<pair<int, int>> validMoves = othello.findValidMoves(1 - color);
        cout << "Valid moves for " << (1 - color == 0 ? "White" : "Black") << ":" << endl;
        for (auto move : validMoves) {
            cout << "(" << move.first << ", " << move.second << ")" << endl;
        }

        
    }

    return 0;
}

#endif // Othello_hpp
