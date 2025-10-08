void pumpkin_registry_create(AppRegistryType *ar, UInt32 creator);
int pumpkin_deploy_files_session(vfs_session_t *session, char *path, AppRegistryType *ar);
int pumpkin_deploy_from_image(vfs_session_t *session, uint8_t *p, uint32_t size, AppRegistryType *ar);
void pumpkin_remove_locks(vfs_session_t *session, char *path);
