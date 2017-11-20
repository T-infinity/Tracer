#include <stdio.h>
#include <chrono>
#include <thread>
#include "Tracer.h"

int main(){
    Tracer::setProcessId(0);

    Tracer::begin("The whole she-bang");

    // create some pokemon
    std::map<std::string,long> pokemon;
    pokemon["fire"] = 1;
    pokemon["water"] = 3;
    pokemon["electric"] = 1;
    pokemon["poison"] = 2;

    Tracer::counter("cats",2);
    Tracer::counter("dogs",3);
    Tracer::counter("pokemon",pokemon);

    Tracer::begin("A timed event");
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    Tracer::end("A timed event");

    Tracer::counter("cats",15);
    Tracer::counter("dogs",11);
    pokemon["fire"]++;
    pokemon["water"]++;
    Tracer::counter("pokemon",pokemon);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Tracer::begin("Another timed event");

    Tracer::counter("dogs",8);
    Tracer::counter("cats",50);
    pokemon["electric"] += 3;
    Tracer::counter("pokemon",pokemon);

    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    Tracer::counter("cats",30);
    Tracer::counter("dogs",4);
    pokemon["poison"]--;
    Tracer::counter("pokemon",pokemon);

    Tracer::end("Another timed event");

    Tracer::end("The whole she-bang");


    return 0;
}
