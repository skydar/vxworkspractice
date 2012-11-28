/* includes */
#include "vxWorks.h"
#include "taskLib.h"
#include "msgQLib.h"
#include "sysLib.h"
#include "stdio.h"

/* globals */

#define  CONSUMER_TASK_PRI   99   /* Priority of the consumer task */
#define  PRODUCER_TASK_PRI  98   /* Priority of the producer task */
#define  TASK_STACK_SIZE    5000   /* stack size for spawned tasks */

struct msg {                         /* data structure for msg passing */
         int tid;                         /* task id */        
         int value;                       /* msg value */
        };

LOCAL MSG_Q_ID msgQId;                   /* message queue id */
LOCAL int numMsg = 8;                    /* number of messages */
LOCAL BOOL notDone;           /* Flag to indicate the completion */

 
/* function prototypes */
LOCAL void producerTask(); /* producer task */
LOCAL void consumerTask(); /* consumer task */

/* user entry */
void msgQDemo()
{
  notDone = TRUE; /* initialize the global flag */
  
  /* Create the message queue*/
  if ((msgQId = msgQCreate(numMsg, sizeof(struct msg), MSG_Q_FIFO)) == NULL)
  {
    perror("Error in creating msgQ");
  }

  /* Spawn the producerTask task */
  if (taskSpawn("tProducerTask", PRODUCER_TASK_PRI, 0, TASK_STACK_SIZE, (FUNCPTR)producerTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
  {
    perror("producerTask: Error in spawning demoTask");
  }

  /* Spawn the consumerTask task */
  if (taskSpawn("tConsumerTask", CONSUMER_TASK_PRI, 0, TASK_STACK_SIZE, (FUNCPTR)consumerTask, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0) == ERROR)
  {
    perror("consumerTask: Error in spawning demoTask");
  }
  
  /* polling is not recommended. But used to make this demonstration simple*/
  while (notDone)
    taskDelay(sysClkRateGet());
  if (msgQDelete(msgQId) == ERROR)
  {
    perror("Error in deleting msgQ");
  }
}

/* producerTask ：发送消息给consumerTask */
void producerTask(void)

{
  int count;
  int value;
  struct msg producedItem; /* producer item – produced data */
  printf("producerTask started: task id = %#x \n", taskIdSelf());
  /* Produce numMsg number of messages and send these messages */
  for (count = 1; count <= numMsg; count++)
  {
    value = count * 10; /* produce a value */
    /* Fill in the data structure for message passing */
    producedItem.tid = taskIdSelf();
    producedItem.value = value;
	
    /* Send Messages */
    if ((msgQSend(msgQId, (char*) &producedItem, sizeof(producedItem),WAIT_FOREVER, MSG_PRI_NORMAL)) == ERROR)
    {
      perror("Error in sending the message");
    }

    else
      printf("ProducerTask: tid = %#x, produced value = %d \n", taskIdSelf(),value);
  }
}

/* consumerTask：获取（消费）消息 */
void consumerTask(void)
{
  int count;
  struct msg consumedItem; /* consumer item – consumed data */
  printf("\n\nConsumerTask: Started -  task id = %#x\n", taskIdSelf());
  /* consume numMsg number of messages */
  for (count = 1; count <= numMsg; count++)
  {
    /* Receive messages */
    if ((msgQReceive(msgQId, (char*) &consumedItem, sizeof(consumedItem),WAIT_FOREVER)) == ERROR)
    {
      perror("Error in receiving the message");
    }
    else
      printf("ConsumerTask: Consuming msg of value %d from tid = %#x\n",consumedItem.value, consumedItem.tid);
  }
  notDone = FALSE; /* set the global flag to FALSE to indicate completion*/
}
