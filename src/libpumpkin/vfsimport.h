Err VFSImportDatabaseFromFileEx(UInt16 volRefNum, const Char *pathNameP, UInt16 *cardNoP, LocalID *dbIDP, VFSImportProcPtr importProcP, void *userDataP, UInt32 importProc68K, UInt32 userData68K);
Err VFSExportDatabaseToFileEx(UInt16 volRefNum, const Char *pathNameP, UInt16 cardNo, LocalID dbID, VFSExportProcPtr exportProcP, void *userDataP, UInt32 exportProc68K, UInt32 userData68K);
