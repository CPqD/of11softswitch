/*
 * Copyright (c) 2011 CPqD.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <config.h>

#include "nx-match.h"

#include <netinet/icmp6.h>

#include "bj_hash.h"
#include "byte-order.h"
#include "hmap.h"
#include "ofpbuf.h"
#include "packets.h"
#include "unaligned.h"
#include "vlog.h"
#include "ofpbuf.h"
#include "flex-array.h"
#include "openflow/match-ext.h"
#include "oflib-exp/ofl-exp-match.h"

#define LOG_MODULE VLM_nx_match


/* For each TLV_* field, define NFI_TLV_* as consecutive integers starting from
 * zero. */
enum nxm_field_index {
#define DEFINE_FIELD(HEADER, WILDCARD, DL_TYPES, NW_PROTO) \
        NFI_TLV_##HEADER,
#include "nx-match.def"
    N_TLV_FIELDS
};




/* Possible masks for TLV_EXT_DL_DST_W. */
static const uint8_t eth_all_0s[ETH_ADDR_LEN]
    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t eth_all_1s[ETH_ADDR_LEN]
    = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
static const uint8_t eth_mcast_1[ETH_ADDR_LEN]
    = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
static const uint8_t eth_mcast_0[ETH_ADDR_LEN]
    = {0xfe, 0xff, 0xff, 0xff, 0xff, 0xff};


/*static const struct nxm_field *
nxm_field_lookup(uint32_t header)
{
    struct nxm_field *f;


    HMAP_FOR_EACH_WITH_HASH (f, struct nxm_field, hmap_node, hash_int(header, 0),
                             &all_nxm_fields) {
        if (f->header == header) {
            return f;
        }
    }

    return NULL;
}*/


