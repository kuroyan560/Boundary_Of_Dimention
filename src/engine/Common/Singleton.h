#pragma once
namespace KuroEngine
{
	namespace DesignPattern
	{
		template<typename T>
		class Singleton
		{
		protected:
			Singleton() {}
			virtual ~Singleton() {}
			Singleton(const Singleton& instance) {}
			Singleton& operator=(const Singleton& r) {}
		public:
			static T* Instance() {
				static T inst;
				return &inst;
			}
		};
	}
}