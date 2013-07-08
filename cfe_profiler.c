#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include "uthash.h"
#include "cfengine.h"

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

typedef struct _bundle_stats bundle_stats;
struct _bundle_stats {
  char *key;          // Hash of the 4th next fields
  char *namespace;
  char *bundletype;
  char *bundle;
  char *agentsubtype;
  uint64_t time_us;   // Total time taken is µs
  UT_hash_handle hh;
};

bundle_stats *bundles_stats = NULL;

void timespec_substract(const struct timespec *x, const struct timespec *y, struct timespec *res);
void add_bundle_call(Promise *pp, uint64_t timing);
int sort_by_time(bundle_stats *a, bundle_stats *b);

void timespec_substract(const struct timespec *x, const struct timespec *y, struct timespec *res) {

  if (y->tv_nsec < x->tv_nsec) {
    res->tv_nsec = x->tv_nsec - y->tv_nsec;
    res->tv_sec = x->tv_sec - y->tv_sec;
  } else {
    res->tv_nsec = 1000000000 + y->tv_nsec - x->tv_nsec;
    res->tv_sec = y->tv_sec + 1 - x->tv_sec;
  }
}

// For each bundle, add an entry to a global hash
void add_bundle_call(Promise *pp, uint64_t timing_us) {

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
    bs->bundle = strdup(pp->bundle);
    bs->time_us = timing_us;
    HASH_ADD_KEYPTR(hh, bundles_stats, bs->key, strlen(bs->key), bs);
  } else {
    bs->time_us += timing_us;
    free(hash);
  }
}

// Display bundle execution statistics
void print_stats() {

  bundle_stats *bs = NULL;
  uint64_t total_time = 0;

  // Get CPU ticks used overall
  for(bs=bundles_stats; bs != NULL; bs=(bundle_stats *)(bs->hh.next)) {
    total_time += bs->time_us;
  }

  printf("\nCfe-profiler-0.1: a CFEngine profiler - http://www.loicp.eu/cfe-profiler\n");
  printf("\n*** Sorted by CPU time - total CPU time: %.2fs ***\n", (float) total_time / 1000000);
  printf("%%     time(s)   Namespace     Type      Bundle\n");

  HASH_SORT(bundles_stats, sort_by_time);

  for(bs=bundles_stats; bs != NULL; bs=(bundle_stats *)(bs->hh.next)) {

    printf("%-7.2f %-9.1f %-10s %-10s %-20s\n",
      (float) bs->time_us / total_time * 100,
      (float) bs->time_us / 1000000,
      bs->namespace,
      bs->bundletype,
      bs->bundle);
  }
}

// Helper function to sort hash by time taken
int sort_by_time(bundle_stats *a, bundle_stats *b) {
  return (a->time_us <= b->time_us);
}

// Our version of ExpandPromise(): collect informations about promise, then run real ExpandPromise
void ExpandPromise(enum cfagenttype agent, const char *scopeid, Promise *pp, void *fnptr, const ReportContext *report_context) {

  struct timespec start, end, diff;
  uint64_t us = 0;
  void (*ExpandPromise_orig) (enum cfagenttype agent, const char *scopeid, Promise *pp, void *fnptr, const ReportContext *report_context);

  // Get a pointer to the real ExpandPromise() function, to call it later
  ExpandPromise_orig = dlsym(RTLD_NEXT, "ExpandPromise");

  if (ExpandPromise_orig == NULL) {
    fprintf(stderr, "Cannot find ExpandPromise symbol, exiting...\n");
    exit(EXIT_FAILURE);
  }

  clock_gettime(CLOCK_MONOTONIC, &start);
  ExpandPromise_orig(agent, scopeid, pp, fnptr, report_context);
  clock_gettime(CLOCK_MONOTONIC, &end);

  // Compute time taken by the execution in microseconds
  timespec_substract(&end, &start, &diff);
  us = diff.tv_sec * 1000000 + diff.tv_nsec / 1000;
  add_bundle_call(pp, us);
}

// Our version of GenericDeInitialize(): a cleanup function we use to fire the output of statistics
void GenericDeInitialize() {

  void *(*GenericDeInitialize_orig) ();
  
  GenericDeInitialize_orig = dlsym(RTLD_NEXT, "GenericDeInitialize");
  GenericDeInitialize_orig();
  print_stats();
}
