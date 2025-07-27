# Testing Framework

This file describes the usage of the testing framework present in the RCP reference implementation. This framework is
**NOT** for unit testing, but for sequencing physical tests to run (for example, a hot fire). This framework is
modelled after [WPILib](https://docs.wpilib.org) from FRC robotics, and their command-based programming system.

NOTE: Unfortunately, the full command-based framework is not exactly possible here, because of lack of proper move
semantics (c++ garbage not important to know) in the AVR compiler used with ATMEGA chips (arduino), so this sort of
messier version is here instead. If we decide to move entirely to ARM chips then we might not have to worry, but for now
we are stuck as is. 

## The `Procedure`

Everything here is based off of a singular base class: the `Procedure` class. This contains the `initialize`, 
`execute`, `isFinished`, and `end` functions that define a `Procedure` (for those who know WPILib, this will be very 
familiar). When you decide to run a `Procedure`, you first call its `initialize` function, so it can set up anything 
it needs to run. From there, the `execute` function is executed repeatedly until `isFinished` returns true. At that 
point, `end` is called, allowing the procedure to clean up after itself. 

The `end` function takes a single boolean parameter, `interrupted`. This signals to the `Procedure` whether or not 
the `Procedure` is terminating normally, or if it was abnormally and prematurely killed.

This system of init-run-end allows a single procedure to be reused throughout the runtime of the code. This also 
means that the timing of when things like the `initialize` function is run is important. A common mistake is to 
assume _construction_  of a command at code startup is the same as the `initialize` function, but this is not the 
case. The _construction_ of the procedure object happens during static initialization (i.e. before the main function),
but the `initialize` function executes when the procedure is about to begin execution, which may be any arbitrary 
amount of time after static initialization.

Point being, if you have code in your procedure dependent on the _current_ position, or time, or current _anything_, 
the current value of that thing **MUST BE CALCULATED IN `initialize`**, lest your procedure think the value of your 
thing at static initialization time is the value of your thing at execution begin. 

## `Procedure` subclasses

The default `Procedure` class is not very useful on its own, since its member functions are empty. To make this 
class useful, we extend this class and override its member functions to add functionality. Then, every subclass can 
be run using the same set of base hosting code. In the reference implementation, in `RCP.cpp`, this is the `runTest` 
function.

Custom `Procedures` can be easily created by subclassing the `Procedure`, then overriding the member functions.

## `Procedure` Compositions

Once again, another topic that will be familiar to WPILib users (unfortunately, no decorators, see move semantics 
complaint).

Many procedures will need things like delays, decisions on which `Procedure` subclass to run determined on a runtime 
variable, or other advanced control structures like parallel execution. You could write your own code to handle this,
but since this requirement is common, there exist pre-written `Procedure` "compositions": called as such because you 
"compose" multiple `Procedures` into one larger, more complex `Procedure`.

Each composition `Procedure` has its arguments set in its constructor, and it handles all the funky control logic 
for you by taking pointers to the children `Procedures`.


### `OneShot`

A `Procedure` to wrap a one time action. Does not take a `Procedure*` as an argument, rather a `Runnable`: a pointer 
to a function that takes no arguments and returns no value. It executes this function in its `initialize` function, 
and its `execute` does nothing. Its `isFinished` function always returns true, so immediately after executing the 
`Runnable` argument, the `Procedure` finishes.

### `WaitProcedure`

Waits the specified number of milliseconds before exiting. This is a good example of making correct use of the 
`initialize` function. The start time is not set in the constructor, rather it is set in the `initialize` function, 
since that is when timing actually starts from.

### `SequentialProcedure`

Takes in a list of `Procedure*` procedures to run in sequence. Once the first `Procedure` finishes, it moves on to 
the second, then the third, and so on. This `Procedure` finishes when all of the `Procedures` in its list have 
completed.

### `ParallelProcedure`

Takes in a list of `Procedure*` procedures to be run in parallel. Of course, true parallelization is not possible 
here (unless your MCU has multiple cores), so every `Procedures` `execute` function are called in the call to the 
`ParallelProcedures` `execute` function, rather than each `Procedure` being executed sequentially. This `Procedure` 
exits when all of its child `Procedures` have exited.

### `ParallelRaceProcedure`

Takes in a list of `Procedure*` procedures to be run in parallel. Of course, like with `ParallelProcedure`, true 
parallelization is not possible, so the same semi-parallelized technique is used. This `Procedure` exits when one of 
its child `Procedures` have exited. The rest are interrupted.

### `ParallelDeadlineProcedure`

Takes in a list of `Procedure*` procedures to be run in parallel. Of course, like with `ParallelProcedure`, true
parallelization is not possible, so the same semi-parallelized technique is used. This `Procedure` exits when the 
specified deadline `Procedure` has exited. The rest are interrupted.

### `SelectorProcedure`

This `Procedure` acts kind of like a ternary statement. It takes in two `Procedure*` arguments, as well as a 
`BoolSupplier` chooser (a function with no arguments that returns a bool). At runtime, the chooser is executed to 
determine which of the `Procedures` is executed.

### `EStopSetterWrapper`

Sometimes, when executing a `Procedure`, a certain emergency stop `Procedure` is desired, rather than the default. 
This `Procedure` simply wraps the execution of one `Procedure` with an alternative emergency stop `Procedure`, then 
returns the emergency stop back to the specified default.