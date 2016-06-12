/**CFile****************************************************************

  FileName    [extraAddMisc.c]

  PackageName [extra]

  Synopsis    [Various ADD manipulating procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraAddMisc.c,v 1.0 2003/05/21 18:03:50 alanmi Exp $]

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

  Synopsis    [Remaps the function to depend on the topmost variables on the manager.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_addRemapUp(
  DdManager * dd,
  DdNode * aF )
{
    static int Permute[MAXINPUTS];
    DdNode * bSupp, * bTemp, * aRes;
    int Counter;

    // get support
    bSupp = Cudd_Support( dd, aF );    Cudd_Ref( bSupp );

    // create the variable map
    // to remap the DD into the upper part of the manager
    Counter = 0;
    for ( bTemp = bSupp; bTemp != b1; bTemp = cuddT(bTemp) )
        Permute[bTemp->index] = dd->invperm[Counter++];

    // transfer the BDD and remap it
    aRes = Cudd_addPermute( dd, aF, Permute );  Cudd_Ref( aRes );

    // remove support
    Cudd_RecursiveDeref( dd, bSupp );

    // return
    Cudd_Deref( aRes );
    return aRes;
}

/**Function********************************************************************

  Synopsis    [Counts the number of different constant nodes of the ADD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_addCountConst(
  DdManager * dd,
  DdNode * aFunc )
{
    DdGen * genDD;
    DdNode * node;
    int Count = 0;

    /* iterate through the nodes */
    Cudd_ForeachNode( dd, aFunc, genDD, node )
    {
        if ( Cudd_IsConstant(node) )
            Count++;
    }
    return Count;
}


/**Function********************************************************************

  Synopsis    [Counts the number of different constant nodes of the array of ADDs.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_addCountConstArray(
  DdManager * dd,
  DdNode ** paFuncs,
  int nFuncs )
{
    st_table * Visited;
    st_generator * gen;
    DdNode * aNode;
    int i, Count = 0;
    assert( nFuncs > 0 );

    // start the table
    Visited = st_init_table( st_ptrcmp, st_ptrhash );

    // collect the unique nodes in the shared ADD
    for ( i = 0; i < nFuncs; i++ )
        cuddCollectNodes( Cudd_Regular( paFuncs[i] ), Visited );

    // count the unique terminals
    st_foreach_item( Visited, gen, (char**)&aNode, NULL) 
    {
        if ( Cudd_IsConstant(aNode) )
            Count++;
    }

    // deref the visited table
    st_free_table( Visited );
    return Count;
}

/**Function********************************************************************

  Synopsis    [Return the encoded set of absolute values of the constant nodes of an ADD.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_bddAddConstants(
  DdManager * dd,
  DdNode * aFunc )
{
    st_table * Visited;
    st_generator * gen;
    DdNode * aNode, * bRes, * bTemp, * bCube;
    int nVars;

    // start the table
    Visited = st_init_table( st_ptrcmp, st_ptrhash );

    // collect the unique nodes in the ADD
    cuddCollectNodes( Cudd_Regular(aFunc), Visited );

    nVars = Cudd_SupportSize( dd, aFunc );

    // start the set of encoded constants
    bRes = b0;  Cudd_Ref( bRes );
    st_foreach_item( Visited, gen, (char**)&aNode, NULL) 
    {
        if ( Cudd_IsConstant(aNode) )
        {
            bCube = Extra_bddBitsToCube( dd, (int)ddAbs(cuddV(aNode)), nVars, NULL );  Cudd_Ref( bCube );
            bRes  = Cudd_bddOr( dd, bCube, bTemp = bRes );                            Cudd_Ref( bRes );
            Cudd_RecursiveDeref( dd, bTemp );
            Cudd_RecursiveDeref( dd, bCube );
        }
    }

    // deref the visited table
    st_free_table( Visited );

    Cudd_Deref( bRes );
    return bRes;
} /* end of Extra_bddAddConstants */


/**Function********************************************************************

  Synopsis    [Restructure the ADD by replacing negative terminals with their abs value.]

  Description []

  SideEffects []

  SeeAlso     [Cudd_addMonadicApply]

******************************************************************************/
DdNode * Extra_addAbsCudd( DdManager * dd, DdNode * f )
{
    if ( cuddIsConstant( f ) )
    {
        CUDD_VALUE_TYPE value = cuddV( f );
        if ( value < 0 )    value = -value;
        return cuddUniqueConst( dd, value );
    }
    return ( NULL );

}                               /* end of Extra_addAbsCudd */


