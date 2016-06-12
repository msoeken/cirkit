/**CFile****************************************************************

  FileName    [extraDdUtils.c]

  PackageName [extra]

  Synopsis    [Various DD manipulating procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraDdUtils.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                    */
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

static void extraCollectNodes( DdNode * Func, st_table * tNodes );
static void extraCollectNodesUnderCut( DdManager * dd, DdNode * Func, int Level, st_table * tNodes, st_table * tVisited );

/**AutomaticEnd***************************************************************/

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Verifies whether the cover belongs to the specified range.]

  Description [Returns 1 if verification succeeds; 0 otherwise.]

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
int
Extra_zddVerifyCover(
  DdManager * dd,
  DdNode * zC,
  DdNode * bFuncOn,
  DdNode * bFuncOnDc)
{
    DdNode * bCover;
    int fVerificationOkay = 1;

    bCover = Extra_zddConvertToBdd( dd, zC );
    Cudd_Ref( bCover );

    if ( Cudd_bddIteConstant( dd, bFuncOn, bCover, dd->one ) != dd->one )
    {
        fVerificationOkay = 0;
        printf(" - Verification not okey: Solution does not cover the on-set\n");
    }
    if ( Cudd_bddIteConstant( dd, bCover, bFuncOnDc, dd->one ) != dd->one )
    {
        fVerificationOkay = 0;
        printf(" - Verification not okey: Solution overlaps with the off-set\n");
    }
    if ( fVerificationOkay )
        printf(" +\n");

    Cudd_RecursiveDeref( dd, bCover );
    return fVerificationOkay;
} /* end of Extra_zddVerifyCover */


/**Function********************************************************************

  Synopsis    [Verify that the function falls within the given range.]

  Description [Returns 1 if the function falls within the specified range. bLower is the on-set.
  bUpper is the complement of the off-set.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_bddVerifyRange( DdManager * dd, DdNode * bFunc, DdNode * bLower, DdNode * bUpper )
{
    int fVerificationOkey = 1;
    
    // make sure that the function belongs to the range [bLower, bUpper]
    if ( !Cudd_bddLeq( dd, bLower, bFunc ) )
    {
        fVerificationOkey = 0;
        printf( "\nVerification not okay: Function does not cover the on-set\n" );
    }
    if ( !Cudd_bddLeq( dd, bFunc, bUpper ) )
    {
        fVerificationOkey = 0;
        printf( "\nVerification not okay: Function overlaps with the off-set\n" );
    }

    return fVerificationOkey;
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_StopManager( DdManager * dd )
{
    int RetValue;
    // check for remaining references in the package
    RetValue = Cudd_CheckZeroRef( dd );
    if ( RetValue > 0 )
        printf( "\nThe number of referenced nodes = %d\n\n", RetValue );
//  Cudd_PrintInfo( dd, stdout );
    Cudd_Quit( dd );
}

/**Function*************************************************************

  Synopsis    [Collects all the nodes of one DD into the table.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
st_table * Extra_CollectNodes( DdNode * Func )
{
    st_table * tNodes;
    tNodes = st_init_table( st_ptrcmp, st_ptrhash );
    extraCollectNodes( Func, tNodes );
    return tNodes;
}

/**Function*************************************************************

  Synopsis    [Collects all the nodes of one DD into the table.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
st_table * Extra_CollectNodesUnderCut( DdManager * dd, DdNode * Func, int Level )
{
    st_table * tNodes;
    st_table * tVisited;
    tNodes = st_init_table( st_ptrcmp, st_ptrhash );
    tVisited = st_init_table( st_ptrcmp, st_ptrhash );
    extraCollectNodesUnderCut( dd, Func, Level, tNodes, tVisited );
    st_free_table( tVisited );
    return tNodes;
}

/**Function*************************************************************

  Synopsis    [Collects all the nodes of the shared DD into the table.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
st_table * Extra_CollectNodesSharing( DdNode * Funcs[], int nFuncs )
{
    st_table * tNodes;
    int i;
    tNodes = st_init_table( st_ptrcmp, st_ptrhash );
    for ( i = 0; i < nFuncs; i++ )
        extraCollectNodes( Funcs[i], tNodes );
    return tNodes;
}


/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function*************************************************************

  Synopsis    [Collects all the BDD nodes into the table.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraCollectNodes( DdNode * Func, st_table * tNodes )
{
    DdNode * FuncR;
    FuncR = Cudd_Regular(Func);
    if ( st_find_or_add( tNodes, (char*)FuncR, NULL ) )
        return;
    if ( cuddIsConstant(FuncR) )
        return;
    extraCollectNodes( cuddE(FuncR), tNodes );
    extraCollectNodes( cuddT(FuncR), tNodes );
}

/**Function*************************************************************

  Synopsis    [Collects all the BDD nodes into the table.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
void extraCollectNodesUnderCut( DdManager * dd, DdNode * Func, int Level, st_table * tNodes, st_table * tVisited )
{
    DdNode * FuncR;
    FuncR = Cudd_Regular(Func);
    if ( st_find_or_add( tVisited, (char*)FuncR, NULL ) )
        return;
    if ( cuddI(dd,FuncR->index) >= Level )
    {
        st_insert( tNodes, (char*)FuncR, NULL );
        return;
    }
    extraCollectNodesUnderCut( dd, cuddE(FuncR), Level, tNodes, tVisited );
    extraCollectNodesUnderCut( dd, cuddT(FuncR), Level, tNodes, tVisited );
}

