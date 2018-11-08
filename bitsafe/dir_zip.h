/*
 *
 * Copyright 2010 JiJie Shi
 *
 * This file is part of bittrace.
 *
 * bittrace is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * bittrace is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with bittrace.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __DIR_ZIP_H__
#define __DIR_ZIP_H__

class CZipImplement 
{ 
public: 
    CZipImplement(void); 
    ~CZipImplement(void);

private: 
    HZIP hz;          //Zip�ļ���� 
    ZRESULT zr;    //��������ֵ 
    ZIPENTRY ze;  //Zip�ļ����

    CStdString m_FolderPath;     //folder·�� 
    CStdString m_FolderName;  //folder��Ҫ��ѹ�����ļ�����

private: 
    //ʵ�ֱ����ļ��� 
    void BrowseFile(CStdString &strFile);

    //��ȡ���·�� 
    void GetRelativePath(CStdString& pFullPath, CStdString& pSubString);

    //����·�� 
    BOOL CreatedMultipleDirectory( LPCTSTR direct); 


public: 
    //ѹ���ļ��нӿ� 
    BOOL Zip_PackFiles(CStdString& pFilePath, CStdString& mZipFileFullPath);

    //��ѹ���ļ��нӿ� 
    BOOL Zip_UnPackFiles(CStdString &mZipFileFullPath, CStdString& mUnPackPath);

public: 
    //��̬�����ṩ�ļ���·����� 
    static BOOL FolderExist(CStdString& strPath); 
}; 

#endif //__DIR_ZIP_H__