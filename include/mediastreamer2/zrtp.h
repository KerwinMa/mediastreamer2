/*
 mediastreamer2 library - modular sound and video processing and streaming
 Copyright (C) 2014 Belledonne Communications

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */

#ifndef ms_zrtp_h
#define ms_zrtp_h

#include <ortp/rtpsession.h>
#include "mediastreamer2/mscommon.h"

#ifdef __cplusplus
extern "C"{
#endif

/* defined in mediastream.h */
struct _MSMediaStreamSessions;

typedef struct MSZrtpParams {
	const char *zid_file; // File where to store secrets and other information
	const char *uri; /* the sip URI of correspondant */
} MSZrtpParams;

typedef struct _MSZrtpContext MSZrtpContext ;

/**
 * check if ZRTP is available
 * @return TRUE if it is available, FALSE if not
 */
MS2_PUBLIC bool_t ms_zrtp_available(void);

/**
 * Create an initialise a ZRTP context
 * @param[in]	stream_sessions		A link to the stream sessions structures, used to get rtp session to add transport modifier and needed to set SRTP sessions when keys are ready
 * @param[in]	params			ZID cache filename and peer sip uri
 * @return	a pointer to the opaque context structure needed by MSZRTP
 */
MS2_PUBLIC MSZrtpContext* ms_zrtp_context_new(struct _MSMediaStreamSessions *stream_sessions, MSZrtpParams *params);

/**
 * Create an initialise a ZRTP context on a channel when a ZRTP exchange was already performed on an other one
 * @param[in]	stream_sessions		A link to the stream sessions structures, used to get rtp session to add transport modifier and needed to set SRTP sessions when keys are ready
 * @param[in]	activeContext		The MSZRTP context of the already active session, used to pass to lib bzrtp its own context which shall remain unique.
 * @param[in]	params			ZID cache filename and peer sip uri
 * @return	a pointer to the opaque context structure needed by MSZRTP
 */
MS2_PUBLIC MSZrtpContext* ms_zrtp_multistream_new(struct _MSMediaStreamSessions *stream_sessions, MSZrtpContext* activeContext, MSZrtpParams *params);

/**
 * Free ressources used by ZRTP context
 * it will also free the libbzrtp context if no more channel are active
 * @param[in/out]	context		the opaque MSZRTP context
 */
MS2_PUBLIC void ms_zrtp_context_destroy(MSZrtpContext *ctx);

/**
 * can be used to give more time for establishing zrtp session
 * @param[in] ctx	The MSZRTP context
 * */
MS2_PUBLIC void ms_zrtp_reset_transmition_timer(MSZrtpContext* ctx);

/**
 * Tell the MSZRTP context that SAS was controlled by user, it will trigger a ZID cache update
 * @param[in]	ctx	MSZRTP context, used to retrieve cache and update it
 */
MS2_PUBLIC void ms_zrtp_sas_verified(MSZrtpContext* ctx);

/**
 * Tell the MSZRTP context that user have requested the SAS verified status to be reseted, it will trigger a ZID cache update
 * @param[in]	ctx	MSZRTP context, used to retrieve cache and update it
 */
MS2_PUBLIC void ms_zrtp_sas_reset_verified(MSZrtpContext* ctx);

#ifdef __cplusplus
}
#endif

#endif /* ms_zrtp_h */