/**Function********************************************************************

  Synopsis    [Finds the minimum discriminant of the array of ADDs.]

  Description [Returns a pointer to a constant ADD.]

  SideEffects [None]

******************************************************************************/
DdNode * Extra_addFindMinArray( DdManager * dd, DdNode ** paFuncs, int nFuncs )
{
    DdNode * aRes, * aCur;
    int i;
    assert( nFuncs > 0 );

    aRes = Cudd_addFindMin( dd, paFuncs[0] );
    for ( i = 1; i < nFuncs; i++ )
    {
        aCur = Cudd_addFindMin( dd, paFuncs[i] );
        aRes = ( cuddV(aRes) <= cuddV(aCur) ) ? aRes: aCur;
    }

    return aRes;
}  /* end of Extra_addFindMinArray */


/**Function********************************************************************

  Synopsis    [Finds the maximum discriminant of the array of ADDs.]

  Description [Returns a pointer to a constant ADD.]

  SideEffects [None]

******************************************************************************/
DdNode * Extra_addFindMaxArray( DdManager * dd, DdNode ** paFuncs, int nFuncs )
{
    DdNode * aRes, * aCur;
    int i;
    assert( nFuncs > 0 );

    aRes = Cudd_addFindMax( dd, paFuncs[0] );
    for ( i = 1; i < nFuncs; i++ )
    {
        aCur = Cudd_addFindMax( dd, paFuncs[i] );
        aRes = ( cuddV(aRes) >= cuddV(aCur) ) ? aRes: aCur;
    }

    return aRes;
}  /* end of Extra_addFindMinArray */


/**Function********************************************************************

  Synopsis    [Absolute value of an ADD.]

  Description [Absolute value of an ADD. Returns NULL if not a terminal case; 
  abs(f) otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_addMonadicApply]

******************************************************************************/
DdNode * Extra_addAbs( DdManager * dd, DdNode * f )
{
    if ( cuddIsConstant( f ) )
    {
        CUDD_VALUE_TYPE value = ( cuddV( f ) > 0 )? cuddV( f ): -cuddV( f );
        DdNode *res = cuddUniqueConst( dd, value );
        return ( res );
    }
    return ( NULL );

} /* end of Extra_addAbs */


/**Function********************************************************************

  Synopsis    [Determines whether this is an ADD or a BDD.]

  Description [Returns 1, if the function is an ADD; 0, if the function is a BDD; -1, if unknown.]

  SideEffects []

  SeeAlso     []

******************************************************************************/
int Extra_addDetectAdd( DdManager * dd, DdNode * Func )
{
    int RetValue;

    if ( Cudd_IsComplement(Func) )
        return 0;
    if ( Func == dd->one )
        return -1;
    if ( Func == dd->zero )
        return 1;

    // treat the case of MTBDD (it may have some strange terminal nodes)
    if ( Cudd_IsConstant(Func) )
        return 1;

    RetValue = Extra_addDetectAdd( dd, cuddE(Func) );
    if ( RetValue == 1 )
        return 1;
    if ( RetValue == 0 )
        return 0;
    return Extra_addDetectAdd( dd, cuddT(Func) );
}

