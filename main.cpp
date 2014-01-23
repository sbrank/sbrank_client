#include <iostream>
#include "libraries/pugixml.hpp"
#include <stdio.h>
#include <string>
#include <vector>
#include <algorithm>
#include <curl/curl.h>

//#include <stdlib.h>
#include <sys/stat.h>
//#include <boost/date_time/posix_time/posix_time.hpp>


#ifdef _WIN32

#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#include "libraries/dirent.h"

#else
//linux libraries cant remember just yet
#include <dirent.h>
#endif

unsigned int getRank(std::string user);
void setInfo();
void checkInfo();
int run();
int sendServerResult(struct GameResult resultStruct);

int writer(char *data, size_t size, size_t nmemb, std::string *buffer);

//would be good to add some more stuff but this is enough for now
struct GameResult
{
	char *user; //this is the person using this application
	char *playerOneName;
	char *playerOneURL;
	int playerOneResult;
	char *playerTwoName;
	char *playerTwoURL;
	int playerTwoResult;
	char *clientVersion;
};

std::string serverIP;
const char *CLIENT_VERSION = "POC 1c-dev"; 
int main()
{

	unsigned int choice;
	bool inMenu = true;

	while(inMenu)
	{
		std::cin.ignore();
		std::cout << "Starbow Ranking System Launcher ("<< CLIENT_VERSION <<")\n\n";
		std::cout << "1. Enter B.net information\n";
		std::cout << "2. Check B.net information\n";
		std::cout << "3. Get ranking from server\n";
		std::cout << "4. Begin ranking!\n";
		std::cout << "5. Quit\n";
		std::cin >> choice;


		switch(choice)
		{
			case 1: setInfo();
				break;
			case 2: checkInfo();
				break;
			case 3: getRank(std::string(""));
				break;
			case 4: run();
				break;
			case 5: inMenu=false;
				break;
			default: 
			{
				std::cin.clear();//std::cout << "Enter a choice between 1 and 5\n";
				std::string dummyLine;
				std::getline(std::cin, dummyLine); ///mmm stackoverflow http://stackoverflow.com/questions/13425093/the-case-switch-statement-in-c
				//nah still doesnt work, seem to always pass junk to the switch, perhaps switch statements are just not the write choice
			}
		}
	}

	return 0;
}

//get the ranking from the server, currently kinda superflous but will just write a stub for now
unsigned int getRank(std::string user)
{
	std::cout << "Currently not implemented\n\n";
	unsigned int ranking=1000;
	return ranking;
}

//writes info to an xml file
void setInfo()
{
	/*
	std::string bnetUrl, sc2profilePath;
	std::cout << "Enter URL to B.net Profile (no error checking!)\n";
	std::getline(std::cin, bnetUrl);
	std::cin.ignore();
	std::cout << "Enter path to local sc2 path between quotes \"path\" (no error checking!)\n";
	//std::cin >> sc2profilePath;
	std::getline(std::cin, sc2profilePath);
	std::cin.ignore();
	//path could be like D:\home\StarCraft II\Accounts\250222938\1-S2-1-2764485\Replays

	std::cout << sc2profilePath.c_str();
	
	/*
	pugi::xml_document configXML;
	configXML.append_child("config");
	pugi::xml_node config = configXML.child("config");
	config.append_attribute("version");
	config.attribute("version").set_value(0.1);
	config.append_attribute("bneturl");
	config.attribute("bneturl").set_value(bnetUrl.c_str());
	config.append_attribute("path");
	config.attribute("path").set_value(sc2profilePath.c_str());
	configXML.save_file("./config.xml");
	std::cout.flush();
	*/
	std::cout << "edit the xml file your self\n";
	std::cin.ignore();
	
}

//read info from xml file
void checkInfo()
{
	pugi::xml_document configXML;
	configXML.load_file("./config.xml");
	pugi::xml_node config = configXML.child("config");
	std::cout << "bneturl: " << config.attribute("bneturl").value() << "\n";
	std::cout << "profilepath: " << config.attribute("path").value() << "\n";

}

