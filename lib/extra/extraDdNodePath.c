/**CFile****************************************************************

  FileName    [extraDdNodePaths.c]

  PackageName [extra]

  Synopsis    [Computation of node profiles and node paths in the DD.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraDdNodePaths.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
/*---------------------------------------------------------------------------*/

typedef struct Extra_NodePair_t_    Extra_NodePair_t;
struct Extra_NodePair_t_ 
{
    DdNode *           bPos;     // the sum of positive paths
    DdNode *           bNeg;     // the sum of negative paths
    DdNode *           bTemp;    // temporary storage for node->next
    Extra_NodePair_t * pPairF0;  // the else cofactor pair
    Extra_NodePair_t * pPairF1;  // the then cofactor pair
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

// accessing the "direct" mark
#define Extra_RegularD(node)      ((DdNode *)((unsigned long)(node) & ~01))
#define Extra_ComplementD(node)   ((DdNode *)((unsigned long)(node) | 01))
#define Extra_IsComplementD(node) ((int) ((long) (node) & 01))

// accessing the "complement" mark
#define Extra_RegularC(node)      ((DdNode *)((unsigned long)(node) & ~02))
#define Extra_ComplementC(node)   ((DdNode *)((unsigned long)(node) | 02))
#define Extra_IsComplementC(node) ((int) ((long) (node) & 02))

// accessing both marks
#define Extra_Regular(node)       ((DdNode *)((unsigned long)(node) & ~03))
#define Extra_Complement(node)    ((DdNode *)((unsigned long)(node) | 03))
#define Extra_IsComplement(node)  ((int) ((long) (node) & 03))

/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/

static int  extraWidthAtLevel( DdManager * dd, DdNode * F, int Level );
static void extraClearFlags( DdManager * dd, DdNode * F );
static void extraClearFlagsCollectCofsAtLevel( DdManager * dd, DdNode * FR, int Level, DdNode * pbCofs[], int * piNode );
static int  extraNodeProfileCount( Extra_NodeSet_t * p, DdNode * F, int Level );
static int  extraNodeProfileCollect( Extra_NodeSet_t * p, DdNode * F, int Level );
static void extraNodePathPropagateOne( DdManager * dd, DdNode * F, DdNode * bPath, Extra_NodePair_t * pPair );

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Returns the number of different cofactors under the cut.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_WidthAtLevel( DdManager * dd, DdNode * Func, int Level )
{
    int nNodes;
    nNodes = extraWidthAtLevel( dd, Func, Level );
    extraClearFlags( dd, Cudd_Regular(Func) );
    return nNodes;
}

/**Function*************************************************************

  Synopsis    [Returns the set of different cofactors under the cut.]

  Description [The last argument is the array, where the cofactors are placed.
  To be on the safe side, if the number of cofactors is not known, the size
  of this array should be made equal to the number of all nodes in the DD.
  The number of nodes actually found and placed in the array is the return
  value of this procedure.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_CofactorsAtLevel( DdManager * dd, DdNode * Func, int Level, DdNode * pbCofs[] )
{
    int nNodes, iNode;
    nNodes = extraWidthAtLevel( dd, Func, Level );
    iNode = 0;
    extraClearFlagsCollectCofsAtLevel( dd, Cudd_Regular(Func), Level, pbCofs, &iNode );
    assert( iNode == nNodes );
    return nNodes;
}

/**Function*************************************************************

  Synopsis    [Returns the number of nodes on each level.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Extra_NodeSet_t * Extra_NodeProfile( DdManager * dd, DdNode * Func, int Level, int fCollect )
{
    Extra_NodeSet_t * p;
    int nNodes, i;
    // allocate the structure
    p = ALLOC( Extra_NodeSet_t, 1 );
    memset( p, 0, sizeof(Extra_NodeSet_t) );
    p->dd    = dd;
    p->Level = Level;
    p->pnNodes = ALLOC( int, Level + 1 );
    memset( p->pnNodes, 0, sizeof(int) * (Level + 1) );
    // counting the number of nodes on each level in the BDD
    // it also counts the number of nodes pointed to under the cut
    p->nNodes = extraNodeProfileCount( p, Cudd_Regular(Func), Level );
    if ( fCollect )
    {
        // allocate storage for the nodes
        p->pbNodes = ALLOC( DdNode **, Level + 1 );
        p->pbNodes[0] = ALLOC( DdNode *, p->nNodes );
        // set the pointers to the nodes on each level
        for ( i = 1; i <= Level; i++ )
            p->pbNodes[i] = p->pbNodes[i-1] + p->pnNodes[i-1];
        // allocate room of the other copy of the counters
        p->pnNodesCopy = ALLOC( int, Level + 1 );
        memset( p->pnNodesCopy, 0, sizeof(int) * (Level + 1) );
        // removes the marks and collects the nodes
        nNodes = extraNodeProfileCollect( p, Cudd_Regular(Func), Level );
        assert( nNodes == p->nNodes );
    }
    else
        extraClearFlags( dd, Cudd_Regular(Func) );
    return p;
}

/**Function*************************************************************

  Synopsis    [Computes the paths in a DD leading to all the nodes above the cut.]

  Description [If this is a BDD and, on some level, both node and its
  complement are present, both paths will be computed independently.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
Extra_NodeSet_t * Extra_NodePaths( DdManager * dd, DdNode * Func, int Level )
{
    Extra_NodeSet_t * p;
    Extra_NodePair_t * pPairs, * pPair;
    DdNode * bNode;
    int nPaths, i, k;

    // compute the node profile; do not distinguish the node and its complement
    p = Extra_NodeProfile( dd, Func, Level, 1 );

    // allocate storage for both pos and neg paths of each node
    pPairs = ALLOC( Extra_NodePair_t, p->nNodes );
    for ( i = 0, pPair = pPairs; i < p->nNodes; i++, pPair++ )
    {
        bNode = p->pbNodes[0][i];
        // store node->next in the temp field
        pPair->bTemp = bNode->next;
        // use the node->next field to point to the pair
        bNode->next = (DdNode *)pPair;
        // initialize the paths for each node, direct and complemented
        pPair->bNeg = b0;  Cudd_Ref( b0 );
        pPair->bPos = b0;  Cudd_Ref( b0 );
    }

    // set the pointers to the pairs of each cofactor of each node
    // and return the correct value to node->next
    for ( i = 0, pPair = pPairs; i < p->nNodes; i++, pPair++ )
    {
        bNode = p->pbNodes[0][i];
        if ( cuddI(dd, bNode->index) >= Level )
        {
            pPair->pPairF0 = NULL;
            pPair->pPairF1 = NULL;
        }
        else
        {
            pPair->pPairF0 = (Extra_NodePair_t *)Cudd_Regular( cuddE(bNode) )->next;
            pPair->pPairF1 = (Extra_NodePair_t *)cuddT(bNode)->next;
        }
        bNode->next = pPair->bTemp;
    }

    // set the starting path in the topmost node
    if ( Cudd_IsComplement(Func) )
        pPairs[0].bNeg = b1; 
    else
        pPairs[0].bPos = b1; 

    // walk through the nodes and compute their paths
    for ( i = 0, pPair = pPairs; i < p->nNodes; i++, pPair++ )
    {
        // skip the node if it is on the last level
        bNode = p->pbNodes[0][i];
        if ( cuddI(dd, bNode->index) >= Level )
            continue;
        // propagate the paths
        if ( pPair->bNeg != b0 )
            extraNodePathPropagateOne( dd, Cudd_Not(bNode), pPair->bNeg, pPair );
        if ( pPair->bPos != b0 )
            extraNodePathPropagateOne( dd, bNode,           pPair->bPos, pPair );
    }

    // count the number of paths on each level
    pPair = pPairs;
    p->nPaths = 0;
    p->pnPaths = ALLOC( int, Level + 1 );
    memset( p->pnPaths, 0, sizeof(int) * (Level + 1) );
    for ( i = 0; i <= Level; i++ )
    {
        for ( k = 0; k < p->pnNodes[i]; k++ )
        {
            // count the number of paths 
            p->pnPaths[i] += (pPair->bNeg != b0);
            p->pnPaths[i] += (pPair->bPos != b0);  
            pPair++;
        }
        p->nPaths += p->pnPaths[i];
    }

    // allocate the storage for paths
    p->pbPaths = ALLOC( DdNode **, Level + 1 );
    p->pbPaths[0] = ALLOC( DdNode *, p->nPaths );
    // set the pointers to the nodes on each level
    for ( i = 1; i <= Level; i++ )
        p->pbPaths[i] = p->pbPaths[i-1] + p->pnPaths[i-1];

    // allocate the storage for cofactors
    p->pbCofs = ALLOC( DdNode **, Level + 1 );
    p->pbCofs[0] = ALLOC( DdNode *, p->nPaths );
    // set the pointers to the nodes on each level
    for ( i = 1; i <= Level; i++ )
        p->pbCofs[i] = p->pbCofs[i-1] + p->pnPaths[i-1];

    // allocate room of the other copy of the counters
    p->pnPathsCopy = ALLOC( int, Level + 1 );
    memset( p->pnPathsCopy, 0, sizeof(int) * (Level + 1) );

    // collects the paths
    pPair = pPairs;
    nPaths = 0;
    for ( i = 0; i <= Level; i++ )
    {
        for ( k = 0; k < p->pnNodes[i]; k++ )
        {
            // count the number of paths 
            if ( pPair->bNeg != b0 )
            {
                p->pbPaths[i][ p->pnPathsCopy[i] ] = pPair->bNeg; // takes ref
                p->pbCofs[i] [ p->pnPathsCopy[i] ] = Cudd_Not(p->pbNodes[i][k]);
                p->pnPathsCopy[i]++;
            }
            else
                Cudd_Deref( b0 );

            if ( pPair->bPos != b0 )
            {
                p->pbPaths[i][ p->pnPathsCopy[i] ] = pPair->bPos; // takes ref
                p->pbCofs[i][ p->pnPathsCopy[i] ] = p->pbNodes[i][k];
                p->pnPathsCopy[i]++;
            }
            else
                Cudd_Deref( b0 );

            pPair++;
        }
        nPaths += p->pnPathsCopy[i];
    }
    assert( nPaths == p->nPaths );
    FREE( pPairs );

    return p;
}


/**Function*************************************************************

  Synopsis    [Dereferences the node path structure.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_NodeSetDeref( Extra_NodeSet_t * p )
{
    int i;
    if ( p->pbPaths )
    {
        for ( i = 0; i < p->nPaths; i++ )
            Cudd_RecursiveDeref( p->dd, p->pbPaths[0][i] );
    }
    if ( p->pbPaths )
    {
        FREE( p->pbPaths[0] );
        FREE( p->pbPaths );
    }
    if ( p->pbCofs )
    {
        FREE( p->pbCofs[0] );
        FREE( p->pbCofs );
    }
    if ( p->pbNodes )
    {
        FREE( p->pbNodes[0] );
        FREE( p->pbNodes );
    }
    FREE( p->pnNodes );
    FREE( p->pnNodesCopy );
    FREE( p->pnPaths );
    FREE( p->pnPathsCopy );
    FREE( p );
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_NodeWidthAtLevel.]

  Description [Returns the number of cofactors under the cut that 
  have not been visited before. The visited cofactors are marked by 
  setting their marks. Mark "D" is set if the cofactor is visited
  in the direct polarity. Mark "C" is set if the cofactor is visited
  in the complemented polarity.]

  SideEffects [None]

  SeeAlso     [extraClearFlags]

******************************************************************************/
int extraWidthAtLevel( DdManager * dd, DdNode * F, int Level )
{
    DdNode * FR, * F0, * F1;

    // check if the node in this polarity is already visited
    FR = Cudd_Regular(F);
    if ( FR != F )
    {
        if ( Extra_IsComplementC(FR->next) ) // already visited
            return 0;
        FR->next = Extra_ComplementC(FR->next); // not visited before
    }
    else
    {
        if ( Extra_IsComplementD(FR->next) ) // already visited
            return 0;
        FR->next = Extra_ComplementD(FR->next); // not visited before
    }

    // check if the node is under the level
    if ( cuddI(dd, FR->index) >= Level )
        return 1;

    // cofactor the node
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

    // visit the nodes cofactors
    return extraWidthAtLevel( dd, F0, Level ) +
           extraWidthAtLevel( dd, F1, Level );
} /* end of extraWidthAtLevel */


/**Function********************************************************************

  Synopsis    [Removes both types of marks on the pointer node->next.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
void extraClearFlags( DdManager * dd, DdNode * FR )
{
    if ( !Extra_IsComplement(FR->next) )
        return;
    // clear the flags
    FR->next = Extra_Regular(FR->next);
    if ( cuddIsConstant(FR) )
        return;
    extraClearFlags( dd, cuddT(FR) );
    extraClearFlags( dd, Cudd_Regular(cuddE(FR)) );
} /* end of extraClearFlags */


/**Function********************************************************************

  Synopsis    [Clears the marks and collects the nodes under the cut.]

  Description [If some node has been visited with complemented and
  non-complemented polarities, this procedure collects it as two different
  nodes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
void extraClearFlagsCollectCofsAtLevel( DdManager * dd, DdNode * FR, int Level, DdNode * pbCofs[], int * piNode )
{
    if ( !Extra_IsComplement(FR->next) )
        return;
    // check if the node is under the level
    if ( cuddI(dd, FR->index) >= Level )
    {
        // write the nodes
        if ( Extra_IsComplementD(FR->next) )
            pbCofs[ (*piNode)++ ] = FR;
        if ( Extra_IsComplementC(FR->next) )
            pbCofs[ (*piNode)++ ] = Cudd_Not(FR);
    }
    // clear the flags
    FR->next = Extra_Regular(FR->next);
    if ( cuddIsConstant(FR) )
        return;
    extraClearFlagsCollectCofsAtLevel( dd, cuddT(FR), Level, pbCofs, piNode );
    extraClearFlagsCollectCofsAtLevel( dd, Cudd_Regular(cuddE(FR)), Level, pbCofs, piNode );
} /* end of extraClearFlagsCollectCofsAtLevel */


/**Function*************************************************************

  Synopsis    [Dereferences the node path structure.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraNodePathPropagateOne( DdManager * dd, DdNode * F, DdNode * bPathInit, Extra_NodePair_t * pPairF )
{
    Extra_NodePair_t * pPair;
    DdNode * FR, * F0, * F1;
    DdNode * bPath, * bTemp;

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
    // get the pair
    pPair = pPairF->pPairF0;
    // get the path
    bPath = Cudd_bddAnd( dd, bPathInit, Cudd_Not(dd->vars[FR->index]) );  Cudd_Ref( bPath );
    // add the path
    if ( Cudd_IsComplement(F0) )
    {
        pPair->bNeg = Cudd_bddOr( dd, bTemp = pPair->bNeg, bPath ); Cudd_Ref( pPair->bNeg );
        Cudd_RecursiveDeref( dd, bTemp );                
    }
    else
    {
        pPair->bPos = Cudd_bddOr( dd, bTemp = pPair->bPos, bPath ); Cudd_Ref( pPair->bPos );
        Cudd_RecursiveDeref( dd, bTemp );                
    }
    Cudd_RecursiveDeref( dd, bPath );                

    // process the positive cofactor
    // get the pair
    pPair = pPairF->pPairF1;
    // get the path
    bPath = Cudd_bddAnd( dd, bPathInit, dd->vars[FR->index] );  Cudd_Ref( bPath );
    // add the path
    if ( Cudd_IsComplement(F1) )
    {
        pPair->bNeg = Cudd_bddOr( dd, bTemp = pPair->bNeg, bPath ); Cudd_Ref( pPair->bNeg );
        Cudd_RecursiveDeref( dd, bTemp );                
    }
    else
    {
        pPair->bPos = Cudd_bddOr( dd, bTemp = pPair->bPos, bPath ); Cudd_Ref( pPair->bPos );
        Cudd_RecursiveDeref( dd, bTemp );                
    }
    Cudd_RecursiveDeref( dd, bPath );                
}


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_NodeProfile.]

  Description [Counts the number of nodes on each level in the BDD.
  Each node is counted once even if it has both direct and complemented
  edges pointing to it. Marks the visited nodes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int extraNodeProfileCount( Extra_NodeSet_t * p, DdNode * F, int LevelMax )
{
    int LevelCur;
    //. if this node is visited skip it
    if ( Cudd_IsComplement(F->next))
        return 0;
    // if this node is not visited, mark it
    F->next = Cudd_Not(F->next);
    // check if the node is below the cut
    LevelCur = cuddI(p->dd, F->index);
    if ( LevelCur >= LevelMax )
    {
        p->pnNodes[LevelMax]++;
        return 1;
    }
    // count this node
    p->pnNodes[LevelCur]++;
    return extraNodeProfileCount( p, Cudd_Regular(cuddE(F)), LevelMax ) +
           extraNodeProfileCount( p, cuddT(F), LevelMax ) + 1;           
} /* end of extraNodeProfileCount */


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_NodeProfile.]

  Description [Collects the nodes on each level in the BDD.
  Unmarks the visited nodes.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int extraNodeProfileCollect( Extra_NodeSet_t * p, DdNode * F, int LevelMax )
{
    int LevelCur;
    //. if this node is visited skip it
    if ( !Cudd_IsComplement(F->next))
        return 0;
    // if this node is not visited, unmark it
    F->next = Cudd_Regular(F->next);
    // check if the node is below the cut
    LevelCur = cuddI(p->dd, F->index);
    if ( LevelCur >= LevelMax )
    {
        p->pbNodes[LevelMax][ p->pnNodesCopy[LevelMax]++ ] = F;
        return 1;
    }
    // collect this node
    p->pbNodes[LevelCur][ p->pnNodesCopy[LevelCur]++ ] = F;
    return extraNodeProfileCollect( p, Cudd_Regular(cuddE(F)), LevelMax ) + 
           extraNodeProfileCollect( p, cuddT(F), LevelMax ) + 1;
} /* end of extraNodeProfileCollect */




////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