/**Function********************************************************************

  Synopsis [ADD restrict according.]

  Description [ADD restrict according to Coudert and Madre's algorithm
  (ICCAD90) applicable to true ADDs (and not only to 0/1 ADDs as in CUDD). 
  Another important difference is that this procedure takes the don't-care set,
  and not the care set as the standard restrict.
  Returns the restricted ADD if successful; otherwise NULL.  If application of 
  restrict results in an ADD larger than the input ADD, the input ADD is returned.]

  SideEffects [None]

  SeeAlso     [Cudd_addConstrain Cudd_addRestrict]

******************************************************************************/
DdNode * Extra_addRestrictAdd( DdManager * dd, DdNode * f, DdNode * d )
{
    DdNode *supp_f, *supp_d;
    DdNode *res, *commonSupport;
    int intersection;
    int sizeF, sizeRes;

    /* Check if supports intersect. */
    supp_f = Cudd_Support( dd, f );
    if ( supp_f == NULL )
    {
        return ( NULL );
    }
    cuddRef( supp_f );
    supp_d = Cudd_Support( dd, d );
    if ( supp_d == NULL )
    {
        Cudd_RecursiveDeref( dd, supp_f );
        return ( NULL );
    }
    cuddRef( supp_d );
    commonSupport = Cudd_bddLiteralSetIntersection( dd, supp_f, supp_d );
    if ( commonSupport == NULL )
    {
        Cudd_RecursiveDeref( dd, supp_f );
        Cudd_RecursiveDeref( dd, supp_d );
        return ( NULL );
    }
    cuddRef( commonSupport );
    Cudd_RecursiveDeref( dd, supp_f );
    Cudd_RecursiveDeref( dd, supp_d );
    intersection = (commonSupport != DD_ONE(dd));
    Cudd_RecursiveDeref( dd, commonSupport );

    if ( intersection )
    {
        do
        {
            dd->reordered = 0;
            res = extraAddRestrictAdd( dd, f, d );
        }
        while ( dd->reordered == 1 );
        sizeF   = Cudd_DagSize( f );
        sizeRes = Cudd_DagSize( res );
        if ( sizeF <= sizeRes )
        {
            cuddRef( res );
            Cudd_RecursiveDeref( dd, res );
            return ( f );
        }
        else
        {
            return ( res );
        }
    }
    else
    {
        return ( f );
    }

}                               /* end of Extra_addRestrictAdd */

/**Function********************************************************************

  Synopsis    [The intersection of paths leading to each terminal, except terminal 0.]

  Description []

  SideEffects [None]

  SeeAlso     [Cudd_addApply]

******************************************************************************/
DdNode * Extra_addForeachTerminalAnd( DdManager * dd, DdNode ** f, DdNode ** g )
{
    if ( cuddIsConstant( *f ) && cuddIsConstant( *g ) )
    {
        if ( *f == *g )
            return *f;
        else
            return dd->zero;
    }
    return NULL;
}                               /* end of Extra_addForeachTerminalAnd */

/**Function********************************************************************

  Synopsis    [The union of paths leading to each terminal, except terminal 0.]

  Description [The same path in two ADDs cannot lead to different terminals 
  unless one of the terminals is 0.]

  SideEffects [None]

  SeeAlso     [Cudd_addApply]

******************************************************************************/
DdNode * Extra_addForeachTerminalOr( DdManager * dd, DdNode ** f, DdNode ** g )
{
    if ( cuddIsConstant( *f ) && cuddIsConstant( *g ) )
    {
        if ( *f == *g )
            return *f;
        if ( *f == dd->zero )
            return *g;
        if ( *g == dd->zero )
            return *f;
        assert( 0 );
    }
    return NULL;
}                               /* end of Extra_addForeachTerminalOr */

/**Function********************************************************************

  Synopsis    [The union of paths leading to each terminal, except terminal 0.]

  Description [It is known that g is the subset of f.]

  SideEffects [None]

  SeeAlso     [Cudd_addApply]

******************************************************************************/
DdNode * Extra_addForeachTerminalSharp( DdManager * dd, DdNode ** f, DdNode ** g )
{
    if ( cuddIsConstant( *f ) && cuddIsConstant( *g ) )
    {
        if ( *f == *g )
            return dd->zero;
        if ( *g == dd->zero )
            return *f;
        assert( 0 );
    }
    return NULL;
}                               /* end of Extra_addForeachTerminalOr */

/**Function********************************************************************

  Synopsis    [Swaps the given terminal with the zero terminal.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * Extra_addSwapTerminals( DdManager * dd, DdNode * aFunc, DdNode * aTerm )
{
    DdNode * aRes;
    do {
        dd->reordered = 0;
        aRes = extraAddSwapTerminals( dd, aFunc, aTerm );
    } while (dd->reordered == 1);
    return aRes;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Swaps the given terminal with the zero terminal.]

  Description []

  SideEffects []

  SeeAlso     []

******************************************************************************/
DdNode * extraAddSwapTerminals( DdManager * dd, DdNode * aFunc, DdNode * aTerm )
{
    DdNode * aRes0, * aRes1, * aRes;

    if ( cuddIsConstant( aFunc ) )
    {
        if ( aFunc == a0 )
            return aTerm;
        if ( aFunc == aTerm )
            return a0;
        return aFunc;
    }

    if ( aRes = cuddCacheLookup2(dd, extraAddSwapTerminals, aFunc, aTerm) )
        return aRes;

    aRes0 = extraAddSwapTerminals( dd, cuddE(aFunc), aTerm );
    if ( aRes0 == NULL ) 
        return NULL;
    cuddRef( aRes0 );

    aRes1 = extraAddSwapTerminals( dd, cuddT(aFunc), aTerm );
    if ( aRes1 == NULL ) 
    {
        Cudd_RecursiveDeref( dd, aRes0 );
        return NULL;
    }
    cuddRef( aRes1 );

    if ( aRes0 == aRes1 )
        aRes = aRes1;
    else 
    {
        aRes = cuddUniqueInter( dd, aFunc->index, aRes1, aRes0 );
        if ( aRes == NULL ) 
        {
            Cudd_RecursiveDeref(dd,aRes0);
            Cudd_RecursiveDeref(dd,aRes1);
            return NULL;
        }
    }
    cuddDeref( aRes0 );
    cuddDeref( aRes1 );
        
    cuddCacheInsert2(dd, extraAddSwapTerminals, aFunc, aTerm, aRes);
    return aRes;
}


