/*
* Operating System Project1 Problem4
* Copyright (c) 2017 Marsl
*/
//-----------------------------------------------------------
/*
* Run: BBC #Cooks #Cashiers #Customers #RackSize
*/
//------------------------------------------------------------
/*
* Cooks, Cashiers, and Customers are each modeled as a thread.
* Cashiers sleep until a customer is present.
* A Customer approaching a cashier can start the order process.
* A Customer cannot order until the cashier is ready.
* Once the order is placed, a cashier has to get a burger from the rack.
* If a burger is not available, a cashier must wait until one is made.
* The cook will always make burgers and place them on the rack.
* The cook will wait if the rack is full.
* There are NO synchronization constraints for a cashier presenting food to the customer.
*/

#include<semaphore.h>
#include<pthread.h>
#include<stdio.h>
#include<stdlib.h>
#include<time.h>

int numcustomer;
sem_t order,rack,burger,semcashier;

void *cook(void *args)
{
	int i=(int)(*(int *)args);
	while(numcustomer!=0)
	{
		sem_wait(&rack);
		sleep(1);
		sem_post(&burger);
		printf("Cook[%d] makes a burger.\n", i);
	}
	return;
}

void *customer(void *args)
{
	int i=(int)(*(int *)args);
	sleep(rand()%i);
    printf("Customer[%d] comes.\n",i);
    sem_wait(&semcashier);
    sem_post(&order);
    return;
}

void *cashier(void *args)
{
	int i=(int)(*(int *)args);
	while(numcustomer!=0)
	{
		sem_wait(&order);
		printf("Cashier[%d] accepts an order.\n",i);
		sem_wait(&burger);
		sem_post(&rack);
		sleep(1);
		numcustomer--;
		printf("Cashier[%d] takes a burger to a customer.\n",i);
		sem_post(&semcashier);
	}
	return;
}

int main(int argu,char *argv[])
{
	pthread_t Tcook[10000],Tcashier[10000],Tcustomer[10000];
	int Icook[10000],Icashier[10000],Icustomer[10000];
	int k;
	int tmp;

	srand(time(0));
	
	if (argu != 5)//five inputs from command line
        {
        	printf("Invalid input!\n");
        	return 1;
        }
	
	for(k=0;k<10000;k=k+1)
	{
		Icook[k] = k+1;
		Icashier[k] = k+1;
		Icustomer[k] = k+1;
	}

	int ncook,ncashier,ncustomer,nrack;
	ncook = atoi(argv[1]);
    ncashier = atoi(argv[2]);
    ncustomer = atoi(argv[3]);
    nrack = atoi(argv[4]);
	
    numcustomer = ncustomer;

	printf("Cooks[%d],Cashiers[%d],Customers[%d],Rack[%d].\n",ncook,ncashier,ncustomer,nrack);
    //initialize semaphore 
	sem_init(&rack,0,nrack);
    sem_init(&burger,0,0);
    sem_init(&semcashier,0,ncashier);
	sem_init(&order,0,0);

	for(k=0;k<ncook;k=k+1)//create thread for cook
	{
		tmp = pthread_create(&Tcook[k],NULL,cook,&Icook[k]);
		if(tmp != 0)
		{
			printf("Fail to create cook\n");
			return 1;
		}
	}
	
	for(k=0;k<ncashier;k=k+1)//create thread for cashier
	{
		tmp = pthread_create(&Tcashier[k],NULL,cashier,&Icashier[k]);
		if(tmp != 0)
		{
			printf("Fail to create canshier\n");
			return 1;
		}
	}
	
	for(k=0;k<ncustomer;k=k+1)//create thread for customer
	{
		tmp = pthread_create(&Tcustomer[k],NULL,customer,&Icustomer[k]);
		if(tmp != 0)
		{
			printf("Fail to create customer\n");
			return 1;
		}
	}

	for(k=0;k<ncook;k=k+1)
        pthread_join(Tcook[k],NULL);
    for(k=0;k<ncashier;k=k+1)
        pthread_join(Tcashier[k],NULL);
	for(k=0;k<ncustomer;k=k+1)
        pthread_join(Tcustomer[k],NULL);

    sem_destroy(&rack);
	sem_destroy(&order);
    sem_destroy(&burger);
    sem_destroy(&semcashier);
    return 0;
}

