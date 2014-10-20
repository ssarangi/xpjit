#include <common/debug.h>

#include <windows.h>
#include <string>
#include <iostream>

ConsoleStreamSubscriber g_consoleStreamSubscriber;
OutputDebugStringSubscriber g_outputDebugStringSubscriber;
OutputStream g_outputStream;

Trace* Trace::m_instance = NULL;
Debug* Debug::m_instance = NULL;

Trace& Trace::getInstance()
{
    if(m_instance == 0)
        m_instance = new Trace();
    return *m_instance;
}

Trace::~Trace()
{
    if(m_instance != 0)
        delete m_instance;
}

Trace& Trace::operator<<(std::string s)
{
    if(Debug::getInstance().isTraceOn())
    {
        std::cout<<s;
    }
    return *this;
}

Debug::Debug()
{
    m_isDebugOn = false;
    m_isTraceOn = false;
    m_showYaccTrace = false;
    m_isDotGen = false;
    m_isCodeOptimizing = false;
}

Debug& Debug::getInstance()
{
    if(m_instance == 0)
        m_instance = new Debug();
    return *m_instance;
}


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

void OutputStream::flush()
{
    for (auto iter : m_subscribers)
    {
        iter->output(m_stringStream);
    }

    m_stringStream.str(std::string());
}

void OutputStream::flush_raw_stream()
{
    for (auto iter : m_subscribers)
    {
        std::stringstream stream(m_pRawStringOStream->str());
        iter->output(stream);
    }
}

void ConsoleStreamSubscriber::output(std::stringstream& stream)
{
    std::cout << stream.str();
}

void OutputDebugStringSubscriber::output(std::stringstream& stream)
{
    OutputDebugString(stream.str().c_str());
}
