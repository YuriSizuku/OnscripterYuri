#include <stdio.h>
int main(int argc,char *argv[]){
  FILE *pFile = fopen(*(argv+1), "rb");
  if(pFile != nullptr){
    fseek(pFile,0,SEEK_END);
    int lSize = ftell(pFile);
    rewind(pFile);
    char *buffer = new char[lSize];
    int result = fread(buffer,1,lSize,pFile);
    fclose(pFile);
    for (int i = 0; i < lSize; ++i) {
      int ch = buffer[i];
      ch ^= 0x84;
      buffer[i] = ch;
    }
    FILE *pOutFile = fopen(*(argv+2), "wb");
    fwrite(buffer, sizeof(char),lSize,pOutFile);
    delete[] buffer;
    fclose(pOutFile);
  }
  return 0;
}

