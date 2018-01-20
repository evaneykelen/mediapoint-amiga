/*******************************************************************/
/*
 *
 *  T H I S   I S   A L S O   A   P L A Y E R   M O D U L E !
 *
 *
 */

#include "nb:pre.h"

#define EXPRESSION_WIDTH 75	// see also vars.c

extern TEXT mainName[];

/************************************************************

	Declarations

	0		VAR = VAR
	1		VAR = VALUE
	2		VAR = "STRING"

	Expressions

	3		VAR = VAR + VALUE
	4		VAR = VALUE + VAR

	5		VAR = VAR + "STRING"
	6		VAR = "STRING" + VAR

	7		VAR = "STRING" + VALUE
	8		VAR = VALUE + "STRING"

	9 	VAR = VAR + VAR
	10	VAR = VALUE + VALUE
	11	VAR = "STRING" + "STRING"

	IF statements

	12	IF VAR < VALUE
	13	IF VAR < VAR
	14	IF VAR < "STRING"

	15	IF VALUE < VAR
	16	IF "STRING" < VAR

	Operators

	1		+
	2		-
	3		*
	4		/

	5		<
	6		<=
	7		>
	8		>=
	9		<>
	10	=

************************************************************/

/**** FUNCTIONS ****/

/******** AllocVIR() ********/

struct VarInfoRecord *AllocVIR(void)
{
struct VarInfoRecord *vir;

	vir = (struct VarInfoRecord *)AllocMem(sizeof(VIR), MEMF_ANY | MEMF_CLEAR);
	return( vir );
}

/******** AddVIR() ********/

void AddVIR(struct List *list, struct VarInfoRecord *VIR)
{
	AddTail(list, (struct Node *)VIR);
}

/******** AllocVAR() ********/

struct VarAssignRecord *AllocVAR(void)
{
struct VarAssignRecord *var;

	var = (struct VarAssignRecord *)AllocMem(sizeof(VAR), MEMF_ANY | MEMF_CLEAR);
	if ( var )
		NewList( &(var->var_CondList) );

	return( var );
}

/******** ParseDeclaration() ********/
/*
 * output: 0  -> OK
 * output: 1  -> = misses
 * output: 2  -> not enough double quotes
 * output: 3  -> var already exists
 * output: -1 -> syntax error
 *
 * In declarations VAR1=VAR2 is *NOT* allowed, only in expression.
 * VarIsVarAllowed takes care of this.
 *
 */

int ParseDeclaration(	struct ScriptInfoRecord *SIR,
											STRPTR parseStr, STRPTR varName1, STRPTR varName2,
											int *value,
											STRPTR varContents, int *type, BOOL VarIsVarAllowed)
{
int pos,numQ,arg;
char *p;
char token[EXPRESSION_WIDTH];

	*value = -1;
	varName1[0] = '\0';
	varName2[0] = '\0';
	varContents[0] = '\0';
	*type = -1;
	token[0] = '\0';

	// CHECK IF = IS AVAILABLE

	pos = stcarg(parseStr, "=");
	if (pos==strlen(parseStr))
		return(1);

	// COUNT NUMBER OF DOUBLE QUOTES

	numQ=0;
	p=parseStr;
	while(*p)
	{
		p=strchr(p,'\"');
		if(!p)
			break;
		p++;
		numQ++;
	}
	if (numQ>0 && numQ!=2)
		return(2);

	if (numQ==2)	// THIS IS A STRING VAR
	{
		sscanf(parseStr, "%s=\"%s\"", varName1, varContents);

		p=parseStr;
		arg=0;
		while(1)
		{
			p = stptok(p,token,sizeof(token),"=");

			if (arg==0)
				strcpy(varName1,token);
			else if (arg==1)
				strcpy(varContents,token);
			if (*p=='\0')
				break;
			p = stpblk(++p);
			arg++;
		}
	}
	else	// THIS IS AN INTEGER VAR OR ANOTHER VAR
	{
		p=parseStr;
		arg=0;
		while(1)
		{
			p = stptok(p,token,sizeof(token),"=");

			if (arg==0)
				strcpy(varName1,token);
			else if (arg==1)
			{
				TrimVarName(token);
				if ( isdigit( token[0] ) )
					sscanf(token,"%d",value);
				else
					strcpy(varName2,token);
			}

			if (*p=='\0')
				break;
			p = stpblk(++p);
			arg++;
		}
	}

	if ( *value==-1 )	// STRING DECL. OR VAR=VAR
	{
		if ( varName2[0] )
		{
			*type=0;		// VAR = VAR
			if ( varName1[0]=='\0' )
				return(-1);
		}
		else
		{
			*type=2;		// VAR = STRING
			if ( varName1[0]=='\0' || varContents[0]=='\0' )
				return(-1);
			RemoveQuotes(varContents);
		}
	}
	else
	{
		*type=1;			// VAR = VALUE
	}

	if ( *type==0 && !VarIsVarAllowed )
		return(-1);

	TrimVarName(varName1);
	TrimVarName(varName2);
	if ( *type!=2 )
		TrimVarName(varContents);

	return(0);
}

/******** ParseExpression() ********/
/*
 * output: 0  -> OK
 * output: 1  -> = misses
 * output: 2  -> not enough double quotes
 * output: -1 -> syntax error
 *
 */

