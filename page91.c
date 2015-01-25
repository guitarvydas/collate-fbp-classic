/* collate example on page 91 of 2nd ed. */

/* restrictions:

   1. Cannot create multiple instances of components, due to static declaration of data.  This would be easy
   to fix - by replacing the static data with malloc'ed data attached to the "data" field of the
   component.  I choose to use static data to make the code as simple and clear as possible.

   2. Preemption is not supported.  Components must call fbp_send or fbp_receive or fbp_yield to yield control.

*/


#include <stdio.h>
#include <malloc.h>
#include "fbp.h"
#include "components.h"

static component* instances[4];

int debug = 0;

/*
  In words, 
  forever
  for any/every child component that has an input
  invoke the component until it generates a single output ip OR
  until it blocks on empty-port fbp_receive or invokes fbp_yield
  end
  for any/every child that has output IPs in its output queue
  move the output IP to the appropriate input port
  end
  end

  The body function of a container is a coroutine.

  The "graph" is hard-wired (compiled-into) the code of the body function.

  The "body" function of a container simply shepherds IP's between components.
  This could easily be done using a table of connections, but
  I choose to do this with code - to keep things simple and visible.
  The algorithm I use here might be inefficient yet implements FBP semantics.
  Clearly many optimizations are possible.

  In this example, components never terminate (left as an exercise for the reader).
*/

static void body (component *self) {
  int i;
  IP ip;
  component *child;
  inport *targetport;
  while (1) {
  top:
    for (i = 0; i < 4; i += 1) { /* input checking loop */
      child = instances[i];
      switch (child->state) {
      case run_pull:
	/* awaken if ready */
	if (notempty(child->waitin)) {
	  /* second "half" of receive - get the value and stuff it into the appropriate child's variable */
	  if (debug) printf ("%s pull unblocked\n", child->name);
	  ip = fbp_dequeueip (child->waitin);
	  *child->readptr = ip;
	  child->state = running;
	}
	continue;
      case run_push:
	child->state = running;
	break;  		/* go straight to output side, to finish unblocked write*/
      case running:
	(*child->body)(child);
	break;  		/* go straight to output side, in case a send has been made */
      case suspended:
	continue;
      case yielded:		/* this is probably just an optimization, child gives up one cycle, then runs */
	child->state = running;
	continue;
      case dead:
	continue;
      }
      for (i = 0; i < 4; i += 1) { /* output checking loop */
	child = instances[i];

	switch (i) {  		/* here, the compiler knows exactly how many output ports there are for
				   each component, so it can hardwire the code in each case */

	case 0: 			/* 1 output port only for component A */
	  if (child->state == running) {
	    if (child->outports[0] != NULL) {
	      targetport = &instances[2]->inports[0];
	      if (targetport->qmax == targetport->qcount) {
		/* block on write */
		if (debug) printf("headers blocked on push\n");
		child->state = suspended;
		targetport->wait_push = child;
		goto top;
	      }
	      fbp_queueip (child->outports[0], targetport);
	      child->outports[0] = NULL; /* mark outport as not blocked */
	      goto top;
	    }
	  }
	  continue;  /* goto for loop, not top: */

	case 1:			/* 1 output port only for component B */
	  if (child->state == running) {
	    if (child->outports[0] != NULL) {
	      targetport = &instances[2]->inports[1];
	      if (targetport->qmax == targetport->qcount) {
		if (debug) printf("records blocked on push\n");
		child->state = suspended;
		targetport->wait_push = child;
		goto top;
	      }
	      fbp_queueip (child->outports[0], targetport);  /* only difference from case 0, is [1] */
	      child->outports[0] = NULL;		     /* mark outport as not blocked */
	      goto top;
	    }
	  }
	  continue;

	case 2:			/* 1 output port for collate component */
	  if (child->state == running) {
	    if (child->outports[0] != NULL) {
	      targetport = &instances[3]->inports[0];
	      if (targetport->qmax == targetport->qcount) {
		if (debug) printf("collate blocked on push\n");
		child->state = suspended;
		targetport->wait_push = child;
		goto top;
	      }
	      fbp_queueip (child->outports[0], targetport);
	      child->outports[0] = NULL; /* mark outport as not blocked */
	      goto top;
	    }
	  }
	  continue;

	case 3:
	  break;  		/* component D has no outports */
	}
      }
    }
  }
}

component *page91 (component *parent) {
  component *self = (component*) malloc (sizeof (component));
  self->qbound = 10; /* bound input queues to length 10 */
  instances[0] = headers(self);
  instances[1] = records(self);
  instances[2] = collate(self);
  instances[3] = consumer(self);
  self->body = body;
  return self;
}

int main (int argc, char **argv) {
  component *example = page91(NULL);
  (*example->body) (example);
}
