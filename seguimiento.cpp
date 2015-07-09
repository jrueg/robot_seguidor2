#include <sstream>
#include <string>
#include <iostream>
#include <opencv/highgui.h>
#include <opencv/cv.h>
#include <raspicam/raspicam_cv.h>
#include <time.h>
#include "control.h"
#include "main.h"

#define activa_gui

using namespace cv;

//Directivas preprocesador
#define FRAME_WIDTH  320
#define FRAME_HEIGHT  240
#define MAX_NUM_OBJECTS 50
#define MIN_OBJECT_AREA  20*20
#define MAX_OBJECT_AREA  FRAME_HEIGHT*FRAME_WIDTH/1.5

#ifdef activa_gui
const string windowName = "Original Image";
const string windowName1 = "HSV Image";
const string windowName2 = "Thresholded Image";
const string windowName3 = "After Morphological Operations";
const string trackbarWindowName = "Trackbars";
void on_trackbar(int, void*)
{//Esta función se ejecuta cuando cambia la posición de un trackbar

}

string intToString(int number){
	std::stringstream ss;
	ss << number;
	return ss.str();
}

void createTrackbars(struct mem_global *mem_global){

	namedWindow(trackbarWindowName, 0);

	//Creacion de memoria para guardar variables

	char TrackbarName[50];
	sprintf(TrackbarName, "H_MIN", (*mem_global).H_MIN);
	sprintf(TrackbarName, "H_MAX", (*mem_global).H_MAX);
	sprintf(TrackbarName, "S_MIN", (*mem_global).S_MIN);
	sprintf(TrackbarName, "S_MAX", (*mem_global).S_MAX);
	sprintf(TrackbarName, "V_MIN", (*mem_global).V_MIN);
	sprintf(TrackbarName, "V_MAX", (*mem_global).V_MAX);

	//Creacion de trackbars

	createTrackbar("H_MIN", trackbarWindowName, &(*mem_global).H_MIN, (*mem_global).H_MAX, on_trackbar);
	createTrackbar("H_MAX", trackbarWindowName, &(*mem_global).H_MAX, (*mem_global).H_MAX, on_trackbar);
	createTrackbar("S_MIN", trackbarWindowName, &(*mem_global).S_MIN, (*mem_global).S_MAX, on_trackbar);
	createTrackbar("S_MAX", trackbarWindowName, &(*mem_global).S_MAX, (*mem_global).S_MAX, on_trackbar);
	createTrackbar("V_MIN", trackbarWindowName, &(*mem_global).V_MIN, (*mem_global).V_MAX, on_trackbar);
	createTrackbar("V_MAX", trackbarWindowName, &(*mem_global).V_MAX, (*mem_global).V_MAX, on_trackbar);


}

