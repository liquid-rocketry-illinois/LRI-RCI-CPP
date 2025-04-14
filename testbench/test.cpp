#include "test.h"
#include <Arduino.h>

namespace Test {
Procedure tests[16] = {};

void Procedure::initialize() {}

void Procedure::execute() {}

void Procedure::end(bool interrupted) {}

bool Procedure::isFinished() {
  return true;
}

OneShot::OneShot(Runnable run) : run(run) {}

void OneShot::initialize() {
  run();
}

WaitProcedure::WaitProcedure(const unsigned long& waitTime) : waitTime(waitTime) {}

void WaitProcedure::initialize() {
  startTime = millis();
}

bool WaitProcedure::isFinished() {
  return millis() - startTime > waitTime;
}

BoolWaiter::BoolWaiter(BoolSupplier supplier) : supplier(supplier) {}

bool BoolWaiter::isFinished() {
  return supplier();
}

template<typename... Procs>
SequentialProcedure::SequentialProcedure(Procs... procs)
    : numProcedures(sizeof...(Procs)), procedures(new Procedure[sizeof...(Procs)]{procs...}) {
  current = 0;
}

void SequentialProcedure::initialize() {
  if(current >= numProcedures) return;
  procedures[current].initialize();
}

void SequentialProcedure::execute() {
  if(current >= numProcedures) return;

  Procedure& proc = procedures[current];
  proc.execute();
  if(proc.isFinished()) {
    proc.end(false);
    current++;
    if(current < numProcedures) procedures[current].initialize();
  }
}

void SequentialProcedure::end(bool interrupted) {
  if(interrupted && current < numProcedures) procedures[current].end(interrupted);
}

bool SequentialProcedure::isFinished() {
  return current >= numProcedures;
}

template<typename... Procs>
ParallelProcedure::ParallelProcedure(Procs... procs)
    : numProcedures(sizeof...(Procs)), procedures(new Procedure[sizeof...(Procs)]{procs...}),
      running(new bool[sizeof...(Procs)]) {
  memset(running, 0, numProcedures);
}

void ParallelProcedure::initialize() {
  for(int i = 0; i < numProcedures; i++) {
    procedures[i].initialize();
    running[i] = true;
  }
}

void ParallelProcedure::execute() {
  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i].execute();

    if(procedures[i].isFinished()) {
      procedures[i].end(false);
      running[i] = false;
    }
  }
}

void ParallelProcedure::end(bool interrupted) {
  if(!interrupted) return;
  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i].end(true);
    running[i] = true;
  }
}

bool ParallelProcedure::isFinished() {
  for(int i = 0; i < numProcedures; i++)
    if(running[i]) return false;
  return true;
}

template<typename... Procs>
ParallelRaceProcedure::ParallelRaceProcedure(Procs... procs) : ParallelProcedure(procs...) {}

void ParallelRaceProcedure::end(bool interrupted) {
  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i].end(true);
  }
}

bool ParallelRaceProcedure::isFinished() {
  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) return true;
  }
  return false;
}

template<typename... Procs>
ParallelDeadlineProcedure::ParallelDeadlineProcedure(Procedure deadline, Procs... procs)
    : numProcedures(sizeof...(Procs)), procedures(new Procedure[sizeof...(Procs)]{procs...}),
      running(new bool[sizeof...(Procs)]), deadline(deadline), deadlineRunning(false) {
  memset(running, 0, numProcedures);
}

void ParallelDeadlineProcedure::initialize() {
  deadline.initialize();
  for(int i = 0; i < numProcedures; i++) {
    procedures[i].initialize();
  }
}

void ParallelDeadlineProcedure::execute() {
  if(deadlineRunning) {
    deadline.execute();
    if(deadline.isFinished()) {
      deadline.end(false);
      deadlineRunning = true;
    }
  }

  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i].execute();

    if(procedures[i].isFinished()) {
      procedures[i].end(false);
      running[i] = false;
    }
  }
}

void ParallelDeadlineProcedure::end(bool interrupted) {
  if(deadlineRunning) deadline.end(true);

  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i].end(true);
  }
}

bool ParallelDeadlineProcedure::isFinished() {
  return !deadlineRunning;
}

} // namespace Test