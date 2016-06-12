/**CFile****************************************************************

  FileName    [extraZddPermute.c]

  PackageName [extra]

  Synopsis    [Procedures to perform variable permutation in ZDDs.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraZddPermute.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

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

static DdNode * extraZddPermuteRecur  (DdManager * dd, DdHashTable * table, DdNode * N, int * permut);
static int      zddCheckPositiveCube (DdManager * dd, DdNode * cube);
static DdNode * cuddZddGetNodeIVO2( DdManager * dd, int  index, DdNode * g, DdNode * h );
static void     cuddHashTableQuitZdd2(DdHashTable * hash);


/**AutomaticEnd***************************************************************/


/*---------------------------------------------------------------------------*/
/* Definition of exported functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis [Existentially abstracts all the variables in cube from f.]

  Description [Existentially abstracts all the variables in cube from f.
  Returns the abstracted ZDD if successful; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_bddExistAbstract Cudd_bddUnivAbstract Cudd_addExistAbstract]

******************************************************************************/
DdNode *
Extra_zddExistAbstract(
  DdManager * dd,
  DdNode * f,
  DdNode * cube)
{
    DdNode *res;

    if (zddCheckPositiveCube(dd, cube) == 0) 
    {
        (void) fprintf(dd->err, "Error: Extra_zddExistAbstract() can only abstract positive cubes\n");
        dd->errorCode = CUDD_INVALID_ARG;
        return(NULL);
    }

    do {
    dd->reordered = 0;
    res = extraZddExistAbstractRecur(dd, f, cube);
    } while (dd->reordered == 1);

    return(res);

} /* end of Extra_zddExistAbstract */


/**Function********************************************************************

  Synopsis    [Permutes the variables of a ZDD.]

  Description [Given a permutation in array permut, creates a new ZDD
  with permuted variables. There should be an entry in array permut
  for each variable in the manager. The i-th entry of permut holds the
  index of the variable that is to substitute the i-th variable.
  Returns a pointer to the resulting ZDD if successful; NULL
  otherwise.]

  SideEffects [None]

  SeeAlso     [Cudd_addPermute Cudd_bddPermute Cudd_bddSwapVariables]

******************************************************************************/
DdNode *
Extra_zddPermute(
  DdManager * dd,
  DdNode * node,
  int * permut)
{
    DdHashTable     *table;
    DdNode          *res;

    do {

    dd->reordered = 0;
    table = cuddHashTableInit(dd,1,2);
    if (table == NULL) return(NULL);
    res = extraZddPermuteRecur(dd,table,node,permut);
    if (res != NULL) cuddRef(res);
    /* Dispose of local cache. */
    cuddHashTableQuitZdd2(table); 

    } while (dd->reordered == 1);

    if (res != NULL) cuddDeref(res);
    return(res);

} /* end of Extra_zddPermute */


/**Function********************************************************************

  Synopsis [Changes the value of several variables in the ZDD.]

  Description [Returns the ZDD obtained from F by changing the value 
  of variables in the cube, if successful; NULL otherwise. ]

  SideEffects [None]

  SeeAlso     [Cudd_bddExistAbstract Cudd_bddUnivAbstract Cudd_addExistAbstract]

******************************************************************************/
DdNode *
Extra_zddChangeVars(
  DdManager * dd,
  DdNode * f,
  DdNode * cube)
{
    DdNode *res;

    if (zddCheckPositiveCube(dd, cube) == 0) 
    {
        (void) fprintf(dd->err, "Error: Extra_zddChangeVars() can only abstract positive cubes\n");
        dd->errorCode = CUDD_INVALID_ARG;
        return(NULL);
    }

    do {
    dd->reordered = 0;
    res = extraZddChangeVars(dd, f, cube);
    } while (dd->reordered == 1);

    return(res);

} /* end of Extra_zddChangeVars */


