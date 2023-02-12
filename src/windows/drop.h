#ifdef __cplusplus
extern "C" {
#endif

void RegisterDropWindow(HWND hwnd, IDropTarget **ppDropTarget);
void UnregisterDropWindow(HWND hwnd, IDropTarget *pDropTarget);

#ifdef __cplusplus
}
#endif
