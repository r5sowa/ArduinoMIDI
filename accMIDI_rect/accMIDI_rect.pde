import processing.serial.*;

int lf = 10; 
String myString = null;
Serial myPort;

void setup() {
  size(800,800,P3D);
  println(Serial.list());
  myPort = new Serial(this, "/dev/tty.usbmodem411", 9600);
  myPort.clear();
  myString = myPort.readStringUntil(lf);
  myString = null;
}

void draw() {
  while (myPort.available() > 0) {
    myString = myPort.readStringUntil(lf);
    if (myString != null) {
      float[] x = float(split(myString, ','));
      print("Dimensions = ");
      println(x.length);
      println(x);
      if (x.length == 3) {
        translate(width/2, height/2);
        background(0);
        
        double xRdouble = (double) x[0];
        double xGdouble = (double) x[1];
        double xBdouble = (double) x[2];
        
        int convert;
        
        int xR = (int) (((xRdouble)*255)/1024);
        int xG = (int) (((xGdouble)*255)/1024);
        int xB = (int) (((xBdouble)*255)/1024);
        
        
        stroke(255,255,255);
        lights();
        ambient(xR,xG,xB);
        
        float r = sqrt(sq(x[0])+sq(x[1])+sq(x[2]));
        rotateX(acos((x[1])/r));
        rotateY(acos((x[0])/r)+PI/2);
        rectMode(CENTER);
        rect(0,0,500,500);
      }
    }
  }
}
