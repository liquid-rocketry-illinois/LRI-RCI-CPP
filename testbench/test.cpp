#include <Arduino.h>

#include "RCP.h"
#include "test.h"

namespace Test {

Procedure* tests[16] = {
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
  new Procedure(),
};
// clang-format on

void Procedure::initialize() {}

void Procedure::execute() {}

void Procedure::end(bool interrupted) {}

bool Procedure::isFinished() {
  return true;
}

EStopSetterWrapper::EStopSetterWrapper(Procedure* proc, Procedure* seqestop, Procedure* endestop)
    : proc(proc), seqestop(seqestop), endestop(endestop) {}

void EStopSetterWrapper::initialize() {
  proc->initialize();
  RCP::ESTOP_PROC = seqestop;
}

void EStopSetterWrapper::execute() {
  proc->execute();
}

bool EStopSetterWrapper::isFinished() {
  return proc->isFinished();
}

void EStopSetterWrapper::end(bool interrupted) {
  proc->end(interrupted);
  RCP::ESTOP_PROC = endestop;
}

EStopSetterWrapper::~EStopSetterWrapper() {
  delete proc;
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
    : procedures(new Procedure* [sizeof...(Procs)] { procs... }), numProcedures(sizeof...(Procs)) {
  current = 0;
}

SequentialProcedure::~SequentialProcedure() {
  for(int i = 0; i < numProcedures; i++) {
    delete procedures[i];
  }

  delete[] procedures;
}

void SequentialProcedure::initialize() {
  current = 0;
  if(current >= numProcedures) return;
  procedures[current]->initialize();
}

void SequentialProcedure::execute() {
  if(current >= numProcedures) return;

  Procedure* proc = procedures[current];
  proc->execute();
  if(proc->isFinished()) {
    proc->end(false);
    current++;
    if(current < numProcedures) procedures[current]->initialize();
  }
}

void SequentialProcedure::end(bool interrupted) {
  if(interrupted && current < numProcedures) procedures[current]->end(interrupted);
}

bool SequentialProcedure::isFinished() {
  return current >= numProcedures;
}

template<typename... Procs>
ParallelProcedure::ParallelProcedure(Procs... procs)
    : procedures(new Procedure* [sizeof...(Procs)] { procs... }), numProcedures(sizeof...(Procs)),
      running(new bool[sizeof...(Procs)]) {
  memset(running, 0, numProcedures);
}

ParallelProcedure::~ParallelProcedure() {
  for(int i = 0; i < numProcedures; i++) {
    delete procedures[i];
  }

  delete[] procedures;
}

void ParallelProcedure::initialize() {
  for(int i = 0; i < numProcedures; i++) {
    procedures[i]->initialize();
    running[i] = true;
  }
}

void ParallelProcedure::execute() {
  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i]->execute();

    if(procedures[i]->isFinished()) {
      procedures[i]->end(false);
      running[i] = false;
    }
  }
}

void ParallelProcedure::end(bool interrupted) {
  if(!interrupted) return;
  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i]->end(true);
    running[i] = false;
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
    procedures[i]->end(true);
    running[i] = false;
  }
}

bool ParallelRaceProcedure::isFinished() {
  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) return true;
  }
  return false;
}

template<typename... Procs>
ParallelDeadlineProcedure::ParallelDeadlineProcedure(Procedure* deadline, Procs... procs)
    : procedures(new Procedure* [sizeof...(Procs)] { procs... }), numProcedures(sizeof...(Procs)),
      running(new bool[sizeof...(Procs)]), deadline(deadline), deadlineRunning(false) {
  memset(running, 0, numProcedures);
}

ParallelDeadlineProcedure::~ParallelDeadlineProcedure() {
  delete deadline;

  for(int i = 0; i < numProcedures; i++) {
    delete procedures[i];
  }

  delete[] procedures;
}

void ParallelDeadlineProcedure::initialize() {
  deadline->initialize();
  for(int i = 0; i < numProcedures; i++) {
    procedures[i]->initialize();
  }
}

void ParallelDeadlineProcedure::execute() {
  if(deadlineRunning) {
    deadline->execute();
    if(deadline->isFinished()) {
      deadline->end(false);
      deadlineRunning = true;
    }
  }

  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i]->execute();

    if(procedures[i]->isFinished()) {
      procedures[i]->end(false);
      running[i] = false;
    }
  }
}

void ParallelDeadlineProcedure::end(bool interrupted) {
  if(deadlineRunning) deadline->end(true);

  for(int i = 0; i < numProcedures; i++) {
    if(!running[i]) continue;
    procedures[i]->end(true);
  }
}

bool ParallelDeadlineProcedure::isFinished() {
  return !deadlineRunning;
}

SelectorProcedure::SelectorProcedure(Procedure* yes, Procedure* no, BoolSupplier chooser)
    : yes(yes), no(no), chooser(chooser) {}

SelectorProcedure::~SelectorProcedure() {
  delete yes;
  delete no;
}

void SelectorProcedure::initialize() {
  choice = chooser();
  (choice ? yes : no)->initialize();
}

void SelectorProcedure::execute() {
  (choice ? yes : no)->execute();
}

bool SelectorProcedure::isFinished() {
  return (choice ? yes : no)->isFinished();
}

void SelectorProcedure::end(bool interrupted) {
  (choice ? yes : no)->end(interrupted);
}

} // namespace Test