int ParseExpression(	struct ScriptInfoRecord *SIR,
											STRPTR parseStr,
											STRPTR varName1, STRPTR varName2, STRPTR varName3,
											int *value1, int *value2, 
											STRPTR varContents1, STRPTR varContents2,
											int *oper, int *type, STRPTR labelName, int *jump,
											STRPTR varName4, STRPTR varName5, STRPTR varName6,
											int *value3, int *value4, 
											STRPTR varContents3, STRPTR varContents4,
											int *oper2, int *type2 )
{
int numQ,arg,len,retval,i,len2,len3,voor,na;
char *p;
char token1[50],token2[50],token3[50],token4[50],token5[50];
char token6[50],token7[50],token8[50],token9[50],token10[50];
char temp[EXPRESSION_WIDTH];

	*value1 = -1;
	*value2 = -1;
	*oper=-1;
	*type=-1;
	*jump=-1;

	*value3 = -1;
	*value4 = -1;
	*oper2=-1;
	*type2=-1;

	varName1[0] = '\0';
	varName2[0] = '\0';
	varName3[0] = '\0';
	varContents1[0] = '\0';
	varContents2[0] = '\0';
	labelName[0] = '\0';

	varName4[0] = '\0';
	varName5[0] = '\0';
	varName6[0] = '\0';
	varContents3[0] = '\0';
	varContents4[0] = '\0';

	token1[0]='\0';
	token2[0]='\0';
	token3[0]='\0';
	token4[0]='\0';
	token5[0]='\0';
	token6[0]='\0';
	token7[0]='\0';
	token8[0]='\0';
	token9[0]='\0';
	token10[0]='\0';

	TrimVarName(parseStr);

	// CHECK IF THIS IS A DECLARATION OR AN EXPRESSION

	if ( strnicmp(parseStr,"IF",2) )	// NOT AN IF VAR = VAR GOTO
	{
		len = stcarg(parseStr,"+-*/");
		if (len==strlen(parseStr))
		{
			retval = ParseDeclaration(SIR, parseStr,varName1,varName2,value1,
																varContents1,type,TRUE);
			return(retval);
		}
	}

	// COUNT NUMBER OF DOUBLE QUOTES

	numQ=0;
	p=parseStr;
	while(*p)
	{
		p=strchr(p,'\"');
		if(!p)
			break;
		p++;
		numQ++;
	}
	if ( numQ>0 && numQ%2 )
		return(2);

	// GET SEPARATE ARGUMENTS

	if ( !strnicmp(parseStr,"IF",2) )	// AN IF 
	{
		p=parseStr;

		i=1;
		voor=-1;
		na=-1;
		while( *(p+i) && i<80 )
		{
			if (	*(p+i) == '<' || *(p+i) == '>' || *(p+i) == '=' ||
						*(p+i) == '+' || *(p+i) == '-' || *(p+i) == '*' || *(p+i) == '/' )
			{
				voor=i-1;
				while( *(p+i) && i<80 )
				{
					if (	*(p+i) == '<' || *(p+i) == '>' || *(p+i) == '=' ||
								*(p+i) == '+' || *(p+i) == '-' || *(p+i) == '*' || *(p+i) == '/' )
						i++;
					else
						break;
				}
				if ( voor!=-1 && *(p+voor)!=' ' )
				{
					strins(p+voor+1," ");
					voor=-1;
					i++;
				}
				if ( *(p+i)!=' ' )
				{
					strins(p+i," ");
					i++;
				}
			}
			i++;
		}
	}

	if ( !strnicmp(parseStr,"IF",2) )	// IF VAR = VAR GOTO
	{
		p=parseStr;
		arg=0;
		len2=0;
		len3=strlen(p);
		while(1)
		{
			if ( !p[0] )
				break;
			len = stcarg(p," ");
			if (len==0)
				break;

			if ( len2>=len3+1 )
				break;

			if ( arg==0 )
			{
				stccpy(token1,p,len+1);
			}
			else if ( arg==1 )
			{
				stccpy(token2,p,len+1);
			}
			else if ( arg==2 )
			{
				stccpy(token3,p,len+1);
			}
			else if ( arg==3 )
			{
				stccpy(token4,p,len+1);
			}
			else if ( arg==4 )
			{
				stccpy(token5,p,len+1);
			}
			else if ( arg==5 )
			{
				stccpy(token6,p,len+1);
			}
			else if ( arg==6 )
			{
				stccpy(token7,p,len+1);
			}
			else if ( arg==7 )
			{
				stccpy(token8,p,len+1);
			}
			else if ( arg==8 )
			{
				stccpy(token9,p,len+1);
			}
			else if ( arg==9 )
			{
				stccpy(token10,p,len+1);
			}

			len2 += (len+1);

			i=0;
			while( *(p+len+1+i) == ' ' )
				i++;

			p=p+(len+1+i);
	
			arg++;
		}

		TrimVarName(token1);
		TrimVarName(token2);
		TrimVarName(token3);
		TrimVarName(token4);
		TrimVarName(token5);
		TrimVarName(token6);
		TrimVarName(token7);
		TrimVarName(token8);
		TrimVarName(token9);
		TrimVarName(token10);

		if ( isdigit( token2[0] ) )
			*type = 15;
		else if ( token2[0]=='\"' )
			*type = 16;
		else
		{
			if ( isdigit( token4[0] ) )
				*type = 12;
			else if ( token4[0]=='\"' )
				*type = 14;
			else
				*type = 13;
		}

		if ( !strnicmp(token5,"GOTO",4) )
			*jump = 1;
		else if ( !strnicmp(token5,"GOSUB",5) )
			*jump = 1;	// was 2, now no longer supported, make it goto
		else if ( !strnicmp(token5,"GOPREV",6) )
			*jump = 3;
		else if ( !strnicmp(token5,"GONEXT",6) )
			*jump = 4;
		else if ( !strnicmp(token5,"MAKE",4) )
			*jump = 5;

		if ( strlen(token3)==1 && !strcmp(token3,"<") )
			*oper = 5;
		else if ( strlen(token3)==2 && !strcmp(token3,"<=") )
			*oper = 6;
		else if ( strlen(token3)==1 && !strcmp(token3,">") )
			*oper = 7;
		else if ( strlen(token3)==2 && !strcmp(token3,">=") )
			*oper = 8;
		else if ( strlen(token3)==2 && !strcmp(token3,"<>") )
			*oper = 9;
		else if ( strlen(token3)==1 && !strcmp(token3,"=") )
			*oper = 10;

		if ( *jump!=5 )
		{
			strcpy(labelName,token6);
			if ( labelName[0]=='\0' )
				return(-1);
		}

		if( *jump==5 && token6[0] && token7[0] && token8[0] )
		{
			sprintf(temp,"%s %s %s",token6,token7,token8);
			if ( token9[0] )
			{
				strcat(temp," ");
				strcat(temp,token9);
			}
			if ( token10[0] )
			{
				strcat(temp," ");
				strcat(temp,token10);
			}
			retval = DoDecla(temp,oper2,type2,token5,token6,token7);
			if ( retval!=0 )
				return(-1);
		}
	}
	else	// VAR = VAR + VAR -- NOT AN IF STATEMENT
	{
		retval = DoDecla(parseStr,oper,type,token1,token2,token3);
		if ( retval!=0 )
			return(-1);
	}

	retval = CheckVarType(*type,
												varName1, varName2, varName3,
												varContents1, varContents2,			
												token1, token2, token3, token4, 
												value1, value2);
	if ( retval==-1 )
	{
		return(-1);
	}

	TrimVarName(varName1);
	TrimVarName(varName2);
	TrimVarName(varName3);
	if ( *jump!=5 )	// NEW
		TrimVarName(labelName);

	if ( *jump!=5 )	// NEW
	{
		if ( labelName[0] !='\0' )
		{
			strcpy(temp,labelName);
			RemoveQuotes(temp);
			if ( FindLabel(SIR,temp)==NULL )
			{
				if ( FindLabel(SIR,mainName)==NULL )
					return(5);
			}
		}
	}
	else if ( *jump==5 )	// MAKE
	{
		retval = CheckVarType(*type2,
													varName4, varName5, varName6,
													varContents3, varContents4,			
													token5, token6, token7, token8, 
													value3, value4);
		if ( retval==-1 )
		{
			return(-1);
		}

		TrimVarName(varName4);
		TrimVarName(varName5);
		TrimVarName(varName6);
	}

	return(0);
}

