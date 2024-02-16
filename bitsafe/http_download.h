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
  
  
    //parmSock: ����վ���ӵ��׽���  
    //serverName: ��վ��ַ  
    //sourcePath: �ļ�·��  
    //fileName: �ļ���  
    //localDirectory: �ļ��洢�ڱ��ص�Ŀ¼  
    //port: �����ļ��˿�  
    bool HTTPClient::DownloadFile(SOCKET parmSock,string serverName, string sourcePath, string fileName, string localDirectory, string port);  
      
    //��ʽ��http����ͷ������Range�ֶΣ����ֶα�ʾ�����ص��ļ��ֽ���  
    void FormatRequestHeader(char *buf,string serverName,string requestFile,std::string Port = "80");  
      
    //��ȡҪ���ص��ļ���С��szBufferΪ���յ�������,nRetΪ���յ������ݳ���  
    bool GetFileLength(const char* szBuffer);  
      
    //�ж��Ƿ���Ҫ�����ļ����粻��Ҫ����false  
    bool NeedToDownloadFile();  
  
  
private:  
    long int RangeInt;  //�����ص��ļ��ֽ���  
    long int ContentLengthInt;  //Ҫ���ص��ļ���С  
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
//��ʽ��http����ͷ  
//////////////////////////////  
void HTTPClient::FormatRequestHeader(char *buf,std::string serverName,std::string requestFile,std::string port)  
{  
    memset(buf,'\0',1024);  
    //��һ��  
    sprintf(buf,"GET %s HTTP/1.1\r\n",requestFile.c_str());  
      
    //�ڶ���  
    strcat(buf,"Host:");  
    strcat(buf,serverName.c_str());  
    strcat(buf,":");  
    strcat(buf,port.c_str());  
    strcat(buf,"\r\n");  
      
    //������  
    strcat(buf,"Accept:*/*");  
    strcat(buf,"\r\n");  
    //////////////////////////////  
    /*strcat(buf,"Accept-Language: zh-cn");  
    strcat(buf,"\r\n");  
        */  
    //////////////////////////  
    //������(���п���)  
    strcat(buf,"User-Agent:GeneralDownloadApplication");  
    strcat(buf,"\r\n");  
      
    //������ �ǳ־�����  
    strcat(buf,"Connection:close");  
    strcat(buf,"\r\n");  
      
    //������ �ϵ������ؼ�  
    char RangeString[15];  
    ltoa(RangeInt,RangeString,10);  
      
    strcat(buf,"RANGE: bytes=");  
    strcat(buf,RangeString);  
    strcat(buf,"-\r\n");  
      
    ///���һ��:����  
    strcat(buf,"\r\n");  
  
  
  
  
}  
/////////////////////////////////////////  
//��ȡҪ���ص��ļ���С����ContentLengthInt��  
///////////////////////////////////////////  
bool HTTPClient::GetFileLength(const char* szBuffer)  
{  
    std::string strBuffer(szBuffer);  
  
  
    //��ȡhttpͷ�Ĵ�С  
    int httpLength = strBuffer.find("\r\n\r\n");   
    //httpͷȫ���Ĵ�С��������'\0'  
    httpLength = httpLength + strlen("\r\n\r\n");    
  
  
    //�洢Content-Range֮�������  
    std::string strContent;  
    //�洢Content-Range���е��ַ���ֵ  
    std::string strContentRange;  
    //��ȡContent-Range��  
    int ContentRangeBegin = strBuffer.find("Content-Range: bytes ");  
    if(ContentRangeBegin == -1)  
    {  
        return false;  
    }  
      
    strContent = strBuffer.substr(ContentRangeBegin-1,strBuffer.find("\r\n\r\n")-ContentRangeBegin+1);  
  
  
    strContentRange = strContent.substr(sizeof("Content-Range: bytes "));  
    ///����Ҫ���ص��ļ���С  
    std::string strLength = strContentRange.substr(strContentRange.find("/")+1);  
    this->ContentLengthInt = atol(strLength.c_str());  
      
    return true;  
}  
  
  
////////////////////////////////////////////////////////////////////  
//�ж��Ƿ���Ҫ�����ļ�  
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
//            parmSock: ����վ���ӵ��׽���  
//        serverName = "127.0.0.1" or "localhost"  
//            sourcePath = "/abc/"  
//            fileName = "config.xml"  
//            localDirectory = "c:\\"  
  
  
bool HTTPClient::DownloadFile(SOCKET parmSock,string serverName, string sourcePath, string fileName, string localDirectory, string port)  
{  
    ofstream fout;  
    string newfile = localDirectory + fileName;  
  
  
    ////////////////////////////////////////////////  
    //�ܹ�ʶ������Ŀ¼���ļ���  
    std::locale::global(std::locale(""));  
    //���жϸ��ļ��Ƿ���ڣ�����ڻ�ȡ�ļ���С  
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
  
  
    //���ļ�����λ�ļ���ָ�뵽�ļ�β  
    fout.open(newfile.c_str(),ios_base::binary|ios_base::app);    
    if(!fout.is_open())  
    {  
        cout<<"open newfile error!"<<endl;  
    }  
  
  
    char szBuffer[1024];  
  
  
    /////////////////////////////////////////  
    //��Ŀ¼���ļ����е�' 'ת��Ϊ"%20"  
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
  
  
    //��ʽ��httpЭ������ͷ  
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
        //ȡ��httpͷ,��СӦ�ò�����1024���ֽ�,ֻ�ڵ�һ�ν���ʱ���д���  
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
    //�ļ����ݿ�ʼ  
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
    //����Ҫ�����ļ���ִ�����´���  
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
  
  
    std::string url = "http://localhost/�� �� �� �� ��/�� �� �� �� .rmvb";  
    std::string localDirectory = "D:\\�� ��\\";  
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