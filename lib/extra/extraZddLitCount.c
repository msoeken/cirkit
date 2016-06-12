/**CFile****************************************************************

  FileName    [extraZddLitCount.c]

  PackageName [extra]

  Synopsis    [Counting literals in the ZDD representing the cover.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddLitCount.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

typedef struct ni
{
    int level;         /* the level of this variable */
    int counter;       /* the number of combinations that have this variable */
    struct ni *next;   /* the pointer to the next entry in the list of vars */

} nodeinfo;


/*---------------------------------------------------------------------------*/
/* Type declarations                                                         */
/*---------------------------------------------------------------------------*/


/*---------------------------------------------------------------------------*/
/* Variable declarations                                                     */
/*---------------------------------------------------------------------------*/

/* pointer to memory allocated by the memory manager */
static nodeinfo *s_pMemory;
/* the number of links allocated */
static int s_nLinksAlloc;

/* the iterator */
static nodeinfo *s_pLinkedList;
static int s_nLinksReturned;

/* hash table for the nodeinfo structures */
static st_table *s_Table;


/*---------------------------------------------------------------------------*/
/* Macro declarations                                                        */
/*---------------------------------------------------------------------------*/


/**AutomaticStart*************************************************************/

/*---------------------------------------------------------------------------*/
/* Static function prototypes                                                */
/*---------------------------------------------------------------------------*/
static nodeinfo* extraZddLitCount (DdManager * dd, DdNode * Set);
            
/* local memory manager */
static int       localMemManagerStart ();
static nodeinfo* localMemManagerLinkNext ();
static void      localMemManagerDissolve ();

/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/