/******** ProcessParseError() ********/

void ProcessParseError(int error)
{
	switch(error)
	{
		case 1: 	Message("The '=' misses."); break;

		case 2: 	Message("Strings must be enclosed by 2 double quotes."); break;

		case 3:		Message("Variable declared twice."); break;

		case 4:		Message("Undeclared variable used."); break;

		case 5:		Message("Label doesn't exist."); break;

		case 6:		Message("Assignment after MAKE misses."); break;

		default:	Message("Syntax error."); break;
	}
}

#if 0
/******** PrintExpr() ********/

void PrintExpr(	STRPTR varName1, STRPTR varName2, STRPTR varName3,
								int value1, int value2, 
								STRPTR varContents1, STRPTR varContents2,
								STRPTR labelName,
								int oper, int type, int jump )
{
char operStr[5], jumpStr[20], declStr[256];

	if (oper==1)
		strcpy(operStr,"+");
	else if (oper==2)
		strcpy(operStr,"-");
	else if (oper==3)
		strcpy(operStr,"*");
	else if (oper==4)
		strcpy(operStr,"/");

	else if (oper==5)
		strcpy(operStr,"<");
	else if (oper==6)
		strcpy(operStr,"<=");
	else if (oper==7)
		strcpy(operStr,">");
	else if (oper==8)
		strcpy(operStr,">=");
	else if (oper==9)
		strcpy(operStr,"<>");
	else if (oper==10)
		strcpy(operStr,"=");

	if (jump==1)
		strcpy(jumpStr,"GOTO");
	else if (jump==2)
		strcpy(jumpStr,"GOSUB");
	else if (jump==3)
		strcpy(jumpStr,"GOPREV");
	else if (jump==4)
		strcpy(jumpStr,"GONEXT");

	if ( type==0 )
		sprintf(declStr, "%s = %s", varName1, varName2);
	else if ( type==1 )
		sprintf(declStr, "%s = %d", varName1, value1);
	else if ( type==2 )
		sprintf(declStr, "%s = \"%s\"", varName1, varContents1);
	else if ( type==3 )
		sprintf(declStr, "%s = %s %s %d", varName1, varName2, operStr, value2);
	else if ( type==4 )
		sprintf(declStr, "%s = %d %s %s", varName1, value1, operStr, varName3);
	else if ( type==5 )
		sprintf(declStr, "%s = %s %s \"%s\"", varName1, varName2, operStr, varContents2);
	else if ( type==6 )
		sprintf(declStr, "%s = \"%s\" %s %s", varName1, varContents1, operStr, varName3);
	else if ( type==7 )
		sprintf(declStr, "%s = \"%s\" %s %d", varName1, varContents1, operStr, value2);
	else if ( type==8 )
		sprintf(declStr, "%s = %d %s \"%s\"", varName1, value1, operStr, varContents2);
	else if ( type==9 )
		sprintf(declStr, "%s = %s %s %s", varName1, varName2, operStr, varName3);
	else if ( type==10 )
		sprintf(declStr, "%s = %d %s %d", varName1, value1, operStr, value2);
	else if ( type==11 )
		sprintf(declStr, "%s = \"%s\" %s \"%s\"", varName1, varContents1, operStr, varContents2);

	else if ( type==12 )
		sprintf(declStr, "IF %s %s %d %s %s", varName1, operStr, value2, jumpStr, labelName);
	else if ( type==13 )
		sprintf(declStr, "IF %s %s %s %s %s", varName1, operStr, varName2, jumpStr, labelName);
	else if ( type==14 )
		sprintf(declStr, "IF %s %s \"%s\" %s %s", varName1, operStr, varContents2, jumpStr, labelName);
	else if ( type==15 )
		sprintf(declStr, "IF %d %s %s %s %s", value1, operStr, varName2, jumpStr, labelName);
	else if ( type==16 )
		sprintf(declStr, "IF \"%s\" %s %s %s %s", varContents1, operStr, varName2, jumpStr, labelName);
} 
#endif

