/* FBP components */


typedef void *IP;

struct s_component;

typedef struct s_inport {
  int head;  /* first item to be dequeued */
  int tail;  /* first empty slot */
  int qmax;
  int qcount;
  IP *queue;  /* array of ip*'s */
  struct s_component *wait_pull;
  struct s_component *wait_push;
  struct s_component *parent;
} inport;

typedef void (*function) (struct s_component *);

enum estate { running, suspended, run_pull, run_push, yielded, dead };

typedef struct s_component {
  enum estate state;
  int qbound;
  inport *inports;		/* array of inports */
  IP *outports;			/* outputs is an array of single ip pointers */
  inport *waitin;		/* port if blocked on read */
  IP *readptr;			/* dest for blocked read (where to stuff value) */
  void *data;
  function body;
  struct s_component *next;
  char *name;
} component;

extern int debug;
