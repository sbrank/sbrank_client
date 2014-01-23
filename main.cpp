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
#include "libraries/dirent.h" //not included in visual studio

#else
//posix libraries, haven't looked up what the ohter ones are
#include <dirent.h>

#endif

unsigned int getRank(std::string user);
void setInfo();
void checkInfo();
int run();
int sendServerResult(struct GameResult resultStruct);
int writer(char *data, size_t size, size_t nmemb, std::string *buffer);

//struct containing all the information we want to send to the server
//stuff from bnet account should be added sometime (this will have to be verified server side as well)
struct GameResult
{
	char *user; //this is bneturl of the person using this application
	char *playerOneName; //bnet id
	char *playerOneURL; //full url to bnet profile must end in a '/'
	int playerOneResult; //game result -1 = loss, 0 = draw, 1 = win
	char *playerTwoName;
	char *playerTwoURL;
	int playerTwoResult;
	char *clientVersion; //version of the client to send to server
};

std::string serverIP;
const char *CLIENT_VERSION = "POC 1c-dev"; //could considered it as 0.03 in a way but not really important

int main()
{
	unsigned int choice;
	bool inMenu = true;

	std::cout << "Press Enter to begin.\n";
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
				//this was all here in the hope of fixing the fact that setinfo() wasnt working properly when i entered in a path
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

//inteded for writing info to an xml file
//this didn't work and was super clunky in a commandline interface anyway
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
	std::cout << "Edit confi.xml with a text editor\n";
}

//read info from xml file to allow the user to check on information
void checkInfo()
{
	pugi::xml_document configXML;
	configXML.load_file("./config.xml");
	pugi::xml_node config = configXML.child("config");
	std::cout << "bneturl: " << config.attribute("bneturl").value() << "\n";
	std::cout << "profilepath: " << config.attribute("path").value() << "\n";
	std::cout << "serverip: " << config.attribute("serverip").value() << "\n";

}