/******** TrimVarName() ********/

void TrimVarName(STRPTR str)
{
char *p;
int i;

	if ( str[0]=='\0' )
		return;

	p = stpblk(str);
	strcpy(str,p);

	i=strlen(str)-1;
	while(i>0)
	{
		if ( isspace( str[i] ) )
			str[i] = '\0';
		else
			break;
		i--;
	}
}

/******** TrimVarNameV2() ********/

void TrimVarNameV2(STRPTR str)
{
char *p;

	if ( str[0]=='\0' )
		return;
	p = stpblk(str);
	strcpy(str,p);
}

/******** DeclToVIR() ********/
/*
 * Parses a declaration string and fills in a VarInfoRecord (VIR)
 *
 */

BOOL DeclToVIR(struct ScriptInfoRecord *SIR, STRPTR declStr, VIR *this_vir)
{
int value,type,error;
char varName1[EXPRESSION_WIDTH], varName2[EXPRESSION_WIDTH], varContents[EXPRESSION_WIDTH];
BOOL allOK=FALSE;

	error = ParseDeclaration(	SIR, declStr, varName1, varName2, &value, 
														varContents, &type, FALSE);
	if (error==0)
	{
		allOK=TRUE;
		strcpy(this_vir->vir_Name, varName1);

		if ( type==1 )			// VAR=VALUE
		{
			this_vir->vir_Type = VCT_INTEGER;
			this_vir->vir_Integer = value;
		}
		else if ( type==2 )	// VAR="STRING"
		{
			this_vir->vir_Type = VCT_STRING;
			strcpy(this_vir->vir_String, varContents);
		}
	}
	else
		ProcessParseError( error );

	return( allOK );
}

/******** VIRToDecl() ********/

void VIRToDecl(VIR *this_vir, STRPTR declStr)
{
	if ( this_vir->vir_Type == VCT_INTEGER )
		sprintf(declStr, "%s = %d", this_vir->vir_Name, this_vir->vir_Integer);
	else if ( this_vir->vir_Type == VCT_STRING )
		sprintf(declStr, "%s = \"%s\"", this_vir->vir_Name, this_vir->vir_String);
}

/******** FindVIR() ********/
/*
 * Return VIR with varName, or NULL
 *
 */

VIR *FindVIR(struct ScriptInfoRecord *SIR, STRPTR varName, int *num)
{
VIR *this_vir, *found_vir=NULL;

	if ( !varName )
		return(NULL);

	*num=0;
	for(this_vir = (VIR *)SIR->VIList.lh_Head; 
			(VIR *)this_vir->vir_Node.ln_Succ;	
			this_vir = (VIR *)this_vir->vir_Node.ln_Succ)
	{
		if ( !stricmp(varName, this_vir->vir_Name) )
		{
			found_vir = this_vir;
			*num = *num + 1;
		}
	}

	return( found_vir );
}

/******** ExprToVAR() ********/

