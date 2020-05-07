#include <Windows.h>
#include <stdio.h>
#include <stdarg.h>
#include "STRING.h"
/*
VerInfo				ProgramVersion =	{
											1,		// Major
											0,		// Minor
											101,	// Sub
											0		// Edit
										};

DateType			DateFormat;
*/
typedef long long			lint;
#define NET_BUFFER_SIZE 10240

int StrCpy( char   * str1,const  char   * str2)					{	for(int n=0;;n++)		if((str1[n] = str2[n]) == 0x00)							return(n);	}
int StrCpy( char   * str1,const STRING  & str2)					{	return(StrCpy(str1,str2.String));															}
int StrCpy(wchar_t * str1,const  char   * str2)					{	for(int n=0;;n++)		if((str1[n] = str2[n]) == 0x00)							return(n);	}
int StrCpy(wchar_t * str1,const STRING  & str2)					{	return(StrCpy(str1,str2.String));															}

int StrNCpy(char    * _out,const  char  * _in,int _max)			{	for(int n=0;;n++) if(n>=_max || (_out[n] = _in[n]) == 0x00)						return(n);	}
int StrNCpy(wchar_t * _out,const  char  * _in,int _max)			{	for(int n=0;;n++) if(n>=_max || (_out[n] = _in[n]) == 0x00)						return(n);	}

int StrLen(const  char   * str)									{	for(int n=0;;n++)		if(str[n] == 0x00)										return(n);	}
int StrLen(const wchar_t * str)									{	for(int n=0;;n++)		if(str[n] == 0x00)										return(n);	}

int StrCat( char   * str1,const  char   * str2)					{	int n = StrLen(str1);	for(int m=0;;m++)	if((str1[m+n] = str2[m]) == 0x00)	return(m);	}

int StrCmp(const  char *	str1,const char		* str2)			{	for(int n=0;str1[n] || str2[n];n++)		{	if(str1[n] < str2[n])				return(-1);	if(str1[n] > str2[n])			return(1);	}	return(0);	}

int StrNCmp(const char *	str1,const char * str2,int maxlen)
{
	for(int n=0;n < maxlen && (str1[n] || str2[n]);n++)
	{
		if(str1[n] < str2[n])	return(-1);
		if(str1[n] > str2[n])	return(1);
	}
	return(0);
}

int StrNCmpA(const char * str1,const char * str2,int maxlen)
{
	for(int n=0;n < maxlen && (str1[n] || str2[n]);n++)
	{
		if(ucs(str1[n]) < ucs(str2[n]))	return(-1);
		if(ucs(str1[n]) > ucs(str2[n]))	return(1);
	}
	return(0);
}

int StrCmpA(const char * str1,const char * str2)
{
	for(int n=0;str1[n] || str2[n];n++)
	{
		if(ucs(str1[n]) < ucs(str2[n]))	return(-1);
		if(ucs(str1[n]) > ucs(str2[n]))	return(1);
	}
	return(0);
}

char	lcs(char ch)					{	return((ch >= 'A' && ch <= 'Z') ? (char) (ch + ('a' - 'A')) : ch);							}
char	ucs(char ch)					{	return((ch >= 'a' && ch <= 'z') ? (char) (ch - ('a' - 'A')) : ch);							}
int		byp(const char * ptr,int pos)	{	while(ptr[pos] == 0x09 || ptr[pos] == 0x20) pos++;							return(pos);	}
int		Strip( char   * _str)			{	int n = StrLen(_str);	while(n > 0 && _str[n-1] <= 0x20) _str[--n] = 0x00;	return(n);		}
int		Strip(wchar_t * _str)			{	int n = StrLen(_str);	while(n > 0 && _str[n-1] <= 0x20) _str[--n] = 0x00;	return(n);		}

bool Delimiter(char & ch)
{
	switch(ch)
	{
	case '<' :	ch = '>';	return(true);
	case '[' :	ch = ']';	return(true);
	case '(' :	ch = ')';	return(true);
	case '_' :	ch = '_';	return(true);
	case '\"' :	ch = '\"';	return(true);
	}
	ch = 0x20;
	return(false);
}

