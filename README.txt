Designed By Omotoye Shamsudeen Adekoya

Documentation for the ARP Assignment 2

Outline
~~~~~~~~

1--> Design Choices
2--> How to Compile
3--> How to run


                                       _____________________
                                       |  DESIGN CHOICES    |   
                                       ---------------------

The program is divided into three sections 

*remote; the remote acts as a client that takes command input from the user in the form of char. The 
 remote then send the command to the "hoist" through the "hoist_server" and receives feedback of the hoist
 height and some other info from the hoist_server.

*hoist_server; the hoist_server acts as a server that takes control input from the remote(client) and sends 
it to the hoist. It takes feedback of the hoist height and sends it to the remote.

*hoist; the hoist is a program that simulates the movement of a hoist in the z-direction.

    The remote and the hoist_server runs as a separate process. The hoist_server forks the hoist process and
exec it as a separate process. Two unnamed pipes are used for communication between the hoist_server and the 
hoist, one pipe for command communication and the second pipe for feedback communication.
    I used 200000(microseconds) per iteration to simulate the speed of the hoist which which represents 
5 iterations per second(5cm/second).
    The remote takes the char 'l' for Lifting the hoist, 'd' for dropping the hoist, 'e' for terminating
the hoist process and itself. To stop the hoist the remote takes CTRL+Z (SIGTSTP), there is a signal
handler in the remote that sets command = 't', the char 't' signifies STOP, it then sends the 't' to 
hoist_server which uses a select to monitor the arrival of a command. As soon as hoist_server gets the command 
it sends it to the hoist which then stops the hoist.
    

                                     _____________________
                                     |   HOW TO COMPILE  |  
                                     ---------------------

--> to compile remote.c
gcc remote.c -o remote

--> to compile hoist_server.c 
gcc hoist_server.c -o hoist_server 

--> to compile hoist.c
gcc hoist.c -o hoist

                                     _____________________
                                     |    HOW TO RUN      |   
                                     ---------------------

****** If you are running the remote and hoist_server on thesame machine ************
./hoist_server <port_no> 
// port_no could be any number from 2000 to 40000, there are other available numbers, i picked this 
range just for safety

./remote 127.0.0.1 <port_no>
// 127.0.0.1 this ip_address signifies that you are on the same machine

****** If you are running the remote and the hoist_server on different machines ***********
.hoist_server <port_no>
//same rule applies as that of thesame machine

./remote <ip_address> <port_no>
// to get the ip_address of the machine you are running the server in, enter the command "ifconfig" into 
// your linux shell, or enter the command "ipconfig" into your window powershell. 
// NB: the quotation mark is not part of the command !!!

Run the hoist server before the remote
follow the prompt messages.

Thanks
Omotoye...




