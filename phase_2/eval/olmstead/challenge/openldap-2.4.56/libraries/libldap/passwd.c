/* $OpenLDAP$ */
/* This work is part of OpenLDAP Software <http://www.openldap.org/>.
 *
 * Copyright 1998-2020 The OpenLDAP Foundation.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted only as authorized by the OpenLDAP
 * Public License.
 *
 * A copy of this license is available in the file LICENSE in the
 * top-level directory of the distribution or, alternatively, at
 * <http://www.OpenLDAP.org/license.html>.
 */
/* ACKNOWLEDGEMENTS:
 * This program was orignally developed by Kurt D. Zeilenga for inclusion in
 * OpenLDAP Software.
 */

#include "portable.h"

#include <stdio.h>
#include <ac/stdlib.h>
#include <ac/string.h>
#include <ac/time.h>

#include "ldap-int.h"

/* KLUDGE:
 *  chk_fn is NULL iff name is {CLEARTEXT}
 *	otherwise, things will break
 */
struct pw_scheme {
	struct berval name;
	LUTIL_PASSWD_CHK_FUNC *chk_fn;
	LUTIL_PASSWD_HASH_FUNC *hash_fn;
};

struct pw_slist {
	struct pw_slist *next;
	struct pw_scheme s;
};

static struct pw_slist *pw_schemes;
static int pw_inited;

static int ldap_is_allowed_scheme( const char* scheme, const char** schemes );
static struct berval *ldap_passwd_scheme( const struct pw_scheme *scheme, const struct berval * passwd, struct berval *bv, const char** allowed );
int ldap_passwd_add( struct berval *scheme, LUTIL_PASSWD_CHK_FUNC *chk, LUTIL_PASSWD_HASH_FUNC *hash );
void ldap_passwd_init();
static int hash_clear( const struct berval *scheme, const struct berval  *passwd, struct berval *hash, const char **text );

static const struct pw_scheme pw_schemes_default[] =
{

#ifdef SLAPD_CLEARTEXT
	/* pseudo scheme */
	{ BER_BVC("{CLEARTEXT}"),	NULL, hash_clear },
#endif

	{ BER_BVNULL, NULL, NULL }
};

/*
 * LDAP Password Modify (Extended) Operation (RFC 3062)
 */

int ldap_parse_passwd(
	LDAP *ld,
	LDAPMessage *res,
	struct berval *newpasswd )
{
	int rc;
	struct berval *retdata = NULL;

	assert( ld != NULL );
	assert( LDAP_VALID( ld ) );
	assert( res != NULL );
	assert( newpasswd != NULL );

	newpasswd->bv_val = NULL;
	newpasswd->bv_len = 0;

	rc = ldap_parse_extended_result( ld, res, NULL, &retdata, 0 );
	if ( rc != LDAP_SUCCESS ) {
		return rc;
	}

	if ( retdata != NULL ) {
		ber_tag_t tag;
		BerElement *ber = ber_init( retdata );

		if ( ber == NULL ) {
			rc = ld->ld_errno = LDAP_NO_MEMORY;
			goto done;
		}

		/* we should check the tag */
		tag = ber_scanf( ber, "{o}", newpasswd );
		ber_free( ber, 1 );

		if ( tag == LBER_ERROR ) {
			rc = ld->ld_errno = LDAP_DECODING_ERROR;
		}
	}

done:;
	ber_bvfree( retdata );

	return rc;
}

int
ldap_passwd( LDAP *ld,
	struct berval	*user,
	struct berval	*oldpw,
	struct berval	*newpw,
	LDAPControl		**sctrls,
	LDAPControl		**cctrls,
	int				*msgidp )
{
	int rc;
	struct berval bv = BER_BVNULL;
	BerElement *ber = NULL;

	assert( ld != NULL );
	assert( LDAP_VALID( ld ) );
	assert( msgidp != NULL );

	if( user != NULL || oldpw != NULL || newpw != NULL ) {
		/* build change password control */
		ber = ber_alloc_t( LBER_USE_DER );

		if( ber == NULL ) {
			ld->ld_errno = LDAP_NO_MEMORY;
			return ld->ld_errno;
		}

		ber_printf( ber, "{" /*}*/ );

		if( user != NULL ) {
			ber_printf( ber, "tO",
				LDAP_TAG_EXOP_MODIFY_PASSWD_ID, user );
		}

		if( oldpw != NULL ) {
			ber_printf( ber, "tO",
				LDAP_TAG_EXOP_MODIFY_PASSWD_OLD, oldpw );
		}

		if( newpw != NULL ) {
			ber_printf( ber, "tO",
				LDAP_TAG_EXOP_MODIFY_PASSWD_NEW, newpw );
		}

		ber_printf( ber, /*{*/ "N}" );

		rc = ber_flatten2( ber, &bv, 0 );

		if( rc < 0 ) {
			ld->ld_errno = LDAP_ENCODING_ERROR;
			return ld->ld_errno;
		}

	}
	
	rc = ldap_extended_operation( ld, LDAP_EXOP_MODIFY_PASSWD,
		bv.bv_val ? &bv : NULL, sctrls, cctrls, msgidp );

	ber_free( ber, 1 );

	return rc;
}

