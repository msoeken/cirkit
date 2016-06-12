/**CFile****************************************************************

  FileName    [extraUtilProgress.c]

  PackageName [extra]

  Synopsis    [Progress bar.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 2.0. Started - September 1, 2003.]

  Revision    [$Id: extraUtilProgress.c,v 1.0 2003/02/01 00:00:00 alanmi Exp $]

***********************************************************************/

#include "stdio.h"
#include "extra.h"

////////////////////////////////////////////////////////////////////////
///                        DECLARATIONS                              ///
////////////////////////////////////////////////////////////////////////

struct ProgressBarStruct
{
    FILE *           pFile;
    int              nItemsTotal;
    int              posTotal;
    int              posCur;
};

static void Extra_ProgressBarShow( ProgressBar * p );
static void Extra_ProgressBarClean( ProgressBar * p );

////////////////////////////////////////////////////////////////////////
///                     FUNCTION DEFITIONS                           ///
////////////////////////////////////////////////////////////////////////

/**Function*************************************************************

  Synopsis    [Starts the progress bar.]

  Description [The first parameter is the output stream (pFile), where
  the progress is printed. The current printing position should be the
  first one on the given line. The second parameters is the total
  number of items that correspond to 100% position of the progress bar.]
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
ProgressBar * Extra_ProgressBarStart( FILE * pFile, int nItemsTotal )
{
    ProgressBar * p;
    p = ALLOC( ProgressBar, 1 );
    memset( p, 0, sizeof(ProgressBar) );
    p->pFile       = pFile;
    p->nItemsTotal = nItemsTotal;
    p->posTotal    = 78;
    p->posCur      = 1;
    Extra_ProgressBarShow( p );
    return p;
}

/**Function*************************************************************

  Synopsis    [Updates the progress bar.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_ProgressBarUpdate( ProgressBar * p, int nItemsCur )
{
    if ( nItemsCur > p->nItemsTotal )
        nItemsCur = p->nItemsTotal;
    p->posCur = nItemsCur * p->posTotal / p->nItemsTotal;
    if ( p->posCur == 0 )
        p->posCur = 1;
    Extra_ProgressBarShow( p );
}


/**Function*************************************************************

  Synopsis    [Stops the progress bar.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_ProgressBarStop( ProgressBar * p )
{
    Extra_ProgressBarClean( p );
    FREE( p );
}

/**Function*************************************************************

  Synopsis    [Prints the progress bar of the given size.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_ProgressBarShow( ProgressBar * p )
{
    int i;
    for ( i = 0; i < p->posCur; i++ )
        fprintf( p->pFile, "-" );
    fprintf( p->pFile, ">" );
    for ( i++  ; i <= p->posTotal; i++ )
        fprintf( p->pFile, " " );
    fprintf( p->pFile, "\r" );
    fflush( stdout );
}

/**Function*************************************************************

  Synopsis    [Cleans the progress bar before quitting.]

  Description []
               
  SideEffects []

  SeeAlso     []

***********************************************************************/
void Extra_ProgressBarClean( ProgressBar * p )
{
    int i;
    for ( i = 0; i <= p->posTotal; i++ )
        fprintf( p->pFile, " " );
    fprintf( p->pFile, "\r" );
    fflush( stdout );
}

////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


