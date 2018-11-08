# bittrace
 trace system internal activities by high perfermonce aio(include multiple implementations windows support, aio,completion port, wmi etc).
 and all file size is very small(include kernel driver, application).
 
 ------------------------------------------------------------------------
 Source code license is The GNU General Public License v3.0 
 ===========================================================
 
 And every file include copyright notation “Copyright 2010 JiJie Shi(weixin:AIChangeLife)”: 
 ================================
 
 Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
 
 Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the  documentation and/or other materials provided with the distribution.
 
 ---------------------------------------------------------------------- 
 
 If use the code to commercial software, that need to get bittrace team permission.

 publish on software site huajun(chinese version), download.cnet(english version) and other software download site:
 
  http://www.onlinedown.net/soft/256910.htm
 
  https://download.cnet.com/Bittrace/3000-2094_4-76472582.html?part=dl-&subj=dl&tag=button
 
 video lesson:
 
  http://v.youku.com/v_show/id_XMTQwMDU0NTYzNg==.html?from=y1.7-1.2
 
 website:
 
  http://www.simpai.net/
 
  kenel driver source code and some function is not public now, but include:
  
   1.direct ui effects
   
   2.asyncio/syncio with kernel mode driver
   
   3.read/write lock, and almost every synchronize modes in windows applications.
   
   4.all system events type struct/description.
   
   5.most windows log support, file io, usb io, network io, registry io, disk io, process activities etc.
   
   6.other important windows features: thread/completion port(fs minifilter queue) etc.
   
  open source for the people who like open source, contact us,I will give you permission:
  high performance cached list control(direct ui)
  kernel driver include:
  file system mini filter driver
  ndis intermediate driver
  ndis filter driver
  tdi driver
  wfp driver
  kernel hook driver
  io port filter driver(usb port)
  disk filter driver 
  registry filter driver
  high perfermonce log driver
 
 project is work on visual studio 2008

 build step:
 1. download wdk(or use internal wdk 8.1 copy)
 
 2. open visual studio 2008
 3. open bittrace/bittrace.sln
 4. compile projects in order: uilib,bittrace

well done.

any question you can contact with me.

Thanks:

Used software(Not open source):

Third party open source code:

uilib                                 

Contact:
    WeiXin:      jianzhi_ai
    WeiXin:      AIChangeLife 
	QQ group:    601169305
    QQ:          2146651351
	
 Notice: 

 1.bittrace will get all process status in system,  is in conflict with visual studio, when you debug bittrace, visual studio may hang.
 
 2.internal function draw system events to ui (the hot path code) is developing advanced feature, not open now.


I have not earning when I develope the open source software suit, it's hard to continue, if you think this software is good,
you can support us, join us, let the development continuous. alipay:dante.1265@hotmail.com

Third party open source project:
DUILib                 Written by Bjarke Viksoe (bjarke@viksoe.dk) Copyright (c) 2006-2007 Bjarke Viksoe.
                       duilib develop team(wangchyz@gmail.com, taxueliuyun@gmail.com, achellies@hotmail.com, tojen.me@gmail.com, ljh_0110@163.com)
					   and I have extend the list control, develop a high performance cached list control, and I like to publish to the duilib project.
					   if duilib team see this project, please contact us.
					   
CrashRpt               Copyright (c) 2003, Michael Carruth All rights reserved.

Email                  Copyright (C) 1996-2006 by Nakashima Tomoaki. All rights reserved. *		http://www.nakka.com/ *		nakka@nakka.com

MD5                    Copyright (C) 1995-1998 Eric Young (eay@cryptsoft.com) * All rights reserved.

SQLite                 

tinyxml                Copyright (c) 2000-2006 Lee Thomason (www.grinninglizard.com)

Third party LICENSE:
CrashRpt:

Copyright (c) 2003, Michael Carruth
All rights reserved.

Redistribution and use in source and binary forms, with or without modification, 
are permitted provided that the following conditions are met:

* Redistributions of source code must retain the above copyright notice, this 
list of conditions and the following disclaimer.

* Redistributions in binary form must reproduce the above copyright notice, 
this list of conditions and the following disclaimer in the documentation 
and/or other materials provided with the distribution.

* Neither the name of the author nor the names of its contributors 
may be used to endorse or promote products derived from this software without 
specific prior written permission.


THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY 
EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES 
OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT 
SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, 
INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED 
TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR 
BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

