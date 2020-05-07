// GalaxyDB.cpp - Custom Database for Galaxy Gaming
//

#include <Windows.h>
#include <stdio.h>
#include "String.h"

typedef long long   lint;

#define MAX_RECORD_SIZE 16384

typedef enum DataTypes
{
    DT_INTEGER = 0,
    DT_STRING,
    DT_SECURE,

    DT_TYPES

} DataType;

struct
{
    DataType        Type;
    const char *    Name;

}   DataTypeName[DT_TYPES] =    {
                                    {   DT_INTEGER, "INTEGER"   },
                                    {   DT_STRING,  "STRING"    },
                                    {   DT_SECURE,  "SECURE"    },
                                };

DataType GetDataType(STRING & _name)
{
    for(int idx=0;idx<DT_TYPES;idx++)
        if(_name == DataTypeName[idx].Name)
            return(DataTypeName[idx].Type);

//  GDB->RecordEvent(ET_BadDataType,_name);
    return(DT_INTEGER);
}

typedef struct DataBaseField
{
    STRING      ID;
    DataType    Type;
    int         Width;
    STRING      Prompt;
    STRING      Help;

    int         GetSize(void);

} DBField;

typedef std::vector<DBField *>  vpDBField;

int DataBaseField::GetSize(void)
{
    switch(Type)
    {
    case DT_INTEGER :
        return(sizeof(int));
    case DT_STRING :
    case DT_SECURE :
        return(Width);
    }
    return(0);
}

typedef struct DataBaseFile
{
    STRING      ID;
    vpDBField   Fields;
    int         RecordSize;

    int         GetOffset(int _fidx);
    DBField *   GetField(const char * _field_id,int * offset=nullptr);
    bool        ReadFields(void);
    void        DumpFields(void);

} DBFile;

typedef std::vector<DBFile *>  vpDBFile;

typedef struct DataFileEntry
{
    DBFile *    File;
    HANDLE      hFile;
    char *      Record;
    int         RecNo;

    DataFileEntry(DBFile * _dbfile) : File(_dbfile), hFile(INVALID_HANDLE_VALUE), Record(nullptr), RecNo(0)  {};
    ~DataFileEntry() { if(Record) { delete [] Record; Record = nullptr; } };

    void        SetRecordNumber(int _recno);
    bool        GetFieldString(const char * _field_id,STRING & _fieldtext);
    bool        SetFieldString(const char * _field_id,const char * _newfieldtext);
    bool        AddRecord(void);
    bool        ReadRecord(int _recno);
    bool        UpdateRecord(void);

} DBEntry;

typedef std::vector<DBEntry *>  vpDBEntry;

struct GalaxyDataBase
{
    vpDBFile    DBFiles;
    vpDBEntry   DBEntries;
    
    DBEntry *   AddFileEntry(STRING & _fileid);
    DBEntry *   FindFileEntry(STRING & _fileid);
    DBFile *    LoadDataBaseFile(STRING & _fileid);
    DBEntry *   OpenFile(STRING & _fileid);
    bool        CloseFile(DBEntry * _dbhandle);
};

GalaxyDataBase  GDB;

#define     MAX_BUFFER  4096

int ReadLine(HANDLE hFile,STRING & _buffer)
{
char    tmp[MAX_BUFFER+1];
int     pos;
DWORD   readed;

    for(pos=0;ReadFile(hFile,&tmp[pos],1,&readed,nullptr) && readed == 1 && tmp[pos] != 0x0A && pos < MAX_BUFFER;pos++);
    tmp[pos] = 0x00;
    _buffer = tmp;
    return(pos);
}

