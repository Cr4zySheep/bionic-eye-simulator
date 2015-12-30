#include <iostream>
#include <string>

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
    cout << "Image converted to gray scale" << endl;
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
    bool mustConvert(false);

    while(carryOn) {
        //1 - Get picture
        webcam.read(frame);
        imshow(paramsName, frame);

        switch((char)waitKey(1)) {
            case 27:
                carryOn = false;
                break;

            case 97:
                mustConvert = !mustConvert;
                break;

            default: break;
        }

        //2 - Convert to grayscale
        if(mustConvert) {
            convertImageToGrayScale(frame);
        }

        uint8_t* pixelPtr = (uint8_t*)frame.data;
        int cn = frame.channels();
        Scalar_<uint8_t> bgrPixel;

        bgrPixel.val[0] = pixelPtr[0];

        //cout << frame.type() << endl;
        //cout << (int)frame.at<unsigned int8_t>(Point(0, 0)) << " " << cvGet2D(frame., 0, 0) << endl;

        //3 - Reduce
        reduceImage(frame, angle, (double)electrodes_height / (double)electrodes_width);


        imshow(windowName, frame);
}

    return 0;
}