//where everything happens when not doing stuff with info
int run()
{
	//wait untill we detect a game has ended and replay saved
	//dump the last replay and find some stuff



	//check bnet profile
	pugi::xml_document configXML;
	configXML.load_file("./config.xml");
	pugi::xml_node config = configXML.child("config");
	std::string bneturl = config.attribute("bneturl").as_string();
	serverIP = config.attribute("serverip").as_string();
	std::string bnetAPIurl;
	
	//well we will just handle only english for now, actually this should work anyway
	//http://us.battle.net/sc2/en/profile/2764485/1/cpc/
	//http://us.battle.net/api/sc2/profile/2764485/1/cpc/matches

	//http://us.battle.net/ + "api/sc2" + /profile/2764485/1/cpc/ + "matches"

	unsigned int urlEnd = bneturl.find("/profile/");
	bnetAPIurl.assign(bneturl.substr(0,21));
	bnetAPIurl.append("api/sc2");
	bnetAPIurl.append(bneturl.substr(urlEnd,200)); //magic 200 here
	bnetAPIurl.append("matches");
	std::cout << bnetAPIurl.c_str() << "\n";
	std::cin.ignore();


	bool waitingForReplay = true;
	bool running = true;

	std::vector<std::string> replayFilenamesNew;
	int replayCountNew = 0;
	int replayCount = 0;
	std::vector<std::string> replayFilenames;


	struct GameResult resultStruct;
	//resultStruct.playerOneResult = 2;
	//sendServerResult(resultStruct);

	while(running)
	{
		
		//check for a replay and stuff (relies on autosave replay because everyone does that right?)
		while(waitingForReplay)
		{
			//get stuff from replay
			DIR *replayDir;
			struct dirent *pDirent;
			int len;

			replayCountNew=0;

			//check if there are replays and write count to file
			if(replayFilenames.size() == 0)
			{			
				replayDir = opendir(config.attribute("path").as_string());
				if (replayDir != NULL)
				{
					while ((pDirent = readdir(replayDir)) != NULL)
					{
						len = strlen (pDirent->d_name);
						if (len >= 4) 
						{
							if (strcmp (".SC2Replay", &(pDirent->d_name[len - 10])) == 0) 
							{
								//printf ("%s\n", pDirent->d_name);
								replayCount++;
								replayFilenames.push_back(pDirent->d_name);
							}
						}	
					}
					closedir (replayDir);
				}
				else
				{
					printf("replay dir is null (firstcheck)");
				}

			}
			else
			{
				
				replayDir = opendir(config.attribute("path").as_string());
				if (replayDir != NULL)
				{
					while ((pDirent = readdir(replayDir)) != NULL)
					{
						len = strlen (pDirent->d_name);
				
						if (len >= 4) 
						{
							if (strcmp (".SC2Replay", &(pDirent->d_name[len - 10])) == 0) 
							{
								//printf ("%s\n", pDirent->d_name);
								++replayCountNew;
								
								//check to make sure it isnt there and then save it
								if(std::find(replayFilenames.begin(), replayFilenames.end(), pDirent->d_name) == replayFilenames.end())
								{
									replayFilenamesNew.push_back(pDirent->d_name);
									replayFilenames.push_back(pDirent->d_name);
									++replayCount;
								}

								
							}
						}
		
					}
					closedir (replayDir);
				}
				else
				{
					printf("replay dir is null");
				}

				
			}

			//printf("replayFilenamesNew size =%i\n",replayFilenamesNew.size()  );
			if(replayFilenamesNew.size() > 0)//replayCountNew > replayCount)
			{
				//replayFilenames = replayFilenamesNew;
				//replayCount = replayCountNew;
				waitingForReplay = false;
				printf("replay=%s\n",replayFilenamesNew.at(0).c_str());
			}
			printf("no of replaysnew=%i\n",replayCountNew);
			printf("no of replays=%i\n",replayCount);
			//printf("last string :%s\n",replayFilenames.at(replayFilenames.size()-1).c_str());
			//reduce cpu usage, every 5 seconds makes much more sense
			printf("sleep 5 seconds\n");
			Sleep(5000);
			
		}

		//use python for stuff
		std::string replayPath;
		replayPath.append(config.attribute("path").as_string());
		replayPath.append("\\");
		replayPath.append(replayFilenamesNew.at(0).c_str()); //they should never have more then 1 new replay but it wont crash if it does
		
		#ifdef _WIN32
			std::string systemString = "extractreplay\\extractreplay.exe \"";
			systemString.append(replayPath);
			systemString.append("\"");
			printf("systemstring is=%s\n",systemString.c_str());
			system(systemString.c_str());
		#else
			std::string systemString = "./exactreplay/exactreplay \"";
			systemString.append(replayPath);
			systemString.append("\"");
			system(systemString.c_str());
		#endif

		//get the file and the info from it
		//it appears the python script always writes to where the replay is opposed to where the application is oh well
		std::string replayFilenameNewOutput = config.attribute("path").as_string();
		replayFilenameNewOutput.append("\\");
		replayFilenameNewOutput.append(replayFilenamesNew.at(0));
		replayFilenameNewOutput.append("_output.txt");
		FILE *outputFile = fopen(replayFilenameNewOutput.c_str(),"r");
		
		printf("replayfilename outputname%s\n",replayFilenameNewOutput.c_str());
		char *outputFileBuffer;
		if(outputFile)
		{
			fseek(outputFile, 0, SEEK_END);
			int fileSize = ftell(outputFile);
			rewind(outputFile);
			outputFileBuffer = (char*) malloc((fileSize+1) * sizeof(char));			
			fread(outputFileBuffer, sizeof(char), fileSize, outputFile);
			fclose(outputFile);
			outputFileBuffer[fileSize] = 0;

			//printf("replaycontents:\n%s\n",outputFileBuffer);
			if( remove( replayFilenameNewOutput.c_str() ) != 0 )
			{
				perror( "Error deleting file" );
			}
			else
			{
				puts( "output file successfully deleted" );
			}

		}
		else
		{
			printf("cant open *_output.txt (did the map only have 1 human player?\n");
		}

		/*
		Player One (name,url,result): cpc,http://us.battle.net/sc2/en/profile/2764485/1/cpc/,Win
		Player Two (name,url,result): Armada,http://us.battle.net/sc2/en/profile/3927986/1/Armada/,Loss
		Replay Info (type,map,endtime,length): 1v1,Polar Night LE;2013-12-17 10:35:36,24.37
		*/
		std::string replayInfo = outputFileBuffer;

		std::string p1Url,p2Url,line1,part2,line2,line3,p1Name, p2Name,p1Result, p2Result;
		//not going to use other info for now its just there in case it might be useful if i start saving match history to db
		line1 = replayInfo.substr(replayInfo.find(": "), replayInfo.find("\n")-replayInfo.find(": "));
		part2 = replayInfo.substr(replayInfo.find("Two "), replayInfo.find("Replay")-replayInfo.find("Two "));
		//printf("\npart2=%s\n",part2.c_str());
		line2 = part2.substr(part2.find(": "), part2.find("\n")-part2.find("t): "));
		line3 = replayInfo.substr(replayInfo.find("th):"), replayInfo.find_last_of("\n") );
		
		//printf("\nline1=%s\n",line1.c_str());
		//printf("line2=%s\n",line2.c_str());
		p1Url = line1.substr(line1.find("http://"),line1.find_last_of("/")+1-line1.find("http://"));
		p2Url = line2.substr(line2.find("http://"),line2.find_last_of("/")+1-line2.find("http://"));
		p1Name = line1.substr(2,line1.find(",")-2);
		p2Name = line2.substr(2,line2.find(",")-2);
		p1Result = line1.substr(line1.find_last_of(",")+1,line1.find_last_of("\n")-line1.find_last_of(","));
		p2Result = line2.substr(line2.find_last_of(",")+1,line2.find_last_of("\n")-line2.find_last_of(",")-1);

		std::cout << "player 1:" << p1Name.c_str() << ";" << p1Url.c_str() << ";" <<p1Result.c_str() << "\n";
		std::cout << "player 2:" << p2Name.c_str() << ";" << p2Url.c_str() << ";" <<p2Result.c_str() << "\n";
		

		
		//i made this way harder then it needed to be im sure
		//1v1,Polar Night LE;2013-12-17 10:35:36,24.37
		//;2013-12-17 10:35:36,24.37
		std::string replayGameType = line3.substr(5,3);//line3.find_first_of(","));
		std::string replayMapName = line3.substr( line3.find_first_of(",")+1, line3.find_first_of(";")-line3.find_first_of(",")-1 );//replayInfo.substr(replayInfo.find_last_of(",")+1,replayInfo.find_last_of(";"));
		printf("replay gametype=%s\n",replayGameType.c_str());
		printf("replaymapname=%s\n",replayMapName.c_str());

		std::string part3 = line3.substr(line3.find(";")+1,line3.find("\n")-line3.find(";")-1);
		std::string replayTimestamp = part3.substr(0, part3.find_last_of(",")-1);

		printf("timestamp=%s\n",replayTimestamp.c_str()); //watchout this uses user time in the server timezone which is super annoying
		printf("length=%s\n", part3.substr(part3.find_last_of(",")+1, part3.find("\n")-1).c_str() );

		if(replayMapName.find("Starbow") != std::string::npos)
		{
			printf("Starbow game detected from replay\n");
		}
		else
		{
			printf("Starbow game not detected from replay!\nPress enter to go back to menu..\n");
			return -1;
		}
		

		//not a new file anymore we used it!
		replayFilenamesNew.clear();


		Sleep(10000); //on rare occasions the profile might not be updated fast enough so this hopefully will fix this, even if it isnt the nicest
		//10seconds so long theres no way it cant be there in that time!
		printf("\nCheck b.net profile for last game\n");
		//get url
		CURL *curl;
		std::string buffer;
		curl = curl_easy_init();
		curl = curl_easy_init();	
		if (curl)
		{
			curl_easy_setopt(curl, CURLOPT_URL, bnetAPIurl.c_str());
			curl_easy_setopt(curl, CURLOPT_HEADER, 0);	/* No we don't need the Header of the web content. Set to 0 and curl ignores the first line */
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 0); /* Don't follow anything else than the particular url requested*/
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writer);	/* Function Pointer "writer" manages the required buffer size */
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &buffer ); /* Data Pointer &buffer stores downloaded web content */	
		}
		else
		{
			fprintf(stderr,"Error 1\n");	/* Badly written error message */
			return 1;	
		}

		/* (D) Fetch the data */
		curl_easy_perform(curl);
		/* res = curl_easy_perform(curl); */
		/* There is no need for "res" as it is just a flag */	
		/* (E) Close the connection */
		curl_easy_cleanup(curl);

		std::string recentResult;
		int endOfGame, startOfGame;
		//std::cout << "buffer contents\n\n" << buffer.c_str();
		endOfGame = buffer.find("}");
		startOfGame = buffer.find("\"map\"");

		recentResult = buffer.substr(startOfGame, endOfGame - startOfGame);
		//std::cout << recentResult.c_str();


		/*
		"map" : "Starbow - Fighting Spirit",
			  "type" : "CUSTOM",
			  "decision" : "WIN",
			  "speed" : "FASTER",
			  "date" : 1389531028

		*/

		bool starBowGame = false;
		std::string mapName, date, decisionSubstring, decision;
		int decisionInt; //-1 loss 0 draw 1 win
		long long epochDate;

		int found = recentResult.find("Starbow");
		if(found != std::string::npos)
		{
			starBowGame = true;
			mapName = recentResult.substr(found, recentResult.find("\",") - found);
		
			//decision" : "WIN",
			decisionSubstring = recentResult.substr(recentResult.find("\"decision"), 24); //a size that is bigger then neccessary but can fit win/loss/other fine
			decision = decisionSubstring.substr(decisionSubstring.find(":") + 3, decisionSubstring.find("\",")-decisionSubstring.find(":") -3 );

			//"date" : 1389531028 //+9
			date = recentResult.substr(recentResult.find("\"date") + 9, 32); //magic but windows nanotime

			if(decision.compare(std::string("WIN")) == 0)
			{
				decisionInt = 1;
			}
			else if(decision.compare(std::string("LOSS")) == 0)
			{
				decisionInt = -1;
			}
			else 
			{
				decisionInt = 0;
			}
			epochDate = atoi(date.c_str());

			std::cout << mapName.c_str() << std::endl;
			std::cout << decision.c_str() << std::endl;
			std::cout << decisionInt << std::endl;
			//std::cout << date.c_str() << std::endl;
			std::cout << epochDate << std::endl;
		
		}
		else
		{
			printf("Most recent game in bnet profile is not a starbow game\nPress enter to go back to menu..\n");
			return -1;
			//testing purposes we will set decisionInt to 1
			//decisionInt = 1;
		}
		std::cout << "player 1:" << p1Name.c_str() << ";" << p1Url.c_str() << ";" <<p1Result.c_str() << "\n";
		std::cout << "player 2:" << p2Name.c_str() << ";" << p2Url.c_str() << ";" <<p2Result.c_str() << "\n";
		
		int p1ResultInt = -99, p2ResultInt = -99;
		
		//printf("p1result=|%s|, p2result=|%s|\n",p1Result.c_str(),p2Result.c_str());

		if (strcmp("Win",p1Result.c_str()) == 0)//p1Result.compare(std::string("Win")) == 0)
		{
			p1ResultInt = 1;
		}
		else if (strcmp("Loss",p1Result.c_str()) == 0)//if(p1Result.compare(std::string("Loss")) == 0)
		{
			p1ResultInt = -1;
		}
		else
		{
			p1ResultInt = 0;
		}

		if (strcmp("Win",p2Result.c_str()) == 0)//if (p2Result.compare(std::string("Win")) == 0)
		{
			p2ResultInt = 1;
		}
		else if (strcmp("Loss",p2Result.c_str()) == 0)//else if(p2Result.compare(std::string("Loss")) == 0)
		{
			p2ResultInt = -1;
		}
		else
		{
			p2ResultInt = 0;
		}

		//the player can be player 1 or 2 so we better check
		if (bneturl.compare(p1Url) == 0)
		{
			if(decisionInt != p1ResultInt)
			{
				printf("Results Disagree! (server=%i, replay=%i)\nPress enter to go back to menu..\n",decisionInt,p2ResultInt);
				return -1;
			}
		}
		else
		{
			if(decisionInt != p2ResultInt)
			{
				printf("Results Disagree! (server=%i, replay=%i)\nPress enter to go back to menu..\n",decisionInt,p2ResultInt);
				return -1;
			}
		}

		//what i was just playing with before commiting
		/*
		boost::posix_time::ptime t(boost::posix_time::time_from_string(replayTimestamp));
		tm pt_tm = to_tm(t);
		time_t foo = mktime(&pt_tm);
		
		printf("bnetepoch to date i think:%s\n", boost::posix_time::time_from_string(epochDate));

		//std::string replayEpochTime = boost::posix_time::to_iso_extended_string(t);
		printf("epoch replay time is %f\n",(double) foo);
		*/

		GameResult resultStruct = {"", (char *)p1Name.c_str(), (char *)p1Url.c_str(), p1ResultInt, (char *)p2Name.c_str(), (char *)p2Url.c_str(), p2ResultInt, (char *)CLIENT_VERSION};


		if(bneturl.compare(p1Url) == 0)
		{
			resultStruct.user = (char *) p1Url.c_str();
		}
		else if(bneturl.compare(p2Url) == 0)
		{
			resultStruct.user = (char *) p2Url.c_str();
		}
		else
		{
			printf("Someone elses replay detected!\nPress enter to go back to menu..\n");
			return -1;
		}


		//fill out the struct
		
		
		/*
		strcpy(resultStruct.playerOneName, p1Name.c_str());
		strcpy(resultStruct.playerOneURL, p1Url.c_str());
		resultStruct.playerOneResult = p1ResultInt;
		strcpy(resultStruct.playerOneName, p1Name.c_str());
		strcpy(resultStruct.playerOneURL, p1Url.c_str());
		resultStruct.playerOneResult = p1ResultInt;;
		*/

		sendServerResult(resultStruct);

		//back to waiting
		waitingForReplay = true;


	}


	//download api url
	//http://www.daniweb.com/software-development/c/threads/269500/libcurl-copy-webpage-contents-to-string
	//std::string buffer;


	return 0;
}


