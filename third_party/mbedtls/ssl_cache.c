/*-*- mode:c;indent-tabs-mode:nil;c-basic-offset:4;tab-width:4;coding:utf-8 -*-│
│ vi: set et ft=c ts=2 sts=2 sw=2 fenc=utf-8                               :vi │
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright The Mbed TLS Contributors                                          │
│                                                                              │
│ Licensed under the Apache License, Version 2.0 (the "License");              │
│ you may not use this file except in compliance with the License.             │
│ You may obtain a copy of the License at                                      │
│                                                                              │
│     http://www.apache.org/licenses/LICENSE-2.0                               │
│                                                                              │
│ Unless required by applicable law or agreed to in writing, software          │
│ distributed under the License is distributed on an "AS IS" BASIS,            │
│ WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.     │
│ See the License for the specific language governing permissions and          │
│ limitations under the License.                                               │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/log/log.h"
#include "libc/time.h"
#include "third_party/mbedtls/common.h"
#include "third_party/mbedtls/platform.h"
#include "third_party/mbedtls/ssl_cache.h"
#include "third_party/mbedtls/ssl_internal.h"
__static_yoink("mbedtls_notice");

/*
 *  SSL session cache implementation
 *
 *  Copyright The Mbed TLS Contributors
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
/*
 * These session callbacks use a simple chained list
 * to store and retrieve the session information.
 */

#if defined(MBEDTLS_SSL_CACHE_C)

void mbedtls_ssl_cache_init( mbedtls_ssl_cache_context *cache )
{
    mbedtls_platform_zeroize( cache, sizeof( mbedtls_ssl_cache_context ) );
    cache->timeout = MBEDTLS_SSL_CACHE_DEFAULT_TIMEOUT;
    cache->max_entries = MBEDTLS_SSL_CACHE_DEFAULT_MAX_ENTRIES;
}

int mbedtls_ssl_cache_get( void *data, mbedtls_ssl_session *session )
{
    int ret = 1;
#if defined(MBEDTLS_HAVE_TIME)
    mbedtls_time_t t = mbedtls_time( NULL );
#endif
    mbedtls_ssl_cache_context *cache = (mbedtls_ssl_cache_context *) data;
    mbedtls_ssl_cache_entry *cur, *entry;

    cur = cache->chain;
    entry = NULL;

    while( cur != NULL )
    {
        entry = cur;
        cur = cur->next;

#if defined(MBEDTLS_HAVE_TIME)
        if( cache->timeout != 0 &&
            (int) ( t - entry->timestamp ) > cache->timeout )
            continue;
#endif

        if( session->ciphersuite != entry->session.ciphersuite ||
            session->compression != entry->session.compression ||
            session->id_len != entry->session.id_len )
            continue;

        if( timingsafe_bcmp( session->id, entry->session.id,
                             entry->session.id_len ) != 0 )
            continue;

        ret = mbedtls_ssl_session_copy( session, &entry->session );
        if( ret != 0 )
        {
            ret = 1;
            goto exit;
        }

#if defined(MBEDTLS_X509_CRT_PARSE_C) && \
    defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE)
        /*
         * Restore peer certificate (without rest of the original chain)
         */
        if( entry->peer_cert.p != NULL )
        {
            /* `session->peer_cert` is NULL after the call to
             * mbedtls_ssl_session_copy(), because cache entries
             * have the `peer_cert` field set to NULL. */

            if( ( session->peer_cert = mbedtls_calloc( 1,
                                 sizeof(mbedtls_x509_crt) ) ) == NULL )
            {
                ret = 1;
                goto exit;
            }

            mbedtls_x509_crt_init( session->peer_cert );
            if( mbedtls_x509_crt_parse( session->peer_cert, entry->peer_cert.p,
                                entry->peer_cert.len ) != 0 )
            {
                mbedtls_free( session->peer_cert );
                session->peer_cert = NULL;
                ret = 1;
                goto exit;
            }
        }
#endif /* MBEDTLS_X509_CRT_PARSE_C && MBEDTLS_SSL_KEEP_PEER_CERTIFICATE */

        ret = 0;
        goto exit;
    }

exit:
    return( ret );
}