int SkipParm(const char * str,int pos)
{
char	eop = str[pos];

	if(Delimiter(eop)) pos++;
	while(str[pos] && str[pos] != eop) pos++;
	if(eop != 0x20 && str[pos] == eop) pos++;
	return(pos);
}

int SkipParm(STRING & _str,int _pos) { return(SkipParm(_str.String,_pos)); }

int NextParm(const char * str,int pos)
{
	pos = SkipParm(str,pos);
	return(byp(str,pos));
}

int NextParm(STRING & _str,int _pos) { return(NextParm(_str.String,_pos)); }

int GetStrParm(char * string,int pos,STRING & dest)
{
int		rpos,idx,size;
char	delim;

	pos = byp(string,pos);
	rpos = pos;
	size = SkipParm(string,rpos) - rpos;
	delim = string[rpos];
	if(Delimiter(delim))
	{
		size -= 2;
		rpos++;
	}
	for(idx=0;idx<size;idx++)
	{
		dest.String[idx] = string[rpos+idx];
	}
	dest.String[idx] = 0x00;
	return(NextParm(string,pos));
}
int GetStrParm(STRING & string,int pos,STRING & dest)	{	return(GetStrParm(string.String,pos,dest));		}

int GetBoolParm (char *   string,int pos,  bool & dest)	{	dest = (string[pos] == 'T' || string[pos] == 'Y') ? true : false;	return(NextParm(string,pos));	}
int GetBoolParm (STRING & string,int pos,  bool & dest)	{	return(GetBoolParm (string.String,pos,dest));	}
int GetIntParm  (char *   string,int pos,   int & dest)	{	dest = atoi(&string[pos]);											return(NextParm(string,pos));	}
int GetIntParm  (STRING & string,int pos,   int & dest)	{	return(GetIntParm(string.String,pos,dest));		}
int GetBigParm  (char *   string,int pos,  lint & dest)	{	dest = _atoi64(&string[pos]);										return(NextParm(string,pos));	}
int GetBigParm  (STRING & string,int pos,  lint & dest)	{	return(GetBigParm(string.String,pos,dest));		}
int GetFloatParm(char *   string,int pos,double & dest)	{	dest = atof(&string[pos]);											return(NextParm(string,pos));	}
int GetFloatParm(STRING & string,int pos,double & dest)	{	return(GetFloatParm(string.String,pos,dest));	}

int PassString		(char * _ptr,int _pos,const	STRING	& _str)		{	_pos += StrCpy(&_ptr[_pos],_str) + 1;												return(_pos);	}
int ReceiveString	(char * _ptr,int _pos,		STRING	& _str)		{	_pos += StrCpy(_str.String,&_ptr[_pos]) + 1;										return(_pos);	}
int PassInteger		(char * _ptr,int _pos,const	int		& _value)	{	_pos += sprintf_s(&_ptr[_pos],NET_BUFFER_SIZE-_pos,"%d",_value) + 1;				return(_pos);	}
int ReceiveInteger	(char * _ptr,int _pos,		int		& _value)	{	STRING	tmp;	_pos = ReceiveString(_ptr,_pos,tmp);	_value = atoi(tmp.String);	return(_pos);	}
int PassFloat		(char * _ptr,int _pos,const	double	& _value)	{	_pos += sprintf_s(&_ptr[_pos],NET_BUFFER_SIZE-_pos,"%lf",_value) + 1;				return(_pos);	}
int ReceiveFloat	(char * _ptr,int _pos,		double	& _value)	{	STRING	tmp;	_pos = ReceiveString(_ptr,_pos,tmp);	_value = atof(tmp.String);	return(_pos);	}
int PassBoolean		(char * _ptr,int _pos,const	bool	& _value)	{	_ptr[_pos++] = _value ? 'T' : 'F';													return(_pos);	}
int ReceiveBoolean	(char * _ptr,int _pos,		bool	& _value)	{	_value = _ptr[_pos++] == 'T' ? true : false;										return(_pos);	}

