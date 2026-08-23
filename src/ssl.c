/******************************************************************************
*   This file is part of TinTin++                                             *
*                                                                             *
*   Copyright 2014-2026 Igor van den Hoven                                    *
*                                                                             *
*   SPDX-License-Identifier: LGPL-2.1-or-later                                *
******************************************************************************/

/******************************************************************************
*                (T)he K(I)cki(N) (T)ickin D(I)kumud Clie(N)t                 *
*                                                                             *
*                         coded by Adam Borowski 2008                         *
*                      recoded by Igor van den Hoven 2014                     *
******************************************************************************/

#include "tintin.h"

#ifdef HAVE_GNUTLS_H

static gnutls_certificate_credentials_t global_ssl_cred = NULL;

static int ssl_check_cert(struct session *ses, gnutls_session_t sslses);

DO_COMMAND(do_ssl)
{
	char temp[BUFFER_SIZE];

	substitute(ses, arg, temp, SUB_VAR|SUB_FUN);

	arg = temp;

	arg = get_arg_in_braces(ses, arg, arg1, GET_ONE);

	if (*arg1 == 0 || *arg == 0)
	{
		show_error(ses, LIST_COMMAND, "#SYNTAX: #SSL <NAME> <HOST> <PORT>");
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
	
	if (!global_ssl_cred)
	{
		gnutls_global_init();
		gnutls_certificate_allocate_credentials(&global_ssl_cred);
	}

	if (gnutls_init(&ssl_ses, GNUTLS_CLIENT) != GNUTLS_E_SUCCESS)
	{
		show_error(ses, LIST_COMMAND, "#SSL: Failed to initialize GnuTLS.");

		return 0;
	}

	gnutls_set_default_priority(ssl_ses);
	gnutls_credentials_set(ssl_ses, GNUTLS_CRD_CERTIFICATE, global_ssl_cred);
	gnutls_transport_set_ptr(ssl_ses, (gnutls_transport_ptr_t) (intptr_t) ses->socket);
	gnutls_server_name_set(ssl_ses, GNUTLS_NAME_DNS, ses->session_host, strlen(ses->session_host));

	do 
	{
		ret = gnutls_handshake(ssl_ses);
	}
	while (ret == GNUTLS_E_AGAIN || ret == GNUTLS_E_INTERRUPTED);

	if (ret < 0)
	{
		tintin_printf2(ses, "#SSL: GnuTLS handshake failed: %s", gnutls_strerror(ret));
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
	char filename[BUFFER_SIZE], *ptr;

	sprintf(filename, "%s_%s", ses->session_host, ses->session_port);

	ptr = filename;

	while (*ptr)
	{
		if (is_varchar(*ptr))
		{
			ptr++;
		}
		else if (*ptr == ':' || *ptr == '.' || *ptr == '-')
		{
			*ptr++ = '_';
		}
		else
		{
			return 0;
		}
	}
	sprintf(result, "%s/ssl/%s.crt", gtd->system->tt_dir, filename);

	return 1;
}


static void load_cert(struct session *ses, gnutls_x509_crt_t *cert)
{
	char cert_file[STRING_SIZE];
	FILE *fp;
	gnutls_datum_t cert_data;
	
	if (!get_cert_file(ses, cert_file))
	{
		return;
	}

	if ((fp = fopen(cert_file, "r")) == NULL)
	{
		return;
	}

	cert_data.size = fread(cert_file, 1, STRING_SIZE, fp);
	cert_data.data = (unsigned char *) cert_file;

	fclose(fp);
	
	if (gnutls_x509_crt_init(cert) != GNUTLS_E_SUCCESS)
	{
		*cert = NULL;

		return;
	}

	if (gnutls_x509_crt_import(*cert, &cert_data, GNUTLS_X509_FMT_PEM) != GNUTLS_E_SUCCESS)
	{
		gnutls_x509_crt_deinit(*cert);

		*cert = NULL;
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

	sprintf(filename, "%s", gtd->system->tt_dir);

	if (mkdir(filename, 0755) && errno != EEXIST)
	{
		tintin_printf(ses, "#SSL: FAILED TO CREATE TINTIN DIR %s (%s)", filename, strerror(errno));

		return;
	}

	sprintf(filename, "%s/ssl", gtd->system->tt_dir);

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
	char filename[BUFFER_SIZE], buf2[BUFFER_SIZE];
	time_t t;
	gnutls_x509_crt_t cert, oldcert;
	const gnutls_datum_t *cert_list;
	unsigned int cert_list_size;
	char *err = NULL;

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

	t = time(NULL);

	if (gnutls_x509_crt_get_activation_time(cert) > t)
	{
		sprintf(buf2, "CERTIFICATE ACTIVATION TIME IS IN THE FUTURE (%s)", str_time(ses, "%c", gnutls_x509_crt_get_activation_time(cert)));

		err = buf2;
	}
	
	if (gnutls_x509_crt_get_expiration_time(cert) < t)
	{
		sprintf(buf2, "CERTIFICATE HAS EXPIRED (%s)", str_time(ses, "%c", gnutls_x509_crt_get_expiration_time(cert)));

		err = buf2;
	}

	if (!oldcert)
	{
		save_cert(ses, cert, 1);
	}
	else if (diff_certs(cert, oldcert))
	{
		t -= gnutls_x509_crt_get_expiration_time(oldcert);

		if (err || t < -31*24*3600)
		{
			if (err)
			{
				char temp[BUFFER_SIZE];

				sprintf(temp, "CERTIFICATE MISMATCH, AND NEW ");
				strcat(temp, err);

				strcpy(buf2, temp);
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

			get_cert_file(ses, filename);

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
