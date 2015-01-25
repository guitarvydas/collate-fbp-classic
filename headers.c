/* page 91 component A */

#include <stdio.h>
#include <malloc.h>
#include "fbp.h"
#include "components.h"

static IP out[1];

/* send 20 ips (each a single integer) to signify the header number */

static int i;
static record *rec;
static int state;

static void body (component *self) {
  switch (state) {
  case 0:
    for (i = 0 ; i < 20 ; i++ ) {
      rec = (IP) malloc (sizeof (record));
      rec->header_number = i;
      sprintf (rec->string, "HEADER %d", i);
      if (debug) printf("headers sends /%s/\n", rec->string);
      if (fbp_send (self, &out[0], (IP)rec)) {state = 1; return; case 1: ; }
    }
    self->state = dead;
  }
}

component *headers(component *parent) {
  component *self = (component *) malloc (sizeof (component));
  self->name = "HEADERS";
  state = 0;
  self->qbound = 2;
  self->inports = NULL;
  self->outports = out;
  fbp_initoutport (self, out, 1);
  self->data = NULL; /* not used in this example */
  self->body = body;
  self->next = NULL;
  return self;
}
