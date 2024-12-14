// ConsoleApplication_test.cpp : このファイルには 'main' 関数が含まれています。プログラム実行の開始と終了がそこで行われます。
//

#include <opencv2/opencv.hpp>
#include <iostream>


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