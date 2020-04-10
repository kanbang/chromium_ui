// WebPage.h: interface for the CWebPage class.
//
#pragma once
#include <atlbase.h>
#include <Mshtml.h>

/*
c++调用js对象的函数：js函数或者对象在C++里面对应的就是IDispatch，比如external,navigator,onbeforeunload
1、pDoc->get_Script(&spDisp)接口，比如调用onbeforeunload函数    设置属性：给QQ空间播放器设置Mute属性。Activex插件本身就实现了IDispatch，通过QueryInterface,然后就可以修改属性
2、IHTMLWindow exeScript
3、js函数在c++里面是特殊的IDispatch对象，直接调用Invoke（DISPID_VALUE，无需调用GetIDsOfNames
4、js对象也是IDispatch对象，先通过GetIDsOfNames获取DISPID，然后Invoke(DISPID调用
5、js对象的属性如果是方法的话，就对应IDispatch对象

c++注入全局对象到页面
	c++通过IDispatchEx函数给页面插入一个object对象（跟external类似）供脚本使用

js调用c++,比如external,navigator（都是继承的IDispatch)
	通过IDispatch接口

IE11也支持修改只读属性、方法
navigator.__defineGetter__('appName',function(){return "Internet Explore"})
document.__defineGetter__('referrer',function(){return "Internet Explore"})
*/
class CWebPage  
{
public:
	CWebPage();
	virtual ~CWebPage();


	bool GetJScript(IHTMLDocument2* pDoc,CComPtr<IDispatch>& spDisp);
	bool GetJScripts(IHTMLDocument2* pDoc,CComPtr<IHTMLElementCollection>& spColl);
};
