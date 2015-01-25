/* page 91 component C (collate) */

#include <stdio.h>
#include <malloc.h>
#include "fbp.h"
#include "components.h"

static inport in[2];
static IP out[1];

/* pull 1 header # from in[0], pull records from in[1] while header # matches */

/* peudo-code:

   rec = fbp_receive (self, in[1]);
   while (1) {
      header = fbp_receive (self, in[0]);
      fbp_send (self, &out[0], header);
      while (header == rec->header_number) {
        fbp_send (self, &out[0], rec);
	rec = fbp_receive (self, in[1]);
      }
   }

*/

/* minimalist C implementation */
static record *rec;
static int state;
static record *header;
static int recnum;

static void body (component *self) {
  switch (state) {
  case 0:
    if (fbp_receive (self, &self->inports[1], (IP *)&rec)) { state = 1; return; case 1: ; } /* pull first record, for its header # */
    if (debug) printf("collate receives record /%s/\n", rec->string);
    while (1) {
      if (fbp_receive (self, &self->inports[0], (IP *)&header)) { state = 2; return; case 2: ; }
      if (debug) printf("collate receives header /%s/\n", header->string);
      recnum = header->header_number;  /* save record #, since header is going to be freed (down the line) */
      if (debug) printf("collate sends header /%s/\n", header->string);
      if (fbp_send (self, &out[0], header)) { state = 3; return; }
    case 3: case 4: case 5: case 6:
      while (recnum == rec->header_number) {
	switch (state) {
	case 3:
	  if (debug) printf ("collate sends record /%s/\n", rec->string);
	  if (fbp_send (self, &out[0], rec)) { state = 5; return; }
	case 5:
	  if (fbp_receive (self, &self->inports[1], (IP *)&rec)) { state = 6; return; }
	case 6:
	  if (debug) printf("collate receives record /%s/\n", rec->string);
	  state = 3;
	}
      }
    }
  }
}



component *collate(component *parent) {
  component *self = (component *) malloc (sizeof(component));
  self->name = "COLLATE";
  state = 0;
  self->qbound = 2;
  self->inports = (inport *) malloc (sizeof(inport) * 2);
  fbp_initinport (self, self->inports, 2, self->qbound);
  self->outports = &out[0];
  fbp_initoutport (self, out, 1);
  self->data = NULL; /* not used in this example */
  self->body = body;
  self->next = NULL;
  return self;
}
