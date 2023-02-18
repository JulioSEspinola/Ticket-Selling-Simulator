
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include <string.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <arpa/inet.h>
using namespace std;

#define BUFFER_SIZE 1024
#define PORT_NUMBER 5437

// Define default values
int rows = 5;
int columns = 5; 
int s[] = {rows,columns};
int ** ticketArr;
int connfd,listenfd;
char sendBuff[BUFFER_SIZE];


// Function to print the plane of seats from 2D array using pointers
//https://codeforwin.org/2017/12/access-two-dimensional-array-using-pointers-c-programming.html
void printSeats(int** matrix) {
    cout<<"\nAvailable Seats : \n";
    cout<<"1 = available | 0 = not available\n\n";
    for(int i=0 ; i<=rows-1 ; i++) 
    {
        for(int j=0 ; j<=columns-1 ; j++)
            cout<< *(*(ticketArr+i)+j)<<" ";
        cout<<endl;
    }
    cout<<endl;
}

/* 
Generates a matrix of numRows x numColumns filled with 1's 
representing the available seats
*/
int** createPlane(){
    srand(time(0));
    int** ticketArray = 0;
    ticketArray = new int*[rows];

    // Populate ticket array with 1s and intialize each row
    for (int i = 0; i < rows; i++){
        ticketArray[i] = new int[columns];
        for (int j = 0; j < columns; j++){
            ticketArray[i][j] = 1;
        }
    }
    return ticketArray; 
}

/* 
This method is responsible for selling the requested ticket provided in the
char array 'coord'

@Param coord - represents the coordiates as arguments
*/
bool sellTicket(int* coord){
    int xCord = coord[0]; 
    int yCord = coord[1];

    if ((xCord >= 0 && xCord<=rows-1) && 
        (yCord >= 0 && columns-1))
    {
        // Check if the ticket is within the range of the plane of seats available
        if (xCord > (rows-1) || yCord > (columns-1))
        {   // Check if ticket selection is out of bounds
            printf("Ticket (%d, %d) is out of range", xCord, yCord);
            sleep(1);
            return false; // Indicate that the sale failed because ticket does not exist in the plane
        }
        else 
        {
            // If ticket is available then sell it by setting it to 0
            if (ticketArr[xCord][yCord] == 1)
            {
                // Sell ticket and set to unavailable
                ticketArr[xCord][yCord] = 0; 
                return true;
            }
            else 
            {
                return false; 
            }
        }
    } 
    else 
    {
        return false;
    }
}

int main(int argc, char *argv[])
{
    // If dimensions were passed in
    if (argc >= 3)
    {
        // Get X dimension
        rows = atoi(argv[1]);     
        // Get Y dimension
        columns = atoi(argv[2]);
    }

    int coordinates[2];

    // Allocate memory for the ticketArr and populate with 1's
    ticketArr = createPlane();
    srand(time(NULL));

    // Set up socket variables
    listenfd = 0; 
    connfd = 0;
    struct sockaddr_in serv_addr; 

    //buffer to load messages into
    sendBuff[BUFFER_SIZE];

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&serv_addr, '0', sizeof(serv_addr));
    memset(sendBuff, '0', sizeof(sendBuff)); 

    // Setting Server address parameters
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(PORT_NUMBER); 

    bind(listenfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)); 

    // Listen
    listen(listenfd, 10);
    char xdim,ydim;
    xdim = (char)rows;
    ydim = (char)columns;
    char dimensions[] = {xdim,',',ydim};

    while(1)
    {   
        system("clear");
        int cnt=0; // Keep track of seats taken
        for (int i = 0; i < rows; i++)
            for (int j = 0; j < columns; j++)
                if (ticketArr[i][j] == 0)
                    cnt++;

        //The counter keeps track of how many seats are taken
        //Once all seats are taken then indicate to the client that we are sold out
        if (cnt == (rows*columns))
        {
            // Display available tickets
            printSeats(ticketArr);   
            // Accept connection
            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
            // Read command from receiver
            read(connfd, sendBuff, sizeof(sendBuff));
    
            // Send a message to indicate that we are sold out
            sprintf(sendBuff, "SOLD OUT"); 
            write(connfd,sendBuff,sizeof(sendBuff)); 
            close(connfd);
            exit(0);
        }
        
        //Seats are sill available
        else
        {
            // Display available tickets
            printSeats(ticketArr);   
            // Accept connection
            connfd = accept(listenfd, (struct sockaddr*)NULL, NULL);
            // Read command from receiver
            read(connfd, sendBuff, sizeof(sendBuff));
            // Print buffer
            printf("%s",sendBuff);

            /* This outer for loop will evaluate an inital message
            after determining what action is being requested, await secondary message from the client
            that will indicate what seats the client is interested in*/
            if (strcmp(sendBuff,"DIMENSIONS") == 0)
            {
                system("clear");
                int buyTickets[2];
                memset(buyTickets,0,sizeof(buyTickets));
                write(connfd,s, sizeof(s)-1);
                // Reads PURCHASE TICKET
                read(connfd,sendBuff,sizeof(sendBuff)-1);

                read(connfd,buyTickets,8); 
                read(connfd,buyTickets,8);
                sellTicket(buyTickets);
                write(connfd, sendBuff,sizeof(sendBuff)-1);
            }
            else if (strcmp(sendBuff,"BUYING TICKET") == 0)
            {
                read(connfd, coordinates, sizeof(coordinates));
                cout << endl << "Selling :" << coordinates[0] << "," << coordinates[1] <<endl;
                sleep(3);
                
                if (sellTicket(coordinates))
                {
                    // Send a message to confirm the purchase was succesful 
                    sprintf(sendBuff, "\nSuccesfully purchased ticket (%d, %d) " ,coordinates[0], coordinates[1]);
                    write(connfd,sendBuff,sizeof(sendBuff));
                }
                else
                {
                    sprintf(sendBuff, "\nFAILED: Ticket at (%d, %d) not available", coordinates[0], coordinates[1]);
                    write(connfd,sendBuff,sizeof(sendBuff));
                }
            }
        }
    }
}     