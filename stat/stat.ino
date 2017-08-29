#include <SoftwareSerial.h>
//include?? R1 100K C1 33n
#include <stdlib.h>
#include <math.h>
#include <stdlib.h>

#define _baudrate   9600
#define _rxpin      4
#define _txpin      8
SoftwareSerial debug( _rxpin, _txpin ); // RX, TX
#define SSID "andy88980" //WIFI SSID_______________andy88980
#define PASS "asdf8520" //WIFI PASSWORD____________asdf8520
#define IP "163.18.57.43"

String GET = "GET /qrcode/newtotalplug.php?name=001&voltage=";
//String GET = "GET /qrcode/totalplug.php?name=001&voltage=1000&current=2000&angle=3000&s=4000&power=5000&vx=6000";
//String GET2 = "GET /qrcode/putplugdata.php?name=001";
String GET3 = "GET /qrcode/wifiplugend.php?name=002";
String GET4 = "GET /qrcode/wifiplugstart.php?name=002";

const int vgs1 = 9;//OC1A
const int vgs2 = 10;//OC1B
const int vgs3 = 5;//OC0B
const int vgs4 = 3;//OC2B
/*int v1=0, v3, temp,a=0,b=0,s=0;
  volatile int old_OCR2B, old_OCR0B;
  bool vgs34_duty_stay = true;*/
/*float v1=0, v2=0, v3=0, v4=0, v5=0, v6=0, v7=0, v8=0, v9=0, v10=0, v11=0, v12=0, v13=0, v14=0, v15=0, v16=0, v17=0, v18=0, v19=0, v20=0;
  float i1=0, i2=0, i3=0, i4=0, i5=0, i6=0, i7=0, i8=0, i9=0, i10=0, i11=0, i12=0, i13=0, i14=0, i15=0, i16=0, i17=0, i18=0, i19=0, i20=0;
  float p1=0, p2=0, p3=0, p4=0, p5=0, p6=0, p7=0, p8=0, p9=0, p10=0, p11=0, p12=0, p13=0, p14=0, p15=0, p16=0, p17=0, p18=0, p19=0, p20=0;*/
float par[25] = {0};
float var[25] = {0};
float iar[25] = {0};
float pavgar[4] = {0};
float vrmsar[4] = {0};
float irmsar[4] = {0};
float vrms = 0, irms = 0, pavg = 0;
int j = 0, z = 0, sta = 0;
float rms(float ar[25]);
float avg(float ar[25]);
int phase = 0;
float vfb = 0;
int re = 0, count = 0, first = 1;
float S = 0;

