#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>

// Replace with your network credentials
const char* ssid = "AndroidAP9FF1";
const char* password = "edmund12";

String sliderValue = "0";

// setting PWM properties
const int freq = 30000;
const int resolution = 8;
const int ledChannel = 0;
const int ledChannel2 = 1;
const char* PARAM_INPUT = "value";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
 <div id="main">
      <div id="wrapper">
      </div>
    </div>
</head>
<body  style="* {
  border: 0;
  box-sizing: border-box;
  margin: 0;
  padding: 0;
}

#main {
  align-items: center;
  display: flex;
  height: 100vh;
  justify-content: center;
  margin: auto;
  width: 100vw;
}

#wrapper {
  border: 1px solid black;
  width: 300px;
  height: 300px;
}

.joystick {
  background-color: blue;
  border-radius: 100%;
  cursor: pointer;
  height: 100%;
  user-select: none;
  width: 100%;
}
">
    <h1 style="text-align:center">
        controller </h1>
    <p style="text-align: center;">
        X: <span id="x_coordinate"> </span>
        Y: <span id="y_coordinate"> </span>
        Speed: <span id="speed"> </span> %
        Angle: <span id="angle"> </span>
    </p>
    <canvas id="canvas" name="game"></canvas>
    
    <script>
const joystick = createJoystick(document.getElementById('wrapper'));

// setInterval(() => console.log(joystick.getPosition()), 16);

function createJoystick(parent) {
  const maxDiff = 100;
  const stick = document.createElement('div');
  stick.classList.add('joystick');

  stick.addEventListener('mousedown', handleMouseDown);
  document.addEventListener('mousemove', handleMouseMove);
  document.addEventListener('mouseup', handleMouseUp);
  stick.addEventListener('touchstart', handleMouseDown);
  document.addEventListener('touchmove', handleMouseMove);
  document.addEventListener('touchend', handleMouseUp);

  let dragStart = null;
  let currentPos = { x: 0, y: 0 };

  function handleMouseDown(event) {
    stick.style.transition = '0s';
    if (event.changedTouches) {
      dragStart = {
        x: event.changedTouches[0].clientX,
        y: event.changedTouches[0].clientY,
      };
      return;
    }
    dragStart = {
      x: event.clientX,
      y: event.clientY,
    };

  }

  function handleMouseMove(event) {
    if (dragStart === null) return;
    event.preventDefault();
    if (event.changedTouches) {
      event.clientX = event.changedTouches[0].clientX;
      event.clientY = event.changedTouches[0].clientY;
    }
    const xDiff = event.clientX - dragStart.x;
    const yDiff = event.clientY - dragStart.y;
    const angle = Math.atan2(yDiff, xDiff);
		const distance = Math.min(maxDiff, Math.hypot(xDiff, yDiff));
		const xNew = distance * Math.cos(angle);
		const yNew = distance * Math.sin(angle);
    stick.style.transform = `translate3d(${xNew}px, ${yNew}px, 0px)`;
    currentPos = { x: xNew, y: yNew };
  }

  function handleMouseUp(event) {
    if (dragStart === null) return;
    stick.style.transition = '.2s';
    stick.style.transform = `translate3d(0px, 0px, 0px)`;
    dragStart = null;
    currentPos = { x: 0, y: 0 };
  }

  parent.appendChild(stick);
  return {
    getPosition: () => currentPos,
  };
}
</html>
)rawliteral";

// motor setup pin outs
// motor b right
#define input1 13 // in4
#define input2 12 // in3
#define speed1 14 // enb


// motor a left
#define input3  18 //in1
#define input4  19 //in2
#define speed2  21 //ena