//where everything happens when not doing stuff with info
int run()
{

	//check bnet profile using libcurl 
	pugi::xml_document configXML;
	configXML.load_file("./config.xml");
	pugi::xml_node config = configXML.child("config");
	std::string bneturl = config.attribute("bneturl").as_string();
	serverIP = config.attribute("serverip").as_string();
	std::string bnetAPIurl;
	
	//this has only been
	//http://us.battle.net/sc2/en/profile/1111111/1/userid/
	//http://us.battle.net/api/sc2/profile/1111111/1/userid/matches

	//http://us.battle.net/ + "api/sc2" + /profile/1111111/1/userid/ + "matches"

	unsigned int urlEnd = bneturl.find("/profile/");
	bnetAPIurl.assign(bneturl.substr(0,21));
	bnetAPIurl.append("api/sc2");
	bnetAPIurl.append(bneturl.substr(urlEnd,200)); //magic 200 here
	bnetAPIurl.append("matches");
	std::cout << bnetAPIurl.c_str() << "\n";
	std::cin.ignore();

	bool waitingForReplay = true;
	std::vector<std::string> replayFilenamesNew;	
	std::vector<std::string> replayFilenames;
	struct GameResult resultStruct;	
	bool running = true;

	//i suspect these are fairly useless
	int replayCountNew = 0;
	int replayCount = 0;

	//waiting for new replay loop
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
								replayCount++;
								replayFilenames.push_back(pDirent->d_name); //add all the replaynames to a vector so we can know when new replays are created
							}
						}	
					}

					//finished with the replay directory so we can close it.
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
							if (strcmp (".SC2Replay", &(pDirent->d_name[len - 10])) == 0) //-10 is the length of .SC2Replay
							{
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

					//finished with the replay directory so we can close it.
					closedir (replayDir);
				}
				else
				{
					printf("replay dir is null");
				}

				
			}

			//found a new replay
			if(replayFilenamesNew.size() > 0)
			{
				waitingForReplay = false;
				printf("replay=%s\n",replayFilenamesNew.at(0).c_str());
			}

			//doesn't really have a purpose
			printf("no of replaysnew=%i\n",replayCountNew);
			printf("no of replays=%i\n",replayCount);

			//reduces cpu usage, not sure how consistant Sleep() is, every 5 seconds sounds often enough though
			printf("sleep 5 seconds\n");
			Sleep(5000);
			
		}

		//using extractreplay
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

			if( remove( replayFilenameNewOutput.c_str() ) != 0 )
			{
				perror( "Error when attempting to delete extractreplay output file" );
			}
			else
			{
				puts( "extractreplay output file successfully deleted" );
			}
		}
		else
		{
			//errors with replayextract should be handled more cleanly, someone should improve replayextract.py
			printf("cant open *_output.txt (did the map only have 1 human player?\nPress enter to go back to menu..\n");
			return -1;
		}


		//parsing the output of extractreplay, this should really be done via string tokenisation
		
		/* example output of extractreplay
		Player One (name,url,result): usernameone,http://us.battle.net/sc2/en/profile/111111/1/usernameone/,Win
		Player Two (name,url,result): usernametwo,http://us.battle.net/sc2/en/profile/222222/1/usernametwo/,Loss
		Replay Info (type,map,endtime,length): 1v1,Starbow mapname;2013-12-17 10:35:36,24.37
		*/

		std::string replayInfo = outputFileBuffer;
		std::string p1Url, p2Url, line1, part2, line2, line3, p1Name, p2Name, p1Result, p2Result;

		line1 = replayInfo.substr(replayInfo.find(": "), replayInfo.find("\n")-replayInfo.find(": "));
		part2 = replayInfo.substr(replayInfo.find("Two "), replayInfo.find("Replay")-replayInfo.find("Two "));	
		line2 = part2.substr(part2.find(": "), part2.find("\n")-part2.find("t): "));
		line3 = replayInfo.substr(replayInfo.find("th):"), replayInfo.find_last_of("\n") );
		
		//player information
		p1Url = line1.substr(line1.find("http://"),line1.find_last_of("/")+1-line1.find("http://"));
		p2Url = line2.substr(line2.find("http://"),line2.find_last_of("/")+1-line2.find("http://"));
		p1Name = line1.substr(2,line1.find(",")-2);
		p2Name = line2.substr(2,line2.find(",")-2);
		p1Result = line1.substr(line1.find_last_of(",")+1,line1.find_last_of("\n")-line1.find_last_of(","));
		p2Result = line2.substr(line2.find_last_of(",")+1,line2.find_last_of("\n")-line2.find_last_of(",")-1);

		std::cout << "player 1:" << p1Name.c_str() << ";" << p1Url.c_str() << ";" <<p1Result.c_str() << "\n";
		std::cout << "player 2:" << p2Name.c_str() << ";" << p2Url.c_str() << ";" <<p2Result.c_str() << "\n";		

		//game information
		std::string replayGameType = line3.substr(5,3);
		std::string replayMapName = line3.substr( line3.find_first_of(",")+1, line3.find_first_of(";")-line3.find_first_of(",")-1 );	
		std::string part3 = line3.substr(line3.find(";")+1,line3.find("\n")-line3.find(";")-1);
		std::string replayTimestamp = part3.substr(0, part3.find_last_of(",")-1);

		printf("replay gametype=%s\n",replayGameType.c_str());
		printf("replaymapname=%s\n",replayMapName.c_str());
		printf("timestamp=%s\n",replayTimestamp.c_str()); //watchout this uses user time in the server timezone which is super annoying, at least i think so
		printf("length=%s\n", part3.substr(part3.find_last_of(",")+1, part3.find("\n")-1).c_str() );

		//make sure its actually a strabow replay
		if(replayMapName.find("Starbow") != std::string::npos)
		{
			printf("Starbow game detected from replay\n");
		}
		else
		{
			printf("Starbow game not detected from replay!\nPress enter to go back to menu..\n");
			return -1;
		}
		

		//not a new file anymore we have used it now!
		replayFilenamesNew.clear();

		//sometimes the users bnet profile isnt updated fast enough so this is put in place to make sure it is there
		//3 seconds was not long enough so 10 seoncds will do.
		Sleep(10000); 


		printf("\nCheck b.net profile for last game\n");
		//the example i was reading, pretty sure its just a copy of the example in the api
		//http://www.daniweb.com/software-development/c/threads/269500/libcurl-copy-webpage-contents-to-string
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

		// (D) Fetch the data
		curl_easy_perform(curl);

		// (E) Close the connection
		curl_easy_cleanup(curl);

		std::string recentResult;
		int endOfGame, startOfGame;

		endOfGame = buffer.find("}");
		startOfGame = buffer.find("\"map\"");

		recentResult = buffer.substr(startOfGame, endOfGame - startOfGame);

		/* url map format
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
			decisionSubstring = recentResult.substr(recentResult.find("\"decision"), 24); //24 is a size that is bigger then neccessary but can fit win/loss/other fine
			decision = decisionSubstring.substr(decisionSubstring.find(":") + 3, decisionSubstring.find("\",")-decisionSubstring.find(":") -3 );

			//"date" this is unixepoch: 1389531028 
			date = recentResult.substr(recentResult.find("\"date") + 9, 32); 

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
			std::cout << epochDate << std::endl;
		
		}
		else
		{
			printf("Most recent game in bnet profile is not a starbow game\nPress enter to go back to menu..\n");
			return -1;
		}

		std::cout << "player 1:" << p1Name.c_str() << ";" << p1Url.c_str() << ";" <<p1Result.c_str() << "\n";
		std::cout << "player 2:" << p2Name.c_str() << ";" << p2Url.c_str() << ";" <<p2Result.c_str() << "\n";
		
		//convert result string to int
		int p1ResultInt = -99, p2ResultInt = -99; //initialising to a nonresult value
		
		if (strcmp("Win",p1Result.c_str()) == 0)
		{
			p1ResultInt = 1;
		}
		else if (strcmp("Loss",p1Result.c_str()) == 0)
		{
			p1ResultInt = -1;
		}
		else
		{
			p1ResultInt = 0;
		}

		if (strcmp("Win",p2Result.c_str()) == 0)
		{
			p2ResultInt = 1;
		}
		else if (strcmp("Loss",p2Result.c_str()) == 0)
		{
			p2ResultInt = -1;
		}
		else
		{
			p2ResultInt = 0;
		}

		//making sure that replay and server result matches
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


		//what i was just playing with before commiting, requires boost so commented out
		/*
		boost::posix_time::ptime t(boost::posix_time::time_from_string(replayTimestamp));
		tm pt_tm = to_tm(t);
		time_t foo = mktime(&pt_tm);
		
		printf("bnetepoch to date i think:%s\n", boost::posix_time::time_from_string(epochDate));

		//std::string replayEpochTime = boost::posix_time::to_iso_extended_string(t);
		printf("epoch replay time is %f\n",(double) foo);
		*/


		//fill out the struct
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


		sendServerResult(resultStruct);

		//back to waiting for the next replay
		waitingForReplay = true;
	}

	return 0;
}


