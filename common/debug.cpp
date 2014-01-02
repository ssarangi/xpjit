#include <common/debug.h>

void OutputStream::addOutputStreamSubscriber(OutputStreamSubscriber* pSubscriber)
{
    m_subscribers.push_back(pSubscriber);
}

void OutputStream::removeOutputStreamSubscriber(OutputStreamSubscriber* pSubscriber)
{
    int found_index = 0;
    int counter = 0;
    for (auto iter : m_subscribers)
    {
        if (iter == pSubscriber)
        {
            found_index = counter;
            break;
        }

        ++counter;
    }
    
    m_subscribers.erase(m_subscribers.begin() + found_index);
}

void OutputStream::output(std::stringstream& stream)
{
    for (auto iter : m_subscribers)
    {
        iter->output(stream);
    }
}