int mbedtls_ssl_cache_set( void *data, const mbedtls_ssl_session *session )
{
    int ret = 1;
#if defined(MBEDTLS_HAVE_TIME)
    mbedtls_time_t t = mbedtls_time( NULL ), oldest = 0;
    mbedtls_ssl_cache_entry *old = NULL;
#endif
    mbedtls_ssl_cache_context *cache = (mbedtls_ssl_cache_context *) data;
    mbedtls_ssl_cache_entry *cur, *prv;
    int count = 0;

    cur = cache->chain;
    prv = NULL;

    while( cur != NULL )
    {
        count++;

#if defined(MBEDTLS_HAVE_TIME)
        if( cache->timeout != 0 &&
            (int) ( t - cur->timestamp ) > cache->timeout )
        {
            cur->timestamp = t;
            break; /* expired, reuse this slot, update timestamp */
        }
#endif

        if( timingsafe_bcmp( session->id, cur->session.id, cur->session.id_len ) == 0 )
            break; /* client reconnected, keep timestamp for session id */

#if defined(MBEDTLS_HAVE_TIME)
        if( oldest == 0 || cur->timestamp < oldest )
        {
            oldest = cur->timestamp;
            old = cur;
        }
#endif

        prv = cur;
        cur = cur->next;
    }

    if( cur == NULL )
    {
#if defined(MBEDTLS_HAVE_TIME)
        /*
         * Reuse oldest entry if max_entries reached
         */
        if( count >= cache->max_entries )
        {
            if( old == NULL )
            {
                ret = 1;
                goto exit;
            }

            cur = old;
        }
#else /* MBEDTLS_HAVE_TIME */
        /*
         * Reuse first entry in chain if max_entries reached,
         * but move to last place
         */
        if( count >= cache->max_entries )
        {
            if( cache->chain == NULL )
            {
                ret = 1;
                goto exit;
            }

            cur = cache->chain;
            cache->chain = cur->next;
            cur->next = NULL;
            prv->next = cur;
        }
#endif /* MBEDTLS_HAVE_TIME */
        else
        {
            /*
             * max_entries not reached, create new entry
             */
            cur = mbedtls_calloc( 1, sizeof(mbedtls_ssl_cache_entry) );
            if( cur == NULL )
            {
                ret = 1;
                goto exit;
            }

            if( prv == NULL )
                cache->chain = cur;
            else
                prv->next = cur;
        }

#if defined(MBEDTLS_HAVE_TIME)
        cur->timestamp = t;
#endif
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C) && \
    defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE)
    /*
     * If we're reusing an entry, free its certificate first
     */
    if( cur->peer_cert.p != NULL )
    {
        mbedtls_free( cur->peer_cert.p );
        mbedtls_platform_zeroize( &cur->peer_cert, sizeof(mbedtls_x509_buf) );
    }
#endif /* MBEDTLS_X509_CRT_PARSE_C && MBEDTLS_SSL_KEEP_PEER_CERTIFICATE */

    /* Copy the entire session; this temporarily makes a copy of the
     * X.509 CRT structure even though we only want to store the raw CRT.
     * This inefficiency will go away as soon as we implement on-demand
     * parsing of CRTs, in which case there's no need for the `peer_cert`
     * field anymore in the first place, and we're done after this call. */
    ret = mbedtls_ssl_session_copy( &cur->session, session );
    if( ret != 0 )
    {
        ret = 1;
        goto exit;
    }

#if defined(MBEDTLS_X509_CRT_PARSE_C) && \
    defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE)
    /* If present, free the X.509 structure and only store the raw CRT data. */
    if( cur->session.peer_cert != NULL )
    {
        cur->peer_cert.p =
            mbedtls_calloc( 1, cur->session.peer_cert->raw.len );
        if( cur->peer_cert.p == NULL )
        {
            ret = 1;
            goto exit;
        }

        memcpy( cur->peer_cert.p,
                cur->session.peer_cert->raw.p,
                cur->session.peer_cert->raw.len );
        cur->peer_cert.len = session->peer_cert->raw.len;

        mbedtls_x509_crt_free( cur->session.peer_cert );
        mbedtls_free( cur->session.peer_cert );
        cur->session.peer_cert = NULL;
    }
#endif /* MBEDTLS_X509_CRT_PARSE_C && MBEDTLS_SSL_KEEP_PEER_CERTIFICATE */

    ret = 0;

exit:
    return( ret );
}

#if defined(MBEDTLS_HAVE_TIME)
void mbedtls_ssl_cache_set_timeout( mbedtls_ssl_cache_context *cache, int timeout )
{
    if( timeout < 0 ) timeout = 0;

    cache->timeout = timeout;
}
#endif /* MBEDTLS_HAVE_TIME */

void mbedtls_ssl_cache_set_max_entries( mbedtls_ssl_cache_context *cache, int max )
{
    if( max < 0 ) max = 0;

    cache->max_entries = max;
}

void mbedtls_ssl_cache_free( mbedtls_ssl_cache_context *cache )
{
    mbedtls_ssl_cache_entry *cur, *prv;

    cur = cache->chain;

    while( cur != NULL )
    {
        prv = cur;
        cur = cur->next;

        mbedtls_ssl_session_free( &prv->session );

#if defined(MBEDTLS_X509_CRT_PARSE_C) && \
    defined(MBEDTLS_SSL_KEEP_PEER_CERTIFICATE)
        mbedtls_free( prv->peer_cert.p );
#endif /* MBEDTLS_X509_CRT_PARSE_C && MBEDTLS_SSL_KEEP_PEER_CERTIFICATE */

        mbedtls_free( prv );
    }

    cache->chain = NULL;
}

#endif /* MBEDTLS_SSL_CACHE_C */