int right_speed =0; // right and left speed of the various motors
int left_speed =0;
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  // setup pin modes and pwm channels
  pinMode(input1,OUTPUT);
  pinMode(input2,OUTPUT);
  pinMode(speed1, OUTPUT);
  pinMode(speed2, OUTPUT);
  pinMode(input3,OUTPUT);
  pinMode(input4,OUTPUT);
  pinMode(2,OUTPUT);
  ledcSetup(ledChannel,freq,resolution);
  ledcAttachPin(speed2,ledChannel);
  ledcSetup(ledChannel2,freq,resolution);
  ledcAttachPin(speed1,ledChannel2);
  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    digitalWrite(2, HIGH);
    Serial.println("Connecting to WiFi..");
  }
  digitalWrite(2, LOW);

  // Print ESP Local IP Address
  Serial.println(WiFi.localIP());

  // Route for root / web page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html);
  });

  // Send a GET request to <ESP_IP>/slider?value=<inputMessage>
  server.on("/slider", HTTP_GET, [] (AsyncWebServerRequest *request) {
    String inputMessage;
    // GET input1 value on <ESP_IP>/slider?value=<inputMessage>
    if (request->hasParam(PARAM_INPUT)) {
      inputMessage = request->getParam(PARAM_INPUT)->value();
      sliderValue = inputMessage;
      
      setspeeds();
      Serial.print("left speed: ");
      Serial.println(left_speed);
      Serial.print("right speed: ");
      Serial.println(right_speed);
      ledcWrite(ledChannel, right_speed);
      ledcWrite(ledChannel2, left_speed);
    }else {
      inputMessage = "No message sent";
    }
    //Serial.println(inputMessage);
    request->send(200, "text/plain", "OK");
  });
  server.begin();
  //motor setup
}  
void loop() {
}
// control the direction of the robot
void forward(int front,int back){
  digitalWrite(front,HIGH);
  digitalWrite(back,LOW);
}
void back(int front, int back){
  digitalWrite(front,LOW);
  digitalWrite(back,HIGH);
}
// stop the rotation of wheels
void stopp(int front,int back){
   digitalWrite(front,LOW);
  digitalWrite(back,LOW); 
}
void setspeeds()
{
  // read the values received from web page
  int comma1 = sliderValue.indexOf(',');
  String part2 = sliderValue.substring(comma1+1);
  int comma2 = part2.indexOf(',');
  int x=sliderValue.substring(0,comma1).toInt();
  int y=part2.substring(0,comma2).toInt();
  int speeed=part2.substring(comma2+1).toInt();
  //Serial.println("xpo:"+sliderValue.substring(0,comma1));
  //Serial.println("ypo:"+part2.substring(0,comma2));
  
   // move back when joystick moves back
  if(y<0)
  {
    back(input1,input2);
    back(input3,input4);
  }
  // move foward when joystick moves forward
  else if(y>0)
  {
    forward(input1,input2);
    forward(input3,input4);
  }
  if(sliderValue.indexOf("0,0,0")>=0){
    stopp(input1,input2);
    stopp(input3,input4);
    speeed = 0;
  }
  // control the direction of the robot by varyuing the speed of the left and right wheels
  if(x!= 0 && y!= 0)
  {
    if (y>0)
    {
      if(x>0){
      //turn front right
        right_speed =200+ abs(y)*55/100;
        left_speed = 200+(abs(y)+abs(x))*55/100;
        
      }
      else if(x<0){
      // turn front left
        left_speed =200+ abs(y)*55/100;
        right_speed = 200+(abs(y)+abs(x))*55/100;      
               
      }
    }
    if(y<0)    
    {
      if(x>0)
      {
      //turn back right
        right_speed =200+ abs(y)*55/100;
        left_speed = 200+(abs(y)+abs(x))*55/100;
     
      }
      else if(x<0)
      {
      // turn back left
        left_speed =200+ abs(y)*55/100;
        right_speed = 200+(abs(y)+abs(x))*55/100;  
     
      }    
    }
  }
  else if(x==0 && y!=0)
  {
  //straight movement    
    if(y>0 || y<0)
    {
      left_speed =200+ abs(y)*55/100;
      right_speed =200+ abs(y)*55/100;     
    }        
  }
  else
  {
      
    if(x>0)
    {
      //hard right
      right_speed = 0;
      left_speed = 200+(abs(y)+abs(x))*55/100;  
    }
    else if(x<0)
    {
      //hard left
      left_speed = 0;
      right_speed = 200+(abs(y)+abs(x))*55/100;      
    }
        
  }  
 }