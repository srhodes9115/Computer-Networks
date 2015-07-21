/* This program is Lab 2 of Computer Networks which is meant to implement a 4 layer model for communication between two hosts for a packet size of up to 2048 bytes. This lab uses a CheckSum error check and stuffs/destuffs ASCII characters that appear in the data segment. 
 * main.c
 * Copyright (C) 2015 Shannon Rhodes <>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define MSS 60 //maximum segment size = layer 3 frame max value
#define MTU 120 //maximum transmission unit = layer 2 frame max value
#define MAX 120 //MAX size of data segment for stuffing characters

int datafilesize = 0; //global variable for the number of characters in the input file
int datanumber = 120; //sets the size for data_segment with stuffed characters
int num_packets = 0; //global variable for the number of packets 

//structure declarations
struct l3hdr {  //layer 3 header
	char ver;
	char src_ipaddr[16];
	char dest_ipaddr[16];
	char reserved[7];
};
struct l3pdu { //structure for layer 3
	struct l3hdr head;
	char data_seg[MAX];
};
struct l2hdr { //header for layer 2
	char STX;
	char count[3]; //holds number of bytes in the frame
	char ACK;
	char seq_num; //order of packets
	char Lframe;
	char Resvd[8];
};
struct l2tr { //tail for layer 2
	char Chksum[4];
	char ETX;
};
struct l2pdu { //structure for layer 2
	struct l2tr tail;
	struct l2hdr header;
	struct l3pdu layer3; //entire layer 3 frame
};

//function declarations
struct l2pdu* transmit(char* buf, char pos);
char* receive(struct l2pdu*layer2,char*buf,char pos);
struct l2pdu* layer1(struct l2pdu*buf,char pos);
struct l2pdu* layer2(struct l3pdu*buf,struct l2pdu*buf2,char pos);
struct l3pdu* layer3(struct l2pdu*lay2,char*buf, char pos);
struct l2pdu* layer4(struct l2pdu*buf,char*str, char pos);
void printStruct3(struct l3pdu* ptr);
void printStruct2(struct l2pdu* ptr);
unsigned int checksum(unsigned char *buf, int count);

int main()
{
	struct l2pdu*p;
	p = layer4(p,"sendfile.txt",'i'); //i equals the send_file direction(in)
	layer4(p, "recvfile.txt",'o'); //o equals the receive_file direction(out)
	return (0);
}

//Calls layer 3, 2,and 1 within layer 4 
struct l2pdu* transmit(char*buf, char pos)
{
	struct l3pdu* strptr3; 
	struct l2pdu* strptr2;
	struct l2pdu* strptr1;
	strptr3 = layer3(strptr2,buf,pos); //empty structures passed in to satisfy parameter requirements
	strptr2= layer2(strptr3,strptr2,pos);
	strptr1 = layer1(strptr2,pos);
	return strptr1;
}

//Calls layer 1,2,and 3 within layer 4
char* receive(struct l2pdu* lay2,char*buf,char pos)
{
	struct l3pdu* final;
	struct l2pdu* ptr2;
	int i,i2;
	layer1(lay2,pos);
	ptr2 = layer2(final,lay2,pos); //empty structures passed in to satisfy parameter requirements
	final = layer3(ptr2, buf,pos);
	for(i = 0; i<num_packets; i++) //put information in final character buffer
	{
		for(i2 = 0; i2< datafilesize; i2++)
		{
			buf[60*i+i2] = (*(final+i)).data_seg[i2]; //60*i = current packet+how far along is the index in that packet
		}	
	}	
	return buf;
}

//physical layer(bottom most) but is a dummy layer at this stage, purely prints out the buffer passed in
struct l2pdu* layer1 (struct l2pdu*lay2, char pos)
{
	int i = 0;
	if (pos == 'i')
	{
		printf("\nLayer 1: Transmit input buffer: ");
		for(i = 0; i < num_packets; i++)
		{
			printStruct2(lay2+i);
		}		
		printf("\nLayer 1: Transmit output buffer: ");
		for(i = 0; i < num_packets; i++)
		{		
			printStruct2(lay2+i);
		}		
		printf("\n");
		return lay2;
	}
	else if (pos == 'o')
	{
		printf("\nLayer 1: Receive input buffer: ");
		for(i = 0; i < num_packets; i++)
		{
			printStruct2(lay2+i);
		}
		printf("\nLayer 1: Receive output buffer ");
		for(i = 0; i < num_packets;i++)
		{		
			printStruct2(lay2+i);
		}
		printf("\n");		
		return lay2;
	}
}

struct l2pdu* layer2(struct l3pdu* buf, struct l2pdu*buf2, char pos)
{
	int i, i2; //counter for loops
	if (pos == 'i') //take the data from layer 3 and put it in the layer 2 structure while initailizing layer 2 header and trailer fields
	{
		struct l2pdu* struct_pointer = calloc(num_packets,sizeof(struct l2pdu)); //pointer to dynamic array of l2pdu structures
		printf("Layer 2: Transmit input buffer: ");
		for(i = 0; i <num_packets;i++)
		{
			printStruct3(buf+i);
		}			
		printf("\n");
		printf("Layer 2: Transmit new buffer: ");
		for(i = 0; i < num_packets; i++)
		{
			//Initialize layer 2 header and add layer 3 data to layer 2
			struct_pointer[i].header.STX = '0';
			//initializing count
			int countsize = 0;
			char temp[3];	
			struct_pointer[i].header.ACK = '0';
			struct_pointer[i].header.seq_num = i%10+48; //sequence number, ascii characters start at 48
			for(i2 = 0; i2 < 8; i2++)
			{
				struct_pointer[i].header.Resvd[i2] = '0';
			}	
			struct_pointer[i].layer3 = *(buf+i);
			
			if (i != (num_packets-1)) //if it is not the last packet
			{
					//Check for ASCII characters occuring in the data segment
				for (i2 = 0; i2<sizeof(struct_pointer[i].layer3.data_seg); i2++)
				{	//conditions to recognize STX, ETX, DLE in their decimal representation
					if ((struct_pointer[i].layer3.data_seg[i2] == 2) || (struct_pointer[i].layer3.data_seg[i2] == 3) || (struct_pointer[i].layer3.data_seg[i2] == 16))
					{	
						int decrement;
						decrement = sizeof(struct_pointer[i].layer3.data_seg);			
						//loop to shift all the data up one position in the array to allow space for the stuffed character
						for(; decrement > i2-1; decrement--)
						{
							struct_pointer[i].layer3.data_seg[decrement+1] = struct_pointer[i].layer3.data_seg[decrement];
						}
						struct_pointer[i].layer3.data_seg[i2] = 16; //stuff the DLE character before the ASCII character in the data segment
						i2++;
						datanumber++;
					}
				}
				sprintf(temp,"%d",datanumber); //converts number to character array
				for (i2 = 0; i2 < 3; i2++)
				{
					struct_pointer[i].header.count[i2] = temp[i2];
				}
				struct_pointer[i].header.Lframe = '0'; //LFrame only 1 for the last packet, otherwise always 0 
				
			}
			else { //last packet case
				//Check for ASCII characters occuring in the data segment
				for (i2 = 0; i2<sizeof(struct_pointer[i].layer3.data_seg); i2++)
				{	//conditions to recognize STX, ETX, DLE in their decimal representation
					if ((struct_pointer[i].layer3.data_seg[i2] == 2) || (struct_pointer[i].layer3.data_seg[i2] == 3) || (struct_pointer[i].layer3.data_seg[i2] == 16))
					{	
						int decrement;
						decrement = sizeof(struct_pointer[i].layer3.data_seg);			
						//loop to shift all the data up one position in the array to allow space for the stuffed character
						for(; decrement > i2-1; decrement--)
						{
							struct_pointer[i].layer3.data_seg[decrement+1] = struct_pointer[i].layer3.data_seg[decrement];
						}
						struct_pointer[i].layer3.data_seg[i2] = 16; //stuff the DLE character before the ASCII character in the data segment
						i2++;
						countsize++;
					}
				}
				int remainder = 0;
				remainder = datafilesize%MSS;
				if (remainder == 0)
				{
					countsize = 120+countsize;
				}
				else
				{
					countsize = remainder+60+countsize; //total size of last packet
				}				
				sprintf(temp,"%d",countsize); //converts number to character array
				if (countsize <10) //inserts this case in the right order or bytes
				{
					struct_pointer[i].header.count[0] = '0';
					struct_pointer[i].header.count[1] = '0';
					struct_pointer[i].header.count[2] = temp[0];
				}				
				else if (countsize <100) //inserts this case in the right order or bytes
				{
					struct_pointer[i].header.count[0] = '0';
					struct_pointer[i].header.count[1] = temp[0];
					struct_pointer[i].header.count[2] = temp[1];
				}
				else
				{ 
					for (i2 = 0; i2 < 3; i2++)
					{
						struct_pointer[i].header.count[i2] = temp[i2];
					}
				}
				struct_pointer[i].header.Lframe = '1'; //last packet LFrame is set to 1 
			}	
			struct_pointer[i].tail.ETX = '0';
			datanumber = 120;
			char check[sizeof(struct_pointer[i])-6]; //array to count the total sum of the fields: Count, ACK, Seq#, LFrame, Layer3 hdr, and Data Segment
			unsigned int tempCheck; //used to comvert the unsigned int to an array
			//take all the fields listed above and put them in the array Check
			int i3 = 0;
			for (i2 = 0; i2 < 3; i2++)
			{
				check[i3] = struct_pointer[i].header.count[i2];
				i3++;
			}
			check[i3] = struct_pointer[i].header.ACK; i3++;
			check[i3] = struct_pointer[i].header.seq_num; i3++;
			check[i3] = struct_pointer[i].header.Lframe; i3++;
			for(i2 =0; i2 < 8; i2++)
			{
				check[i3] = struct_pointer[i].header.Resvd[i2]; i3++;
			}
			check[i3] = struct_pointer[i].layer3.head.ver; i3++;
			for (i2 = 0; i2 < 16; i2++)
			{
				check[i3] = struct_pointer[i].layer3.head.src_ipaddr[i2]; i3++;
			}
			for (i2 = 0; i2 < 16; i2++)
			{
				check[i3] = struct_pointer[i].layer3.head.dest_ipaddr[i2]; i3++;
			}
			for (i2 = 0; i2 < 7; i2++)
			{
				check[i3] = struct_pointer[i].layer3.head.reserved[i2]; i3++;
			}
			for (i2 = 0; i2 < datanumber; i2++)
			{
				check[i3] = struct_pointer[i].layer3.data_seg[i2]; i3++;
			}
			tempCheck = checksum(check,sizeof(struct_pointer[i].layer3.data_seg)+54);
			//shift data to move from 5 digits to 4 digit CheckSum field in layer
			struct_pointer[i].tail.Chksum[0] = (char)((int)'0')+((tempCheck/1000)%10);
			struct_pointer[i].tail.Chksum[1] = (char)((int)'0')+((tempCheck/100)%10);
			struct_pointer[i].tail.Chksum[2] = (char)((int)'0')+((tempCheck/10)%10);
			struct_pointer[i].tail.Chksum[3] = (char)((int)'0')+((tempCheck)%10);
			printf("\n");
			printStruct2(struct_pointer+i);
		}
		free(buf); //frees up memory for layer 3 dynamic structure 
		return struct_pointer;
	}
	else if (pos == 'o')
	{ //take the layer 2 structure and set the header/tail fields to null, pass on the data from the input buffer to the current buffer with null fields
		int i,i2;
		struct l2pdu* struct_pointer = calloc(num_packets,sizeof(struct l2pdu)); //pointer to dynamic array of l2pdu structures
		printf("Layer 2: Receive input buffer ");
		for (i = 0; i < num_packets; i++)
		{
			printStruct2(buf2+i);
		}
		printf("\n");
		printf("Layer 2: Receive new buffer: ");
		for(i = 0; i <num_packets; i++)
		{
			struct_pointer[i] = *(buf2+i);
			char check[sizeof(struct_pointer[i])-6]; //array to store values in fields used in the checkSUM algorithm 
			unsigned int tempCheck; //used to convert result of CheckSum function to an array
			//calculate count for all relevant fields
			int i3 =0;
			for (i2 = 0; i2 <3; i2++)
			{
				check[i3] = struct_pointer[i].header.count[i2]; i3++;
			}
			check[i3] = struct_pointer[i].header.ACK; i3++;
			check[i3] = struct_pointer[i].header.seq_num; i3++;
			check[i3] = struct_pointer[i].header.Lframe; i3++;
			for (i2 = 0; i2 < 8; i2++)
			{
				check[i3] = struct_pointer[i].header.Resvd[i2]; i3++;
			}
			check[i3] = struct_pointer[i].layer3.head.ver; i3++;
			for (i2 = 0; i2 < 16; i2++)
			{
				check[i3] = struct_pointer[i].layer3.head.src_ipaddr[i2]; i3++;
			}
			for (i2 = 0; i2 < 16; i2++)
			{
				check[i3] = struct_pointer[i].layer3.head.dest_ipaddr[i2]; i3++;
			}
			for (i2 = 0; i2 < 7; i2++)
			{
				check[i3] = struct_pointer[i].layer3.head.reserved[i2]; i3++;
			}
			for (i2 = 0; i2 < datanumber; i2++)
			{
				check[i3] = struct_pointer[i].layer3.data_seg[i2]; i3++;
			}
			int x;
			tempCheck = checksum(check,sizeof(struct_pointer[i].layer3.data_seg)+54);
			//test that the temp array computed CheckSum algorithm matches the values stored in the CheckSum field for each frame
			if (
			(((char)((int)'0')+((tempCheck/1000)%10)) == struct_pointer[i].tail.Chksum[0]) 
			&& (((char)((int)'0')+((tempCheck/100)%10) == struct_pointer[i].tail.Chksum[1]) 
			&& (((char)((int)'0')+((tempCheck/10)%10) == struct_pointer[i].tail.Chksum[2]) 
			&& (((char)((int)'0')+((tempCheck)%10) == struct_pointer[i].tail.Chksum[3])))))
			{
				printf("\n Checksum successful \n");
			}
			else
			{
				printf("\n Error in checksum \n");
			}
			//removes the stuffed characters in the data segment
			for (i2 = 0; i2<sizeof(struct_pointer[i].layer3.data_seg); i2++)
			{
				//check for DLE and then ASCII character following it 
				if (struct_pointer[i].layer3.data_seg[i2] == 16)
				{
					i2++; 
					if ((struct_pointer[i].layer3.data_seg[i2] == 2) || (struct_pointer[i].layer3.data_seg[i2] == 3) || (struct_pointer[i].layer3.data_seg[i2] == 16))
					{	
						int i3;
						//shift data down one index following a stuffed character
						for(i3 = i2-1; i3 < sizeof(struct_pointer[i].layer3.data_seg); i3++)
						{
							struct_pointer[i].layer3.data_seg[i3] = struct_pointer[i].layer3.data_seg[i3+1];
						}
					}
					i2--;
				}
			}
			//Take layer 2 header and tail fields and set them to NULL
			struct_pointer[i].header.STX = 0; 
			struct_pointer[i].header.ACK = 0;
			struct_pointer[i].header.seq_num = 0;
			struct_pointer[i].header.Lframe = 0;
			for (i2 = 0; i2 < 3; i2++)
			{
				struct_pointer[i].header.count[i2] = 0;
			}
			for(i2= 0; i2 < 8; i2++)
			{
				struct_pointer[i].header.Resvd[i2] = 0;
			}
			for(i2=0;i2 < 4; i2++)
			{
				struct_pointer[i].tail.Chksum[i2] = 0;
			}
			struct_pointer[i].tail.ETX = 0;
			printStruct2(struct_pointer+i);
		}
		printf("\n");
		free(buf2);
		return struct_pointer;
	}
}

struct l3pdu* layer3(struct l2pdu*lay2,char*buf, char pos)
{
	int i = 0;
	int i2 = 0;
	if (pos == 'i')
	{ //take the string buffer and put its information into the layer3 structure, initialize header fields to 0
		printf("Layer 3: Transmit receiving buffer = %s \n",buf);
		num_packets = (datafilesize+(MSS-1))/MSS; //finds the total number of packets and ensures that it is rounded up
		struct l3pdu *struct_pointer =calloc(num_packets,sizeof(struct l3pdu)); //pointer to dynamic array of layer 3 structures
		//Initialize all headers to 0, put data buffer into Layer 3 Structure
		printf("Layer 3: Transmit new buffer: ");
		for(i = 0; i < num_packets; i++)
		{
			struct_pointer[i].head.ver = '0';
			for (i2 = 0; i2 <16; i2++)
			{
				struct_pointer[i].head.src_ipaddr[i2] = '0';
				struct_pointer[i].head.dest_ipaddr[i2] = '0';
			}
			for (i2 = 0; i2 < 7; i2++)
			{
				struct_pointer[i].head.reserved[i2] = '0';
			}
			for (i2 = 0; i2 < MSS; i2++)
			{
				struct_pointer[i].data_seg[i2] = buf[i2+(MSS*i)]; //shifts data you are filling in by MSS
			}
			printStruct3(struct_pointer+i);
		}
			printf("\n");
			free(buf);
			return struct_pointer;
	}
	else if (pos == 'o') 
	{ //takes layer 2 structure and puts its data into a layer 3 structure setting its header fields to null
		printf("Layer 3: Receive receiving buffer: ");
		for(i = 0; i < num_packets; i++)
		{
			printStruct2(lay2+i);
		}	
		printf("\n");
		struct l3pdu *struct_pointer =calloc(num_packets,sizeof(struct l3pdu)); 
		printf("Layer 3: Receive new buffer: ");
		for(i = 0; i <num_packets; i++)
		{
			struct_pointer[i] = (*(lay2+i)).layer3;
			struct_pointer[i].head.ver = 0;
			for (i2 = 0; i2 <16; i2++)
			{
				struct_pointer[i].head.src_ipaddr[i2] = 0;
				struct_pointer[i].head.dest_ipaddr[i2] = 0;
			}
			for (i2 = 0; i2 < 7; i2++)
			{
				struct_pointer[i].head.reserved[i2] = 0;
			}
			printStruct3(struct_pointer+i);
		}
		printf("\n");
		free(lay2);
		return struct_pointer;				
	}
}

//application (top most) layer that interacts with user
struct l2pdu* layer4(struct l2pdu*buf, char*str, char pos)
{ 
	if (pos == 'i')
	{
		FILE *fp;
		char *send_data = (char *) malloc (sizeof(char) * 2048); //2048 buffer
		char c;
		int i = 0;	
		fp = fopen(str,"r"); //read only capabilities
	
		printf("Layer 4: Transmit data from file name = %s  \n",str);
		while((c=fgetc(fp)) != EOF)  //read in data from file character by character
		{	
			send_data[i] = c;
			datafilesize++; //increment global variable that holds the number of characters
			i++;	
		}
		printf("Layer 4: Transmit new data buffer= %s \n", send_data);
		fclose(fp);
		return transmit(send_data,'i');
	}
	else if (pos == 'o')
	{
		FILE *fp;
		int i;
		char c;
		printf("Layer 4: begin receive\n");
		char*receive_data = (char*)malloc(sizeof(char)*2048);
		receive_data = receive(buf,receive_data,pos); //final character buffer after traveling through all layers
		printf("Layer 4: receiving buffer: \n");
		fp = fopen(str,"w");
		printf("Writing to file: ");
		for(i = 0; i < datafilesize; i++)
		{
			c = receive_data[i]; 
			printf("%c",c);
			fputc(c,fp); //write the buffer to the output file
		}
		printf("\n");
		fclose(fp);
	}
}

//function to print layer 3 structure 
void printStruct3(struct l3pdu* ptr)
{	
	int i;
	printf("%c",(*ptr).head.ver);
	for (i = 0; i < 16; i++)
	{
		printf("%c",(*ptr).head.src_ipaddr[i]);
	}
	for (i = 0; i <16;i++)
	{
		printf("%c",(*ptr).head.dest_ipaddr[i]);
	}
	for(i= 0; i < 7; i++)
	{
		printf("%c",(*ptr).head.reserved[i]);
	}
	for(i = 0; i < sizeof((*ptr).data_seg); i++)
	{
		printf("%c",(*ptr).data_seg[i]);
	}
}

//function to print layer 2 structure
void printStruct2(struct l2pdu* ptr)
{
	int i;
	printf("%c",(*ptr).header.STX);
	for (i = 0; i < 3; i++)
	{
		printf("%c",(*ptr).header.count[i]);
	}
	printf("%c%c%c",(*ptr).header.ACK,(*ptr).header.seq_num,(*ptr).header.Lframe);
	for(i = 0; i < 8; i++)
	{
		printf("%c",(*ptr).header.Resvd[i]);
	}
	printStruct3(&(*ptr).layer3); 
	for(i = 0;i < 4; i++)
	{
		printf("%c",(*ptr).tail.Chksum[i]);
	}
	printf("%c",(*ptr).tail.ETX);
}

//internet check sum algorithm function
unsigned int checksum (unsigned char *buf, int count)
{
	unsigned int sum = 0;
	while (count--)
	{
		sum += *buf++;
		if (sum & 0xFFFFFFFF00000000)
		{
			/* carry occured so wrap around */
			sum &= 0xFFFFFFFF;
			sum++;
		}
	}
	return ~(sum& 0xFFFFFFFF);
}
