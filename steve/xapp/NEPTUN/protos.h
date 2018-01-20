/**** doit ****/

int XappDoIt(PROCESSINFO *ThisPI);
void GetVarsFromPI(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI);
void PutVarsToPI(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI);
BOOL PerformActions(struct Neptun_record *nep_rec, PROCESSINFO *ThisPI);
BOOL GetNepValues(struct Neptun_record *nep_rec, int which);
BOOL SendAndGet_Neptune(STRPTR cmd, STRPTR result, int len);
void GetNepVersion(struct Neptun_record *nep_rec);

/******** EOF ********/
