typedef struct VFSExplorer VFSExplorer;

VFSExplorer *VFSExplorerCreate(FormType *frm, UInt16 tableID, UInt16 upID, UInt16 downID, char *root);
Boolean VFSExplorerHandleEvent(VFSExplorer *vfse, EventType *event);
void VFSExplorerRefresh(FormType *frm, VFSExplorer *vfse, Boolean redraw);
char *VFSExplorerCurrentPath(VFSExplorer *vfse);
char *VFSExplorerSelectedItem(VFSExplorer *vfse);
void VFSExplorerEnter(VFSExplorer *vfse);
void VFSExplorerPaginate(VFSExplorer *vfse, Int16 n);
void VFSExplorerDestroy(VFSExplorer *vfse);
