#ifndef Othello_hpp
#define Othello_hpp
#include <vector>
using namespace std;

class Othello {
    private:
        int Colorturn = 1;
        vector<vector<int>> board;
        const int dx[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
        const int dy[8] = { 1, -1, 0, 0, 1, -1, -1, 1 };
        pair<int,int> putPos = {-1,-1};
        vector<pair<int, int>> canFlipStones = {};

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

        vector<vector<int>> canFlipArea(const vector<vector<int>>& inBoard) {
            canFlipStones = {};
            int x = 0, y = 0;
            for(int x = 0; x < 8; x++){
                for(int y = 0; y < 8; y++){
                    for(int i = 0; i < 8; i++){
                        if(inBoard[x][y] == 0){
                            if(canFlipDirection(inBoard,x, y, dx[i], dy[i])){
                                canFlipStones.push_back(make_pair(x, y));
                                outBoard[x][y] = Colorturn;
                            }
                        }
                    }
                }
            }
        }
        
        bool isPut(const vector<vector<int>>& inBoard) {
            int count = 0;
            for(int i = 0; i < canFlipStones.size(); i++){
                if(inBoard[canFlipStones[i].first][canFlipStones[i].second] == Colorturn){
                    putPos = canFlipStones[i];
                    count++;
                }
            }
            return count == 1;
        }

        void putAndFlip() {
            board[putPos.first][putPos.second] = Colorturn;
            int nx,ny;
            for (int i = 0; i < 8; i++) {
                nx = putPos.first + dx[i], ny = putPos.second + dy[i];
                for (int j = 0; board[nx][ny] == (3 - Colorturn) && j < 7; j++) {
                    board[nx][ny] = Colorturn;
                    nx += dx[i];
                    ny += dy[i];
                }
            }
        }

        vector<vector<int>> diff(const vector<vector<int>>& inBoard) {
            vector<vector<int>> diffBoard(8,vector<int>(8,0));
            int diffCount = 0;
            for (int i = 0; i < 8; i++) {
                for (int j = 0; j < 8; j++) {
                    if (board[i][j] != inBoard[i][j]) {
                        diffBoard[i][j] = 3;
                        diffCount++;
                    }
                }
            }
            if (diffCount) {
                return diffBoard;
            } else {
                changeColor();
                return canFlipArea(inBoard);
            }
        }

    public:
        Othello() : outBoard(8, vector<int>(8, 0)), board(0) { }
        vector<vector<int>> outBoard;

        void changeColor(){
            Colorturn = 3 - Colorturn;
        }

        vector<vector<int>> main(const vector<vector<int>>& inBoard, const bool& force = 0) {
            outBoard = vector<vector<int>>(8, vector<int>(8, 0));
            if (board.empty()) {
                board = inBoard;
            }
            outBoard = canFlipArea(inBoard);
            if (putPos.first == -1 || force) {
                if (isPut(inBoard)) putAndFlip();
            } else {
                outBoard = diff(inBoard);
            }
            return outBoard;
        }
};

#endif // Othello_hpp