/**Function********************************************************************

  Synopsis    [Performs the recursive step of Extra_addRestrictAdd.]

  Description [Performs the recursive step of Extra_addRestrictAdd.
  Returns the restricted ADD if successful; otherwise NULL.]

  SideEffects [None]

  SeeAlso     [Cudd_addRestrict]

******************************************************************************/
DdNode * extraAddRestrictAdd( DdManager * dd, DdNode * aF, DdNode * aD )
{
    DdNode * aF0, * aF1, * aD0, * aD1, * aDtot, * aRes0, * aRes1, * aRes;
    int topf, topd;

    statLine( dd );

    // if there is no don't-care, return aF
    if ( aD == dd->zero )
        return aF;
    // if they are equal, aF is a complete don't-care (can be simplified to zero)
    if ( aF == aD )
        return dd->zero;
    // here, they are not equal and don't-care is not empty

    // if they are both constants, they are *different* constants, so return aF
    if ( cuddIsConstant(aF) && cuddIsConstant(aD) )
        return aF;
    // here, at least one of them is not a constant

    // check the cache
    aRes = cuddCacheLookup2( dd, extraAddRestrictAdd, aF, aD );
    if ( aRes != NULL )
        return aRes;

    topf = cuddI( dd, aF->index );
    topd = cuddI( dd, aD->index );

    if ( topd < topf )
    { // abstract the top variable from aD

        // find cofactors of aD
        aD0 = cuddE(aD);
        aD1 = cuddT(aD);
        // intersect the cofactor don't-cares
        aDtot = cuddAddApplyRecur( dd, Extra_addForeachTerminalAnd, aD0, aD1 );
        if ( aDtot == NULL )
            return ( NULL );
        cuddRef( aDtot );
        aRes = extraAddRestrictAdd( dd, aF, aDtot );
        if ( aRes == NULL )
        {
            Cudd_RecursiveDeref( dd, aDtot );
            return ( NULL );
        }
        cuddRef( aRes );
        Cudd_RecursiveDeref( dd, aDtot );
        cuddCacheInsert2( dd, extraAddRestrictAdd, aF, aD, aRes );
        cuddDeref( aRes );
        return aRes;
    }
    // here topf <= topc

    aF0 = cuddE(aF);
    aF1 = cuddT(aF);
    if ( topd == topf )
    {
        aD0 = cuddE(aD);
        aD1 = cuddT(aD);
    }
    else
    {
        aD0 = aD1 = aD;
    }

    aRes0 = extraAddRestrictAdd( dd, aF0, aD0 );
    if ( aRes0 == NULL )
        return NULL;
    cuddRef( aRes0 );

    aRes1 = extraAddRestrictAdd( dd, aF1, aD1 );
    if ( aRes1 == NULL )
    {
        Cudd_RecursiveDeref( dd, aRes0 );
        return NULL;
    }
    cuddRef( aRes1 );

    aRes = ( aRes0 == aRes1 ) ? aRes0 : cuddUniqueInter( dd, aF->index, aRes1, aRes0 );
    if ( aRes == NULL )
    {
        Cudd_RecursiveDeref( dd, aRes0 );
        Cudd_RecursiveDeref( dd, aRes1 );
        return ( NULL );
    }
    cuddDeref( aRes0 );
    cuddDeref( aRes1 );

    cuddCacheInsert2( dd, extraAddRestrictAdd, aF, aD, aRes );
    return aRes;

}   /* end of extraAddRestrictAdd */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/
