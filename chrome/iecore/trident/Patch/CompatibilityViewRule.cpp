#include "chrome/iecore/trident/stdafx.h"
#include "CompatibilityViewRule.h"
#include <algorithm>

const char IECOMP_BLACKLIST_SECTION[] = "[iecompat]";
const char WHITESPACECHARS[] =  "\r\n";

CIECompatibilityViewData::CIECompatibilityViewData():m_bInit(false)
{

}
CIECompatibilityViewData::~CIECompatibilityViewData()
{

}
void CIECompatibilityViewData::Initialize()
{
  if( m_bInit)
    return;
  CComCritSecLock<CComAutoCriticalSection> lock(m_cs);
  if(m_bInit==FALSE)
  {
    // todo ylq
    m_bInit=true;
    std::wstring path;
    path+=L"\\data\\iecomapt.ini";
    ReadConfigFile(path);
  }
}

BOOL CIECompatibilityViewData::MatchBlackList(LPCSTR lpszUrl )
{

	Initialize();

	std::map<std::string,std::string>::iterator it = domain_black_list_.begin();
	for(;it!=domain_black_list_.end();++it)
	{
		if( PathMatchSpecA(lpszUrl,it->first.c_str()))
			return TRUE;
	}
	return FALSE;
}

BOOL CIECompatibilityViewData::MatchSelfBlackList(LPCSTR domain)
{
  std::map<std::string,DWORD>::iterator it = domain_self_list_.begin();
  for(;it!=domain_self_list_.end();++it)
  {
    if( PathMatchSpecA(domain,it->first.c_str()))
      return it->second==1?TRUE:FALSE;
  }
  return FALSE;
}

void CIECompatibilityViewData::ReadConfigFile(std::wstring szPath)
{

	DWORD dwFileSize = 0 ;
	HANDLE hFile = ::CreateFile(szPath.c_str(), GENERIC_READ  , FILE_SHARE_READ , NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, 0);
	if(INVALID_HANDLE_VALUE == hFile)
		return;
	dwFileSize = ::GetFileSize(hFile, NULL) ;
	if(dwFileSize && 0xFFFFFFFF != dwFileSize )
	{
		std::string file_data;
		file_data.resize(dwFileSize);
		DWORD dwSize;
		if(::ReadFile(hFile, (LPVOID)file_data.c_str(),dwFileSize, &dwSize, NULL))
		{
			::CloseHandle(hFile);
			InitializeFromBuffer(file_data);
		}
	}
}

void CIECompatibilityViewData::InitializeFromBuffer(const std::string& buffer)
{
	if (buffer.empty())
		return;
	ReadBlacklist(buffer);
}
void CIECompatibilityViewData::ReadBlacklist(const std::string& input) 
{
	size_t section_start_pos = input.find(IECOMP_BLACKLIST_SECTION);
	size_t section_end_pos = std::string::npos;
	if (std::string::npos != section_start_pos) {
		section_end_pos = input.find("[", section_start_pos + 
			strlen(IECOMP_BLACKLIST_SECTION));
		if (std::string::npos == section_end_pos) {
			section_end_pos = input.size();
		}

		size_t item_start_pos = section_start_pos + strlen(IECOMP_BLACKLIST_SECTION)+2;

		do 
		{
			size_t item_end_pos = input.find_first_of(WHITESPACECHARS, item_start_pos);
			if (std::string::npos == item_end_pos) {
				item_end_pos = input.size();
			}

			std::string key;
			std::string value;
			size_t equal_pos = input.find("=", item_start_pos + 1);
			if (std::string::npos == equal_pos) {
				if (item_end_pos > item_start_pos) {
					key = ReadValue(input, item_start_pos, item_end_pos);
				}
			} else {
				key = ReadValue(input, item_start_pos, equal_pos);
				value = ReadValue(input, equal_pos + 1, item_end_pos);
			}
			if (!key.empty()) {
				std::transform(key.begin(), key.end(), key.begin(), tolower);
				domain_black_list_[key] = value;
			}
			item_start_pos = input.find_first_not_of(WHITESPACECHARS, item_end_pos + 2);
		} while(std::string::npos != item_start_pos && 
			item_start_pos < input.size() &&
			item_start_pos < section_end_pos);
	}
}

std::string CIECompatibilityViewData::ReadValue(const std::string& input, size_t start_pos, size_t end_pos) 
{
	if (end_pos <= start_pos)
		return "";

	std::string key = input.substr(start_pos, end_pos - start_pos);
	return key;
}

void CIECompatibilityViewData::AddSelfList(LPCSTR domain,DWORD dwValue)
{
  std::string temp = domain;
  std::map<std::string,DWORD>::iterator it = domain_self_list_.find(domain);
  if(it==domain_self_list_.end())
    domain_self_list_.insert(std::make_pair(domain,dwValue));
  else
    it->second = dwValue;
}