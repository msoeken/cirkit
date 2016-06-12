/**CFile****************************************************************

  FileName    [extraZddIsop.c]

  PackageName [extra]

  Synopsis    [Various specialied ISOP computation procedures.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddIsop.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "extra.h"

/*---------------------------------------------------------------------------*/
/* Constant declarations                                                     */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Structure declarations                                                     */
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

static DdNode * extraZddIsopCoverRandom  (DdManager * dd, DdHashTable * table, DdNode * bOn, DdNode * bOnDc, int * pPerm, int cVar);
static int * extraGenerateRandomPermutation (DdManager * dd, int nVars);
static void     cuddHashTableQuitZdd2(DdHashTable * hash);

/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [The alternative implementation of irredundant sum-of-products.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsop]

******************************************************************************/
DdNode* Extra_zddIsopCoverAllVars( 
  DdManager * dd,     /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc)  /* the on-set + dc-set */
{
    DdNode  *res;

    /* find the support */
    DdNode * bSuppOn, * bSuppOnDc, * bSupp;
    bSuppOn = Cudd_Support( dd, bFuncOn );
    Cudd_Ref( bSuppOn );
    bSuppOnDc = Cudd_Support( dd, bFuncOnDc );
    Cudd_Ref( bSuppOnDc );
    bSupp = Cudd_bddAnd( dd, bSuppOn, bSuppOnDc );
    Cudd_Ref( bSupp );
    Cudd_RecursiveDeref( dd, bSuppOn );
    Cudd_RecursiveDeref( dd, bSuppOnDc );

    do {
        dd->reordered = 0;
        res = extraZddIsopCoverAllVars( dd, bFuncOn, bFuncOnDc, bSupp );
    } while (dd->reordered == 1);

    Cudd_RecursiveDeref( dd, bSupp );
    return(res);

} /* end of Extra_zddIsopCoverAllVars */


/**Function********************************************************************

  Synopsis    [The alternative implementation of irredundant sum-of-products.]

  Description []

  SideEffects []

  SeeAlso     [Cudd_zddMinimal]

******************************************************************************/
DdNode* Extra_zddIsopCoverUnateVars( 
  DdManager * dd,     /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc)  /* the on-set + dc-set */
{
    DdNode  *res;

    /* find the support */
    DdNode * bSuppOn, * bSuppOnDc, * bSupp;
    bSuppOn = Cudd_Support( dd, bFuncOn );
    Cudd_Ref( bSuppOn );
    bSuppOnDc = Cudd_Support( dd, bFuncOnDc );
    Cudd_Ref( bSuppOnDc );
    bSupp = Cudd_bddAnd( dd, bSuppOn, bSuppOnDc );
    Cudd_Ref( bSupp );
    Cudd_RecursiveDeref( dd, bSuppOn );
    Cudd_RecursiveDeref( dd, bSuppOnDc );

    do {
        dd->reordered = 0;
        res = extraZddIsopCoverUnateVars( dd, bFuncOn, bFuncOnDc, bSupp );
    } while (dd->reordered == 1);

    Cudd_RecursiveDeref( dd, bSupp );
    return(res);

} /* end of Extra_zddIsopCoverUnateVars */

/**Function********************************************************************

  Synopsis    [Computes an ISOP cover with a random ordering of variables.]

  Description []

  SideEffects []

  SeeAlso     [Cudd_zddMinimal]

******************************************************************************/
DdNode* Extra_zddIsopCoverRandom( 
  DdManager * dd,     /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc)  /* the on-set + dc-set */
{
    DdHashTable * table;
    DdNode      * res;
    int         * pPerm = extraGenerateRandomPermutation( dd, dd->sizeZ/2 );

    do {
        dd->reordered = 0;
        table = cuddHashTableInit(dd,2,2);
        if (table == NULL) return(NULL);
        res = extraZddIsopCoverRandom(dd,table,bFuncOn,bFuncOnDc,pPerm,0);
        if (res != NULL) cuddRef(res);
        /* Dispose of local cache. */
        cuddHashTableQuitZdd2(table); 
    } while (dd->reordered == 1);
    free( pPerm ); 
/*
    {
    DdNode * bIsop = Cudd_zddConvertToBdd( dd, res );   Cudd_Ref( bIsop );
    assert( Cudd_bddIteConstant( dd, bFuncOn,     bIsop, b1 ) == b1 );
    assert( Cudd_bddIteConstant( dd, bIsop,   bFuncOnDc, b1 ) == b1 );
    Cudd_RecursiveDeref( dd, bIsop );
    }
*/
    if (res != NULL) cuddDeref(res);
    return(res);

} /* end of Extra_zddIsopCoverRandom */

