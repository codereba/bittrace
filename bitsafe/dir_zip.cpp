/*
 * Copyright 2010-2024 JiJie.Shi.
 *
 * This file is part of bittrace.
 * Licensed under the Gangoo License, Version 1.0 (the "License");
 * you may not use this file except in compliance with the License.
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "StdAfx.h" 
#include "..\DuiLib\XUnzip.h"
#include "dir_zip.h"

CZipImplement::CZipImplement(void) 
{ 
}

CZipImplement::~CZipImplement(void) 
{ 
}

#if 0
BOOL CZipImplement::Zip_UnPackFiles(CStdString &mZipFileFullPath, CStdString& mUnPackPath) 
{ 
    if ((mUnPackPath == L"") || (mZipFileFullPath == L"")) 
    { 
        return FALSE ; 
    }

    CFileFind tFFind; 
    if (!tFFind.FindFile(mZipFileFullPath)) 
    { 
        return FALSE; 
    }

    CStdString tZipFilePath = mUnPackPath; 
    if(!CZipImplement::FolderExist(tZipFilePath)) 
    { 
        wchar_t* temp=tZipFilePath.GetBuffer(tZipFilePath.GetLength()); 
        if (FALSE == CreatedMultipleDirectory(temp)) 
        { 
            return FALSE; 
        } 
    } 

    hz=OpenZip(mZipFileFullPath,0); 
    if(hz == 0) 
    { 
        return FALSE; 
    }

    zr=SetUnzipBaseDir(hz,mUnPackPath); 
    if(zr != ZR_OK) 
    { 
        CloseZip(hz); 
        return FALSE;       
    }

    zr=GetZipItem(hz,-1,&ze); 
    if(zr != ZR_OK) 
    { 
        CloseZip(hz); 
        return FALSE;       
    }

    int numitems=ze.index; 
    for (int i=0; i     { 
        zr=GetZipItem(hz,i,&ze); 
        zr=UnzipItem(hz,i,ze.name);

        if(zr != ZR_OK) 
        { 
            CloseZip(hz); 
            return FALSE;       
        } 
    }

    CloseZip(hz); 
    return TRUE; 
}

BOOL CZipImplement::FolderExist(CStdString& strPath) 
{ 
    CStdString sCheckPath = strPath;

    if(sCheckPath.Right(1) != L"\\") 
        sCheckPath += L"\\";

    sCheckPath += L"*.*";

    WIN32_FIND_DATA wfd; 
    BOOL rValue = FALSE;

    HANDLE hFind = FindFirstFile(sCheckPath, &wfd);

    if ((hFind!=INVALID_HANDLE_VALUE) && 
        (wfd.dwFileAttributes&FILE_ATTRIBUTE_DIRECTORY) || (wfd.dwFileAttributes&FILE_ATTRIBUTE_ARCHIVE)) 
    { 
        rValue = TRUE; 
    }

    FindClose(hFind); 
    return rValue; 
}

void CZipImplement::BrowseFile(CStdString &strFile) 
{ 
    CFileFind ff; 
    CStdString szDir = strFile;

    if(szDir.Right(1) != L"\\") 
        szDir += L"\\";

    szDir += L"*.*";

    BOOL res = ff.FindFile(szDir); 
    while(res) 
    { 
        res = ff.FindNextFile(); 
        if(ff.IsDirectory() && !ff.IsDots()) 
        { 
            CStdString strPath = ff.GetFilePath();

            CStdString subPath; 
            GetRelativePath(strPath,subPath); 
            ZipAddFolder(hz,subPath); 
            BrowseFile(strPath); 
        } 
        else if(!ff.IsDirectory() && !ff.IsDots()) 
        { 
            CStdString strPath = ff.GetFilePath(); 
            CStdString subPath;

            GetRelativePath(strPath,subPath); 
            ZipAdd(hz,subPath,strPath); 
        } 
    }

    ff.Close(); 
}

void CZipImplement::GetRelativePath(CStdString& pFullPath,CStdString& pSubString) 
{ 
    pSubString = pFullPath.Right(pFullPath.GetLength() - this->m_FolderPath.GetLength() + this->m_FolderName.GetLength()); 
}

BOOL CZipImplement::CreatedMultipleDirectory( LPCTSTR direct ) 
{ 
    std::wstring Directoryname = direct;

    if (Directoryname[Directoryname.length() - 1] !=  '\\') 
    { 
        Directoryname.append(1, '\\'); 
    } 
    std::vector< std::wstring> vpath; 
    std::wstring strtemp; 
    BOOL  bSuccess = FALSE; 
    for (int i = 0; i < Directoryname.length(); i++) 
    { 
        if ( Directoryname[i] != '\\') 
        { 
            strtemp.append(1,Directoryname[i]);    
        } 
        else 
        { 
            vpath.push_back(strtemp); 
            strtemp.append(1, '\\'); 
        } 
    } 
    std::vector:: const_iterator vIter; 
    for (vIter = vpath.begin();vIter != vpath.end(); vIter++) 
    { 
        bSuccess = CreateDirectory(vIter->c_str(), NULL) ? TRUE :FALSE; 
    }

    return bSuccess; 
}

#endif //0