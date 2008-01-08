/* ====================================================================
 * Copyright (c) 1999-2004 Carnegie Mellon University.  All rights
 * reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. 
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in
 *    the documentation and/or other materials provided with the
 *    distribution.
 *
 * This work was supported in part by funding from the Defense Advanced 
 * Research Projects Agency and the National Science Foundation of the 
 * United States of America, and the CMU Sphinx Speech Consortium.
 *
 * THIS SOFTWARE IS PROVIDED BY CARNEGIE MELLON UNIVERSITY ``AS IS'' AND 
 * ANY EXPRESSED OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, 
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL CARNEGIE MELLON UNIVERSITY
 * NOR ITS EMPLOYEES BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, 
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ====================================================================
 *
 */

/*
  generate.c : A sentence generation routine originally built by
  Arthur Toth. 

  HISTORY 
  $Log: generate.c,v $
  Revision 1.1  2006/04/15 20:59:59  archan
  Added Arthur Toth's n-gram based sentence generator to evallm.  Arthur has given me a very strong precaution to use the code.  So, I will also make sure the current version will only be used by a few.
  
 */

#include <stdlib.h>
#include <time.h>
#include "toolkit.h"
#include "../libs/sih.h"
#include "ngram.h"
#include "idngram2lm.h"
#include "../win32/compat.h"

#define BEGIN_OF_SENTENCE_STRING "<s>"

/*
  Comments from Arthur Chan 

  The initial check-in also lack of the capability of dealing with
  potentially more than 4 bil words. Of course, this is more a
  theoretical issue. 

 */
/*
  Comments from Arthur Toth 
 */
/* generate_words implements the "generate" command in the evallm command loop.

   It can take the following flags (yes, I know I should add this to the help
   text in evallm.c):
   -text filename    The name of the file where text will be generated.
   -size 10000       An optional flag specifying the number of generated words.
                     If unspecified, it will default to 10000.
   -seed 1237688     An optional flag specifying a seed for the random number
                     generator.  If unspecified, it will be set to the time in
                     seconds since the epoch.

   Caveats:
   1) It currently only works for binary language model files.

   2) It currently assumes the language model is a trigram, although it 
   probably wouldn't take much effort to make it more general.

   3) When generating the first word, it currently tries to use an initial 
   history of "<s> <s>" if "<s>" is in the vocabulary and just picks the word 
   at index 1 otherwise.  Something better could probably be done in this case
   (e.g. looking for other context cue words in the vocab list).

   4) Certain platforms appear to use something other than RAND_MAX for the
   maximum number that can be generated by random().  This could cause strange
   things to happen, but could be fixed very easily by replacing RAND_MAX by
   the appropriate platform-specific constant.
*/
void generate_words(ng_t *png,arpa_lm_t *pang, int num_words,int random_seed,char *output_filename)
{
  FILE *output_file;
  int i,j,bo_case,initial_history_id;
  id__t sought_trigram[3];
  double p,acc,trigram_prob;
  vocab_sz_t lm_vocab_sz;
  char** lm_vocab;

  if(png!=NULL && pang!=NULL)
    quit(-1,"Confused by multiple input type.\n");
    
  if(png!=NULL){
    lm_vocab_sz=png->vocab_size;
    lm_vocab=png->vocab;
  }

  if(pang!=NULL){
    quit(-1,"Currently doesn't support arpa input, please use the binary format created by idngram2lm.\n");
    lm_vocab_sz=pang->vocab_size;
    lm_vocab=pang->vocab;
  }

  if (!(output_file=fopen(output_filename,"w"))) {
    fprintf(stderr,"Error: could not open %s for writing.\n",output_filename);
    fprintf(stderr, "Syntax: generate -seed seed_of_random_generator -size size_of_file -text output text file \n");

    return;
  }

  if (random_seed==-1)
    random_seed=(unsigned int)time(NULL);
  
  srandom(random_seed);

  printf("Using %d as a random seed.\n",random_seed);

  initial_history_id = -1;
  for (i=0; i<lm_vocab_sz; ++i){
    if (!strcmp(BEGIN_OF_SENTENCE_STRING,lm_vocab[i])){
      initial_history_id = i;
      fprintf(stderr,"Found %s in the vocabulary at index %d.\n",
	      BEGIN_OF_SENTENCE_STRING,i);
      break;
    }
  }

  if (initial_history_id == -1)    {
    fprintf(stderr,"Did not find %s in the vocabulary.\n",
	    BEGIN_OF_SENTENCE_STRING);
      /* Okay, we should do something much more intelligent here than
         just picking a vocabulary word, but I don't have the time right
         now and the corpora I use have "<s>" and should hit this case 
         anyway.
      */
    initial_history_id=1;
  }

  sought_trigram[0]=initial_history_id;
  sought_trigram[1]=initial_history_id;
  fprintf(stderr,"Using an initial history of \"%s %s\"\n",
	  lm_vocab[sought_trigram[0]],lm_vocab[sought_trigram[1]]);   

  for (i=1; i<=num_words; ++i) {
    p=((double)random())/RAND_MAX; /* This is platform-specific and needs to be fixed */

    if (p<.5){
      acc=0.0;
      for (j=0; j<=lm_vocab_sz; ++j){
	sought_trigram[2]=j;
	bo_ng_prob(2, sought_trigram, png, DEFAULT_VERBOSITY,
		   &trigram_prob, &bo_case);
	acc+=trigram_prob;
	if (p<=acc) break;
      }

      if (p>acc) 
	fprintf(stderr, "WARNING: The sum over w3 of Pr(w3|%s,%s) was %f,"
		"which was less than the randomly generated number %f.\n",
		acc,lm_vocab[sought_trigram[0]],lm_vocab[sought_trigram[1]],p);
    }else {
      acc=1.0;
      for (j=lm_vocab_sz; j>=0; --j){
	sought_trigram[2]=j;
	bo_ng_prob(2, sought_trigram, png, DEFAULT_VERBOSITY,
		   &trigram_prob, &bo_case);
	acc-=trigram_prob;
	if (p>=acc) break;
      }

      if (p<acc)
	fprintf(stderr, "WARNING: 1-(sum over w3 of Pr(w3|%s,%s) was %f,"
		"which was greater than the randomly generated number %f.\n",
		acc,lm_vocab[sought_trigram[0]],lm_vocab[sought_trigram[1]],p);
    }

    fprintf(output_file,"%s ",lm_vocab[sought_trigram[2]]);
    if (!(i%10000))
      printf("%d words output.\n",i);
    sought_trigram[0] = sought_trigram[1];
    sought_trigram[1] = sought_trigram[2];
  }
  fprintf(output_file,"\n");
}