/*static int
parse_nxm_entry(struct flex_array * entry, const struct nxm_field *f,
                const void *value, const void *mask)
{

    switch (f->index) {
        /* Metadata. 
    case NFI_TLV_EXT_IN_PORT:{
        const uint8_t *p = (const uint8_t *) value;    
        f->value = p;
        //ext_put_32(entry, TLV_EXT_IN_PORT, (p[0] << 24) | (p[1] << 16) | (p[2] << 8) | p[3]);
        return 0;
    }

        /* Ethernet header. 
   case NFI_NXM_OF_ETH_DST:
        if ((wc->wildcards & (FWW_DL_DST | FWW_ETH_MCAST))
            != (FWW_DL_DST | FWW_ETH_MCAST)) {
            return NXM_DUP_TYPE;
        } else {
            wc->wildcards &= ~(FWW_DL_DST | FWW_ETH_MCAST);
            memcpy(flow->dl_dst, value, ETH_ADDR_LEN);
            return 0;
        }
    case NFI_NXM_OF_ETH_DST_W:
        if ((wc->wildcards & (FWW_DL_DST | FWW_ETH_MCAST))
            != (FWW_DL_DST | FWW_ETH_MCAST)) {
            return NXM_DUP_TYPE;
        } else if (eth_addr_equals(mask, eth_mcast_1)) {
            wc->wildcards &= ~FWW_ETH_MCAST;
            flow->dl_dst[0] = *(uint8_t *) value & 0x01;
            return 0;
        } else if (eth_addr_equals(mask, eth_mcast_0)) {
            wc->wildcards &= ~FWW_DL_DST;
            memcpy(flow->dl_dst, value, ETH_ADDR_LEN);
            flow->dl_dst[0] &= 0xfe;
            return 0;
        } else if (eth_addr_equals(mask, eth_all_0s)) {
            return 0;
        } else if (eth_addr_equals(mask, eth_all_1s)) {
            wc->wildcards &= ~(FWW_DL_DST | FWW_ETH_MCAST);
            memcpy(flow->dl_dst, value, ETH_ADDR_LEN);
            return 0;
        } else {
            return NXM_BAD_MASK;
        }
    case NFI_TLV_EXT_DL_SRC: {
        const uint8_t *p = (const uint8_t *) value;
        ext_put_eth(entry, TLV_EXT_DL_SRC, p);
        return 0;
    }
    case NFI_TLV_EXT_DL_TYPE: {      
        const uint16_t *p = (const uint16_t *) value;
        ext_put_16(entry, TLV_EXT_DL_TYPE, ntohs(*p));
        return 0;
    }
        /* 802.1Q header. */
   /* case NFI_NXM_OF_VLAN_TCI:
        if (wc->vlan_tci_mask) {
            return NXM_DUP_TYPE;
        } else {
            cls_rule_set_dl_tci(rule, get_unaligned_be16(value));
            return 0;
        }
    case NFI_NXM_OF_VLAN_TCI_W:
        if (wc->vlan_tci_mask) {
            return NXM_DUP_TYPE;
        } else {
            cls_rule_set_dl_tci_masked(rule, get_unaligned_be16(value),
                                       get_unaligned_be16(mask));
            return 0;
        }

        /* IP header. */
 /*   case NFI_NXM_OF_IP_TOS:
        if (*(uint8_t *) value & 0x03) {
            return NXM_BAD_VALUE;
        } else {
            flow->nw_tos = *(uint8_t *) value;
            return 0;
        }
    case NFI_NXM_OF_IP_PROTO:
        flow->nw_proto = *(uint8_t *) value;
        return 0;

        /* IP addresses in IP and ARP headers. */
   /* case NFI_NXM_OF_IP_SRC:
    case NFI_NXM_OF_ARP_SPA:
        if (wc->nw_src_mask) {
            return NXM_DUP_TYPE;
        } else {
            cls_rule_set_nw_src(rule, get_unaligned_be32(value));
            return 0;
        }
    case NFI_NXM_OF_IP_SRC_W:
    case NFI_NXM_OF_ARP_SPA_W:
        if (wc->nw_src_mask) {
            return NXM_DUP_TYPE;
        } else {
            ovs_be32 ip = get_unaligned_be32(value);
            ovs_be32 netmask = get_unaligned_be32(mask);
            if (!cls_rule_set_nw_src_masked(rule, ip, netmask)) {
                return NXM_BAD_MASK;
            }
            return 0;
        }
    case NFI_NXM_OF_IP_DST:
    case NFI_NXM_OF_ARP_TPA:
        if (wc->nw_dst_mask) {
            return NXM_DUP_TYPE;
        } else {
            cls_rule_set_nw_dst(rule, get_unaligned_be32(value));
            return 0;
        }
    case NFI_NXM_OF_IP_DST_W:
    case NFI_NXM_OF_ARP_TPA_W:
        if (wc->nw_dst_mask) {
            return NXM_DUP_TYPE;
        } else {
            ovs_be32 ip = get_unaligned_be32(value);
            ovs_be32 netmask = get_unaligned_be32(mask);
            if (!cls_rule_set_nw_dst_masked(rule, ip, netmask)) {
                return NXM_BAD_MASK;
            }
            return 0;
        }*/

        /* IPv6 addresses. */
    /*case NFI_NXM_NX_IPV6_SRC:
        if (!ipv6_mask_is_any(&wc->ipv6_src_mask)) {
            return NXM_DUP_TYPE;
        } else {
            struct in6_addr ipv6;
            memcpy(&ipv6, value, sizeof ipv6);
            cls_rule_set_ipv6_src(rule, &ipv6);
            return 0;
        }
    case NFI_NXM_NX_IPV6_SRC_W:
        if (!ipv6_mask_is_any(&wc->ipv6_src_mask)) {
            return NXM_DUP_TYPE;
        } else {
            struct in6_addr ipv6, netmask;
            memcpy(&ipv6, value, sizeof ipv6);
            memcpy(&netmask, mask, sizeof netmask);
            if (!cls_rule_set_ipv6_src_masked(rule, &ipv6, &netmask)) {
                return NXM_BAD_MASK;
            }
            return 0;
        }
    case NFI_NXM_NX_IPV6_DST:
        if (!ipv6_mask_is_any(&wc->ipv6_dst_mask)) {
            return NXM_DUP_TYPE;
        } else {
            struct in6_addr ipv6;
            memcpy(&ipv6, value, sizeof ipv6);
            cls_rule_set_ipv6_dst(rule, &ipv6);
            return 0;
        }
    case NFI_NXM_NX_IPV6_DST_W:
        if (!ipv6_mask_is_any(&wc->ipv6_dst_mask)) {
            return NXM_DUP_TYPE;
        } else {
            struct in6_addr ipv6, netmask;
            memcpy(&ipv6, value, sizeof ipv6);
            memcpy(&netmask, mask, sizeof netmask);
            if (!cls_rule_set_ipv6_dst_masked(rule, &ipv6, &netmask)) {
                return NXM_BAD_MASK;
            }
            return 0;
        }

        /* TCP header. */
  /*  case NFI_NXM_OF_TCP_SRC:
        flow->tp_src = get_unaligned_be16(value);
        return 0;
    case NFI_NXM_OF_TCP_DST:
        flow->tp_dst = get_unaligned_be16(value);
        return 0;

        /* UDP header. */
   /* case NFI_NXM_OF_UDP_SRC:
        flow->tp_src = get_unaligned_be16(value);
        return 0;
    case NFI_NXM_OF_UDP_DST:
        flow->tp_dst = get_unaligned_be16(value);
        return 0;

        /* ICMP header. */
   /* case NFI_NXM_OF_ICMP_TYPE:
        flow->tp_src = htons(*(uint8_t *) value);
        return 0;
    case NFI_NXM_OF_ICMP_CODE:
        flow->tp_dst = htons(*(uint8_t *) value);
        return 0;

        /* ICMPv6 header. */
   /* case NFI_NXM_NX_ICMPV6_TYPE:
        flow->tp_src = htons(*(uint8_t *) value);
        return 0;
    case NFI_NXM_NX_ICMPV6_CODE:
        flow->tp_dst = htons(*(uint8_t *) value);
        return 0;

        /* IPv6 Neighbor Discovery. */
   /*  case NFI_NXM_NX_ND_TARGET:
        /* We've already verified that it's an ICMPv6 message. */
    /*     if ((flow->tp_src != htons(ND_NEIGHBOR_SOLICIT)) 
                    && (flow->tp_src != htons(ND_NEIGHBOR_ADVERT))) {
            return NXM_BAD_PREREQ;
        }
        memcpy(&flow->nd_target, value, sizeof flow->nd_target);
        return 0;
    case NFI_NXM_NX_ND_SLL:
        /* We've already verified that it's an ICMPv6 message. */
   /*      if (flow->tp_src != htons(ND_NEIGHBOR_SOLICIT)) {
            return NXM_BAD_PREREQ;
        }
        memcpy(flow->arp_sha, value, ETH_ADDR_LEN);
        return 0;
    case NFI_NXM_NX_ND_TLL:
        /* We've already verified that it's an ICMPv6 message. */
    /*     if (flow->tp_src != htons(ND_NEIGHBOR_ADVERT)) {
            return NXM_BAD_PREREQ;
        }
        memcpy(flow->arp_tha, value, ETH_ADDR_LEN);
        return 0;
    }
    case N_NXM_FIELDS:
        NOT_REACHED();
    
    }
    NOT_REACHED();
}*/

