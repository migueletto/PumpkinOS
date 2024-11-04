void Pterm0(void);
int32_t Cconin(void);
int32_t Cconout(int16_t c);
int32_t Cauxin(void);
int32_t Cauxout(int16_t c);
int32_t Cprnout(int16_t c);
int32_t Crawio(int16_t w);
int32_t Crawcin(void);
int32_t Cnecin(void);
int32_t Cconws(uint8_t *buf);
int32_t Cconrs(void);
int32_t Cconis(void);
int32_t Dsetdrv(int16_t drv);
int16_t Cconos(void);
int16_t Cprnos(void);
int16_t Cauxis(void);
int16_t Cauxos(void);
int32_t Maddalt(void *start, int32_t size);
int32_t Srealloc(int32_t len);
int16_t Dgetdrv(void);
void Fsetdta(DTA *buf);
uint16_t Tgetdate(void);
int16_t Tsetdate(uint16_t date);
uint32_t Tgettime(void);
int16_t Tsettime(uint16_t time);
DTA *Fgetdta(void);
uint16_t Sversion(void);
void Ptermres(int32_t keepcnt, int16_t retcode);
int16_t Dfree(DISKINFO *buf, int16_t driveno);
int32_t Dcreate(char *path);
int32_t Ddelete(char *path);
int16_t Dsetpath(char *path);
int16_t Fcreate(char *fname, int16_t attr);
int32_t Fopen(char *fname, int16_t mode);
int16_t Fclose(int16_t handle);
int32_t Fread(int16_t handle, int32_t count, void *buf);
int32_t Fwrite(int16_t handle, int32_t count, void *buf);
int16_t Fdelete(char *fname);
int32_t Fseek(int32_t offset, int16_t handle, int16_t seekmode);
int16_t Fattrib(char *filename, int16_t wflag, int16_t attrib);
void *Mxalloc(int32_t amount, int16_t mode);
int16_t Fdup(int16_t handle);
int16_t Fforce(int16_t stdh, int16_t nonstdh);
int16_t Dgetpath(char *path, int16_t driveno);
void *Malloc(int32_t number);
int32_t Mfree(void *block);
int32_t Mshrink(void *block, int32_t newsiz);
void Pterm(uint16_t retcode);
int32_t Fsfirst(char *filename, int16_t attr);
int16_t Fsnext(void);
int32_t Frename(char *oldname, char *newname);
void Fdatime(DOSTIME *timeptr, int16_t handle, int16_t wflag);
int32_t Flock(int16_t handle, int16_t mode, int32_t start, int32_t length);
int32_t Nversion(void);
int32_t Frlock(int16_t handle, int32_t start, int32_t length);
int32_t Frunlock(int16_t handle, int32_t start);
int32_t Flock2(int16_t handle, int32_t length);
int32_t Funlock(int16_t handle);
int32_t Fflush(int16_t handle);
void Syield(void);
int32_t Fpipe(int16_t *usrh);
int32_t Ffchown(int16_t fd, int16_t uid, int16_t gid);
int32_t Ffchmod(int16_t fd, int16_t mode);
int16_t Fsync(int16_t handle);
int32_t Finstat(int16_t fh);
int32_t Foutstat(int16_t fh);
int32_t Fgetchar(int16_t fh, int16_t mode);
int32_t Fputchar(int16_t fh, int32_t ch, int16_t mode);
int32_t Pwait(void);
int16_t Pnice(int16_t delta);
int16_t Pgetpid(void);
int16_t Pgetppid(void);
int16_t Pgetpgrp(void);
int16_t Psetpgrp(int16_t pid, int16_t newgrp);
int16_t Pgetuid(void);
int16_t Psetuid(int16_t id);
int32_t Pkill(int16_t pid, int16_t sig);
int32_t Psignal(int16_t sig, int32_t handler);
int16_t Pvfork(void);
int16_t Pgetgid(void);
int16_t Psetgid(int16_t id);
int32_t Psigblock(int32_t mask);
int32_t Psigsetmask(int32_t mask);
int32_t Pusrval(int32_t val);
int16_t Pdomain(int16_t dom);
void Psigreturn(void);
int16_t Pfork(void);
int32_t Pwait3(int16_t flag, int32_t *rusage);
int32_t Fselect(uint16_t timeout, int32_t *rfds, int32_t *wfds);
int32_t Prusage(int32_t *r);
int32_t Psetlimit(int16_t lim, int32_t value);
int32_t Talarm(int32_t time);
void Pause(void);
int32_t Sysconf(int16_t n);
int32_t Psigpending(void);
int32_t Dpathconf(uint8_t *name, int16_t mode);
int32_t Pmsg(int16_t mode, int32_t mbox, void *msg);
int32_t Fmidipipe(int16_t pid, int16_t in, int16_t out);
int32_t Prenice(int16_t pid, int16_t delta);
int32_t Dopendir(char *name, int16_t flag);
int32_t Dreaddir(int16_t len, int32_t dirhandle, char *buf);
int32_t Drewinddir(int32_t handle);
int32_t Dclosedir(int32_t dirhandle);
int32_t Fxattr(int16_t flag, char *name, XATTR *xattr);
int32_t Flink(char *oldname, char *newname);
int32_t Fsymlink(char *oldname, char *newname);
int32_t Freadlink(int16_t bufsiz, char *buf, char *name);
int32_t Dcntl(int16_t cmd, char *name, int32_t arg);
int32_t Fchown(char *name, int16_t uid, int16_t gid);
int32_t Fchmod(char *name, int16_t mode);
int16_t Pumask(int16_t mode);
int32_t Psemaphore(int16_t mode, int32_t id, int32_t timeout);
int32_t Dlock(int16_t mode, int16_t drv);
void Psigpause(int32_t mask);
int32_t Psigaction(int16_t sig, struct sigaction *act, struct sigaction *oact);
int32_t Pgeteuid(void);
int32_t Pgetegid(void);
int32_t Pwaitpid(int16_t pid, int16_t flag, int32_t *rusage);
int32_t Dgetcwd(char *path, int16_t drv, int16_t size);
void Salert(char *msg);
int32_t Tmalarm(int32_t time);
int32_t Psigintr(int16_t vec, int16_t sig);
int32_t Suptime(int32_t *uptime, int32_t *loadaverage);
int16_t Ptrace(int16_t request, int16_t pid, void *addr, int32_t data);
int32_t Mvalidate(int16_t pid, void *start, int32_t size, int32_t *flags);
int32_t Dxreaddir(int16_t ln, int32_t dirh, char *buf, XATTR *xattr, int32_t *xr);
int32_t Pseteuid(int16_t euid);
int32_t Psetegid(int16_t egid);
int16_t Pgetauid(void);
int16_t Psetauid(int16_t id);
int32_t Pgetgroups(int16_t len, int16_t *gidset);
int32_t Psetgroups(int16_t len, int16_t *gidset);
int32_t Tsetitimer(int16_t which, int32_t *interval, int32_t *value, int32_t *ointerval, int32_t *ovalue);
int32_t Dchroot(char *path);
int32_t Fstat64(int16_t flag, char *name, STAT *stat);
int32_t Fseek64(int32_t hioffset, uint32_t lowoffset, int16_t handle, int16_t seekmode, int64_t *newpos);
int32_t Dsetkey(int32_t hidev, int32_t lowdev, char *key, int16_t cipher);
int32_t Psetreuid(int16_t ruid, int16_t euid);
int32_t Psetregid(int16_t rgid, int16_t egid);
void Sync(void);
int32_t Shutdown(int32_t mode);
int32_t Dreadlabel(char *path, char *label, int16_t length);
int32_t Dwritelabel(char *path, char *label);
int32_t Ssystem(int16_t mode, int32_t arg1, int32_t arg2);
int32_t Tgettimeofday(struct timeval *tv, timezone *tzp);
int32_t Tsettimeofday(struct timeval *tv, timezone *tzp);
int Tadjtime(struct timeval *delta, struct timeval *olddelta);
int32_t Pgetpriority(int16_t which, int16_t who);
int32_t Psetpriority(int16_t which, int16_t who, int16_t pri);
int32_t Fpoll(POLLFD *fds, uint32_t nfds, uint32_t timeout);
int32_t Fwritev(int16_t handle, struct iovec *iov, int32_t niov);
int32_t Freadv(int16_t handle, struct iovec *iov, int32_t niov);
int32_t Ffstat64(int16_t fd, STAT *stat);
int32_t Psysctl(int32_t *name, uint32_t namelen, void *old, uint32_t *oldlenp, void *new, uint32_t newlen);
int32_t Fsocket(int32_t domain, int32_t type, int32_t protocol);
int32_t Fsocketpair(int32_t domain, int32_t type, int32_t protocol, int16_t *fds);
int32_t Faccept(int16_t fd, struct sockaddr *name, uint32_t *anamelen);
int32_t Fconnect(int16_t fd, struct sockaddr *name, uint32_t anamelen);
int32_t Fbind(int16_t fd, struct sockaddr *name, uint32_t anamelen);
int32_t Flisten(int16_t fd, int32_t backlog);
int32_t Frecvmsg(int16_t fd, struct msghdr *msg, int32_t flags);
int32_t Fsendmsg(int16_t fd, struct msghdr *msg, int32_t flags);
int32_t Frecvfrom(int16_t fd, void *buf, int32_t buflen, int32_t flags, struct sockaddr *to, uint32_t *addrlen);
int32_t Fsendto(int16_t fd, void *buf, int32_t buflen, int32_t flags, struct sockaddr *to, uint32_t addrlen);
int32_t Fsetsockopt(int16_t fd, int32_t level, int32_t name, void *val, uint32_t valsize);
int32_t Fgetsockopt(int16_t fd, int32_t level, int32_t name, void *val, uint32_t *valsize);
int32_t Fgetpeername(int16_t fd, struct sockaddr *asa, uint32_t *alen);
int32_t Fgetsockname(int16_t fd, struct sockaddr *asa, uint32_t *alen);
int32_t Fshutdown(int16_t fd, int32_t how);
int32_t Maccess(void *start, int32_t size, int16_t mode);
int32_t Fchown16(char *name, int16_t uid, int16_t gid, int16_t flag);
int32_t Fchdir(int16_t handle);
int32_t Ffdopendir(int16_t fd);
int16_t Fdirfd(int32_t handle);
int32_t Dxopendir(char *name, int16_t flag);