BOOL ExprToVAR(	struct ScriptInfoRecord *SIR, STRPTR declStr, VAR *this_var,
								STRPTR labelStr )
{
int value1,value2,type,oper,jump,error,num;
int value3,value4,type2,oper2;
char	varName1[EXPRESSION_WIDTH], varName2[EXPRESSION_WIDTH], varName3[EXPRESSION_WIDTH],
			varContents1[EXPRESSION_WIDTH], varContents2[EXPRESSION_WIDTH], labelName[EXPRESSION_WIDTH];
char	varName4[EXPRESSION_WIDTH], varName5[EXPRESSION_WIDTH], varName6[EXPRESSION_WIDTH],
			varContents3[EXPRESSION_WIDTH], varContents4[EXPRESSION_WIDTH];
BOOL allOK=FALSE;
VCR *vcr;

	error = ParseExpression(	SIR,
														declStr,
														varName1, varName2, varName3,
														&value1, &value2,
														varContents1, varContents2,
														&oper, &type, labelName, &jump,
														varName4, varName5, varName6,
														&value3, &value4,
														varContents3, varContents4,
														&oper2, &type2  );
	if ( error==0 )
	{
		allOK=TRUE;

		if ( type>=0 && type<=11 )	// EXPRESSION OR DECLARATION, NOT IF
		{
			if ( !FillInThisVarA(this_var,oper,type,varName1,varName2,varName3,varContents1,varContents2,value1,value2,SIR) )
				return(FALSE);
		}
		else if ( type>=12 && type<=16 )	// IF STATEMENTS
		{
			// allocate VCR (VarCondRecord)

			vcr = (VCR *)AllocMem(sizeof(VCR),MEMF_CLEAR | MEMF_ANY);
			if ( !vcr )
				return( FALSE );
			else
				AddTail(&this_var->var_CondList, (struct Node *)vcr);

			this_var->var_CheckBeforeAssign = FALSE;
			this_var->var_Operator = 0;								// NO OPER
			this_var->var_SourceBits = SB_NOSOURCES;	// WAS 0;

			// fill in VAR (VarAssignRecord) - IF VAR1 < VAR2 ETC. -> VAR1 IN VAR
			//																										 -> VAR2 IN VCR

			this_var->var_ResultVIR = FindVIR(SIR, varName1, &num);
			if ( num!=1 )
			{
				Message("You can't put a value or string before a condition.");
				FreeMem(vcr,sizeof(VCR));
				return(FALSE);
			}

			this_var->var_Type2 = VCT_EMPTY;	// NO 2ND VAR IN A CONDITION

			// fill in VCR

			switch( oper )
			{
				case  5:	vcr->vcr_Condition = COND_LESS; 				break;
				case  6:	vcr->vcr_Condition = COND_LESSEQUAL;		break;
				case  7:	vcr->vcr_Condition = COND_GREATER; 			break;
				case  8:	vcr->vcr_Condition = COND_GREATEREQUAL; break;
				case  9:	vcr->vcr_Condition = COND_NOTEQUAL; 		break;
				case 10:	vcr->vcr_Condition = COND_EQUAL;				break;
			}

			vcr->vcr_CheckBits = 0;

			vcr->vcr_JumpType = TG_NOJUMP;
			switch( jump )
			{
				case  1:	vcr->vcr_JumpType = TG_GOTO;						break;
				case  2:	vcr->vcr_JumpType = TG_GOTO;						break;	// was gosub, now no longer supported, make it goto
				case  3:	vcr->vcr_JumpType = TG_SPPREVGOSUB;			break;
				case  4:	vcr->vcr_JumpType = TG_SPNEXTGOSUB;			break;
				case  5:	vcr->vcr_JumpType = TG_MAKE;						break;
			}

			if ( type==13 || type==15 || type==16 )
			{
				vcr->vcr_Check.vc_VIR = FindVIR(SIR, varName2, &num);
				if ( num!=1 )
				{
					Message("Undeclared variable used. (1)");
					FreeMem(vcr,sizeof(VCR));
					return(FALSE);
				}
				vcr->vcr_CheckBits = VC_CHECKVIR;
			}
			else if ( type==12 )
			{
				vcr->vcr_Type = VCT_INTEGER;
				vcr->vcr_Integer = value2;
			}
			else if ( type==14 )
			{
				vcr->vcr_Type = VCT_STRING;
				strcpy(vcr->vcr_String, varContents2);
			}

			if ( vcr->vcr_JumpType != TG_MAKE )
				RemoveQuotes( labelName );

			// NEW 
			if ( vcr->vcr_JumpType != TG_MAKE )
				vcr->vcr_LabelSNR = FindLabel(SIR,labelName);
			// NEW 

			strcpy( vcr->vcr_LabelName,labelName );

			// NEW 
			if ( vcr->vcr_JumpType != TG_MAKE )
			{
				if ( !vcr->vcr_LabelSNR )
					vcr->vcr_LabelSNR = FindLabel(SIR,mainName);
			}
			// NEW 

			if ( vcr->vcr_JumpType == TG_MAKE )
			{
				if ( !FillInThisVarB(this_var,oper2,type2,varName4,varName5,varName6,varContents3,varContents4,value3,value4,SIR) )
					return(FALSE);
			}

			strcpy(labelStr,labelName);
		}
	}
	else
		ProcessParseError( error );

	return( allOK );
}

/******** VARToExpr() ********/

BOOL VARToExpr(struct ScriptInfoRecord *SIR, STRPTR declStr, VAR *this_var)
{
//BOOL allOK=FALSE;
VCR *vcr;
TEXT temp[100];

	if (	this_var->var_CondList.lh_TailPred !=
				(struct Node *)&this_var->var_CondList )	// cond. list is NOT empty
	{
		vcr = (VCR *)this_var->var_CondList.lh_Head;
		if ( vcr )	// THIS IS A CONDITION EXPRESSION
		{
			// CREATE 'IF ...' PART 

			sprintf(declStr, "IF %s ", this_var->var_ResultVIR->vir_Name);

			// CREATE CONDITION PART (<, > etc )

			if ( vcr->vcr_Condition == COND_LESS )
				strcat(declStr, "< ");			
			else if ( vcr->vcr_Condition == COND_LESSEQUAL )
				strcat(declStr, "<= ");			
			else if ( vcr->vcr_Condition == COND_EQUAL )
				strcat(declStr, "= ");			
			else if ( vcr->vcr_Condition == COND_GREATEREQUAL )
				strcat(declStr, ">= ");			
			else if ( vcr->vcr_Condition == COND_GREATER )
				strcat(declStr, "> ");			
			else if ( vcr->vcr_Condition == COND_NOTEQUAL )
				strcat(declStr, "<> ");			

			// CREATE AFTER CONDITION PART (IF ... COND ...)

			if ( vcr->vcr_CheckBits & VC_CHECKVIR )
			{
				sprintf(temp, "%s ", vcr->vcr_Check.vc_VIR->vir_Name);
				strcat(declStr, temp);
			}
			else
			{
				if ( vcr->vcr_Type == VCT_INTEGER )
				{
					sprintf(temp, "%d ", vcr->vcr_Integer);
					strcat(declStr, temp);
				}
				else if ( vcr->vcr_Type == VCT_STRING )
				{
					sprintf(temp, "\"%s\" ", vcr->vcr_String);
					strcat(declStr, temp);
				}
			}

			// CREATE GOTO

			if ( vcr->vcr_JumpType == TG_GOTO )
				strcat(declStr, "GOTO ");
			//else if ( vcr->vcr_JumpType == TG_GOSUB )
			//	strcat(declStr, "GOSUB ");
			else if ( vcr->vcr_JumpType == TG_SPNEXTGOSUB )
				strcat(declStr, "GONEXT ");
			else if ( vcr->vcr_JumpType == TG_SPPREVGOSUB )
				strcat(declStr, "GOPREV ");
			else if ( vcr->vcr_JumpType == TG_MAKE )
				strcat(declStr, "MAKE ");
			else
				strcat(declStr, "*ERROR* ");

			// CREATE LABEL TO JUMP TO

			if ( vcr->vcr_JumpType != TG_MAKE )
				strcat(declStr,"\"");
			
			// NEW 
			if ( vcr->vcr_JumpType != TG_MAKE )
				strcat(declStr, vcr->vcr_LabelSNR->objectName);
			else
				strcat(declStr, vcr->vcr_LabelName );
			// NEW 

			if ( vcr->vcr_JumpType != TG_MAKE )
				strcat(declStr,"\"");

			if ( vcr->vcr_JumpType == TG_MAKE )
				CreateDeclStrB(declStr,this_var);
		}
	}
	else	// THIS IS AN EXPRESSION (NOT A CONDITION)
	{
		CreateDeclStrA(declStr,this_var);
	}

	return( TRUE );	//allOK );
}

