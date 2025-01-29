#include <arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <PS2X_lib.h>  //for v1.6

//PS2X Stuff:
#define PS2_DAT        12  //D6      
#define PS2_CMD        5   //D1  
#define PS2_SEL        16  //D0
#define PS2_CLK        13  //D7  
//buttons read as digital, no rumble.
#define rumble false
#define pressures false
//Create intance of controller class:
PS2X ps2x;
//Not sure what this is yet:
int error = 0;
byte type = 0;
byte vibrate = 0;

const int deadbandWidth = 50;

const char* moustache_ssid     = "Moustache AP";         // The SSID (name) of the Wi-Fi network you want to connect to
const char* moustache_password = "thereentnospoon";     // The password of the Wi-Fi network
const char* moustache_ip = "192.168.1.11";
const char* moustache_port = 4210 // port to send UDP packets to

WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on

int timeSinceLastPacketSent = 0;

void setup() {
  Serial.begin(115200);         // Start the Serial communication to send messages to the computer
  delay(10);
  Serial.println('\n');

  // Connect to controller:
  delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
  //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
  error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
  if(error == 0){
    Serial.print("Found Controller, configured successful ");
    Serial.print("pressures = ");
  if (pressures)
    Serial.println("true ");
  else
    Serial.println("false");
  Serial.print("rumble = ");
  if (rumble)
    Serial.println("true)");
  else
    Serial.println("false");
    Serial.println("Try out all the buttons, X will vibrate the controller, faster as you press harder;");
    Serial.println("holding L1 or R1 will print out the analog stick values.");
    Serial.println("Note: Go to www.billporter.info for updates and to report bugs.");
  }  
  else if(error == 1)
    Serial.println("No controller found, check wiring, see readme.txt to enable debug. visit www.billporter.info for troubleshooting tips");
   
  else if(error == 2)
    Serial.println("Controller found but not accepting commands. see readme.txt to enable debug. Visit www.billporter.info for troubleshooting tips");

  else if(error == 3)
    Serial.println("Controller refusing to enter Pressures mode, may not support it. ");
    
  type = ps2x.readType(); 
  switch(type) {
    case 0:
      Serial.print("Unknown Controller type found \n");
      break;
    case 1:
      Serial.print("DualShock Controller found \n");
      break;
    case 2:
      Serial.print("GuitarHero Controller found \n");
      break;
   case 3:
      Serial.print("Wireless Sony DualShock Controller found \n");
      break;
   }

  // Connect to the network
  WiFi.mode(WIFI_STA);
  WiFi.begin(moustache_ssid, moustache_password);
  Serial.print("Connecting to ");
  Serial.print(moustache_ssid); Serial.println(" ...");
  int i = 0;
  while (WiFi.status() != WL_CONNECTED) { // Wait for the Wi-Fi to connect
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
  Serial.println('\n');
  Serial.println("Connection established!");
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());         // Send the IP address of the ESP8266 to the computer

  if (! Udp.begin(localUdpPort)) {
    Serial.println("Udp failed to start. No sockets available to use.");
  }

  WiFi.printDiag(Serial);

  Udp.begin(localUdpPort);

}

