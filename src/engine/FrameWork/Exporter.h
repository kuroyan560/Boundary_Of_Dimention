#pragma once
#include"json.hpp"

namespace KuroEngine
{
	class Exporter
	{
	private:
		static Exporter* s_instance;

	public:
		static Exporter* Instance()
		{
			assert(s_instance != nullptr);
			return s_instance;
		}

	public:
		Exporter()
		{
			assert(s_instance == nullptr);
			s_instance = this;
		}
		~Exporter() {}
	};
}