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
 
 #ifndef HTTPClient_H_  
#define HTTPClient_H_  
  
  
#include <string>  
#include <winsock.h>  
#include <iostream>  
#include <fstream>  
//#include "//Log.h"  
#pragma comment(lib,"ws2_32.lib")  
  
  
//#define tempDirectory "c:\\"  
using namespace std;  
  
  
class HTTPClient  
{  
public:  
    HTTPClient(void);  
    ~HTTPClient(void);  
  
  
    //parmSock: 与网站连接的套接字  
    //serverName: 网站网址  
    //sourcePath: 文件路径  
    //fileName: 文件名  
    //localDirectory: 文件存储在本地的目录  
    //port: 下载文件端口  
    bool HTTPClient::DownloadFile(SOCKET parmSock,string serverName, string sourcePath, string fileName, string localDirectory, string port);  
      
    //格式化http请求头，增加Range字段，该字段表示已下载的文件字节数  
    void FormatRequestHeader(char *buf,string serverName,string requestFile,std::string Port = "80");  
      
    //获取要下载的文件大小，szBuffer为接收到的数据,nRet为接收到得数据长度  
    bool GetFileLength(const char* szBuffer);  
      
    //判断是否需要下载文件，如不需要返回false  
    bool NeedToDownloadFile();  
  
  
private:  
    long int RangeInt;  //已下载的文件字节数  
    long int ContentLengthInt;  //要下载的文件大小  
};  
  
  
  
  
#endif  
  
