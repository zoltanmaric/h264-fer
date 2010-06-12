
#pragma once

#include <string> 
 
// .Net System Namespaces 
using namespace System; 
using namespace System::Runtime::InteropServices;

inline 
String ^ ToManagedString(const char * pString) { 
 return Marshal::PtrToStringAnsi(IntPtr((char *) pString)); 
} 
 
inline 
const std::string ToStdString(String ^ strString) { 
 IntPtr ptrString = IntPtr::Zero; 
 std::string strStdString; 
 try { 
  ptrString = Marshal::StringToHGlobalAnsi(strString); 
  strStdString = (char *) ptrString.ToPointer(); 
 } 
 finally { 
  if (ptrString != IntPtr::Zero) { 
   Marshal::FreeHGlobal(ptrString); 
  } 
 } 
 return strStdString; 
} 

namespace fer_h264 
{

	public ref class Starter
	{
		public: void PokreniKoder();
		public: void NastaviKoder();
		public: void PokreniDekoder();
		public: void DohvatiKarakteristike(int % brojTipova1, int % brojTipova2, int % brojTipova3, int % brojTipova4, int % brojTipova5, int % velicina, int % trajanje);
		public: void PostaviInterval(int FrameStart, int FrameEnd, int qp);
		public: void PostaviUlaz(String ^% ulaz);
	};
}