/**Function********************************************************************

  Synopsis    [The alternative implementation of irredundant sum-of-products.]

  Description [The main idea of this function is to existentially abstract a variable
  from OnSet if it does not appear in OnDcSet and universally abstract a variable 
  from OnDcSet if it does not appear in OnSet.]

  SideEffects []

  SeeAlso     [Extra_zddIsop]

******************************************************************************/
DdNode* Extra_zddIsopCoverReduced( 
  DdManager * dd,     /* the DD manager */
  DdNode * bFuncOn,    /* the on-set */
  DdNode * bFuncOnDc)  /* the on-set + dc-set */
{
    DdNode  *res;
    do {
    dd->reordered = 0;
    res = extraZddIsopCoverReduced( dd, bFuncOn, bFuncOnDc );
    } while (dd->reordered == 1);
    return(res);

} /* end of Extra_zddIsopCoverReduced */


/*---------------------------------------------------------------------------*/
/* Definition of Internal Functions                                          */
/*---------------------------------------------------------------------------*/



/**Function********************************************************************

  Synopsis    [Performs the recursive step of the alternative ISOP computation.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode* extraZddIsopCoverAllVars( 
  DdManager * dd,     /* the DD manager */
  DdNode * bA,        /* the on-set */
  DdNode * bB,        /* the on-set + dc-set */
  DdNode * bS)        /* the support of on-set and (on-set + dc-set) */
{
    DdNode  *zRes;
    DdNode*(*cacheOp)(DdManager*,DdNode*,DdNode*);

    statLine(dd); 

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    return z0;
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    return z1;

    /* check cache */
    cacheOp = (DdNode*(*)(DdManager*,DdNode*,DdNode*))extraZddIsopCoverAllVars;
    zRes = cuddCacheLookup2Zdd(dd, cacheOp, bA, bB);
    if (zRes)
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bB0, *bB1, *bG0, *bG1, *bHA, *bHB;
        DdNode *zRes0, *zRes1, *zRes2, *zTemp;

        DdNode *bSuppTemp, *bSuppNew;
        int varCur, varBest = -1, scoreCur, scoreBest = 1000000;
        DdNode *zRes0best, *zRes1best, *zRes2best;
        int Counter = 0;

        Cudd_Ref( zRes0best = z0 );
        Cudd_Ref( zRes1best = z0 );
        Cudd_Ref( zRes2best = z0 );

        for( bSuppTemp = bS; bSuppTemp != b1; bSuppTemp = Cudd_T(bSuppTemp) )
        {
            /* get the current variable */
            varCur = bSuppTemp->index;


            if ( Counter++ != 0 && varCur % 2 != 0 )
                continue;
            

            /* remove this variable from the support */
            bSuppNew = cuddBddExistAbstractRecur( dd, bS, dd->vars[varCur] );
            if ( bSuppNew == NULL )
            {
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                return NULL;
            }
            cuddRef( bSuppNew );

            /* cofactor the functions */
            bA0 = cuddCofactorRecur( dd, bA, Cudd_Not(dd->vars[varCur]) );
            if ( bA0 == NULL ) 
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                return NULL;
            }
            cuddRef( bA0 );

            bA1 = cuddCofactorRecur( dd, bA,          dd->vars[varCur]  );
            if ( bA1 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                return NULL;
            }
            cuddRef( bA1 );

            bB0 = cuddCofactorRecur( dd, bB, Cudd_Not(dd->vars[varCur]) );
            if ( bB0 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                return NULL;
            }
            cuddRef( bB0 );

            bB1 = cuddCofactorRecur( dd, bB,          dd->vars[varCur]  );
            if ( bB1 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                return NULL;
            }
            cuddRef( bB1 );

            /* at this point, bSuppNew, bA0, bA1, bB0, bB1 are referenced 
               as well as zRes0best, zRes1best, zRes2best */

            /*=====================================================*/
            /* g0 = F1(x=0) & !F12(x=1) */
            bG0 = cuddBddAndRecur( dd, bA0, Cudd_Not(bB1) );
            if ( bG0 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);
                return NULL;
            }
            cuddRef( bG0 );

            /* P0 = IrrCover( g0, F12(x=0) ) */
            zRes0 = extraZddIsopCoverAllVars( dd, bG0, bB0, bSuppNew );
            if ( zRes0 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);

                Cudd_IterDerefBdd(dd, bG0);
                return NULL;
            }
            cuddRef( zRes0 );
            Cudd_IterDerefBdd(dd, bG0);


            /* g0 = F1(x=1) & !F12(x=0) */
            bG1 = cuddBddAndRecur( dd, bA1, Cudd_Not(bB0) );
            if ( bG1 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);

                Cudd_RecursiveDerefZdd(dd, zRes0);
                return NULL;
            }
            cuddRef( bG1 );

            /* P1 = IrrCover( g1, F12(x=1) ) */
            zRes1 = extraZddIsopCoverAllVars( dd, bG1, bB1, bSuppNew );
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);

                Cudd_IterDerefBdd(dd, bG1);
                Cudd_RecursiveDerefZdd(dd, zRes0);
                return NULL;
            }
            cuddRef( zRes1 );
            Cudd_IterDerefBdd(dd, bG1);

            /* h1 = F1(x=0) & !bdd(P0)  +   F1(x=1) & !bdd(P1) */
            bG0 = extraZddConvertToBddAndAdd( dd, zRes0, Cudd_Not(bA0) );
            if ( bG0 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);

                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
            cuddRef( bG0 );

            bG1 = extraZddConvertToBddAndAdd( dd, zRes1, Cudd_Not(bA1) );
            if ( bG1 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);

                Cudd_IterDerefBdd(dd, bG0);
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
            cuddRef( bG1 );

            bHA = cuddBddAndRecur( dd, bG0, bG1 );
            if ( bHA == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);

                Cudd_IterDerefBdd(dd, bG0);
                Cudd_IterDerefBdd(dd, bG1);
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
            cuddRef( bHA );
            bHA = Cudd_Not(bHA);
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);

            /* h12 = F12(x=0) & F12(x=1) */
            bHB = cuddBddAndRecur( dd, bB0, bB1 );
            if ( bHB == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);

                Cudd_IterDerefBdd(dd, bHA);
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
            cuddRef( bHB );

            /* P1 = IrrCover( h1, h12 ) */
            zRes2 = extraZddIsopCoverAllVars( dd, bHA, bHB, bSuppNew );
            if ( zRes2 == NULL )
            {
                Cudd_RecursiveDeref(dd, bSuppNew);
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                Cudd_RecursiveDeref(dd, bB1);

                Cudd_IterDerefBdd(dd, bHA);
                Cudd_IterDerefBdd(dd, bHB);
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
            cuddRef( zRes2 );
            Cudd_IterDerefBdd(dd, bHA);
            Cudd_IterDerefBdd(dd, bHB);

            scoreCur = Cudd_zddCount(dd,zRes0) + Cudd_zddCount(dd,zRes1) + Cudd_zddCount(dd,zRes2);
            if ( scoreCur < scoreBest )
            {
                varBest   = varCur;
                scoreBest = scoreCur;

                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                zRes0best = zRes0;
                zRes1best = zRes1;
                zRes2best = zRes2;
            }
            else
            { /* leave without changes */
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                Cudd_RecursiveDerefZdd(dd, zRes2);
            }
            /*=====================================================*/

            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);
        }

        assert( varBest != -1 );
        /* the best variable is in varBest, 
           the best subproblems are in zRes0best, zRes1best, zRes2best */


        /* --------------- compose the result ------------------ */
        if (zRes0best != z0) 
        {
            zTemp = cuddZddGetNodeIVO(dd, varBest * 2 + 1, zRes0best, zRes2best);
            if (zTemp == NULL) 
            {
                Cudd_RecursiveDerefZdd(dd, zRes0best);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                Cudd_RecursiveDerefZdd(dd, zRes2best);
                return NULL;
            }
        }
        else 
            zTemp = zRes2best;
        cuddRef( zTemp );
        Cudd_RecursiveDerefZdd( dd, zRes0best );
        Cudd_RecursiveDerefZdd( dd, zRes2best );

        if (zRes1best != z0) 
        {
            zRes = cuddZddGetNodeIVO(dd, varBest * 2, zRes1best, zTemp);
            if (zRes == NULL) 
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                Cudd_RecursiveDerefZdd(dd, zRes1best);
                return NULL;
            }
        }
        else 
            zRes = zTemp;
        cuddRef( zRes );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zRes1best );
        cuddDeref( zRes );

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, cacheOp, bA, bB, zRes);
        return zRes;
        /*=====================================================*/
    }
} /* end of extraZddIsopCoverAllVars */



