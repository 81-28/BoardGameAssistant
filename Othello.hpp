#ifndef   Othello_hpp
#define   Othello_hpp
#include <bits/stdc++.h>
#include <vector>
using namespace std;

class Othello {
    // 8x8のボード
    // 0: 空きマス, 1: 黒, 2: 白(inBoardの場合)
    // 0: 空きマス, 1: 黒, 2: 白 3:おける場所(outBoardの場合)
public:
    Othello() :inBoard(8, vector<int>(8, 0)), outBoard(8, vector<int>(8, 0)) {
        
        
    }
    int Colorturn = 1;
    int dx[8] = { 0, 0, 1, -1, 1, -1, 1, -1 };
    int dy[8] = { 1, -1, 0, 0, 1, -1, -1, 1 };



    // ボードの表示
    void displayoutBoard() {
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (outBoard[i][j] == 0) cout << "- ";
                else if (outBoard[i][j] == 1) cout << "W ";
                else cout << "B ";
            }
            cout << endl;
        }
    }

    // Colorturnの変更
    int changeColor(int Colorturn){
        if(Colorturn == 1){
            Colorturn = 2;
        }else{
            Colorturn = 1;
        }
        return Colorturn;
    }
     
    // 出力行列の初期化 
    vector<vector<int>> initializeoutBoard(){
        for(int x = 0; x < 8; x++){
            for(int y = 0; y < 8; y++){
                outBoard[x][y] = 0;
            }
        }
        return outBoard;

    }


    // 合法手を探す
    // 出力はoutBoard
    vector<vector<int>> canFlipArea(vector<vector<int>> Board) {
        int x = 0, y = 0;
        for(int x = 0; x < 8; x++){
            for(int y = 0; y < 8; y++){
                for(int i = 0; i < 8; i++){
                    if(inBoard[x][y] == 0){
                        if(canFlipDirection(x, y, dx[i], dy[i],Colorturn)){
                            flipDirection(x, y, dx[i], dy[i], Colorturn); 
                            outBoard[x][y] = Colorturn;
                        }
                    }
                }
            }
        }
        
        return Board;
    }


    
    // 駒を反転する
    void flipDirection(int x, int y, int Colorturn, int dirX, int dirY) {
        int nx = x + dirX, ny = y + dirY;

        if (!canFlipDirection(x, y, color, dirX, dirY)) return;

        while (inBoard[nx][ny] != Colorturn) {
            outBoard[nx][ny] = 3;
            nx += dirX;
            ny += dirY;
        }
    }
   
    

    // 特定の方向で駒を挟めるか判定
    bool canFlipDirection(vector<vector<int>> inBoard int x int y,int dirX,int dirY,int Colorturn) {
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
    // 正しい位置に置いているかの判定
    bool canPut(int Colorturn) {
        int temp = Colorturn;
        for(int i = 0; i < 8; i++){
            for (int j = 0; j < 8; j++) {
                if !(outBoard[i][j] == Colorturn && (inBoard[i][j] == Colorturn || inBoard[i][j] == 0)) {
                    return false;
                }
        }
        return true;
    }




    
    //ひっくりかえし切れたかの判定(Colorturnを変える処理も含む)
    bool ReverceAll(vector<vector<int>> inBoard, int x, int y, int Colorturn) {
        
        for(int i = 0; i < 8; i++){
            for(int j = 0; j < 8; j++){
                if(outBoard[i][j] == 3 && inBoard[i][j] != Colorturn){
                    return false;
                }
            }
        }

        changeColor(Colorturn);
        return true;
       
    }



};
}
#endif // Othello_hpp