void drawObject(int x, int y, Mat &frame){

	circle(frame, Point(x, y), 20, Scalar(0, 255, 0), 2);
	if (y - 25>0)
		line(frame, Point(x, y), Point(x, y - 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, 0), Scalar(0, 255, 0), 2);
	if (y + 25<FRAME_HEIGHT)
		line(frame, Point(x, y), Point(x, y + 25), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(x, FRAME_HEIGHT), Scalar(0, 255, 0), 2);
	if (x - 25>0)
		line(frame, Point(x, y), Point(x - 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(0, y), Scalar(0, 255, 0), 2);
	if (x + 25<FRAME_WIDTH)
		line(frame, Point(x, y), Point(x + 25, y), Scalar(0, 255, 0), 2);
	else line(frame, Point(x, y), Point(FRAME_WIDTH, y), Scalar(0, 255, 0), 2);

	putText(frame, intToString(x) + "," + intToString(y), Point(x, y + 30), 1, 1, Scalar(0, 255, 0), 2);

}

#endif

void morphOps(Mat &thresh){

	//Operaciones morfologicas

	//Modelo para el tratamiento
	Mat erodeElement = getStructuringElement( MORPH_RECT,Size(3,3));
	Mat dilateElement = getStructuringElement( MORPH_RECT,Size(8,8));

	//Erosion + dilatacion (Apertura)
	erode(thresh,thresh,erodeElement);
	dilate(thresh,thresh,dilateElement);

}

void trackFilteredObject(struct mem_global *mem_global, Mat threshold, Mat &cameraFeed){

	Mat temp;
	threshold.copyTo(temp);
	//Vectores para la salida de findcontours
	vector< vector<Point> > contours;
	vector<Vec4i> hierarchy;

	findContours(temp,contours,hierarchy,CV_RETR_CCOMP,CV_CHAIN_APPROX_SIMPLE );
	
	double refArea = 0;
	if (hierarchy.size() > 0) {
		int numObjects = hierarchy.size();
        if(numObjects<MAX_NUM_OBJECTS){ //Asegurar que no es ruido
			
			for (int index = 0; index >= 0; index = hierarchy[index][0]) {

				Moments moment = moments((cv::Mat)contours[index]);
				double area = moment.m00;
				//A continuación se busca el objeto mas grande, suponemos objeto a seguir
                if(area>MIN_OBJECT_AREA && area<MAX_OBJECT_AREA && area>refArea){
					(*mem_global).x = moment.m10/area;
					(*mem_global).y = moment.m01/area;
					(*mem_global).objetoEncontrado = true;
					refArea = area;
				}
				else{
					//En el caso de que no se encuentre ninguno
					(*mem_global).objetoEncontrado = false;
					(*mem_global).x = 160;
					(*mem_global).y = 120;
				}
			}
			#ifdef activa_gui
				//Dibujar cruceta
				if ((*mem_global).objetoEncontrado == true){
					putText(cameraFeed, "Siguiendo objeto", Point(0, 50), 2, 1, Scalar(0, 255, 0), 2);
					
					drawObject((*mem_global).x, (*mem_global).y, cameraFeed);
				}
			#endif
		}
		
		#ifdef activa_gui
		else putText(cameraFeed, "DEMASIADO RUIDO, AJUSTA FILTRO!", Point(0, 50), 1, 2, Scalar(0, 0, 255), 2);
		#endif
	}
	
}

void seguimiento(struct mem_global *mem_global)
{
	//Matrices para guardas las imagenes
	Mat cameraFeed;
	Mat HSV;
	Mat threshold;

	#ifdef activa_gui
		createTrackbars(mem_global);
	#endif

	//Abrir raspicam y configurar la captura
	
	raspicam::RaspiCam_Cv Camera;
	Camera.set(CV_CAP_PROP_FRAME_WIDTH, FRAME_WIDTH);
	Camera.set(CV_CAP_PROP_FRAME_HEIGHT, FRAME_HEIGHT);
	Camera.open();
	
	//Comprobacion temporal
	//struct timespec tstart = {0,0}, tend = {0,0};

	//Control de servos
	//Servo en x
	int pos0 = 50;
	controlador_p con_s0(0.05, 90, -90, 10);
	con_s0.setpoint(160);
	servoBlaster(0, pos0);

	//Servo en y
	int pos1 = 60;
	controlador_p con_s1(0.05, 90, -90, 10);
	con_s1.setpoint(120);
	servoBlaster(1, pos1);

	while ((*mem_global).salida){

		//Comprobacion temporal
		//clock_gettime(CLOCK_MONOTONIC, &tstart);
		
		//Obtener imagen
		Camera.grab();
		Camera.retrieve(cameraFeed);
		//Convertir a HSV
		cvtColor(cameraFeed,HSV,COLOR_BGR2HSV);
		//Binarizar con los umbrales seleccionados
		inRange(HSV,Scalar((*mem_global).H_MIN,(*mem_global).S_MIN,(*mem_global).V_MIN),Scalar((*mem_global).H_MAX,(*mem_global).S_MAX,(*mem_global).V_MAX),threshold);
		//Tratar la imagen con los operadores morfologicos
		morphOps(threshold);
		//Obtener objetos de imagen tratada
		trackFilteredObject(mem_global ,threshold,cameraFeed);
		
		#ifdef activa_gui
			//Mostrar ventanas
			imshow(windowName2, threshold);
			//imshow(windowName1,HSV);
			imshow(windowName, cameraFeed);
			
			waitKey(10);
		#endif

		//Comprobacion temporal
		//clock_gettime(CLOCK_MONOTONIC, &tend);
		
		//Control de servos
		if ((*mem_global).objetoEncontrado){
			//Servo en x
			pos0 -=	con_s0.calculo_realim((*mem_global).x);
			if (pos0 > 90) pos0 = 90;
			if (pos0 < 10) pos0 = 10;
			servoBlaster(0, pos0);

			//Servo en y
			pos1 -= con_s1.calculo_realim((*mem_global).y);
			if (pos1 > 70) pos1 = 70;
			if (pos1 < 40) pos1 = 40;
			servoBlaster(1, pos1);
		}

	}


	servoBlaster(0, 50);
	servoBlaster(1, 60);

}