int StartsWith(const char * _buffer,const char * _substr)
{
int	len = StrLen(_substr);

	return(StrNCmpA(_buffer,_substr,len) == 0 ? len : 0);
}
/*
void ParseTime(const char * _str,DBTime * _time)
{
int	h,m,s;
double	f;

	f = atof(_str);
	s = int(f);
	f -= s;
	f *= 1000;
	h = s / 10000; s -= h * 10000;
	m = s /   100; s -= m *   100;

	_time->sep.Hour = h;
	_time->sep.Minute = m;
	_time->sep.Second = s;
	_time->sep.Millseconds = int(f);
}

void ParseDate(const char * _str,DBDate * _date)
{
int	y,m,d;

	d = atoi(_str);
	y = d / 10000;	d -= y * 10000;
	m = d /   100;	d -= m *   100;

	_date->sep.Year = y;
	_date->sep.Month = m;
	_date->sep.Day = d;
}

void ParseTimeStamp(const char * _str,DBTimeStamp * _timestamp)
{
int		y,m,d,hh,mm,ss;
double	f;

	d = atoi(_str);
	y = d / 10000;	d -= y * 10000;
	m = d /   100;	d -= m *   100;

	_timestamp->sep.Year = y;
	_timestamp->sep.Month = m;
	_timestamp->sep.Day = d;



	f = atof(&_str[SkipParm(_str,0)]);
	ss = int(f);
	f -= ss;
	f *= 1000;
	hh = ss / 10000; ss -= hh * 10000;
	mm = ss /   100; ss -= mm *   100;

	_timestamp->sep.Hour = hh;
	_timestamp->sep.Minute = mm;
	_timestamp->sep.Second = ss;
	_timestamp->sep.Millseconds = int(f);
}
*/
int Find(char _ch,STRING & _str)
{
	for(int idx=0;_str.String[idx];idx++)
		if(_str.String[idx] == _ch)
			return(idx);
	return(-1);
}

