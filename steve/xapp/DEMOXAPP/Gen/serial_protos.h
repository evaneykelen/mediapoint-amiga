BOOL Open_SerialPort(struct SerRecord *);
void Close_SerialPort(struct SerRecord *);
BOOL SendStringViaSer(struct SerRecord *, UBYTE *);
void GetInfoFile(STRPTR, STRPTR, STRPTR, int *, int *);
