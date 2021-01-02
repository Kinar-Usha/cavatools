/*
  Copyright (c) 2020 Peter Hsu.  All Rights Reserved.  See LICENCE file for details.
*/

#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "caveat.h"
#include "opcodes.h"
#include "insn.h"
#include "perfctr.h"


struct perfCounters_t perf;


void perf_create(const char* shm_name)
{
  dieif(!shm_name, "Must include --perf=");
  int n = (insnSpace.bound - insnSpace.base) / 2;
  long sz = sizeof(struct perf_header_t);
  sz += n * sizeof(struct count_t);
  sz += n * sizeof(long) * 3;
  //  sz += insnSpace.bound - insnSpace.base;
  int fd = shm_open(shm_name, O_CREAT|O_TRUNC|O_RDWR, S_IRWXU);
  dieif(fd<0, "shm_open() failed in perf_create");
  dieif(ftruncate(fd, sz)<0, "ftruncate() failed in perf_create");
  perf.h = (struct perf_header_t*)mmap(0, sz, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  dieif(perf.h==0, "mmap() failed in perf_create");
  memset((char*)perf.h, 0, sz);
  perf.h->base  = insnSpace.base;
  perf.h->bound = insnSpace.bound;
  perf.h->size = sz;
  perf.count_array = (struct count_t*)( (char*)perf.h + sizeof(struct perf_header_t) );
  perf.ib_miss = (long*)&perf.count_array[n];
  perf.ic_miss = (long*)&perf.ib_miss[n];
  perf.dc_miss = (long*)&perf.ic_miss[n];
  //  perf.text_segment = (char*)&perf.dc_miss[n];
  //  memcpy(perf.text_segment, (char*)insnSpace.base, (insnSpace.bound-insnSpace.base));
  for (Addr_t pc=perf.h->base; pc<perf.h->bound; pc+=2)
    decode_instruction(&perf.count_array[(pc-perf.h->base)/2].i, pc);
}

void perf_open(const char* shm_name)
{
  int fd = shm_open(shm_name, O_RDONLY, 0);
  dieif(fd<0, "shm_open() failed in perf_open");
  perf.h = (struct perf_header_t*)mmap(0, sizeof(struct perf_header_t), PROT_READ, MAP_SHARED, fd, 0);    
  dieif(perf.h==0, "first mmap() failed in perf_open");
  long sz = perf.h->size;
  dieif(munmap(perf.h, sizeof(struct perf_header_t))<0, "munmap() failed in perf_open");
  perf.h = (struct perf_header_t*)mmap(0, sz, PROT_READ, MAP_SHARED, fd, 0);
  dieif(perf.h==0, "second mmap() failed in perf_open");
  perf.count_array = (struct count_t*)( (char*)perf.h + sizeof(struct perf_header_t) );
  long n = (perf.h->bound - perf.h->base) / 2;
  perf.ib_miss = (long*)&perf.count_array[n];
  perf.ic_miss = (long*)&perf.ib_miss[n];
  perf.dc_miss = (long*)&perf.ic_miss[n];
  //  perf.text_segment = (char*)&perf.ic_miss[n];
}

void perf_close()
{
  dieif(munmap(perf.h, perf.h->size)<0, "munmap() failed in perf_close");
}
