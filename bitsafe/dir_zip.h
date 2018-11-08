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
    HZIP hz;          //Zip文件句柄 
    ZRESULT zr;    //操作返回值 
    ZIPENTRY ze;  //Zip文件入口

    CStdString m_FolderPath;     //folder路径 
    CStdString m_FolderName;  //folder将要被压缩的文件夹名

private: 
    //实现遍历文件夹 
    void BrowseFile(CStdString &strFile);

    //获取相对路径 
    void GetRelativePath(CStdString& pFullPath, CStdString& pSubString);

    //创建路径 
    BOOL CreatedMultipleDirectory( LPCTSTR direct); 


public: 
    //压缩文件夹接口 
    BOOL Zip_PackFiles(CStdString& pFilePath, CStdString& mZipFileFullPath);

    //解压缩文件夹接口 
    BOOL Zip_UnPackFiles(CStdString &mZipFileFullPath, CStdString& mUnPackPath);

public: 
    //静态方法提供文件夹路径检查 
    static BOOL FolderExist(CStdString& strPath); 
}; 

#endif //__DIR_ZIP_H__