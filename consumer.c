/* page 91 component D */

#include <stdio.h>
#include <malloc.h>
#include "fbp.h"
#include "components.h"

static inport in[1];

/* receive input, print it, drop it */

static int h;
static record *rec;
static int state;

static void body (component *self) {
  switch (state) {
  case 0:
    while (1) {
      if (fbp_receive (self, &self->inports[0], (IP *)&rec)) { state = 1; return; case 1: ; }
      printf("consumer receives /%s/\n", rec->string);
      free (rec);
    }
  }
}

component *consumer(component *parent) {
  component *self = (component *) malloc (sizeof (component));
  self->name = "CONSUMER";
  state = 0;
  self->qbound = 2;
  self->inports = (inport *) malloc (sizeof(inport) * 1);
  fbp_initinport (self, self->inports, 1, self->qbound);
  self->outports = NULL;
  self->data = NULL; /* not used in this example */
  self->body = body;
  self->next = NULL;
  return self;
}