/**Function********************************************************************

  Synopsis    [Performs the recursive step of the alternative ISOP computation.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode* extraZddIsopCoverUnateVars( 
  DdManager * dd,     /* the DD manager */
  DdNode * bA,        /* the on-set */
  DdNode * bB,        /* the on-set + dc-set */
  DdNode * bS)        /* the support of on-set and (on-set + dc-set) */
{
    DdNode  *zRes;
    DdNode*(*cacheOp)(DdManager*,DdNode*,DdNode*);

    statLine(dd); 

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    return z0;
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    return z1;

    /* check cache */
    cacheOp = (DdNode*(*)(DdManager*,DdNode*,DdNode*))extraZddIsopCoverUnateVars;
    zRes = cuddCacheLookup2Zdd(dd, cacheOp, bA, bB);
    if (zRes)
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bB0, *bB1, *bG0, *bG1, *bHA, *bHB;
        DdNode *zRes0, *zRes1, *zRes2, *zTemp;

        DdNode *bSuppTemp, *bSuppNew;
        int Counter = 0;

        DdNode *bA0best = NULL, *bA1best = NULL, *bB0best = NULL, *bB1best = NULL;
        int varCur, varBest = -1;

        /* find the first unate variable */
        for( bSuppTemp = bS; bSuppTemp != b1; bSuppTemp = Cudd_T(bSuppTemp) )
        {
            /* get the current variable */
            varCur = bSuppTemp->index;

            /* cofactor the functions */
            bA0 = cuddCofactorRecur( dd, bA, Cudd_Not(dd->vars[varCur]) );
            if ( bA0 == NULL ) 
            {
                if ( varBest != -1 )
                { /* was previously assigned */
                    Cudd_RecursiveDeref( dd, bA0best );
                    Cudd_RecursiveDeref( dd, bA1best );
                    Cudd_RecursiveDeref( dd, bB0best );
                    Cudd_RecursiveDeref( dd, bB1best );
                    Cudd_RecursiveDeref( dd, bSuppNew );
                }
                return NULL;
            }
            cuddRef( bA0 );

            bA1 = cuddCofactorRecur( dd, bA,          dd->vars[varCur]  );
            if ( bA1 == NULL )
            {
                if ( varBest != -1 )
                { /* was previously assigned */
                    Cudd_RecursiveDeref( dd, bA0best );
                    Cudd_RecursiveDeref( dd, bA1best );
                    Cudd_RecursiveDeref( dd, bB0best );
                    Cudd_RecursiveDeref( dd, bB1best );
                    Cudd_RecursiveDeref( dd, bSuppNew );
                }
                Cudd_RecursiveDeref(dd, bA0);
                return NULL;
            }
            cuddRef( bA1 );

            bB0 = cuddCofactorRecur( dd, bB, Cudd_Not(dd->vars[varCur]) );
            if ( bB0 == NULL )
            {
                if ( varBest != -1 )
                { /* was previously assigned */
                    Cudd_RecursiveDeref( dd, bA0best );
                    Cudd_RecursiveDeref( dd, bA1best );
                    Cudd_RecursiveDeref( dd, bB0best );
                    Cudd_RecursiveDeref( dd, bB1best );
                    Cudd_RecursiveDeref( dd, bSuppNew );
                }
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                return NULL;
            }
            cuddRef( bB0 );

            bB1 = cuddCofactorRecur( dd, bB,          dd->vars[varCur]  );
            if ( bB1 == NULL )
            {
                if ( varBest != -1 )
                { /* was previously assigned */
                    Cudd_RecursiveDeref( dd, bA0best );
                    Cudd_RecursiveDeref( dd, bA1best );
                    Cudd_RecursiveDeref( dd, bB0best );
                    Cudd_RecursiveDeref( dd, bB1best );
                    Cudd_RecursiveDeref( dd, bSuppNew );
                }
                Cudd_RecursiveDeref(dd, bA0);
                Cudd_RecursiveDeref(dd, bA1);
                Cudd_RecursiveDeref(dd, bB0);
                return NULL;
            }
            cuddRef( bB1 );


            /* check unateness */
            if ( Cudd_bddIteConstant( dd, bA0, bB1, b1 ) == b1 ||
                 Cudd_bddIteConstant( dd, bA1, bB0, b1 ) == b1 )
            { /* unate */

                if ( varBest != -1 )
                { /* was previously assigned */
                    Cudd_RecursiveDeref( dd, bA0best );
                    Cudd_RecursiveDeref( dd, bA1best );
                    Cudd_RecursiveDeref( dd, bB0best );
                    Cudd_RecursiveDeref( dd, bB1best );
                    Cudd_RecursiveDeref( dd, bSuppNew );
                }

                varBest = varCur;

                bA0best = bA0;
                bA1best = bA1;
                bB0best = bB0;
                bB1best = bB1;

                bSuppNew = cuddBddExistAbstractRecur( dd, bS, dd->vars[varCur] );
                if ( bSuppNew == NULL )
                {
                    Cudd_RecursiveDeref( dd, bA0best );
                    Cudd_RecursiveDeref( dd, bA1best );
                    Cudd_RecursiveDeref( dd, bB0best );
                    Cudd_RecursiveDeref( dd, bB1best );
                    return NULL;
                }
                cuddRef( bSuppNew );
                break;
            }
            else
            {
                if ( varBest == -1 )
                {
                    varBest = varCur;

                    bA0best = bA0;
                    bA1best = bA1;
                    bB0best = bB0;
                    bB1best = bB1;

                    bSuppNew = Cudd_T( bS );
                    cuddRef( bSuppNew );
                }
                else
                {
                    Cudd_RecursiveDeref(dd, bA0);
                    Cudd_RecursiveDeref(dd, bA1);
                    Cudd_RecursiveDeref(dd, bB0);
                    Cudd_RecursiveDeref(dd, bB1);
                }
            }

        }

        bA0 = bA0best;
        bA1 = bA1best;
        bB0 = bB0best;
        bB1 = bB1best;

        /* at this point, bSuppNew, bA0, bA1, bB0, bB1 are referenced */


        /*=====================================================*/
        /* g0 = F1(x=0) & !F12(x=1) */
        bG0 = cuddBddAndRecur( dd, bA0, Cudd_Not(bB1) );
        if ( bG0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);
            return NULL;
        }
        cuddRef( bG0 );

        /* P0 = IrrCover( g0, F12(x=0) ) */
        zRes0 = extraZddIsopCoverUnateVars( dd, bG0, bB0, bSuppNew );
        if ( zRes0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bG0);
            return NULL;
        }
        cuddRef( zRes0 );
        Cudd_IterDerefBdd(dd, bG0);


        /* g0 = F1(x=1) & !F12(x=0) */
        bG1 = cuddBddAndRecur( dd, bA1, Cudd_Not(bB0) );
        if ( bG1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( bG1 );

        /* P1 = IrrCover( g1, F12(x=1) ) */
        zRes1 = extraZddIsopCoverUnateVars( dd, bG1, bB1, bSuppNew );
        if ( zRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( zRes1 );
        Cudd_IterDerefBdd(dd, bG1);

        /* h1 = F1(x=0) & !bdd(P0)  +   F1(x=1) & !bdd(P1) */
        bG0 = extraZddConvertToBddAndAdd( dd, zRes0, Cudd_Not(bA0) );
        if ( bG0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG0 );

        bG1 = extraZddConvertToBddAndAdd( dd, zRes1, Cudd_Not(bA1) );
        if ( bG1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bG0);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG1 );

        bHA = cuddBddAndRecur( dd, bG0, bG1 );
        if ( bHA == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHA );
        bHA = Cudd_Not(bHA);
        Cudd_IterDerefBdd(dd, bG0);
        Cudd_IterDerefBdd(dd, bG1);

        /* h12 = F12(x=0) & F12(x=1) */
        bHB = cuddBddAndRecur( dd, bB0, bB1 );
        if ( bHB == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bHA);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHB );

        /* P1 = IrrCover( h1, h12 ) */
        zRes2 = extraZddIsopCoverUnateVars( dd, bHA, bHB, bSuppNew );
        if ( zRes2 == NULL )
        {
            Cudd_RecursiveDeref(dd, bSuppNew);
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bHA);
            Cudd_IterDerefBdd(dd, bHB);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( zRes2 );
        Cudd_IterDerefBdd(dd, bHA);
        Cudd_IterDerefBdd(dd, bHB);

        /*=====================================================*/

        Cudd_RecursiveDeref(dd, bSuppNew);
        Cudd_RecursiveDeref(dd, bA0);
        Cudd_RecursiveDeref(dd, bA1);
        Cudd_RecursiveDeref(dd, bB0);
        Cudd_RecursiveDeref(dd, bB1);


        assert( varBest != -1 );
        /* the best variable is in varBest, 
           the best subproblems are in zRes0best, zRes1best, zRes2best */


        /* --------------- compose the result ------------------ */
        if (zRes0 != z0) 
        {
            zTemp = cuddZddGetNodeIVO(dd, varBest * 2 + 1, zRes0, zRes2);
            if (zTemp == NULL) 
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                Cudd_RecursiveDerefZdd(dd, zRes2);
                return NULL;
            }
        }
        else 
            zTemp = zRes2;
        cuddRef( zTemp );
        Cudd_RecursiveDerefZdd( dd, zRes0 );
        Cudd_RecursiveDerefZdd( dd, zRes2 );

        if (zRes1 != z0) 
        {
            zRes = cuddZddGetNodeIVO(dd, varBest * 2, zRes1, zTemp);
            if (zRes == NULL) 
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
        }
        else 
            zRes = zTemp;
        cuddRef( zRes );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zRes1 );
        cuddDeref( zRes );

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, cacheOp, bA, bB, zRes);
        return zRes;
        /*=====================================================*/
    }
} /* end of extraZddIsopCoverUnateVars */