bool DataBaseFile::ReadFields(void)
{
char            filename[80];
unsigned int    len;
HANDLE          hFile;
vSTRING         parms;
STRING          str;
DBField *       newfld;

    len = sprintf_s(filename,80,"Files\\%s.file",ID.String);       // Create full system filename
    hFile = CreateFileA(filename,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,INVALID_HANDLE_VALUE);
    if(hFile != INVALID_HANDLE_VALUE)                   // If file opened...
    {
        while(ReadLine(hFile,str))
        {
            newfld = new DBField;
            str.Split(0,parms);
            for(unsigned int idx=0;idx<parms.size();idx+=2)
            {
                if(parms[idx] == "FIELD")   {   newfld->ID =      parms[idx+1];                 }
                if(parms[idx] == "TYPE")    {   newfld->Type =    GetDataType(parms[idx+1]);    }
                if(parms[idx] == "WIDTH")   {   newfld->Width =   atoi(parms[idx+1].String);    }
                if(parms[idx] == "PROMPT")  {   newfld->Prompt =  parms[idx+1];                 }
                if(parms[idx] == "HELP")    {   newfld->Help =    parms[idx+1];                 }
            }
            Fields.push_back(newfld);
        }
        CloseHandle(hFile);
        RecordSize = GetOffset(Fields.size());
        return(true);
    }
    return(false);
}

void DataBaseFile::DumpFields(void)
{
    while(Fields.size())
    {
        DBField *   fld = Fields[Fields.size()-1];
        Fields.pop_back();
        delete fld;
    }
}

int DataBaseFile::GetOffset(int _fidx)
{
int offset = 0;

    for(int idx=0;idx<_fidx;idx++)
    {
        offset += Fields[idx]->GetSize();
    }
    if(offset > MAX_RECORD_SIZE)
    {
//      GDB->RecordEvent(ET_MaxRecSizeExceeded,_fileid);
        offset = MAX_RECORD_SIZE;
    }
    return(offset);
}

DBField * DataBaseFile::GetField(const char * _field_id,int * offset)
{
STRING  str = _field_id;

    *offset = 0;

    for(unsigned int idx=0;idx<Fields.size();idx++)
    {
        if(str == Fields[idx]->ID)
        {
            return(Fields[idx]);
        }
        *offset += Fields[idx]->GetSize();
    }

    return(nullptr);
}

void DataFileEntry::SetRecordNumber(int _recno)
{
lint    offset;

    RecNo = _recno;
    offset = (lint) RecNo * File->RecordSize;
    SetFilePointer(hFile,(DWORD) offset,NULL,SEEK_SET);
}

bool DataFileEntry::GetFieldString(const char * _field_id,STRING & _fieldtext)
{
DBField *  fld;
int        offset,size;

    fld = File->GetField(_field_id,&offset);
    if(fld != nullptr && (fld->Type == DT_STRING || fld->Type == DT_SECURE))
    {
        size = fld->GetSize();
        _fieldtext.Grow(size);
        for(int idx=0;idx < size && (_fieldtext.String[idx] = Record[offset++]) != 0x00;idx++);
        return(true);
    }
    return(false);
}

bool DataFileEntry::SetFieldString(const char * _field_id,const char * _newfieldtext)
{
DBField *  fld;
int        offset;

    fld = File->GetField(_field_id,&offset);
    if(fld != nullptr && (fld->Type == DT_STRING || fld->Type == DT_SECURE))
    {
        for(int size=fld->GetSize(),idx=0;(Record[offset++] = _newfieldtext[idx]) != 0x00 && idx < size;idx++);
        return(true);
    }
    return(false);
}

DataFileEntry * GalaxyDataBase::FindFileEntry(STRING & _fileid)
{
    for(unsigned int idx=0;idx<DBEntries.size();idx++)
    {
        if(DBEntries[idx]->File->ID == _fileid)
        {
            return(DBEntries[idx]);
        }
    }
    return(nullptr);
}

DBFile * GalaxyDataBase::LoadDataBaseFile(STRING & _fileid)
{
DBFile *    dbfile;

    dbfile = new DBFile;
    dbfile->ID = _fileid;
    dbfile->ReadFields();
    DBFiles.push_back(dbfile);
    return(dbfile);
}

