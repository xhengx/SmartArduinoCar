#include <SoftwareSerial.h>

//SoftwareSerial esp(13, 12); //Rx, Tx

#define DEFAULT_SPEED 150
#define MAX_SPEED     250
#define MIN_SPEED     100
#define STOP_SPEED    0

int LED = 13;
int speed = DEFAULT_SPEED; //记录当前速度
int is_turn = 0; //记录是否转向
int is_back = 0; //记录是否向后
//轮子
int  w_left_from0  = 3;  //pwm
int  w_left_from1  = 2;
int w_right_from0  = 5;  //pwm
int w_right_from1  = 4;
int  w_left_back0  = 6;  //pwm
int  w_left_back1  = 7;
int w_right_back0  = 9;  //pwm
int w_right_back1  = 8;

void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  setup_WIFI();
  setup_wheel();
  delay(1000);
}

//初始化轮子
void setup_wheel() {
  pinMode(w_left_from0, OUTPUT);
  pinMode(w_left_from1, OUTPUT);
  
  pinMode(w_right_from0, OUTPUT);
  pinMode(w_right_from1, OUTPUT);
  
  pinMode(w_left_back0, OUTPUT);
  pinMode(w_left_back1, OUTPUT);
  
  pinMode(w_right_back0, OUTPUT);
  pinMode(w_right_back1, OUTPUT);

  
//  min_speed();
}
//速度相关
void default_speed() {
  change_speed(w_left_from0, DEFAULT_SPEED);
  change_speed(w_right_from0, DEFAULT_SPEED);
  change_speed(w_left_back0, DEFAULT_SPEED);
  change_speed(w_right_back0, DEFAULT_SPEED);

  speed = DEFAULT_SPEED; //记录当前速度
}
//全速前进
void full_speed() {
  change_speed(w_left_from0, MAX_SPEED);
  change_speed(w_right_from0, MAX_SPEED);
  change_speed(w_left_back0, MAX_SPEED);
  change_speed(w_right_back0, MAX_SPEED);

  speed = MAX_SPEED; //记录当前速度
}
void stop() {
  if (is_back) { _move_front(); } //纠正当前的输出
  change_speed(w_left_from0, STOP_SPEED);
  change_speed(w_right_from0, STOP_SPEED);
  change_speed(w_left_back0, STOP_SPEED);
  change_speed(w_right_back0, STOP_SPEED);

  speed = STOP_SPEED;
}
//低速前进
void min_speed() {
  change_speed(w_left_from0, MIN_SPEED);
  change_speed(w_right_from0, MIN_SPEED);
  change_speed(w_left_back0, MIN_SPEED);
  change_speed(w_right_back0, MIN_SPEED);

  speed = MIN_SPEED; //记录当前速度
}
//设定为当前速度
void setup_current_speed() {
  change_speed(w_left_from0, speed);
  change_speed(w_right_from0, speed);
  change_speed(w_left_back0, speed);
  change_speed(w_right_back0, speed);
}
//改变速度
void change_speed(int pin, int speed) {
  analogWrite(pin, speed);
}
//设置WIFI选项
void setup_WIFI() {
  Serial.begin(115200);
  Serial.setTimeout(300);
  //Serial.print("AT+CWSAP=\"xxcar\",\"1234567890\",5,3\r\n"); //设置密码
  //Serial.print("AT+CIPMUX=0\r\n");
  //AT+CIPSTART="UDP","192.168.4.1",8888,8889 //建立UDP监听, 8888为发送端口,8889为接收端口
  //Serial.print("AT+CIPSERVER=1,8080\r\n");
  Serial.print("AT+CIPSTART=\"UDP\",\"192.168.4.1\",8888,8889\r\n");
  Serial.flush();
//  Serial.print("AT+CIPSTO=0\r\n");
//  Serial.print("AT+RST\r\n");
}
//主循环
void loop() {
   if (Serial.available() > 0) {
    String str = Serial.readStringUntil('\n');
//    Serial.println("--->" + str);
    String cmd = str.substring(str.indexOf(":") + 1);
    
    if      (cmd == "left") {  move_left();  Serial.println("left");}
    else if (cmd == "right") { move_right(); Serial.println("right");}
    else if (cmd == "from") {  move_front(); Serial.println("front");}
    else if (cmd == "back") {  move_back(); Serial.println("back");}
    else if (cmd == "stop") { stop(); }
    else                    { 
      
      String subCmd = cmd.substring(0, cmd.indexOf(":") );
      if (subCmd == "speed") {
         speed = cmd.substring(cmd.indexOf(":") + 1).toInt();
         Serial.println(speed);
          setup_current_speed();
         
      }
    }
    
    if (cmd == "on") {
         digitalWrite(LED, HIGH);
         Serial.println("AT");
     }
     else if (cmd == "off") {
        digitalWrite(LED, LOW);
     }
  }
}
//方向控制
void move_left() { 
  _move_left();
  is_turn = 1;
}
void move_right() {
  _move_right();
  is_turn = 1;
}
void move_back() {
  if (is_turn) {
    setup_current_speed();
    is_turn = 0;
  }
  _move_back();
  is_back = 1;
}
void move_front() {
  if (is_turn) {
    setup_current_speed();
    is_turn = 0;
  }
  _move_front();
  is_back = 0;
}

void _move_right() {
  change_speed(w_left_from0, MAX_SPEED);
  change_speed(w_left_back0, MAX_SPEED);

  change_speed(w_right_from0, MIN_SPEED);
  change_speed(w_right_back0, MIN_SPEED);
}
void _move_left() {

  change_speed(w_left_from0, MIN_SPEED);
  change_speed(w_left_back0, MIN_SPEED);

  change_speed(w_right_from0, MAX_SPEED);
  change_speed(w_right_back0, MAX_SPEED);
  
}
void _move_back() {
  digitalWrite(w_left_from0, LOW);
  digitalWrite(w_left_from1, HIGH);
  digitalWrite(w_right_from0, LOW);
  digitalWrite(w_right_from1, HIGH);
  digitalWrite(w_left_back0, LOW);
  digitalWrite(w_left_back1, HIGH);
  digitalWrite(w_right_back0, LOW);
  digitalWrite(w_right_back1, HIGH);
}
void _move_front() {
  digitalWrite(w_left_from0, HIGH);
  digitalWrite(w_left_from1, LOW);
  digitalWrite(w_right_from0, HIGH);
  digitalWrite(w_right_from1, LOW);
  digitalWrite(w_left_back0, HIGH);
  digitalWrite(w_left_back1, LOW);
  digitalWrite(w_right_back0, HIGH);
  digitalWrite(w_right_back1, LOW);
}


