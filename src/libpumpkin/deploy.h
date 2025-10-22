void pumpkin_registry_create(UInt32 creator);
int pumpkin_deploy_files_session(vfs_session_t *session, char *path);
int pumpkin_deploy_from_image(vfs_session_t *session, uint8_t *p, uint32_t size);
void pumpkin_remove_locks(vfs_session_t *session, char *path);
