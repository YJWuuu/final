#include "mbed.h"
#include "bbcar.h"

RawSerial xbee(D12, D11);
EventQueue queue(32 * EVENTS_EVENT_SIZE);
Thread t;

Ticker servo_ticker;
Ticker encoder_ticker;
PwmOut pin8(D8), pin9(D9);
DigitalIn pin3(D3);

DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);

DigitalInOut pin10(D10);

DigitalIn sw2(SW2);

Serial uart(D1,D0); //tx,rx
RawSerial pc(USBTX, USBRX);


BBCar car(pin8, pin9, servo_ticker);
parallax_encoder encoder0(pin3, encoder_ticker);
parallax_ping  ping1(pin10);

void reachWall_cm(int cm);
void led2On1sec(void);

int main(void)
{
    led2 = 1;
/********************************Xbee********************************/
    int i = 0;
pc.printf("%d\r\n", ++i);
    pc.baud(9600);

    char xbee_reply[4];
pc.printf("%d\r\n", ++i);
    // XBee setting
    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
pc.printf("%d\r\n", ++i);
    if(xbee_reply[0] == 'O' && xbee_reply[1] == 'K'){
        pc.printf("enter AT mode.\r\n");
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
pc.printf("%d\r\n", ++i);
    xbee.printf("ATMY 0x250\r\n");
    // reply_messange(xbee_reply, "setting MY : 0x250");
pc.printf("%d\r\n", ++i);
    xbee.printf("ATDL 0x150\r\n");
    // reply_messange(xbee_reply, "setting DL : 0x150");
pc.printf("%d\r\n", ++i);
    xbee.printf("ATID 0x1\r\n");
    // reply_messange(xbee_reply, "setting PAN ID : 0x1");
pc.printf("%d\r\n", ++i);
    xbee.printf("ATWR\r\n");
    // reply_messange(xbee_reply, "write config");
pc.printf("%d\r\n", ++i);
    xbee.printf("ATMY\r\n");
    // check_addr(xbee_reply, "MY");
pc.printf("%d\r\n", ++i);
    xbee.printf("ATDL\r\n");
    // check_addr(xbee_reply, "DL");
pc.printf("%d\r\n", ++i);
    xbee.printf("ATCN\r\n");
    // reply_messange(xbee_reply, "exit AT mode");
    xbee.getc();
pc.printf("%d\r\n", ++i);
    // start
    pc.printf("start\r\n");

    t.start(callback(&queue, &EventQueue::dispatch_forever));
    
    led2On1sec();
/********************************Xbee********************************/

    led1 = 1;
    led2 = 1;
    led3 = 1;

    // please contruct you own calibration table with each servo
    double pwm_table0[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table0[] = {-16.668, -16.109, -14.913, -11.643, -5.582, 0.000, 4.944, 10.846, 14.514, 15.870, 16.508};
    double pwm_table1[] = {-150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150};
    double speed_table1[] = {-17.465, -17.146, -16.109, -12.840, -6.061, 0.000, 6.300, 12.760, 16.030, 17.146, 17.465};

    // first and fourth argument : length of table
    car.setCalibTable(11, pwm_table0, speed_table0, 11, pwm_table1, speed_table1);

    while (sw2);

    // go straight
    xbee.printf("Go straight\r\n");

    car.goStraightCalib(15);
    wait(4);
    reachWall_cm(20);
    led2On1sec();

    // turn left
    xbee.printf("Turn left\r\n");
    car.rotateLeft_90();
    car.goStraightCalib(15);
    wait(3);
    reachWall_cm(15);
    
    car.rotateRight_90();
    car.goStraightCalib(-15);
    wait(0.5);
    car.stop();
    led2On1sec();

    // OpenMV Cam to idenfity an image
    xbee.printf("Snapshot\r\n");

    char s[21];
    sprintf(s,"takePicture");
    uart.puts(s);
    pc.printf("send\r\n");
    
    uart.getc();
    led2On1sec();

    // go back
    // car.goStraightCalib(-13);
    // wait(1);

    // turn right
    xbee.printf("Turn right\r\n");
    // car.turnRight();
    car.rotateRight_90();
    car.goStraightCalib(15);
    wait(3.5);
    // reachWall_cm(10);
    led2On1sec();
    
    // reverse parking
    xbee.printf("Reverse parking\r\n");
    car.rotateLeft_90();
    car.goStraightCalib(-15);
    wait(1.5);
    car.stop();
    wait(0.5);
    led2On1sec();

    xbee.printf("Leave mission 1\r\n");
    car.goStraightCalib(15);
    wait(2);
    car.rotateRight_90();
    car.goStraightCalib(15);
    wait(1.5);
    car.rotateRight_90();
    led2On1sec();

    // mission 2
    xbee.printf("Mission 2 starts. Go straight\r\n");
    car.goStraightCalib(15);
    wait(3);
    reachWall_cm(20);
    led2On1sec();

    // turn right
    xbee.printf("Turn right\r\n");
    car.rotateRight_90();
    car.goStraightCalib(15);
    wait(2);
    led2On1sec();

    // turn right
    xbee.printf("Turn right\r\n");
    car.rotateRight_90();
    car.goStraightCalib(15);
    wait(2.5);
    led2On1sec();
    
    // identify the object
    xbee.printf("Identify the object\r\n");
    car.identifyObject();
    float d[5];
    for (int i = 0; i < 5; i++) {
        d[i] = (float)ping1;
        wait(0.6);
    }
    if (d[2] > d[1] && d[2] > d[3]) {
        led1 = 0;
        pc.printf("Indentify: Concave\r\n");
        xbee.printf("Indentify: Concave\r\n");
    }
    else if (d[3] > d[2] && d[2] > d[1]) {
        led2 = 0;
        pc.printf("Indentify: Right Angle Triangle\r\n"); 
        xbee.printf("Indentify: Right Angle Triangle\r\n"); 
    }
    else if (d[2] < d[1] - 3 && d[2] < d[3] - 3) {
        led3 = 0;
        pc.printf("Indentify: Triangle\r\n"); 
        xbee.printf("Indentify: Triangle\r\n"); 
    }
    else {
        led1 = 0;
        led2 = 0;
        pc.printf("Indentify: Rectangle\r\n"); 
        xbee.printf("Indentify: Rectangle\r\n"); 
    }
    led1 = 1;
    led2 = 1;
    led3 = 1;
    car.stop();
    car.identifyObject2();
    // back
    car.goStraightCalib(-15);
    wait(0.5);
    led2On1sec();

    // leave mission 2
    xbee.printf("Leave mission 2\r\n");
    car.goStraightCalib(-15);
    wait(2);
    car.rotateLeft_90();
    car.goStraightCalib(15);
    reachWall_cm(20);
    led2On1sec();
    
    car.rotateRight_90();
    car.goStraightCalib(15);
    reachWall_cm(20);
    led2On1sec();

}
void reachWall_cm(int cm)
{
    while (1) {
        if ((float)ping1 > cm) {
            led1 = 1;
        }
        else {
            led1 = 0;
            car.stop();
            break;
        }
        wait(.01);
    }
    return;
}
void led2On1sec(void)
{
    led2 = 0;
    wait(.1);
    led2 = 1;
}

void reply_messange(char *xbee_reply, char *messange){
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    xbee_reply[2] = xbee.getc();
    if(xbee_reply[1] == 'O' && xbee_reply[2] == 'K'){
        pc.printf("%s\r\n", messange);
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
        xbee_reply[2] = '\0';
    }
}

void check_addr(char *xbee_reply, char *messenger){
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    xbee_reply[2] = xbee.getc();
    xbee_reply[3] = xbee.getc();
    pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
    xbee_reply[2] = '\0';
    xbee_reply[3] = '\0';
}