/**Function********************************************************************

  Synopsis    [Computes combinations in F with vars in Cube having the negative polarity.]

  Description [Returns the ZDD of those assignments in F that have variables in 
  Cube in the negative polarity; NULL otherwise.]

  SideEffects [None]

  SeeAlso     [Extra_zddCofactor0]

******************************************************************************/
DdNode *
Extra_zddCofactor0(
  DdManager * dd,
  DdNode * f,
  DdNode * cube)
{
    DdNode *res;

    if (zddCheckPositiveCube(dd, cube) == 0) 
    {
        (void) fprintf(dd->err, "Error: Extra_zddCofactor0() found that the variable set is not a cube\n");
        dd->errorCode = CUDD_INVALID_ARG;
        return(NULL);
    }

    do {
    dd->reordered = 0;
    res = extraZddCofactor0(dd, f, cube);
    } while (dd->reordered == 1);

    return(res);

} /* end of Extra_zddCofactor1 */


/**Function********************************************************************

  Synopsis    [Computes combinations in F with vars in Cube having the positive polarity.]

  Description [Returns the ZDD of those assignments in F that have variables in 
  Cube in the positive polarity. If the flag fIncludeVars is 1, the resulting combinations
  contain the variables w.r.t. which the cofactoring took place. If fIncludeVars is 0,
  the positive polarity variables are not included in the set of combinations. (Notice
  that there is no need for a similar flag in Extra_zddCofactor0() because due to the ZDD
  reduction rules, the negative polarity variables are missing in the resulting assignments).]

  SideEffects [None]

  SeeAlso     [Extra_zddCofactor0]

******************************************************************************/
DdNode *
Extra_zddCofactor1(
  DdManager * dd,
  DdNode * f,
  DdNode * cube,
  int fIncludeVars)
{
    DdNode *res;

    if (zddCheckPositiveCube(dd, cube) == 0) 
    {
        (void) fprintf(dd->err, "Error: Extra_zddCofactor1() found that the variable set is not a cube\n");
        dd->errorCode = CUDD_INVALID_ARG;
        return(NULL);
    }

    do {
    dd->reordered = 0;
    res = extraZddCofactor1(dd, f, cube, fIncludeVars);
    } while (dd->reordered == 1);

    return(res);

} /* end of Extra_zddCofactor1 */




/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Performs the recursive steps of Extra_zddExistAbstract.]

  Description [Performs the recursive steps of Extra_zddExistAbstract.
  Returns the ZDD obtained by removing the variables of cube from f 
  if successful; NULL otherwise. ]

  SideEffects [None]

  SeeAlso     [Cudd_bddExistAbstract Cudd_bddUnivAbstract]

******************************************************************************/
DdNode *
extraZddExistAbstractRecur(
  DdManager * dd,
  DdNode * F,
  DdNode * cube)
{
    DdNode *zRes;
    /* statLine(dd); */
    /* Cube is guaranteed to be a cube at this point. */    
    if (F == dd->one || F == dd->zero || cube == dd->one )   
        return(F);    
    /* From now on, F and cube are non-constant. */

    /* Abstract a variable that does not appear in F. */
    while ( dd->permZ[F->index] > dd->permZ[cube->index] ) 
    {
        cube = cuddT(cube);
        if (cube == dd->one) 
            return(F);
    }

    /* Check the cache. */
    if (F->ref != 1 && (zRes = cuddCacheLookup2Zdd(dd, extraZddExistAbstractRecur, F, cube)) != NULL) 
        return(zRes);
    else
    {
        DdNode *zRes0, *zRes1;

        /* If the two indices are the same, so are their levels. */
        if (F->index == cube->index) 
        {
            /* solve the problem for the else branch */
            zRes0 = extraZddExistAbstractRecur(dd, cuddE(F), cuddT(cube));
            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* solve the problem for the then branch */
            zRes1 = extraZddExistAbstractRecur(dd, cuddT(F), cuddT(cube));
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            /* find the result by computing the union of these two sets */
            zRes = cuddZddUnion( dd, zRes0, zRes1 );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddRef( zRes );
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            Cudd_RecursiveDerefZdd( dd, zRes1 );
            cuddDeref( zRes );
        }
        else /* if (cuddIZ(dd,F->index) < cuddIZ(dd,cube->index)) */
        {
            /* solve the problem for the else branch */
            zRes0 = extraZddExistAbstractRecur(dd, cuddE(F), cube);
            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* solve the problem for the then branch */
            zRes1 = extraZddExistAbstractRecur(dd, cuddT(F), cube);
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            /* combine the solutions using the current variable */
            zRes = cuddZddGetNode( dd, F->index, zRes1, zRes0 );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddDeref( zRes0 );
            cuddDeref( zRes1 );
        }

        /* insert the result into cache */
        if (F->ref != 1)
        cuddCacheInsert2(dd, extraZddExistAbstractRecur, F, cube, zRes);
        return zRes;
    }
} /* end of extraZddExistAbstractRecur */