DataFileEntry * GalaxyDataBase::AddFileEntry(STRING & _fileid)
{
DBFile *        dbfile;
DataFileEntry * fentry;
char            filename[80];
unsigned int    len,idx;

    if(dbfile = LoadDataBaseFile(_fileid))
    {
        fentry = new DataFileEntry(dbfile);
        len = sprintf_s(filename,80,"%s.dat",_fileid.String);       // Create full system filename
        fentry->hFile = CreateFileA(filename,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_ALWAYS,FILE_ATTRIBUTE_NORMAL,INVALID_HANDLE_VALUE);
        if(fentry->hFile != INVALID_HANDLE_VALUE)                   // If file opened...
        {
            fentry->Record = new char[dbfile->RecordSize];
            for(idx=0;idx<DBEntries.size();idx++)
            {
                if(DBEntries[idx] == nullptr)
                {
                    DBEntries[idx] = fentry;
                    break;
                }
            }
            if(idx >= DBEntries.size())
            {
                DBEntries.push_back(fentry);
            }
            return(fentry);
        }
//      else GDB->RecordEvent(ET_FailedToOpenDataBaseFile,_fileid);
        delete fentry;
    }
//  else GDB->RecordEvent(ET_BadDataBaseFileID,_fileid);
    return(nullptr);
}

DataFileEntry * GalaxyDataBase::OpenFile(STRING & _fileid)
{
    if(DataFileEntry * fEntry = FindFileEntry(_fileid))
    {
        return(fEntry);
    }
    return(AddFileEntry(_fileid));
}

bool GalaxyDataBase::CloseFile(DataFileEntry * _fentry)
{
    for(unsigned int idx=0;idx<DBEntries.size();idx++)
    {
//      if(DBEntries[idx] == _fentry && DBEntries[idx]->Unit == ThisUnit)
        if(DBEntries[idx] == _fentry)
        {
            DBEntries[idx] = nullptr;
            delete _fentry;
            return(true);
        }
    }
    return(false);
}

bool DataFileEntry::AddRecord(void)
{
DWORD   written,pos;

    pos = SetFilePointer(hFile,0,NULL,SEEK_END);
    if(WriteFile(hFile,Record,File->RecordSize,&written,NULL) == FALSE || written != File->RecordSize)
    {
        return(false);
    }
    RecNo = pos / File->RecordSize;
    return(true);
}

bool DataFileEntry::ReadRecord(int _recno)
{
DWORD   readed;

    SetRecordNumber(_recno);
    if(ReadFile(hFile,Record,File->RecordSize,&readed,NULL) == FALSE || readed != File->RecordSize)
    {
//      GDB->RecordEvent(ET_ReadFailed,_fileid);
        return(false);
    }
    return(true);
}

bool DataFileEntry::UpdateRecord(void)
{
DWORD   written;

    SetRecordNumber(RecNo);
    if(WriteFile(hFile,Record,File->RecordSize,&written,NULL) == FALSE || written != File->RecordSize)
    {
//      GDB->RecordEvent(ET_UpdateFailed,_fileid);
        return(false);
    }
    return(true);
}

/*
int main(int _argc,char * * _argv)
{
DataFileEntry * dbfile;
STRING          str;
int             recno;

    str = "USER";
    dbfile = GDB.OpenFile(str);

    dbfile->SetFieldString("UserID","NIKKI");
    dbfile->SetFieldString("UserName","Nikki Lo");
    dbfile->SetFieldString("Password","OpenUp");

    dbfile->AddRecord();

    recno = 0;
    dbfile->ReadRecord(recno);
                                              printf("Current Record:\n");
    dbfile->GetFieldString("UserID"  ,str);  printf("  User ID: %s\n",str.String);
    dbfile->GetFieldString("UserName",str);  printf("User Name: %s\n",str.String);
    dbfile->GetFieldString("Password",str);  printf(" Password: %s\n",str.String);

    dbfile->SetFieldString("Password","PleaseOpenUp");

    dbfile->UpdateRecord();

    recno = 0;
    dbfile->ReadRecord(recno);
                                              printf("Updated Record:\n");
    dbfile->GetFieldString("UserID"  ,str);  printf("  User ID: %s\n",str.String);
    dbfile->GetFieldString("UserName",str);  printf("User Name: %s\n",str.String);
    dbfile->GetFieldString("Password",str);  printf(" Password: %s\n",str.String);

    GDB.CloseFile(dbfile);

    getchar();
}
*/
