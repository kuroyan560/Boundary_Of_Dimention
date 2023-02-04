#include"Subject.h"
#include"Observer.h"

void KuroEngine::DesignPattern::Subject::Notify(const std::string& EventKey)
{
	for (auto ptr : m_observers)
	{
		ptr->OnNotify(EventKey);
	}
}