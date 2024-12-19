#include <opencv2/opencv.hpp>
#include "Othello.hpp"

// 画像の圧縮倍率
const double mag = 5;
const double lenMag = 4.2;
const double padding = 2;
// 色の範囲(HSV)
const cv::Scalar lowerGreen(35, 60, 100), upperGreen(90, 255, 255);
const cv::Scalar lowerBlack(0, 0, 0), upperBlack(255, 191, 191);
const cv::Scalar lowerWhite(0, 0, 192), upperWhite(255, 95, 255);
// 盤面の輪郭の座標
std::vector<cv::Point> boardContour;
// 切り取った正方形の一辺
const int len = 160;
// 盤面情報 0:無,1:黒,2:白,3:無にすべき,4:黒にすべき,5:白にすべき
std::vector<std::vector<int>> dusplayBoard(8, std::vector<int>(8, 0));
// オセロの情報,処理を行うクラス
Othello othello;


// 盤面の位置の検出を行う関数
void detectBoard(cv::Mat& frame) {
    cv::Mat comp, blur, hsv, mask, edges;

    // 画像を縮小し、平滑化
    cv::resize(frame, comp, cv::Size(), 1 / mag, 1 / mag);
    cv::GaussianBlur(comp, blur, cv::Size(3, 3), 0.0);
    // HSVに変換
    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);
    // 緑色の検出
    cv::inRange(hsv, lowerGreen, upperGreen, mask);
    // cv::imshow("mask", mask);
    // エッジ検出
    cv::Canny(mask, edges, 50, 150);
    cv::imshow("edge", edges);

    // 四角形を検出
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Point> newBoardContour;
    for (const auto& contour : contours) {
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 0.01 * cv::arcLength(contour, true), true);
        if (approx.size() == 4) {
            double area = cv::contourArea(approx);
            if (area > 1000) { // 十分大きな四角形だけを対象
                newBoardContour = approx;
                break;
            }
        }
    }
    if (newBoardContour.empty()) {
        std::cerr << "オセロ盤が検出されませんでした！" << std::endl;
        return;
    } else {
        boardContour = newBoardContour;
        // boardContourの順序を、左上、右上、右下、左下にする
        // boardContourの中で左上の点を探す（最小のx + yの値）
        int minIndex = 0;
        for (int i = 1; i < boardContour.size(); ++i) {
            if (boardContour[i].x + boardContour[i].y < boardContour[minIndex].x + boardContour[minIndex].y) {
                minIndex = i;
            }
        }
        // 左上の点をboardContour[0]にするよう配列を回転
        std::rotate(boardContour.begin(), boardContour.begin() + minIndex, boardContour.end());
        std::swap(boardContour[1], boardContour[3]);
        return;
    }
}

