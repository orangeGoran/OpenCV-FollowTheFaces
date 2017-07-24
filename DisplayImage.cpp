#include "opencv2/objdetect.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include <time.h>

#include <iostream>
#include <stdio.h>
#include<stdlib.h>
#include <cmath>

using namespace std;
using namespace cv;

/* Function Headers */
Point detectFace( Mat frame, Point priorCenter );

CascadeClassifier face_cascade, eyes_cascade;

String display_window = "Display";
String face_window = "Face View";

int maxFaces = 5;
int numOfPrevFaces = 0;

int timesDissapered[5];
int timesAppeared[5];
int timesAppearedFirst[5];

int showAfterTimeOfAppeared = 5;
int hideAfterTimeOfDissapeared = 5;

int c = 1, d = 1;
Mat PreviousAllFaces[5];
Point PreviousAllFacesCenters[5];

int main() {
  VideoCapture cap(0); // capture from default camera
  Mat frame;
  Point priorCenter(0, 0);

  face_cascade.load("haarcascade_frontalface_alt.xml"); // load face classifiers
  eyes_cascade.load("haarcascade_eye_tree_eyeglasses.xml"); // load eye classifiers

  namedWindow(face_window, CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO | CV_GUI_EXPANDED);
  moveWindow(face_window,20,20);
  // Loop to capture frames
  while(cap.read(frame)) {
    detectFace(frame, priorCenter);
    int c = 1, d = 1;
    if(waitKey(30) >= 0) {// spacebar
      break;
    }
  }
  return 0;
}

/**
* Output a frame of only the the rectangle centered at point
*/
Mat outputFrame(Mat frame, Point center, int w, int h) { // funkcija za risanje outputa na Face View window
  int x = (center.x - w/2);
  int y = (center.y - 3*h/5);

  if(x + w > frame.size().width - 2 || x < 0 ||
  y + h > frame.size().height - 2 || y < 0 &&
  frame.size().width > 16 &&
  frame.size().height > 16)
  return frame(Rect(5, 5, 10, 10));

  // output frame of only face
  return frame(Rect(x, y, w, h));
}