/**Function********************************************************************

  Synopsis    [Performs the recursive steps of Extra_zddChangeVars.]

  Description [Performs the recursive steps of Extra_zddChangeVars.
  Returns the ZDD obtained from F by changing the value of variables 
  in the cube if successful; NULL otherwise. ]

  SideEffects [None]

  SeeAlso     [Cudd_zddChange]

******************************************************************************/
DdNode *
extraZddChangeVars(
  DdManager * dd,
  DdNode * zF,
  DdNode * zCube)
{
    DdNode *zRes;
    /* statLine(dd); */
    /* cube is guaranteed to be a cube at this point. */    
    /* if there is no F or we have run out of cube vars, return F */
    assert( zCube != z0 );
    if ( zF == z0 || zCube == z1 )   
        return zF;
    /* if F is constant 1 and cube is not constant 1, return the cube */
    if ( zF == z1 )
        return zCube;
    /* From now on, F and cube are non-constant. */

    /* Check the cache. */
    if ( zF->ref != 1 && (zRes = cuddCacheLookup2Zdd(dd, extraZddChangeVars, zF, zCube)) != NULL) 
        return(zRes);
    else
    {
        DdNode * zF0,   * zF1,   * zCubeNew;
        DdNode * zRes0, * zRes1;
        int ZddVar;
        int fLev = dd->permZ[zF->index];
        int cLev = dd->permZ[zCube->index];

        /* cofactor F */
        if (fLev <= cLev) /* F is on top */ 
        { 
            ZddVar = zF->index;
            zF0 = cuddE(zF);
            zF1 = cuddT(zF);
        }
        else /* if (fLev > cLev) */  /* F is not on top */
        { 
            ZddVar = zCube->index;
            zF0 = zF;
            zF1 = z0;
        }

        /* cofactor the cube */
        if (fLev >= cLev) /* cube is on top */
            zCubeNew = cuddT(zCube);
        else /* cube is not on top */
            zCubeNew = zCube;

        /* solve the problem for the else branch */
        zRes0 = extraZddChangeVars(dd, zF0, zCubeNew);
        if ( zRes0 == NULL )
            return NULL;
        cuddRef( zRes0 );

        /* solve the problem for the then branch */
        zRes1 = extraZddChangeVars(dd, zF1, zCubeNew);
        if ( zRes1 == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            return NULL;
        }
        cuddRef( zRes1 );

        /* combine the solutions using the current variable */
        /* swap the braches if the cube variable is on top */
        if ( fLev >= cLev )
            zRes = cuddZddGetNode( dd, ZddVar, zRes0, zRes1 );
        else
            zRes = cuddZddGetNode( dd, ZddVar, zRes1, zRes0 );
        if ( zRes == NULL )
        {
            Cudd_RecursiveDerefZdd( dd, zRes0 );
            Cudd_RecursiveDerefZdd( dd, zRes1 );
            return NULL;
        }
        cuddDeref( zRes0 );
        cuddDeref( zRes1 );

        /* insert the result into cache */
        if (zF->ref != 1)
        cuddCacheInsert2(dd, extraZddChangeVars, zF, zCube, zRes);
        return zRes;
    }
} /* end of extraZddChangeVars */



