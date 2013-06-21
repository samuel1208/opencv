

#include "CLRWeakLearner.hpp"
#include <stdio.h>
#include "opencv2/calib3d.hpp"
   #include "opencv2/core.hpp"
   #include "opencv2/features2d.hpp"
   #include "opencv2/highgui.hpp"
   #include "opencv2/nonfree.hpp"
using namespace cv;
int main()
{
    // CWeakLearner *wl = NULL;

    // float pFeaPos[9] = {1,3,3,2,4,3,5,9,3};
    // float pFeaNeg[16] = {5,3,3,4,7,4,3,4,10,9,3,4, 12, 11,3,4};

    // float test[2] = {1,1};
    // float label;

    // wl = new CLRWeakLearner(NULL, 2);

    // wl->initial();
    // wl->solve(pFeaPos, 3, 3, pFeaNeg,4,4);
    // wl->detect(test,&label);
    // printf("%f", label);
    // wl->unInitial();

    // if( argc != 3 )
    //   { return -1; }

     Mat img_1 = imread("/home/samuel/Data/images.jpeg", 0 );
     // Mat img_2 = imread( argv[2], CV_LOAD_IMAGE_GRAYSCALE );

     // if( !img_1.data || !img_2.data )
     //  { return -1; }

     //-- Step 1: Detect the keypoints using SURF Detector
     int minHessian = 400;

SurfFeatureDetector detector(minHessian,4,2,false, true);

     std::vector<KeyPoint> keypoints_1, keypoints_2;

     detector.detect( img_1, keypoints_1 );
     //  detector.detect( img_2, keypoints_2 );

     //-- Step 2: Calculate descriptors (feature vectors)
     SurfDescriptorExtractor extractor(minHessian,4,2,false, true);

     Mat descriptors_1, descriptors_2;

     extractor.compute( img_1, keypoints_1, descriptors_1 );
     // extractor.compute( img_2, keypoints_2, descriptors_2 );

    
    
    return 0;
    
}