STRING	Ranks("?A23456789TJQK");
STRING	Suits("?CDHS");
/*
void GetCardString(CardInfo * _cards,int _count,STRING & _str,Purpose _type)
{
	_str = "";
	for(int pos=0,idx=0;idx<_count;idx++)
	{
		if(pos > 0 && _type == SP_FOR_DISPLAY) _str += ' ';
		_str += Ranks.String[_cards[idx].Bits.Rank];
		_str += Suits.String[_cards[idx].Bits.Suit];
	}
}

bool ParseCard(CardInfo * _cards,int _count,char * _str)
{
int		idx,pos,rank,suit;

	for(pos=idx=0;idx<_count;idx++)
	{
		pos = byp(_str,pos);
		rank = Find(_str[pos++],Ranks);
		suit = Find(_str[pos++],Suits);
		if(rank > 0 && suit > 0)
		{
			_cards[idx].Bits.Rank = rank;
			_cards[idx].Bits.Suit = suit;
		}
		else return(false);
	}
	return(true);
}

bool ParseCard(CardInfo * _cards,int _count,STRING * _str) { return(ParseCard(_cards,_count,_str->String)); }

int GetDataTypeSize(DataType _type,int _width=0)
{
	switch(_type)
	{
	default :
	case DT_INTEGER :	return(sizeof(int));
	case DT_MONEY :
	case DT_LONGINT :	return(sizeof(lint));
	case DT_WORD :		return(sizeof(short));
	case DT_BYTE :		return(sizeof(char));
	case DT_CHAR :		return(sizeof(char));
	case DT_BOOLEAN :	return(sizeof(bool));
	case DT_FLOAT :		return(sizeof(double));
	case DT_STRING :	return(_width);
	case DT_SECURE :	return(_width);
	case DT_STAMP :	return(sizeof(DBTimeStamp));
	case DT_DATE :		return(sizeof(DBDate));
	case DT_TIME :		return(sizeof(DBTime));
	case DT_CARD :		return(sizeof(CardInfo));
	case DT_ODOMETER :	return(sizeof(Odometer));
	}
}

void SendParameter(DataType _type,char * _buffer,int & _boff,char * _data)
{
	int size = GetDataTypeSize(_type);
	if(size == 0)
	{
		size = StrCpy(&_buffer[_boff],((STRING *) _data)->String) + 1;
	}
	else memcpy(&_buffer[_boff],_data,size);
	_boff += size;
}

int SendParameters(ParmInfo * _parms,int _count,void * _record,char * _buffer,int _pos)
{
	for(int idx=0;idx<_count;idx++)
	{
		SendParameter(_parms[idx].type,_buffer,_pos,&((char *) _record)[_parms[idx].offset]);
	}
	return(_pos);
}

void ReceiveParameter(DataType _type,char * _buffer,int & _boff,void * _data)
{
	int size = GetDataTypeSize(_type);
	if(size == 0)
	{
		size = StrCpy(((STRING *) _data)->String,&_buffer[_boff]) + 1;
	}
	else memcpy(_data,&_buffer[_boff],size);
	_boff += size;
}

int ReceiveParameters(ParmInfo * _parms,int _count,void * _record,char * _buffer,int _pos)
{
	for(int idx=0;idx<_count;idx++)
	{
		ReceiveParameter(_parms[idx].type,_buffer,_pos,&((char *) _record)[_parms[idx].offset]);
	}
	return(_pos);
}

void ReceiveTextParameter(DataType _type,char * _buffer,int & _boff,char * _data)
{
lint	val;
int		tmp;

	switch(_type)
	{
	case DT_INTEGER :	_boff = GetIntParm(_buffer,_boff,				*((int  *) _data));														break;
	case DT_MONEY :
	case DT_LONGINT :	_boff = GetBigParm(_buffer,_boff,val);			*((lint *) _data) = val;												break;
	case DT_WORD :		_boff = GetIntParm(_buffer,_boff,tmp);			*((short *) _data) = (short) tmp;										break;
	case DT_BYTE :		_boff = GetIntParm(_buffer,_boff,tmp);			*((char  *) _data) = (char)  tmp;										break;
	case DT_CHAR :		*_data = _buffer[_boff++];															_boff = NextParm(_buffer,_boff);	break;
	case DT_BOOLEAN :	_boff = GetBoolParm (_buffer,_boff,				*((bool   *) _data));													break;
	case DT_FLOAT :		_boff = GetFloatParm(_buffer,_boff,				*((double *) _data));													break;
	case DT_STRING :
	case DT_SECURE :	_boff = GetStrParm	(_buffer,_boff,				*((STRING *) _data));													break;
	case DT_CARD :		ParseCard((CardInfo *) _data,1,&_buffer[_boff]);									_boff = NextParm(_buffer,_boff);	break;
	case DT_ODOMETER :	_boff = GetBigParm(_buffer,_boff,val);			((Odometer *) _data)->target = val;										break;
	case DT_DATE :		ParseDate(&_buffer[_boff],(DBDate *) _data);										_boff = NextParm(_buffer,_boff);	break;
	case DT_TIME :		ParseTime(&_buffer[_boff],(DBTime *) _data);										_boff = NextParm(_buffer,_boff);	break;
	case DT_STAMP :		ParseTimeStamp(&_buffer[_boff],(DBTimeStamp *) _data);								_boff = NextParm(_buffer,_boff);	break;
	}
}

int ReceiveTextParameters(ParmInfo * _parms,int _count,void * _record,char * _buffer,int _pos)
{
	for(int idx=0;idx<_count;idx++)
	{
		ReceiveTextParameter(_parms[idx].type,_buffer,_pos,&((char *) _record)[_parms[idx].offset]);
	}
	return(_pos);
}
*/
STRING & STRING::Reverse(void)
{
int	len = Length();

	if(len > 1)
	{
		STRING tmp(String);
		for(int pos=0,idx=--len;idx>=0;idx--,pos++)
		{
			String[pos] = tmp.String[idx];
		}
	}
	return(*this);
}

