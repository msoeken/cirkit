/**CFile****************************************************************

  FileName    [extraBddDistance.c]

  PackageName [extra]

  Synopsis    [Various reusable software utilities.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraBddDistance.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

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

  Synopsis [Returns the distance decomposition of the BDD.]

  Description [The distance decomposition is the decomposition into a set
  of Boolean functions such that each pair of functions in the set has Humming 
  distance > 1 for all on-set minterms. The value returned is the number of 
  functions in the set. If the value returned is 1, it means that there is no 
  distance-based decomposition (in this case there are no refed entries in pbRes). 
  Otherwise, the number returned is the number of decomposed components. 
  The *referenced* components are returned in the array pbRes given by the user.
  The user also gives the limit (resLimit) on the number of components returned.
  If the value returned by the function is equal to the limit, the initial
  function may have not been completely decomposed!]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_bddDistanceDecompose( DdManager * dd, DdNode * bFunc, DdNode * pbRes[], int resLimit )
{
    DdNode * bFuncCur, * bStart, * bTemp, * bComp;
    int Counter;

    bFuncCur = bFunc;   Cudd_Ref( bFuncCur );
    Counter = 0;
    while ( bFuncCur != b0 && Counter < resLimit )
    {
        // find the starting point of the next group
        bStart = Extra_bddFindOneCube( dd, bFuncCur );             Cudd_Ref( bStart );
        // find the next distance-decomposable component
        bComp  = Extra_bddDistance1Iter( dd, bFuncCur, bStart );   Cudd_Ref( bComp ); 
        Cudd_RecursiveDeref( dd, bStart );
        // add this component to the array
        pbRes[ Counter++ ] = bComp;
        // substract this component from the function
        bFuncCur = Cudd_bddAnd( dd, bTemp = bFuncCur, Cudd_Not(bComp) ); Cudd_Ref( bFuncCur );
        Cudd_RecursiveDeref( dd, bTemp );
    }
    Cudd_RecursiveDeref( dd, bFuncCur );

    // in case there is no decomposition, deref the entry in the array
    if ( Counter == 1 )
    {
        Cudd_RecursiveDeref( dd, pbRes[0] );
        pbRes[0] = NULL;
    }
    return Counter;
}

/**Function********************************************************************

  Synopsis [Returns one components that is connected with the cube.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddDistance1Iter( DdManager * dd, DdNode * bFunc, DdNode * bCube )
{
    DdNode * bRes, * bResNext;

    bRes = bCube;   Cudd_Ref( bRes );
    while ( 1 ) 
    {
        bResNext = Extra_bddDistance1( dd, bFunc, bRes );   Cudd_Ref( bResNext );
        if ( bResNext == bRes ) // we came to the fixed point
        {
            Cudd_RecursiveDeref( dd, bResNext );
            break;
        }
        Cudd_RecursiveDeref( dd, bRes );
        bRes = bResNext; // takes ref       
    }

    Cudd_Deref( bRes );
    return bRes;
}


/**Function********************************************************************

  Synopsis [Returns the BDD that is distance 1 from the core, including the core.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddDistance1( DdManager * dd, DdNode * bFunc, DdNode * bCore )
{
    DdNode  * bRes;

    assert( Cudd_bddLeq( dd, bCore, bFunc ) );

    do {
        dd->reordered = 0;
        bRes = extraBddDistance1( dd, bFunc, bCore );
    } while (dd->reordered == 1);

    // the result should contain the core and should be contained in bFunc
    Cudd_Ref( bRes );
    assert( Cudd_bddLeq( dd, bCore, bRes ) );
    assert( Cudd_bddLeq( dd, bRes,  bFunc ) );
    Cudd_Deref( bRes );

    return bRes;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive steps of Extra_bddDistance1.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraBddDistance1( DdManager * dd, DdNode * bFunc, DdNode * bCore )
{
    DdNode * bRes;

    if ( bCore == b0 )
        return b0;
    if ( bCore == b1 )
        return bFunc;
    assert( bFunc != b0 );
    if ( bFunc == b1 )
        return b1;
    // both bFunc and bCore are non-constants

    if ( ( bRes = cuddCacheLookup2(dd, extraBddDistance1, bFunc, bCore) ) )
        return bRes;
    else
    {
        DdNode * bFuncR, * bF0, * bF1;
        DdNode * bCoreR, * bC0, * bC1;
        DdNode * bRes0, * bRes1, * bTemp, * bAdd;
        int LevelF, LevelC, index;

        bFuncR = Cudd_Regular( bFunc ); 
        bCoreR = Cudd_Regular( bCore ); 

        LevelF = dd->perm[bFuncR->index];
        LevelC = dd->perm[bCoreR->index];

        if ( LevelF <= LevelC )
        {
            index = dd->invperm[LevelF];
            if ( bFuncR != bFunc )
            {
                bF0 = Cudd_Not( cuddE(bFuncR) );
                bF1 = Cudd_Not( cuddT(bFuncR) );
            }
            else
            {
                bF0 = cuddE(bFuncR);
                bF1 = cuddT(bFuncR);
            }
        }
        else
        {
            index = dd->invperm[LevelC];
            bF0 = bF1 = bFunc;
        }

        if ( LevelC <= LevelF )
        {
            if ( bCoreR != bCore )
            {
                bC0 = Cudd_Not( cuddE(bCoreR) );
                bC1 = Cudd_Not( cuddT(bCoreR) );
            }
            else
            {
                bC0 = cuddE(bCoreR);
                bC1 = cuddT(bCoreR);
            }
        }
        else
            bC0 = bC1 = bCore;


        // solve the problem for the negative cofactor
        bRes0 = extraBddDistance1( dd, bF0, bC0 );
        if ( bRes0 == NULL ) 
            return NULL;
        cuddRef( bRes0 );

        // compute minterms that are dist-1 in the current var 
        bAdd = cuddBddAndRecur( dd, bF0, bC1 );
        if ( bAdd == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bAdd );

        // add these minterms to the cofactor
        bRes0 = cuddBddAndRecur( dd, bTemp = Cudd_Not(bRes0), Cudd_Not(bAdd) );
        if ( bRes0 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bTemp );
            Cudd_RecursiveDeref( dd, bAdd );
            return NULL;
        }
        cuddRef( bRes0 );
        bRes0 = Cudd_Not( bRes0 );
        Cudd_RecursiveDeref( dd, bTemp );
        Cudd_RecursiveDeref( dd, bAdd );



        // solve the problem for the positive cofactor
        bRes1 = extraBddDistance1( dd, bF1, bC1 );
        if ( bRes1 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bRes1 );

        // compute minterms that are dist-1 in the current var 
        bAdd = cuddBddAndRecur( dd, bF1, bC0 );
        if ( bAdd == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bRes0 );
            Cudd_RecursiveDeref( dd, bRes1 );
            return NULL;
        }
        cuddRef( bAdd );

        // add these minterms to the cofactor
        bRes1 = cuddBddAndRecur( dd, bTemp = Cudd_Not(bRes1), Cudd_Not(bAdd) );
        if ( bRes1 == NULL ) 
        {
            Cudd_RecursiveDeref( dd, bTemp );
            Cudd_RecursiveDeref( dd, bAdd );
            Cudd_RecursiveDeref( dd, bRes0 );
            return NULL;
        }
        cuddRef( bRes1 );
        bRes1 = Cudd_Not( bRes1 );
        Cudd_RecursiveDeref( dd, bTemp );
        Cudd_RecursiveDeref( dd, bAdd );


        // consider the case when Res0 and Res1 are the same node 
        if ( bRes0 == bRes1 )
            bRes = bRes1;
        // consider the case when Res1 is complemented 
        else if ( Cudd_IsComplement(bRes1) ) 
        {
            bRes = cuddUniqueInter(dd, index, Cudd_Not(bRes1), Cudd_Not(bRes0));
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
            bRes = cuddUniqueInter( dd, index, bRes1, bRes0 );
            if ( bRes == NULL ) 
            {
                Cudd_RecursiveDeref(dd,bRes0);
                Cudd_RecursiveDeref(dd,bRes1);
                return NULL;
            }
        }
        cuddDeref( bRes0 );
        cuddDeref( bRes1 );
            
        // insert the result into cache 
        cuddCacheInsert2(dd, extraBddDistance1, bFunc, bCore, bRes);
        return bRes;
    }
}  /* end of extraBddDistance1 */

/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

