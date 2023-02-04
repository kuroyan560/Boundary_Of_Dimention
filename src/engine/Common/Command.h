#pragma once
namespace KuroEngine
{
	namespace DesignPattern
	{
		class Command
		{
		public:
			virtual ~Command() {}
			virtual void Execute() = 0;
			virtual void Undo() = 0;
		};
	}
}