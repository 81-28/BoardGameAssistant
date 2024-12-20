#ifndef Othello_hpp
#define Othello_hpp
#include <vector>
using namespace std;

class Othello {
    private:
        vector<vector<int>> outBoard;
        vector<vector<int>> board;
        int turn;
        const int dx[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
        const int dy[8] = { 1, -1, 0, 0, 1, -1, -1, 1 };
        vector<pair<int, int>> putPos;
        vector<pair<int, int>> canFlipStones;
        int diffCount;
        bool ok;

        // 指定された方向に石をひっくり返せるかどうかを確認する関数
        bool canFlipDirection(const vector<vector<int>>& inBoard, const int& x, const int& y, const int& dirX, const int& dirY) {
            int nx = x + dirX, ny = y + dirY;
            bool foundMyColorOpponent = false;

            while (0 <= nx && nx < 8 && 0 <= ny && ny < 8) {
                if (inBoard[nx][ny] == 0) {
                    return false; // 空きマスまたは駒が存在しない場合
                }
                if (inBoard[nx][ny] == turn) {
                    return foundMyColorOpponent; // 自分の駒が見つかった
                }
                foundMyColorOpponent = true; // 相手の駒を発見
                nx += dirX;
                ny += dirY;
            }
            return false;
        }

        // ひっくり返せる石の位置を返し、リストに追加する
        vector<vector<int>> canFlipArea() {
            vector<vector<int>> result(8, vector<int>(8, 0));
            canFlipStones.clear();
            for(int x = 0; x < 8; x++){
                for(int y = 0; y < 8; y++){
                    for(int i = 0; i < 8; i++){
                        if(board[x][y] == 0){
                            if(canFlipDirection(board,x, y, dx[i], dy[i])){
                                canFlipStones.push_back({x, y});
                                result[x][y] = turn;
                                break;
                            }
                        }
                    }
                }
            }
            return result;
        }

        // 石を置けるかどうかを確認する関数
        bool isPut(const vector<vector<int>>& inBoard) {
            putPos.clear();
            for (const pair<int, int>& p:canFlipStones) {
                if(inBoard[p.first][p.second] == turn){
                    putPos.push_back(p);
                }
            }
            return putPos.size() == 1;
        }

        // 石を置いてひっくり返す関数
        void putAndFlip() {
            board[putPos[0].first][putPos[0].second] = turn;
            int nx,ny;
            for (int i = 0; i < 8; i++) {
                if (canFlipDirection(board, putPos[0].first, putPos[0].second, dx[i], dy[i])) {
                    nx = putPos[0].first + dx[i], ny = putPos[0].second + dy[i];
                    for (int j = 0; (0 <= nx && nx < 8 && 0 <= ny && ny < 8) && board[nx][ny] == (3 - turn) && j < 7; j++) {
                        board[nx][ny] = turn;
                        nx += dx[i];
                        ny += dy[i];
                    }
                }
            }
        }

        // 盤面の差分を返す
        vector<vector<int>> diff(const vector<vector<int>>& inBoard) {
            vector<vector<int>> diffBoard(8,vector<int>(8,0));
            diffCount = 0;
            for (int i = 0; i < 8; i++) for (int j = 0; j < 8; j++) {
                if (board[i][j] != inBoard[i][j]) {
                    diffBoard[i][j] = board[i][j] + 3;
                    diffCount++;
                }
            }
            return diffBoard;
        }

        // ターンを変更する
        void changeColor() {
            for (int i = 0; i < 3; i++) {
                turn = 3 - turn;
                outBoard = canFlipArea();
                if(!canFlipStones.empty()) return;
            }
            finish = true;
        }

    public:
        Othello() : turn(1), diffCount(0), ok(true), finish(false) {}
        bool finish;

        // メインの処理を行う関数
        vector<vector<int>> main(const vector<vector<int>>& inBoard, const bool& force = false) {
            outBoard = vector<vector<int>>(8, vector<int>(8, 0));
            if(force) {
                finish = false;
                board = inBoard;
                changeColor();
                // outBoard = canFlipArea();
                return outBoard;
            }

            if (board.empty()) {
                board = inBoard;
            }
            if (ok) {
                if (isPut(inBoard)) {
                    putAndFlip();
                    ok = false;
                }
            } else {
                if(diffCount == 0) {
                    changeColor();
                    ok = true;
                }
            }

            if (ok) {
                outBoard = canFlipArea();
            } else {
                outBoard = diff(inBoard);
            }
            return outBoard;
        }
};

#endif // Othello_hpp
