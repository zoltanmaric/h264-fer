
#pragma once

#include <string> 
 
// .Net System Namespaces 
using namespace System; 
using namespace System::Runtime::InteropServices;

inline String ^ ToManagedString(const char * pString);
 
inline const std::string ToStdString(String ^ strString);

namespace fer_h264 
{

	public ref class Starter
	{
		public: void PokreniKoder();
		public: void NastaviKoder();
		public: void PokreniDekoder();
		public: void DohvatiStatistiku(int % brojTipova1, int % brojTipova2, int % brojTipova3, int % brojTipova4, int % brojTipova5, int % velicina, int % trajanje);
		public: void PostaviParametre(int FrameStart, int FrameEnd, int qp, int OsnovnoPredvidanje, int VelicinaProzora, int ToleriranaGreska);
		public: void PostaviUlazIzlaz(String ^% ulaz, String ^% izlaz);
	};
}