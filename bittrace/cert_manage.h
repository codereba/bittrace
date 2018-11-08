/*
 *
 * Copyright 2010 JiJie Shi(weixin:AIChangeLife)
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

#ifndef __CERT_MANAGE_H__
#define __CERT_MANAGE_H__

#define CODEREBA_CERT_ENCODE_TYPE ( PKCS_7_ASN_ENCODING | X509_ASN_ENCODING )
#define CODEREBA_CERTIFICATE_FILE_NAME L"codereba_cert.cer"
#define CODEREBA_CERT_STORE_NAME L"CODEREBA_CERT"

LRESULT WINAPI check_cedereba_cert_installed(); 

LRESULT WINAPI install_cedereba_cert(); 

#endif //__CERT_MANAGE_H__