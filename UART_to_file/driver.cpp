#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <conio.h>
#include <windows.h>

// UART variables
BOOL bWriteRC;
BOOL bReadRC;
HANDLE hCom;
DWORD iBytesWritten;
DWORD iBytesRead;

void send_data (int dec_val) {  // send data in decimal form
  char c;
  // send 8-bit address
  c = (char) dec_val;
  bWriteRC = WriteFile(hCom, &c, 1, &iBytesWritten,NULL);
}

//char get_data() {
//  // read data
//  char rxchar = 0;
//  bReadRC = ReadFile(hCom, &rxchar, 1, &iBytesRead, NULL);
//  return rxchar;
//}

int main(int argc, const char* argv[]) {

  // declarations
  int i = 0;
  char rxchar = 0;
  FILE *LOG;


  // UART initializations
  BOOL bPortReady;
  DCB dcb;

  hCom = CreateFile( TEXT("\\\\.\\COM14"),
    GENERIC_READ | GENERIC_WRITE,
    0, // exclusive access
    0, // no security
    OPEN_EXISTING,
    0, // no overlapped I/O
    0); // null template 
  if (hCom == INVALID_HANDLE_VALUE) {
    // error opening port; abort
    printf("error\n");
    exit(0);
  }

  //bPortReady = SetupComm(hCom, 2, 128); // set buffer sizes
  bPortReady = SetupComm(hCom, 2048, 2048); // set buffer sizes
  bPortReady = GetCommState(hCom, &dcb);
  dcb.BaudRate = 115200;
  dcb.ByteSize = 8;
  dcb.Parity = NOPARITY;
  // dcb.Parity = EVENPARITY;
  dcb.StopBits = ONESTOPBIT;
  dcb.fAbortOnError = TRUE;
  bPortReady = SetCommState(hCom, &dcb);
  
  //if (argc > 2) {
  //  if (atoi(argv[1]) == 1) {
  //    printf("reading from address %d\n", atoi(argv[2]));
  //  }
  //} else {
  //  printf("Usage: UART_to_file.exe [COM port number] [filename]\n");
  //}
 
  LOG = fopen("C:\\Users\\ragarci2\\workspace_v5_4\\Fixed_LT_FIFO_plus\\MHZ_868\\loggy.txt","w");
  char mybuf[500];
  //setvbuf(LOG, mybuf, _IOLBF, 500);
  setvbuf(LOG, NULL, _IONBF, 0);

  i=0;
  while(1) {
    bReadRC = ReadFile(hCom, &rxchar, 1, &iBytesRead, NULL);
    if (iBytesRead > 0 && rxchar != '\r'){
      mybuf[i] = rxchar;
      i++;
    }
    if (rxchar == '\n') {
      mybuf[i] = '\0';
      fprintf(LOG,"%s",mybuf);
      i=0;
    }
  }

  return 0;
}