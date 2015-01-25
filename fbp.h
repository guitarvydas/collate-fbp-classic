#include "component.h"
extern void fbp_initinport (component *parent, inport *arr, int n, int qmax);
extern void fbp_initoutport (component *parent, IP *arr, int n);
extern void fbp_queueip (IP outip, inport *in);
IP fbp_dequeueip (inport *in);
extern int fbp_yield (component *self);
extern int fbp_send (component *self, IP *out, IP i);
extern int fbp_receive (component *self, inport *in, void **value);
extern int notempty(inport*);
