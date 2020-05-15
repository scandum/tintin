/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2008-2019 Adam Borowski and Igor van den Hoven                  *
*                                                                             *
*   TinTin++ is free software; you can redistribute it and/or modify          *
*   it under the terms of the GNU General Public License as published by      *
*   the Free Software Foundation; either version 3 of the License, or         *
*   (at your option) any later version.                                       *
*                                                                             *
*   This program is distributed in the hope that it will be useful,           *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of            *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
*   GNU General Public License for more details.                              *
*                                                                             *
*                                                                             *
*   You should have received a copy of the GNU General Public License         *
*   along with TinTin++.  If not, see https://www.gnu.org/licenses.           *
******************************************************************************/

/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                         coded by Adam Borowski 2008                         *
*                   modifications by Igor van den Hoven 2014                  *
******************************************************************************/

#include "tintin.h"

#ifdef HAVE_GNUTLS_H

static gnutls_certificate_credentials_t ssl_cred = 0;

static int ssl_check_cert(struct session *ses, gnutls_session_t sslses);

DO_COMMAND(do_ssl)
{
	char temp[BUFFER_SIZE];

	substitute(ses, arg, temp, SUB_VAR|SUB_FUN);

	arg = temp;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0 || *arg == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SSL {name} {host} {port}");
	}
	else
	{
		ses = new_session(ses, arg1, arg, 0, 1);
	}
	return ses;
}

gnutls_session_t ssl_negotiate(struct session *ses)
{
	gnutls_session_t ssl_ses;

	int ret;
	
	if (!ssl_cred)
	{
		gnutls_global_init();
		gnutls_certificate_allocate_credentials(&ssl_cred);
	}

	gnutls_init(&ssl_ses, GNUTLS_CLIENT);
	gnutls_set_default_priority(ssl_ses);
	gnutls_credentials_set(ssl_ses, GNUTLS_CRD_CERTIFICATE, ssl_cred);
	gnutls_transport_set_ptr(ssl_ses, (gnutls_transport_ptr_t) (long int) ses->socket);

	do 
	{
		ret = gnutls_handshake(ssl_ses);
	}
	while (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED);

	if (ret)
	{
		tintin_printf2(ses, "#SSL: handshake failed error: %s", gnutls_strerror(ret));
		gnutls_deinit(ssl_ses);
		return 0;
	}
/*
	{
		char *debug = gnutls_session_get_desc(ssl_ses);

		tintin_printf2(ses, "#SSL: %s", debug);

		gnutls_free(debug);
	}
*/
	if (!ssl_check_cert(ses, ssl_ses))
	{
		gnutls_deinit(ssl_ses);
		return 0;
	}
	return ssl_ses;
}


static int get_cert_file(struct session *ses, char *result)
{
	char name[BUFFER_SIZE], *ptr;

	sprintf(name, "%s_%s", ses->session_host, ses->session_port);

	ptr = name;

	while (*ptr)
	{
		if (*ptr == ':')
		{
			*ptr++ = '.';
		}
		else if (isalnum((int) *ptr) || *ptr == '-' || *ptr == '.' || *ptr == '_')
		{
			ptr++;
		}
		else
		{
			return 0;
		}
	}

	sprintf(result, "%s/%s/ssl/%s.crt", gtd->home, TINTIN_DIR, name);

	return 1;
}


static void load_cert(struct session *ses, gnutls_x509_crt_t *cert)
{
	char cert_file[STRING_SIZE];
	FILE *fp;
	gnutls_datum_t bptr;
	
	if (!get_cert_file(ses, cert_file))
	{
		return;
	}

	if ((fp = fopen(cert_file, "r")) == NULL)
	{
		return;
	}

	bptr.size = fread(cert_file, 1, STRING_SIZE, fp);
	bptr.data = (unsigned char *) cert_file;

	fclose(fp);
	
	gnutls_x509_crt_init(cert);

	if (gnutls_x509_crt_import(*cert, &bptr, GNUTLS_X509_FMT_PEM))
	{
		gnutls_x509_crt_deinit(*cert);

		*cert = 0;
	}
}

static void save_cert(struct session *ses, gnutls_x509_crt_t cert, int new)
{
	char filename[BUFFER_SIZE], buf[STRING_SIZE];
	FILE *fp;
	size_t len;
	
	len = STRING_SIZE;

	if (gnutls_x509_crt_export(cert, GNUTLS_X509_FMT_PEM, buf, &len))
	{
		return;
	}

	sprintf(filename, "%s/%s", gtd->home, TINTIN_DIR);

	if (mkdir(filename, 0777) && errno != EEXIST)
	{
		tintin_printf(ses, "#SSL: FAILED TO CREATE TINTIN DIR %s (%s)", filename, strerror(errno));

		return;
	}

	sprintf(filename, "%s/%s/ssl", gtd->home, TINTIN_DIR);

	mkdir(filename, 0755);

	if (mkdir(filename, 0755) && errno != EEXIST)
	{
		tintin_printf(ses, "#SSL: CANNOT CREATE CERTS DIR %s (%s)", filename, strerror(errno));

		return;
	}

	if (!get_cert_file(ses, filename))
	{
		return;
	}

	if (new)
	{
		tintin_printf(ses, "#SSL: THIS IS THE FIRST TIME YOU CONNECT TO THIS SERVER.");
	}

	tintin_printf(ses, "#SSL: SAVING SERVER CERTIFICATE TO %s", filename);

	if ((fp = fopen(filename, "w")) == NULL)
	{
		tintin_printf(ses, "#SSL: SAVE FAILED (%s)", strerror(errno));

		return;
	}

	if (fwrite(buf, 1, len, fp) != len)
	{
		tintin_printf(ses, "#SSL: SAVE FAILED (%s)", strerror(errno));
		fclose(fp);
		unlink(filename);

		return;
	}

	if (fclose(fp))
	{
		tintin_printf(ses, "#SSL: SAVE FAILED (%s)", strerror(errno));
	}
}


