/*
 *	File: argument_parser.cpp
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

//#include "stdafx.h"
#ifdef __cplusplus
#include "common_argument_parser.hpp"

#if defined(_MSC_VER) & (_MSC_VER>1400)
#pragma warning (disable:4996)
#endif

//#define SHORTCUTS_DELIM_SYMBOL2	','
//#define INFO_START_DELIM		':'

static const char s_vcSubStr[] = { SHORTCUTS_DELIM_SYMBOL2 ,INFO_START_DELIM,0};

class CInputBase 
{
public:
	int isFound;std::string helpStr;
	CInputBase():isFound(0){}
	virtual common::argumentParser::argType::Type getType()const {return common::argumentParser::argType::noArg;}
	virtual const char* valueRight()const { return ""; }
	virtual const char* valueLeft()const { return ""; }
};

class CInputArgRight : public CInputBase 
{
public:
	std::string defaultOrValueRight;
	CInputArgRight(const std::string& a_defRight): defaultOrValueRight(a_defRight){}
	virtual common::argumentParser::argType::Type getType()const {return common::argumentParser::argType::rightArg;}
	virtual const char* valueRight()const { return defaultOrValueRight.c_str(); }
};

class CInputArgRightAndLeft : public CInputArgRight
{
public:
	std::string defaultOrValueLeft;
	CInputArgRightAndLeft(const std::string& a_defRight,const std::string& a_defLeft)
		: CInputArgRight(a_defRight), defaultOrValueLeft(a_defLeft)
	{}
	virtual common::argumentParser::argType::Type getType()const{return common::argumentParser::argType::bougthArg;}
	virtual const char* valueLeft()const { return defaultOrValueLeft.c_str(); }
};

struct InfoAndCleanPair {
	std::string option,infoStr; CInputBase* pInfo;
	InfoAndCleanPair(CInputBase* a_pInfo):pInfo(a_pInfo){}
};

common::argument_parser::argument_parser()
{
	m_unMaxForHelp = 0;
}

common::argument_parser::~argument_parser()
{
	this->Clear();
}


common::argument_parser&  common::argument_parser::operator<<(const std::string& a_strOptionName)
{
	return AddOption(a_strOptionName, argumentParser::argType::noArg,"");
}


common::argument_parser& common::argument_parser::AddOption(
	const std::string& a_strOptionName, argumentParser::argType::Type a_argType,
	const char* a_defRight, const char* a_defLeft)
{
	char *pcOptionName, *pcNext,*pcToSet=NULL;
	size_t unHelpStart = a_strOptionName.length();
	size_t i, unStrLenPlus1;
	char* pcBuffer = (char*)malloc(unHelpStart + 1);
	InfoAndCleanPair aPair=0;
	std::string aLeft, aRight;
	std::string strTemp;
	char cBrk(0);

	switch (a_argType) 
	{
	case argumentParser::argType::noArg:
		aPair=new CInputBase();
		break;
	case argumentParser::argType::rightArg:
		if(a_defRight){aRight=a_defRight;}
		aPair = new CInputArgRight(aRight);
		break;
	case argumentParser::argType::bougthArg:
		if(a_defLeft){aLeft=a_defLeft;}
		aPair = new CInputArgRightAndLeft(aRight, aLeft);
		break;
	default: break;
	}

	if(!aPair.pInfo||!pcBuffer){return *this;}
	memcpy(pcBuffer, a_strOptionName.c_str(), unHelpStart+1);
	pcOptionName = pcBuffer;

	if(a_argType== argumentParser::argType::bougthArg){
		do {
			if (cBrk == INFO_START_DELIM) {
				unHelpStart = pcOptionName - pcBuffer;
				aPair.pInfo->helpStr = pcOptionName;
				goto returnPoint;
			}
			else if (pcToSet) { *pcToSet = SHORTCUTS_DELIM_SYMBOL2; }
			pcNext = strpbrk(pcOptionName, s_vcSubStr);
			if (pcNext) { cBrk = *pcNext; pcToSet = pcNext; *pcNext++ = 0; }
			unStrLenPlus1 = strlen(pcOptionName)+1;
			for(i=1;i<unStrLenPlus1;++i){
				strTemp = std::string(pcOptionName, i);
				if (m_htOptionsInAndOut.count(strTemp)) {
					if (m_htOptionsInAndOut[strTemp] != aPair.pInfo) {
						delete m_htOptionsInAndOut[strTemp];
						m_htOptionsInAndOut.erase(strTemp);
					}
				}
				m_htOptionsInAndOut[strTemp] = aPair.pInfo;
			}  // for(i=1;i<unStrLenPlus1;++i){

			pcOptionName = pcNext;
		} while (pcOptionName);
		goto returnPoint;
	}

	do{
		if(cBrk==INFO_START_DELIM){
			unHelpStart = pcOptionName - pcBuffer;
			aPair.pInfo->helpStr = pcOptionName;
			goto returnPoint;
		}else if(pcToSet){*pcToSet=SHORTCUTS_DELIM_SYMBOL2;}
		pcNext = strpbrk(pcOptionName, s_vcSubStr);
		if(pcNext){cBrk=*pcNext; pcToSet=pcNext;*pcNext++=0;}
		if(m_htOptionsInAndOut.count(pcOptionName)){
			if(m_htOptionsInAndOut[pcOptionName]!= aPair.pInfo){
				delete m_htOptionsInAndOut[pcOptionName];
				m_htOptionsInAndOut.erase(pcOptionName);
			}
		}
		m_htOptionsInAndOut[pcOptionName]= aPair.pInfo;
		pcOptionName = pcNext;
	} while (pcOptionName);

returnPoint:
	if(pcBuffer){
		aPair.option = pcBuffer;
		aPair.infoStr = aPair.pInfo->helpStr;
		m_vectForHelp.push_back(aPair);
		free(pcBuffer);
		if(unHelpStart>m_unMaxForHelp){m_unMaxForHelp=unHelpStart;}
	}
	else{delete aPair.pInfo;}
	return *this;
}


const char* common::argument_parser::valueLeft(const char* a_option_name)
{
    if(m_htOptionsInAndOut.count(a_option_name)){
		CInputBase* pInput = m_htOptionsInAndOut[a_option_name];
		if(pInput && pInput->isFound){return pInput->valueLeft();}
	}
    return NULL;
}


const char* common::argument_parser::operator[](const char* a_option_name)
{
    if(m_htOptionsInAndOut.count(a_option_name)){
		CInputBase* pInput = m_htOptionsInAndOut[a_option_name];
		if(pInput && pInput->isFound){return pInput->valueRight();}
	}
    return NULL;
}


std::string common::argument_parser::HelpString()const
{
	std::string aReturnStr;

	size_t unSize = m_vectForHelp.size();

	for (size_t i(0); i<unSize; ++i) {
		aReturnStr += (m_vectForHelp[i].option+":");
		aReturnStr += std::string(m_unMaxForHelp - m_vectForHelp[i].option.length() + 2,' ');
		aReturnStr += (m_vectForHelp[i].infoStr + "\n");
	}

    return aReturnStr;
}


void common::argument_parser::Clear()
{
	const size_t cunSize(m_vectForHelp.size());

	for(size_t i(0);i<cunSize;++i){
		delete m_vectForHelp[i].pInfo;
	}

	m_htOptionsInAndOut.clear();
	m_vectForHelp.clear();
	m_unMaxForHelp = 0;
}


void common::argument_parser::ParseCommandLine(int& a_argc, char** a_argv)
{
	CInputBase* pInput;
	CInputArgRight* pInpRight;
	CInputArgRightAndLeft* pInpBoth;
	size_t j, unStrLen;
	bool bFound;
	char cTmp;

    for (int i(0); i<((int)a_argc);){

		unStrLen = strlen(a_argv[i]); bFound = false;
		for(j=0;j<unStrLen;++j){
			cTmp = (a_argv[i])[j];
			if(!m_htOptionsInAndOut.count(a_argv[i])){++i;continue;}
		}
		if(!m_htOptionsInAndOut.count(a_argv[i])){++i;continue;}

		pInput = m_htOptionsInAndOut[a_argv[i]];
		if(!pInput || pInput->isFound){++i;continue;}
		pInput->isFound = 1;

		if (i < (--a_argc))
		{
			memmove(a_argv+i, a_argv+i+1, (a_argc-i) * sizeof(char*));

			switch (pInput->getType())
			{
			case argumentParser::argType::noArg:
				break;
			case argumentParser::argType::bougthArg:
				if((i>0) && ((a_argv[i-1])[0]!='-')){

					//if(pInput->valueRig)

					pInpBoth = (CInputArgRightAndLeft*)pInput;
					pInpBoth->defaultOrValueLeft = a_argv[i-1];
					memmove(a_argv+(i-1), a_argv+i, (a_argc-i+1) * sizeof(char*));
					--a_argc; --i;
				}
			case argumentParser::argType::rightArg:
				if (((a_argv[i])[0] != '-') && (i < (a_argc--))) {
					pInpRight = (CInputArgRight*)pInput;
					pInpRight->defaultOrValueRight = a_argv[i];
					memmove(a_argv + i, a_argv + i + 1, (a_argc - i) * sizeof(char*));
				}
				break;
			default:
				break;
			}

		}

	}// for (int i(0); i<a_argc;)
}


#else   // #ifdef __cplusplus
#endif  // #ifdef __cplusplus