// Detect face and display it
Point detectFace(Mat frame, Point priorCenter) {

  std::vector<Rect> faces;
  Mat frame_gray, frame_lab, output, temp;
  int h = frame.size().height - 1;
  int w = frame.size().width - 1;
  int count_tmp = 0;
  int count = 0;
  int stevec = 0;
  int minNeighbors = 2;
  bool faceNotFound = false;

  cvtColor(frame, frame_gray, COLOR_BGR2GRAY);   // Convert to gray
  equalizeHist(frame_gray, frame_gray);          // Equalize histogram

  // Detect face with open source cascade
  face_cascade.detectMultiScale(frame_gray, faces, 1.1, minNeighbors, 0|CASCADE_SCALE_IMAGE, Size(30, 30));

  Mat currentAllFaces[faces.size()];
  Point currentAllFacesCenters[faces.size()];
  // iterate over faces
  for( size_t i = 0; i < faces.size(); i++ ) //iterate over all faces
  {
    Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 ); // postavis se na sredino obraza
    h = 240; //postavis velikost za vsaki obraz na novemu frame
    w = 144; //postavis velikost za vsaki obraz na novemu frame
    temp = outputFrame(frame, center, w, h);
    currentAllFaces[i] = temp;
    currentAllFacesCenters[i] = center;

  }
  if(faces.size() == 0) {
    faceNotFound = true;
  }

  count_tmp = (sizeof(currentAllFaces)/sizeof(*currentAllFaces));

  if(faceNotFound) { //ce ni nobenega obraza
    temp = frame;
    //preveris, ce je slucajno od prej nekaj oseb gor
    //ce niso pokazes prazen oz default frame
    if(numOfPrevFaces == 0) imshow(face_window, temp);
    else {
      //ce je oseba ze od prej gor potem preveri njen cas
      int stevecZivihOseb = 0;
      for (size_t i = 0; i < numOfPrevFaces; i++) {
        timesDissapered[i] = (int)time(NULL);
        if(abs(timesDissapered[i]-timesAppeared[i]) > hideAfterTimeOfDissapeared) {
          //im dead :(
        } else {
          stevecZivihOseb += 1;
        }
      }
      if(stevecZivihOseb == 0) {
        numOfPrevFaces = 0;
        imshow(face_window, temp);
      }else {
        printf("TODO\n");
      }
    }
  }else {
    // count_tmp = (sizeof(currentAllFaces)/sizeof(*currentAllFaces));
    // printf("Vseh pizdarij je %d; Vseh ze narejenih obrazov je%d\n",count_tmp, numOfPrevFaces );
    count = 0;
    for (size_t i = 0; i < count_tmp; i++) { //dobis dejansko stevilo obrazov, ker ima neke smeti zraven
      // printf("Visina obraza: %d\n", (currentAllFaces[i].size().height));

      if(currentAllFaces[i].size().height == 240){
        count++;
      }
    }

    //count_tmp: najdeno stevilo obrazov plus smeti
    //currentAllFaces: vsi obrazi in prav tako s smetmi zraven
    //count: celotno stevilo najdenih pravih obrazov
    //numOfPrevFaces: stevilo obrazov od prej
    //PreviousAllFaces: prejsnji obrazi
    //PreviousAllFacesCenters: vsebuje points kjer se nahaja prejsnji obraz
    //currentAllFacesCenters: vsebuje points kjer se nahaja trenutni obraz
    //stevec:
    //im1:
    //sz1:
    //workflow: sprehodi se po prejsnjih obrazih, ce je obraz se enkrat najdenih
    //mu popravi vrednosti, drugace ga izbrisi ven; izbrisi pa tiste obraze
    //katere nisi se enkrat nasel (boolean array)

    // printf("Count: %d\n", count);

    //pokazemo okno na face_window; ce ni nobenega koristnega obraza potem pokazi cel window
    if(count == 0) {
      printf("Tukaj!\n");
      if(numOfPrevFaces == 0) imshow(face_window, frame);
      else {
        for (size_t i = 0; i < numOfPrevFaces; i++) {
          timesDissapered[i] = (int)time(NULL);
          if(abs(timesDissapered[i]-timesAppeared[i]) > hideAfterTimeOfDissapeared) {
            // printf("TODO ???Skrij me :(\n");
          }
        }
      }
    } else {

      // premore le 5 oseb; zato jih pokaze le toliko
      // printf("Vseh pravih obrazov je: %d, ob %d\n ", count, (int)time(NULL));

      //ce ni nobenega od prejsnjih obrazov, potem jih ustvari; sprejmi pa le
      //prvih pet; prav tako zavrzi smeti v currentAllFaces[i]

      if (numOfPrevFaces == 0){
        for (size_t i = 0; i < count_tmp; i++) { //sprehodi se po currentAllFaces(s smetmi)
          if(numOfPrevFaces >= maxFaces) { //ce imas ze pet obrazov koncaj prerisovanje v PreviousAllFaces[numOfPrevFaces]
            break;
          }
          Mat im1 = currentAllFaces[i];
          Size sz1 = im1.size();
          if(sz1.height != 240) continue; // ce je smet, ga zavrzemo
          PreviousAllFaces[numOfPrevFaces] = im1; //drugace ga dodamo na PreviousAllFaces
          PreviousAllFacesCenters[numOfPrevFaces] = currentAllFacesCenters[i]; //in postavimo points
          timesAppeared[numOfPrevFaces] = (int)time(NULL);
          timesAppearedFirst[numOfPrevFaces] = (int)time(NULL);
          timesDissapered[numOfPrevFaces] = -1;
          numOfPrevFaces +=1; // povecamo stevilo prejsnjih obrazov za 1
        }
      }else {
        //ce pa ze imamo nakaj obrazov potem preveri ce je notri in posodobi vrednosti
        //ce ni notri ga dodaj, ce je se prostora

        //sprehodi se po prejsnjjih obrazih
        //preveri ce sta si dva obraza primerna
        //ce sta ga posodobi

        bool visitedPreviousFaces[numOfPrevFaces];
        bool usedCurrentFaces[count_tmp];
        int velikostPreviousTmp = numOfPrevFaces;
        for (size_t i = 0; i < count_tmp; i++) {
          usedCurrentFaces[i] = false;
        }
        for (size_t i = 0; i < numOfPrevFaces; i++) {
          visitedPreviousFaces[i] = false;
          Mat im0 = PreviousAllFaces[i];
          Size sz0 = im0.size();

          //sprehodi se po currentAllFaces; hranimo spremenljivko bool jeIsti,
          // ki pove ce smo nasli istega elementa ali ne
          bool jeIsti = false;
          size_t j = 0;
          for (; j < count_tmp; j++) {
            Mat im1 = currentAllFaces[j];
            Size sz1 = im1.size();
            if(sz1.height != 240) continue; //preskocimo smeti

            //preverimo ce sta enaka
            if(abs(currentAllFacesCenters[i].x - PreviousAllFacesCenters[j].x) < frame.size().width / 10 && abs(currentAllFacesCenters[i].y - PreviousAllFacesCenters[j].y) < frame.size().height / 10) {
              usedCurrentFaces[j] = true;
              jeIsti = true;
              break;
            }
          }
          //ce nismo nasli istega ga dodamo, ce je se prostora
          if(!jeIsti && numOfPrevFaces < 5) {
            //dodamo potem
          }else if(jeIsti) { //ce je isti ga posodobimo
            visitedPreviousFaces[i] = true;
            PreviousAllFaces[i] = currentAllFaces[j];
            PreviousAllFacesCenters[i] = currentAllFacesCenters[j];
            timesAppeared[i] = (int)time(NULL);
            timesDissapered[i] = -1;
          }else {
            // printf("Prevec obrazov %d\n", numOfPrevFaces);
          }
        }

        //dobimo se nedodane osebe
        for (size_t i = 0; i < count_tmp; i++) {
          if(numOfPrevFaces >= 5) {
            break;
          }
          if(!usedCurrentFaces[i]) {
            Mat im1 = currentAllFaces[i];
            Size sz1 = im1.size();
            if(sz1.height != 240) continue; //preskocimo smeti
            timesAppearedFirst[numOfPrevFaces] = (int)time(NULL);
            PreviousAllFaces[numOfPrevFaces] = currentAllFaces[i];
            PreviousAllFacesCenters[numOfPrevFaces] = currentAllFacesCenters[i];
            numOfPrevFaces += 1;
          }
        }
        Mat PreviousAllFacesTmp[maxFaces];
        Point PreviousAllFacesCentersTmp[maxFaces];
        int timesDissaperedTmp[maxFaces];
        int timesAppearedTmp[maxFaces];
        int timesAppearedFirstTmp[maxFaces];
        int numOfPrevFacesTmp = 0;
        for (size_t i = 0; i < numOfPrevFaces; i++) {

          // printf("Korak %d od %d\n", i, numOfPrevFaces);
          // printf("   Obiskan: %d\n", visitedPreviousFaces[i]);
          // printf("   Pokazal: %d\n", timesAppeared[i]);
          // printf("   Zginill: %d\n", timesDissapered[i]);
          // printf("   Sem za skriti: %d\n", (abs(timesDissapered[i]-timesAppeared[i]) > hideAfterTimeOfDissapeared));

          if(i >= velikostPreviousTmp){ //ce ja kaksen nov
            timesAppearedTmp[numOfPrevFacesTmp] = (int)time(NULL);
            timesAppearedFirstTmp[numOfPrevFacesTmp] = (int)time(NULL);
            timesDissaperedTmp[numOfPrevFacesTmp] = -1;
            PreviousAllFacesTmp[numOfPrevFacesTmp] = PreviousAllFaces[i];
            PreviousAllFacesCentersTmp[numOfPrevFacesTmp] = PreviousAllFacesCenters[i];
            numOfPrevFacesTmp += 1;
          }else{
            //ce je se tukaj oz je obiskan
            if(visitedPreviousFaces[i]){
              timesAppearedTmp[numOfPrevFacesTmp] = (int)time(NULL);
              timesAppearedFirstTmp[numOfPrevFacesTmp] = timesAppearedFirst[i];
              timesDissaperedTmp[numOfPrevFacesTmp] = -1;
              PreviousAllFacesTmp[numOfPrevFacesTmp] = PreviousAllFaces[i];
              PreviousAllFacesCentersTmp[numOfPrevFacesTmp] = PreviousAllFacesCenters[i];
              numOfPrevFacesTmp += 1;
            } else { //ce ga ni
              if(timesDissapered[i] == -1) { //ce prej se ni zginil
                timesAppearedTmp[numOfPrevFacesTmp] = timesAppeared[i];
                timesAppearedFirstTmp[numOfPrevFacesTmp] = timesAppearedFirst[i];
                timesDissaperedTmp[numOfPrevFacesTmp] = (int)time(NULL);
                PreviousAllFacesTmp[numOfPrevFacesTmp] = PreviousAllFaces[i];
                PreviousAllFacesCentersTmp[numOfPrevFacesTmp] = PreviousAllFacesCenters[i];
                numOfPrevFacesTmp += 1;
              }else if(abs(timesDissapered[i]-timesAppeared[i]) > hideAfterTimeOfDissapeared){
                //je cas da izgine po dveh sekundah
                //ne naredimo nicesar
              }else{
                //drugace ce je ze prej izginil samo ni ce poteklo hideAfterTimeOfDissapeared casa
                //ga vseeno pokazemo ampak povecamo time dissapeared
                timesAppearedTmp[numOfPrevFacesTmp] = timesAppeared[i];
                timesAppearedFirstTmp[numOfPrevFacesTmp] = timesAppearedFirst[i];
                timesDissaperedTmp[numOfPrevFacesTmp] = (int)time(NULL);
                PreviousAllFacesTmp[numOfPrevFacesTmp] = PreviousAllFaces[i];
                PreviousAllFacesCentersTmp[numOfPrevFacesTmp] = PreviousAllFacesCenters[i];
                numOfPrevFacesTmp += 1;
              }
            }
          }
        }
        for(size_t loop = 0; loop < numOfPrevFacesTmp; loop++) {
          PreviousAllFaces[loop] = PreviousAllFacesTmp[loop];
          PreviousAllFacesCenters[loop] = PreviousAllFacesCentersTmp[loop];
          timesAppeared[loop] = timesAppearedTmp[loop];
          timesAppearedFirst[loop] = timesAppearedFirstTmp[loop];
          timesDissapered[loop] = timesDissaperedTmp[loop];
        }
        numOfPrevFaces = numOfPrevFacesTmp;
      }


      Mat im3;

      int realNumOfFaces = 0;
      for (size_t i = 0; i < numOfPrevFaces; i++) {
        Mat im1 = PreviousAllFaces[i];
        Size sz1 = im1.size();
        if(sz1.height != 240) continue; //naj se ne bi zgodilo
        if(abs(timesAppearedFirst[i] - (int)time(NULL)) > showAfterTimeOfAppeared) {
          realNumOfFaces +=1;
        }
      }
      // if(realNumOfFaces > 1) {
      //   for (size_t i = 0; i < numOfPrevFaces; i++) {
      //
      //     printf("Korak %d od %d\n", i, numOfPrevFaces);
      //     printf("   Pokazal: %d\n", timesAppeared[i]);
      //     printf("   Zginill: %d\n", timesDissapered[i]);
      //     printf("   Sem za skriti: %d\n", (abs(timesDissapered[i]-timesAppeared[i]) > hideAfterTimeOfDissapeared));
      //   }
      // }
      im3.create(240, 144*numOfPrevFaces, CV_8UC3);

      stevec = 0;
      for (size_t j = 0; j < numOfPrevFaces && realNumOfFaces > 0; j++) {
        Mat im1 = PreviousAllFaces[j];
        Size sz1 = im1.size();
        if(sz1.height != 240) continue; //naj se ne bi zgodilo
        if(abs(timesAppearedFirst[j] - (int)time(NULL)) > showAfterTimeOfAppeared) {
          im1.copyTo(im3(Rect(sz1.width * stevec, 0, sz1.width, sz1.height)));
          stevec += 1;
        }
      }
      if(realNumOfFaces > 0) imshow(face_window, im3);
    }
  }
  imshow( display_window, frame );
}