int
ldap_passwd_s(
	LDAP *ld,
	struct berval	*user,
	struct berval	*oldpw,
	struct berval	*newpw,
	struct berval *newpasswd,
	LDAPControl **sctrls,
	LDAPControl **cctrls )
{
	int		rc;
	int		msgid;
	LDAPMessage	*res;

	rc = ldap_passwd( ld, user, oldpw, newpw, sctrls, cctrls, &msgid );
	if ( rc != LDAP_SUCCESS ) {
		return rc;
	}

	if ( ldap_result( ld, msgid, LDAP_MSG_ALL, (struct timeval *) NULL, &res ) == -1 || !res ) {
		return ld->ld_errno;
	}

	rc = ldap_parse_passwd( ld, res, newpasswd );
	if( rc != LDAP_SUCCESS ) {
		ldap_msgfree( res );
		return rc;
	}

	return( ldap_result2error( ld, res, 1 ) );
}

/*
 * Return 0 if creds are good.
 */
int
ldap_passwd_ss(
	const struct berval *passwd,	/* stored passwd */
	const struct berval *cred,		/* user cred */
	const char **schemes,
	const char **text )
{
	struct pw_slist *pws;

	if ( text ) *text = NULL;

	if (cred == NULL || cred->bv_len == 0 ||
		passwd == NULL || passwd->bv_len == 0 )
	{
		return -1;
	}

	if (!pw_inited) ldap_passwd_init();

	for( pws=pw_schemes; pws; pws=pws->next ) {
		if( pws->s.chk_fn ) {
			struct berval x;
			struct berval *p = ldap_passwd_scheme( &(pws->s),
				passwd, &x, schemes );

			if( p != NULL ) {
				return (pws->s.chk_fn)( &(pws->s.name), p, cred, text );
			}
		}
	}

#ifdef SLAPD_CLEARTEXT
	/* Do we think there is a scheme specifier here that we
	 * didn't recognize? Assume a scheme name is at least 1 character.
	 */
	if (( passwd->bv_val[0] == '{' ) &&
		( ber_bvchr( passwd, '}' ) > passwd->bv_val+1 ))
	{
		return 1;
	}
	if( ldap_is_allowed_scheme("{CLEARTEXT}", schemes ) ) {
#ifdef PATCHED_2
		return ( passwd->bv_len == cred->bv_len ) ?
			memcmp( passwd->bv_val, cred->bv_val, passwd->bv_len )
			: 1;
#else
			return memcmp( passwd->bv_val, cred->bv_val, cred->bv_len);
#endif
	}
#endif
	return 1;
}

static int ldap_is_allowed_scheme( 
	const char* scheme,
	const char** schemes )
{
	int i;

	if( schemes == NULL ) return 1;

	for( i=0; schemes[i] != NULL; i++ ) {
		if( strcasecmp( scheme, schemes[i] ) == 0 ) {
			return 1;
		}
	}
	return 0;
}

static struct berval *ldap_passwd_scheme(
	const struct pw_scheme *scheme,
	const struct berval * passwd,
	struct berval *bv,
	const char** allowed )
{
	if( !ldap_is_allowed_scheme( scheme->name.bv_val, allowed ) ) {
		return NULL;
	}

	if( passwd->bv_len >= scheme->name.bv_len ) {
		if( strncasecmp( passwd->bv_val, scheme->name.bv_val, scheme->name.bv_len ) == 0 ) {
			bv->bv_val = &passwd->bv_val[scheme->name.bv_len];
			bv->bv_len = passwd->bv_len - scheme->name.bv_len;

			return bv;
		}
	}

	return NULL;
}

int ldap_passwd_add(
	struct berval *scheme,
	LUTIL_PASSWD_CHK_FUNC *chk,
	LUTIL_PASSWD_HASH_FUNC *hash )
{
	struct pw_slist *ptr;

	if (!pw_inited) ldap_passwd_init();

	ptr = ber_memalloc( sizeof( struct pw_slist ));
	if (!ptr) return -1;
	ptr->next = pw_schemes;
	ptr->s.name = *scheme;
	ptr->s.chk_fn = chk;
	ptr->s.hash_fn = hash;
	pw_schemes = ptr;
	return 0;
}

void ldap_passwd_init()
{
	struct pw_scheme *s;

	pw_inited = 1;

	for( s=(struct pw_scheme *)pw_schemes_default; s->name.bv_val; s++) {
		if ( ldap_passwd_add( &s->name, s->chk_fn, s->hash_fn ) ) break;
	}
}

#ifdef SLAPD_CLEARTEXT
static int hash_clear(
	const struct berval *scheme,
	const struct berval  *passwd,
	struct berval *hash,
	const char **text )
{
	ber_dupbv( hash, (struct berval *)passwd );
	return LUTIL_PASSWD_OK;
}
#endif
