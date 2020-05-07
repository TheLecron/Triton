#pragma once

#include <memory>
#include <vector>

typedef long long			lint;

#define	StringSize			256

char	lcs(char ch);
char	ucs(char ch);
int		byp(const char * ptr,int pos);

//extern DateType		DateFormat;

#define SECURE_CHAR '*'

typedef std::vector<struct STRING>	vSTRING;
typedef std::vector<vSTRING>		vvSTRING;

struct STRING
{
	char *	String;
	int		Size;


			STRING()					:	String(new char[StringSize]), Size(StringSize)	{	memset(String,0,Size);													}
			STRING(const char * _str)	:	String(new char[StringSize]), Size(StringSize)	{	int len = STRLEN(_str);	Grow(len+1); memcpy(String,_str,len+1);			}
			STRING(const STRING & _str)	:	String(new char[StringSize]), Size(StringSize)	{	operator=(_str);														}
			~STRING()																		{	delete [] String;														}

	void		Grow(int _size);

	int			CMP(char _chr1,char _chr2)								{	return(_chr1 - _chr2);																		}
	int			STRCMP(const char * _str1,const char * _str2)			{	for(int pos=0;				(_str1[pos] || _str2[pos]);pos++) if(int res = CMP(		_str1[pos] ,	_str2[pos]))	return(res); return(0);	}
	int			STRCMPA(const char * _str1,const char * _str2)			{	for(int pos=0;				(_str1[pos] || _str2[pos]);pos++) if(int res = CMP(ucs( _str1[pos]),ucs(_str2[pos])))	return(res); return(0);	}
	int			STRNCMP(const char * _str1,const char * _str2,int _max)	{	for(int pos=0;pos<_max &&	(_str1[pos] || _str2[pos]);pos++) if(int res = CMP(		_str1[pos] ,	_str2[pos]))	return(res); return(0);	}
	int			STRLEN(const char * _str1)								{	for(int pos=0;;pos++)	if( _str1[pos]					== 0) return(pos);					}
	int			STRCPY(		 char * _str1,const char * _str2)			{	for(int pos=0;;pos++)	if((_str1[pos] = _str2[pos])	== 0) return(pos);					}
	int			STRCAT(		 char * _str1,const char * _str2)			{	return(STRCPY(&_str1[STRLEN(_str1)],_str2));												}

	char &		operator[](int _pos)			{	Grow(_pos + 1);	return(String[_pos]);	}

	bool		operator==(const STRING & _str)	{	return(STRCMP(String,_str.String) == 0);	}
	bool		operator==(const char * _str)	{	return(STRCMP(String,_str		) == 0);	}

	bool		operator!=(const STRING & _str)	{	return(STRCMP(String,_str.String) != 0);	}
	bool		operator!=(const char * _str)	{	return(STRCMP(String,_str		) != 0);	}

	STRING &	operator=(const STRING & _str)	{	int len = STRLEN(_str.String);	Grow(len + 1); memcpy(String,_str.String,len + 1);	return(*this);	}
	STRING &	operator=(const char * _str)	{	int len = STRLEN(_str		);	Grow(len + 1); memcpy(String,_str,		 len + 1);	return(*this);	}
	STRING &	operator=(char _ch)				{	String[0] = _ch; String[1] = 0;														return(*this);	}

	STRING &	operator+=(const STRING & _str)	{	int len1 = Length(),len2 = STRLEN(_str.String);	Grow(len1 + len2 +  2); memcpy(&String[len1],_str.String,len2 + 1);		return(*this);	}
	STRING &	operator+=(const char * _str)	{	int len1 = Length(),len2 = STRLEN(_str);		Grow(len1 + len2 +  2); memcpy(&String[len1],_str,		len2 + 1);		return(*this);	}
	STRING &	operator+=(const char _ch)		{	int len1 = Length();							Grow(len1        +  2); String[len1++] = _ch; String[len1] = 0x00;		return(*this);	}
	STRING &	operator+=(const double _value)	{	STRING tmp;	operator+=(tmp.ConvertFloat(_value,10));																	return(*this);	}

