#include "TrafficLight.h"
#include <iostream>
#include <random>

/* Implementation of class "MessageQueue" */

template <typename T> T MessageQueue<T>::receive() {
  // FP.5a : The method receive should use std::unique_lock<std::mutex> and
  //   _condition
  //       .wait()
  //       // to wait for and receive new messages and pull them from the queue
  //       using move semantics.
  // The received object should then be returned by the receive function.
  std::unique_lock<std::mutex> uLock(_mutex);
  _cond.wait(uLock, [this] { return !_messages.empty(); });

  T msg = std::move(_messages.back());
  _messages.pop_back();

  return msg;
}

template <typename T> void MessageQueue<T>::send(T &&msg) {
  // FP.4a : The method send should use the mechanisms
  // std::lock_guard<std::mutex>
  // as well as _condition.notify_one() to add a new message to the queue
  // and
  // afterwards send a notification.

  std::lock_guard<std::mutex> lck(_mutex);
  _messages.push_back(std::move(msg));
  _cond.notify_one();
}
TrafficLight::TrafficLight() { _currentPhase = TrafficLightPhase::red; }

void TrafficLight::waitForGreen() {
  // FP.5b : add the implementation of the method waitForGreen, in which an
  // infinite while-loop runs and repeatedly calls the receive function on the
  // message queue. Once it receives TrafficLightPhase::green, the method
  // returns.
  while (true) {
    TrafficLightPhase msg = _msgsQue.receive();
    if (msg == TrafficLightPhase::green) {
      break;
    }
    // std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}

TrafficLightPhase TrafficLight::getCurrentPhase() { return _currentPhase; }

void TrafficLight::simulate() {
  // FP.2b : Finally, the private method „cycleThroughPhases“ should be
  // started in a thread when the public method „simulate“ is called. To do
  // this, use the thread queue in the base class.
  threads.emplace_back(std::thread(&TrafficLight::cycleThroughPhases, this));
}

// virtual function which is executed in a thread
void TrafficLight::cycleThroughPhases() {
  // FP.2a : Implement the function with an infinite loop that measures the
  // time between two loop cycles and toggles the current phase of the traffic
  // light between red and green and sends an update method to the message
  // queue using move semantics. The cycle duration should be a random value
  // between 4 and 6 seconds. Also, the while-loop should use
  // std::this_thread::sleep_for to wait 1ms between two cycles.
  auto lastLoop = std::chrono::system_clock::now();
  while (true) {
    auto timeNow = std::chrono::system_clock::now();
    std::chrono::duration<double> elapsed_seconds = timeNow - lastLoop;
    if (elapsed_seconds.count() >= 4) {
      if (_currentPhase == TrafficLightPhase::green) {
        _currentPhase = TrafficLightPhase::red;
        _msgsQue.send(std::move(TrafficLightPhase::red));
      } else {
        _currentPhase = TrafficLightPhase::green;
        _msgsQue.send(std::move(TrafficLightPhase::green));
      }
    }
    lastLoop = timeNow;
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
}