// 盤面の解析を行う関数
cv::Mat analyzeOthelloBoard(cv::Mat& frame, cv::Mat& boardImg, const bool& force = false) {
    cv::Mat result = boardImg;
    if (boardContour.empty()) {
        //std::cerr << "オセロ盤の位置が不明です！" << std::endl;
        cv::resize(result, result, cv::Size(), lenMag, lenMag);
        return result;
    }

    cv::Mat hsv;
    // 盤面を正面から見たように変換
    std::vector<cv::Point2f> srcPoints, dstPoints;
    for (const auto& point : boardContour) {
        srcPoints.push_back(cv::Point2f(point.x * mag, point.y * mag));
    }
    dstPoints = { {0, 0}, {len, 0}, {len, len}, {0, len} };

    // 透視変換行列を計算
    cv::Mat transformMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::Mat warpedBoard;
    cv::warpPerspective(frame, warpedBoard, transformMatrix, cv::Size(len, len));

    // HSVに変換
    cv::cvtColor(warpedBoard, hsv, cv::COLOR_BGR2HSV);
    // 石の色の範囲だけを抽出
    cv::Mat black, white;
    cv::inRange(hsv, lowerBlack, upperBlack, black);
    cv::inRange(hsv, lowerWhite, upperWhite, white);
    cv::imshow("B", black);
    cv::imshow("W", white);


    // 距離変換を適用して石の中心を検出
    cv::Mat dist;
    // 色ごとに正規化し、重ねる
    cv::Mat distB, distW;
    cv::distanceTransform(black, distB, cv::DIST_L2, 5);
    cv::normalize(distB, distB, 0, 1.0, cv::NORM_MINMAX);
    cv::distanceTransform(white, distW, cv::DIST_L2, 5);
    cv::normalize(distW, distW, 0, 1.0, cv::NORM_MINMAX);
    cv::bitwise_or(distB, distW, dist);
    // cv::imshow("dist", dist);

    // 距離値が極大となる点を検出
    cv::Mat peaks;
    cv::threshold(dist, peaks, 0.75, 1.0, cv::THRESH_BINARY);
    cv::imshow("peak", peaks);
    peaks.convertTo(peaks, CV_8U);

    std::vector<std::vector<cv::Point>> peakContours;
    cv::findContours(peaks, peakContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    int blackStones = 0, whiteStones = 0;
    std::vector<std::vector<int>> newBoard(8, std::vector<int>(8, 0));
    for (const auto& contour : peakContours) {
        // 各極大点の重心を計算
        cv::Moments m = cv::moments(contour);
        if (m.m00 > 0) {
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);

            // 重心周辺の色を取得、判定
            cv::Rect cellPos(cx - len / 32, cy - len / 32, len / 16, len / 16);
            // cellPosが画像範囲内に収まっているか確認
            if (0 <= cellPos.x && cellPos.x + cellPos.width <= warpedBoard.cols && 0 <= cellPos.y && cellPos.y + cellPos.height <= warpedBoard.rows) {
                cv::Mat cell = warpedBoard(cellPos);
                cv::Scalar stoneColor = cv::mean(cell);
                if (stoneColor[0] > 150) {
                    stoneColor = cv::Scalar(0, 0, 255);
                    newBoard[cx / (len / 8)][cy / (len / 8)] = 2;
                    whiteStones++;
                }
                else {
                    stoneColor = cv::Scalar(255, 0, 0);
                    newBoard[cx / (len / 8)][cy / (len / 8)] = 1;
                    blackStones++;
                }
                cv::circle(warpedBoard, cv::Point(cx, cy), 1, stoneColor , -1);
            } else {
                std::cerr << "cellPos is out of bounds: " << cellPos << std::endl;
            }
        }
    }

    // オセロの処理
    dusplayBoard = othello.main(newBoard,force);
    // 終了判定
    if (othello.finish) {
        string text = "Black: " + to_string(blackStones) + " White: " + to_string(whiteStones);
        cv::putText(result, text, cv::Point(10, len/3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
        if (blackStones == whiteStones) {
            text = " Draw";
        } else if (blackStones > whiteStones) {
            text = " Black Won";
        } else {
            text = " White Won";
        }
        cv::putText(result, text, cv::Point(10, len*2/3), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1, cv::LINE_AA);
        cv::resize(result, result, cv::Size(), lenMag, lenMag);
        return result;
    }

    // 枠線を元の画像に描画
    for (size_t i = 0; i < boardContour.size(); ++i) {
        cv::line(frame, boardContour[i] * mag, boardContour[(i + 1) % boardContour.size()] * mag, cv::Scalar(0, 255, 0), 2);
    }
    // アシストを描画
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (dusplayBoard[i][j] == 1) cv::rectangle(result, cv::Point(i * len / 8 + padding, j * len / 8 + padding), cv::Point((i + 1) * len / 8 - padding, (j + 1) * len / 8 - padding), cv::Scalar(255, 0, 0), 1);
            if (dusplayBoard[i][j] == 2) cv::rectangle(result, cv::Point(i * len / 8 + padding, j * len / 8 + padding), cv::Point((i + 1) * len / 8 - padding, (j + 1) * len / 8 - padding), cv::Scalar(0, 0, 255), 1);
            if (dusplayBoard[i][j] == 3) cv::circle(result, cv::Point(i * len / 8 + len / 16, j * len / 8 + len / 16), len / 32, cv::Scalar(0, 255, 0), -1);
            if (dusplayBoard[i][j] == 4) cv::circle(result, cv::Point(i * len / 8 + len / 16, j * len / 8 + len / 16), len / 32, cv::Scalar(127, 127, 127), -1);
            if (dusplayBoard[i][j] == 5) cv::circle(result, cv::Point(i * len / 8 + len / 16, j * len / 8 + len / 16), len / 32, cv::Scalar(255, 255, 255), -1);
            // if (dusplayBoard[i][j] == 3) cv::rectangle(result, cv::Point(i * len / 8 + padding, j * len / 8 + padding), cv::Point((i + 1) * len / 8 - padding, (j + 1) * len / 8 - padding), cv::Scalar(0, 255, 0), 1);
            // if (dusplayBoard[i][j] == 4) cv::rectangle(result, cv::Point(i * len / 8 + padding, j * len / 8 + padding), cv::Point((i + 1) * len / 8 - padding, (j + 1) * len / 8 - padding), cv::Scalar(127, 127, 127), 1);
            // if (dusplayBoard[i][j] == 5) cv::rectangle(result, cv::Point(i * len / 8 + padding, j * len / 8 + padding), cv::Point((i + 1) * len / 8 - padding, (j + 1) * len / 8 - padding), cv::Scalar(255, 255, 255), 1);
            // if (newBoard[i][j] == 1) cv::circle(warpedBoard, cv::Point(i * len / 8 + len / 16, j * len / 8 + len / 16), len / 32, cv::Scalar(255, 0, 0), -1);
            // if (newBoard[i][j] == 2) cv::circle(warpedBoard, cv::Point(i * len / 8 + len / 16, j * len / 8 + len / 16), len / 32, cv::Scalar(0, 0, 255), -1);
        }
    }
    cv::resize(warpedBoard, warpedBoard, cv::Size(), lenMag, lenMag);
    cv::imshow("square", warpedBoard);
    cv::resize(result, result, cv::Size(), lenMag, lenMag);
    return result;
}

