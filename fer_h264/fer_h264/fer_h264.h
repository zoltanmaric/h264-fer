
#pragma once

namespace fer_h264 
{

	public ref class Starter
	{
		public: void PokreniKoder();
		public: void PokreniDekoder();
		public: void PostaviInterval(int FrameStart, int FrameEnd);
	};
}