#pragma once
#include "chrome/iecore/trident/preheader.h"
#include <string>
#include <map>
#include <vector>

class CIECompatibilityViewData
{
public:
	CIECompatibilityViewData();
	~CIECompatibilityViewData();

	static CIECompatibilityViewData& GetInstance()
	{
		static CIECompatibilityViewData instance;
		return instance;
	} 
	void Initialize();
	BOOL MatchBlackList(LPCSTR lpszUrl ); 
  BOOL MatchSelfBlackList(LPCSTR  domain);
  void AddSelfList(LPCSTR domain,DWORD dwValue);
private:
	void ReadConfigFile(std::wstring szPath);
	void InitializeFromBuffer(const std::string& buffer);
	void ReadBlacklist(const std::string& input);
	std::string ReadKey(const std::string& input,size_t start_pos, size_t end_pos) ;
	std::string ReadValue(const std::string& input,size_t start_pos, size_t end_pos) ;

	bool m_bInit;
	std::map<std::string, std::string> domain_black_list_;
  std::map<std::string,DWORD> domain_self_list_;
  //
  CComAutoCriticalSection m_cs;
};