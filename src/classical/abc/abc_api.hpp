/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2017  EPFL
 *
 * Permission is hereby granted, free of charge, to any person
 * obtaining a copy of this software and associated documentation
 * files (the "Software"), to deal in the Software without
 * restriction, including without limitation the rights to use,
 * copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following
 * conditions:
 *
 * The above copyright notice and this permission notice shall be
 * included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES
 * OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
 * NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT
 * HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
 * OTHER DEALINGS IN THE SOFTWARE.
 */

/**
 * @file abc_api.hpp
 *
 * @brief ABC interface
 *
 * Interface to ABC
 *
 * @author Heinz Riener
 * @since  2.3
 */

#ifndef ABC_API_HPP
#define ABC_API_HPP

#define LIN64
#include <base/main/main.h>
#include <aig/aig/aig.h>
#include <aig/gia/gia.h>
#include <misc/util/abc_global.h>
#include <sat/cnf/cnf.h>

namespace abc {
void Abc_UtilsPrintHello( Abc_Frame_t * pAbc );
void Abc_UtilsSource( Abc_Frame_t * pAbc );
char * Abc_UtilsGetUsersInput( Abc_Frame_t * pAbc );
void Mf_ManDumpCnf( Gia_Man_t * p, char * pFileName, int nLutSize, int fCnfObjIds, int fAddOrCla, int fVerbose );
void * Mf_ManGenerateCnf( Gia_Man_t * pGia, int nLutSize, int fCnfObjIds, int fAddOrCla, int fMapping, int fVerbose );
Aig_Man_t * Gia_ManToAig( Gia_Man_t * p, int fChoices );
Gia_Man_t * Gia_ManFromAig( Aig_Man_t * p );
Abc_Ntk_t * Abc_NtkFromAigPhase( Aig_Man_t * pMan );
Aig_Man_t * Abc_NtkToDar( Abc_Ntk_t * pNtk, int fExors, int fRegisters );
}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
