//
// Copyright 2016 United States Government as represented by the Administrator of the National Aeronautics and Space Administration. 
// No copyright is claimed in the United States under Title 17, U.S. Code. All Other Rights Reserved.
// 
// The “Parfait: A Toolbox for CFD Software Development [LAR-18839-1]” platform is licensed under the 
// Apache License, Version 2.0 (the "License"); you may not use this file except in compliance with the License. 
// You may obtain a copy of the License at http://www.apache.org/licenses/LICENSE-2.0. 
// 
// Unless required by applicable law or agreed to in writing, software distributed under the License is distributed 
// on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. 
// See the License for the specific language governing permissions and limitations under the License.

#pragma once
#include <string>
#include <thread>
#include <map>
#include <iostream>
#include <fstream>
#include <mutex>

class Event {
public:
    std::string category = "DEFAULT";
    int process_id = 0;
    int thread_id = -1;
    std::string phase = "i";
    std::string name = "DEFAULT";
    std::string timestamp = "";
    std::map<std::string,long> args;
    static uint64_t getThreadId();
    static std::string getTimeStamp();
};

class TraceWriter {
public:
    TraceWriter(std::string file_name);
    TraceWriter();
    ~TraceWriter();
    void log(Event e);
    void setThreadName(std::string name,int processId,int threadId);
    void beginFlowEvent(int eventId,int processId,int threadId,std::string timestamp);
    void endFlowEvent(int eventId, int processId);
private:
    bool is_first = true;
    std::mutex lock;
    std::ofstream file;
    std::string file_name;

    std::string timestamp();
    void printFileHeader();
    void printFileFooter();
    void addThreadId(Event &e) const;
    void addTimeStamp(Event &event);
    void writeEvent(const Event &e);
    void beginItem();
    void initialize(const std::string &file_name);

    class JsonItem{
    public:
        void addPair(std::string key, const char* value);
        void addPair(std::string key, const std::string& value);
        void addPair(std::string key, const JsonItem& item);
        template<typename T>
        void addPair(std::string key, T value);
        std::string getString() const { return json + "}";}
    private:
        bool empty = true;
        std::string json = "{";
        void prependCommaIfNeeded();
    };
};

template<typename T>
void TraceWriter::JsonItem::addPair(std::string key, T value) {
    prependCommaIfNeeded();
    json += "\"" + key + "\": " + std::to_string(value);
}

class TimeStampedEvent {
    inline TimeStampedEvent()
        : process_id(0), thread_id(Event::getThreadId()),
          timestamp(Event::getTimeStamp())
    {}

    std::string category = "DEFAULT";
    int process_id = 0;
    int thread_id = -1;
    std::string phase = "i";
    std::string name = "DEFAULT";
    std::string timestamp = "";
};

inline uint64_t Event::getThreadId(){
    auto tid = std::this_thread::get_id();
    uint64_t *p = (uint64_t *) &tid;
    return *p;
}

inline std::string Event::getTimeStamp() {
    auto m =  std::chrono::duration_cast< std::chrono::microseconds >(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    );
    return std::to_string(m.count());
}

class Tracer {
public:
    static void setProcessId(int id);
    static void setThreadName(std::string name);
    static void log(const Event& e);
    static void counter(std::string name,long n);
    static void counter(std::string name,const std::map<std::string,long>& things);
    static void logSimpleEvent(std::string name, std::string category = "DEFAULT");
    static void begin(std::string name, std::string category = "DEFAULT");
    static void end(std::string name, std::string category = "DEFAULT");
    static void beginFlowEvent(int id);
    static void endFlowEvent(int id);
private:
    static TraceWriter& instance();
    static int& processId();
};

class TraceShell {
private:
    std::string name;
    std::string category;
public:
    TraceShell(std::string name, std::string category = "DEFAULT");
    ~TraceShell();
};

inline void Tracer::setProcessId(int id) {
    processId() = id;
}
inline void Tracer::log(const Event& e) {
    instance().log(e);
}
inline TraceWriter& Tracer::instance() {
    static TraceWriter writer("trace_" + std::to_string(processId()) + ".trace");
    return writer;
}
inline int& Tracer::processId() {
    static int p_id = -1;
    return p_id;
}
inline void Tracer::logSimpleEvent(std::string name, std::string category) {
    Event e;
    e.process_id = processId();
    e.category = category;
    e.name = name;
    e.phase = "i";
    log(e);
}
inline void Tracer::begin(std::string name, std::string category) {
    Event e;
    e.process_id = processId();
    e.name = name;
    e.category = category;
    e.phase = "B";
    log(e);
}
inline void Tracer::end(std::string name, std::string category) {
    Event e;
    e.process_id = processId();
    e.name = name;
    e.category = category;
    e.phase = "E";
    log(e);
}

inline void Tracer::setThreadName(std::string name) {
    instance().setThreadName(name,processId(),0);
}

inline void Tracer::endFlowEvent(int id) {
    instance().endFlowEvent(id,processId());
}