STRING & STRING::Dollars(long long _cents,bool _signed)
{
STRING	str;
int		len,skip,in,out,idx,sign;

	sign = _cents < 0 ? -1 : 1;
	if(sign == -1) _cents = -_cents;
	str.Format("%03l",_cents);
	len = str.Length();
	skip = (len % 3) + 1;
	in = out = 0;
	String[out++] = '$';
	if(sign == -1) String[out++] = '-';
	for(idx=0;idx<skip;idx++) String[out++] = str.String[in++];
	for(int commas = (len > 5) ? (len / 3) - 1 : 0;commas;commas--)
	{
		String[out++] = ',';
		for(idx=0;idx<3;idx++) String[out++] = str.String[in++];
	}
	String[out++] = '.';
	for(idx=0;idx<2;idx++) String[out++] = str.String[in++];
	String[out] = 0;

	return(*this);
}
/*
void SetDateFormat(DateType _format)
{
	DateFormat = _format;
}

int GetHour(int _hour)
{
	switch(DateFormat)
	{
	case DT_CIVILIAN :
		if(_hour > 12) _hour -= 12;
		if(_hour == 0) _hour  = 12;
	case DT_MILITARY :
	default :
		break;
	}
	return(_hour);
}

const char * GetAMPM(int _hour)
{
	return(DateFormat == DT_CIVILIAN ? (_hour > 11 ? "PM" : "AM") : "");
}

STRING & STRING::Time(void)			{	SYSTEMTIME st;	GetLocalTime(&st);	Format("%02d:%02d:%02d%s",												GetHour(st.wHour),st.wMinute,st.wSecond,GetAMPM(st.wHour));	return(*this);	}
STRING & STRING::Date(void)			{	SYSTEMTIME st;	GetLocalTime(&st);	Format("%02d/%02d/%04d",					st.wMonth,st.wDay,st.wYear);													return(*this);	}
STRING & STRING::TimeStamp(void)	{	SYSTEMTIME st;	GetLocalTime(&st);	Format("%02d/%02d/%04d %02d:%02d:%02d%s",	st.wMonth,st.wDay,st.wYear,	GetHour(st.wHour),st.wMinute,st.wSecond,GetAMPM(st.wHour));	return(*this);	}
*/
int STRING::indexOf(const char * _str)
{
int	len,count;

	len = STRLEN(_str);
	if(len <= Length())
	{
		count = Length() - len + 1;
		for(int idx=0;idx<count;idx++)
		{
			if(StrNCmp(&String[idx],_str,len) == 0)
				return(idx);
		}
	}
	return(-1);
}

int STRING::lastIndexOf(const char * _str)
{
int	idx,len,count;

	len = StrLen(_str);
	if(len <= Length())
	{
		count = Length() - len + 1;
		for(idx=count;idx;)
		{
			if(STRNCMP(&String[--idx],_str,len) == 0)
				return(idx);
		}
	}
	return(-1);
}

int STRING::Split(char _ch,vSTRING & _vstr)
{
STRING	tmp;
int		start,end;

	_vstr.clear();
	tmp.Grow(NET_BUFFER_SIZE);	// hehe
	for(start=byp(0);String[start];)
	{
		if(_ch)
		{
			for(end=start;String[end] && String[end] != _ch;end++);
			memcpy(tmp.String,&String[start],end-start);
			tmp.String[end-start] = 0x00;
			for(start=end;String[start] == _ch;start++);
		}
		else start = GetStrParm(*this,start,tmp);
		_vstr.push_back(tmp);
	}
	return(_vstr.size());
}

void STRING::Grow(int _size)
{
	if(_size > Size)
	{
		_size += StringSize;
		char * tmp = new char[_size];
		memcpy(tmp,String,Size);
		memset(&tmp[Size],0,_size-Size);
		delete [] String;
		String = tmp;
		Size = _size;
	}
}

STRING & STRING::ConvertFloat(const double _value,int _digits)
{
STRING	tmp;
int		dec,sgn,len;

	_fcvt_s(tmp.String,StringSize,_value,_digits,&dec,&sgn);
	len = 0;
	if(sgn) String[len++] = '-';
	memcpy(&String[len],tmp.String,dec);	len += dec;
	String[len++] = '.';
	StrCpy(&String[len],&tmp.String[dec]);
 	for(len = Length();len;)
	{
		switch(String[--len])
		{
		case '0' :	String[len] = 0;				break;
		case '.' :	String[len] = 0;
		default :						len = 0;	break;
		}
	}
	return(*this);
}

