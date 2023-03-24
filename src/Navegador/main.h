void InitPrefs(AppPrefs *prefs);
void SavePrefs(void);
AppPrefs *LoadPrefs(void);
AppPrefs *GetPrefs(void);
char *GetAppVersion(void);
char *GetRomVersion(void);
Int16 GetRomVersionNumber(void);
void SetWait(Int16 wait);
void ToggleBacklight(void);
