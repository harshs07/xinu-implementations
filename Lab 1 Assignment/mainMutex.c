/*  main.c  - main */

#include <xinu.h>

pid32 producer_id;
pid32 consumer_id;
pid32 timer_id;

int32 consumed_count = 0;
const int32 CONSUMED_MAX = 100;

/* Define your circular buffer structure and semaphore variables here */
/* */
int32 buffer[10];				//random size 10
int32 head=0,tail=0;
sid32 mutex;

/* Place your code for entering a critical section here */
int mutex_acquire(sid32 mutex)
{
	wait(mutex);
}

/* Place your code for leaving a critical section here */
void mutex_release(sid32 mutex)
{
	signal(mutex);
}

/* Place the code for the buffer producer here */

process producer(void)
{	printf("Producer entry\n");
	/* */
	int i;
	while(1)
	{		
	mutex_acquire(mutex);	
	if(buffer[tail]==-1)		//Check buffer full condition
		{	
		i=tail;	
		buffer[tail]=1;
		tail=(tail+1)%10; 
		printf("Produced data at %d \n",i);		/*keeping I/O function inside critical section just for understanding printed output. Tried using it outside the critical section as well */
		}
	mutex_release(mutex);		
	}
	return OK;
}

/* Place the code for the buffer consumer here */
process consumer(void)
{	printf("Consumer entry\n");
	
	int i;	
	while(1)
	{		
	mutex_acquire(mutex);
		if(buffer[head]!=-1)   //Check buffer empty condition
		{	
		i=head;		
		buffer[head]=-1;
		head=(head+1)%10;
		consumed_count += 1;
		printf("Consumed data at %d \n", i);		
		}
	mutex_release(mutex);	 	
	}	
	return OK;
}


/* Timing utility function - please ignore */
process time_and_end(void)
{
	int32 times[5];
	int32 i;

	for (i = 0; i < 5; ++i)
	{
		times[i] = clktime_ms;
		yield();

		consumed_count = 0;
		while (consumed_count < CONSUMED_MAX * (i+1))
		{
			yield();
		}

		times[i] = clktime_ms - times[i];
	}

	kill(producer_id);
	kill(consumer_id);

	for (i = 0; i < 5; ++i)
	{
		kprintf("TIME ELAPSED (%d): %d\n", (i+1) * CONSUMED_MAX, times[i]);
	}
}

process	main(void)
{
	recvclr();

	/* Create the shared circular buffer and semaphores here */
	/* */
	mutex=semcreate(1);
	int i;
	for(i=0;i<10;i++)	//Initialize buffer to empty
	buffer[i]=-1;
	
	producer_id = create(producer, 4096, 50, "producer", 0);
	consumer_id = create(consumer, 4096, 50, "consumer", 0);
	timer_id = create(time_and_end, 4096, 50, "timer", 0);

	resched_cntl(DEFER_START);
	resume(producer_id);
	resume(consumer_id);
	/* Uncomment the following line for part 3 to see timing results */
	resume(timer_id); 
	resched_cntl(DEFER_STOP);

	return OK;
}

