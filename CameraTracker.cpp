#include "CameraTracker.h"

using namespace cv;

CameraTracker::CameraTracker() : serial(NULL)
{
    serial = new SimpleSerial("/dev/ttyACM0",9600);
    serial->writeChar('0');//ctor
}

CameraTracker::~CameraTracker()
{
    delete serial;//dtor
}

void CameraTracker::run()
{


    RASPIVID_CONFIG * config = (RASPIVID_CONFIG*)malloc(sizeof(RASPIVID_CONFIG));

	config->width=320;
	config->height=240;
	config->bitrate=0;	// zero: leave as default
	config->framerate=0;
	config->monochrome=0;
	//some boolean variables for different functionality within this
	//program
    bool trackObjects = true;
    bool useMorphOps = true;
	//Matrix to store each frame of the webcam feed
	Mat cameraFeed;
	//matrix storage for HSV image
	Mat HSV;
	//matrix storage for binary threshold image
	Mat threshold;
	//x and y values for the location of the object
	int x=0, y=0;
	//video capture object to acquire webcam feed
    RaspiCamCvCapture * capture = (RaspiCamCvCapture *) raspiCamCvCreateCameraCapture2(0, config);
	free(config);

	//start an infinite loop where webcam feed is copied to cameraFeed matrix
	//all of our operations will be performed within this loop
	while(1){
	    IplImage* image = raspiCamCvQueryFrame(capture);
	    Mat cameraFeed(image);

		//convert frame from BGR to HSV colorspace
		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
		//filter HSV image between values and store filtered image to
		//threshold matrix
		inRange(HSV,Scalar(H_MIN,S_MIN,V_MIN),Scalar(H_MAX,S_MAX,V_MAX),threshold);
		//perform morphological operations on thresholded image to eliminate noise
		//and emphasize the filtered object(s)
		if(useMorphOps)
            morphOps(threshold);
		//pass in thresholded frame to our object tracking functionc
		//this function will return the x and y coordinates of the
		//filtered object
		if(trackObjects)
			trackFilteredObject(x,y,threshold,cameraFeed);

		//show frames
		imshow("threshold",threshold);
		//imshow("camerafeed",cameraFeed);
		//simshow("hsv",HSV);


		//delay 30ms so that screen can refresh.
		//image will not appear without this waitKey() command
		waitKey(30);
	}
}

void CameraTracker::morphOps(Mat& thresh){

	//create structuring element that will be used to "dilate" and "erode" image.
	//the element chosen here is a 3px by 3px rectangle
	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
    //dilate with larger element so make sure object is nicely visible
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	erode(thresh,thresh,erodeElement);
	erode(thresh,thresh,erodeElement);

	dilate(thresh,thresh,dilateElement);
	dilate(thresh,thresh,dilateElement);
}

void CameraTracker::trackFilteredObject(int& x, int& y, Mat threshold, Mat& cameraFeed)
{
	Mat temp;
	threshold.copyTo(temp);
	//these two vectors needed for output of findContours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;
	//find contours of filtered image using openCV findContours function
	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	//use moments method to find our filtered object
	double refArea = 0;
	bool objectFound = false;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
        //if number of objects greater than MAX_NUM_OBJECTS we have a noisy filter
        if(numObjects<MAX_NUM_OBJECTS)
        {
			for (int index = 0; index >= 0; index = hierarchy[index][0]){

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;
				//if the area is less than 20 px by 20px then it is probably just noise
				//if the area is the same as the 3/2 of the image size, probably just a bad filter
				//we only want the object with the largest area so we safe a reference area each
				//iteration and compare it to the area in the next iteration.
                if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
					x = moment.m10/area;
					y = moment.m01/area;
					objectFound = true;
					refArea = area;
				}
				else{
				    objectFound = false;
				}
			}
			//let user know you found an object
			if(objectFound ==true)
			{
			    std::cout << "Tracking Object x: " << x << " y: " << y << std::endl;
                if(x < 130)
                    serial->writeChar('4');
                else if(x > 190)
                    serial->writeChar('6');
			}
			else
                serial->writeChar('0');

        }else std::cout << "Too much noise, adjust filter !" << std::endl;
	}
}

