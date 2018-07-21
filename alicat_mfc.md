#pragma SPARK_NO_PREPROCESSOR // disable spark preprocssor to avoid issues with callbacks
#include "application.h"


#define MFC_BAUD         19200
#define MFC_SERIAL_CONF  SERIAL_8N1



bool waiting_for_response = true;
// the setup routine runs once when you press reset:
void setup() {
  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  Serial1.begin(19200);
  Serial.println("1");
    while(Serial1.available()) {
    char c = Serial1.read();
  }
  delay(1000);
  Serial.println("how did we get here??");

}
String temp_message = "";
String Pressure = "";   //If you want to use global variables.
String Temp = "";
String VolFlow = "";
String MassFlow = "";
String Setpoint = "";
String Gas = "";
int garbage_byte_counter = 0;





void parse_temps(String Listfromflow) {  // this is for the  flow controler with default units
    String holder ="";
    int firstSpace = Listfromflow.indexOf(' ',+2);   //Find the first space and add two more to then look for values
    for(int x=1; x<firstSpace; x++){   // This range of the array should have the Pressure
      holder += Listfromflow.charAt(x);
      }
    Pressure = holder; //holder.toFloat();
    //Serial.println(holder);  //DEBUG
    Serial.print("Pressure = ");  Serial.println(Pressure);

    int secondSpace = Listfromflow.indexOf(' ', firstSpace + 1 );  //Use the stoping and add one space to find the next range in the array
    holder ="";
    for(int x=firstSpace; x<secondSpace; x++){
      holder += Listfromflow.charAt(x);
      }
    Temp = holder; //holder.toFloat();
    //Serial.println(holder);  //DEBUG
    Serial.print("Temp = ");  Serial.println(Temp);

    int thirdSpace = Listfromflow.indexOf(' ', secondSpace + 1 );
    holder ="";
    for(int x=secondSpace; x<thirdSpace; x++){
      holder += Listfromflow.charAt(x);
    }
    VolFlow = holder; //holder.toFloat();
    //Serial.println(holder);
    Serial.print("VolFlow = ");  Serial.println(VolFlow);

    int fourthSpace = Listfromflow.indexOf(' ', thirdSpace + 1 );
    holder ="";
    for(int x=thirdSpace; x<fourthSpace; x++){
      holder += Listfromflow.charAt(x);
    }
    MassFlow = holder; //holder.toFloat();
    //Serial.println(holder); //DEBUG
    Serial.print("MassFlow = ");  Serial.println(MassFlow);

    int fifthSpace = Listfromflow.indexOf(' ', fourthSpace + 1 );
    holder ="";
    for(int x=fourthSpace; x<fifthSpace; x++){
      holder += Listfromflow.charAt(x);
    }
    Setpoint = holder; //holder.toFloat();
    //Serial.println(holder);//DEBUG
    Serial.print("Setpoint = ");  Serial.println(Setpoint);

    int Endoflist = Listfromflow.length();  //The gas name is bit odd because the lenght of the name changes, use the end and walk back.
    holder ="";
    for(int x=Endoflist-7; x<Endoflist; x++){
      holder += Listfromflow.charAt(x);
    }
    Gas = holder;
    //Serial.println(holder);//DEBUG
    Serial.print("Gas = ");  Serial.println(Gas);

 }


//------------------------------------------------------------------------------------------------------
//Functions
void RequestReceivedata(String Address){
 //If you want to poll an  you just need to send its address (A to Z and @ for streaming)
 // as well as a <CR> or in arduino a `\r`.
   Serial1.print(Address); //ask for vaules
   Serial.print(" address = ");Serial.println(Address); // DEBUG
   delay(100);
   garbage_byte_counter = 0; //reset counter
  if(Serial1.available() > 0) //look to see if there is data
  {
    waiting_for_response = true;
    while(waiting_for_response && Serial1.available() > 0) //While there is data do this
    {
        byte c = Serial1.read();    // get the data and store it char by char well using ASCII
        if (c == 0) {
          // do nothing when 0 character encountered
        } else if (c >= 32 && c <= 126) {
          // regular ASCII range
          temp_message += (char) c;
        } else  if (c == 13) {
          // return character received
          Serial.print(" (+");
          Serial.print(garbage_byte_counter);
          Serial.println(" garbage bytes)");
          Serial.print("Whole line: ");  //DEBUG
          Serial.println(temp_message);
          Serial.println();
          //Serial.print("parse function ");//DEBUG
          parse_temps(temp_message);  // if your useing global variables this will update all of them
          temp_message = "";
          garbage_byte_counter = 0;
          waiting_for_response = false;
        } else if (c == 10) {
          // newline after carriage return -- ignore
        } else {
          // unknown character (127=backspace, 27=esc, etc.)
          garbage_byte_counter++;
          // DEBUG:
          Serial.print("NA");
          Serial.print("(");
          Serial.print((int) c);
          Serial.print(") ");
        }
     }
  }

}




void loop (){
String msg = "A\r";
String holder = "";

RequestReceivedata(msg);
Serial.println(holder);
Serial.print("-------------------");
Serial.println(holder);
delay(3000);
}
