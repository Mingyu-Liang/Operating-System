# CS356-SJTU-OperatingSystem-Project1
## Android Process Tree
* Tools (All IN THE FOLDER DESCRIPITION)
* 1.  AVD(Android Virtual Devices)
* 2.  NDK(cross compile the program and run it on AVD).
* 3.  Linux code in Android(building modules)

## Problem1
### Write a new system call in Linux.
* The system call you write should take two arguments and return the process tree information in a depthfirst-search (DFS) order.
* Each system call must be assigned a number. Your system call should be assigned number 356.

## Problem2
### Test your new system call
* Write a simple C program which calls ptree
* Print the entire process tree (in DFS order)
* Using tabs to indent children with respect to their parents.

## Problem3
* Generate a new process and output “StudentIDParent” with PID, then generates its children process output “StudentIDChild” with PID.
* Use execl to execute ptree in the child process，show the relationship between above two process.

## Project4(Burger Buddies Problem)
* Cooks, Cashiers, and Customers are each modeled as a thread.
* Cashiers sleep until a customer is present.
* A Customer approaching a cashier can start the order process.
* A Customer cannot order until the cashier is ready.
* Once the order is placed, a cashier has to get a burger from the rack.
* If a burger is not available, a cashier must wait until one is made.
* The cook will always make burgers and place them on the
  rack.
* The cook will wait if the rack is full.There are NO synchronization constraints for a cashier presenting food to the customer.