void loop() {

  if(error == 1) {
    // Controller conneciton lost, attempt to reconnect
    Serial.println("Connection to controller lost");
    delay(300);  //added delay to give wireless ps2 module some time to startup, before configuring it
    //setup pins and settings: GamePad(clock, command, attention, data, Pressures?, Rumble?) check for error
    error = ps2x.config_gamepad(PS2_CLK, PS2_CMD, PS2_SEL, PS2_DAT, pressures, rumble);
    return; 
  }
  ps2x.read_gamepad(); 
  
  unsigned char message[9];
  unsigned char LX = static_cast<byte>(ps2x.Analog(PSS_LX));
  unsigned char LY = static_cast<byte>(ps2x.Analog(PSS_LY));
  unsigned char RX = static_cast<byte>(ps2x.Analog(PSS_RX));
  unsigned char RY = static_cast<byte>(ps2x.Analog(PSS_RY));
  bool L1_pressed = ps2x.ButtonPressed(PSB_L1);
  bool R1_pressed = ps2x.ButtonPressed(PSB_R1);
  bool buttons[8] = {false}; //Left, Right, Up, Down, Circle, Square, Triangle, X.
//  if(L1_pressed){
//    Serial.println("L1 Pressed");
//    delay(500);
//    buttons +=1;
//  }
//  if(R1_pressed){ //TODO: R1 button seems to be buggered on this controller.
//    Serial.println("R1 Pressed");
//    delay(500);
//    buttons += 2;
//  }
//  unsigned char buttons = 0b00000011;

  if(ps2x.ButtonPressed(PSB_PAD_LEFT)){
    Serial.println("Left Pressed");
//    delay(500);
    buttons[0] = true;
//    Serial.println("Left Pressed");
  }
  if(ps2x.ButtonPressed(PSB_PAD_RIGHT)){
    Serial.println("Right Pressed");
//    delay(500);
    buttons[1] = true;
//    Serial.println("Right Pressed");
  }
  if(ps2x.ButtonPressed(PSB_PAD_UP)){
    Serial.println("Up Pressed");
    buttons[2] = true;
  }
  if(ps2x.ButtonPressed(PSB_PAD_DOWN)){
    Serial.println("Down Pressed");
    buttons[3] = true;
  }
  if(ps2x.ButtonPressed(PSAB_SQUARE)){
    Serial.println("Square Pressed");
    buttons[4] = true;
  }
  if(ps2x.ButtonPressed(PSAB_CIRCLE)){
    Serial.println("Circle Pressed");
    buttons[5] = true;
  }
  if(ps2x.ButtonPressed(PSAB_TRIANGLE)){
    Serial.println("Triangle Pressed");
    buttons[6] = true;
  }
  if(ps2x.ButtonPressed(PSAB_CROSS)){
    Serial.println("Cross Pressed");
    buttons[7] = true;
  }

  unsigned char buttons_byte = BoolsToByte(buttons);

  // Constants to send to robot. These are here because it's sometimes useful to be able to change things during live testing, and much easier to do so on the controller than on the robot itself.
  int measured_angular_velocity_rpm = 720;
  int max_melty_throttle = 1590;
  unsigned char spin1 = 200;
  unsigned char spin2 = 49;
  unsigned char spin3 = 0;
  unsigned char spin4 = 0;
  unsigned char throt1 = 75;
  
  LX = deadband(LX, deadbandWidth);
  LY = deadband(LY, deadbandWidth);
  RX = deadband(RX, deadbandWidth);
  RY = deadband(RY, deadbandWidth);
//  Serial.printf("LX: %d\tLY: %d\tRX: %d\tRY: %d\n", LX, LY, RX, RY);

  //Serial.printf("Buttons = %x or %c\n", buttons);
  
  message[0] = ((unsigned char*) &LX)[0];
  message[1] = ((unsigned char*) &LY)[0];
  message[2] = ((unsigned char*) &RX)[0];
  message[3] = ((unsigned char*) &buttons_byte)[0];
  
  message[4] = ((unsigned char*) &spin1)[0];
  message[5] = ((unsigned char*) &spin2)[0];
  message[6] = ((unsigned char*) &spin3)[0];
  message[7] = ((unsigned char*) &spin4)[0];

  message[8] = ((unsigned char*) &throt1)[0];

  if (timeSinceLastPacketSent >= 10) { //TODO: rm if statement. Useful to change 10 -> 1000 when debugging.
    Serial.println("Attempting to send udp packet");
    timeSinceLastPacketSent = 0;
    Udp.beginPacket(moustache_ip, moustache_port);
    Udp.write(message, sizeof(message));
    Udp.endPacket();
  }

  delay(10);
  timeSinceLastPacketSent += 10;
}

unsigned char deadband(unsigned char input, int deadbandWidth){
  int output;
  if(deadbandWidth > 127){
    Serial.println("Deadband too wide for 8 bit input");
  }
  else if (abs(input - 127) <= deadbandWidth){
    // Input within the deadband
    output = 127;
  }
  else
    output = input;
  return output;
}

unsigned char BoolsToByte(bool b[8])
{
    unsigned char c = 0;
    for (int i=0; i < 8; ++i)
        if (b[i])
            c |= 1 << i;
    return c;
}
