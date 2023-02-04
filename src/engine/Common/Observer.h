#pragma once
#include<string>

namespace KuroEngine
{
	namespace DesignPattern
	{
		class Observer
		{
		public:
			virtual ~Observer() {}
			virtual void OnNotify(const std::string& EventKey) = 0;
		};
	}
}