
#include <stdio.h>

int main(){
  int x[3] = {100, 101, 102};
  void *p = &x;
  printf("x[0] == %d\n" ,*(int *)p);
  (char *)p += sizeof(x[0]);   // <---- note this line
  printf("x[0] == %d\n" ,*(int *)p);
}
    
