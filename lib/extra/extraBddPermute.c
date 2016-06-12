/**CFile****************************************************************

  FileName    [extraBddPermute.c]

  PackageName [extra]

  Synopsis    [Fast variable permuation in the BDD.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraBddPermute.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Stucture declarations                                                     */
/*---------------------------------------------------------------------------*/

struct Extra_PermMan_t_
{
    // parameters given by the user
    DdManager * dd;              // the CUDD BDD manager
    int *       pPermute;        // the pointer to the permutation array given by the user
    int *       pSupport;        // the temporary array to store the permutation
    int *       pArray;          // the temporary array to store the permutation
    int         nArrayAlloc;     // the allocated size of the array

    // the hash table
    int *       pHashSign;       // the signature
    DdNode **   pHashFunc;       // the table itself
    DdNode **   pHashRes;        // the table itself
    int         nTableSize;      // the size of the hash table
    int         Signature;       // the signature counter
    int         fResizing;       // this flag is 1, when the manager is resizing;
    
    // the referenced node list
    DdNode **   pRefNodes;
    int         nRefNodes;

    // the key increase limit
    int         KeyLimit;        // 0 if not set
    int         KeysPrev;        
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
static DdNode * Extra_Permute_rec( Extra_PermMan_t * pMan, DdNode * Func );
//static void Experiment7( BFunc * pFunc );
 
/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Initializes the permutation manager.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
Extra_PermMan_t * Extra_PermutationManagerInit()
{
    Extra_PermMan_t * p;

    p = ALLOC( Extra_PermMan_t, 1 );
    memset( p, 0, sizeof(Extra_PermMan_t) );

    p->nTableSize      = Cudd_Prime( 1000 );
    p->pHashSign       = CALLOC( int,     p->nTableSize );
    p->pHashFunc       = ALLOC( DdNode *, p->nTableSize );
    p->pHashRes        = ALLOC( DdNode *, p->nTableSize );
    p->pRefNodes       = ALLOC( DdNode *, p->nTableSize );

    p->nArrayAlloc     = 1000;
    p->pArray          = ALLOC( int, p->nArrayAlloc );
    p->pSupport        = ALLOC( int, p->nArrayAlloc );

    p->KeyLimit        = 0;
    return p;
}

/**Function*************************************************************

  Synopsis    [Disposes of the permutation manager.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_PermutationManagerQuit( Extra_PermMan_t * p )
{
    free( p->pHashSign );
    free( p->pHashFunc );
    free( p->pHashRes );
    free( p->pRefNodes );
    free( p->pArray );
    free( p->pSupport );
    free( p );
}


/**Function*************************************************************

  Synopsis    [Sets the key increase limit.]

  Description [When the permutation procedure creates more nodes
  than allowed by the key increase limit, it returns NULL.]

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_PermutationManagerKeyIncreaseLimitSet( Extra_PermMan_t * p, int Limit )
{
    p->KeyLimit = Limit;
}

/**Function*************************************************************

  Synopsis    [Resets the key increase limit.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_PermutationManagerKeyIncreaseLimitReset( Extra_PermMan_t * p )
{
    p->KeyLimit = 0;
}

/**Function*************************************************************

  Synopsis    [Permute the given function using the permutation manager.]

  Description [The array pPermute is specified similarly to how is it specified
  for the function Cudd_bddPermute().]

  SideEffects []

  SeeAlso     [Cudd_bddPermute Cudd_addPermute]

***********************************************************************/
DdNode * Extra_Permute( Extra_PermMan_t * pMan, DdManager * dd, DdNode * Func, int * pPermute )
{
    DdNode * Res;
    int i;

    // write the initial data
    pMan->dd       = dd;
    pMan->pPermute = pPermute;

    // prepare temporary data
    pMan->nRefNodes = 0;
    pMan->Signature++;

    // perform the permutation
    if ( (Res = Extra_Permute_rec( pMan, Func )) == NULL )
    { // resizing of the tables is needed
//      printf( "Resizing!\n" );
        if ( pMan->fResizing )
        {
            // deref the temporary nodes
            for ( i = 0; i < pMan->nRefNodes; i++ )
                Cudd_RecursiveDeref( dd, pMan->pRefNodes[i] );
            pMan->nRefNodes = 0;
            
            // resize the tables
            pMan->nTableSize      = Cudd_Prime( 4*Cudd_DagSize(Func) );
            free( pMan->pHashSign );
            free( pMan->pHashFunc );
            free( pMan->pHashRes );
            free( pMan->pRefNodes );
            pMan->pHashSign       = CALLOC( int,     pMan->nTableSize );
            pMan->pHashFunc       = ALLOC( DdNode *, pMan->nTableSize );
            pMan->pHashRes        = ALLOC( DdNode *, pMan->nTableSize );
            pMan->pRefNodes       = ALLOC( DdNode *, pMan->nTableSize );

            // permute again - this time it will work
            Res = Extra_Permute_rec( pMan, Func );
            assert( Res != NULL );
        }
    }
    Cudd_Ref( Res );

    // deref the temporary nodes
    for ( i = 0; i < pMan->nRefNodes; i++ )
        Cudd_RecursiveDeref( dd, pMan->pRefNodes[i] );
    pMan->nRefNodes = 0;

    Cudd_Deref( Res );
    return Res;
}

/**Function*************************************************************

  Synopsis    [Remaps the given function into the topmost variables of the BDD manager.]

  Description []

  SideEffects []

  SeeAlso     [Cudd_bddPermute Extra_bddRemapUp]

***********************************************************************/
DdNode * Extra_RemapUp( Extra_PermMan_t * pMan, DdManager * dd, DdNode * Func )
{
    int Counter, lev;

    if ( pMan->nArrayAlloc < ddMax(dd->size,dd->sizeZ) )
    {
        pMan->nArrayAlloc = ddMax(dd->size,dd->sizeZ);
        free( pMan->pArray );
        free( pMan->pSupport );
        pMan->pArray   = ALLOC( int, pMan->nArrayAlloc );
        pMan->pSupport = ALLOC( int, pMan->nArrayAlloc );
    }

    // get support
    Extra_SupportArray( dd, Func, pMan->pSupport );

    // create the variable map
    // to remap the DD into the upper part of the manager
    Counter = 0;
    // walk through the levels
    for ( lev = 0; lev < dd->size; lev++ )
        if ( pMan->pSupport[ dd->invperm[lev] ] )
            pMan->pArray[ dd->invperm[lev] ] = dd->invperm[Counter++];

    // write the initial data
    pMan->dd       = dd;
    pMan->pPermute = pMan->pArray;
    // return the permuted function
    return Extra_Permute( pMan, dd, Func, pMan->pPermute );
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Permute the given function using the permutation manager.]

