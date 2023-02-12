#include <PalmOS.h>

//static void quicksort(UInt8 *baseP, Int32 low, Int32 high, Int32 width, CmpFuncPtr comparF, Int32 other, UInt8 *aux);

void SysQSort(void *baseP, UInt16 numOfElements, Int16 width, CmpFuncPtr comparF, Int32 other) {
  Int32 i, j;
  UInt8 *array, *aux;

  if (baseP != NULL && numOfElements > 1 && width > 0 && comparF != NULL) {
    if ((aux = MemPtrNew(width > 1 ? width : 4)) != NULL) {
      array = (UInt8 *)baseP;

      for (i = 1 ; i < numOfElements; i++) {
         j = i;
         while (j > 0 && comparF(&array[(j-1)*width], &array[j*width], other) > 0) {
           MemMove(aux, &array[j*width], width);
           MemMove(&array[j*width], &array[(j-1)*width], width);
           MemMove(&array[(j-1)*width], aux, width);
           j--;
         }
      }
      MemPtrFree(aux);
    }
  }
}

/*
void SysQSort(void *baseP, UInt16 numOfElements, Int16 width, CmpFuncPtr comparF, Int32 other) {
  Int32 c, d, flag = 0;
  UInt8 *array, *aux;

  if (baseP != NULL && numOfElements > 1 && width > 0 && comparF != NULL) {
    if ((aux = MemPtrNew(width)) != NULL) {
      array = (UInt8 *)baseP;

      for (c = 1 ; c < numOfElements; c++) {
        MemMove(aux, &array[c*width], width);
        flag = 0;

        for (d = c - 1 ; d >= 0; d--) {
          if (comparF(&array[d*width], aux, other) > 0) {
            MemMove(&array[(d+1)*width], &array[d*width], width);
            flag = 1;
          } else {
            break;
          }
        }
        if (flag) {
          MemMove(&array[(d+1)*width], aux, width);
        }
      }
      MemPtrFree(aux);
    }
  }
}
*/

/*
void SysQSort(void *baseP, UInt16 numOfElements, Int16 width, CmpFuncPtr comparF, Int32 other) {
  UInt8 *aux;

  if (baseP != NULL && numOfElements > 1 && width > 0 && comparF != NULL) {
    if ((aux = MemPtrNew(width)) != NULL) {
      quicksort((UInt8 *)baseP, 0, numOfElements-1, width, comparF, other, aux);
      MemPtrFree(aux);
    }
  }
}

static void swap(UInt8 *baseP, Int32 i, Int32 j, Int32 width, UInt8 *aux) {
  MemMove(aux, &baseP[i*width], width);
  MemMove(&baseP[i*width], &baseP[j*width], width);
  MemMove(&baseP[j*width], aux, width);
}

static void quicksort(UInt8 *baseP, Int32 low, Int32 high, Int32 width, CmpFuncPtr comparF, Int32 other, UInt8 *aux) {
  Int32 i, j, pivot;

  if (low < high) {
    pivot = low;
    i = low;
    j = high;

    while (i < j) {
      while (comparF(&baseP[i*width], &baseP[pivot*width], other) <= 0 && i < high) i++;
      while (comparF(&baseP[j*width], &baseP[pivot*width], other) > 0) j--;
      if (i < j) swap(baseP, i, j, width, aux);
    }

    if (pivot != j) swap(baseP, pivot, j, width, aux);
    quicksort(baseP, low, j-1,  width, comparF, other, aux);
    quicksort(baseP, j+1, high, width, comparF, other, aux);
  }
}
*/