void setup() {
  pinMode(vgs1, OUTPUT);
  pinMode(vgs2, OUTPUT);
  pinMode(vgs3, OUTPUT);
  pinMode(vgs4, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);

  //Serial.begin(9600);
  Serial.begin( _baudrate );
  delay(1000);
  debug.begin( _baudrate );
  //sendDebug("AT+RST");
  //delay(2000);
  sendDebug("AT");
  Loding("sent AT");
  connectWiFi();
  pinMode(7, OUTPUT);

  TCCR1A = _BV(COM1A1) | _BV(WGM11) | _BV(COM1B1) | _BV(COM1B0);//OC1A clear when match ,OC1B set when match
  TCCR1B = _BV(CS10) | _BV(WGM13);//Set prescaler at 1 ,timer1 clk is 16MHz,fast pwm mode(Top on ICR1)
  ICR1 = 5333;//set timer1 top ->1.5kHz
  OCR1A = 100;//init set duty at 50% ,vgs1;
  OCR1B = 100;//init set duty at 50% ,vgs2;
  TIMSK1 = 0b01 ;//interrupt  timer1 overflow enable

  /*
    TCCR0A = _BV(COM0B1) | _BV(COM0B0) | _BV(WGM00);//OC0B set when match with TCNT2  ,set OC0A at top,
    TCCR0B = _BV(WGM02) | _BV(CS00);//set timer0 clk at 16Mhz
    OCR0A = 200;//Top of timer0
    OCR0B = 0;//init set duty at 50%,vgs3;//　互補


    TCCR2A = _BV(COM2B1) | _BV(WGM20);//set OC2A is top.  OC2B clear when match with TCNT2
    TCCR2B = _BV(WGM22) | _BV(CS20);//set timer2 clk at 16Mhz
    OCR2A = 200;//Top of timer2
    OCR2B = 0;//init set duty at 50%,vgs4;
  */

  ADCSRA &= 0xf8;
  ADCSRA |= 0x04;
  TCNT0 = TCNT1 = TCNT2 = 0;

  //Serial.begin(115200);//Timer.h
}
void loop()
{
  if (sta == 0)
  {
    if (re == 1)
    {
      sta = 1;
    }
    vrmsar[z % 4] = rms(var);
    irmsar[z % 4] = rms(iar);
    pavgar[z % 4] = avg(par);
    vrms = (vrmsar[0] + vrmsar[1] + vrmsar[2] + vrmsar[3]) / 4;
    irms = (irmsar[0] + irmsar[1] + irmsar[2] + irmsar[3]) / 4;
    pavg = (pavgar[0] + pavgar[1] + pavgar[2] + pavgar[3]) / 4;
    S = vrms * irms;
    //OCR2B = pavg * 0.2;
    //OCR0B = 201 - irms * 20;
    if (z == 11)
    {
      z = 0;
      re = 1;
      //sta = 1;
      TIMSK1 = 0b00;
    }
  } else {
    delay(500);   // 60 second
    SentOnCloud();
    delay(500);
    //SentOnCloud2();
    //delay(200);
    sta = 0;
    re = 0; //new
    TIMSK1 = 0b01;
    first = 0;
  }

}

//

ISR(TIMER1_OVF_vect)
{
  j++;
  if (j == 25)
  {
    j = 0;
    z++;
  }
  phase = analogRead(A2);
  if (phase < 600)
  {
    vfb = -1;
  }
  else if (phase > 300)
  {
    vfb = 1;
  }
  var[j] = analogRead(A0) * vfb * 0.254; //  1/1024*5*52
  //var[j] = (analogRead(A0)-512)*0.00488; // /1024*5*1
  //iar[j] = analogRead(A1);
  iar[j] = (analogRead(A1) - 512) * 0.0267; //  1/1024*5*2.5*  (2*1.1)
  par[j] = var[j] * iar[j];
  //digitalWrite(2, j % 2);

}
float rms(float ar[25])
{
  float a;
  a = (ar[0] * ar[0] + ar[1] * ar[1] + ar[2] * ar[2] + ar[3] * ar[3] + ar[4] * ar[4] + ar[5] * ar[5] + ar[6] * ar[6] + ar[7] * ar[7] + ar[8] * ar[8] + ar[9] * ar[9] + ar[10] * ar[10] + ar[11] * ar[11] + ar[12] * ar[12] + ar[13] * ar[13] + ar[14] * ar[14] + ar[15] * ar[15] + ar[16] * ar[16] + ar[17] * ar[17] + ar[18] * ar[18] + ar[19] * ar[19] + ar[20] * ar[20] + ar[21] * ar[21] + ar[22] * ar[22] + ar[23] * ar[23] + ar[24] * ar[24]) / 25;
  a = sqrt(a);
  return a;
}
float avg(float ar[25])
{
  float a;
  a = (ar[0] + ar[1] + ar[2] + ar[3] + ar[4] + ar[5] + ar[6] + ar[7] + ar[8] + ar[9] + ar[10] + ar[11] + ar[12] + ar[13] + ar[14] + ar[15] + ar[16] + ar[17] + ar[18] + ar[19] + ar[20] + ar[21] + ar[22] + ar[23] + ar[24]) / 25;
  return a;
}
//////////////////////////////////////////////////////wifi part
boolean connectWiFi()
{
  debug.println("AT+CWMODE=1");
  delay(3000);
  Wifi_connect();
}
//////////////////////////////////////////////////////read plugstatus
void SentOnCloud()
{
  int d[66];
  // 設定 ESP8266 作為 Client 端
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  sendDebug(cmd);
  if ( debug.find( "Error" ) )
  {
    Serial.print( "RECEIVED: Error\nExit1" );
    //Wifi_connect();
    return;
  }
  cmd = GET + String(vrms) + "&current=" + String(irms) + "&power=" + String(pavg) + "&s=" + String(S) + "&PF=" + String(pavg / S) + "&first=" + String(first) + "\r\n";
  debug.print( "AT+CIPSEND=" );
  debug.println( cmd.length() );
  delay(200);
  int i = 0;
  if (debug.find( ">" ) )
  {
    Serial.print(">");
    Serial.print(cmd);
    debug.print(cmd);
    Serial.print("get data from php: ");
    //Serial.println(debug.readString());
    if (debug.find("3:")) {
      Serial.println(debug.read());
      d[0] = debug.read();
      Serial.println(d[0]);
      d[1] = debug.read();
      Serial.println(d[1]);
      d[2] = debug.read();
      Serial.println(d[2]);
      debug.print( "AT+CIPCLOSE" );
      if (d[2] == 49) {
        Serial.println("Schedule on");
        if (d[0] == 49) {
          digitalWrite(7, HIGH);
          delay(200);
          schedule(0);
          Serial.println("Schedule 0");
        } else {
          digitalWrite(7, LOW);
          delay(200);
          schedule(1);
          Serial.println("Schedule 1");
        }
      } else {
        Serial.println("Schedule off");
        if (d[0] == 49) {
          Serial.println("HIGH");
          digitalWrite(7, HIGH);
        } else {
          Serial.println("LOW");
          digitalWrite(7, LOW);
        }
      }
      int p;
      for (p = 0; p < 3; p++) {
        d[p] = NULL;
      }
    }

  }
  else
  {
    debug.print( "AT+CIPCLOSE" );
  }
  if ( debug.find("OK") )
  {
    Serial.println( "RECEIVED 2: OK" );
    //sta = 0;
    //re = 0; //new
    //TIMSK1 = 0b01;
  }
  else
  {
    Serial.println( "RECEIVED: Error\nExit2" );
    digitalWrite(7,digitalRead(7));
    count++;
    if (count > 2) {
      sendDebug("AT");
      delay(2000);
      Loding("sent AT");
      Wifi_connect();
      count = 0;
    }
  }
}
//////////////////////////////////////////////////////send plugdata