  Description [Can return NULL, if the size of the arrays is not enough.]

  SideEffects []

  SeeAlso     []

***********************************************************************/
DdNode * Extra_Permute_rec( Extra_PermMan_t * pMan, DdNode * Func )
{
    DdNode * F, * Res0, * Res1, * Res;
    int HKey = -1, fComp;
    
    fComp = Cudd_IsComplement(Func);
    F = Cudd_Regular(Func);
    if ( cuddIsConstant(F) )
        return Func;

    // check the hash-table
    if ( F->ref != 1 )
    {
        // search cache - use linear probing
        for ( HKey = hashKey2(pMan->Signature,F,pMan->nTableSize); pMan->pHashSign[HKey] == pMan->Signature; HKey = (HKey+1) % pMan->nTableSize )
            if ( pMan->pHashFunc[HKey] == F )
                return Cudd_NotCond( pMan->pHashRes[HKey], fComp );
    }
    // the entry in not found in the cache - compute it
    Res0 = Extra_Permute_rec( pMan, cuddE(F) );  
    if ( Res0 == NULL )
        return NULL;
    Cudd_Ref( Res0 );
    Res1 = Extra_Permute_rec( pMan, cuddT(F) );  
    if ( Res1 == NULL )
    {
        Cudd_RecursiveDeref( pMan->dd, Res0 );
        return NULL;
    }
    Cudd_Ref( Res1 );

    // derive the result using ITE
    Res = Cudd_bddIte( pMan->dd, pMan->dd->vars[ pMan->pPermute[F->index] ], Res1, Res0 ); Cudd_Ref( Res );
    Cudd_RecursiveDeref( pMan->dd, Res0 );
    Cudd_RecursiveDeref( pMan->dd, Res1 );

    // add to the hash table
    if ( HKey != -1 )
    {
        // the next free entry is already found - it is pointed to by HKey
        // while we traversed the diagram, the hash entry to which HKey points,
        // might have been used. Make sure that its signature is different.
        for ( ; pMan->pHashSign[HKey] == pMan->Signature; HKey = (HKey+1) % pMan->nTableSize );
        pMan->pHashSign[HKey] = pMan->Signature;
        pMan->pHashFunc[HKey] = F;
        pMan->pHashRes[HKey]  = Res;
        // add to the ref node list (takes ref)
        pMan->pRefNodes[pMan->nRefNodes++] = Res;  
        // perform resizing of the hash table if the number of nodes is too large
        if ( pMan->nRefNodes > pMan->nTableSize/2 )
        {
            pMan->fResizing = 1;
            return NULL;
        }
    }
    else
        Cudd_Deref( Res );
    return Cudd_NotCond( Res, fComp );
}

#if 0

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Experiment7( BFunc * pFunc )
{
    DdManager * dd = pFunc->dd;
//  dsdman * dsdMan;        // the DSD manager
    Extra_PermMan_t * permMan;
    int CubeLimit   = 10000;
    int fShared     = 1;
    int fDeriveSop  = 1;
    int fUseNands   = 0;
    int fVerbose    = 0;
    int fShortNames = 1;
    DdNode * bFuncPerm;
    DdNode * bFuncPerm2;
    int pPermute[1000];
    int i, o;

//  dsdMan = dsdStart( dd, dd->size, CubeLimit, fShared, fDeriveSop, fUseNands, fVerbose );
//  dsdDecompose( dsdMan, pFunc->pOutputs, pFunc->nOutputs );
//  dsdTreePrint( stdout, dsdMan, pFunc->pInputNames, pFunc->pOutputNames, fShortNames );
//  dsdStop( dsdMan );

    for ( i = 0; i < dd->size; i++ )
        pPermute[i] = dd->size-1-i;

    permMan = Extra_PermutationManagerInit();
    for ( o = 0; o < pFunc->nOutputs; o++ )
    {
        bFuncPerm = Extra_Permute( permMan, dd, pFunc->pOutputs[o], pPermute );   Cudd_Ref( bFuncPerm );

        printf( "nodes before = %d.\n", Cudd_DagSize(pFunc->pOutputs[o]) );
        printf( "nodes after  = %d.\n", Cudd_DagSize(bFuncPerm) );

        bFuncPerm2 = Cudd_bddPermute( dd, pFunc->pOutputs[o], pPermute );   Cudd_Ref( bFuncPerm );
        assert( bFuncPerm == bFuncPerm2 );

        Cudd_RecursiveDeref( dd, bFuncPerm );
        Cudd_RecursiveDeref( dd, bFuncPerm2 );
    }
    Extra_PermutationManagerQuit( permMan );
}

#endif