void STRING::Center(int _width)
{
	if(Length() < _width)
	{
		Grow(_width);
		_width -= Length();
		_width /= 2;
		if(_width)
		{
			STRING tmp = String;
			Fill(' ',_width);
			operator+=(tmp);
		}
	}
}

void STRING::LeftPad(int _length,char _pad_char)
{
	if(Length() < _length)
	{
		STRING	tmp;
		tmp.Fill(_pad_char,_length-Length());
		tmp += String;
		*this = tmp;
	}
}

void STRING::RightPad(int _length,char _pad_char)
{
	if(Length() < _length)
	{
		STRING	tmp;
		tmp.Fill(_pad_char,_length-Length());
		*this += tmp;
	}
}

const char *	LowerHexChar = "0123456789abcdef";
const char *	UpperHexChar = "0123456789ABCDEF";

int STRING::Rows(void)
{
int	ch,rows = 0;

	for(int off=0;String[off];rows++)
	{
		for(ch = String[off];ch != 0x00 && ch != 0x0D;ch = String[++off]);
		if(ch == 0x0D)
		{
			while(String[++off] == 0x0A);
		}
	}
	return(rows);
}

STRING & STRING::Format(const char * fmt, ...)
{
STRING	tmp;
va_list	args;
bool	zeros;
int		pad_size;
lint	longint;
char	ch,val;

	*String = 0;
	va_start(args,fmt);
	for(int pos=0;fmt[pos];pos++)
	{
		zeros = false;
		pad_size = 0;
		if((ch = fmt[pos]) == '%')
		{
			if((ch = fmt[++pos]) >= '0' && ch <= '9')
			{
				zeros = (ch == '0');
				pad_size = atoi(&fmt[pos]);
				while((ch = fmt[++pos]) >= '0' && ch <= '9');
			}
			switch(ch)
			{
			case 'c' :																											operator+=(va_arg(args,char));	break;
			case 'x' :	val = va_arg(args,char);	operator+=(LowerHexChar[val>>4]);	operator+=(LowerHexChar[val&0x0F]);										break;
			case 'X' :	val = va_arg(args,char);	operator+=(UpperHexChar[val>>4]);	operator+=(UpperHexChar[val&0x0F]);										break;
			case 's' :	tmp = va_arg(args,char *);									tmp.RightPad(pad_size,' ');					operator+=(tmp);				break;
			case 'S' :	tmp = va_arg(args,STRING);									tmp.RightPad(pad_size,' ');					operator+=(tmp);				break;
			case 'f' :	tmp.ConvertFloat(va_arg(args,double),10);					tmp.LeftPad(pad_size,zeros ? '0' : ' ');	operator+=(tmp);				break;
			case 'd' :	_itoa_s(va_arg(args,int),tmp.String,StringSize,10);			tmp.LeftPad(pad_size,zeros ? '0' : ' ');	operator+=(tmp);				break;
			case 'l' :	_i64toa_s(va_arg(args,long long),tmp.String,StringSize,10);	tmp.LeftPad(pad_size,zeros ? '0' : ' ');	operator+=(tmp);				break;
			case '$' :	longint = va_arg(args,long long);							tmp.Dollars(longint);						operator+=(tmp);				break;
//			case 'T' :	tmp.TimeStamp();																						operator+=(tmp);				break;
//			case 'v' :	tmp.Format((ProgramVersion.Edit ? "%d.%d(%d)-%d" : "%d.%d(%d)"),ProgramVersion.Major,ProgramVersion.Minor,ProgramVersion.Sub,ProgramVersion.Edit);	operator+=(tmp);	break;
			case 0 :	pos--;	operator+=('%');	break;
			default :			operator+=(ch);		break;
			}
		}
		else operator+=(ch);
    }
    va_end(args);
	return(*this);
}

bool LaunchThread(LPTHREAD_START_ROUTINE lpStartAddress,LPVOID lpParameter)
{
	if(HANDLE hThread = CreateThread(NULL,0,lpStartAddress,lpParameter,0,nullptr))
	{
		CloseHandle(hThread);
		return(true);
	}
	return(false);
}
