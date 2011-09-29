/*
 * nbee_link.cpp
 *
 *  Created on: Jul 18, 2011
 *      Author: rdenicol
 */


#include <string.h>
#include <nbee/nbee.h>
#include <netinet/in.h>

#include "nbee_link.h"
#include "../lib/bj_hash.h"
#include "../include/openflow/match-ext.h"


nbPacketDecoder *Decoder;
nbPacketDecoderVars* PacketDecoderVars;
nbNetPDLLinkLayer_t LinkLayerType;
nbPDMLReader *PDMLReader;
int PacketCounter= 1;
struct pcap_pkthdr * pkhdr;

static struct hmap all_packet_fields = HMAP_INITIALIZER(&all_packet_fields);

extern "C" int nbee_link_initialize()
{

	char ErrBuf[ERRBUF_SIZE + 1];
	int NetPDLProtoDBFlags = nbPROTODB_FULL;
	int NetPDLDecoderFlags = nbDECODER_GENERATEPDML_COMPLETE;
	int ShowNetworkNames = 0;

	char* NetPDLFileName = "customnetpdl.xml";

	pkhdr = new struct pcap_pkthdr;

	if (nbIsInitialized() == nbFAILURE)
	{
		if (nbInitialize(NetPDLFileName, NetPDLProtoDBFlags, ErrBuf, sizeof(ErrBuf)) == nbFAILURE)
		{
			printf("Error initializing the NetBee Library; %s\n", ErrBuf);
			return nbFAILURE;
		}
	}

	Decoder= nbAllocatePacketDecoder(NetPDLDecoderFlags, ErrBuf, sizeof(ErrBuf));
	if (Decoder == NULL)
	{
		printf("Error creating the NetPDLParser: %s.\n", ErrBuf);
		return nbFAILURE;
	}

	// Get the PacketDecoderVars; let's do the check, although it is not really needed
	if ((PacketDecoderVars= Decoder->GetPacketDecoderVars()) == NULL)
	{
		printf("Error: cannot get an instance of the nbPacketDecoderVars class.\n");
		return nbFAILURE;
	}
	// Set the appropriate NetPDL configuration variables
//	PacketDecoderVars->SetVariableNumber((char*) NETPDL_VARIABLE_SHOWNETWORKNAMES, ShowNetworkNames);

	if (PacketDecoderVars->SetVariableNumber((char*) NETPDL_VARIABLE_SHOWNETWORKNAMES, ShowNetworkNames)==nbFAILURE)
	{
		printf("Error: cannot set variables of the decoder properly.\n");
		return nbFAILURE;
	}

	PDMLReader = Decoder->GetPDMLReader();

	return 0;

}

extern "C" int nbee_link_convertpkt(struct ofpbuf * pktin, struct hmap * pktout)
{
	//pkhdr->ts.tv_sec = 0;
	pkhdr->caplen = pktin->size; //need this information
	pkhdr->len = pktin->size; //need this information
	printf("\nPacket size: %d \n",pktin->size);

//	memset(curr_packet, 0x00,sizeof())
	_nbPDMLPacket * curr_packet;

	if (pktin->size == 0)
		return 0;

	// Decode packet
	if (Decoder->DecodePacket(LinkLayerType, PacketCounter, pkhdr, (const unsigned char*) (pktin->data)) == nbFAILURE)
	{
		printf("\nError decoding a packet %s\n\n", Decoder->GetLastError());
		// Let's break and save what we've done so far
		return -1;
	}
	PacketCounter++;

	PDMLReader->GetCurrentPacket(&curr_packet);

	_nbPDMLProto * proto;
	_nbPDMLField * field;

	proto = curr_packet->FirstProto;

	while (1)
        {
//		uint8_t i;
 //       	for (i=0;i<3;i++)
//		{
//			printf("%c",proto->Name[i]);
//		}
//		printf("\n");
        	field = proto->FirstField;
              	while(1)
               	{

			printf("\nfield position %ld,  %s :",field->Position,*field);
			
			if((char)field->LongName[0]<58 && (char)field->LongName[0]>47)
                        {
	                        int i,pow;
                                uint32_t type;
                                uint8_t size;
				packet_fields_t * pktout_field;
		                pktout_field = (packet_fields_t*) malloc(sizeof(packet_fields_t));
				
                                field_values_t *new_field;
                                new_field = (field_values_t *)malloc(sizeof(field_values_t));

                                for (type=0,i=0,pow=100;i<3;i++,pow = (pow==1 ? pow : pow/10))
        	                        type = type + (pow*(field->LongName[i]-48));
		                        
				size = field->Size;

                                pktout_field->header = NXM_HEADER(VENDOR_FROM_TYPE(type),FIELD_FROM_TYPE(type),size); 
                                printf("\n Header ID: %d",pktout_field->header);
                                new_field->value = (uint8_t*) malloc(field->Size);
                                memcpy(new_field->value,((uint8_t*)pktin->data + field->Position),field->Size);

				printf("\n\nField %s value: ",field->LongName);

				for(i=0;i<field->Size;i++)
                        	{
                                	printf("%02hx",new_field->value[i]);
                        	}

				printf("\n");
				
				packet_fields_t *iter;
				bool done=0;
				HMAP_FOR_EACH(iter,packet_fields_t, hmap_node,pktout)
				{
					//printf("\nHeader: %d",iter->header);
					if(iter->header == pktout_field->header)
					{
						printf("\n Adding entry to existing Hash Map");
						list_t_push_back(&iter->fields,&new_field->list_node);
						done=1;
						break;
					}
				}

				if (!done)
				{
					list_t_init(&pktout_field->fields);
					printf("\nNew Hash Map");
                                	list_t_push_back(&pktout_field->fields,&new_field->list_node);
                                	hmap_insert(pktout, &pktout_field->hmap_node,
	                        	hash_int(pktout_field->header, 0));
				}
				done =0;

			}

			if(field->NextField == NULL && field->ParentField == NULL)
			{
				printf("\nbreaking");
				break;
			}
			else if (field->NextField == NULL && field->ParentField != NULL)
			{
				field = field->ParentField;
				printf("\nParent");
			}
			else if (!field->NextField->isField)
			{
				printf("\nblock : %s",*field->NextField);
				field = field->NextField->FirstChild;
			}
			else
			{
				printf("\n next field: %s ",*field->NextField);
				field = field->NextField;
			}

		}

		printf("\n");
		if (proto->NextProto == NULL)
		{
			break;
		}
		proto = proto->NextProto;
	}

	printf("Packet %ld done\n",curr_packet->Number);
    return 1;
}

int main (int *argc, char **argv){




}
