/*
 *	File: common_argument_parser.impl.hpp
 *
 *	Created on: 25 Nov, 2016
 *	Created by: Davit Kalantaryan (Email: davit.kalantaryan@desy.de)
 *
 *  This file implements the class for parsing 
 *  command line arguments
 *  boost has classes for this purpose,
 *  but sometime dependency from boost is not needed
 *
 */
#ifndef __common_argument_parser_impl_hpp__
#define __common_argument_parser_impl_hpp__

#ifndef __common_argument_parser_hpp__
#error Direct include not allowed!
#endif
#ifdef development_stage
#include "argument_parser.hpp"
#endif

#ifdef __cplusplus


#if 0
template <typename TypeArgc, typename TypeArgv>
void common::argument_parser::ParseCommandLine(TypeArgc a_argc, TypeArgv a_argv[])
{
	SInput aInput;
    const char *aOption0, *aOptionVal;

    for (int i(0); i<((int)a_argc);)
	{
        //pnIsArg = m_htOptionsIn.LookupStr(a_argv[i]);
        if (m_htOptionsIn2.count(a_argv[i]))
		{
			aInput = m_htOptionsIn2[a_argv[i]];
			aOption0 = a_argv[i];
			aOptionVal = aInput.defaultValue.c_str();
			if (i < (--a_argc))
			{
				memmove(a_argv + i, a_argv + i + 1, (a_argc - i)*sizeof(TypeArgv));
                if ((aInput.isArg)&&((a_argv[i])[0]!='-')&&(i<(a_argc--)))
				{
					aOptionVal = a_argv[i];
					memmove(a_argv + i, a_argv + i + 1, (a_argc - i)*sizeof(TypeArgv)); 
				}
			}
            m_htOptionsFound.insert(std::pair<std::string, std::string>(aOption0,aOptionVal));
		}
		else  // if (pnIsArg)
		{
			++i;
		}
	}// for (int i(0); i<a_argc;)
}
#endif  //  #if 0


#else   // #ifdef __cplusplus
#endif  // #ifdef __cplusplus


#endif  // #ifndef __common_argument_parser_impl_hpp__
