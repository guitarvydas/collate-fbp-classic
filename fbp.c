#include <malloc.h>
#include <assert.h>
#include "fbp.h"

void fbp_initinport (component *parent, inport *arr, int n, int qmax) {
  while (--n >= 0) {
    arr[n].head = 0;
    arr[n].tail = 0;
    arr[n].qmax = qmax;
    arr[n].qcount = 0;
    arr[n].queue = (IP*) malloc (qmax * sizeof (IP));
    arr[n].wait_pull = NULL;
    arr[n].wait_push = NULL;
    arr[n].parent = parent;
  }
}

void fbp_initoutport (component *parent, IP *outarr, int n) {
  while (--n) {
    outarr[n] = NULL;
  }
}

static void inchead (inport *in) {
  in->head += 1;
  if (in->head >= in->qmax)
    in->head = 0;
}

static void inctail (inport *in) {
  in->tail += 1;
  if (in->tail >= in->qmax)
    in->tail = 0;
}

void fbp_queueip (IP outip, inport *in) {
  /* move ip from out and queue it on in - called by scheduler */
  /* guaranteed that in queue is not full at this point */
  if (in->qcount == 0) {
    in->qcount = 1;
    in->head = in->tail;
    in->queue[in->head] = outip;
    inctail(in);
  } else {
    in->qcount += 1;
    in->queue[in->tail] = outip;
    inctail(in);
  }
  if (in->wait_pull) {
    /* signal one waiter */
    component *c;
    c = in->wait_pull;
    if (debug) printf("%s signalled for pull\n", c->name);
    in->wait_pull = c->next;
    c->next = NULL;
    c->state = run_pull;
  }
}

IP fbp_dequeueip (inport *in) {
  IP res;
  assert (in->qcount > 0);
  res = in->queue[in->head];
  in->qcount -= 1;
  inchead(in);
  if (in->wait_push) {
    /* signal one waiter */
    component *c;
    c = in->wait_push;
    if (debug) printf("%s signalled for push\n", c->name);
    in->wait_push = c->next;
    c->next = NULL;
    c->state = run_push;
  }
  return res;
}


int fbp_yield (component *self) {
  self->state = yielded;
  return 1;
}  /* must execute return immediately after calling yield */


int fbp_send (component *self, IP *out, IP i) {
  assert (*out == NULL);  /* only 1 ip can be sent at a time, before yielding to kernel (which then delivers it to an input) */
  *out = i;
  return 1;  /* always yield to scheduler, it will attempt to move the ip to the input queue (or suspend) */
}

int fbp_receive (component *self, inport *in, IP *value) {
  if (in->qcount == 0) {
    if (debug) printf ("%s blocks on receive\n", self->name);
    self->state = suspended;
    self->readptr = value;  /* "kernel" will stuff read into *value when reawoken */
    in->wait_pull = self;
    self->waitin = in;
    return 1;
  }
  *value = fbp_dequeueip (in);
  return 0;
}

int notempty (inport *in) {
  return (in->qcount > 0);
}