/******** DoDecla() ********/

int DoDecla(STRPTR parseStr, int *oper, int *type,
						STRPTR token1, STRPTR token2, STRPTR token3)
{
int pos, arg, len, len2, len3, i;
char *p;

	token1[0]='\0';
	token2[0]='\0';
	token3[0]='\0';

	pos = stcarg(parseStr, "=");
	if (pos==strlen(parseStr))
		return(1);

	p=parseStr;
	arg=0;
	len2=0;
	len3=strlen(p);
	while(1)
	{
		if ( !p[0] )
			break;
		len = stcarg(p,"+-*/=");
		if (len==0)
			break;

		if ( len2>=len3+1 )
			break;

		if ( arg==0 && *(p+len) != '=' )	// Check if this is a=b+c
			return(1);
		else
		{
			if ( *(p+len) == '+' )
				*oper = 1;
			else if ( *(p+len) == '-' )
				*oper = 2;
			else if ( *(p+len) == '*' )
				*oper = 3;
			else if ( *(p+len) == '/' )
				*oper = 4;
		}

		if ( arg==0 )
			stccpy(token1,p,len+1);
		else if ( arg==1 )
			stccpy(token2,p,len+1);
		else if ( arg==2 )
			stccpy(token3,p,len+1);

		len2 += (len+1);

		i=0;
		while( *(p+len+1+i) == ' ' )
			i++;

		p=p+(len+1);

		if ( arg==2 )
			break;

		arg++;
	}

	TrimVarName(token1);
	TrimVarName(token2);
	TrimVarName(token3);

	if ( isdigit( token2[0] ) )
	{
		if ( isdigit( token3[0] ) )
			*type = 10;
		else if ( token3[0]=='\"' )
			*type = 8;
		else if ( token3[0] )
			*type = 4;
		else
			*type = 1;	// var = value
	}		
	else if ( token2[0]=='\"' )
	{
		if ( isdigit( token3[0] ) )
			*type = 7;
		else if ( token3[0]=='\"' )
			*type = 11;
		else if ( token3[0] )
			*type = 6;
		else
			*type = 2;	// var = "string"
	}		
	else
	{
		if ( isdigit( token3[0] ) )
			*type = 3;
		else if ( token3[0]=='\"' )
			*type = 5;
		else if ( token3[0] )
			*type = 9;
		else
			*type = 0;	// var = var
	}		

	return(0);
}

/******** CheckVarType() ********/

int CheckVarType(	int type,
									STRPTR varName1, STRPTR varName2, STRPTR varName3,
									STRPTR varContents1, STRPTR varContents2,
									STRPTR token1, STRPTR token2, STRPTR token3, STRPTR token4, 
									int *value1, int *value2 )
{
	switch( type )
	{
		case 0:		strcpy(varName1,token1);
							strcpy(varName2,token2);
							if ( varName1[0]=='\0' || varName2[0]=='\0' )
								return(-1);
							break;

		case 1:		strcpy(varName1,token1);
							sscanf(token2,"%d",value1);
							if ( varName1[0]=='\0' || *value1==-1 )
								return(-1);
							break;

		case 2:		strcpy(varName1,token1);
							strcpy(varContents1,token2);
							if ( varName1[0]=='\0' || varContents1[0]=='\0' )
								return(-1);
							RemoveQuotes(varContents1);
							break;

		case 3:		strcpy(varName1,token1);
							strcpy(varName2,token2);
							sscanf(token3,"%d",value2);
							if ( varName1[0]=='\0' || varName2[0]=='\0' || *value2==-1 )
								return(-1);
							break;

		case 4:		strcpy(varName1,token1);
							sscanf(token2,"%d",value1);
							strcpy(varName3,token3);
							if ( varName1[0]=='\0' || *value1==-1 || varName3[0]=='\0' )
								return(-1);
							break;

		case 5:		strcpy(varName1,token1);
							strcpy(varName2,token2);
							strcpy(varContents2,token3);
							if ( varName1[0]=='\0' || varName2[0]=='\0' || varContents2[0]=='\0' )
								return(-1);
							RemoveQuotes(varContents2);
							break;

		case 6:		strcpy(varName1,token1);
							strcpy(varContents1,token2);
							strcpy(varName3,token3);
							if ( varName1[0]=='\0' || varContents1[0]=='\0' || varName3[0]=='\0' )
								return(-1);
							RemoveQuotes(varContents1);
							break;

		case 7:		strcpy(varName1,token1);
							strcpy(varContents1,token2);
							sscanf(token3,"%d",value2);
							if ( varName1[0]=='\0' || varContents1[0]=='\0' || *value2==-1 )
								return(-1);
							RemoveQuotes(varContents1);
							break;

		case 8:		strcpy(varName1,token1);
							sscanf(token2,"%d",value1);
							strcpy(varContents2,token3);
							if ( varName1[0]=='\0' || *value1==-1 || varContents2[0]=='\0' )
								return(-1);
							RemoveQuotes(varContents2);
							break;

		case 9:		strcpy(varName1,token1);
							strcpy(varName2,token2);
							strcpy(varName3,token3);
							if ( varName1[0]=='\0' || varName2[0]=='\0' || varName3[0]=='\0' )
								return(-1);
							break;

		case 10:	strcpy(varName1,token1);
							sscanf(token2,"%d",value1);
							sscanf(token3,"%d",value2);
							if ( varName1[0]=='\0' || *value1==-1 || *value2==-1 )
								return(-1);
							break;

		case 11:	strcpy(varName1,token1);
							strcpy(varContents1,token2);
							strcpy(varContents2,token3);
							if ( varName1[0]=='\0' || varContents1[0]=='\0' || varContents2[0]=='\0' )
								return(-1);
							RemoveQuotes(varContents1);
							RemoveQuotes(varContents2);
							break;

		case 12:	strcpy(varName1,token2);			// IF 
							sscanf(token4,"%d",value2);
							if ( varName1[0]=='\0' || *value2==-1 )
								return(-1);
							break;

		case 13:	strcpy(varName1,token2);			// IF
							strcpy(varName2,token4);
							if ( varName1[0]=='\0' || varName2[0]=='\0' )
								return(-1);
							break;

		case 14:	strcpy(varName1,token2);			// IF
							strcpy(varContents2,token4);
							if ( varName1[0]=='\0' || varContents2[0]=='\0' )
								return(-1);
							RemoveQuotes(varContents2);
							break;

		case 15:	sscanf(token2,"%d",value1);		// IF
							strcpy(varName2,token4);
							if ( *value1==-1 || varName2[0]=='\0' )
								return(-1);
							break;

		case 16:	strcpy(varContents1,token2);	// IF
							strcpy(varName2,token4);
							if ( varContents1[0]=='\0' || varName2[0]=='\0' )
								return(-1);
							RemoveQuotes(varContents1);
							break;

		default:	return(-1);	// syntax error
	}

	return(0);
}