//HTTPClient.cpp  
  
  
#include "HTTPClient.h"  
  
  
  
  
HTTPClient::HTTPClient(void)  
{  
    RangeInt = 0;  
    ContentLengthInt = 0;  
}  
  
  
HTTPClient::~HTTPClient(void)  
{  
}  
//////////////////////////////////////////////////  
//格式化http请求头  
//////////////////////////////  
void HTTPClient::FormatRequestHeader(char *buf,std::string serverName,std::string requestFile,std::string port)  
{  
    memset(buf,'\0',1024);  
    //第一行  
    sprintf(buf,"GET %s HTTP/1.1\r\n",requestFile.c_str());  
      
    //第二行  
    strcat(buf,"Host:");  
    strcat(buf,serverName.c_str());  
    strcat(buf,":");  
    strcat(buf,port.c_str());  
    strcat(buf,"\r\n");  
      
    //第三行  
    strcat(buf,"Accept:*/*");  
    strcat(buf,"\r\n");  
    //////////////////////////////  
    /*strcat(buf,"Accept-Language: zh-cn");  
    strcat(buf,"\r\n");  
        */  
    //////////////////////////  
    //第四行(可有可无)  
    strcat(buf,"User-Agent:GeneralDownloadApplication");  
    strcat(buf,"\r\n");  
      
    //第五行 非持久连接  
    strcat(buf,"Connection:close");  
    strcat(buf,"\r\n");  
      
    //第六行 断点续传关键  
    char RangeString[15];  
    ltoa(RangeInt,RangeString,10);  
      
    strcat(buf,"RANGE: bytes=");  
    strcat(buf,RangeString);  
    strcat(buf,"-\r\n");  
      
    ///最后一行:空行  
    strcat(buf,"\r\n");  
  
  
  
  
}  
/////////////////////////////////////////  
//获取要下载的文件大小存入ContentLengthInt中  
///////////////////////////////////////////  
bool HTTPClient::GetFileLength(const char* szBuffer)  
{  
    std::string strBuffer(szBuffer);  
  
  
    //获取http头的大小  
    int httpLength = strBuffer.find("\r\n\r\n");   
    //http头全部的大小，不包括'\0'  
    httpLength = httpLength + strlen("\r\n\r\n");    
  
  
    //存储Content-Range之后的数据  
    std::string strContent;  
    //存储Content-Range行中的字符串值  
    std::string strContentRange;  
    //获取Content-Range行  
    int ContentRangeBegin = strBuffer.find("Content-Range: bytes ");  
    if(ContentRangeBegin == -1)  
    {  
        return false;  
    }  
      
    strContent = strBuffer.substr(ContentRangeBegin-1,strBuffer.find("\r\n\r\n")-ContentRangeBegin+1);  
  
  
    strContentRange = strContent.substr(sizeof("Content-Range: bytes "));  
    ///保存要下载的文件大小  
    std::string strLength = strContentRange.substr(strContentRange.find("/")+1);  
    this->ContentLengthInt = atol(strLength.c_str());  
      
    return true;  
}  
  
  
////////////////////////////////////////////////////////////////////  
//判断是否需要下载文件  
////////////////////////////////////////////////////////////////////  
bool HTTPClient::NeedToDownloadFile()  
{  
    if(this->RangeInt == this->ContentLengthInt)  
    {  
        return false;  
    }  
    return true;  
}  
////////////////////////////////////////////////////////////////////  
//            parmSock: 与网站连接的套接字  
//        serverName = "127.0.0.1" or "localhost"  
//            sourcePath = "/abc/"  
//            fileName = "config.xml"  
//            localDirectory = "c:\\"  
  
  
bool HTTPClient::DownloadFile(SOCKET parmSock,string serverName, string sourcePath, string fileName, string localDirectory, string port)  
{  
    ofstream fout;  
    string newfile = localDirectory + fileName;  
  
  
    ////////////////////////////////////////////////  
    //能够识别中文目录和文件名  
    std::locale::global(std::locale(""));  
    //先判断该文件是否存在，如存在获取文件大小  
    ifstream fin(newfile.c_str());  
    /*if(!fin.is_open()) 
    { 
        cout<<"newfile is not exist, create it!"<<endl; 
    }*/  
    fin.seekg(0,ios::end);  
    streampos ps = fin.tellg();  
    if(ps > 0)  
    {  
        this->RangeInt = ps;  
    }  
    //cout<<"ps ="<<ps<<endl;  
    fin.close();  
  
  
    //打开文件，定位文件内指针到文件尾  
    fout.open(newfile.c_str(),ios_base::binary|ios_base::app);    
    if(!fout.is_open())  
    {  
        cout<<"open newfile error!"<<endl;  
    }  
  
  
    char szBuffer[1024];  
  
  
    /////////////////////////////////////////  
    //将目录和文件名中的' '转换为"%20"  
    /////////////////////////////////////////  
    //sourcePath  
    int npos = sourcePath.find(" ");  
    while(npos + 1)  
    {  
        npos = sourcePath.find(" ");  
        if(npos == -1)  
            break;  
        sourcePath.replace(npos,strlen(" "),"%20");  
  
  
    }  
    //fileName  
    npos = fileName.find(" ");  
    while(npos + 1)  
    {  
        npos = fileName.find(" ");  
        if(npos == -1)  
            break;  
        fileName.replace(npos,strlen(" "),"%20");  
    }  
  
  
    /////////////////////////////////////////  
  
  
    string requestFile = sourcePath + fileName;  
  
  
    //格式化http协议请求头  
    FormatRequestHeader(szBuffer,serverName,requestFile,port);  
      
    int nRet = send(parmSock, szBuffer, strlen(szBuffer), 0);  
    if (nRet == SOCKET_ERROR)  
    {  
        cout<<"send() error"<<endl;  
        closesocket(parmSock);   
        fout.close();  
  
  
        WSACleanup();  
        return false;  
    }  
  
  
    //  
    // Receive the file contents and print to stdout  
    //  
    ///////////////////////////////////////////////////////////////  
        //取出http头,大小应该不超过1024个字节,只在第一次接收时进行处理  
    ///////////////////////////////////////////////////////////////  
    nRet = recv(parmSock, szBuffer, sizeof(szBuffer), 0);  
    if (nRet == SOCKET_ERROR)  
    {  
        cout<<"recv() error"<<endl;  
        closesocket(parmSock);  
        fout.close();  
  
  
        WSACleanup();  
        return false;  
    }  
      
    int i = 0;  
    while(1)  
    {  
        if((szBuffer[i]=='\r')&&(szBuffer[i+1]=='\n')&&(szBuffer[i+2]=='\r')&&(szBuffer[i+3]=='\n'))  
        {  
            break;  
        }  
        i++;  
    }  
    //文件内容开始  
    int fileBegin = i+4;  
  
  
    bool File_Exist = GetFileLength(szBuffer);  
    if(!File_Exist)  
    {  
        cout<<"Can not get the size of the file.Make sure the file exist."<<endl;  
        closesocket(parmSock);  
        fout.close();  
  
  
        WSACleanup();  
        return false;  
    }  
  
  
    bool Need_Download = NeedToDownloadFile();  
    if(!Need_Download)  
    {  
        closesocket(parmSock);  
        fout.close();  
  
  
        WSACleanup();  
        return true;  
    }  
    /////////////////////////////////////////////////////////////  
    //如需要下载文件，执行以下代码  
    /////////////////////////////////////////////////////////////  
    for(i = fileBegin; i < nRet; i++)  
    {  
        this->RangeInt += 1;  
        fout.write(&szBuffer[i],sizeof(szBuffer[i]));  
    }  
  
  
    while(1)  
    {  
        // Wait to receive, nRet = NumberOfBytesReceived  
        nRet = recv(parmSock, szBuffer, sizeof(szBuffer), 0);  
  
  
        if (nRet == SOCKET_ERROR)  
        {  
            cout<<"recv() error"<<endl;  
            closesocket(parmSock);  
            fout.close();  
  
  
            WSACleanup();  
            return false;  
        }  
  
  
        /////////////////////////////////////////////////////////////////  
          
        //cout<<"\nrecv() returned "<<nRet<<" bytes"<<endl;  
        if (nRet == 0)  
            break;  
        // Write to stdout  
        this->RangeInt += nRet;  
        /*if(this->RangeInt>7700000) 
            break;*/  
        fout.write(szBuffer,nRet);  
    }  
  
  
    closesocket(parmSock);      
    fout.close();  
  
  
  
  
    WSACleanup();  
    return true;  
}  
  
  
  
