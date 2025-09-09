typedef struct VFSExplorer VFSExplorer;

VFSExplorer *VFSExplorerCreate(FormType *frm, UInt16 tableID, char *root);
Boolean VFSExplorerHandleEvent(VFSExplorer *vfse, EventType *event);
void VFSExplorerRefresh(FormType *frm, VFSExplorer *vfse);
char *VFSExplorerCurrentPath(VFSExplorer *vfse);
char *VFSExplorerSelectedItem(VFSExplorer *vfse);
void VFSExplorerDestroy(VFSExplorer *vfse);
