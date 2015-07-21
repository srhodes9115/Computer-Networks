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
char buffer[MTU];
int receiveNumber = 0;
char Sender[16];
char Receiver[16];

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
struct l2pdu* layer2(struct l3pdu*buf,struct l2pdu*buf2,char pos);
struct l3pdu* layer3(struct l2pdu*lay2,char*buf, char pos);
struct l2pdu* layer4(struct l2pdu*buf,char*str, char pos);
void printStruct3(struct l3pdu* ptr);
void printStruct2(struct l2pdu* ptr);
unsigned int checksum(unsigned char *buf, int count);
void layer2struct(struct l2pdu *pointer);

int main()
{
	char direction;
	printf("Enter i or o for trasmit or receive:\n");
	scanf("%c",&direction);
	if (direction != 'i' && direction != 'o')
	{
		printf("Incorrect input, rerun\n");
	}
	printf("Sender IP Address: ");
	scanf("%s",Sender);
	printf("\n Receiver IP Address: ");
	scanf("%s",Receiver);
	struct l2pdu*p;
	if (direction == 'i')
	{
		p = layer4(p,"sendfile.txt",direction); //i equals the send_file direction(in)
	}
	else if (direction == 'o')
	{
		p = layer4(p, "recvfile.txt",direction); //o equals the receive_file direction(out)
	}	
	return (0);
}

//Calls layer 3, 2,and 1 within layer 4 
struct l2pdu* transmit(char*buf, char pos)
{
	struct l3pdu* strptr3; 
	struct l2pdu* strptr2;
	strptr3 = layer3(strptr2,buf,pos); //empty structures passed in to satisfy parameter requirements
	strptr2= layer2(strptr3,strptr2,pos);
	return strptr2;
}

