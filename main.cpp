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

int main() {
    VideoCapture webcam;

    if(webcam.open(0)) {
        cout << "Great ! It works !" << endl;
    } else {
        cout << "Or not..." << endl;
        return -1;
    }

    namedWindow("a", WINDOW_AUTOSIZE);
    bool carryOn(true);
    bool mustConvert(false);

    while(carryOn) {
        Mat frame;
        webcam.read(frame);

        switch((char)waitKey(1)) {
            case 27:
                cout << "Exit program. Good bye !" << endl;
                carryOn = false;
                break;

            case 97:
                mustConvert = !mustConvert;
                break;

            default: break;
        }

        if(mustConvert) {
            convertImageToGrayScale(frame);
        }

        imshow("a", frame);
}

    return 0;
}