inline void Tracer::beginFlowEvent(int id) {
    instance().beginFlowEvent(id,processId(),0,"derp");
}

inline void Tracer::counter(std::string name, long n) {
    counter(name,{{name,n}});
}

inline void Tracer::counter(std::string name, const std::map<std::string,long>& things) {
    Event e;
    e.process_id = processId();
    e.name = name;
    e.category = "category";
    e.phase = "C";
    e.args = things;
    log(e);
}
inline TraceWriter::TraceWriter(std::string file_name_in)
    :file_name(file_name_in){
    initialize(file_name);
}
inline TraceWriter::TraceWriter() : file_name("trace.trace") {
    initialize(file_name);
}
inline std::string TraceWriter::timestamp() {
    auto m =  std::chrono::duration_cast< std::chrono::microseconds >(
        std::chrono::high_resolution_clock::now().time_since_epoch()
    );
    return std::to_string(m.count());
}
inline TraceWriter::~TraceWriter() {
    lock.lock();
    printFileFooter();
    file.close();
    lock.unlock();
}
inline void TraceWriter::log(Event e) {

    addThreadId(e);
    addTimeStamp(e);

    writeEvent(e);
}

inline void TraceWriter::beginItem(){
    if(is_first)
        is_first = false;
    else
        file << ",";
    file << std:: endl;
}

inline void TraceWriter::writeEvent(const Event &e) {
    std::lock_guard<std::mutex> guard(lock);
    beginItem();
    JsonItem item;
    item.addPair("cat", e.category);
    item.addPair("pid", e.process_id);
    item.addPair("tid", e.thread_id);
    item.addPair("ts", std::stol(e.timestamp));
    item.addPair("ph", e.phase);
    item.addPair("name", e.name);
    JsonItem args;
    for(auto& arg:e.args){
        args.addPair(arg.first,arg.second);
    }
    item.addPair("args",args);

    file << item.getString();
    file.flush();
}
inline void TraceWriter::addThreadId(Event &e) const {
    if(e.thread_id == -1) {
        auto tid = std::this_thread::get_id();
        uint64_t *p = (uint64_t *) &tid;
        e.thread_id = *p;
    }
}
inline void TraceWriter::addTimeStamp(Event &e) {
    e.timestamp = timestamp();
}
inline void TraceWriter::printFileHeader() { file << "["; }
inline void TraceWriter::printFileFooter() { file << std::endl << "]"; }
inline void TraceWriter::initialize(const std::string &file_name) {
    lock.lock();
    file.open(file_name);
    printFileHeader();
    lock.unlock();
}

inline void TraceWriter::setThreadName(std::string name,int processId, int threadId) {
    std::lock_guard<std::mutex> guard(lock);
    beginItem();
    auto tid = std::this_thread::get_id();
    uint64_t *p = (uint64_t *) &tid;
    threadId = *p;

    JsonItem item;
    item.addPair("name","thread_name");
    item.addPair("ph","M");
    item.addPair("pid",processId);
    item.addPair("tid",threadId);
    JsonItem threadNamePair;
    threadNamePair.addPair("name", name);
    item.addPair("args", threadNamePair);
    file << item.getString();
}

inline void TraceWriter::beginFlowEvent(int eventId, int processId, int threadId,std::string timestamp){
    std::lock_guard<std::mutex> guard(lock);
    beginItem();
    Event e;
    addThreadId(e);
    addTimeStamp(e);

    JsonItem item;
    item.addPair("cat","foo");
    item.addPair("name","flow_event");
    item.addPair("pid",processId);
    item.addPair("tid",e.thread_id);
    item.addPair("ts",std::stol(e.timestamp));
    item.addPair("id",eventId);
    item.addPair("ph","s");
    file << item.getString();
}

inline void TraceWriter::endFlowEvent(int eventId, int processId){
    std::lock_guard<std::mutex> guard(lock);
    beginItem();
    Event e;
    addThreadId(e);
    addTimeStamp(e);
    JsonItem item;
    item.addPair("cat","foo");
    item.addPair("name","flow_event");
    item.addPair("pid",processId);
    item.addPair("tid",e.thread_id);
    item.addPair("ts",std::stol(e.timestamp));
    item.addPair("id",eventId);
    item.addPair("ph","f");
    item.addPair("bp","e");
    file << item.getString();
}

inline void TraceWriter::JsonItem::addPair(std::string key, const char* value) {
    prependCommaIfNeeded();
    json += "\"" + key + "\": \"" + value + "\"";
}

inline void TraceWriter::JsonItem::prependCommaIfNeeded(){
    if(empty)
        empty = false;
    else
        json += ", ";
}

inline void TraceWriter::JsonItem::addPair(std::string key, const std::string &value) {
    prependCommaIfNeeded();
    json += "\"" + key + "\": \"" + value + "\"";
}

inline void TraceWriter::JsonItem::addPair(std::string key, const TraceWriter::JsonItem &item) {
    prependCommaIfNeeded();
    json += "\"" + key + "\": " + item.getString();
}
