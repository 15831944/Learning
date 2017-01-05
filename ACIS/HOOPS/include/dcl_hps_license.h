/*******************************************************************/
/*    Copyright (c) 1989-2015 by Spatial Corp.                     */
/*    All rights reserved.                                         */
/*    Protected by U.S. Patents 5,257,205; 5,351,196; 6,369,815;   */
/*                              5,982,378; 6,462,738; 6,941,251    */
/*    Protected by European Patents 0503642; 69220263.3            */
/*    Protected by Hong Kong Patent 1008101A                       */
/*******************************************************************/
#ifndef DECL_HPS_LICENSE

#ifdef _MSC_VER
# ifdef __hps_license
#  define DECL_HPS_LICENSE __declspec(dllexport)
# else
#  define DECL_HPS_LICENSE __declspec(dllimport)
# endif
#else
# define DECL_HPS_LICENSE
#endif

#endif /* DECL_HPS_LICENSE */
