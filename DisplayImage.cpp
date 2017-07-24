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

int showAfterTimeOfAppeared = 2;
int hideAfterTimeOfDissapeared = 1;


Mat PreviousAllFaces[5];
Point PreviousAllFacesCenters[5];

int main() {
  VideoCapture cap(0); // capture from default camera
  Mat frame;
  Point priorCenter(0, 0);

  face_cascade.load("haarcascade_frontalface_alt.xml"); // load face classifiers
  eyes_cascade.load("haarcascade_eye_tree_eyeglasses.xml"); // load eye classifiers

  namedWindow(face_window, CV_WINDOW_AUTOSIZE | CV_WINDOW_FREERATIO | CV_GUI_EXPANDED);

  // Loop to capture frames
  while(cap.read(frame)) {
    // Apply the classifier to the frame, i.e. find face
    priorCenter = detectFace(frame, priorCenter);

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

// Find face from eyes
Point faceFromEyes(Point priorCenter, Mat face) {

  std::vector<Rect> eyes;
  int avg_x = 0;
  int avg_y = 0;

  // Try to detect eyes, if no face is found
  eyes_cascade.detectMultiScale(face, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30));

  // Iterate over eyes
  for(size_t j = 0; j < eyes.size(); j++) {

    // centerpoint of eyes
    Point eye_center(priorCenter.x + eyes[j].x + eyes[j].width/2, priorCenter.y + eyes[j].y + eyes[j].height/2);

    // Average center of eyes
    avg_x += eye_center.x;
    avg_y += eye_center.y;
  }

  // Use average location of eyes
  if(eyes.size() > 0) {
    priorCenter.x = avg_x / eyes.size();
    priorCenter.y = avg_y / eyes.size();
  }

  return priorCenter;
}

// Rounds up to multiple
int roundUp(int numToRound, int multiple) {

  if (multiple == 0) return numToRound;

  int remainder = abs(numToRound) % multiple;
  if (remainder == 0) return numToRound;
  if (numToRound < 0) return -(abs(numToRound) - remainder);
  return numToRound + multiple - remainder;
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
  printf("**************************start**************************\n");
  for( size_t i = 0; i < faces.size(); i++ ) //iterate over all faces
  {
    Point center( faces[i].x + faces[i].width*0.5, faces[i].y + faces[i].height*0.5 ); // postavis se na sredino obraza
    // h = roundUp(faces[i].height, frame.size().height / 4);
    // w = 3 * h / 5;
    h = 240; //postavis velikost za vsaki obraz na novemu frame
    w = 144; //postavis velikost za vsaki obraz na novemu frame
    temp = outputFrame(frame, center, w, h);
    currentAllFaces[i] = temp;
    currentAllFacesCenters[i] = center;
    // printf("%d - %d\n", center.x, center.y);

    // printf("%d: Obraz stevilka: %d, dolzina: %d\n", i, (currentAllFaces[i].size().height),(currentAllFaces[i].size().width));
    // printf("frane size width: %d\n",frame.size().width);
    // printf("h %d, w %d\n",h,w);
  //   int c = 1, d = 1;
   //
  //  for ( c = 1 ; c <= 32767 ; c++ ){
  //     for ( d = 1 ; d <= 6000 ; d++ ){
   //
  //     }
  //   }
  }
  if(faces.size() == 0) {
    faceNotFound = true;
  }

  count_tmp = (sizeof(currentAllFaces)/sizeof(*currentAllFaces));
  printf("Vseh pizdarij je %d; Vseh ze narejenih obrazov je%d\n",count_tmp, numOfPrevFaces );

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
        // printf("se ne spreminja ker ne najdem vec osebe\n");
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

    //pokazemo okno na face_window; ce ni nobenega face potem pokazi cel window
    if(count == 0) {
      printf("Tukaj!\n");
      if(numOfPrevFaces == 0) imshow(face_window, frame);
      else {
        for (size_t i = 0; i < numOfPrevFaces; i++) {
          timesDissapered[i] = (int)time(NULL);
          if(abs(timesDissapered[i]-timesAppeared[i]) > hideAfterTimeOfDissapeared) {
            printf("Skrij me :(\n");
          }
        }
      }
    } else {

      // premore le 5 oseb; zato jih pokaze le toliko
       // seconds
      // tv.tv_usec // microseconds
      // printf("Vseh pravih obrazov je: %d, ob %d\n ", count, (int)time(NULL));
      // scanf("%d\n", );

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
            // printf("%d < %d AND %d < %d : %d\n", abs(currentAllFacesCenters[i].x - PreviousAllFacesCenters[j].x), frame.size().width / 6 , abs(currentAllFacesCenters[i].y - PreviousAllFacesCenters[j].y) , frame.size().height / 6,0);
            if(abs(currentAllFacesCenters[i].x - PreviousAllFacesCenters[j].x) < frame.size().width / 6 && abs(currentAllFacesCenters[i].y - PreviousAllFacesCenters[j].y) < frame.size().height / 6) {
              // printf("Je isti!!!\n" );
              usedCurrentFaces[j] = true;
              jeIsti = true;
              break;
            }
          }
          // printf("jeIsti: %d\n", jeIsti );
          //ce nismo nasli istega ga dodamo, ce je se prostora
          if(!jeIsti && numOfPrevFaces < 5) {
            // PreviousAllFaces[numOfPrevFaces] = currentAllFaces[j-1];
            // printf("tukaj sem\n");
            // PreviousAllFacesCenters[numOfPrevFaces] = currentAllFacesCenters[j-1];
            // printf("tukaj sem\n");
            // numOfPrevFaces += 1;
          }else if(jeIsti) { //ce je isti ga posodobimo
            visitedPreviousFaces[i] = true;
            PreviousAllFaces[i] = currentAllFaces[j];
            PreviousAllFacesCenters[i] = currentAllFacesCenters[j];
            timesAppeared[i] = (int)time(NULL);
            timesDissapered[i] = -1;
          }else {
            printf("Prevec obrazov\n");
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
            printf("Nov obraz\n");

            PreviousAllFaces[numOfPrevFaces] = currentAllFaces[i];
            // printf("tukaj sem\n");
            PreviousAllFacesCenters[numOfPrevFaces] = currentAllFacesCenters[i];
            // printf("tukaj sem\n");
            numOfPrevFaces += 1;
          }
        }

        Mat PreviousAllFacesTmp[maxFaces];
        Point PreviousAllFacesCentersTmp[maxFaces];
        int timesDissaperedTmp[maxFaces];
        int timesAppearedTmp[maxFaces];
        int numOfPrevFacesTmp = 0;
        for (size_t i = 0; i < numOfPrevFaces; i++) {
          printf("Korak %d od %d\n", i, numOfPrevFaces);
          // printf("**velikostPreviousTmp %d\n", velikostPreviousTmp);

          if(i >= velikostPreviousTmp){ //ce ja kaksen nov
            printf("Jst se nisem obstajal\n");

            timesAppearedTmp[numOfPrevFacesTmp] = (int)time(NULL);
            timesDissaperedTmp[numOfPrevFacesTmp] = -1;
            PreviousAllFacesTmp[numOfPrevFacesTmp] = PreviousAllFaces[i];
            PreviousAllFacesCentersTmp[numOfPrevFacesTmp] = PreviousAllFacesCenters[i];
            numOfPrevFacesTmp += 1;
          }else{
            printf("***jst sem ze obstajal\n");

            //ce je se tukaj oz je obiskan
            if(visitedPreviousFaces[i]){
              printf("******sem spet obiskan\n");
              timesAppearedTmp[numOfPrevFacesTmp] = (int)time(NULL);
              timesDissaperedTmp[numOfPrevFacesTmp] = -1;
              PreviousAllFacesTmp[numOfPrevFacesTmp] = PreviousAllFaces[i];
              PreviousAllFacesCentersTmp[numOfPrevFacesTmp] = PreviousAllFacesCenters[i];
              numOfPrevFacesTmp += 1;
            } else { //ce ga ni
              if(timesDissapered[i] == -1) { //ce prej se ni zginil
                printf("*********sem prvic izginil\n");
                timesDissaperedTmp[numOfPrevFacesTmp] = (int)time(NULL);
                PreviousAllFacesTmp[numOfPrevFacesTmp] = PreviousAllFaces[i];
                PreviousAllFacesCentersTmp[numOfPrevFacesTmp] = PreviousAllFacesCenters[i];
                numOfPrevFacesTmp += 1;
              }else if(abs(timesDissapered[i]-timesAppeared[i]) > hideAfterTimeOfDissapeared){
                printf("*********se ne stejem vec med vami\n");
                //je cas da izgine po dveh sekundah
                //ne naredimo nicesar
                // numOfPrevFacesTmp += 1;

              }else{
                printf("*********xoxoxoxoxxoxoxoxoxoxoxoxoxoxooxsem prisel nazaj\n");
                //drugace ce je ze prej izginil samo ni ce poteklo hideAfterTimeOfDissapeared casa
                //ga vseeno pokazemo ampak povecamo time dissapeared
                timesAppearedTmp[numOfPrevFacesTmp] = timesAppeared[i];
                timesDissaperedTmp[numOfPrevFacesTmp] = (int)time(NULL);
                PreviousAllFacesTmp[numOfPrevFacesTmp] = PreviousAllFaces[i];
                PreviousAllFacesCentersTmp[numOfPrevFacesTmp] = PreviousAllFacesCenters[i];
                numOfPrevFacesTmp += 1;
              }
            }


          }
        }
        for(size_t loop = 0; loop < numOfPrevFacesTmp; loop++) {
          // copied[loop] = original[loop];
          PreviousAllFaces[loop] = PreviousAllFacesTmp[loop];
          PreviousAllFacesCenters[loop] = PreviousAllFacesCentersTmp[loop];
          timesAppeared[loop] = timesAppearedTmp[loop];
          timesDissapered[loop] = timesDissaperedTmp[loop];
        }
        numOfPrevFaces = numOfPrevFacesTmp;
        printf("XXXXXXXXXX konec prepisov xxxxxxxxxx\n");

      }


      Mat im3;
      // if(count > 5){
      //   im3.create(240, 144*5, CV_8UC3); // ustvaris horizontalni view za 5 oseb
      // }else im3.create(240, 144*count, CV_8UC3); // ustvaris horizontalni view za count oseb
      im3.create(240, 144*numOfPrevFaces, CV_8UC3);


      // sprehodi se po PreviousAllFaces in izrisi obraze po vrsti
      // printf("Vseh obrazov v finalni obliki: %d\n", numOfPrevFaces );
      stevec = 0;
      for (size_t j = 0; j < numOfPrevFaces; j++) {
        Mat im1 = PreviousAllFaces[j];
        Size sz1 = im1.size();

        // printf("%d: ima prvi param: %d, drugi param: %d, tretji param %d\n", PreviousAllFacesCenters[j].x * j, PreviousAllFacesCenters[j].x, PreviousAllFacesCenters[j].y);
        // im1.copyTo(im3(Rect(PreviousAllFaces[j].x * 0, 0, PreviousAllFaces[j].x, PreviousAllFaces[j].y)));
        // printf("%d: ima prvi param: %d, drugi param: %d, tretji param %d\n",j, sz1.width * j, sz1.width, sz1.height);
        // printf("........%d: ima prvi param: %d, drugi param: %d, tretji param %d\n",j, PreviousAllFacesCenters[j].x * j, PreviousAllFacesCenters[j].x, PreviousAllFacesCenters[j].y);
        // printf("\n");
        // if(sz1.height != 240) continue; //naj se ne bi zgodilo
        if(abs(timesAppeared[j] - (int)time(NULL)) > showAfterTimeOfAppeared) {

        }
        im1.copyTo(im3(Rect(sz1.width * stevec, 0, sz1.width, sz1.height)));
        stevec += 1;
      }
      // printf("\n");
      // printf("\n");
      //pokazi vse dobljene obraze
      imshow(face_window, im3);
    }
  }

  imshow( display_window, frame );
  printf("***************************end***************************\n");

  return priorCenter;
}
