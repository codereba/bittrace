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