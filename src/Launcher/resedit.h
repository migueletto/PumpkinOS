typedef enum {
  setProperties,
  getProperties,
  finishForm
} dynamic_form_phase_t;

typedef enum {
  numericItem,
  alphaItem,
  checkboxItem,
  numPairItem
} dynamic_form_item_type_t;

typedef struct {
  char *label;
  dynamic_form_item_type_t type;
  UInt16 maxChars;
} dynamic_form_item_t;

void showDynamicForm(const dynamic_form_item_t *items, char *title, void (*callback)(FormType *frm, dynamic_form_phase_t phase, void *data), void *data);

void setField(FormType *frm, UInt16 fieldId, char *s, Boolean focus);
void setField4char(FormType *frm, UInt16 fieldId, UInt32 value, Boolean focus);
void setFieldNum(FormType *frm, UInt16 fieldId, UInt32 value, Boolean focus);
Boolean getField(FormType *frm, UInt16 fieldId, char *buf, int size);
UInt32 getField4Char(FormType *frm, UInt16 fieldId);
UInt32 getFieldNum(FormType *frm, UInt16 fieldId);
void setControlValue(FormType *frm, UInt16 ctlId, Int16 value);
Int16 getControlValue(FormType *frm, UInt16 ctlId);

Boolean editString(FormType *frm, char *title, MemHandle h);
Boolean editBitmap(FormType *frm, char *title, MemHandle h);
Boolean editForm(FormType *frm, char *title, MemHandle h);
Boolean editBinary(FormType *frm, char *title, MemHandle h);
