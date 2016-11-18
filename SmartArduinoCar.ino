#include <SoftwareSerial.h>
#include <Servo.h>
#include <pt.h>


// 自动壁障行驶
Servo s;
int servo_pin = A5; //舵机
int pos = 0;
int last_pos = 0;
int n = 30;
int longest = -1; //记录最远的距离

typedef enum {LEFT = -1, FORWARD, RIGHT, BACK} Direction;

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
int w_right_back0  = 11;  //pwm,避开舵机库
int w_right_back1  = 8;

static struct pt auto_thread, read_thread, dic_thread;
void setup() {
  // put your setup code here, to run once:
  pinMode(LED, OUTPUT);
  setup_WIFI();
  setup_wheel();
  setup_sr04_servo();
  PT_INIT(&auto_thread);
  PT_INIT(&read_thread);
  PT_INIT(&dic_thread);
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
//  Serial.print("AT+CWSAP=\"xxcar1\",\"1234567890\",5,3\r\n"); //设置密码
//  Serial.print("AT+CIPMUX=0\r\n");
//  AT+CIPSTART="UDP","192.168.4.1",8888,8889 //建立UDP监听, 8888为发送端口,8889为接收端口
  //Serial.print("AT+CIPSERVER=1,8080\r\n");
  Serial.print("AT+CIPSTART=\"UDP\",\"192.168.4.1\",8888,8889\r\n");
  Serial.flush();
//  Serial.print("AT+CIPSTO=0\r\n");
//  Serial.print("AT+RST\r\n");
}
int auto_flag = 0;

static int auto_threadIMP(struct pt *pt)
{
  PT_BEGIN(pt);
  while (1)
  {
    PT_WAIT_UNTIL(pt,auto_flag == 1);
    auto_move();
    PT_YIELD(pt); //Check the other events.
  }
  PT_END(pt);
}
static int read_threadIMP(struct pt *pt) 
{
  PT_BEGIN(pt);
  while (1) {
    PT_WAIT_UNTIL(pt, Serial.available());
    String str = Serial.readStringUntil('\n');
//    Serial.println("--->" + str);
    String cmd = str.substring(str.indexOf(":") + 1);
    
    if      (cmd == "left") {  auto_flag = 0; move_left();  Serial.println("left");}
    else if (cmd == "right") { auto_flag = 0; move_right(); Serial.println("right");}
    else if (cmd == "from") {  auto_flag = 0; move_front(); Serial.println("front");}
    else if (cmd == "back") {  auto_flag = 0; move_back();  Serial.println("back");}
    else if (cmd == "stop") {  auto_flag = 0; stop();  }//default_speed(); }
    else if (cmd == "auto") {  auto_flag = 1;       default_speed(); }
    else                    { 
      
      String subCmd = cmd.substring(0, cmd.indexOf(":") );
      if (subCmd == "speed") {
         speed = cmd.substring(cmd.indexOf(":") + 1).toInt();
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
  PT_END(pt);
}
//主循环
void loop() {
  
  read_threadIMP(&read_thread);
  auto_threadIMP(&auto_thread);
  dic_threadIMP(&dic_thread);
 // delay(60);

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
int dic = FORWARD; //方向
static int dic_threadIMP(struct pt *pt)
{
  PT_BEGIN(pt);
  while (1)
  {
    PT_WAIT_UNTIL(pt,auto_flag == 1);
    dic = direction();
    delay(30);
    PT_YIELD(pt); //Check the other events.
  }
  PT_END(pt);
}
void auto_move() {
  
  
  switch (dic) {
    case LEFT:
      Serial.println("move left");
      move_left();
      
      break;
    case FORWARD:
      Serial.println("goooo");
      move_front();
      
      break;
    case RIGHT:
      Serial.println("move right");
      move_right();
      
      break;
    case BACK:
       move_back();
       Serial.println("baccck");
       break;
  }
  delay(100);
  last_pos = 90;
  servo_run(last_pos, servo_pin);
}

void setup_sr04_servo() {
//  s.attach(servo_pin); //舵机库和analogWrite共用同一个定时器,所以会产生干扰,尝试避开9,10接口
  pinMode(A5,OUTPUT);//设定舵机接口为输出接口
  pinMode(Trig, OUTPUT);
  pinMode(Echo, INPUT);
  last_pos = 90;
  servo_run(last_pos, servo_pin);
//  s.write(last_pos);
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
/*
 * 方向检测
 * 先判断是否要后退
 * 然后再判断方向
 * 
 */
int direction() {
  send_trig_single();
  int dis = distance();
  Direction left_foward_right = FORWARD; // -1, 0, 1
  Serial.println(dis);
  if (dis <= 50 || dis >= 1000) { stop(); return BACK; }
  if (dis <= 100) {
      Serial.println("Opps, turn");
      longest = dis;
      Serial.println(dis);
      stop();
      //先向右找
      servo_run(45, servo_pin);
      send_trig_single();
      int right = distance();

      delay(15);

      servo_run(90 + 40, servo_pin);
      send_trig_single();
      int left = distance();

      delay(15);
      left_foward_right = right > left ? LEFT : RIGHT;

      if (max(right, left) <= 50) {
        left_foward_right = BACK;
      }
      
              
  }

  return left_foward_right;
}

void servo_run(int ang, int pin) {
  for (int i = 0; i < 50; ++i) {
    servopulse(ang, pin);
  }
  delay(30);
}

void servopulse(int angle, int pin)//定义一个脉冲函数
{
  int pulsewidth=(angle*11)+500;  //将角度转化为500-2480的脉宽值
  digitalWrite(pin,HIGH);    //将舵机接口电平至高
  delayMicroseconds(pulsewidth);  //延时脉宽值的微秒数
  digitalWrite(pin,LOW);     //将舵机接口电平至低
//  delayMicroseconds(20000-pulsewidth);
  delay(20-pulsewidth/1000);
}