/**Function********************************************************************

  Synopsis    [Performs the recursive steps of Extra_zddCofactor0.]

  Description [Performs the recursive steps of Extra_zddCofactor0.
  Returns the ZDD of those assignments in F that have variables in 
  Cube in the negative polarity; NULL otherwise. ]

  SideEffects [None]

  SeeAlso     [extraZddCofactor1]

******************************************************************************/
DdNode *
extraZddCofactor0(
  DdManager * dd,
  DdNode * zF,
  DdNode * zCube)
{
    DdNode *zRes;
    /* statLine(dd); */
    /* cube is guaranteed to be a cube at this point. */    

    /* if there is no F or we have run out of cube vars, return F */
    assert( zCube != z0 );
    if ( zF == z0 || zCube == z1 )   
        return zF;

    /* if F is constant 1 (the set composed of the empty subset only)   
    and cube is not constant 1, return constant 1, because cofactoring 
    the empty subset w.r.t. negative vars gives the empty subset */
    if ( zF == z1 )
        return z1;
    /* From now on, F and Cube are non-constant. */

    /* Check the cache. */
    if ( zF->ref != 1 && (zRes = cuddCacheLookup2Zdd(dd, extraZddCofactor0, zF, zCube)) != NULL) 
        return zRes;
    else
    {
        DdNode * zRes0, * zRes1;
        int fLev = dd->permZ[zF->index];
        int cLev = dd->permZ[zCube->index];

        if ( fLev > cLev ) /* cube is on top - F always has this var in the negative polarity */
        {
            zRes = extraZddCofactor0(dd, zF, cuddT(zCube));
            if ( zRes == NULL )
                return NULL;
        }
        else if ( fLev < cLev ) /* F is on top - we should take both branches */
        {
            /* cofactor F */
            /* solve the problem for the else branch */
            zRes0 = extraZddCofactor0(dd, cuddE(zF), zCube);
            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* solve the problem for the then branch */
            zRes1 = extraZddCofactor0(dd, cuddT(zF), zCube);
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            zRes = cuddZddGetNode( dd, zF->index, zRes1, zRes0 );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddDeref( zRes0 );
            cuddDeref( zRes1 );
        }
        else /* ( fLev == cLev ) */ /* take only the negative branch of F */
        {
            /* solve the problem for the else branch */
            zRes = extraZddCofactor0(dd, cuddE(zF), cuddT(zCube));
            if ( zRes == NULL )
                return NULL;
        }


        /* insert the result into cache */
        if (zF->ref != 1)
        cuddCacheInsert2(dd, extraZddCofactor0, zF, zCube, zRes);
        return zRes;
    }
} /* end of extraZddCofactor0 */