int 
ext_pull_match(struct ofl_ext_match *match_src, struct hmap * match_dst)
{

    uint32_t header, match_len;
    struct nxm_field *f = (struct nxm_field *) malloc(sizeof(struct nxm_field)); 
    
    uint8_t *p =  match_src->match_fields.entries;
    if (!p) {
        return 1;
    }
    match_len = match_src->match_fields.size;
    while ((header = ext_entry_ok(p, match_len)) != 0) {
        unsigned length = NXM_LENGTH(header);
        f->header = header;
        f->value = p + 4;
        if (NXM_HASMASK(header))
            f->mask = p + 4 + length / 2;
        else {
            f->mask = malloc(length);
            memset(f->mask,0xf,length);
            }
        hmap_insert(match_dst, &f->hmap_node,
                        hash_int(f->header, 0)); 
        p += 4 + length;
        match_len -= 4 + length;
    }

    return match_len ? 1 : 0;
}

int
nxm_field_bytes(uint32_t header)
{
    unsigned int length = NXM_LENGTH(header);
    return NXM_HASMASK(header) ? length / 2 : length;
}

/* Returns the width of the data for a field with the given 'header', in
 * bits. */
int
nxm_field_bits(uint32_t header)
{
    return nxm_field_bytes(header) * 8;
}

/* nx_pull_match() and helpers. */


uint32_t
ext_entry_ok(const void *p, unsigned int match_len)
{
    unsigned int payload_len;
    uint32_t header;
    
    if (match_len < 4) {
        if (match_len) {
            VLOG_DBG(LOG_MODULE,"ext_match ends with partial ext_header");
        }
        return 0;
    }

    memcpy(&header, p, 4);
    payload_len = NXM_LENGTH(header);
    if (!payload_len) {
        VLOG_DBG(LOG_MODULE, "ext_entry %08"PRIx32" has invalid payload "
                    "length 0", header);
        return 0;
    }
    if (match_len < payload_len + 4) {
        VLOG_DBG(LOG_MODULE, "%"PRIu32"-byte ext_entry but only "
                    "%u bytes left in nx_match", payload_len + 4, match_len);
        return 0;
    }
    return header;
}

/* ext_put_match() and helpers.
 *
 * 'put' functions whose names end in 'w' add a wildcarded field.
 * 'put' functions whose names end in 'm' add a field that might be wildcarded.
 * Other 'put' functions add exact-match fields.
 */

void
ext_put_header(struct flex_array *f, uint32_t header)
{ 
   flex_array_put(f, &header, sizeof header);

}

