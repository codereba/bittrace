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

#ifndef __CERT_MANAGE_H__
#define __CERT_MANAGE_H__

#define CODEREBA_CERT_ENCODE_TYPE ( PKCS_7_ASN_ENCODING | X509_ASN_ENCODING )
#define CODEREBA_CERTIFICATE_FILE_NAME L"codereba_cert.cer"
#define CODEREBA_CERT_STORE_NAME L"CODEREBA_CERT"

LRESULT WINAPI check_cedereba_cert_installed(); 

LRESULT WINAPI install_cedereba_cert(); 

#endif //__CERT_MANAGE_H__