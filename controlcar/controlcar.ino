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
<html>
<head>
    <title>
        CONTROLLER
    </title>
    <meta name="viewport" content="user-scalable=no">
    
</head>
<body  style="position: fixed; font-family: 'Gill Sans', 'Gill Sans MT', Calibri, 'Trebuchet MS', sans-serif ;
color:rgb(128, 128, 128);
font-size: xx-large;">
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

        function send(x,y,speed,angle){
            var xhr = new XMLHttpRequest();
      xhr.open("GET", "/slider?value="+x+","+y+","+speed, true);
      xhr.send();
        }


    </script>
    <script>
        var canvas, ctx;

        window.addEventListener('load', () => {

            canvas = document.getElementById('canvas');
            ctx = canvas.getContext('2d');          
            resize(); 

            document.addEventListener('mousedown', startDrawing);
            document.addEventListener('mouseup', stopDrawing);
            document.addEventListener('mousemove', Draw);

            document.addEventListener('touchstart', startDrawing);
            document.addEventListener('touchend', stopDrawing);
            document.addEventListener('touchcancel', stopDrawing);
            document.addEventListener('touchmove', Draw);
            window.addEventListener('resize', resize);

            document.getElementById("x_coordinate").innerText = 0;
            document.getElementById("y_coordinate").innerText = 0;
            document.getElementById("speed").innerText = 0;
            document.getElementById("angle").innerText = 0;
        });

      


        var width, height, radius, x_orig, y_orig;
        function resize() {
            width = window.innerWidth;
            radius = 50;
            height = radius * 6.5;
            ctx.canvas.width = width;
            ctx.canvas.height = height;
            background();
            joystick(width / 2, height / 3);
        }

        function background() {
            x_orig = width / 2;
            y_orig = height / 3;

            ctx.beginPath();
            ctx.arc(x_orig, y_orig, radius + 20, 0, Math.PI * 2, true);
            ctx.fillStyle = '#ECE5E5';
            ctx.fill();
        }

        function joystick(width, height) {
            ctx.beginPath();
            ctx.arc(width, height, radius, 0, Math.PI * 2, true);
            ctx.fillStyle = '#F08080';
            ctx.fill();
            ctx.strokeStyle = '#F6ABAB';
            ctx.lineWidth = 8;
            ctx.stroke();
        }

        let coord = { x: 0, y: 0 };
        let paint = false;

        function getPosition(event) {
            var mouse_x = event.clientX || event.touches[0].clientX;
            var mouse_y = event.clientY || event.touches[0].clientY;
            coord.x = mouse_x - canvas.offsetLeft;
            coord.y = mouse_y - canvas.offsetTop;
        }

        function is_it_in_the_circle() {
            var current_radius = Math.sqrt(Math.pow(coord.x - x_orig, 2) + Math.pow(coord.y - y_orig, 2));
            if (radius >= current_radius) return true
            else return false
        }


        function startDrawing(event) {
            paint = true;
            getPosition(event);
            if (is_it_in_the_circle()) {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                background();
                joystick(coord.x, coord.y);
                Draw();
            }
        }


        function stopDrawing() {
            paint = false;
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            background();
            joystick(width / 2, height / 3);
            document.getElementById("x_coordinate").innerText = 0;
            document.getElementById("y_coordinate").innerText = 0;
            document.getElementById("speed").innerText = 0;
            document.getElementById("angle").innerText = 0;
            send( 0,0,0,0);
        }

        function Draw(event) {

            if (paint) {
                ctx.clearRect(0, 0, canvas.width, canvas.height);
                background();
                var angle_in_degrees,x, y, speed;
                var angle = Math.atan2((coord.y - y_orig), (coord.x - x_orig));

                if (Math.sign(angle) == -1) {
                    angle_in_degrees = Math.round(-angle * 180 / Math.PI);
                }
                else {
                    angle_in_degrees =Math.round( 360 - angle * 180 / Math.PI);
                }


                if (is_it_in_the_circle()) {
                    joystick(coord.x, coord.y);
                    x = coord.x;
                    y = coord.y;
                }
                else {
                    x = radius * Math.cos(angle) + x_orig;
                    y = radius * Math.sin(angle) + y_orig;
                    joystick(x, y);
                }

            
                getPosition(event);

                var speed =  Math.round(100 * Math.sqrt(Math.pow(x - x_orig, 2) + Math.pow(y - y_orig, 2)) / radius);

                var x_relative = Math.round(x - x_orig);
                var y_relative = Math.round(y - y_orig);
                

                document.getElementById("x_coordinate").innerText =  x_relative;
                document.getElementById("y_coordinate").innerText =y_relative*-1;
                document.getElementById("speed").innerText = speed;
                document.getElementById("angle").innerText = angle_in_degrees;

                send( x_relative,y_relative*-1,speed,angle_in_degrees);
            }
        } 
    </script>
    
</body>
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