void
ext_put_8(struct flex_array *f, uint32_t header, uint8_t value)
{
    ext_put_header(f, header);
    flex_array_put(f, &value, sizeof value);
    f->total++;
}

void
ext_put_16(struct flex_array *f, uint32_t header, uint16_t value)
{
    ext_put_header(f, header);
    flex_array_put(f, &value, sizeof value);
    f->total++;
}

void
ext_put_16w(struct flex_array *f, uint32_t header, uint16_t value, uint16_t mask)
{
    ext_put_header(f, header);
    flex_array_put(f, &value, sizeof value);
    flex_array_put(f, &mask, sizeof mask);
    f->total++;
}

void
ext_put_16m(struct flex_array *f, uint32_t header, uint16_t value, uint16_t mask)
{
    switch (mask) {
    case 0:
        break;

    case CONSTANT_HTONS(UINT16_MAX):
        ext_put_16(f, header, value);
        break;

    default:
        ext_put_16w(f, NXM_MAKE_WILD_HEADER(header), value, mask);
        break;
    }
}

void
ext_put_32(struct flex_array *f, uint32_t header, uint32_t value)
{
    ext_put_header(f, header);
    flex_array_put(f, &value, sizeof value); 
    f->total++;
    
}

void
ext_put_32w(struct flex_array *f, uint32_t header, uint32_t value, uint32_t mask)
{
    ext_put_header(f, header);
    flex_array_put(f, &value, sizeof value);
    flex_array_put(f, &mask, sizeof mask);
}

void
nxm_put_32m(struct flex_array *f, uint32_t header, uint32_t value, uint32_t mask)
{
    switch (mask) {
    case 0:
        break;

    case CONSTANT_HTONL(UINT32_MAX):
        ext_put_32(f, header, value);
        break;

    default:
        ext_put_32w(f, NXM_MAKE_WILD_HEADER(header), value, mask);
        break;
    }
}

void
ext_put_64(struct flex_array *f, uint32_t header, uint64_t value)
{
    ext_put_header(f, header);
    flex_array_put(f, &value, sizeof value);
}

void
ext_put_64w(struct flex_array *f, uint32_t header, uint64_t value, uint64_t mask)
{
    ext_put_header(f, header);
    flex_array_put(f, &value, sizeof value);
    flex_array_put(f, &mask, sizeof mask);
}

void
ext_put_64m(struct flex_array *f, uint32_t header, uint64_t value, uint64_t mask)
{
    switch (mask) {
    case 0:
        break;

    case CONSTANT_HTONLL(UINT64_MAX):
        ext_put_64(f, header, value);
        break;

    default:
        ext_put_64w(f, NXM_MAKE_WILD_HEADER(header), value, mask);
        break;
    }
}

void
ext_put_eth(struct flex_array *f, uint32_t header,
            const uint8_t value[ETH_ADDR_LEN])
{
    ext_put_header(f, header);
    flex_array_put(f, value, ETH_ADDR_LEN);
    f->total++;
}

/*static void
ext_put_eth_dst(struct ofpbuf *b,
                uint32_t wc, const uint8_t value[ETH_ADDR_LEN])
{
    switch (wc & (FWW_DL_DST | FWW_ETH_MCAST)) {
    case FWW_DL_DST | FWW_ETH_MCAST:
        break;
    case FWW_DL_DST:
        nxm_put_header(b, NXM_OF_ETH_DST_W);
        ofpbuf_put(b, value, ETH_ADDR_LEN);
        ofpbuf_put(b, eth_mcast_1, ETH_ADDR_LEN);
        break;
    case FWW_ETH_MCAST:
        nxm_put_header(b, NXM_OF_ETH_DST_W);
        ofpbuf_put(b, value, ETH_ADDR_LEN);
        ofpbuf_put(b, eth_mcast_0, ETH_ADDR_LEN);
        break;
    case 0:
        nxm_put_eth(b, NXM_OF_ETH_DST, value);
        break;
    }
}

/*static void
nxm_put_ipv6(struct ofpbuf *b, uint32_t header,
             const struct in6_addr *value, const struct in6_addr *mask)
{
    if (ipv6_mask_is_any(mask)) {
        return;
    } else if (ipv6_mask_is_exact(mask)) {
        nxm_put_header(b, header);
        ofpbuf_put(b, value, sizeof *value);
    } else {
        nxm_put_header(b, NXM_MAKE_WILD_HEADER(header));
        ofpbuf_put(b, value, sizeof *value);
        ofpbuf_put(b, mask, sizeof *mask);
    }
}*/



