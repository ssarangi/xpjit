#ifndef DEBUG_H
#define DEBUG_H

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <assert.h>
#include <exception>

#include <common/llvm_warnings_push.h>
#include <llvm/Support/raw_ostream.h>
#include <common/llvm_warnings_pop.h>

using namespace std;

#if defined _DEBUG
#define ICARUS_NOT_IMPLEMENTED(STR)      \
{                                        \
    throw std::exception((const char*)(STR)); \
}
#endif

#define ADD_HEADER(txt) \
{                                                                                                                  \
    g_outputStream << "-------------------------------------------------------------------\n";            \
    g_outputStream << "                         " << txt << "                        \n";                 \
    g_outputStream << "-------------------------------------------------------------------\n";            \
}

class Debug 
{
public:
    void setDebug(bool isOn) { m_isDebugOn = isOn; }
    bool isDebuggable() { return m_isDebugOn; }

    void setTrace(bool isOn) { m_isTraceOn = isOn; }
    bool isTraceOn() { return m_isTraceOn; } 

    void setYaccTrace(bool isOn) { m_showYaccTrace = isOn; }
    bool isYaccTraceOn() { return m_showYaccTrace; }

    void setDotGen(bool isOn) { m_isDotGen = isOn; }
    bool isDotGen() { return m_isDotGen; }

    void setCodeOptimization(bool isOn) { m_isCodeOptimizing = isOn; }
    bool isOptimizing() { return m_isCodeOptimizing; }

    static Debug& getInstance();
private:
    bool m_isDebugOn;
    bool m_isTraceOn;
    bool m_showYaccTrace;
    bool m_isDotGen;
    bool m_isCodeOptimizing;
    static Debug* m_instance;
    Debug();
};

class OutputStreamSubscriber 
{
public:
    OutputStreamSubscriber() {}
    virtual void output(std::stringstream& stream) = 0;
};

class ConsoleStreamSubscriber : public OutputStreamSubscriber
{
public:
    ConsoleStreamSubscriber()
        : OutputStreamSubscriber()
    {}

    virtual void output(std::stringstream& stream);
};

class OutputDebugStringSubscriber : public OutputStreamSubscriber
{
public:
    OutputDebugStringSubscriber()
        : OutputStreamSubscriber()
    {}

    virtual void output(std::stringstream& stream);
};

class OutputStream
{
public:
    OutputStream()
    {
        m_pRawStringOStream = new llvm::raw_string_ostream(m_outputString);
    }
    
    ~OutputStream()
    {
        delete m_pRawStringOStream;
    }

    void addOutputStreamSubscriber(OutputStreamSubscriber* pSubscriber);
    void removeOutputStreamSubscriber(OutputStreamSubscriber* pSubscriber);

    llvm::raw_ostream& operator()() { return *m_pRawStringOStream; }
    template <typename T>
    OutputStream& operator<<(const T& s)
    {
        *m_pRawStringOStream << s;
        flush();
        return *this;
    }

private:
    void flush();

private:
    std::vector<OutputStreamSubscriber *> m_subscribers;
    std::stringstream m_stringStream;
    std::string m_outputString;
    llvm::raw_string_ostream *m_pRawStringOStream;
};

extern ConsoleStreamSubscriber g_consoleStreamSubscriber;
extern OutputDebugStringSubscriber g_outputDebugStringSubscriber;
extern OutputStream g_outputStream;

#endif //DEBUG_H