#include "HTTPClient.h"  
//#include <iostream>  
int main()  
{  
    HTTPClient m;  
    //m.DownloadFile("localhost","/abc/","config.xml","D:\\");  
  
  
    std::string url = "http://localhost/新 建 文 件 夹/陨 落 星 辰 .rmvb";  
    std::string localDirectory = "D:\\中 国\\";  
    std::string port = "5550";  
//  cout<<m.ContentLengthInt<<endl;  
//  cout<<m.RangeInt<<endl;  
    ///////////////////////////////////////////////////////////////////  
    std::string removeProtocol;  
    std::string serverName;  
    std::string sourcePath;  
    std::string fileName;  
  
  
    int ret;  
  
  
    ret = url.find("http://");  
    if(ret != -1)  
    {  
        removeProtocol = url.substr(strlen("http://"));  
    }  
    else  
    {  
        removeProtocol = url;  
    }  
      
/*  ret = url.find("https://"); 
    if(ret != -1) 
    { 
        removeProtocol = url.substr(strlen("https://")); 
    } 
    else 
    { 
        removeProtocol = url; 
    } 
*/  
    ret = removeProtocol.find_first_of('/');  
    serverName = removeProtocol.substr(0,ret);  
      
    int net = removeProtocol.find_last_of('/');  
    sourcePath = removeProtocol.substr(ret,net-ret+1);  
  
  
    fileName = removeProtocol.substr(net+1);  
    //////////////////////////////////////////////////////////////////  
    WSADATA wsaData;  
    int nRet;  
    //  
    // Initialize WinSock.dll  
    //  
    nRet = WSAStartup(0x101, &wsaData);  
    if (nRet)  
    {  
        WSACleanup();  
        return false;  
    }  
  
  
    if (wsaData.wVersion != 0x101)  
    {  
        WSACleanup();  
        return false;  
    }  
      
    IN_ADDR        iaHost;  
    LPHOSTENT    lpHostEntry;  
  
  
    iaHost.s_addr = inet_addr(serverName.c_str());  
    if (iaHost.s_addr == INADDR_NONE)  
    {  
        // Wasn't an IP address string, assume it is a name  
        lpHostEntry = gethostbyname(serverName.c_str());  
    }  
    else  
    {  
        // It was a valid IP address string  
        lpHostEntry = gethostbyaddr((const char *)&iaHost,   
                        sizeof(struct in_addr), AF_INET);  
    }  
    if (lpHostEntry == NULL)  
    {  
        cout<<"gethostbyname() error"<<endl;  
        return false;  
    }  
  
  
    //      
    // Create a TCP/IP stream socket  
    //  
    SOCKET    parmSock;      
  
  
    parmSock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);  
    if (parmSock == INVALID_SOCKET)  
    {  
        cout<<"socket() error"<<endl;   
        return false;  
    }  
  
  
    //  
    // Find the port number for the HTTP service on TCP  
    //  
    LPSERVENT lpServEnt;  
    SOCKADDR_IN saServer;  
  
  
    lpServEnt = getservbyname("http", "tcp");  
    if (lpServEnt == NULL)  
        saServer.sin_port = htons(80);  
    else  
        saServer.sin_port = lpServEnt->s_port;  
  
  
  
  
    //  
    // Fill in the rest of the server address structure  
    //  
    saServer.sin_family = AF_INET;  
    saServer.sin_addr = *((LPIN_ADDR)*lpHostEntry->h_addr_list);  
  
  
  
  
    //  
    // Connect the socket  
    //  
    nRet = connect(parmSock, (LPSOCKADDR)&saServer, sizeof(SOCKADDR_IN));  
    if (nRet == SOCKET_ERROR)  
    {  
        cout<<"connect() error"<<endl;  
        closesocket(parmSock);  
        return false;  
    }  
    //////////////////////////////////////////////////////////////////  
    m.DownloadFile(parmSock,serverName,sourcePath,fileName,localDirectory,port);  
  
  
  
  
    system("pause");  
    return 0;  
}  