int sendServerResult(struct GameResult resultStruct)
{
	

	#ifdef _WIN32

		WSADATA wsaData;
		int iResult;

		// Initialize Winsock
		iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
		if (iResult != 0) 
		{
			printf("WSAStartup failed: %d\n", iResult);
		}

	#endif

	struct addrinfo *result = NULL, *res = NULL, hints;
	int s;

	memset(&hints, 0, sizeof(struct addrinfo));
	#define DEFAULT_PORT "3310"
	//#define DEFAULT_IP "107.170.15.40" //this is the server on digital ocean

	hints.ai_family = AF_UNSPEC;  // Allow IPv4 or IPv6 
    hints.ai_socktype = SOCK_DGRAM; 
    hints.ai_flags = AI_PASSIVE;
    hints.ai_protocol = 0;         // Any protocol 

	s = getaddrinfo(serverIP.c_str(), DEFAULT_PORT, &hints, &result);
    if (s != 0) 
	{
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        
		#ifdef _WIN32
			WSACleanup();
			return 1;
		#else
			exit(EXIT_FAILURE);
			return 1;
		#endif
    }
	
	#ifdef _WIN32
		SOCKET sockfd = INVALID_SOCKET;
		
	#else	
		int sockfd;
		
	#endif

	res = result;
	sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	#ifdef _WIN32

		if (sockfd == INVALID_SOCKET) 
		{
			printf("Error at socket(): %ld\n", WSAGetLastError());
			freeaddrinfo(result);
			WSACleanup();
			return 1;
		}

	#endif

	//char *msg = "REALLY exciting message!!!!";

	//conect time
	int connectStatus;
	connectStatus = connect(sockfd, res->ai_addr, res->ai_addrlen);





	printf("connect status = %i\n",connectStatus);
	if (connectStatus != 0)
	{
	   printf("connect error  = %s\n",gai_strerror(connectStatus));	
	}
	
	//sending

	//int msgLength = strlen(msg);
	int bytesSent;
	//char *sendData;
	

	//sendData = (char*) malloc(sizeof(resultStruct));
	//memcpy(sendData,&resultStruct,sizeof(resultStruct));
	
	char sendData[4096];//[6][1024]; //4096 is a little large but oh well
	//char *one, *two;
	/*
	sprintf(one,"%s",resultStruct.playerOneResult);
	sprintf(two,"%s",resultStruct.playerTwoResult);

	strcpy(sendData[0],resultStruct.playerOneName);
	strcpy(sendData[1],resultStruct.playerOneURL);
	strcpy(sendData[2],one);
	strcpy(sendData[3],resultStruct.playerTwoName);
	strcpy(sendData[4],resultStruct.playerTwoURL);
	strcpy(sendData[5],two);
	*/

	sprintf(sendData,"%s;%s;%s;%i;%s;%s;%i;\0",resultStruct.user,resultStruct.playerOneName,resultStruct.playerOneURL,resultStruct.playerOneResult
										,resultStruct.playerTwoName,resultStruct.playerTwoURL,resultStruct.playerTwoResult
		);

	//printf("resultstruct size is =%i\n", sizeof(resultStruct));
	//printf("sendData size is=%i\n",sizeof(sendData));
	
	//send(sd, (void*)&a, sizeof(a), 0);

	
	bytesSent = send(sockfd, sendData, sizeof(sendData),0);//msg,msgLength,0);
	printf("bytes sent = %i\n",bytesSent);
	
	//probably should receive a confirmation?
	/*
	char buffer[4096]; //this should be an entire api page, much more then neccessary
	int receivedBytes = recv(sockfd,buffer,4096,0);

	if (receivedBytes < 0)
	{
		printf("error: %s\n",gai_strerror(receivedBytes));
		return 1;
	}
	else if (receivedBytes == 0)
	{
		printf("the server has closed its connection to you\n");
		return 1;
	}
	else
	{ 
		printf("received stuff = %s\n",&buffer);
	}
	*/
	//system("pause");
	
}




int writer(char *data, size_t size, size_t nmemb, std::string *buffer)
{
	//fprintf(stderr,"Hello I am a function pointer\n");
	int result = 0;

	if(buffer != NULL) 
	{
		buffer -> append(data, size * nmemb);
		result = size * nmemb;
	}

	return result;
} 