/* Copyright (c) 2011, CPqD, Brasil
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of the Ericsson Research nor the names of its
 *     contributors may be used to endorse or promote products derived from
 *     this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 *
 * Author: Eder Leão Fernandes <ederlf@cpqd.com.br>
 */
 #ifndef OFL_EXP_EXT_MESSAGES_H

#include <sys/types.h>
#include <stdbool.h>
#include <stdio.h>

#include "openflow/openflow.h"
#include "openflow/match-ext.h"
#include "../oflib/ofl-messages.h"


struct ofl_ext_msg_header {
    struct ofl_msg_experimenter   header; /* OPENFLOW_EXTENDED_MATCH_ID */
    
    uint32_t   type;
};


/* extT_SET_FLOW_FORMAT request. */
struct ofl_ext_set_flow_format {
    struct ofp_header header;
    uint32_t subtype;           /* extT_SET_FLOW_FORMAT. */
    uint32_t format;            /* One of extFF_*. */
};

/* EXT_FLOW_MOD (analogous to OFPT_FLOW_MOD). */
struct ofl_ext_flow_mod {
    struct ofl_ext_msg_header header;
    uint64_t cookie;              /* Opaque controller-issued identifier. */
    uint64_t cookie_mask;         /* Mask used to restrict the cookie bits
                                     that must match when the command is
                                     OFPFC_MODIFY* or OFPFC_DELETE*. A value
                                     of 0 indicates no restriction. */  
    uint8_t table_id;             /* ID of the table to put the flow in */
    uint8_t command;             /* One of OFPFC_*. */
    uint16_t idle_timeout;        /* Idle time before discarding (seconds). */
    uint16_t hard_timeout;        /* Max time before discarding (seconds). */
    uint16_t priority;            /* Priority level of flow entry. */
    uint32_t buffer_id;           /* Buffered packet to apply to (or -1).
                                     Not meaningful for OFPFC_DELETE*. */
    uint32_t out_port;            /* For OFPFC_DELETE* commands, require
                                     matching entries to include this as an
                                     output port.  A value of OFPP_NONE
                                     indicates no restriction. */
    uint32_t out_group;            /* For OFPFC_DELETE* commands, require
                                    matching entries to include this as an
                                    output group. A value of OFPG_ANY
                                    indicates no restriction. */
    uint16_t flags;                /* One of OFPFF_*. */
    
    struct ofl_match_header        *match;        /* Fields to match */
    size_t                          instructions_num;
    struct ofl_instruction_header **instructions; /* Instruction set */
};

/* EXT_FLOW_REMOVED (analogous to OFPT_FLOW_REMOVED). */
struct ofl_ext_flow_removed {
    struct ofl_ext_msg_header header;
    uint64_t cookie;          /* Opaque controller-issued identifier. */
    uint16_t priority;        /* Priority level of flow entry. */
    uint8_t reason;           /* One of OFPRR_*. */
    uint8_t table_id;         /* ID of the table */
    uint32_t duration_sec;    /* Time flow was alive in seconds. */
    uint32_t duration_nsec;   /* Time flow was alive in nanoseconds beyond
                                 duration_sec. */
    uint16_t idle_timeout;    /* Idle timeout from original flow mod. */
    uint64_t packet_count;
    uint64_t byte_count;
    struct ofl_match_header        *match;        /* Fields to match */

};


/* Nicira vendor stats request of type EXT_FLOW (analogous to OFPST_FLOW
 * request). */
struct ofl_ext_flow_stats_request {
    
    struct ofl_ext_msg_header header;
    uint8_t table_id;         /* ID of table to read (from ofp_table_stats),
                                 0xff for all tables. */
    uint8_t pad1;               /* Align to 64 bits. */
    uint16_t out_port;        /* Require matching entries to include this
                                 as an output port.  A value of OFPP_NONE
                                 indicates no restriction. */
    uint32_t out_group;       /* Require matching entries to include this
                                 as an output group.  A value of OFPG_ANY
                                 indicates no restriction. */
    uint64_t cookie;          /* Require matching entries to contain this
                                 cookie value */
    uint64_t cookie_mask;     /* Mask used to restrict the cookie bits that
                                 must match. A value of 0 indicates
                                 no restriction. */
   struct ofl_match_header   *match;        /* Fields to match */

};


