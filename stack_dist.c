/* STACK_DIST.C -- James Dinan <dinan@cse.ohio-state.edu> -- July, 2010
 *
 * This program reads a data reference trace on stdin and produces as output a
 * stack distance histogram for the reference trace.  Data references should
 * appear one-per-line on the input and may be any string.  Leading whitespace
 * is ignored as well as empty lines and lines with a '#' as the first
 * non-whitespace character.
 *
 * Stack distance is defined for data reference X as the number of unique data
 * references since the last time X was referenced.  This program builds up a
 * hash table of data references (strings) and maintains a queue of data
 * references.  An entry in the queue holds a pointer to a string in the hash
 * table, so when we search the queue to find the time X was referenced we do
 * (less expensive) pointer comparison rather than string comparison.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <glib.h>

#include "list.h"

#ifdef ENABLE_DEBUGGING
#define DEBUG(CMD) CMD
#else
#define DEBUG(CMD)
#endif

#define DEFAULT_NBUCKETS 1000000
#define B_OVFL   nbuckets
#define B_INF    nbuckets+1

/* Tunable parameters */
int nbuckets  = DEFAULT_NBUCKETS;

/*
 * Helper for hash table string comparison
 */
gboolean compare_strings(gconstpointer a, gconstpointer b) {
  if (strcmp(a,b) == 0)
    return TRUE;
  else
    return FALSE;
}


/*
 * Process an input string in-place in the input buffer.
 *
 * We skip the whitespace at the beginning of a line, empty lines, and lines
 * that begin with a '#'.
 *
 * @return The length of the new string (always <= strlen(input)).
 */
static inline int process_string(char *input) {
  int i, j;

  // Scan for the first non-whitespace character
  for (i = 0; input[i] != '\0'; i++) {
    const char c = input[i];
    if ( !(c == ' ' || c == '\t' || c == '\n') )
      break;
  }

  // Skip empty lines or lines that start with a '#'
  if (input[i] == '\0' || input[i] == '#')
    return 0;

  // Shift the string left and leave off the newline
  for (j = 0; input[i] != '\n' && input[i] != '\0'; i++, j++)
    input[j] = input[i];

  input[j] = '\0';

  return j;
}


/*
 * Process command line arguments
 */
void process_args(int argc, char **argv) {
  int   arg;
  char *endptr;

  while ((arg = getopt(argc, argv, "b:h")) != -1) {
    switch (arg) {
    case 'b':
      nbuckets = strtol(optarg, &endptr, 10);
      if (endptr == optarg) {
        fprintf(stderr, "Error, invalid depth: %s\n", optarg);
        exit(1);
      }
      break;

    case 'h':
      printf("Stack Distance Tool - Analyze data reference traces to produce a locality histogram\n");
      printf("  Usage: %s [args] (input read on STDIN)\n\n", basename(argv[0]));
      printf("Options:\n");
      printf("  -b  Number of buckets (default: %d)\n", nbuckets);
      printf("  -h  Help\n");
      exit(0);
      break;

    default:
      fprintf(stderr, "Try '-h' for help.\n");
      exit(1);
    }
  }

  if (optind < argc) {
    fprintf(stderr, "Error, extra arguments (input should be on STDIN): %s ...\n", argv[optind]);
    exit(1);
  }
}


int main(int argc, char **argv) {
  GHashTable   *data_elem;
  list_t       *access_list;
  int           i;
  char          input[1000];
  unsigned int *histogram;

  process_args(argc, argv);

  histogram   = malloc(sizeof(unsigned int) * (nbuckets+2));
  data_elem   = g_hash_table_new(g_str_hash, compare_strings);
  access_list = list_create();

  // Initialize buckets
  for (i = 0; i < nbuckets+2; i++)
    histogram[i] = 0;

  // Analyze the trace
  while (fgets(input, 1000, stdin) != NULL) {
    int   distance;
    void *lookup;
    int   length;
    
    length = process_string(input);

    // Skip this line?
    if (length == 0)
      continue;

    lookup = g_hash_table_lookup(data_elem, input);

    // Cold start: Not in the list yet
    if (lookup == NULL) {
      char *data = strdup(input);
      void *elem = list_push(access_list, data);
      g_hash_table_insert(data_elem, data, elem);  // Store pointer to list element
      histogram[B_INF] += 1;
      DEBUG(printf("Added %p\n", data);)
    }

    // Hit: We've seen this data before
    else {
      DEBUG(printf("Found %p\n", lookup);)
      distance = list_move_to_head(access_list, lookup);

      // Is distance greater than the largest bucket
      if (distance > nbuckets)
        histogram[B_OVFL] += 1;
      else
        histogram[distance] += 1;
    }
  }

  DEBUG(list_print(access_list);)

  // Print out the histogram
  {
    int last_bucket;
    unsigned long long sum = 0;  // For normalizing
    unsigned long long cum = 0;  // Cumulative output

    // Find the last non-zero bucket
    last_bucket = nbuckets-1;
    while (histogram[last_bucket] == 0)
      last_bucket--;

    for (i = 0; i <= last_bucket; i++)
      sum += histogram[i];
    sum += histogram[B_OVFL];

    printf("# Dist\t     Refs\t   Refs(%%)\t  Cum_Ref\tCum_Ref(%%)\n");

    for (i = 0; i <= last_bucket; i++) {
      cum += histogram[i];
      printf("%6d\t%9d\t%0.8f\t%9lld\t%0.8f\n", i, histogram[i], histogram[i]/(double)sum, cum, cum/(double)sum);
    }

    cum += histogram[B_OVFL];
    printf("#OVFL \t%9d\t%0.8f\t%9lld\t%0.8f\n", histogram[B_OVFL], histogram[B_OVFL]/(double)sum, cum, cum/(double)sum);

    printf("#INF  \t%9d\n", histogram[B_INF]);
  }

  return 0;
}
