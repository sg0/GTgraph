#include "utils.h"

void writeToFile(graph* g) {

	FILE* outfp;
	LONG_T i;

	if ((WRITE_TO_FILE) && (STORE_IN_MEMORY == 1)) {
		fprintf(stderr, "Writing to file ... ");
		outfp = fopen(OUTFILE, "w");
		fprintf(outfp, "c FILE			: %s\n", OUTFILE);
		fprintf(outfp, "c No. of vertices	: %ld\n", g->n);
		fprintf(outfp, "c No. of directed edges	: %ld\n", g->m); 
		fprintf(outfp, "c Max. weight		: %ld\n", MAX_WEIGHT);
		fprintf(outfp, "c Min. weight		: %ld\n", MIN_WEIGHT);
		fprintf(outfp, "c A directed arc from u to v of weight w\n");
		fprintf(outfp, "c is represented below as ' a  u  v  w '\n");
		fprintf(outfp, "p sp %ld %ld\n", g->n, g->m);
	
		for (i=0; i<g->m; i++)
			fprintf(outfp, "a %ld %ld %ld\n", g->start[i]+1, \
					g->end[i]+1, g->w[i]);
		fclose(outfp);
		fprintf(stderr, "done\n");	
	}
}

#if defined(WRITE_CSR_BINARY)
void writeToFileCSRBinary(graph* g) {

	FILE* outfp;
	int64_t i, j, k;

	if ((WRITE_TO_FILE) && (STORE_IN_MEMORY == 1)) {
		fprintf(stderr, "Writing to binary CSR file ... ");
		outfp = fopen(OUTFILE, "wb");
    int64_t nv = (int64_t)g->n, ne = (int64_t)(2*g->m);
    
    fwrite(&nv,sizeof(int64_t),1,outfp);
    fwrite(&ne,sizeof(int64_t),1,outfp);

    int64_t* ec = (int64_t*) malloc((nv+1)*sizeof(int64_t));
    int64_t* ec_tmp = (int64_t*) malloc((nv+1)*sizeof(int64_t));
    
    struct tuple_t {
      int64_t i_, j_;
      double w_;
    };
     
    struct edge_t {
      int64_t tail_;
      double weight_;
    };

    int cmp(const void *e0, const void *e1)
    {
        struct tuple_t *ed0 = (struct tuple_t *)e0;
        struct tuple_t *ed1 = (struct tuple_t *)e1;
        return ((ed0->i_ < ed1->i_) || ((ed0->i_ == ed1->i_) && (ed0->j_ < ed1->j_))); 
    } 
            
    struct tuple_t* elist = (struct tuple_t*) malloc(ne*sizeof(struct tuple_t));
        
    memset(ec, 0, (nv+1)*sizeof(int64_t));
    memset(ec_tmp, 0, (nv+1)*sizeof(int64_t));
	
    for (i=0; i<ne/2; i++)
    {
      elist[i].i_ = g->start[i];
      elist[i].j_ = g->end[i];
      elist[i].w_ = (double)g->w[i];
      elist[i+1].i_ = g->end[i];
      elist[i+1].j_ = g->start[i];
      elist[i+1].w_ = (double)g->w[i];

      ec_tmp[g->start[i]+1]+=1;
      ec_tmp[g->end[i]+1]+=1;
    }
    
    for (k=0; k<nv; k++)
      ec[k+1] = ec[k] + ec_tmp[k];

    qsort(elist,ne,sizeof(struct tuple_t),cmp);
	   
    fwrite(&ec[0],sizeof(int64_t),nv+1,outfp);
    
    for (i=0; i<nv; i++) {
      for (j=ec[i]; j<ec[i+1]; j++) {
          struct edge_t edge;
          edge.tail_ = elist[j].j_;
          edge.weight_ = elist[j].w_;
          fwrite(&edge,sizeof(struct edge_t),1,outfp);
      }
    }

    fclose(outfp);
		fprintf(stderr, "done\n");	

    free(ec);
    free(ec_tmp);
    free(elist);
	}
}
#endif

void updateLog() {

	FILE* logfp;

	logfp = fopen(LOGFILE, "w");
	fprintf(logfp, "\n");
	fprintf(logfp, "Graph Generator Parameters\n\n");
	fprintf(logfp, "**************************\n");
	fprintf(logfp, "n			: %ld\n", n);
	fprintf(logfp, "m 			: %ld\n", m);
	fprintf(logfp, "a			: %lf\n", a);
	fprintf(logfp, "b			: %lf\n", b);
	fprintf(logfp, "c                       : %lf\n", c);
        fprintf(logfp, "d                       : %lf\n", d);
	fprintf(logfp, "SELF_LOOPS		: %d\n", SELF_LOOPS);			
	fprintf(logfp, "MAX_WEIGHT		: %ld\n", \
			MAX_WEIGHT);
	fprintf(logfp, "MIN_WEIGHT		: %ld\n", \
			MIN_WEIGHT);
	fprintf(logfp, "STORE_IN_MEMORY		: %d\n", \
			STORE_IN_MEMORY);
	fprintf(logfp, "WRITE_TO_FILE		: %d\n", \
			WRITE_TO_FILE);
	fprintf(logfp, "SORT_EDGELISTS		: %d\n", \
			SORT_EDGELISTS);
	fprintf(logfp, "SORT_TYPE	        : %d\n", \
                        SORT_TYPE);
	fprintf(logfp, "OUTFILE			: %s\n", OUTFILE);
	fprintf(logfp, "LOGFILE			: %s\n", LOGFILE);
	fprintf(logfp, "\n");
	fclose(logfp);
				
}

