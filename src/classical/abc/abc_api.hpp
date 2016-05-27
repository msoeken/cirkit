/* CirKit: A circuit toolkit
 * Copyright (C) 2009-2015  University of Bremen
 * Copyright (C) 2015-2016  EPFL
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
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
Cnf_Dat_t * Mf_ManGenerateCnf( Gia_Man_t * pGia, int nLutSize, int fCnfObjIds, int fAddOrCla, int fVerbose );
Aig_Man_t * Gia_ManToAig( Gia_Man_t * p, int fChoices );
Abc_Ntk_t * Abc_NtkFromAigPhase( Aig_Man_t * pMan );
}

#endif

// Local Variables:
// c-basic-offset: 2
// eval: (c-set-offset 'substatement-open 0)
// eval: (c-set-offset 'innamespace 0)
// End:
