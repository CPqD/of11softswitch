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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>

#include "../lib/byte-order.h"
#include "ofl-exp-match.h"
#include "../oflib/ofl-log.h"
#include "../oflib/ofl-print.h"
#include "nx-match.h"

#define LOG_MODULE ofl_exp
OFL_LOG_INIT(LOG_MODULE)

int
ofl_exp_match_pack(struct ofl_match_header *src, struct ofp_match_header *dst){
    
    if(src->type == EXT_MATCH){   
        
        struct ofl_ext_match *m = (struct ofl_ext_match *) src;
        struct ext_match *dst_match = (struct ext_match *)dst;
        dst_match->header.type = htons(m->header.type);
        dst_match->header.length = htons(m->header.length);
        memset(dst_match->pad, 0x00, 4); 
        memcpy(&dst_match->match_fields , &m->match_fields, sizeof(m->match_fields) + m->match_fields.size);

     } else {
        OFL_LOG_WARN(LOG_MODULE, "Experimenter match is not NXFF_NXM");
        return -1;
    }
    
    return 0;
}

ofl_err
ofl_exp_match_unpack(struct ofp_match_header *src, size_t *len, struct ofl_match_header **dst){

    
    struct ofl_ext_match *m = malloc(*len);    
    struct ext_match *src_match = (struct ext_match*) src;
    
    if (*len < ntohs(src->length)) {
        OFL_LOG_WARN(LOG_MODULE, "Received match has invalid length (set to %u, but only %zu received).", 
ntohs(src->length), *len);
        return ofl_error(OFPET_BAD_MATCH, OFPBMC_BAD_LEN);
    }  
    
    m = (struct ofl_ext_match *) malloc(sizeof(struct ofl_ext_match) + src_match->match_fields.size);
    m->header.type = ntohs(src_match->header.type);
    m->header.length = ntohs(src_match->header.length);
    memcpy(&m->match_fields , &src_match->match_fields, sizeof(src_match->match_fields));
    nx_ntoh(src_match, m,src_match->match_fields.size);
    *len -=  m->header.length;

    *dst = &m->header;
    return 0;
}

int     
ofl_exp_match_free(struct ofl_match_header *m){
    if (m->type == EXT_MATCH) {
            free(m);
    }
    else {
          OFL_LOG_WARN(LOG_MODULE, "Trying to free no extended match structure.");
    } 
    return 0;     
}
    
size_t  
ofl_exp_match_length(struct ofl_match_header *m){

    if(!m)
        return 0;
    else {
        struct ofl_ext_match *match = (struct ofl_ext_match *) m;
        return match->header.length;
    }
}

char *  
ofl_exp_match_to_string(struct ofl_match_header *m){
    char *str;
    size_t str_size;
    FILE *stream = open_memstream(&str, &str_size);
    ofl_exp_match_print(stream, m);
    fclose(stream);
    return str;
}


void
ofl_exp_match_print(FILE *stream, struct ofl_match_header *match){

    switch (match->type) {
        case (EXT_MATCH): {
            int i;
            uint32_t header;
            unsigned length;
            
            struct ofl_ext_match *m = (struct ofl_ext_match *)match;
            void *p = m->match_fields.entries;
            fprintf(stream, "extended_match{");
            for (i = 0; i < m->match_fields.total; i++){
                header = ext_entry_ok(p,m->match_fields.size);
                length = NXM_LENGTH(header);
                switch(header){
                    case (NXM_OF_IN_PORT):{
                        uint32_t *value = p + 4;
                        /*Check for byte order */
                        fprintf(stream, "port=\"");   
                        if(!get_byteorder(*value)){
    
                            ofl_port_print(stream, htonl(*value));
                        }
                        else ofl_port_print(stream, *value);
                        fprintf(stream, "\"");
                        p += length + 4; 
                        break;
                    }
                    case (NXM_OF_ETH_SRC): {
                        
                    
                        break;
                    } 
                    case (NXM_OF_ETH_TYPE): {
                        uint16_t *value = p + 4;
                        if(!get_byteorder(*value)){
                            printf("ENTREI %d\n\n", *value);
                            fprintf(stream, ", dltype=\"0x%"PRIx16"\"", htons(*value));   
                        }
                        else fprintf(stream, ", dltype=\"0x%"PRIx16"\"", *value); 
                        p += length + 4; 
                        break;
                    
                    
                    }
                    
                
                }
            
            }
        }     
    }
}