/* Body for Nicira vendor stats reply of type EXT_FLOW (analogous to
 * OFPST_FLOW reply). */
struct ofl_ext_flow_stats {

    struct ofl_ext_msg_header header;
    uint16_t length;          /* Length of this entry. */
    uint8_t table_id;         /* ID of table flow came from. */
    uint8_t pad;
    uint32_t duration_sec;    /* Time flow has been alive in seconds. */
    uint32_t duration_nsec;   /* Time flow has been alive in nanoseconds
                                 beyond duration_sec. */
    uint16_t priority;        /* Priority of the entry. Only meaningful
                                 when this is not an exact-match entry. */
    uint16_t idle_timeout;    /* Number of seconds idle before expiration. */
    uint16_t hard_timeout;    /* Number of seconds before expiration. */
    uint64_t cookie;          /* Opaque controller-issued identifier. */
    uint64_t packet_count;    /* Number of packets in flow. */
    struct ofl_match_header        *match;         /* Description of fields. */
    size_t instructions_num;
    struct ofl_instruction_header **instructions; /* Instruction set. */

};


/* Nicira vendor stats request of type extST_AGGREGATE (analogous to
 * OFPST_AGGREGATE request). */
struct ofl_ext_aggregate_stats_request {

    struct ofl_ext_msg_header header;
    uint8_t table_id;         /* ID of table to read (from ofp_table_stats)
                                 0xff for all tables. */
    uint8_t pad;           /* Align to 64 bits. */
    uint16_t out_port;        /* Require matching entries to include this
                                 as an output port.  A value of OFPP_NONE
                                 indicates no restriction. */
    uint32_t out_group;       /* Require matching entries to include this
                                 as an output group.  A value of OFPG_ANY
                                 indicates no restriction. */
    uint64_t cookie;          /* Require matching entries to contain this
                                 cookie value */
    uint64_t cookie_mask;     /* Mask used to restrict the cookie bits that
                                 must match. A value of 0 indicates
                                 no restriction. */
    struct ofl_match_header  *match;         /* Description of fields. */

};

int     
ofl_ext_message_pack(struct ofl_msg_experimenter *msg, uint8_t **buf, size_t *buf_len);

int     
ofl_ext_message_unpack(struct ofp_header *oh, size_t *len, struct ofl_msg_experimenter **msg);

char *
ofl_ext_message_to_string(struct ofl_msg_experimenter *msg);

int
ofl_msg_ext_pack_flow_mod(struct ofl_ext_flow_mod *msg, uint8_t **buf, size_t *buf_len);

int
ofl_msg_ext_pack_flow_removed(struct ofl_ext_flow_removed *msg, uint8_t **buf, size_t *buf_len);

int
ofl_msg_pack_stats_request_flow(struct ofl_ext_flow_stats_request *msg, uint8_t **buf, size_t *buf_len);

int
ofl_msg_pack_stats_reply_flow(struct ofl_ext_flow_stats *msg, uint8_t **buf, size_t *buf_len);

int
ofl_msg_pack_stats_reply_aggregate(struct ofl_msg_stats_reply_aggregate *msg, uint8_t **buf, size_t *buf_len);

ofl_err
ofl_ext_unpack_flow_mod(struct ofp_header *src, size_t *len, struct ofl_msg_experimenter **msg);

ofl_err
ofl_ext_unpack_flow_removed(struct ofp_header *src, size_t *len, struct ofl_msg_experimenter **msg);

int 
ofl_ext_msg_free(struct ofl_msg_experimenter *msg);


#endif