int sendServerResult(struct GameResult resultStruct)
{
	
	#ifdef _WIN32

		WSADATA wsaData;
		int iResult;

		// Initialise Winsock
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
	
	//create the socket
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


	//conecting
	int connectStatus;
	connectStatus = connect(sockfd, res->ai_addr, res->ai_addrlen);

	if (connectStatus != 0)
	{
	   printf("connect error  = %s\n",gai_strerror(connectStatus));	
	}
	
	//sending to the server
	int bytesSent;
	const unsigned int MESSAGE_LENGTH = 4096; //4096 surely should be long enough
	char sendData[MESSAGE_LENGTH]; 

	//need to put the contents of the struct into a string to pass it to send()
	//much easier than using memcpy to copy the struct into a string, and don't have to worry about endianess this way.
	sprintf(sendData,"%s;%s;%s;%i;%s;%s;%i;\0",resultStruct.user,resultStruct.playerOneName,resultStruct.playerOneURL,resultStruct.playerOneResult
										,resultStruct.playerTwoName,resultStruct.playerTwoURL,resultStruct.playerTwoResult
		);

	bytesSent = send(sockfd, sendData, sizeof(sendData),0);
	printf("bytes sent = %i\n",bytesSent);


	//probably want to have a recv() message accepted at some point possibly
}


//used for libcurl code
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