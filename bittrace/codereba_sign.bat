certmgr.exe -add codereba_cert.cer -s -r localMachine root
certmgr.exe -add codereba_cert.cer -s -r localMachine trustedpublisher
Bcdedit.exe -set TESTSIGNING ON