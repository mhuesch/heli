#define LED 12
#define statusA 8
#define statusB 9
bool finished;
bool handshakeComplete;
byte inputbufferA[5];
byte inputbufferB[5];
int incomingByteHeader = 0;
int heli_id = 0;
static byte dataPointer, maskPointer;
static const byte mask[] = {1, 2, 4, 8, 16, 32, 64, 128};
static byte data[4];

void setup() {
	pinMode(LED, OUTPUT);
        pinMode(statusA, OUTPUT);
        pinMode(statusB, OUTPUT);
	digitalWrite(LED, HIGH);
        digitalWrite(statusA, LOW);
        digitalWrite(statusB, LOW);
	finished = false;
	handshakeComplete = false;
	Serial.begin(9600);
        
	inputbufferA[0] = byte(128);
	inputbufferA[1] = byte(63);
	inputbufferA[2] = byte(63);
	inputbufferA[3] = byte(0);
	inputbufferA[4] = byte(63);
	inputbufferB[0] = byte(128);
	inputbufferB[1] = byte(63);
	inputbufferB[2] = byte(63);
	inputbufferB[3] = byte(0);
	inputbufferB[4] = byte(63);

}

//sends 38Khz pulse when using a 16Mhz ic
void sendPulse(long us) {
	for(int i = 0; i < (us / 26) - 1; i++) {
		 digitalWrite(LED, HIGH);
		 delayMicroseconds(10);
		 digitalWrite(LED, LOW);
		 delayMicroseconds(10);
	}
}

void sendHeader() {
	sendPulse(2002);
	delayMicroseconds(1998);
}

void sendFooter() {
	sendPulse(312);
	delay(60);
}

void sendControlPacket(byte channel, byte yaw, byte pitch, byte throttle, byte trim) {

    data[0] = yaw;
	data[1] = pitch;
	data[2] = throttle;
	data[3] = trim;
	dataPointer = 4;
	maskPointer = 8;

	if (channel == 0x80) {
        	data[2] = data[2] + 128;
        }
 
	sendHeader();

	while(dataPointer > 0) {
		sendPulse(312);	
		if(data[4 - dataPointer] & mask[--maskPointer]) {
			delayMicroseconds(712);
		} else {
			delayMicroseconds(312);
		}
		if(!maskPointer) {
			maskPointer = 8;
			dataPointer--;
		}
	}

	sendFooter();
}

byte readThreeChars(){
  String cmd_val;
  char inChar = -1;
  byte index = 0;
  while (Serial.available() > 0){
    if(index < 4){
      inChar = Serial.read();
      cmd_val += (char)inChar;
      index++;
    }
  }
  byte rtn_val = cmd_val.toInt();
  return rtn_val;
}

void loop() {
  
  if (Serial.available() > 0) {
    Serial.println("New loop");
    
    //Determine whether this is a command to heli A, heli B, or a sync command
    heli_id = (Serial.read() - '0');
    
    if (heli_id == 0) {
      //Helicopter 1
      Serial.println("Helicopter A command");
      incomingByteHeader = (Serial.read() - '0');
      switch(incomingByteHeader) {
        case(0):inputbufferA[0] = 0;break; //channel
        case(1):inputbufferA[1] = readThreeChars();Serial.print("changed heli A yaw value to: ");Serial.println(inputbufferA[1]);break; //yaw
        //case(1):inputbufferA[1] = (Serial.read()-'0');Serial.print("changed heli A yaw value to: ");Serial.println(inputbufferA[1]);break; //yaw
        case(2):inputbufferA[2] = readThreeChars();Serial.print("changed heli A pitch value to: ");Serial.println(inputbufferA[2]);break; //pitch
        case(3):inputbufferA[3] = readThreeChars();Serial.print("changed heli A throttle value to: ");Serial.println(inputbufferA[3]);break; //throttle
        case(4):inputbufferA[4] = readThreeChars();Serial.print("changed heli A trim value to: ");Serial.println(inputbufferA[4]);break; //trim
        default:break;
      }
    }
    if (heli_id == 1) {
      // Helicopter 2
      Serial.println(" Helicopter B command");
      incomingByteHeader = (Serial.read() - '0');
    
      switch(incomingByteHeader) {
        case(0):inputbufferB[0] = 128;break; //channel
        case(1):inputbufferB[1] = readThreeChars();Serial.print("changed heli B yaw value to: ");Serial.println(inputbufferB[1]);break; //yaw
        case(2):inputbufferB[2] = readThreeChars();Serial.print("changed heli B pitch value to: ");Serial.println(inputbufferB[2]);break;//pitch
        case(3):inputbufferB[3] = readThreeChars();Serial.print("changed heli B throttle value to: ");Serial.println(inputbufferB[3]);break;//throttle
        case(4):inputbufferB[4] = readThreeChars();Serial.print("changed heli B trim value to: ");Serial.println(inputbufferB[4]);break;//trim
        default:break;
      }
    }
    if (heli_id == 2) {
      Serial.print("SYNC");
      incomingByteHeader = (Serial.read() - '0');
 
      if (incomingByteHeader == 0) {
        Serial.println("heli A!");
        unsigned long timenow = millis();
        while (millis() - timenow <= 10000) {
          digitalWrite(statusA, HIGH);
          sendControlPacket(inputbufferA[0],inputbufferA[1],inputbufferA[2],inputbufferA[3],inputbufferA[4]);
        }
      }
      if (incomingByteHeader == 1) {
        Serial.println(" heli B!");
        unsigned long timenow = millis();
        while (millis() - timenow <= 10000) {
          digitalWrite(statusB, HIGH);
          sendControlPacket(inputbufferB[0],inputbufferB[1],inputbufferB[2],inputbufferB[3],inputbufferB[4]);
        }
      }
      digitalWrite(statusA, LOW);
      digitalWrite(statusB, LOW);
    }  
  }
  
  //Throw away the linefeed
  char linefeed = Serial.read();
  
  sendControlPacket(inputbufferA[0],inputbufferA[1],inputbufferA[2],inputbufferA[3],inputbufferA[4]);
  //sendControlPacket(inputbufferB[0],inputbufferB[1],inputbufferB[2],inputbufferB[3],inputbufferB[4]);
}