/******** FillInThisVarA() ********/

BOOL FillInThisVarA(	VAR *this_var, int oper, int type, STRPTR varName1, STRPTR varName2, STRPTR varName3,
											STRPTR varContents1, STRPTR varContents2,
											int value1, int value2,
											struct ScriptInfoRecord *SIR )
{
int num;

	this_var->var_CheckBeforeAssign = FALSE;

	if ( oper==1 )
		this_var->var_Operator = OPER_ADD;			
	else if ( oper==2 )
		this_var->var_Operator = OPER_SUBTRACT;			
	else if ( oper==3 )
		this_var->var_Operator = OPER_MULTIPLY;			
	else if ( oper==4 )
		this_var->var_Operator = OPER_DIVIDE;			

	this_var->var_SourceBits = 0;

	this_var->var_ResultVIR = (VIR *)FindVIR(SIR, varName1, &num);
	if ( num!=1 )
	{
		Message("Undeclared variable used. (2)");
		return(FALSE);
	}

	// VAR1

	if ( type==0 || type==3 || type==5 || type==9 )
	{
		this_var->var_Source1.vs_VIR = FindVIR(SIR, varName2, &num);
		if ( num!=1 )
		{
			Message("Undeclared variable used. (3)");
			return(FALSE);
		}
		this_var->var_SourceBits |= SB_SOURCE1VIR;
	}
	else if ( type==1 || type==4 || type==8 || type==10 )
	{
		this_var->var_Type1 = VCT_INTEGER;
		this_var->var_Integer1 = value1;
	}
	else if ( type==2 || type==6 || type==7 || type==11 )
	{
		this_var->var_Type1 = VCT_STRING;
		strcpy(this_var->var_String1, varContents1);
	}

	// VAR2

	if ( type==4 || type==6 || type==9 )
	{
		this_var->var_Source2.vs_VIR = FindVIR(SIR, varName3, &num);
		if ( num!=1 )
		{
			Message("Undeclared variable used. (4)");
			return(FALSE);
		}
		this_var->var_SourceBits |= SB_SOURCE2VIR;
	}
	else if ( type==3 || type==7 || type==10 )
	{
		this_var->var_Type2 = VCT_INTEGER;
		this_var->var_Integer2 = value2;
	}
	else if ( type==5 || type==8 || type==11 )
	{
		this_var->var_Type2 = VCT_STRING;
		strcpy(this_var->var_String2, varContents2);
	}
	else
	{
		this_var->var_Type2 = VCT_EMPTY;	// DECLARATION NOT AN EXPRESSION
		this_var->var_Operator = OPER_ADD;			
	}

	return(TRUE);
}

/******** FillInThisVarB() ********/

BOOL FillInThisVarB(	VAR *this_var, int oper, int type, STRPTR varName1, STRPTR varName2, STRPTR varName3,
											STRPTR varContents1, STRPTR varContents2,
											int value1, int value2,
											struct ScriptInfoRecord *SIR )
{
int num;

	this_var->var_CheckBeforeAssign = FALSE;

	if ( oper==1 )
		this_var->var_Operator_v2 = OPER_ADD;			
	else if ( oper==2 )
		this_var->var_Operator_v2 = OPER_SUBTRACT;			
	else if ( oper==3 )
		this_var->var_Operator_v2 = OPER_MULTIPLY;			
	else if ( oper==4 )
		this_var->var_Operator_v2 = OPER_DIVIDE;			

	this_var->var_SourceBits_v2 = 0;

	this_var->var_ResultVIR_v2 = (VIR *)FindVIR(SIR, varName1, &num);
	if ( num!=1 )
	{
		Message("Undeclared variable used. (5)");
		return(FALSE);
	}

	// VAR1

	if ( type==0 || type==3 || type==5 || type==9 )
	{
		this_var->var_Source3.vs_VIR = FindVIR(SIR, varName2, &num);
		if ( num!=1 )
		{
			Message("Undeclared variable used. (6)");
			return(FALSE);
		}
		this_var->var_SourceBits_v2 |= SB_SOURCE3VIR;
	}
	else if ( type==1 || type==4 || type==8 || type==10 )
	{
		this_var->var_Type3 = VCT_INTEGER;
		this_var->var_Integer3 = value1;
	}
	else if ( type==2 || type==6 || type==7 || type==11 )
	{
		this_var->var_Type3 = VCT_STRING;
		strcpy(this_var->var_String3, varContents1);
	}

