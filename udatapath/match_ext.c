/* Copyright (c) 2011, CPqD, Brazil
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

#include <stdbool.h>
#include <string.h>
#include "lib/nx-match.h"
#include "match_ext.h"
#include "oflib-exp/ofl-exp-match.h"

bool
match_ext_strict(struct ofl_ext_match *a, struct ofl_ext_match *b) {

    /* It's terribly inefficient */
    int i,j;
    bool match = true;
    uint8_t * p1 =  a->match_fields.entries;
    uint8_t * p2 =  b->match_fields.entries;
    uint32_t header1, header2;  
    
    for(i = 0; i < a->match_fields.total; i++){
         memcpy(&header1, p1, 4);
         uint8_t found = 0;
         unsigned int len1 = NXM_LENGTH(header1); 
         for(j = 0; j < b->match_fields.total; j++){     
            memcpy(&header2, p2, 4);
            unsigned int len2 = NXM_LENGTH(header2);
            if (header1 == header2){
                found = 1;
                p1 +=4;
                p2 +=4;
                if (!memcmp(p1,p2,len1)){
                     p1 +=len1;
                     p2 =  b->match_fields.entries;
                     match = true;
                     break;
                }
                else {
                    match = false; 
                }
                
            } 
            p2 += len2 + 4;       
         }
         if (!found)
            return false;
        
    }
    return match;
    
}

/* Returns true if the two ethernet addresses match, considering their masks. */
static bool
eth_matches(uint8_t *a, uint8_t *am, uint8_t *b, uint8_t *bm) {
    return (((~am[0] & ~bm[0] & (a[0] ^ b[0])) == 0x00) &&
            ((~am[1] & ~bm[1] & (a[1] ^ b[1])) == 0x00) &&
            ((~am[2] & ~bm[2] & (a[2] ^ b[2])) == 0x00) &&
            ((~am[3] & ~bm[3] & (a[3] ^ b[3])) == 0x00) &&
            ((~am[4] & ~bm[4] & (a[4] ^ b[4])) == 0x00) &&
            ((~am[5] & ~bm[5] & (a[5] ^ b[5])) == 0x00));
}

/* Returns true if the two ethernet addresses match, considering their masks. */
static bool
ipv6_matches(uint8_t *a, uint8_t *am, uint8_t *b, uint8_t *bm) {
    return (((~am[0] & ~bm[0] & (a[0] ^ b[0])) == 0x00) &&
            ((~am[1] & ~bm[1] & (a[1] ^ b[1])) == 0x00) &&
            ((~am[2] & ~bm[2] & (a[2] ^ b[2])) == 0x00) &&
            ((~am[3] & ~bm[3] & (a[3] ^ b[3])) == 0x00) &&
            ((~am[4] & ~bm[4] & (a[4] ^ b[4])) == 0x00) &&
            ((~am[5] & ~bm[5] & (a[5] ^ b[5])) == 0x00) &&
            ((~am[6] & ~bm[6] & (a[6] ^ b[6])) == 0x00));
}

/* Returns true if the given field is set among the wildcards */
static inline bool
wc(uint32_t wildcards, uint32_t field) {
    return ((wildcards & field) != 0);
}

/* Returns true if the given field is set in either wildcard field */
static inline bool
wc2(uint32_t wildcards_a, uint32_t wildcards_b, uint32_t field) {
    return (wc(wildcards_a, field) || wc(wildcards_b, field));
}

bool
match_ext_nonstrict(struct ofl_ext_match *a, struct ofl_ext_match *b) {
    
}
