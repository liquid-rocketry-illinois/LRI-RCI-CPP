#ifndef TESTS_H
#define TESTS_H

#include <Arduino.h>

namespace Test {
class Procedure;

extern Procedure* tests[16];

typedef void (*Runnable)();
typedef bool (*BoolSupplier)();

class OneShot;
class WaitProcedure;
class SequentialProcedure;
class ParallelProcedure;
class ParallelRaceProcedure;
class ParallelDeadlineProcedure;

class Procedure {
 public:
  Procedure() = default;

  virtual void initialize();
  virtual void execute();
  virtual void end(bool interrupted);
  virtual bool isFinished();

  virtual ~Procedure() = default;
};

class EStopSetterWrapper : public Procedure {
  Procedure* const proc;
  Procedure* const seqestop;
  Procedure* const endestop;

 public:
  EStopSetterWrapper(Procedure* proc, Procedure* seqestop, Procedure* endestop);

  virtual void initialize() override;
  virtual void execute() override;
  virtual void end(bool interrupted) override;
  virtual bool isFinished() override;

  virtual ~EStopSetterWrapper();
};

class OneShot : public Procedure {
  Runnable run;

 public:
  OneShot(Runnable run);

  virtual void initialize() override;
};

class WaitProcedure : public Procedure {
  const unsigned long waitTime;
  unsigned long startTime;

 public:
  WaitProcedure(const unsigned long& waitTime);

  void initialize() override;
  bool isFinished() override;
};

class BoolWaiter : public Procedure {
  BoolSupplier supplier;

 public:
  BoolWaiter(BoolSupplier supplier);

  virtual bool isFinished() override;
};

class SequentialProcedure : public Procedure {
  Procedure** const procedures;
  const int numProcedures;
  int current;

 public:
  template<typename... Procs> SequentialProcedure(Procs... procs);
  virtual ~SequentialProcedure() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void end(bool interrupted) override;
  virtual bool isFinished() override;
};

class ParallelProcedure : public Procedure {
 protected:
  Procedure** const procedures;
  const unsigned int numProcedures;
  bool* const running;

 public:
  template<typename... Procs> ParallelProcedure(Procs... procs);
  virtual ~ParallelProcedure() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void end(bool interrupted) override;
  virtual bool isFinished() override;
};

class ParallelRaceProcedure : public ParallelProcedure {
 public:
  template<typename... Procs> ParallelRaceProcedure(Procs... procs);

  virtual void end(bool interrupted) override;
  virtual bool isFinished() override;
};

class ParallelDeadlineProcedure : public Procedure {
  Procedure** const procedures;
  const unsigned int numProcedures;
  bool* const running;
  Procedure* const deadline;
  bool deadlineRunning;

 public:
  template<typename... Procs> ParallelDeadlineProcedure(Procedure* deadline, Procs... procs);
  virtual ~ParallelDeadlineProcedure() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void end(bool interrupted) override;
  virtual bool isFinished() override;
};

class SelectorProcedure : public Procedure {
  Procedure* const yes;
  Procedure* const no;
  const BoolSupplier chooser;
  bool choice;

 public:
  SelectorProcedure(Procedure* yes, Procedure* no, BoolSupplier chooser);
  virtual ~SelectorProcedure() override;

  virtual void initialize() override;
  virtual void execute() override;
  virtual void end(bool interrupted) override;
  virtual bool isFinished() override;
};

} // namespace Test

#endif