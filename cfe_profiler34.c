#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <err.h>
#include "uthash.h"
#include "cfengine.h"
#include <time.h>

#define __USE_GNU

#include <dlfcn.h> // for dlsym()

/*
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>
 *
 *
 *
 *  Cfe-profiler: a CFEngine profiler - Loic Pefferkorn <loic-cfe@loicp.eu>
 *
 *  http://www.loicp.eu/cfe-profiler
 *
 */

const int MAX_HASH_LEN = 1024;
const uint64_t NANOSECS_IN_SEC = 1000000000L;

// man program_invocation_name, GNU extension
extern char *program_invocation_short_name;

typedef struct _bundle_stats bundle_stats;
struct _bundle_stats {
  char *key;          // Hash of the 4th next fields
  char *namespace;
  char *bundletype;
  char *name;
  struct timespec elapsed_time;   // Total time taken 
  UT_hash_handle hh;
};

bundle_stats *bundles_stats = NULL;

uint64_t cfep_timespec2ns(struct timespec x);
void cfep_timespec_sub(const struct timespec *x, const struct timespec *y, struct timespec *res);
void add_bundle_call(Promise *pp, struct timespec elapsed_time);
int sort_by_time(bundle_stats *a, bundle_stats *b);

uint64_t cfep_timespec2ns(struct timespec x) {

  return x.tv_sec * NANOSECS_IN_SEC + x.tv_nsec;
}

void cfep_timespec_sub(const struct timespec *x, const struct timespec *y, struct timespec *res) {

  res->tv_sec = x->tv_sec - y->tv_sec;
  res->tv_nsec = x->tv_nsec - y->tv_nsec;
  if (res->tv_nsec < 0) {
    res->tv_sec--;
    res->tv_nsec += NANOSECS_IN_SEC;
  }
}

void timespec_addto(struct timespec *x, const struct timespec *y) {

    x->tv_sec += y->tv_sec;
    x->tv_nsec += y->tv_nsec;
    if ( x->tv_nsec >= NANOSECS_IN_SEC) {
      x->tv_sec ++;
      x->tv_nsec -= NANOSECS_IN_SEC;
    }
}

// For each bundle, add an entry to a global hash
void add_bundle_call(Promise *pp, struct timespec elapsed_time) {

  bundle_stats *bs = NULL;
  char *hash = NULL;

  hash = malloc(MAX_HASH_LEN);
  if (hash == NULL)
    err(1, "Cannot allocate memory for hash\n");

  snprintf(hash, MAX_HASH_LEN, "%s%s%s",
    pp->namespace,
    pp->bundletype,
    pp->bundle);

  HASH_FIND_STR(bundles_stats, hash, bs);

  if (bs == NULL) {
    bs = malloc(sizeof(*bs));
    if (bs == NULL)
     err(1, "Cannot allocate memory for bundle stats\n");

    bs->key = hash;
    bs->namespace = strdup(pp->namespace);
    bs->bundletype = strdup(pp->bundletype);
    bs->name = strdup(pp->bundle);
    bs->elapsed_time = elapsed_time;
    HASH_ADD_KEYPTR(hh, bundles_stats, bs->key, strlen(bs->key), bs);
  } else {
    timespec_addto(&bs->elapsed_time, &elapsed_time);
    free(hash);
  }
}

// Display bundle execution statistics
void print_stats() {

  // Statistics are only relevant while overriding ExpandPromise() in cf-agent 
  if (strcmp(program_invocation_short_name, "cf-agent") != 0 ) {
    return;
  }

  bundle_stats *bs = NULL;
  struct timespec total_time;

  total_time.tv_sec = 0;
  total_time.tv_nsec = 0;

  // Get CPU ticks used overall
  for(bs=bundles_stats; bs != NULL; bs=(bundle_stats *)(bs->hh.next)) {
    timespec_addto(&total_time, &bs->elapsed_time);
  }

  printf("\nCfe-profiler-0.1: a CFEngine profiler - http://www.loicp.eu/cfe-profiler\n");
  puts("\n*** Sorted by wall-clock time ***\n");
  printf("%7s %9s %15s %20s\n", 
    "Time(s)", "Namespace", "Type", "Bundle");

  HASH_SORT(bundles_stats, sort_by_time);

  for(bs=bundles_stats; bs != NULL; bs=(bundle_stats *)(bs->hh.next)) {

    printf("%7.2f %9s %15s %20s\n",
      (float) cfep_timespec2ns(bs->elapsed_time) / NANOSECS_IN_SEC,
      bs->namespace,
      bs->bundletype,
      bs->name);
  }
}

// Helper function to sort hash by time taken
int sort_by_time(bundle_stats *a, bundle_stats *b) {
  return (cfep_timespec2ns(a->elapsed_time) <= cfep_timespec2ns(b->elapsed_time));
}

// Our version of ExpandPromise(): collect informations about promise, then run real ExpandPromise
void ExpandPromise(enum cfagenttype agent, const char *scopeid, Promise *pp, void *fnptr, const ReportContext *report_context) {

  struct timespec start, end, diff;
  static int atexit_handler_registered = 0;
  void (*ExpandPromise_orig) (enum cfagenttype agent, const char *scopeid, Promise *pp, void *fnptr, const ReportContext *report_context);

  // Print statistics at the end of cf-agent execution
  if (atexit_handler_registered == 0) {
    if (atexit(print_stats) != 0) {
      fprintf(stderr, "Cannot register atexit() handler\n");
      exit(EXIT_FAILURE);
    }
    atexit_handler_registered = 1;
  }

  // Get a pointer to the real ExpandPromise() function, to call it later
  ExpandPromise_orig = dlsym(RTLD_NEXT, "ExpandPromise");

  if (ExpandPromise_orig == NULL) {
    fprintf(stderr, "Cannot find ExpandPromise symbol, exiting...\n");
    exit(EXIT_FAILURE);
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  ExpandPromise_orig(agent, scopeid, pp, fnptr, report_context);
  clock_gettime(CLOCK_MONOTONIC, &end);

  // Compute time taken by the execution
  cfep_timespec_sub(&end, &start, &diff);
  add_bundle_call(pp, diff);
}
