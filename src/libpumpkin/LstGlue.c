#include <PalmOS.h>

Int16 LstGlueGetTopItem(const ListType *listP) {
  return listP ? listP->topItem : 0;
}

FontID LstGlueGetFont(const ListType *listP) {
  return listP ? listP->font : 0;
}

void LstGlueSetFont(ListType *listP, FontID fontID) {
  if (listP) {
    listP->font = fontID;
  }
}

Char **LstGlueGetItemsText(const ListType *listP) {
  return listP ? listP->itemsText : NULL;
}

// Enables or disables incremental search for a sorted popup list.
// If incremental search is enabled, when the list is displayed the user can navigate the list by entering up to five characters.
// The list will scroll to present the first list item that matches the entered characters.
// This feature only works for popup lists, and only works if the list is sorted and the list items are available to the List Manager
// (that is, you don’t pass NULL to LstSetListChoices).

void LstGlueSetIncrementalSearch(ListType *listP, Boolean incrementalSearch) {
  if (listP) {
    listP->attr.search = incrementalSearch;
  }
}

void *LstGlueGetDrawFunction(ListType *listP) {
  return listP ? listP->drawItemsCallback : NULL;
}