int main() {
    cv::VideoCapture cap(0);
    if (!cap.isOpened()) {
        std::cerr << "カメラが開けませんでした！" << std::endl;
        return -1;
    }

    std::cerr << "\nPress Space to Detect Board\n\n";

    int imgNum = 0;

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;
        //frame = cv::imread("OthelloBoard_5.jpg");

        // キー入力を待つ
        int key = cv::waitKey(10);
        // ESCキーで終了
        if (key == 27) break;
        // Sキーで保存
        if (key == 's') {
            cv::imwrite("OthelloBoard_" + std::to_string(imgNum) + ".jpg", frame);
            std::cerr << "\nimg saved!\n\n";
            imgNum++;
        }
        // SPACEキーで盤面認識
        if (key == 32) {
            detectBoard(frame);
        }

        cv::Mat boardImg(len, len, CV_8UC3);
        boardImg = cv::Scalar(0, 0, 0);
        cv::rectangle(boardImg, cv::Point(0, 0), cv::Point(len, len), cv::Scalar(0, 15, 0), 2, cv::LINE_4);

        // 盤面の解析(Fキーで強制更新)
        boardImg = analyzeOthelloBoard(frame, boardImg, key == 'f');
        cv::imshow("Othello Board", frame);
        cv::imshow("board", boardImg);
    }

    return 0;
}



#include <opencv2/core/version.hpp>
#ifdef _DEBUG
#define LIBEXT CVAUX_STR(CV_VERSION_MAJOR) CVAUX_STR(CV_VERSION_MINOR) CVAUX_STR(CV_VERSION_REVISION) CV_VERSION_STATUS "d.lib"
#else
#define LIBEXT CVAUX_STR(CV_VERSION_MAJOR) CVAUX_STR(CV_VERSION_MINOR) CVAUX_STR(CV_VERSION_REVISION) CV_VERSION_STATUS ".lib"
#endif
#ifdef HAVE_OPENCV_WORLD
#pragma comment(lib, "opencv_world"       LIBEXT)
#else
#pragma comment(lib, "opencv_core"        LIBEXT)
#pragma comment(lib, "opencv_highgui"     LIBEXT)
#pragma comment(lib, "opencv_imgproc"     LIBEXT)
#pragma comment(lib, "opencv_video"       LIBEXT)
#pragma comment(lib, "opencv_videoio"     LIBEXT)
#pragma comment(lib, "opencv_features2d"  LIBEXT)
#pragma comment(lib, "opencv_objdetect"   LIBEXT)
#pragma comment(lib, "opencv_calib3d"     LIBEXT)
#pragma comment(lib, "opencv_imgcodecs"   LIBEXT)
#ifdef HAVE_OPENCV_XFEATURES2D
#pragma comment(lib, "opencv_xfeatures2d" LIBEXT)
#endif
#endif