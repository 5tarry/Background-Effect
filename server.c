#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <pthread.h>
#include <fcntl.h>
#include <termios.h>
#define BUF_SIZE 512
#define MAX_BUF 32
#define D1 0x01
#define D2 0x02
#define D3 0x04
#define D4 0x08

static struct termios init_setting, new_setting;
char seg_num[10] = { 0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xd8, 0x80, 0x90 };
char seg_dnum[10] = { 0x40, 0x79, 0x24, 0x30, 0x19, 0x12, 0x02, 0x58, 0x00, 0x10 };
int client_fd;
int server_fd;
char buffer[BUF_SIZE];
int msg_size;
char message[BUF_SIZE]= "capture";
char prev_message[BUF_SIZE]= {0,};
int cnt=0;
int pic=0;
void* function_receive(void* num){ //client로부터 buffer에 인식된 pose값을 받아온다.(‘0’:flower, ‘1’:heart, ‘2’: superman, ‘3’: 동작 인식 안됨)
  while (1){
    if (client_fd < 0){
      printf("Server: accept failed.\n");
      return 0;
    }
    msg_size = read(client_fd, buffer, 1024);
    printf("buffer : %s\n", buffer);
  }
}
void* function_send(void* num){ //client에게 button이 눌릴경우 “capture”라는 message를 보낸다.
  while(1){
    if(pic==1){
      printf("message : %s \n", message);
      write(client_fd,message,sizeof(message)-1);
      pic=0;
    }
  }
}
int main(int argc, char*argv[]){
  unsigned short data[4];
  int dev_seg = open("/dev/my_segment", O_RDWR); //read&write='O_RDWR
  if (dev_seg == -1) {
    printf("seg Opening was not possible!\n");
    return -1;
  }
  printf("seg device opening was successfull!\n");
  int dev_gpio = open("/dev/my_gpio", O_RDWR); // if you want read='0_RDONLY' write='O_WRONLY', read&write='O_RDWR
  if (dev_gpio == -1) {
    printf("gpio Opening was not possible!\n");
    return -1;
  }
  printf("gpio device opening was successfull!\n");
  int dev_led = open("/dev/my_led", O_RDWR); // if you want read='0_RDONLY' write='O_WRONLY', read&write='O_RDWR
  if (dev_led == -1) {
    printf("led Opening was not possible!\n");
    return -1;
  }
  printf("led device opening was successfull!\n");
  int delay_time = 1000000;
  int tmp_n = 0;
  char buff;
  char prev = '0';
  struct sockaddr_in server_addr, client_addr;
  char temp[20];
  socklen_t len;
  pthread_t pthread_receive,pthread_send;
  // server socket 생성
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == -1){
    printf("Server : Can't open stream socket\n");
    exit(0);
  }
  //server_Addr 을 NULL로 초기화
  memset(&server_addr, 0x00, sizeof(server_addr));
  //server_addr 셋팅
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  server_addr.sin_port = htons(9000);
  //bind() 호출
  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <0){
    printf("Server : Can't bind local address.\n");
    exit(0);
  }
  //소켓을 수동 대기모드로 설정
  if (listen(server_fd, 5) < 0){
  printf("Server : Can't listening connect.\n");
  exit(0);
  }
  printf("Server : wating connection request.\n");
  //client 소켓과 연결
  len = sizeof(client_addr);
  client_fd = accept(server_fd, (struct sockaddr *)&client_addr, &len);
  inet_ntop(AF_INET, &client_addr.sin_addr.s_addr, temp, sizeof(temp));
  printf( "Server : %s client connected.\n", temp);
  //멀티 thread 생성
  pthread_create(&pthread_receive, NULL, function_receive, NULL); //receive를 위한 thread 생성
  pthread_create(&pthread_send, NULL, function_send, NULL); //send를 위한 thread 생성
  memset(buffer, 0x00, sizeof(buffer));
  while (1) {
    read(dev_gpio, &buff, 1); //button값을 읽어 buff에 저장
    write(dev_led, buffer, 1); //client로부터 받은 pose class(buffer)값을 dev_led에 저장하여 led_driver.c로 전달
    data[0] = (seg_num[3] << 4) | D1;
    data[1] = (seg_num[2] << 4) | D2;
    data[2] = (seg_num[1] << 4) | D3;
    data[3] = (seg_num[0] << 4) | D4;
    if (buff == '1' && prev != buff) { //button이 눌리면
      for (tmp_n = 0; tmp_n < 4; tmp_n++) {
        write(dev_seg, &data[tmp_n], 2); //7-segment에 3,2,1,0을 1초마다 출력
        usleep(delay_time);
      }
      write(dev_seg, 0x0000, 2);
      pic = 1; //function_send함수에서 ”capture”하라는 메세지로 보내게된다.
    }
    prev = buff;
  }
  close(server_fd);
  write(dev_seg, 0x0000, 2);
  close(dev_seg);
  close(dev_gpio);
  close(dev_led);
  return 0;
}
