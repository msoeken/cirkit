/**CFile****************************************************************

  FileName    [extraDdSigma.c]

  PackageName [extra]

  Synopsis    [Computing sigma edges for BDDs and ADDs.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraDdSigma.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

#define SIGMA_FULL               (~((unsigned)0))
#define SIGMA_MASK(n)           ((~((unsigned)0)) >> (32-n))

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

typedef struct Extra_NodeInfo_t_    Extra_NodeInfo_t;
struct Extra_NodeInfo_t_ 
{
    unsigned    uPos;
    unsigned    uNeg;
    DdNode *    bTemp;
};

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

static void extraSigmaPropagateOne( DdManager * dd, DdNode * F, unsigned uSize );


/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Returns the sum of sigma edges on each level above the given level.]

  Description [Sigma 0(1) edge is an edge which points to the terminal node 0(1).
  The sum of edges on a given level is the OR of all sigma edges that originate
  above the given level and cross the given level. If the level is K, then all 
  the nodes that are on level K and below are understood to be below the given 
  level. All the nodes whose BDD level is less than K are above the given level.
  If there are no sigma edges on a certain level, the corresponding entry is set 
  to 0. The size of each array to store the sigma edges should be at least equal 
  to Level. Returns the number of minterms normalized from the 32-var-based space
  into the space whose size is equal to the number of dimensions of the function.
  For each level, only the edges originating on the given level are counted.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_SigmaCountMinterm( DdManager * dd, DdNode * F, unsigned pSigma0[], unsigned pSigma1[] )
{
    Extra_NodeSet_t * p;
    Extra_NodeInfo_t * pPairs, * pPair;
    DdNode * bNode, * bNode0, * bNode1;
    unsigned Total;
    int nSuppSize, i, k;

    // compute the node profile; do not distinguish the node and its complement
    p = Extra_NodeProfile( dd, F, dd->size, 1 );

    // allocate storage for both pos and neg minterm counts for each node
    pPairs = ALLOC( Extra_NodeInfo_t, p->nNodes );
    for ( i = 0, pPair = pPairs; i < p->nNodes; i++, pPair++ )
    {
        // store node->next in the temp field
        bNode = p->pbNodes[0][i];
        pPair->bTemp = bNode->next;
        // use the node->next field to point to the pair
        bNode->next = (DdNode *)pPair;
        // initialize the paths for each node, direct and complemented
        pPair->uNeg = 0;
        pPair->uPos = 0;
    }

    // determine support size
    nSuppSize = 0;
    for ( i = 0; i < dd->size; i++ )
        if ( p->pnNodes[i] )
            nSuppSize++;

    // the code will work correctly only if support is less than 32
    assert( nSuppSize < 32 );

    // set the starting path in the topmost node
    if ( Cudd_IsComplement(F) )
        pPairs[0].uNeg = (1 << nSuppSize); 
    else
        pPairs[0].uPos = (1 << nSuppSize); 

    // walk through the nodes and compute their paths
    for ( i = 0, pPair = pPairs; i < p->nNodes; i++, pPair++ )
    {
        // skip the node if it is on the last level
        bNode = p->pbNodes[0][i];
        if ( cuddIsConstant(bNode) )
            continue;
        // propagate the paths
        if ( pPair->uNeg )
            extraSigmaPropagateOne( dd, Cudd_Not(bNode), pPair->uNeg );
        if ( pPair->uPos )
            extraSigmaPropagateOne( dd, bNode,           pPair->uPos );
    }

    // collect the sum total of sigma paths on each level
    pPair = pPairs;
    Total = 0;
    for ( i = 0; i < dd->size; i++ )
    {
        // clean storage
        pSigma0[i] = pSigma1[i] = 0;
        for ( k = 0; k < p->pnNodes[i]; k++ )
        {
            bNode = p->pbNodes[i][k];
            // consider the cofactors
            bNode0 = cuddE(bNode);
            if ( Cudd_IsConstant(bNode0) )
            {
                if ( bNode0 == b0 )
                {
                    pSigma0[i] += pPair->uPos/2;
                    pSigma1[i] += pPair->uNeg/2;
                }
                if ( bNode0 == b1 )
                {
                    pSigma0[i] += pPair->uNeg/2;
                    pSigma1[i] += pPair->uPos/2;
                }
            }
            bNode1 = cuddT(bNode);
            if ( cuddIsConstant(bNode1) )
            {
                assert( bNode1 == b1 );
                pSigma0[i] += pPair->uNeg/2;
                pSigma1[i] += pPair->uPos/2;
            }
            pPair++;
        }
        Total += pSigma0[i] + pSigma1[i];
    }
    assert( Total == (((unsigned)1) << nSuppSize) );

    // write node->next where it belongs
    for ( i = 0; i < p->nNodes; i++ )
        p->pbNodes[0][i]->next = pPairs[i].bTemp;
    FREE( pPairs );
    Extra_NodeSetDeref( p );
}

/**Function*************************************************************

  Synopsis    [Returns the sum total of sigma-1 edges above certain level.]

  Description [To compute the sum total of sigma-0 edges, use the
  complement of the function.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode * Extra_bddSigma1Total( 
  DdManager * dd,  
  DdNode * bFunc,
  int Level ) 
{
    DdNode * bRes;
    DdNode * bLevel;
    if ( Level < dd->size )
        bLevel = dd->vars[Level];
    else
        bLevel = b1;
    do {
        dd->reordered = 0;
        bRes = extraBddSigma1Total( dd, bFunc, bLevel );
    } while ( dd->reordered == 1 );
    return bRes;

} /* end of Extra_bddSigma1Total */

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the reordering-sensitive step of Extra_bddSigma1Total().]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddSigma1Total( 
  DdManager * dd,  
  DdNode * bFunc,
  DdNode * bLevel) 
{
    DdNode * bFR, * bRes;

    // if this is the sigma-1 edge, return it value
    if ( bFunc == b1 )
        return b1;

    // if we are below the given level, return constant 0
    bFR = Cudd_Regular(bFunc); 
    if ( cuddI(dd, bFR->index) >= bLevel->index )
        return b0;

    if ( bRes = cuddCacheLookup2(dd, extraBddSigma1Total, bFunc, bLevel) )
        return bRes;
    else
    {
        DdNode * bRes0, * bRes1;             
        DdNode * bF0, * bF1;             

        // cofactor the functions
        if ( bFR != bFunc ) // bFunc is complemented 
        {
            bF0 = Cudd_Not( cuddE(bFR) );
            bF1 = Cudd_Not( cuddT(bFR) );
        }
        else
        {
            bF0 = cuddE(bFR);
            bF1 = cuddT(bFR);
        }

        bRes0 = extraBddSigma1Total( dd, bF0, bLevel );
        if ( bRes0 == NULL ) 
            return NULL;
        cuddRef( bRes0 );

        bRes1 = extraBddSigma1Total( dd, bF1, bLevel );
        if ( bRes1 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bRes1 );
        /* only aRes0 and aRes1 are referenced at this point */

        /* consider the case when Res0 and Res1 are the same node */
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        /* consider the case when Res1 is complemented */
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, bFR->index, Cudd_Not(bRes1), Cudd_Not(bRes0));
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref(dd,bRes0);
                Cudd_RecursiveDeref(dd,bRes1);
                return NULL;
            }
            bRes = Cudd_Not(bRes);
        } 
        else 
        {
            bRes = cuddUniqueInter( dd, bFR->index, bRes1, bRes0 );
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref(dd,bRes0);
                Cudd_RecursiveDeref(dd,bRes1);
                return NULL;
            }
        }
        cuddDeref( bRes0 );
        cuddDeref( bRes1 );
            
        /* insert the result into cache */
        cuddCacheInsert2(dd, extraBddSigma1Total, bFunc, bLevel, bRes);
        return bRes;
    }
} /* end of extraBddSigma1Total */

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Dereferences the node path structure.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraSigmaPropagateOne( DdManager * dd, DdNode * F, unsigned uSize )
{
    Extra_NodeInfo_t * pPair;
    DdNode * FR, * F0, * F1, * Cof, * CofR;

    // get the cofactors
    FR = Cudd_Regular(F);
    if ( FR != F )
    {
        F0 = Cudd_Not( cuddE(FR) );
        F1 = Cudd_Not( cuddT(FR) );
    }
    else
    {
        F0 = cuddE(FR);
        F1 = cuddT(FR);
    }

    // process the negative cofactor
    Cof = F0;    CofR = Cudd_Regular(F0);
    // get the pair
    pPair = (Extra_NodeInfo_t *)CofR->next;
    // add the path
    if ( CofR != Cof )
        pPair->uNeg += uSize/2;
    else
        pPair->uPos += uSize/2;

    // process the positive cofactor
    Cof  = F1;    CofR = Cudd_Regular(F1);
    // get the pair
    pPair = (Extra_NodeInfo_t *)CofR->next;
    // add the path
    if ( CofR != Cof )
        pPair->uNeg += uSize/2;
    else
        pPair->uPos += uSize/2;
}


////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


