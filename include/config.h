#ifndef CONFIG_H
#define CONFIG_H

#define SHARED_MEMORY_NAME "shared-mem"
#define SHARED_LOCATION_NAME "/mem-controller-location"
#define SHARED_REQUESTS_NAME "/mem-request-queue"
#define SHARED_COMPLETIONS_NAME "/mem-completion-queue"

#if !defined(REQUEST_QUEUE_SIZE)
#define REQUEST_QUEUE_SIZE 100
#endif

#if !defined MAX_CLIENTS
#define MAX_CLIENTS 1
#endif

struct list {
  struct list *next;
  int data;
};

struct shared_memory {
  struct list *head;
  struct list **tail;
};

struct memory_location {
  int ready;

  struct shared_memory *shared_mem_ptr;
  struct request_queue_list *shared_requests_list;
  struct request_queue_list *shared_completions_list;
};

#endif /* CONFIG_H */