void schedule(int a) {
  String cmd = "AT+CIPSTART=\"TCP\",\"";
  cmd += IP;
  cmd += "\",80";
  sendDebug(cmd);
  if ( debug.find( "Error" ) )
  {
    Serial.print( "SCHEDULE RECEIVED: Error\nExit1" );
    return;
  }
  if (a == 0) {
    cmd = GET3 + "\r\n";
  } else {
    cmd = GET4 + "\r\n";
  }
  debug.print( "AT+CIPSEND=" );
  debug.println( cmd.length() );
  int i = 0;
  if (debug.find( ">" ) )
  {
    Serial.print(">");
    Serial.print(cmd);
    debug.print(cmd);
    debug.print( "AT+CIPCLOSE" );
  }
  else
  {
    debug.print( "AT+CIPCLOSE" );
  }
  if ( debug.find("OK") )
  {
    Serial.println( "RECEIVED second 2: OK" );
  }
  else
  {
    Serial.println( "SCHEDULE RECEIVED: Error\nExit2" );
  }
}

void Wifi_connect()
{
  String cmd = "AT+CWJAP=\"";
  cmd += SSID;
  cmd += "\",\"";
  cmd += PASS;
  cmd += "\"";
  sendDebug(cmd);
  Loding("Wifi_connect");
}
void Loding(String state) {
  for (int timeout = 0 ; timeout < 10 ; timeout++)
  {
    if (debug.find("OK"))
    {
      Serial.println("RECEIVED: OK");
      break;
    }
    else if (timeout == 9) {
      Serial.print( state );
      Serial.println(" fail...\nExit2");
    }
    else
    {
      Serial.print("Wifi Loading...");
      delay(500);
    }
  }
}
void sendDebug(String cmd)
{
  Serial.print("SEND: ");
  Serial.println(cmd);
  debug.println(cmd);
}
