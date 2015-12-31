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

            uchar average(sum / (blockH * blockW));

            //Put this color in the target picture !
            finalImg.ptr<uchar>(y)[x] = average;
        }
    }

    img = finalImg;
}

//Reverse the mat send in argument (like if you look in a spoon)
void reverseImage(Mat& img) {
    //Temporary matrix
    Mat finalImg(img.rows, img.cols, CV_8UC1);

    //The top-left corner pixel go the right-bottom corner
    for(int y(0); y < img.rows; ++y) {
        uchar* p = img.ptr<uchar>(y);
        for(int x(0); x < img.cols; ++x) {
            finalImg.ptr<uchar>(img.rows - y - 1)[img.cols - x - 1] = p[x];
        }
    }

    img = finalImg;
}

void extendImage(Mat& img, int zoom) {
    if(zoom <= 0) {
        zoom = 1;
    }

    Mat finalImg(img.rows * zoom, img.cols * zoom, CV_8UC1);

    for(int y(0); y < img.rows; ++y) {
        uchar* p = img.ptr<uchar>(y);
        for(int x(0); x < img.cols; ++x) {
            rectangle(finalImg, Point(x * zoom, y * zoom), Point((x + 1) * zoom, (y + 1) * zoom), Scalar(p[x]), CV_FILLED);
        }
    }

    img = finalImg;
}

void saveImage(string const& filename, Mat& img) {
    imwrite(filename + ".jpg", img);
    cout << "Saved as " + filename << ".jpg\n";
}

void useWebcam() {
    VideoCapture webcam;

    if(webcam.open(0)) {
        cout << "Great ! It works !" << endl;
    } else {
        cout << "Or not..." << endl;
        return;
    }

    string modifiedWindow("Modified picture");
    namedWindow(modifiedWindow, WINDOW_AUTOSIZE);
    string initialWindow("Initial picture");
    namedWindow(initialWindow, WINDOW_AUTOSIZE);
    string reduceWindow("Reduced picture");
    namedWindow(reduceWindow, WINDOW_AUTOSIZE);
    Mat frame;
    webcam.read(frame);
    int height(frame.size().height);
    int width(frame.size().width);
    int electrodes_width(10); //Number of electrodes
    int electrodes_height(6);
    int angle(100); //Percentage of the width of the initial picture which will be used
    int zoom(1); //time to extend the final picture

    //Add some trackbars
    createTrackbar("width", initialWindow, &electrodes_width, width);
    createTrackbar("height", initialWindow, &electrodes_height, height);
    createTrackbar("angle (%)", initialWindow, &angle, 100);
    createTrackbar("zoom", initialWindow, &zoom, 20);

    bool carryOn(true);
    bool mustSave(false);

    while(carryOn) {
        //Input handling
        switch((char)waitKey(1)) {
            case 27:
                carryOn = false;
                break;

            case 115:
                mustSave = true;
                break;

            default: break;
        }

        //1 - Get picture
        webcam.read(frame);
        imshow(initialWindow, frame);
        if(mustSave) {
            saveImage("1_initial", frame);
        }

        //2 - Convert to grayscale
        convertImageToGrayScale(frame);
        if(mustSave) {
            saveImage("2_grayscale", frame);
        }

        //3 - Reduce to get less information
        reduceImage(frame, angle, (double)electrodes_height / (double)electrodes_width);
        imshow(reduceWindow, frame);
        if(mustSave) {
            saveImage("3_reduce", frame);
        }

        //4 - Reduce against in electrodes_heigth * electrodes_width
        pixeliseImage(frame, electrodes_width, electrodes_height);
        if(mustSave) {
            saveImage("4_pixelise", frame);
        }

        //5 - Reverse the picture
        reverseImage(frame);
        if(mustSave) {
            saveImage("5_reverse", frame);
        }

        //Extend the picture because some times, it's to small
        extendImage(frame, zoom);

        //Show image
        imshow(modifiedWindow, frame);
        mustSave = false;
    }

    destroyAllWindows();
}

void useFile() {
    string filename("");
    cout << "Filename : ";
    getline(cin, filename);

    Mat img;

    //Check if errors
    if(!loadImage(img, filename)) {
        return;
    }

    cout << "Load successful !\n\n";

    //Display initial picture
    namedWindow("initial picture", WINDOW_AUTOSIZE);
    imshow("initial picture", img);
    waitKey(0); //Wait before next step
    destroyWindow("initial picture");

    //Display grayscale picture
    namedWindow("grayscale picture", WINDOW_AUTOSIZE);
    convertImageToGrayScale(img);
    imshow("grayscale picture", img);
    waitKey(0); //Wait before next step
    destroyWindow("grayscale picture");

    //Display reduce picture
    string window("before reduce picture"), window2("after reduce picture");
    namedWindow(window, WINDOW_AUTOSIZE);
    int angle(100),
        electrodes_width(10), electrodes_height(6),
        width(img.size().width), height(img.size().height);
    Mat baseImg;
    img.copyTo(baseImg);
    createTrackbar("angle (%)", window, &angle, 100);
    createTrackbar("width", window, &electrodes_width, width);
    createTrackbar("height", window, &electrodes_height, height);
    while(static_cast<char>(waitKey(1)) != 13) {
        baseImg.copyTo(img);

        reduceImage(img, angle, (double)electrodes_height / (double)electrodes_width);

        imshow(window, baseImg);
        imshow(window2, img);
    }
    destroyWindow(window);
    destroyWindow(window2);

    //Display pixelise picture
    window = "before pixelise picture";
    window2 = "after pixelise picture";
    namedWindow(window, WINDOW_AUTOSIZE);
    Mat zoomImg;
    img.copyTo(baseImg);
    int zoom(1);
    createTrackbar("zoom", window, &zoom, 20);
    while(static_cast<char>(waitKey(1)) != 13) {
        baseImg.copyTo(img);

        pixeliseImage(img, electrodes_width, electrodes_height);

        img.copyTo(zoomImg);
        extendImage(zoomImg, zoom);

        imshow(window, baseImg);
        imshow(window2, zoomImg);
    }
    destroyWindow(window);
    destroyWindow(window2);

    //Display reverse picture
    namedWindow("reverse picture", WINDOW_AUTOSIZE);
    reverseImage(img);
    extendImage(img, zoom);
    imshow("reverse picture", img);
    waitKey(0); //Wait before next step
    destroyWindow("reverse picture");
}

int main() {
    bool quit(false);
    while(!quit) {
        cout << "What do you want to use ?\n";
        cout << "1 - your webcam\n";
        cout << "2 - a local picture\n";
        cout << "3 - quit\n\n";
        cout << "Enter 1 or 2 or 3 and then press enter\n\n";
        string input("");
        getline(cin, input);

        if(!input.empty()) {
            if(input[0] == '1') {
                useWebcam();
            } else if(input[0] == '2') {
                useFile();
            } else if(input[0] == '3') {
                quit = true;
            }
        }
        cout << "\n\n";
    }

    return 0;
}
