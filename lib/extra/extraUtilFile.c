/**CFile****************************************************************

  FileName    [extraUtilFile.c]

  PackageName [extra]

  Synopsis    [Various reusable software utilities.]

  Author      [Alan Mishchenko]
  
  Affiliation [UC Berkeley]

  Date        [Ver. 1.0. Started - September 1, 2003.]

  Revision    [$Id: extraUtilFile.c,v 1.0 2003/09/01 00:00:00 alanmi Exp $]

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


/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
int Extra_FileNameCheckExtension( char * FileName, char * Extension )
{
    char * pDot;
    // find "dot" if it is present in the file name
    pDot = strstr( FileName, "." );
    if ( pDot && strcmp( pDot+1, Extension ) == 0 )
        return 1;
    else
        return 0;
}

/**Function*************************************************************

  Synopsis    [Returns the composite name of the file.]

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
char * Extra_FileNameAppend( char * pBase, char * pSuffix )
{
    static char Buffer[100];
    sprintf( Buffer, "%s%s", pBase, pSuffix );
    return Buffer;
}

/**Function*************************************************************

  Synopsis    []

  Description []

  SideEffects []

  SeeAlso     []

***********************************************************************/
char * Extra_FileNameGeneric( char * FileName )
{
    char * pDot;
    char * pUnd;
    char * pRes;
    
    // find the generic name of the file
    pRes = util_strsav( FileName );
    // find the pointer to the "." symbol in the file name
//  pUnd = strstr( FileName, "_" );
    pUnd = NULL;
    pDot = strstr( FileName, "." );
    if ( pUnd )
        pRes[pUnd - FileName] = 0;
    else if ( pDot )
        pRes[pDot - FileName] = 0;
    return pRes;
}

/*---------------------------------------------------------------------------*/
/* Definition of internal functions                                          */
/*---------------------------------------------------------------------------*/

/*---------------------------------------------------------------------------*/
/* Definition of static Functions                                            */
/*---------------------------------------------------------------------------*/


////////////////////////////////////////////////////////////////////////
///                       END OF FILE                                ///
////////////////////////////////////////////////////////////////////////


