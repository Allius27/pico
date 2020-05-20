/*
 *  This code is released under the MIT License.
 *  Copyright (c) 2013 Nenad Markus
 */

#include <stdio.h>

#include <opencv2/imgproc.hpp>
#include <opencv2/highgui.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/imgcodecs.hpp>

#include "../../picornt.h"

using namespace cv;

/*
	a portable time function
*/

#ifdef __GNUC__
#include <time.h>
float getticks()
{
	struct timespec ts;

	if(clock_gettime(CLOCK_MONOTONIC, &ts) < 0)
		return -1.0f;

	return ts.tv_sec + 1e-9f*ts.tv_nsec;
}
#else
#include <windows.h>
float getticks()
{
	static double freq = -1.0;
	LARGE_INTEGER lint;

	if(freq < 0.0)
	{
		if(!QueryPerformanceFrequency(&lint))
			return -1.0f;

		freq = lint.QuadPart;
	}

	if(!QueryPerformanceCounter(&lint))
		return -1.0f;

	return (float)( lint.QuadPart/freq );
}
#endif

/*
	
*/

picornt picorntCore;

int minsize;
int maxsize;

float angle;

float scalefactor;
float stridefactor;

float qthreshold;

int usepyr;
int noclustering;
int verbose;

void process_image(Mat & frame)
{
	int i, j;
	float t;

	uint8_t* pixels;
	int nrows, ncols, ldim;

	#define MAXNDETECTIONS 2048
	int ndetections;
	float rcsq[4*MAXNDETECTIONS];

    static Mat * gray = new Mat(Size(frame.cols , frame.rows), CV_8UC1);
    static Mat * pyr[5];

    // initialize pointer array
	if(!pyr[0])
    {
        pyr[0] = gray;

        pyr[1] = new Mat(Size(frame.cols/2,  frame.rows/2), CV_8UC1, 1);
        pyr[2] = new Mat(Size(frame.cols/4,  frame.rows/4), CV_8UC1, 1);
        pyr[3] = new Mat(Size(frame.cols/8,  frame.rows/8), frame.depth(), 1);
        pyr[4] = new Mat(Size(frame.cols/16, frame.rows/16), frame.depth(), 1);
	}

	// get grayscale image
    if(frame.channels() == 3)
        cvtColor(frame, *gray, COLOR_BGR2GRAY);
    else
        gray->copyTo(frame,0);

	// perform detection with the pico library
	t = getticks();

	if(usepyr)
	{
		int nd;

		//
		pyr[0] = gray;

        pixels = (uint8_t*)pyr[0]->data;
        nrows = pyr[0]->rows;
        ncols = pyr[0]->cols;
        ldim = pyr[0]->step;

		ndetections = picorntCore.find_objects(rcsq, MAXNDETECTIONS, angle, pixels, nrows, ncols, ldim, scalefactor, stridefactor, MAX(16, minsize), MIN(128, maxsize));

		for(i=1; i<5; ++i)
		{
            cv::resize(*pyr[i-1], *pyr[i], Size(), 0, 0, INTER_LINEAR);

            pixels = (uint8_t*)pyr[i]->data;
            nrows = pyr[i]->rows;
            ncols = pyr[i]->cols;
            ldim = pyr[i]->step;

            nd = picorntCore.find_objects(&rcsq[4*ndetections], MAXNDETECTIONS-ndetections, angle, pixels, nrows, ncols, ldim, scalefactor, stridefactor, MAX(64, minsize>>i), MIN(128, maxsize>>i));

			for(j=ndetections; j<ndetections+nd; ++j)
			{
				rcsq[4*j+0] = (1<<i)*rcsq[4*j+0];
				rcsq[4*j+1] = (1<<i)*rcsq[4*j+1];
				rcsq[4*j+2] = (1<<i)*rcsq[4*j+2];
			}

			ndetections = ndetections + nd;
		}
	}
	else
	{
		//
        pixels = (uint8_t*)gray->data;
        nrows = gray->rows;
        ncols = gray->cols;
        ldim = gray->step;

		//
		ndetections = picorntCore.find_objects(rcsq, MAXNDETECTIONS, angle, pixels, nrows, ncols, ldim, scalefactor, stridefactor, minsize, MIN(nrows, ncols));
	}

	if(!noclustering)
		ndetections = picorntCore.cluster_detections(rcsq, ndetections);

	t = getticks() - t;

    for(i=0; i<ndetections; ++i)
        if(rcsq[4*i+3]>=qthreshold) // check the confidence threshold
        {
            auto point = rcsq[4*i+1];
            circle(frame, Point(rcsq[4*i+1], rcsq[4*i+0]), rcsq[4*i+2]/2, CV_RGB(255, 0, 0), 2, 8, 0); // we draw circles here since height-to-width ratio of the detected face regions is 1.0f
        }

	// if the `verbose` flag is set, print the results to standard output
	if(verbose)
	{
		//
		for(i=0; i<ndetections; ++i)
			if(rcsq[4*i+3]>=qthreshold) // check the confidence threshold
				printf("%d %d %d %f\n", (int)rcsq[4*i+0], (int)rcsq[4*i+1], (int)rcsq[4*i+2], rcsq[4*i+3]);

		//
		//printf("# %f\n", 1000.0f*t); // use '#' to ignore this line when parsing the output of the program
	}
}