//Calls layer 1,2,and 3 within layer 4
char* receive(struct l2pdu* lay2,char*buf,char pos)
{
	struct l3pdu* final;
	struct l2pdu* ptr2;
	int i,i2;
	ptr2 = layer2(final,lay2,pos); //empty structures passed in to satisfy parameter requirements
	final = layer3(ptr2, buf,pos);


	for(i = 0; i<num_packets; i++) //put information in final character buffer
	{
		if (i==(num_packets-1))
		{
			for(i2 = 0; i2< datafilesize; i2++)
			{
				buf[60*i+i2] = (*(final+i)).data_seg[i2]; //60*i = current packet+how far along is the index in that packet
			}
		}
		else 
		{
			for(i2 = 0; i2< 60; i2++)
			{
				buf[60*i+i2] = (*(final+i)).data_seg[i2]; //60*i = current packet+how far along is the index in that packet
			}
		}	
	}	
	return buf;
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
			
			//transmit to other users
			if (i != num_packets-1)
			{
				int returnvalue;
				layer2struct(struct_pointer+i);
				returnvalue = layer1(buffer,datanumber,'t',1);
				if (returnvalue == -1)
				{ 
					printf("\n Did not trasmit correctly\n ");
				}
			}
			else
			{
				int returnvalue;
				layer2struct(struct_pointer+i);
				returnvalue = layer1(buffer,countsize,'t',1);
				if (returnvalue == -1)
				{
					printf("\n Did not trasmit correctly\n ");
				}
			}
			printf("\n");
			printStruct2(struct_pointer+i);
		}
		free(buf); //frees up memory for layer 3 dynamic structure 
		return struct_pointer;
	}
	else if (pos == 'o')
	{ 
	//connects to Kulkerni layer.0
	int q =0;
	int fill = 0;
	int s;
	int failure;
	int STOP =2;
	char framepointer[MTU];
	char charstring[3];
	int charactercount;
	int datacount;

	struct l2pdu *receive1;
	receive1 = calloc(34, sizeof(struct l2pdu));

	printf("Received Data Entering Layer2:\n");
	while(STOP!=0)
	{
		failure = layer1(framepointer, MTU, 'r', 0);
		if(failure == -1)
		{
			printf("Error occurred in Reception\n");
		}
		else
		{
			int j;
			fill++;
			receive1[q].header.STX = framepointer[fill]; fill++;
			for(s=0; s<3; s++)
			{
				receive1[q].header.count[s] = framepointer[fill];
				charstring[s] = framepointer[fill];
				fill++;
			}
			
			sscanf(charstring, "%d",&charactercount);
			
			datafilesize = charactercount - 60;			
			receiveNumber = receiveNumber + datafilesize;	  //accounts for the headers and trailers

			receive1[q].header.ACK = framepointer[fill]; fill++;
			receive1[q].header.seq_num = framepointer[fill]; fill++;
			receive1[q].header.Lframe = framepointer[fill]; fill++;			
			for(s=0; s<8; s++)
			{
			receive1[q].header.Resvd[s] = framepointer[fill];
			fill++;
			}
		
			receive1[q].layer3.head.ver = framepointer[fill]; fill++;
			for(s = 0; s<16; s++)
			{
			receive1[q].layer3.head.src_ipaddr[s] = framepointer[fill]; 
			fill++;
			}
			for(s=0; s<16; s++)
			{
			receive1[q].layer3.head.dest_ipaddr[s] = framepointer[fill];
			fill++;
			}
			for(s=0; s<7; s++)
			{
			receive1[q].layer3.head.reserved[s] = framepointer[fill];
			fill++;
			}
			for(s=0; s<datafilesize; s++)
			{
			receive1[q].layer3.data_seg[s] = framepointer[fill];
			fill++;
			}	
			for(s=0; s<4; s++)
			{
			receive1[q].tail.Chksum[s] =  framepointer[fill];
			fill++;
			}
			receive1[q].tail.ETX = framepointer[fill];		
			if(receive1[q].header.Lframe == '1')
			{
				num_packets = q+1;
				STOP=0;
			}		
			printStruct2(receive1+q);
			q++;
			fill=0;
		}
	}
//take the layer 2 structure and set the header/tail fields to null, pass on the data from the input buffer to the current buffer with null fields
		int i,i2;
		struct l2pdu* struct_pointer = calloc(num_packets,sizeof(struct l2pdu)); //pointer to dynamic array of l2pdu structures
		printf("Layer 2: Receive input buffer ");
		for (i = 0; i < num_packets; i++)
		{
			printStruct2(receive1+i);
		}
		printf("\n");
		printf("Layer 2: Receive new buffer: ");
		for(i = 0; i <num_packets; i++)
		{
			struct_pointer[i] = *(receive1+i);
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
				struct_pointer[i].head.src_ipaddr[i2] = Sender[i2];
				struct_pointer[i].head.dest_ipaddr[i2] = Receiver[i2];
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
		char*temp = (char*)malloc(sizeof(char)*2048);
		receive_data = receive(buf,temp,pos); //final character buffer after traveling through all layers
		printf("Layer 4: receiving buffer: \n");
		fp = fopen(str,"w");
		printf("Writing to file: ");
		for(i = 0; i < receiveNumber; i++)
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

void layer2struct(struct l2pdu *pointer)
{
	int t;
	int a =0;
	//buffer[a] = '0';
	a++;
	buffer[a]= (*pointer).header.STX; a++;	
	for(t=0; t<3; t++) 
		{
		buffer[a] = (*pointer).header.count[t]; a++; 
		}
	buffer[a] = (*pointer).header.ACK; a++;
	buffer[a] = (*pointer).header.seq_num; a++;
	buffer[a] = (*pointer).header.Lframe; a++;
	for(t=0; t<8; t++) 
		{
		buffer[a] = (*pointer).header.Resvd[t]; a++;
		}
	for (t=0; t<16; t++)
		{
		buffer[a] = (*pointer).layer3.head.src_ipaddr[t]; a++;
		}
	for (t=0; t<16; t++)
		{
		buffer[a] = (*pointer).layer3.head.dest_ipaddr[t]; a++;
		}
	for (t=0; t<7; t++)
		{
		buffer[a] = (*pointer).layer3.head.reserved[t]; a++;
		}
      	buffer[a] = (*pointer).layer3.head.ver; a++;
	
	for(t=0; t<sizeof((*pointer).layer3.data_seg); t++)
		{
		buffer[a] = (*pointer).layer3.data_seg[t]; a++;
		}

	for(t=0; t<4; t++)
		{
		buffer[a] = (*pointer).tail.Chksum[t]; a++;
		}
	buffer[a] = (*pointer).tail.ETX; a++;

	printStruct2(pointer);
}

