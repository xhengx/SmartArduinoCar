#include <SoftwareSerial.h>
#include <Servo.h>

// 自动壁障行驶
Servo s;
int pos = 0;
int last_pos = 0;
int n = 30;
int longest = -1; //记录最远的距离

typedef enum {LEFT = -1, FORWARD, RIGHT} Direction;

#define Trig A4
#define Echo A3

///

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
  setup_sr04_servo();
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
int auto_flag = 0;
void loop() {
   if (Serial.available() > 0) {
    String str = Serial.readStringUntil('\n');
//    Serial.println("--->" + str);
    String cmd = str.substring(str.indexOf(":") + 1);
    
    if      (cmd == "left") {  auto_flag = 0; move_left();  Serial.println("left");}
    else if (cmd == "right") { auto_flag = 0; move_right(); Serial.println("right");}
    else if (cmd == "from") {  auto_flag = 0; move_front(); Serial.println("front");}
    else if (cmd == "back") {  auto_flag = 0; move_back(); Serial.println("back");}
    else if (cmd == "stop") {  auto_flag = 0; stop(); }
    else if (cmd == "auto") {  auto_flag = 1; }
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
  if (auto_flag) {auto_move();}
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

void auto_move() {
  int dic = direction();

  switch (dic) {
    case LEFT:
//      Serial.println("转左");
      move_left();
      break;
    case FORWARD:
//      Serial.println("前行");
      move_front();
      break;
    case RIGHT:
//      Serial.println("转右");
      move_right();
      break;
  }
}

void setup_sr04_servo() {
//  s.attach(A5); //舵机库和analogWrite共用同一个定时器,所以会产生干扰
   pinMode(A5,OUTPUT);//设定舵机接口为输出接口
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  last_pos = 90;
  servopulse(last_pos);
  //s.write(last_pos);
}

void send_trig_single() {
  digitalWrite(Trig, LOW);
  delayMicroseconds(2);
  digitalWrite(Trig, HIGH);
  delayMicroseconds(10);
  digitalWrite(Trig, LOW);
}

float distance() {
  return pulseIn(Echo, HIGH) / 58.0;
}

int direction() {
  send_trig_single();
  int dis = distance();
  Direction left_foward_right = FORWARD; // -1, 0, 1
  Serial.println(dis);
  if (dis <= 10) {
      Serial.println("Opps, turn");
      longest = dis;
      Serial.println(dis);
      stop();
      //先向左找
      for (pos = last_pos; pos >= 0  ;  pos -= 10) {
        //s.write(pos);
        servopulse(pos);
        delay(15);
        Serial.println(pos);
        send_trig_single();
        int left = distance();
        if (left >= longest) {
          longest = left;
          left_foward_right = LEFT; //像左行
          break;
        }
      }
      last_pos = pos;
      //再向右找
      for (pos = last_pos; pos <= 180  ;  pos += 10) {
//        s.write(pos);
        servopulse(pos); 
        delay(15);

        send_trig_single();
        int right = distance();
        if (right >= longest) {
          longest = right;
          left_foward_right = RIGHT; //向右行
          break;
        }
      }
      last_pos = pos;
      
//      s.write(pos);                  
  }

  return left_foward_right;
}

void servopulse(int angle)//定义一个脉冲函数
{
  int pulsewidth=(angle*11)+500;  //将角度转化为500-2480的脉宽值
  digitalWrite(A5,HIGH);    //将舵机接口电平至高
  delayMicroseconds(pulsewidth);  //延时脉宽值的微秒数
  digitalWrite(A5,LOW);     //将舵机接口电平至低
  delayMicroseconds(20000-pulsewidth);
}

