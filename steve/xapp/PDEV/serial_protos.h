BOOL Open_SerialPort(struct standard_record *std_rec);
void Close_SerialPort(struct standard_record *std_rec);
void GetInfoFile(STRPTR appName, STRPTR devName, int *portNr, int *baudRate);