static int diff_certs(gnutls_x509_crt_t c1, gnutls_x509_crt_t c2)
{
	char buf1[STRING_SIZE], buf2[STRING_SIZE];
	size_t len1, len2;
	
	len1 = len2 = STRING_SIZE;

	if (gnutls_x509_crt_export(c1, GNUTLS_X509_FMT_DER, buf1, &len1))
	{
		return 1;
	}

	if (gnutls_x509_crt_export(c2, GNUTLS_X509_FMT_DER, buf2, &len2))
	{
		return 1;
	}

	if (len1 != len2)
	{
		return 1;
	}

	return memcmp(buf1, buf2, len1);
}


static int ssl_check_cert(struct session *ses, gnutls_session_t ssl_ses)
{
	char filename[BUFFER_SIZE], buf2[BUFFER_SIZE], *bptr;
	time_t t;
	gnutls_x509_crt_t cert, oldcert;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size;
	char *err = 0;

	oldcert = 0;

	load_cert(ses, &oldcert);

	if (gnutls_certificate_type_get(ssl_ses) != GNUTLS_CRT_X509)
	{
		err = "#SSL: SERVER DOES NOT USE x509 -> NO KEY RETENTION.";
		goto nocert;
	}
	
	if ((cert_list = gnutls_certificate_get_peers(ssl_ses, &cert_list_size)) == NULL)
	{
		err = "#SSL: SERVER HAS NO x509 CERTIFICATE -> NO KEY RETENTION.";
		goto nocert;
	}
	
	gnutls_x509_crt_init(&cert);

	if (gnutls_x509_crt_import(cert, &cert_list[0], GNUTLS_X509_FMT_DER) < 0)
	{
		err = "#SSL: SERVER'S CERTIFICATE IS INVALID.";
		goto badcert;
	}
	
	t = time(0);

	if (gnutls_x509_crt_get_activation_time(cert) > t)
	{
		sprintf(buf2, "%s", ctime(&t));

		if ((bptr = strchr(buf2, '\n')))
		{
			*bptr = 0;
		}
		sprintf(filename, "CERTIFICATE ACTIVATION TIME IS IN THE FUTURE (%s)", buf2);

		err = filename;
	}
	
	if (gnutls_x509_crt_get_expiration_time(cert)<t)
	{
		sprintf(buf2, "%s", ctime(&t));

		if ((bptr = strchr(buf2, '\n')))
		{
			*bptr = 0;
		}
		sprintf(filename, "CERTIFICATE HAS EXPIRED (%s)", buf2);
		err = filename;
	}
	
	if (!oldcert)
	{
		save_cert(ses, cert, 1);
	}
	else if (diff_certs(cert, oldcert))
	{
		t -= gnutls_x509_crt_get_expiration_time(oldcert);

		if (err || t < -7*24*3600)
		{
			if (err)
			{
				sprintf(buf2, "CERTIFICATE MISMATCH, AND NEW %s", err);
			}
			else
			{
				sprintf(buf2, "SERVER CERTIFICATE IS DIFFERENT FROM THE SAVED ONE.");
			}
			err = buf2;
		}
		else
		{
			if (t > 0)
			{
				tintin_printf(ses, "#SSL: SERVER CERTIFICATE HAS CHANGED, BUT THE OLD ONE WAS EXPIRED.");
			}
			else
			{
				tintin_printf(ses, "#SSL: SERVER CERTIFICATE HAS CHANGED, BUT THE OLD ONE WAS ABOUT TO EXPIRE.");
			}

			/* Replace the old cert */

			save_cert(ses, cert, 0);
			gnutls_x509_crt_deinit(oldcert);
			oldcert = 0;
		}
	}
	else
	{
		/* All is well */
		gnutls_x509_crt_deinit(oldcert);
		oldcert = 0;
	}

badcert:
	gnutls_x509_crt_deinit(cert);
	
nocert:
	if (oldcert)
	{
		gnutls_x509_crt_deinit(oldcert);
	}

	if (err)
	{
		if (oldcert)
		{
			tintin_printf(ses, "#SSL ERROR: %s", err);

			tintin_printf(ses, "#SSL ALERT: THE SERVER'S SETTINGS WERE CHANGED IN AN UNEXPECTED WAY.");
			tintin_printf(ses, "#SSL ALERT: YOU MAY BE VULNERABLE TO MAN-IN-THE-MIDDLE ATTACKS.");
			tintin_printf(ses, "#SSL ALERT: TO CONTINUE, PLEASE DELETE THE FILE:");
			tintin_printf(ses, "#SSL ALERT: %s", filename);
			tintin_printf(ses, "#SSL ERROR: ABORTING CONNECTION.");
			return 0;
		}
		else
		{
			tintin_printf(ses, "#SSL ALERT: %s", err);
			tintin_printf(ses, "#SSL ALERT: YOU MAY BE VULNERABLE TO MAN-IN-THE-MIDDLE ATTACKS.");
			return 2;
		}
	}
	else
	{
		return 1;
	}
}

#else

DO_COMMAND(do_ssl)
{
	tintin_printf2(ses, "The GnuTLS library wasn't found. Install GnuTLS, run ./configure, and recompile for SSL support.");

	return ses;
}

#endif
