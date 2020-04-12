/*
 * main.cpp
 *
 *  Created on: 27 Mar 2020
 *      Author: jinxluck
 *
 *  Client for server, only taking JSON formatted messages,
 *  and only certain commands!
 */
//std libraries
#include <stdio.h>
#include <iostream>

//socket libraries
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

//json libraries
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"

//lazy declarations
using namespace rapidjson;
using namespace std;

//server port and ip adress
#define PORT 1955
char ip[20] = "192.168.6.2";

/*
 * buffer flushing function:
 * used to flush the buffer for old inputs
 */
void flush(void)
{
	while((getchar())!='\n');
}

int main(int argc, char const *argv[])
{
/*-------------- socket creation and checks -------------------------------------*/
    int sock = 0, valread;
    struct sockaddr_in serv_addr;

    //create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        printf("\n Socket creation error \n");
        return -1;
    }

    //set adress type and port
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IPv4 and IPv6 addresses from text to binary form
    //set ip adress
    if(inet_pton(AF_INET, ip, &serv_addr.sin_addr)<=0)
    {
        printf("\nInvalid address/ Address not supported \n");
        return -1;
    }

    //connect to server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("\nConnection Failed \n");
        return -1;
    }

    //success message
    printf("Connected to %s on port: %d\n\n",ip, PORT);

/*-------------------- json handling ------------------------------------------*/
    //create json document, for json handling

    /*json structure:
     * {
     * "command": "string",
     * "message": "string"
     * }
     */
    const char * json_out =
    		"{"
    		"\"command\":\"none\","
    		"\"temp\":0,"
    		"\"message\":\"none\""
    		"}";

    //create JSON document for in and out messages's
    Document d_out, d_in;

    //insert JSON structure for outbound message
    d_out.Parse(json_out);

/*-------------------- message handling ---------------------------------------*/
    //message handling variables
    int size = 1024;
    char buffer[1024] = {0};
    char msg[1024] = {0};
    char switch_con;

    //eternal while loop for client handling
    while(1)
    {
    	//clear buffers
    	memset(buffer, '\0', strlen(buffer));
    	memset(msg, '\0', strlen(msg));

    	//menu message
    	printf("Welcome to the message menu:\n"
    			"1) Set a command\n\n");

    	//get/update switch statement variable
    	switch_con = getchar();
    	flush(); //flush the buffer for good measurement

    	//switch used for easy implementation af new options
    	switch(switch_con)
    	{
    	case '1':
    		//command options
    		printf("Commands available:\n"
    				"GETTEMP\n"
    				"QUIT\n"
    				"Command: ");

    		//get option chosen
    		gets(buffer);

    		//insert command into JSON doc
    		Value& s = d_out["command"];
    		s.SetString(buffer, strlen(buffer), d_out.GetAllocator());

    		//clear buffer for good measurement
    		memset(buffer, '\0', size);

    		break;
    	}

    	//Convert json doc to Json string
		StringBuffer jsonBuffer;
		Writer<StringBuffer> writer(jsonBuffer);
		d_out.Accept(writer);

    	//Send message and wait for return message
    	send(sock , jsonBuffer.GetString() , 1000 , 0);

/*-------------------- message recieve wait handling ---------------------------------------*/
    	printf("Waiting for response\n\n");

    	//time out loop (in case server is inresponsive)
    	int looper = 0;
    	for(;(valread = read( sock , buffer, 1024)) == 1;looper++)
    	{
    		//value should probably be higher in reality
    		if(looper==1000)
    		{
    			printf("\n---Time out error---\n\n");
    			exit(1);
    		}
    	}

/*-------------------- message received handling ---------------------------------------*/
    	//insert message into JSON doc
    	d_in.Parse(buffer);

/*------------------------------ JSON structure check ----------------------------------*/
    	bool check1 = 0;
    	bool check2 = 0;
    	bool check3 = 0;
    	if(!(check1 = d_in.HasMember("command")))
    		printf("message error: structure missing \"command\"\n");
    	if(!(check2 = d_in.HasMember("temp")))
    		printf("message error: structure missing \"temp\"\n");
    	if(!(check3 = d_in.HasMember("message")))
    		printf("message error: structure missing \"message\"\n");
    	if(check1 && check2 && check3) //if structure is correct
    	{
    		//get command
    		strncpy(msg, d_in["command"].GetString(), strlen(d_in["command"].GetString()));

/*------------------------------ if QUIT has been given ---------------------------------*/
    		if(strncmp(msg, "QUIT", strlen("QUIT")) == 0)
    		{
    			//print return messages
    			printf("command: %s\n"
    					"return message: %s \n\n"
    					, d_in["command"].GetString(), d_in["message"].GetString());

    			//exit program
    			return 0;
    		}
/*---------------------- if ERROR has occured message wise on the server part ----------*/
    		else if(strncmp(msg, "ERROR", strlen("ERROR")) == 0)
    		{
    			//print return messages
      			printf("command: %s\n"
        				"return message: %s \n\n"
        				, d_in["command"].GetString(), d_in["message"].GetString());

      			//exit with fail
        			exit(1);
    		}
    		else
/*------------------------------ if GETTEMP command has been given ---------------------------------*/
    		{
    			//print whole JSON formatted package
    			printf("Return package:\n"
    					"command: %s\n"
    					"temp: %d\n"
    					"return message: %s\n\n"
    					, d_in["command"].GetString(), d_in["temp"].GetInt(), d_in["message"].GetString());
    		}

    	}
    }

    //will never be reached...
    return 0;
}


