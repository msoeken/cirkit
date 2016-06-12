/**CFile****************************************************************

  FileName    [extraZddGraph.c]

  PackageName [extra]

  Synopsis    [Graph operators proposed in O. Coudert. Solving Graph 
  Optimization Problems with ZBDDs. Proc. ED&T '97, pp. 224-228.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddGraph.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Finds the set of all cliques of the graph.]

  Description []

  SideEffects [None]

  SeeAlso     [Extra_zddMaxCliques Extra_zddIncremCliques]

******************************************************************************/
DdNode *
Extra_zddCliques(
  DdManager * dd,    /* manager */
  DdNode * Graph,    /* graph whose cliques are sought */
  int fMaximal)      /* if TRUE, only the maximal cliques are returned */
{
    DdNode  *Support, *Universe, *AllEdges, *CompGraph, *Result;

    /* find support the graph */
    Support = Extra_zddSupport( dd, Graph );
    if ( Support == NULL )
        return NULL;
    Cudd_Ref( Support );

    /* find ZDD for all combinations of vars belonging to support (2^Support) */
    Universe = Extra_zddUniverse( dd, Support );
    if ( Universe == NULL )
    {
        Cudd_RecursiveDerefZdd( dd, Support );
        return NULL;
    }
    Cudd_Ref( Universe );

    /* find ZDD for all edges composed of vars belonging to the support */ 
    AllEdges = Extra_zddTuples( dd, 2, Support );
    if ( AllEdges == NULL )
    {
        Cudd_RecursiveDerefZdd( dd, Support );
        Cudd_RecursiveDerefZdd( dd, Universe );
        return NULL;
    }
    Cudd_Ref( AllEdges );
    Cudd_RecursiveDerefZdd( dd, Support );

//  cuddGarbageCollectZdd( dd, 1 );


    /* find ZDD of the complemented graph */ 
    CompGraph = Cudd_zddDiff( dd, AllEdges, Graph );
    if ( CompGraph == NULL )
    {
        Cudd_RecursiveDerefZdd( dd, Universe );
        Cudd_RecursiveDerefZdd( dd, AllEdges );
        return NULL;
    }
    Cudd_Ref( CompGraph );
    Cudd_RecursiveDerefZdd( dd, AllEdges );

//  printf("\nReturn value of Cudd_DebugCheck( ddman ) is %d\n", Cudd_DebugCheck( dd ) );
//  printf("\nReturn value of Cudd_CheckKeys( ddman ) is %d\n", Cudd_CheckKeys( dd ) );


//  printf("\nUniverse =\n");
//  Extra_zddPrintMinterm( dd, Universe );

//  printf("\nAllEdges =\n");
//  Extra_zddPrintMinterm( dd, AllEdges );

//  printf("\nCompGraph =\n");
//  Extra_zddPrintMinterm( dd, CompGraph );


    /* find ZDD of the set of all cliques */ 
    if ( fMaximal )
    {
        Result = Extra_zddMaxNotSupSet( dd, Universe, CompGraph);

/*      DdNode *Temp = Extra_zddNotSupSet( dd, Universe, CompGraph);
        Cudd_Ref( Temp );

        Result = Extra_zddMaximal( dd, Temp );
        Cudd_Ref( Result );
        Cudd_RecursiveDerefZdd( dd, Temp );
        Cudd_Deref( Result );
*/  }
    else
    {
        Result = Extra_zddNotSupSet( dd, Universe, CompGraph);
    }

    if ( Result == NULL )
    {
        Cudd_RecursiveDerefZdd( dd, Universe );
        Cudd_RecursiveDerefZdd( dd, CompGraph );
        return NULL;
    }
    Cudd_Ref( Result );
    Cudd_RecursiveDerefZdd( dd, Universe );
    Cudd_RecursiveDerefZdd( dd, CompGraph );

    Cudd_Deref( Result );
    return Result;

} /* end of Extra_zddCliques */


/**Function********************************************************************

  Synopsis    [Incrementally computes the set of all cliques.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_zddIncremCliques( 
  DdManager * dd,    /* the manager */
  DdNode    * G,     /* the graph whose cliques of size n are contructed */ 
  DdNode    * C)     /* the set of cliques of size n-1 */ 
{
    DdNode  *res;
    int     autoDynZ;

    autoDynZ = dd->autoDynZ;
    dd->autoDynZ = 0;

    do {
    dd->reordered = 0;
    res = extraZddIncremCliques(dd, G, C);
    } while (dd->reordered == 1);
    dd->autoDynZ = autoDynZ;
    return(res);

} /* end of Extra_zddIncremCliques */


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_zddIncremCliques().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraZddIncremCliques(
  DdManager * dd,    /* the manager */
  DdNode    * G,     /* the graph whose cliques of size n are contructed */ 
  DdNode    * C)     /* the set of cliques of size n-1 */ 
{
    DdNode *zRes;
    statLine(dd); 

    /* terminal cases */
    if ( C == dd->zero )
        return dd->zero;

    /* imposible terminal cases */
    if ( G == dd->zero )
        assert( 0 );
    if ( G == dd->one )
        assert( 0 );
    if ( C == dd->one )
        assert( 0 );

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddIncremCliques, G, C);
    if (zRes)
        return(zRes);
    else
    {
        int LevelC = cuddIZ(dd,C->index);
        int LevelG = cuddIZ(dd,G->index);
        DdNode *zRes0, *zRes1, *zC0, *zOtherNodes;

        assert ( LevelC >= LevelG );

        if ( LevelC > LevelG )
            zC0 = C;
        else /* if ( LevelC == LevelG ) */
            zC0 = cuddE(C);

        /* compute the set of cliques WITHOUT this variable */
        zRes0 = extraZddIncremCliques( dd, cuddE(G), zC0 );
        if ( zRes0 == NULL )
            return NULL;
        cuddRef( zRes0 );

        /* compute the set of nodes to which this node is connected */
        zOtherNodes = extraZddSinglesToComb( dd, cuddT(G) );
        if ( zOtherNodes == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            return NULL;
        }
        cuddRef( zOtherNodes );

        /* compute the set of cliques WITH this variable */
        zRes1 = extraZddSubSet( dd, zC0, zOtherNodes );
        if ( zRes1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            Cudd_RecursiveDerefZdd( dd, zOtherNodes );
            return NULL;
        }
        cuddRef( zRes1 );
        Cudd_RecursiveDerefZdd( dd, zOtherNodes );

        /* compose Res0 and Res1 with the given ZDD variable */
        zRes = cuddZddGetNode( dd, G->index, zRes1, zRes0 );
        if ( zRes == NULL ) 
        {
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            Cudd_RecursiveDerefZdd( dd, zRes1 );
            return NULL;
        }
        cuddDeref( zRes0 );
        cuddDeref( zRes1 );

        cuddCacheInsert2(dd, extraZddIncremCliques, G, C, zRes);
        return zRes;
    }
} /* end of extraZddIncremCliques */

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

