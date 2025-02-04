#pragma once

#include <memory>
#include <vector>

#include "abstract_scheduler.hpp"

namespace opossum {

class AbstractTask;

/**
 * Holds the singleton instance (or the lack of one) of the currently active Scheduler
 */
class CurrentScheduler {
 public:
  static const std::shared_ptr<AbstractScheduler>& get();
  static void set(const std::shared_ptr<AbstractScheduler>& instance);

  /**
   * The System runs without a Scheduler in most Tests and with one almost everywhere else. Tasks need to work
   * regardless of a Scheduler existing or not, use this method to query its existence.
   */
  static bool is_set();

  /**
   * If there is an active Scheduler, block execution until all @tasks have finished
   * If there is no active Scheduler, returns immediately since all @tasks have executed when they were scheduled
   */
  static void wait_for_tasks(const std::vector<std::shared_ptr<AbstractTask>>& tasks);

 private:
  static std::shared_ptr<AbstractScheduler> _instance;
};
}  // namespace opossum
