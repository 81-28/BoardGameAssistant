﻿// ConsoleApplication_test.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <opencv2/opencv.hpp>
#include <iostream>

// 画像の圧縮倍率
const double mag = 5;
// 緑とする範囲(HSV)
const cv::Scalar lowerGreen(35, 60, 60), upperGreen(90, 255, 255);

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
    dstPoints = { {0, 0}, {400, 0}, {400, 400}, {0, 400} };
    cv::Mat transformMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::Mat warpedBoard;
    cv::warpPerspective(frame, warpedBoard, transformMatrix, cv::Size(400, 400));

    // 盤面を解析したい
    // 画像を平滑化
    cv::GaussianBlur(warpedBoard, blur, cv::Size(31, 31), 0.0);
    cv::imshow("blur2", blur);
    // HSVに変換
    cv::cvtColor(blur, hsv, cv::COLOR_BGR2HSV);
    // 緑色の検出
    cv::inRange(hsv, lowerGreen, upperGreen, mask);
    cv::imshow("mask2", mask);
    //// エッジ検出
    cv::Canny(mask, edges, 50, 150);
    cv::imshow("edge2", edges);
    std::vector<cv::Vec3f> circles;
    cv::HoughCircles(edges, circles, cv::HOUGH_GRADIENT, 1, 50, 100, 20, 1, 100);
    for (const cv::Vec3f& circle : circles) {
        cv::Point center(cvRound(circle[0]), cvRound(circle[1]));
        int radius = cvRound(circle[2]);
        cv::circle(warpedBoard, center, radius/10, cv::Scalar(0, 0, 255), 2);
    }

    // 枠線を元の画像に描画
    for (size_t i = 0; i < boardContour.size(); ++i) {
        cv::line(frame, boardContour[i] * mag, boardContour[(i + 1) % boardContour.size()] * mag, cv::Scalar(0, 255, 0), 2);
    }
    // 切り取ったオセロ版を描画
    cv::imshow("square", warpedBoard);
}

int main() {
    // 画像の読み込み
    cv::Mat frame = cv::imread("C:\\Users\\agugu\\aaa.png");
    if (frame.empty()) {
        std::cerr << "画像が見つかりません！" << std::endl;
        return -1;
    }

    // HSV変換
    cv::Mat hsvImage;
    cv::cvtColor(frame, hsvImage, cv::COLOR_BGR2HSV);

    // 範囲1: lower と upper
    cv::Scalar lower1(45, 89, 30);
    cv::Scalar upper1(90, 255, 255);
    cv::Mat green1;
    cv::inRange(hsvImage, lower1, upper1, green1);
        detectAndAnalyzeOthelloBoard(frame);

    // 範囲2: lower と upper
    cv::Scalar lower2(45, 64, 89);
    cv::Scalar upper2(90, 255, 255);
    cv::Mat green2;
    cv::inRange(hsvImage, lower2, upper2, green2);

    // 論理ORを使用してマスクを統合
    cv::Mat green;
    cv::bitwise_or(green1, green2, green);

    // 結果の表示
    cv::imshow("Original Image", frame);
    cv::imshow("Green Mask", green);

    // キー入力待ち
    cv::waitKey(0);

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