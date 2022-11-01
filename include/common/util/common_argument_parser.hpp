/*
 *	File: argument_parser.hpp
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
#ifndef __common_argument_parser_hpp__
#define __common_argument_parser_hpp__

#ifdef __cplusplus

#include <string>
#include <map>
#include <vector>

#define SHORTCUTS_DELIM_SYMBOL2	','
#define INFO_START_DELIM		':'

typedef class CInputBase*		TInputBase;
//typedef char*					TcharPtr;
struct InfoAndCleanPair;

namespace common
{
	namespace argumentParser{namespace argType { enum Type { noArg, rightArg, leftArg, bougthArg }; }}
	class argument_parser
	{
	public:
		argument_parser();
		virtual ~argument_parser();

		argument_parser& AddOption(
			const std::string& optionName, 
			argumentParser::argType::Type isArg= argumentParser::argType::rightArg,
			const char* defLeft=NULL, const char* defRight=NULL); // adding new option
		argument_parser& operator<<(const std::string& optionName); // adding new option
        const char* operator[](const char* option_name); // find option Right 
		const char* valueLeft(const char* option_name); // find option Left value
		void	ParseCommandLine(int& argc, char** argv);
        std::string	HelpString()const;
		void Clear();

	private:
		size_t								m_unMaxForHelp;
        std::map<std::string,TInputBase>	m_htOptionsInAndOut;
		std::vector<InfoAndCleanPair>		m_vectForHelp;
	};

}

//#include "common_argument_parser.impl.hpp"


#else   // #ifdef __cplusplus
#endif  // #ifdef __cplusplus


#endif  // #ifndef __common_argument_parser_hpp__