void countsort_aux(LONG_T q, LONG_T *lKey, LONG_T *lSorted, \
		   LONG_T* auxKey, LONG_T* auxSorted, \
		   LONG_T R, LONG_T bitOff, LONG_T m) 
{
	register LONG_T j, k, last, temp, offset;
    
	static LONG_T *myHisto, *psHisto;

	LONG_T *mhp, *mps, *allHisto;

	myHisto  = (LONG_T *) malloc(R*sizeof(LONG_T));
	psHisto  = (LONG_T *) malloc(R*sizeof(LONG_T));

	mhp = myHisto;

	for (k=0 ; k<R ; k++)
		mhp[k] = 0;
    
	for (k=0; k<q; k++)
		mhp[bits(lKey[k],bitOff,m)]++;

	for (k=0; k<R; k++) {
		last = psHisto[k] = myHisto[k];
		for (j=1 ; j<1 ; j++) {
			temp = psHisto[j*R + k] = last + myHisto[j*R +  k];
			last = temp;
		}
	}

	allHisto = psHisto;
	
	offset = 0;

	mps = psHisto;

	for (k=0 ; k<R ; k++) {
		mhp[k]  = (mps[k] - mhp[k]) + offset;
		offset += allHisto[k];
	}
    
	for (k=0; k<q; k++) {
		j = bits(lKey[k],bitOff,m);
		lSorted[mhp[j]] = lKey[k];
		auxSorted[mhp[j]] = auxKey[k];
		mhp[j]++;
	}

	free(psHisto);
	free(myHisto);
}


void countingSort(LONG_T* keys, LONG_T* auxKey1, \
		   WEIGHT_T* auxKey2, LONG_T m)
{
	LONG_T *keysSorted;
	LONG_T *index, *indexSorted;
	LONG_T i, *t1, *t2;
	WEIGHT_T* wt;

	keysSorted = (LONG_T *) malloc(m*sizeof(LONG_T));
	index = (LONG_T *) malloc(m*sizeof(LONG_T));
	indexSorted = (LONG_T *) malloc(m*sizeof(LONG_T));

	for (i=0; i<m; i++) {
		index[i] = i;
		keysSorted[i] = 0;
			
	}

	countsort_aux(m, keys, keysSorted, index, indexSorted, (1<<11),  0, 11);

	/* for temp computation, reuse arrays instead of new malloc */
	t1 = keys;
        t2 = index;

	countsort_aux(m, keysSorted, t1, indexSorted, t2, (1<<11), 11, 11);
	countsort_aux(m, t1, keysSorted, t2, indexSorted, (1<<10), 22, 10);
	
	free(t2);

	for (i=0; i<m; i++) {
		keys[i] = keysSorted[i];
	}
	
	t1 = keysSorted;

	for (i = 0; i < m; i++) {
		t1[i] = auxKey1[indexSorted[i]];	
	}
	
	for (i=0; i<m; i++) {
		auxKey1[i] = t1[i];
	}

	free(t1);

	wt = (WEIGHT_T *) malloc(m*sizeof(WEIGHT_T));

        for (i = 0; i<m; i++) {
                wt[i] = auxKey2[indexSorted[i]];
        }

	for (i = 0; i<m; i++) {
		auxKey2[i] = wt[i];
	}

	free(indexSorted);
	free(wt);

}

void heapSort(LONG_T* keys, LONG_T* auxKey1, WEIGHT_T* auxKey2, LONG_T n)
{
	LONG_T i;

	for (i = (n/2)-1; i >= 0; i--)
		siftDown(keys, auxKey1, auxKey2, i, n);

	for (i = n-1; i >= 1; i--) {
		swapL(&keys[0], &keys[i]);
                swapL(&auxKey1[0], &auxKey1[i]);
                swapW(&auxKey2[0], &auxKey2[i]);
		siftDown(keys, auxKey1, auxKey2, 0, i-1);
	}

}


void siftDown(LONG_T* keys, LONG_T* auxKey1, WEIGHT_T* auxKey2, LONG_T root, LONG_T bottom)
{

	LONG_T done, maxChild;

	done = 0;
	while ((root*2 <= bottom) && (!done)) {
		if (root*2 == bottom)
			maxChild = root * 2;
		else if (keys[root * 2] > keys[root * 2 + 1])
			maxChild = root * 2;
		else
			maxChild = root * 2 + 1;

		if (keys[root] < keys[maxChild]) {
			swapL(&keys[root], &keys[maxChild]);
			swapL(&auxKey1[root], &auxKey1[maxChild]);
			swapW(&auxKey2[root], &auxKey2[maxChild]);
			root = maxChild;
		} else {
			done = 1;
		}
	}
}

void swapL(LONG_T* x, LONG_T* y) {
	LONG_T temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

void swapW(WEIGHT_T* x, WEIGHT_T* y) {
	WEIGHT_T temp;
	temp = *x;
	*x = *y;
	*y = temp;
}