/**Function*************************************************************

  Synopsis    [Counts the number of literals in one combination.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_zddLitCountComb( DdManager * dd, DdNode * zComb )
{
    int Counter;
    if ( zComb == z0 )
        return 0;
    Counter = 0;
    for ( ; zComb != z1; zComb = cuddT(zComb) )
        Counter++;
    return Counter;
}

/**Function********************************************************************

  Synopsis    [Computes how many times variables occur in combinations of the ZDD.]

  Description [Returns values in the array of integers with as many cells 
               as there are ZDD variables in the manager. The i-th cell 
               of the array contains the number of times the i-th ZDD 
               variable occurs in combinations represented by the ZDD Set.
               It is the user's responsibility to delocate the array 
               using function free().]

  SideEffects []

  SeeAlso     [Cudd_zddSubSet, Cudd_zddSupSet, Cudd_zddNotSupSet]

******************************************************************************/
int *
Extra_zddLitCount(
  DdManager * dd,
  DdNode * Set)
{
    nodeinfo *NodeInfo = NULL;
    int* Result = NULL;

    /* start the local memory manager (and prepare the interator) */
    int MemAlloc = localMemManagerStart(dd,Set);
    if ( MemAlloc == 0 ) 
        goto failure;

    /* start the hash table for nodeinfo structures */
    s_Table = NULL;
    s_Table = st_init_table(st_ptrcmp,st_ptrhash);
    if ( s_Table == NULL ) 
        goto failure;

    /* call the function recursively */
    NodeInfo = extraZddLitCount(dd, Set);
    if ( NodeInfo == NULL ) 
        goto failure;

    /* debugging */
//  printf( "\nExtra_zddLitCount(): memory internally allocated %dK\n", MemAlloc );
//  printf( "Extra_zddLitCount(): the number of entries allocated %d\n", s_nLinksAlloc );
//  printf( "Extra_zddLitCount(): the number of entries used %d\n", s_nLinksReturned );
//  printf( "\nExtra_zddLitCount(): the number of paths is %d\n", NodeInfo->counter );

    /* allocate memory for the return result */
    Result = (int*) malloc( dd->sizeZ * sizeof( int ) );
    if ( Result == NULL )
        goto failure;
    memset( Result, 0, dd->sizeZ * sizeof( int ) );

    /* skip the path counting nodeinfo */
    NodeInfo = NodeInfo->next;
    /* copy information into this array */
    while ( NodeInfo )
    {
        /* write the number of occurences into the array */
        Result[ dd->invpermZ[NodeInfo->level] ] = NodeInfo->counter;
        /* take the next nodeinfo */
        NodeInfo = NodeInfo->next;
    }

    /* delocate hash table and memory for node info structures */
    localMemManagerDissolve();
    st_free_table( s_Table );
    return Result;

failure:
    dd->errorCode = CUDD_MEMORY_OUT;
    if ( MemAlloc ) localMemManagerDissolve();
    if ( s_Table ) st_free_table( s_Table );
    return NULL;

} /* end of Extra_zddLitCount */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis [Performs the recursive step of Cudd_zddSupSet.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
nodeinfo * 
extraZddLitCount( 
    DdManager *dd, 
    DdNode *Set)
{   
    nodeinfo *pNode = NULL;

    /* terminal cases */
    if ( Set == dd->zero || Set == dd->one )
        return NULL;

    /* chech whether this nodeinfo entry is in the hash table */
    if ( st_lookup( s_Table, (char*)Set, (char**)&pNode ) )
        return pNode;
    else 
    { /* this nodeinfo entry does not exist - build it */

        int nPathsE, nPathsT;
        /* to get the number of paths in the branch, 
         * we can look up the counter of the top most entry */
        nodeinfo *pNodeE, *pNodeT, **pp;

        /* solve subproblems */
        pNodeE = extraZddLitCount( dd, cuddE( Set ) );
        if ( pNodeE == NULL ) /* terminal node */
            nPathsE = (int)( cuddE( Set ) == dd->one );
        else
        {
            assert( pNodeE->level == -1 );
            nPathsE = pNodeE->counter;
            pNodeE = pNodeE->next;
        }

        pNodeT = extraZddLitCount( dd, cuddT( Set ) );
        if ( pNodeT == NULL ) /* terminal node */
            nPathsT = (int)( cuddT( Set ) == dd->one );
        else
        {
            assert( pNodeT->level == -1 );
            nPathsT = pNodeT->counter;
            pNodeT = pNodeT->next;
        }

        /* add the path count nodeinfo structure to the list */
        pNode = localMemManagerLinkNext();
        pNode->level = -1;
        pNode->counter = nPathsE + nPathsT;

        /* add the current level nodeinfo structure to the list */
        pNode->next = localMemManagerLinkNext();
        pNode->next->level = dd->permZ[ Set->index ];
        pNode->next->counter = nPathsT;

        /* set the pointer to the tail of the list */
        pp = &(pNode->next->next);

        /* merge two nodeinfo lists */
        while ( pNodeE && pNodeT )
        {
            (*pp) = localMemManagerLinkNext();
            if ( pNodeE->level == pNodeT->level )
            {
                (*pp)->level = pNodeE->level;
                (*pp)->counter = pNodeE->counter + pNodeT->counter;
                pNodeE = pNodeE->next;
                pNodeT = pNodeT->next;
            }
            else if ( pNodeE->level < pNodeT->level )
            {
                (*pp)->level = pNodeE->level;
                (*pp)->counter = pNodeE->counter;
                pNodeE = pNodeE->next;
            }
            else /* if ( pNodeE->level > pNodeT->level ) */
            {
                (*pp)->level = pNodeT->level;
                (*pp)->counter = pNodeT->counter;
                pNodeT = pNodeT->next;
            }
            pp = &((*pp)->next);
        }

        if ( pNodeE || pNodeT )
        {
            if ( pNodeE )
                (*pp) = pNodeE;
            else
                (*pp) = pNodeT;
            /* no need to copy the node list */
        }
        else
            (*pp) = NULL;

        if ( st_add_direct( s_Table, (char*)Set, (char*)pNode ) == ST_OUT_OF_MEM )
            return NULL;

        return pNode;
    }

} /* end of extraZddLitCount */


/**Function********************************************************************

  Synopsis [Allocates memory used locally for node info structures.]

  Description [Returns the number of Kbytes allocated; 0 in case of failure.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static int localMemManagerStart( 
  DdManager * dd,
  DdNode * Set)
{
    int MemSize;
    nodeinfo* pTemp;
    int i;

    /* estimate roughly the number of needed nodeinfo structures */
    /* count the number of nodes and multiply it by the number of variables */
    /* this is a very crude estimation - may want to improve in the future */
    s_nLinksAlloc = Cudd_DagSize( Set ) * Cudd_SupportSize( dd, Set );
    MemSize = s_nLinksAlloc * sizeof( nodeinfo );

    /* allocate memory for these structures as one large chunk */
    s_pMemory = (nodeinfo*) malloc( MemSize );
    if ( s_pMemory == NULL )
        return 0;
    memset( s_pMemory, 0, MemSize );

    /* connect all links into a linked list */
    pTemp = s_pMemory;
    for ( i = 0; i < s_nLinksAlloc-1; i++ )
    {
        pTemp->next = pTemp + 1;
        pTemp = pTemp->next;
    }

    /* prepare the iterator */
    s_pLinkedList = s_pMemory;
    s_nLinksReturned = 0;

    return MemSize/1024 + (MemSize%1024 > 0);
}

/**Function********************************************************************

  Synopsis [Returns the next node info structure.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static nodeinfo* localMemManagerLinkNext( void )
{
    assert( s_nLinksReturned < s_nLinksAlloc );

    s_nLinksReturned++;
    return s_pLinkedList++;
}


/**Function********************************************************************

  Synopsis [Releases the allocated memory.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/

static void localMemManagerDissolve( void )
{
    free( s_pMemory );
}
