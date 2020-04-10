#pragma once

class IBHOEventObserver
{
public:
  virtual void OnNewWindow3()=0;
  virtual void OnBeforeNavigate2()=0;
  virtual void OnNaviagateComplete()=0;
  virtual void OnDocumentComplete()=0;
  virtual void OnTitleChange()=0;
};