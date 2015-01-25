typedef struct s_record {
  int header_number;
  char string[128];
} record;

extern component *page91();
extern component *collate (component *);
extern component *headers (component *);
extern component *records (component *);
extern component *consumer (component *);
