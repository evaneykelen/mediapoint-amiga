/**** cdf.c ****/

void Init_CDF_Record(struct CDF_Record *CDF_rec);
BOOL Parse_CDF_File(struct CDF_Record *CDF_rec);
BOOL Write_CDF_File(struct CDF_Record *CDF_rec, STRPTR path);

/**** ecp_main.c ****/

BOOL Report(STRPTR str);
BOOL IsTheButtonHit(void);

/**** parsebig.c ****/

BOOL ProcessBigFile(struct CDF_Record *CDF_rec);

/**** session.c ****/

BOOL Session(STRPTR CDF, STRPTR TempScript, STRPTR BigFile, STRPTR Type, STRPTR LogName );

/**** X_DoTrans.c ****/

BOOL ValidateSession(struct CDF_Record *CDF_rec);
BOOL InitConnection(struct CDF_Record *CDF_rec);
BOOL OpenConnection(struct CDF_Record *CDF_rec);
BOOL LogOn(struct CDF_Record *CDF_rec);
BOOL DoTransaction(struct CDF_Record *CDF_rec, struct ParseRecord *PR);
BOOL LogOff(struct CDF_Record *CDF_rec);
BOOL CloseConnection(struct CDF_Record *CDF_rec);
BOOL DeInitConnection(struct CDF_Record *CDF_rec);

/******** E O F ********/
