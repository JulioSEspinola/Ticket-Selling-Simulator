

#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fstream>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <stdbool.h>
#include <iostream>
using namespace std;

#define BUFFER_SIZE 1024

// Define port number
char recvBuff[BUFFER_SIZE];

// Variables to be read from connection.ini
char* IP; 
int port;
int timeout;


int sockfd,n;
struct sockaddr_in serv_addr;
bool incomplete;
int rows = -1;
int columns = -1;
char * mode;

//Connection file for reading IP, Port, and timeout.
string settingFile;

int getRandTime()
{
    srand(time(NULL));
    int times [] = {3, 5, 7};
    int randTime;
    randTime = rand() % 3;
    return times[randTime];
}

bool checkSettingFile(){
    int length = 0;
    string line;
    ifstream myfile (settingFile);
    if (myfile.is_open())
    {
        while ( getline (myfile,line) )
        {
            length++;
        }
    }
    myfile.close();
    if (length != 4)
    {
        return false;
    }
    else 
    {
        return true;
    }
}

/* 
Method responsible for communicating the desire to 
purchase a ticket
*/

void purchaseOrder(int* coord)
{
    // Load inital command into buffer
    sprintf(recvBuff,"BUYING TICKET");
    // Write intial command across socket
    write(sockfd,recvBuff, sizeof(recvBuff));
    // Load desired ticket position
    write(sockfd,coord, 8);
    
    n = read(sockfd, recvBuff, sizeof(recvBuff)-1);
    cout << recvBuff<<endl;

    //Checks whether the tickes are sold out. If the are. The purchase fails.
    if (strcmp(recvBuff,"SOLD OUT") == 0)
    {
        incomplete = false;
    }
    else
    {
        incomplete = true;
    }
                 
}

int main(int argc, char* argv[])
{
    int cord[2];
    incomplete = true;

    // Make sure the user is following the correct format
    if (argc < 2)
    {
        cout << "Usage: " << argv[0] << " ";
        cout << "<mode> <connectionFile>"<<endl; 
        exit(0);
    }

    mode = argv[1];
    settingFile = argv[1];

    // // Initialize variables related to connection settings
    if (checkSettingFile)
    {
        string line;
        string settingArr[3]; //Setting string arr
        ifstream myfile ("connectionini.txt");
        if (myfile.is_open())
        {
            getline(myfile,line); // skip first line[connection]
            getline(myfile,line); // Grab IP line
            settingArr[0] = line.substr(5,13); // Strip IP = 
            getline(myfile,line); // Grab Port
            settingArr[1] = line.substr(7,11); // Strip port = 
            getline(myfile,line); // Grab timeout
            settingArr[2] = line.substr(10,12);
                
            IP = (char*)settingArr[0].c_str();
            port = stoi(settingArr[1]);
            timeout = stoi(settingArr[2]);
        }
        myfile.close();
    } 
    else
    {
        cout << "ERROR: Could not read connection file"<< endl;
    }

    while (incomplete)
    {    
        recvBuff[BUFFER_SIZE];

        // initalize all values in messBuff to 0
        memset(recvBuff, '0',sizeof(recvBuff)); 

        if ((sockfd = socket(AF_INET,SOCK_STREAM,0)) < 0){ // Create the socket if anything less than 0; error
            printf("\n Error : Could not create socket \n");
            return 1;
        }

        memset(&serv_addr, '0', sizeof(serv_addr));  // fill server address with 0s 

        serv_addr.sin_family = AF_INET;  //set server domain
        serv_addr.sin_port = htons(port); // set server port
        serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        
        if(inet_pton(AF_INET, (const char*)IP, &serv_addr.sin_addr)<0)
        { // run inet pton and error catch
            printf("\n inet_pton error occured\n");
            cout << IP<< endl;
            return 1;
        } 

        //connect and error catch 
        if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        {
            printf("\n ERROR : Connect Failed \n");
            return 1;
        
        } 

        //Gets fucntionality if the user wants to purchase tickets automatically.
        if (strcmp(mode, "automatic") == 0)
        {
            int buyTicket[2];
            if (columns < 1 || rows < 1)
            {
                // sets xSeat and ySeat
                sprintf(recvBuff,"DIMENSIONS");
                write(sockfd,recvBuff,sizeof(recvBuff)-1);
                read(sockfd, cord, 8);
                rows = cord[0];
                columns = cord[1];

                srand(time(NULL)); 

                buyTicket[0] = rand() % (rows) + 1;
                buyTicket[1] = rand() % (columns) + 1;

                purchaseOrder(buyTicket);
            }
            else
            {
                buyTicket[0] = rand() % (rows);
                buyTicket[1] = rand() % (columns);
                purchaseOrder(buyTicket);
            }
        } 
        // Gets functionality if user wants to purchase tickers manually
        else if (strcmp(mode, "manual")==0) // Fully functional DONT TOUCH
        { 
            cout << "Ticket row Position: ";
            cin  >> cord[0]; // input for x
            cout << "Ticket column position: ";
            cin >> cord[1]; // input for y
            
            if (cord[0] > -1 && cord[1] > -1)
            {
                purchaseOrder(cord); // Attempt to purchase the designated ticket
            }
            //Checks if the input is a alphabetic character
            else if (isalpha(cord[0]) || isalpha(cord[1]))
            {
                cout << "Bad input! try another seat\n"<<endl;
                cout << "Ticket row Position: ";
                cin  >> cord[0]; // input for x
                cout << "Ticket column position: ";
                cin >> cord[1]; // input for y
                purchaseOrder(cord);
            }
            //Wrong input
            else
            {
                cout << "Bad input! try another seat\n"<<endl;
                cout << "Ticket row Position: ";
                cin  >> cord[0]; // input for x
                cout << "Ticket column position: ";
                cin >> cord[1]; // input for y
                purchaseOrder(cord);
            }
        }
        sleep(getRandTime());
    }
    cout << "We are out of tickets."<<endl;
}