/**Function********************************************************************

  Synopsis    [Performs the recursive step of the alternative ISOP computation.]

  Description []

  SideEffects []

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/
DdNode* extraZddIsopCoverReduced( 
  DdManager * dd,     /* the DD manager */
  DdNode * bA,        /* the on-set */
  DdNode * bB)        /* the on-set + dc-set */
{
    DdNode  *zRes;
    statLine(dd); 

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    return z0;
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    return z1;

    /* check cache */
    zRes = cuddCacheLookup2Zdd(dd, extraZddIsopCoverReduced, bA, bB);
    if (zRes)
        return zRes;
    else
    {
        // these flag determine whether we do simplification or not
        int fReduceA = 1;
        int fReduceB = 1;

        DdNode *bA0, *bA1, *bB0, *bB1, *bG0, *bG1, *bHA, *bHB;
        DdNode *zRes0, *zRes1, *zRes2;

        DdNode *bAA = Cudd_Regular(bA);
        DdNode *bBB = Cudd_Regular(bB);

        int LevA = dd->perm[bAA->index];
        int LevB = dd->perm[bBB->index];

        /* the index of the positive ZDD variable in the result */
        int TopVar;

        if ( LevA < LevB && fReduceA ) // A is on top and can be reduced
        {
            DdNode * bAnew;
            printf("A");
            bAnew = cuddBddExistAbstractRecur( dd, bA, dd->vars[bAA->index] );
            if ( bAnew == NULL )
                return NULL;
            cuddRef( bAnew );

            zRes = extraZddIsopCoverReduced( dd, bAnew, bB );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDeref( dd, bAnew );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDeref( dd, bAnew );
            cuddDeref( zRes );
            return zRes;
        }
        if ( LevA > LevB && fReduceB ) // B is on top and can be reduced
        {
            DdNode * bBnew;
            printf("B");
            bBnew = cuddBddExistAbstractRecur( dd, Cudd_Not(bB), dd->vars[bBB->index] );
            if ( bBnew == NULL )
                return NULL;
            bBnew = Cudd_Not(bBnew);
            cuddRef( bBnew );

            zRes = extraZddIsopCoverReduced( dd, bA, bBnew );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDeref( dd, bBnew );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDeref( dd, bBnew );
            cuddDeref( zRes );
            return zRes;
        }


        /* consider normal cases */
        if ( LevA <= LevB )
        {
            int fIsComp = (int)(bA != bAA);
            bA0 = Cudd_NotCond( cuddE(bAA), fIsComp );
            bA1 = Cudd_NotCond( cuddT(bAA), fIsComp );
            TopVar = bAA->index;
        }
        else
            bA0 = bA1 = bA;

        if ( LevB <= LevA )
        {
            int fIsComp = (int)(bB != bBB);
            bB0 = Cudd_NotCond( cuddE(bBB), fIsComp );
            bB1 = Cudd_NotCond( cuddT(bBB), fIsComp );
            TopVar = bBB->index;
        }
        else
            bB0 = bB1 = bB;

        /* g0 = F1(x=0) & !F12(x=1) */
        bG0 = cuddBddAndRecur( dd, bA0, Cudd_Not(bB1) );
        if ( bG0 == NULL ) return NULL;
        cuddRef( bG0 );

        /* P0 = IrrCover( g0, F12(x=0) ) */
        zRes0 = extraZddIsopCoverReduced( dd, bG0, bB0 );
        if ( zRes0 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            return NULL;
        }
        cuddRef( zRes0 );
        Cudd_IterDerefBdd(dd, bG0);


        /* g0 = F1(x=1) & !F12(x=0) */
        bG1 = cuddBddAndRecur( dd, bA1, Cudd_Not(bB0) );
        if ( bG1 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( bG1 );

        /* P1 = IrrCover( g1, F12(x=1) ) */
        zRes1 = extraZddIsopCoverReduced( dd, bG1, bB1 );
        if ( zRes1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( zRes1 );
        Cudd_IterDerefBdd(dd, bG1);

        /* h1 = F1(x=0) & !bdd(P0)  +   F1(x=1) & !bdd(P1) */
        bG0 = extraZddConvertToBddAndAdd( dd, zRes0, Cudd_Not(bA0) );
        if ( bG0 == NULL )
        {
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG0 );

        bG1 = extraZddConvertToBddAndAdd( dd, zRes1, Cudd_Not(bA1) );
        if ( bG1 == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG1 );

        bHA = cuddBddAndRecur( dd, bG0, bG1 );
        if ( bHA == NULL )
        {
            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHA );
        bHA = Cudd_Not(bHA);
        Cudd_IterDerefBdd(dd, bG0);
        Cudd_IterDerefBdd(dd, bG1);

        /* h12 = F12(x=0) & F12(x=1) */
        bHB = cuddBddAndRecur( dd, bB0, bB1 );
        if ( bHB == NULL )
        {
            Cudd_IterDerefBdd(dd, bHA);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHB );

        /* P1 = IrrCover( h1, h12 ) */
        zRes2 = extraZddIsopCoverReduced( dd, bHA, bHB );
        if ( zRes2 == NULL )
        {
            Cudd_IterDerefBdd(dd, bHA);
            Cudd_IterDerefBdd(dd, bHB);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( zRes2 );
        Cudd_IterDerefBdd(dd, bHA);
        Cudd_IterDerefBdd(dd, bHB);

        /* --------------- compose the result ------------------ */
        zRes = extraComposeCover( dd, zRes0, zRes1, zRes2, TopVar );
        if ( zRes == NULL ) return NULL;

        /* insert the result into cache and return */
        cuddCacheInsert2(dd, extraZddIsopCoverReduced, bA, bB, zRes);
        return zRes;
    }
} /* end of extraZddIsopCoverReduced */


/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/


/**Function********************************************************************

  Synopsis    [Implements the recursive step of Extra_zddIsopCoverRandom.]

  Description []

  SideEffects [None]

  SeeAlso     []

******************************************************************************/
static DdNode *
extraZddIsopCoverRandom(
  DdManager * dd,        /* DD manager */
  DdHashTable * table,   /* computed table */
  DdNode * bA,           /* BDD for the on-set */
  DdNode * bB,           /* BDD for the (on+dc)-set */ 
  int * permut,          /* the permutation of variables */
  int   cVar             /* the current variable */ )
{
    DdNode  *zRes;
    DdNode * ba, * bb;
    /*  statLine(dd); */

    /* if there is no area (the cover should be empty), there is nowhere to expand */
    if ( bA == b0 )    return z0;
    /* if the area is the total boolean space, the cover is expanded into a single large cube */
    if ( bB == b1 )    return z1;

    ba = Cudd_Regular(bA);
    bb = Cudd_Regular(bB);

    /* check cache */
    if ( (ba->ref != 1 || bb->ref != 1) && 
         (zRes = cuddHashTableLookup2(table, bA, bB)) != NULL )
        return zRes;
    else
    {
        DdNode *bA0, *bA1, *bB0, *bB1, *bG0, *bG1, *bHA, *bHB;
        DdNode *zRes0, *zRes1, *zRes2, *zTemp;
        int Counter = 0;
        int varCur = permut[cVar];
        assert( varCur >= 0 && varCur < dd->size );

        /* cofactor the functions */
        bA0 = cuddCofactorRecur( dd, bA, Cudd_Not(dd->vars[varCur]) );
        if ( bA0 == NULL ) 
            return NULL;
        cuddRef( bA0 );

        bA1 = cuddCofactorRecur( dd, bA,          dd->vars[varCur]  );
        if ( bA1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            return NULL;
        }
        cuddRef( bA1 );

        bB0 = cuddCofactorRecur( dd, bB, Cudd_Not(dd->vars[varCur]) );
        if ( bB0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            return NULL;
        }
        cuddRef( bB0 );

        bB1 = cuddCofactorRecur( dd, bB,          dd->vars[varCur]  );
        if ( bB1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            return NULL;
        }
        cuddRef( bB1 );
        /* at this point, bA0, bA1, bB0, bB1 are referenced */


        /*=====================================================*/
        /* g0 = F1(x=0) & !F12(x=1) */
        bG0 = cuddBddAndRecur( dd, bA0, Cudd_Not(bB1) );
        if ( bG0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);
            return NULL;
        }
        cuddRef( bG0 );

        /* P0 = IrrCover( g0, F12(x=0) ) */
        zRes0 = extraZddIsopCoverRandom(dd,table,bG0,bB0,permut,cVar+1);
        if ( zRes0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bG0);
            return NULL;
        }
        cuddRef( zRes0 );
        Cudd_IterDerefBdd(dd, bG0);


        /* g0 = F1(x=1) & !F12(x=0) */
        bG1 = cuddBddAndRecur( dd, bA1, Cudd_Not(bB0) );
        if ( bG1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( bG1 );

        /* P1 = IrrCover( g1, F12(x=1) ) */
        zRes1 = extraZddIsopCoverRandom(dd,table,bG1,bB1,permut,cVar+1);
        if ( zRes1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            return NULL;
        }
        cuddRef( zRes1 );
        Cudd_IterDerefBdd(dd, bG1);

        /* h1 = F1(x=0) & !bdd(P0)  +   F1(x=1) & !bdd(P1) */
        bG0 = extraZddConvertToBddAndAdd( dd, zRes0, Cudd_Not(bA0) );
        if ( bG0 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG0 );

        bG1 = extraZddConvertToBddAndAdd( dd, zRes1, Cudd_Not(bA1) );
        if ( bG1 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bG0);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bG1 );

        bHA = cuddBddAndRecur( dd, bG0, bG1 );
        if ( bHA == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bG0);
            Cudd_IterDerefBdd(dd, bG1);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHA );
        bHA = Cudd_Not(bHA);
        Cudd_IterDerefBdd(dd, bG0);
        Cudd_IterDerefBdd(dd, bG1);

        /* h12 = F12(x=0) & F12(x=1) */
        bHB = cuddBddAndRecur( dd, bB0, bB1 );
        if ( bHB == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bHA);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( bHB );

        /* P1 = IrrCover( h1, h12 ) */
        zRes2 = extraZddIsopCoverRandom(dd,table,bHA,bHB,permut,cVar+1);
        if ( zRes2 == NULL )
        {
            Cudd_RecursiveDeref(dd, bA0);
            Cudd_RecursiveDeref(dd, bA1);
            Cudd_RecursiveDeref(dd, bB0);
            Cudd_RecursiveDeref(dd, bB1);

            Cudd_IterDerefBdd(dd, bHA);
            Cudd_IterDerefBdd(dd, bHB);
            Cudd_RecursiveDerefZdd(dd, zRes0);
            Cudd_RecursiveDerefZdd(dd, zRes1);
            return NULL;
        }
        cuddRef( zRes2 );
        Cudd_IterDerefBdd(dd, bHA);
        Cudd_IterDerefBdd(dd, bHB);

        /*=====================================================*/
        Cudd_RecursiveDeref(dd, bA0);
        Cudd_RecursiveDeref(dd, bA1);
        Cudd_RecursiveDeref(dd, bB0);
        Cudd_RecursiveDeref(dd, bB1);

        /* --------------- compose the result ------------------ */
        if (zRes0 != z0) 
        {
            zTemp = cuddZddGetNodeIVO(dd, varCur * 2 + 1, zRes0, zRes2);
            if (zTemp == NULL) 
            {
                Cudd_RecursiveDerefZdd(dd, zRes0);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                Cudd_RecursiveDerefZdd(dd, zRes2);
                return NULL;
            }
        }
        else 
            zTemp = zRes2;
        cuddRef( zTemp );
        Cudd_RecursiveDerefZdd( dd, zRes0 );
        Cudd_RecursiveDerefZdd( dd, zRes2 );

        if (zRes1 != z0) 
        {
            zRes = cuddZddGetNodeIVO(dd, varCur * 2, zRes1, zTemp);
            if (zRes == NULL) 
            {
                Cudd_RecursiveDerefZdd(dd, zTemp);
                Cudd_RecursiveDerefZdd(dd, zRes1);
                return NULL;
            }
        }
        else 
            zRes = zTemp;
        cuddRef( zRes );
        Cudd_RecursiveDerefZdd( dd, zTemp );
        Cudd_RecursiveDerefZdd( dd, zRes1 );

        /* Do not keep the result if the reference count is only 1, since
        ** it will not be visited again.
        */
        if ( ba->ref != 1 || bb->ref != 1 )
        {
            ptrint fanout = (ptrint) ba->ref * bb->ref;
            cuddSatDec(fanout);
            if (!cuddHashTableInsert2(table, bA, bB, zRes, fanout)) {
                Cudd_RecursiveDerefZdd(dd, zRes);
                return(NULL);
            }
        }
        cuddDeref( zRes );
        return zRes;
        /*=====================================================*/
    }
} /* end of extraZddIsopCoverRandom */