/**Function********************************************************************

  Synopsis    [Performs the recursive steps of Extra_zddCofactor1.]

  Description [Performs the recursive steps of Extra_zddCofactor1.
  Returns the ZDD of those assignments in F that have variables in 
  Cube in the positive polarity; NULL otherwise. ]

  SideEffects [None]

  SeeAlso     [Extra_zddCofactor1]

******************************************************************************/
DdNode *
extraZddCofactor1(
  DdManager * dd,
  DdNode * zF,
  DdNode * zCube,
  int fIncludeVars)
{
    DdNode *zRes;
    DdNode*(*cacheOp)(DdManager*,DdNode*,DdNode*);

    /* statLine(dd); */
    /* cube is guaranteed to be a cube at this point. */    

    /* if there is no F or we have run out of cube vars, return F */
    assert( zCube != z0 );
    if ( zF == z0 || zCube == z1 )   
        return zF;

    /* if F is constant 1 (the set composed of only the empty subset)   
    and cube is not constant 1, return 0, because cofactoring 
    the empty subset w.r.t. positive vars is empty */
    if ( zF == z1 )
        return z0;
    /* From now on, F and cube are non-constant. */

    /* Check the cache. */
    cacheOp = (DdNode*(*)(DdManager*,DdNode*,DdNode*))extraZddCofactor1;
    if ( zF->ref != 1 && (zRes = cuddCacheLookup2Zdd(dd, cacheOp, zF, zCube)) != NULL) 
        return zRes;
    else
    {
        DdNode * zRes0, * zRes1;
        int fLev = dd->permZ[zF->index];
        int cLev = dd->permZ[zCube->index];


        if ( fLev > cLev ) /* cube is on top - F does not have this var in the positive polarity */
        {
            zRes = z0;
        }
        else if ( fLev < cLev ) /* F is on top - we should take both branches */
        {
            /* cofactor F */
            /* solve the problem for the else branch */
            zRes0 = extraZddCofactor1(dd, cuddE(zF), zCube, fIncludeVars);
            if ( zRes0 == NULL )
                return NULL;
            cuddRef( zRes0 );

            /* solve the problem for the then branch */
            zRes1 = extraZddCofactor1(dd, cuddT(zF), zCube, fIncludeVars);
            if ( zRes1 == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                return NULL;
            }
            cuddRef( zRes1 );

            zRes = cuddZddGetNode( dd, zF->index, zRes1, zRes0 );
            if ( zRes == NULL )
            {
                Cudd_RecursiveDerefZdd( dd, zRes0 );
                Cudd_RecursiveDerefZdd( dd, zRes1 );
                return NULL;
            }
            cuddDeref( zRes0 );
            cuddDeref( zRes1 );
        }
        else /* ( fLev == cLev ) */ /* take only the positive branch of F */
        {
            /* solve the problem for the then branch */
            if ( fIncludeVars )
            { /* this option includes the variables w.r.t. which
                 cofactoring has taken place, back into the set 
                 in the positive polarity */

                zRes1 = extraZddCofactor1(dd, cuddT(zF), cuddT(zCube), fIncludeVars);
                if ( zRes1 == NULL )
                    return NULL;
                cuddRef( zRes1 );

                // add the missing variable in the positive polarity
                zRes = cuddZddGetNode( dd, zF->index, zRes1, z0 );
                if ( zRes == NULL )
                {
                    Cudd_RecursiveDerefZdd( dd, zRes1 );
                    return NULL;
                }
                cuddDeref( zRes1 );
            }
            else
            { /* this option permanently removes the variables w.r.t. which 
                 the cofactoring has taken place */

                zRes = extraZddCofactor1(dd, cuddT(zF), cuddT(zCube), fIncludeVars);
                if ( zRes == NULL )
                    return NULL;
            }
        }


        /* insert the result into cache */
        if (zF->ref != 1)
        cuddCacheInsert2(dd, cacheOp, zF, zCube, zRes);
        return zRes;
    }
} /* end of extraZddCofactor1 */



/*---------------------------------------------------------------------------*/
/* Definition of static functions                                            */
/*---------------------------------------------------------------------------*/

