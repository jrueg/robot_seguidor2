echo Iniciando Servo Blaster
cd /home/pi/ServoBlaster/
./servod --p1pins="11,12"
cd /home/pi/robot_seguidor/
echo Descargando ultima version desde Github
git pull
echo Compilando programa...
g++ -std=c++0x -o robot *.cpp -I/usr/local/include/ -lraspicam -lraspicam_cv -lrt -lmmal -lmmal_core -lmmal_util -lopencv_core -lopencv_highgui -lopencv_imgproc -L/opt/vc/lib -lpthread -lwiringPi
echo Fin compilado.
echo Ejecutando...
./robot