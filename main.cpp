#include <iostream>
#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

bool loadImage(Mat& img, string filename) {
    img = imread(filename.c_str(), IMREAD_COLOR);

    if(img.empty()) {
        cout << "Could not open or find image : " << filename << endl;
        return false;
    } else {
        cout << "Opened : " << filename << endl;
        return true;
    }
}

void convertImageToGrayScale(Mat& img) {
    cvtColor(img, img, COLOR_BGR2GRAY);
}
/*
int main(int argc, char** argv)
{
    string imgName("img.png");
    if(argc > 1) {
        imgName = argv[1];
    }

    Mat img;
    if(!loadImage(img, imgName)) {
        return -1;
    }
    convertImageToGrayScale(img);

    namedWindow("Bionic Eye", WINDOW_AUTOSIZE);
    imshow("Bionic Eye", img);

    waitKey(0);
    return 0;
}*/

void reduceImage(Mat& img, int angle, double scale) {
    int w(img.cols * angle / 100),
        h(w * scale),
        x((img.cols - w) / 2),
        y((img.rows - h) / 2);

    //Avoid matrix of 0 x i
    if(w == 0 || h == 0) {
        return;
    }

    if(w + x > img.cols) {
        img = Mat(img, Rect(0, y, img.cols, h));
    } else if(h + y> img.rows) {
        img = Mat(img, Rect(x, 0, w, img.rows));
    } else {
        img = Mat(img, Rect(x, y, w, h));
    }
}

//No more imagination, sorry
//Need the picture in gray scale
void pixeliseImage(Mat& img, int electrodes_w, int electrodes_h) {
    //To avoid errors
    if(img.channels() != 1) {
        cout << "Too more channels. Channel expected 1." << endl;
        return;
    }
    if(electrodes_w == 0) {
        electrodes_w = 1;
    } else if(electrodes_w > img.cols) {
        electrodes_w = img.cols;
    }
    if(electrodes_h == 0) {
        electrodes_h = 1;
    } else if(electrodes_h > img.rows) {
        electrodes_h = img.rows;
    }


    Mat finalImg(electrodes_h, electrodes_w, CV_8UC1);
    int blockW(img.cols / electrodes_w),
        blockH(img.rows / electrodes_h);

    //Need to cut the big picture in several small picture
    for(int x(0); x < electrodes_w; ++x) {
        for(int y(0); y < electrodes_h; ++y) {

            //Now let's do the average of fray of the target part
            Mat target(img, Rect(blockW * x, blockH * y, blockW, blockH));

            int nRows(target.rows),
                nCols(target.cols * target.channels()),
                sum(0);

            if(target.isContinuous()) {
                nCols *= nRows;
                nRows = 1;
            }

            uchar* p(nullptr);
            for(int i(0); i < nRows; ++i) {
                p = target.ptr<uchar>(i);
                for(int j(0); j < nCols; ++j) {
                    sum += p[j];
                }
            }

            int average(sum / (blockH * blockW));

            //Put this color in the target picture !
            finalImg.ptr<uchar>(y)[x] = average;

        }
    }

    img = finalImg;
    //rectangle(img, Point(0, 0), Point(50, 50), Scalar(126), CV_FILLED);
}

int main() {
    VideoCapture webcam;

    if(webcam.open(0)) {
        cout << "Great ! It works !" << endl;
    } else {
        cout << "Or not..." << endl;
        return -1;
    }

    string windowName("Modified picture");
    namedWindow(windowName, WINDOW_AUTOSIZE);
    string paramsName("Initial picture");
    namedWindow(paramsName, WINDOW_AUTOSIZE);
    Mat frame;
    webcam.read(frame);
    int height(frame.size().height);
    int width(frame.size().width);
    int electrodes_width(10); //Number of electrodes
    int electrodes_height(6);
    int angle(100); //Percentage of the width of the initial picture which will be used

    //Add some trackbars
    createTrackbar("electrodes width", paramsName, &electrodes_width, width);
    createTrackbar("electrodes height", paramsName, &electrodes_height, height);
    createTrackbar("angle (%)", paramsName, &angle, 100);

    bool carryOn(true);

    while(carryOn) {
        //1 - Get picture
        webcam.read(frame);
        imshow(paramsName, frame);

        switch((char)waitKey(1)) {
            case 27:
                carryOn = false;
                break;

            default: break;
        }

        //2 - Convert to grayscale
        convertImageToGrayScale(frame);

        //3 - Reduce to get less information
        reduceImage(frame, angle, (double)electrodes_height / (double)electrodes_width);

        //4 - Reduce against in electrodes_heigth * electrodes_width
        pixeliseImage(frame, electrodes_height, electrodes_width);

        //Show image
        imshow(windowName, frame);
    }

    return 0;
}
