#include <common/debug.h>

#include <windows.h>
#include <string>
#include <iostream>

ConsoleStreamSubscriber g_consoleStreamSubscriber;
OutputDebugStringSubscriber g_outputDebugStringSubscriber;
OutputStream g_outputStream;

Debug* Debug::m_instance = NULL;

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

    for (auto iter : m_subscribers)
    {
        std::stringstream stream(m_pRawStringOStream->str());
        iter->output(stream);
    }

    delete m_pRawStringOStream;
    m_outputString = "";
    m_pRawStringOStream = new llvm::raw_string_ostream(m_outputString);
}

void ConsoleStreamSubscriber::output(std::stringstream& stream)
{
    std::cout << stream.str();
}

void OutputDebugStringSubscriber::output(std::stringstream& stream)
{
    OutputDebugString(stream.str().c_str());
}