	// VAR2

	if ( type==4 || type==6 || type==9 )
	{
		this_var->var_Source4.vs_VIR = FindVIR(SIR, varName3, &num);
		if ( num!=1 )
		{
			Message("Undeclared variable used. (7)");
			return(FALSE);
		}
		this_var->var_SourceBits_v2 |= SB_SOURCE4VIR;
	}
	else if ( type==3 || type==7 || type==10 )
	{
		this_var->var_Type4 = VCT_INTEGER;
		this_var->var_Integer4 = value2;
	}
	else if ( type==5 || type==8 || type==11 )
	{
		this_var->var_Type4 = VCT_STRING;
		strcpy(this_var->var_String4, varContents2);
	}
	else
	{
		this_var->var_Type4 = VCT_EMPTY;	// DECLARATION NOT AN EXPRESSION
		this_var->var_Operator_v2 = OPER_ADD;			
	}

	return(TRUE);
}

/******** CreateDeclStrA() ********/

void CreateDeclStrA(STRPTR declStr, VAR *this_var)
{
TEXT temp[100];

	// CREATE 'VAR1 = '
	
	sprintf(declStr, "%s = ", this_var->var_ResultVIR->vir_Name);

	// CREATE 'VAR1 = VAR2'

	if ( this_var->var_SourceBits & SB_SOURCE1VIR )
	{
		sprintf(temp, "%s ", this_var->var_Source1.vs_VIR->vir_Name);
		strcat(declStr, temp);
	}
	else
	{
		if ( this_var->var_Type1 == VCT_INTEGER )
		{
			sprintf(temp, "%d ", this_var->var_Integer1);
			strcat(declStr, temp);
		}
		else if ( this_var->var_Type1 == VCT_STRING )
		{
			sprintf(temp, "\"%s\" ", this_var->var_String1);
			strcat(declStr, temp);
		}
	}

	// 'EXPRESSIONS' CAN BE DECLARATIONS OR EXPRESSIONS

	if ( this_var->var_Type2 != VCT_EMPTY )
	{
		// CREATE 'VAR1 = VAR2 + '

		if ( this_var->var_Operator == OPER_ADD )
			strcat(declStr, "+ ");
		else if ( this_var->var_Operator == OPER_SUBTRACT )
			strcat(declStr, "- ");
		else if ( this_var->var_Operator == OPER_MULTIPLY )
			strcat(declStr, "* ");
		else if ( this_var->var_Operator == OPER_DIVIDE )
			strcat(declStr, "/ ");

		// CREATE 'VAR1 = VAR2 + VAR3'

		if ( this_var->var_SourceBits & SB_SOURCE2VIR )
		{
			sprintf(temp, "%s", this_var->var_Source2.vs_VIR->vir_Name);
			strcat(declStr, temp);
		}
		else
		{
			if ( this_var->var_Type2 == VCT_INTEGER )
			{
				sprintf(temp, "%d", this_var->var_Integer2);
				strcat(declStr, temp);
			}
			else if ( this_var->var_Type2 == VCT_STRING )
			{
				sprintf(temp, "\"%s\"", this_var->var_String2);
				strcat(declStr, temp);
			}
		}
	}
}

/******** CreateDeclStrB() ********/

void CreateDeclStrB(STRPTR declStr, VAR *this_var)
{
TEXT temp[100];
TEXT buf[100];

	// CREATE 'VAR1 = '
	
	sprintf(buf, "%s = ", this_var->var_ResultVIR_v2->vir_Name);

	// CREATE 'VAR1 = VAR2'

	if ( this_var->var_SourceBits_v2 & SB_SOURCE3VIR )
	{
		sprintf(temp, "%s ", this_var->var_Source3.vs_VIR->vir_Name);
		strcat(buf, temp);
	}
	else
	{
		if ( this_var->var_Type3 == VCT_INTEGER )
		{
			sprintf(temp, "%d ", this_var->var_Integer3);
			strcat(buf, temp);
		}
		else if ( this_var->var_Type3 == VCT_STRING )
		{
			sprintf(temp, "\"%s\" ", this_var->var_String3);
			strcat(buf, temp);
		}
	}

	// 'EXPRESSIONS' CAN BE DECLARATIONS OR EXPRESSIONS

	if ( this_var->var_Type4 != VCT_EMPTY )
	{
		// CREATE 'VAR1 = VAR2 + '

		if ( this_var->var_Operator_v2 == OPER_ADD )
			strcat(buf, "+ ");
		else if ( this_var->var_Operator_v2 == OPER_SUBTRACT )
			strcat(buf, "- ");
		else if ( this_var->var_Operator_v2 == OPER_MULTIPLY )
			strcat(buf, "* ");
		else if ( this_var->var_Operator_v2 == OPER_DIVIDE )
			strcat(buf, "/ ");

		// CREATE 'VAR1 = VAR2 + VAR3'

		if ( this_var->var_SourceBits_v2 & SB_SOURCE4VIR )
		{
			sprintf(temp, "%s", this_var->var_Source4.vs_VIR->vir_Name);
			strcat(buf, temp);
		}
		else
		{
			if ( this_var->var_Type4 == VCT_INTEGER )
			{
				sprintf(temp, "%d", this_var->var_Integer4);
				strcat(buf, temp);
			}
			else if ( this_var->var_Type4 == VCT_STRING )
			{
				sprintf(temp, "\"%s\"", this_var->var_String4);
				strcat(buf, temp);
			}
		}
	}

	strcat(declStr,buf);
}

/******** E O F ********/
