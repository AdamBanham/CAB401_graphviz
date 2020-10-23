/* $Id$ $Revision$ */
/* vim:set shiftwidth=4 ts=8: */

/*************************************************************************
 * Copyright (c) 2011 AT&T Intellectual Property 
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * which accompanies this distribution, and is available at
 * http://www.eclipse.org/legal/epl-v10.html
 *
 * Contributors: See CVS logs. Details at http://www.graphviz.org/
 *************************************************************************/

/*
 * Written by Stephen North and Eleftherios Koutsofios.
 */

#include "config.h"
#include <advisor-annotate.h>
#include "gvc.h"
#include "gvio.h"
#include "omp.h"
#include <math.h>

#ifdef WIN32_DLL
__declspec(dllimport) boolean MemTest;
__declspec(dllimport) int GvExitOnUsage;
/*gvc.lib cgraph.lib*/
#else   /* not WIN32_DLL */
#include "globals.h"
#endif

#include <stdlib.h>
#include <time.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

static GVC_t *Gvc;
static graph_t * G;

#ifndef _WIN32
static void intr(int s)
{
/* if interrupted we try to produce a partial rendering before exiting */
    if (G)
	gvRenderJobs(Gvc, G);
/* Note that we don't call gvFinalize() so that we don't start event-driven
 * devices like -Tgtk or -Txlib */
    exit (gvFreeContext(Gvc));
}

#ifndef NO_FPERR
static void fperr(int s)
{
    fprintf(stderr, "caught SIGFPE %d\n", s);
    /* signal (s, SIG_DFL); raise (s); */
    exit(1);
}
#endif
#endif

static graph_t *create_test_graph(void)
{
#define NUMNODES 5

    Agnode_t *node[NUMNODES];
    Agedge_t *e;
    Agraph_t *g;
    Agraph_t *sg;
    int j, k;
    char name[10];

    /* Create a new graph */
    g = agopen("new_graph", Agdirected,NIL(Agdisc_t *));

    /* Add nodes */
    for (j = 0; j < NUMNODES; j++) {
	sprintf(name, "%d", j);
	node[j] = agnode(g, name, 1);
	agbindrec(node[j], "Agnodeinfo_t", sizeof(Agnodeinfo_t), TRUE);	//node custom data
    }

    /* Connect nodes */
    for (j = 0; j < NUMNODES; j++) {
	for (k = j + 1; k < NUMNODES; k++) {
	    e = agedge(g, node[j], node[k], NULL, 1);
	    agbindrec(e, "Agedgeinfo_t", sizeof(Agedgeinfo_t), TRUE);	//edge custom data
	}
    }
    sg = agsubg (g, "cluster1", 1);
    agsubnode (sg, node[0], 1);

    return g;
}

int main(int argc, char **argv)
{
    double start = omp_get_wtime();
    graph_t *prev = NULL;
    double runs[10];
    double dotruns[10];
    int run = 0;
    double difference, before;
    int r, rc = 0;

    Gvc = gvContextPlugins(lt_preloaded_symbols, DEMAND_LOADING);
    GvExitOnUsage = 1;
    gvParseArgs(Gvc, argc, argv);
    for (int i = 0; i < argc; i++) {
        printf("arg %d is %s\n", i, argv[i]);
    }
#ifndef _WIN32
    signal(SIGUSR1, gvToggle);
    signal(SIGINT, intr);
#ifndef NO_FPERR
    signal(SIGFPE, fperr);
#endif
#endif
    if (MemTest) {
	while (MemTest--) {
	    /* Create a test graph */
	    G = create_test_graph();

	    /* Perform layout and cleanup */
	    gvLayoutJobs(Gvc, G);  /* take layout engine from command line */
	    gvFreeLayout(Gvc, G);
	    agclose (G);
	}
    }
    else if ((G = gvPluginsGraph(Gvc))) {
	    gvLayoutJobs(Gvc, G);  /* take layout engine from command line */
	    gvRenderJobs(Gvc, G);
    }
    else {
	while ((G = gvNextInputGraph(Gvc))) {
	    if (prev) {
		gvFreeLayout(Gvc, prev);
		agclose(prev);
	    }
        before = omp_get_wtime( );
	    gvLayoutJobs(Gvc, G);  /* take layout engine from command line */
        dotruns[run] = omp_get_wtime() - before;
	    gvRenderJobs(Gvc, G);
            gvFinalize(Gvc);
	    r = agreseterrors();
	    rc = MAX(rc,r);
	    prev = G;
        difference = omp_get_wtime( ) - before;
        runs[run] = difference;
        run++;
        printf("completed runtime [%.3f, %.3f] secs.\n",
            dotruns[run-1],
            difference);
	}
    }
    r = gvFreeContext(Gvc);
    double meantime = 0;
    double dottime = 0;
    double count = 0;
    for (int i = 0; i < run; i++) {
        meantime += runs[i];
        dottime += dotruns[i];
        count++;
    }
    meantime = meantime / (count);
    dottime = dottime / (count);
    printf("completed runs with mean of %.3f out of %.3f secs.\n",
        dottime,
        meantime );
    return (MAX(rc,r));
}
