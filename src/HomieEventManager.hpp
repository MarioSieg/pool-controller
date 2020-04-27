/*
  Event

  This class serves as a "callback" manager to register events
  to happen on certain triggers or after certain intervals.
 */
#pragma once

#include "Arduino.h"

/**
 * Event structure is the basic Event
 * object that can be dispatched by the
 * manager.
 */
struct HomieNodeEvent
{
  HomieNodeEvent() : label(NULL) {}
  HomieNodeEvent(const char *cLabel) : label(cLabel) {}
  HomieNodeEvent(const char *cLabel, const char *cExtra) : label(cLabel), extra(cExtra) {}
  const char *extra;
  const char *label;
};

/**
 * EventTask is a structure that serves as an
 * abstract class of a "dispatchable" object.
 */
struct HomieNodeEventTask
{
  virtual boolean execute(HomieNodeEvent* evt) = 0;
};

/**
 * The Subscriber is the object that
 * encapsulates the Event it's listening for
 * and the EventTask to be executed.
 */
struct Subscriber
{
  Subscriber() : label(NULL), task(NULL) {}
  Subscriber(const char *cLabel, HomieNodeEventTask *cTask) : label(cLabel), task(cTask) {}

  const char *label;
  HomieNodeEventTask *task;
};

/**
 * TimedTask is an Event that executes after a certain
 * amount of milliseconds.
 */
struct TimedTask
{
  TimedTask() : ms(0), evt(NULL) , alive(false), cyclic(false), current(0)  {}
  TimedTask(unsigned long cMs, HomieNodeEvent* cEvt) : ms(cMs), evt(cEvt), cyclic(false), alive(true), current(60)  {}

  /**
   * Evaluates the state of the timed task and if
   * it's time to execute it or not. Resets the current
   * counter if it reaches the timed threshold.
   */
  boolean eval()
  {
    if (current >= ms)
    {
      current = 0;
      return true;
    }

    return false;
  }

  unsigned long ms;
  unsigned long current;
  HomieNodeEvent* evt;
  boolean alive; // State of the timed task
  boolean cyclic;
};

/**
 * The EventManager is responsible for gathering subscribers
 * and dispatching them when the requested Event is
 * triggered.
 */
class HomieEventManager
{
  public:
    HomieEventManager();
    void subscribe(Subscriber sub);
    boolean trigger(HomieNodeEvent* evt);
    void triggerInterval(TimedTask timed);
    void disableTimedTask(const char * label);
    void tick();
  private:
    TimedTask _interval[5]; // 5 available interval slots
    unsigned int _intervalSize;
    Subscriber _sub[5]; // 5 available subscriber slots
    unsigned int _subSize;
    unsigned int _subPos;
    unsigned long _previousMs;
};