/**Function********************************************************************

  Synopsis    [Implements the recursive step of Extra_zddPermute.]

  Description [ Recursively puts the ZDD in the order given in the array permut.
  Checks for trivial cases to terminate recursion, then splits on the
  children of this node.  Once the solutions for the children are
  obtained, it puts into the current position the node from the rest of
  the ZDD that should be here. Then returns this ZDD.
  The key here is that the node being visited is NOT put in its proper
  place by this instance, but rather is switched when its proper position
  is reached in the recursion tree.<p>
  The DdNode * that is returned is the same ZDD as passed in as node,
  but in the new order.]

  SideEffects [None]

  SeeAlso     [Extra_zddPermute cuddBddPermuteRecur]

******************************************************************************/
static DdNode *
extraZddPermuteRecur(
  DdManager * dd /* DD manager */,
  DdHashTable * table /* computed table */,
  DdNode * N /* ZDD to be reordered */,
  int * permut /* permutation array */)
{
    DdNode  *T,*E, *res;

    /*  statLine(dd); */
    /* Check for terminal case of constant node. */
    if (cuddIsConstant(N)) 
        return(N);

    /* If problem already solved, look up answer and return. */
    if (N->ref != 1 && (res = cuddHashTableLookup1(table,N)) != NULL) 
        return (res);
 
    /* Split and recur on children of this node. */
    T = extraZddPermuteRecur(dd,table,cuddT(N),permut);
    if (T == NULL) return(NULL);
    cuddRef(T);
    E = extraZddPermuteRecur(dd,table,cuddE(N),permut);
    if (E == NULL) {
        Cudd_RecursiveDerefZdd(dd, T);
        return(NULL);
    }
    cuddRef(E);

    /* Move variable that should be in this position to this position
    ** by retrieving the single var ZDD for that variable, and calling
    ** cuddZddGetNodeIVO with the T and E we just created.
    */
    res = cuddZddGetNodeIVO2(dd,permut[N->index],T,E);
    if (res == NULL) {
        Cudd_RecursiveDerefZdd(dd, T);
        Cudd_RecursiveDerefZdd(dd, E);
        return(NULL);
    }
    cuddRef(res);
    Cudd_RecursiveDerefZdd(dd, T);
    Cudd_RecursiveDerefZdd(dd, E);

    /* Do not keep the result if the reference count is only 1, since
    ** it will not be visited again.
    */
    if (N->ref != 1) {
        ptrint fanout = (ptrint) N->ref;
        cuddSatDec(fanout);
        if (!cuddHashTableInsert1(table,N,res,fanout)) {
            Cudd_RecursiveDerefZdd(dd, res);
            return(NULL);
        }
    }
    cuddDeref(res);
    return(res);

} /* end of extraZddPermuteRecur */


/**Function********************************************************************

  Synopsis [Checks whether cube is a ZDD representing a single subset of elements]

  Description [Returns 1 in case of success; 0 otherwise.]

  SideEffects [None]

******************************************************************************/
static int
zddCheckPositiveCube(
  DdManager * dd,
  DdNode * cube)
{
    while ( !cuddIsConstant(cube) )
    {
        if ( cuddE(cube) != z0 )
            return 0;
        cube = cuddT(cube);
    }
    return (int)( cube == z1 );
} /* end of zddCheckPositiveCube */

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


/**Function********************************************************************

  Synopsis [Wrapper for cuddUniqueInterZdd that is independent of variable
  ordering.]

  Description [Wrapper for cuddUniqueInterZdd that is independent of
  variable ordering (IVO). This function does not require parameter
  index to precede the indices of the top nodes of g and h in the
  variable order.  Returns a pointer to the result node under normal
  conditions; NULL if reordering occurred or memory was exhausted.
  
  This function is the same as cuddZddGetNodeIVO(), except for the call to
  cuddZddChangeAux() instead of cuddZddProduct(). This implementation 
  work for this case as well as for cuddZddIsop() (not verified!)]

  SideEffects [None]

  SeeAlso     [cuddZddGetNodeIVO cuddZddGetNode cuddZddIsop]

******************************************************************************/
static DdNode *
cuddZddGetNodeIVO2(
  DdManager * dd,
  int  index,
  DdNode * g,
  DdNode * h)
{
    DdNode  *f, *r, *t;
    DdNode  *zdd_one = DD_ONE(dd);
    DdNode  *zdd_zero = DD_ZERO(dd);

    f = cuddUniqueInterZdd(dd, index, zdd_one, zdd_zero);
    if (f == NULL) {
    return(NULL);
    }
    cuddRef(f);
    t = cuddZddChangeAux(dd, g, f);
    if (t == NULL) {
    Cudd_RecursiveDerefZdd(dd, f);
    return(NULL);
    }
    cuddRef(t);
    Cudd_RecursiveDerefZdd(dd, f);
    r = cuddZddUnion(dd, t, h);
    if (r == NULL) {
    Cudd_RecursiveDerefZdd(dd, t);
    return(NULL);
    }
    cuddRef(r);
    Cudd_RecursiveDerefZdd(dd, t);

    cuddDeref(r);
    return(r);

} /* end of cuddZddGetNodeIVO2 */