/**Function********************************************************************

  Synopsis    [Generates a random permutation of numbers.]

  Description []

  SideEffects [] 

  SeeAlso     [Extra_zddIsopCover]

******************************************************************************/

int * extraGenerateRandomPermutation( DdManager * dd, int nVars )
{
    int f, g, Part, numRand, numCur, countNums = 0;
    int * pIn  = (int*) malloc( nVars * sizeof(int) );
    int * pOut = (int*) malloc( nVars * sizeof(int) );

    /* start the input number array */
    for ( f = 0; f < nVars; f++ )
        pIn[f] = f;

    /* determine the range accounting for one number */
    Part = (int)(2147483561.0/nVars);
    for ( f = 0; f < nVars; f++ )
    {
        // find the random variable
        numRand = (int)Cudd_Random( dd )/Part;
        // find the next available output
        for ( g = numRand; g < numRand+nVars; g++ )
            if ( pIn[g%nVars] != -1 )
                break;
        assert( pIn[g%nVars] != -1 );

        numCur = g%nVars;

        // write the number into the permutation array
        pOut[countNums]  = pIn[numCur];   
        // remove the number from the input array
        pIn[numCur] = -1;
        // increment the counter of numbers
        countNums++;

//      printf( "#%d: Num = %d\n", countNums, numCur );
    }
    assert( countNums == nVars );

    free( pIn );
    return pOut;
}

