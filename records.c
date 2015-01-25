/* page 91 component B */

#include <stdio.h>
#include <malloc.h>
#include "fbp.h"
#include "components.h"

static IP out[1];

/* send 20 * 100 IPs each set of 100 tagged with an IP header number */

static int i, j;
static int state;
static record *rec;

static void body (component *self) {
  switch (state) {
  case 0:
    for (i = 0 ; i < 20 ; i++ ) {
      for (j = 0 ; j < 100 ; j ++) {
        rec = (record *) malloc (sizeof (record));
	rec->header_number = i;
	sprintf (rec->string, "REC %d %d", i, j);
	if (debug) printf("records sends /%s/\n", rec->string);
	if (fbp_send (self, &out[0], (IP)rec)) { state = 1; return; case 1: ; }
      }
    }
    self->state = dead;
  }
}


component *records(component *parent) {
  component *self = (component *) malloc (sizeof (component));
  self->name = "RECORDS";
  state = 0;
  self->qbound = 2;
  self->inports = NULL;
  self->outports = &out[0];
  fbp_initoutport (self, out, 1);
  self->data = NULL; /* not used in this example */
  self->body = body;
  self->next = NULL;
  return self;
}
