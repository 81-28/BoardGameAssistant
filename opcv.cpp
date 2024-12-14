#include <opencv2/opencv.hpp>
#include <iostream>

// 画像の圧縮倍率
const double mag = 5;
// 緑とする範囲(HSV)
const cv::Scalar lowerGreen(35, 60, 100), upperGreen(90, 255, 255);
// 切り取った正方形の一辺
const int len = 80;

// 緑色か確かめる関数
bool isGreen(const cv::Scalar& color) {
    cv::Mat colorMat(1, 1, CV_8UC3, cv::Scalar(color[0], color[1], color[2]));
    cv::Mat hsv;
    cv::cvtColor(colorMat, hsv, cv::COLOR_BGR2HSV);

    cv::Vec3b hsvColor = hsv.at<cv::Vec3b>(0, 0);
    int h = hsvColor[0], s = hsvColor[1], v = hsvColor[2];

    // 緑のHSV範囲を指定
    return (lowerGreen[0] <= h && h <= upperGreen[0]) && (lowerGreen[1] <= s && s <= upperGreen[1]) && (lowerGreen[2] <= v && v <= upperGreen[2]);
}

// 盤面の検出と解析を行う関数
void detectAndAnalyzeOthelloBoard(cv::Mat& frame) {
    cv::Mat comp, blur, hsv, mask, edges;


    // 画像を縮小し、平滑化
    cv::resize(frame, comp, cv::Size(), 1/mag, 1/mag);
    cv::GaussianBlur(comp, blur, cv::Size(15, 15), 0.0);
    // HSVに変換
    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);
    // 緑色の検出
    cv::inRange(hsv, lowerGreen, upperGreen, mask);
    cv::imshow("mask", mask);
    // エッジ検出
    cv::Canny(mask, edges, 50, 150);
    cv::imshow("edge", edges);

    // 四角形を検出
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(edges, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    std::vector<cv::Point> boardContour;
    for (const auto& contour : contours) {
        std::vector<cv::Point> approx;
        cv::approxPolyDP(contour, approx, 0.02 * cv::arcLength(contour, true), true);
        if (approx.size() == 4) {
            double area = cv::contourArea(approx);
            if (area > 1000) { // 十分大きな四角形だけを対象
                boardContour = approx;
                break;
            }
        }
    }
    if (boardContour.empty()) {
        std::cerr << "オセロ盤が検出されませんでした！" << std::endl;
        return;
    }

    // 盤面を正面から見たように変換
    std::vector<cv::Point2f> srcPoints, dstPoints;
    for (const auto& point : boardContour) {
        srcPoints.push_back(cv::Point2f(point.x * mag, point.y * mag));
    }
    dstPoints = { {0, 0}, {len, 0}, {len, len}, {0, len} };
    cv::Mat transformMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::Mat warpedBoard;
    cv::warpPerspective(frame, warpedBoard, transformMatrix, cv::Size(len, len));

    // 盤面を解析したい
    // 画像を平滑化
    cv::GaussianBlur(warpedBoard, blur, cv::Size(7, 7), 0.0);
    cv::imshow("blur2", blur);
    // HSVに変換
    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);
    // 緑色の検出
    cv::inRange(hsv, lowerGreen, upperGreen, mask);
    cv::imshow("mask2", mask);
    //// エッジ検出
    // cv::Canny(mask, edges, 50, 150);
    // cv::imshow("edge2", edges);

    cv::Mat gray;
    cv::bitwise_not(mask, gray);

    // 距離変換を適用して石の中心を検出
    cv::Mat dist;
    cv::distanceTransform(gray, dist, cv::DIST_L2, 5);
    cv::normalize(dist, dist, 0, 1.0, cv::NORM_MINMAX);
    cv::imshow("dist", dist);

    // 距離値が極大となる点を検出
    cv::Mat peaks;
    cv::threshold(dist, peaks, 0.6, 1.0, cv::THRESH_BINARY);
    cv::imshow("peak", peaks);
    peaks.convertTo(peaks, CV_8U);

    std::vector<std::vector<cv::Point>> peakContours;
    cv::findContours(peaks, peakContours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    cv::Mat adp;
    cv::cvtColor(warpedBoard, gray, cv::COLOR_BGR2GRAY);
    cv::adaptiveThreshold(gray, adp, 255, cv::ADAPTIVE_THRESH_GAUSSIAN_C, cv::THRESH_BINARY, 191, -20);
    cv::imshow("adp", adp);

    for (const auto& contour : peakContours) {
        // 各極大点の重心を計算
        cv::Moments m = cv::moments(contour);
        if (m.m00 > 0) {
            int cx = static_cast<int>(m.m10 / m.m00);
            int cy = static_cast<int>(m.m01 / m.m00);

            // 重心を円として描画
            cv::Rect cellPos(cx - len / 32, cy - len / 32, len / 16, len / 16);
            // cellPosが画像範囲内に収まっているか確認
            if (cellPos.x >= 0 && cellPos.y >= 0 && cellPos.x + cellPos.width <= warpedBoard.cols && cellPos.y + cellPos.height <= warpedBoard.rows) {
                cv::Mat cell = warpedBoard(cellPos);
                cv::Scalar color = cv::mean(cell);
                cv::circle(warpedBoard, cv::Point(cx, cy), 1, (color[0] > 150) ? cv::Scalar(0, 0, 255) : cv::Scalar(255, 0, 0), -1);
            } else {
                std::cerr << "cellPos is out of bounds: " << cellPos << std::endl;
            }
        }
    }



    // 枠線を元の画像に描画
    for (size_t i = 0; i < boardContour.size(); ++i) {
        cv::line(frame, boardContour[i] * mag, boardContour[(i + 1) % boardContour.size()] * mag, cv::Scalar(0, 255, 0), 2);
    }
    // 切り取ったオセロ版を描画
    cv::resize(warpedBoard, warpedBoard, cv::Size(), 5, 5);
    cv::imshow("square", warpedBoard);
}

int main() {
     cv::VideoCapture cap(1);
     if (!cap.isOpened()) {
         std::cerr << "カメラが開けませんでした！" << std::endl;
         return -1;
     }

    // cv::Mat img(600, 600, CV_8UC3);
    // img = cv::Scalar(255, 255, 255);
    // cv::rectangle(img, cv::Point(0, 0), cv::Point(600, 600), cv::Scalar(0, 0, 255), 8, cv::LINE_4);

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;
        detectAndAnalyzeOthelloBoard(frame);
        cv::imshow("Othello Board Detection", frame);

        // cv::imshow("aa", img);

        if (cv::waitKey(30) == 27) break; // ESCキーで終了
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