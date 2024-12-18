#ifndef Othello_hpp
#define Othello_hpp
#include <vector>
using namespace std;

class Othello {
    private:
        vector<vector<int>> outBoard;
        vector<vector<int>> board;
        int Colorturn = 1;
        const int dx[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
        const int dy[8] = { 1, -1, 0, 0, 1, -1, -1, 1 };
        vector<pair<int, int>> putPos = {};
        vector<pair<int, int>> canFlipStones = {};
        bool finish = false;

        bool canFlipDirection(const vector<vector<int>>& inBoard, const int& x, const int& y, const int& dirX, const int& dirY) {
            int nx = x + dirX, ny = y + dirY;
            bool foundMyColorOpponent = false;

            while (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) {
                if (inBoard[nx][ny] == 0) {
                    return false; // 空きマスまたは駒が存在しない場合
                }
                if (inBoard[nx][ny] == Colorturn) {
                    return foundMyColorOpponent; // 自分の駒が見つかった
                }
                foundMyColorOpponent = true; // 相手の駒を発見
                nx += dirX;
                ny += dirY;
            }
            return false;
        }

        void canFlipArea() {
            canFlipStones = {};
            for(int x = 0; x < 8; x++){
                for(int y = 0; y < 8; y++){
                    for(int i = 0; i < 8; i++){
                        if(board[x][y] == 0){
                            if(canFlipDirection(board,x, y, dx[i], dy[i])){
                                canFlipStones.push_back(make_pair(x, y));
                                outBoard[x][y] = Colorturn;
                                break;
                            }
                        }
                    }
                }
            }
        }
        
        bool isPut(const vector<vector<int>>& inBoard) {
            putPos = {};
            for (pair<int, int> p:canFlipStones) {
                if(inBoard[p.first][p.second] == Colorturn){
                    putPos.push_back(p);
                }
            }
            return putPos.size() == 1;
        }

        void putAndFlip() {
            board[putPos[0].first][putPos[0].second] = Colorturn;
            int nx,ny;
            for (int i = 0; i < 8; i++) {
                nx = putPos[0].first + dx[i], ny = putPos[0].second + dy[i];
                for (int j = 0; (nx >= 0 && nx < 8 && ny >= 0 && ny < 8) && board[nx][ny] == (3 - Colorturn) && j < 7; j++) {
                    board[nx][ny] = Colorturn;
                    nx += dx[i];
                    ny += dy[i];
                }
            }
        }

        void diff(const vector<vector<int>>& inBoard) {
            vector<vector<int>> diffBoard(8,vector<int>(8,0));
            int diffCount = 0;
            for (int i = 0; i < 8; i++) for (int j = 0; j < 8; j++) {
                if (board[i][j] != inBoard[i][j]) {
                    diffBoard[i][j] = 3;
                    diffCount++;
                }
            }
            if (diffCount) {
                outBoard = diffBoard;
            } else {
                changeColor();
            }
        }

    public:
        Othello(){}

        void changeColor(){
            Colorturn = 3 - Colorturn;
            putPos = {};
            canFlipArea();
        }

        vector<vector<int>> main(const vector<vector<int>>& inBoard, const bool& force = 0) {
            outBoard = vector<vector<int>>(8, vector<int>(8, 0));
            if (!board.size()) {
                board = inBoard;
            }
            canFlipArea();
            if (!putPos.size() || force) {
                if (isPut(inBoard)) {
                    putAndFlip();
                    diff(inBoard);
                }
            } else {
                diff(inBoard);
            }
            return outBoard;
        }
};

#endif // Othello_hpp
