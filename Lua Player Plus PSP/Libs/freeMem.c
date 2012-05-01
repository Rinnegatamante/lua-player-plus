#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <malloc.h>

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Calculates total free memory:
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
typedef struct
{
      void *buffer;
      void *next;
} _LINK;

int freemem(){
   int size = 4096, total = 0;
    _LINK *first = NULL, *current = NULL, *lnk;

    while(1){
       lnk = (_LINK*)malloc(sizeof(_LINK));
       if (!lnk)
          break;

       total += sizeof(_LINK);

       lnk->buffer = malloc(size);
       if (!lnk->buffer){
          free(lnk);
          break;
       }

       total += size;
       lnk->next = NULL;

       if (current){
          current->next = (void*)lnk;
          current = lnk;
       } else
          current = first = lnk;
    }

    lnk = first;
    while (lnk){
       free(lnk->buffer);
       current = lnk->next;
       free(lnk);
       lnk = current;
    }
    return total;
}