void process_webcam_frames()
{
    cv::VideoCapture * capture;
    cv::Mat frame;

    capture = new cv::VideoCapture(2);

    if( !capture )
	{
		printf("# cannot initialize video capture ...\n");
		return;
	}

	// the main loop
    while(true)
	{
        if ( !capture->read(frame) )
            break;

        // wait 5 miliseconds
        int key = cv::waitKey(5);

        // we terminate the loop if the user has pressed 'q'
        if( frame.empty() || key=='q')
            break;
        else
		{
            cv::flip( frame, frame, 1);

            process_image(frame);
            imshow("process", frame);
		}
	}

	// cleanup
    capture->release();
    cv::destroyAllWindows();
}

int main(int argc, char* argv[])
{
	//
	int arg;
	char input[1024], output[1024];

	//
	if(argc<2 || 0==strcmp("-h", argv[1]) || 0==strcmp("--help", argv[1]))
	{
		printf("Usage: pico <path/to/cascade> <options>...\n");
		printf("Detect objects in images.\n");
		printf("\n");

		// command line options
		printf("Mandatory arguments to long options are mandatory for short options too.\n");
		printf("  -i,  --input=PATH          set the path to the input image\n");
		printf("                               (*.jpg, *.png, etc.)\n");
		printf("  -o,  --output=PATH         set the path to the output image\n");
		printf("                               (*.jpg, *.png, etc.)\n");
		printf("  -m,  --minsize=SIZE        sets the minimum size (in pixels) of an\n");
		printf("                               object (default is 128)\n");
		printf("  -M,  --maxsize=SIZE        sets the maximum size (in pixels) of an\n");
		printf("                               object (default is 1024)\n");
		printf("  -a,  --angle=ANGLE         cascade rotation angle:\n");
		printf("                               0.0 is 0 radians and 1.0 is 2*pi radians\n");
		printf("                               (default is 0.0)\n");
		printf("  -q,  --qthreshold=THRESH   detection quality threshold (>=0.0):\n");
		printf("                               all detections with estimated quality\n");
		printf("                               below this threshold will be discarded\n");
		printf("                               (default is 5.0)\n");
		printf("  -c,  --scalefactor=SCALE   how much to rescale the window during the\n");
		printf("                               multiscale detection process (default is 1.1)\n");
		printf("  -t,  --stridefactor=STRIDE how much to move the window between neighboring\n");
		printf("                               detections (default is 0.1, i.e., 10%%)\n");
		printf("  -u,  --usepyr              turns on the coarse image pyramid support\n");
		printf("  -n,  --noclustering        turns off detection clustering\n");
		printf("  -v,  --verbose             print details of the detection process\n");
		printf("                               to `stdout`\n");

		//
		printf("Exit status:\n");
		printf(" 0 if OK,\n");
		printf(" 1 if trouble (e.g., invalid path to input image).\n");

		//
		return 0;
	}
    else
    {
    	if ( !picorntCore.loadModel(std::string{argv[1]}) )
    		return 1;
    }

	// set default parameters
	minsize = 128;
	maxsize = 1024;

	angle = 0.0f;

	scalefactor = 1.1f;
	stridefactor = 0.1f;

	qthreshold = 5.0f;

	usepyr = 0;
	noclustering = 0;
	verbose = 0;

	//
	input[0] = 0;
	output[0] = 0;

	// parse command line arguments
	arg = 2;

	while(arg < argc)
	{
		//
		if(0==strcmp("-u", argv[arg]) || 0==strcmp("--usepyr", argv[arg]))
		{
			usepyr = 1;
			++arg;
		}
		else if(0==strcmp("-i", argv[arg]) || 0==strcmp("--input", argv[arg]))
		{
			if(arg+1 < argc)
			{
				//
				sscanf(argv[arg+1], "%s", input);
				arg = arg + 2;
			}
			else
			{
				printf("# missing argument after '%s'\n", argv[arg]);
				return 1;
			}
		}
		else if(0==strcmp("-o", argv[arg]) || 0==strcmp("--output", argv[arg]))
		{
			if(arg+1 < argc)
			{
				//
				sscanf(argv[arg+1], "%s", output);
				arg = arg + 2;
			}
			else
			{
				printf("# missing argument after '%s'\n", argv[arg]);
				return 1;
			}
		}
		else if(0==strcmp("-m", argv[arg]) || 0==strcmp("--minsize", argv[arg]))
		{
			if(arg+1 < argc)
			{
				//
				sscanf(argv[arg+1], "%d", &minsize);
				arg = arg + 2;
			}
			else
			{
				printf("# missing argument after '%s'\n", argv[arg]);
				return 1;
			}
		}
		else if(0==strcmp("-M", argv[arg]) || 0==strcmp("--maxsize", argv[arg]))
		{
			if(arg+1 < argc)
			{
				//
				sscanf(argv[arg+1], "%d", &maxsize);
				arg = arg + 2;
			}
			else
			{
				printf("# missing argument after '%s'\n", argv[arg]);
				return 1;
			}
		}
		else if(0==strcmp("-a", argv[arg]) || 0==strcmp("--angle", argv[arg]))
		{
			if(arg+1 < argc)
			{
				//
				sscanf(argv[arg+1], "%f", &angle);
				arg = arg + 2;
			}
			else
			{
				printf("# missing argument after '%s'\n", argv[arg]);
				return 1;
			}
		}
		else if(0==strcmp("-c", argv[arg]) || 0==strcmp("--scalefactor", argv[arg]))
		{
			if(arg+1 < argc)
			{
				//
				sscanf(argv[arg+1], "%f", &scalefactor);
				arg = arg + 2;
			}
			else
			{
				printf("# missing argument after '%s'\n", argv[arg]);
				return 1;
			}
		}
		else if(0==strcmp("-t", argv[arg]) || 0==strcmp("--stridefactor", argv[arg]))
		{
			if(arg+1 < argc)
			{
				//
				sscanf(argv[arg+1], "%f", &stridefactor);
				arg = arg + 2;
			}
			else
			{
				printf("# missing argument after '%s'\n", argv[arg]);
				return 1;
			}
		}
		else if(0==strcmp("-q", argv[arg]) || 0==strcmp("--qthreshold", argv[arg]))
		{
			if(arg+1 < argc)
			{
				//
				sscanf(argv[arg+1], "%f", &qthreshold);
				arg = arg + 2;
			}
			else
			{
				printf("# missing argument after '%s'\n", argv[arg]);
				return 1;
			}
		}
		else if(0==strcmp("-n", argv[arg]) || 0==strcmp("--noclustering", argv[arg]))
		{
			noclustering = 1;
			++arg;
		}
		else
		{
			printf("# invalid command line argument '%s'\n", argv[arg]);
			return 1;
		}
	}

	// if(verbose)
	// {
	// 	//
	// 	printf("# Copyright (c) 2013, Nenad Markus\n");
	// 	printf("# All rights reserved.\n\n");

	// 	printf("# cascade parameters:\n");
	// 	printf("#	version = %d\n", ((int*)cascade)[0]);
	// 	printf("#	tdepth = %d\n", ((int*)cascade)[2]);
	// 	printf("#	ntrees = %d\n", ((int*)cascade)[3]);
	// 	printf("# detection parameters:\n");
	// 	printf("#	minsize = %d\n", minsize);
	// 	printf("#	maxsize = %d\n", maxsize);
	// 	printf("#	scalefactor = %f\n", scalefactor);
	// 	printf("#	stridefactor = %f\n", stridefactor);
	// 	printf("#	qthreshold = %f\n", qthreshold);
	// 	printf("#	usepyr = %d\n", usepyr);
	// }

	//
    if(0 == input[0])
                process_webcam_frames();
    else
    {
        cv::Mat img = imread(input);

        if(img.empty())
        {
            printf("# cannot load image from '%s'\n", argv[3]);
            return 1;
        }

        process_image(img);

        //
        if(0!=output[0])
        {
            imwrite(std::string{output}, img);
        }
        else if(!verbose)
        {
            imshow(std::string{input}, img);
            waitKey(0);
        }
    }

	return 0;
}