/**Function********************************************************************

  Synopsis    [Shuts down a hash table.]

  Description [Shuts down a hash table, dereferencing all the values.
  
  Basically the same as cuddHashTableQuit(), only uses Cudd_RecursiveDerefZdd
  instead of Cudd_RecursiveDeref. ]

  SideEffects [None]

  SeeAlso     [cuddHashTableInit]

******************************************************************************/
void
cuddHashTableQuitZdd2(
  DdHashTable * hash)
{
#ifdef __osf__
#pragma pointer_size save
#pragma pointer_size short
#endif
    unsigned int i;
    DdManager *dd = hash->manager;
    DdHashItem *bucket;
    DdHashItem **memlist, **nextmem;
    unsigned int numBuckets = hash->numBuckets;

    for (i = 0; i < numBuckets; i++) {
    bucket = hash->bucket[i];
    while (bucket != NULL) {
        Cudd_RecursiveDerefZdd(dd, bucket->value);
        bucket = bucket->next;
    }
    }

    memlist = hash->memoryList;
    while (memlist != NULL) {
    nextmem = (DdHashItem **) memlist[0];
    FREE(memlist);
    memlist = nextmem;
    }

    FREE(hash->bucket);
    FREE(hash);
#ifdef __osf__
#pragma pointer_size restore
#endif

    return;

} /* end of cuddHashTableQuitZdd2 */

