Socket Programming
CS342 Assignment 3

How to Run : 
In the terminal, execute : 
make clean && make all
Then to initialise the server, execute : 
./server <PORT NUMBER>
Ex : ./server 8888
To initialise a client, execute : 
./client <SERVER_IP> <SERVER_PORT>
Ex : ./client localhost 8888

Database format:
Database is stored in database.txt file. It is formatted as follows :
Each line has 3 space separated values : 
1. First is UPC_CODE lying between 0 and 999
2. Second is price of 1 item
3. Third is the name of the product, string of 1 word, max length 50

You can change the database as needed, by modifying database.txt

Group Members :
- Aditya Trivedi 190101005
- Atharva Vijay Varde 190101018
- Shashwat Sharma 190123055
