#include <opencv2/opencv.hpp>
#include <iostream>

void detectAndOverlayOthelloBoard(cv::Mat& frame) {
    cv::Mat gray, edges;
    // グレースケール化とエッジ検出
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::GaussianBlur(gray, gray, cv::Size(5, 5), 0);
    cv::Canny(gray, edges, 50, 150);

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
        srcPoints.push_back(cv::Point2f(point.x, point.y));
    }
    dstPoints = { {0, 0}, {400, 0}, {400, 400}, {0, 400} };

    cv::Mat transformMatrix = cv::getPerspectiveTransform(srcPoints, dstPoints);
    cv::Mat warpedBoard;
    cv::warpPerspective(frame, warpedBoard, transformMatrix, cv::Size(400, 400));

    // 石の色を解析して元画像に重ねる
    int rows = 8, cols = 8;
    int cellWidth = warpedBoard.cols / cols;
    int cellHeight = warpedBoard.rows / rows;

    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            cv::Rect cell(j * cellWidth, i * cellHeight, cellWidth, cellHeight);
            cv::Mat cellROI = warpedBoard(cell);

            // 平均色で石の判定
            cv::Scalar avgColor = cv::mean(cellROI);
            cv::Point2f topLeft(j * cellWidth, i * cellHeight);
            cv::Point2f topLeftWarped;
            cv::perspectiveTransform(std::vector<cv::Point2f>{topLeft}, std::vector<cv::Point2f>{topLeftWarped}, transformMatrix.inv());

            cv::Point center(static_cast<int>(topLeftWarped.x + cellWidth / 2), static_cast<int>(topLeftWarped.y + cellHeight / 2));

            if (avgColor[0] < 100) {
                cv::circle(frame, center, cellWidth / 4, cv::Scalar(0, 0, 0), -1); // 黒石
            }
            else if (avgColor[0] > 150) {
                cv::circle(frame, center, cellWidth / 4, cv::Scalar(255, 255, 255), -1); // 白石
            }
        }
    }

    // 枠線を元の画像に描画
    for (size_t i = 0; i < boardContour.size(); ++i) {
        cv::line(frame, boardContour[i], boardContour[(i + 1) % boardContour.size()], cv::Scalar(0, 255, 0), 2);
    }
}

int main() {
    cv::VideoCapture cap(1);
    if (!cap.isOpened()) {
        std::cerr << "カメラが開けませんでした！" << std::endl;
        return -1;
    }

    while (true) {
        cv::Mat frame;
        cap >> frame;
        if (frame.empty()) break;

        detectAndOverlayOthelloBoard(frame);

        cv::imshow("Othello Board Detection", frame);
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