	STRING &	ConvertFloat(const double _value,int _digits);
	STRING &	Format(const char * _fmt, ...);
	int			Rows(void);
	int			Length(void)							{	return(STRLEN(String));																						}
	int			Value(void)								{	return(atoi(String));																						}
	void		Strip(void)								{	for(int len=Length();len > 0 && String[--len] <= 0x20;) String[len] = 0x00;									}
	void		Secure(void)							{	for(int pos=0;String[pos];pos++) String[pos] = SECURE_CHAR;													}
	void		Center(int _width);
	STRING &	Fill(const char _ch,int _count)			{	Grow(_count+2); memset(String,_ch,_count); String[_count] = 0x00;							return(*this);	}
	void		LeftPad(int _length,char _pad_char);
	void		RightPad(int _length,char _pad_char);
	int			byp(int _pos=0)							{	for(;String[_pos] == 0x09 || String[_pos] == 0x20;_pos++);	return(_pos);									}
	int			lastIndexOf(const char * _str);
	STRING &	Replace(char _old,char _new)			{	for(int idx=0;String[idx];idx++)	if(String[idx] == _old)	String[idx] = _new;				return(*this);	}
	STRING &	Dollars(long long _cents,bool _signed=false);
	STRING &	Time(void);
	STRING &	Date(void);
	STRING &	TimeStamp(void);
	STRING &	Reverse(void);
	bool		CompareUCS(const STRING & _str)			{	return(STRCMPA(String,_str.String) == 0);																	}
	void		Upper(void)								{	for(int idx=0;String[idx];idx++) String[idx] = ucs(String[idx]);											}
	int			indexOf(const char * _str);
	int			lastIndexOf(char * _str);
	int			Split(char _ch,vSTRING & _vstr);
	int			Parse(char _ch,vvSTRING & _vvstr);
};

int		PassInteger		(char * _ptr,int _pos,const int    & _value);
int		ReceiveInteger	(char * _ptr,int _pos,		int    & _value);
int		PassBoolean		(char * _ptr,int _pos,const bool   & _value);
int		ReceiveBoolean	(char * _ptr,int _pos,		bool   & _value);
int		PassFloat		(char * _ptr,int _pos,const double & _value);
int		ReceiveFloat	(char * _ptr,int _pos,		double & _value);
int		PassString		(char * _ptr,int _pos,const STRING & _str);
int		ReceiveString	(char * _ptr,int _pos,		STRING & _str);

int		SkipParm		(const char * _str,int _pos);
int		SkipParm		(STRING & _str,int _pos);
int		NextParm		(const char * _str,int _pos);
int		NextParm		(STRING & _str,int _pos);

int		GetIntParm		(STRING & string,int pos,   int & dest);
int		GetBoolParm		(STRING & string,int pos,  bool & dest);
int		GetFloatParm	(STRING & string,int pos,double & dest);
int		GetStrParm		(STRING & string,int pos,STRING & dest);
int		GetBigParm		(STRING & string,int pos,  lint & dest);

#undef StrCpy
#undef StrCat
#undef StrCmp

int		StrCpy( char   * str1,const  char   * str2);
int		StrCpy( char   * str1,const STRING  & str2);
int		StrCpy(wchar_t * str1,const  char   * str2);
int		StrCpy(wchar_t * str1,const STRING  & str2);
int		StrNCpy(char    * _out,const  char  * _in,int _max);
int		StrNCpy(wchar_t * _out,const  char  * _in,int _max);
int		StrLen(const  char   * str);
int		StrLen(const wchar_t * str);
int		StrCat( char   * str1,const  char   * str2);
int		StrCmp(const  char *	str1,const char		* str2);
int		StrCmpA(const char *	str1,const char * str2);
int		StrNCmp(const char *	str1,const char * str2,int maxlen);
int		StrNCmpA(const char *	str1,const char * str2,int maxlen);
int		Strip( char   * _str);
int		Strip(wchar_t * _str);

int		StartsWith(const char * _buffer,const char * _substr);

//void	ParseTime(const char * _str,DBTime * _time);
//void	ParseDate(const char * _str,DBDate * _date);
//void	ParseTimeStamp(const char * _str,DBTimeStamp * _timestamp);

//void			SetDateFormat(DateType _format);
int				GetHour(int _hour);
const char *	GetAMPM(int _hour);

bool LaunchThread(LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter);
