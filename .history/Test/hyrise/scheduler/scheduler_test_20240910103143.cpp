#include <memory>
#include <vector>

#include "hyrise/concurrency/Utils/Testing.h"
#include "hyrise/concurrency/Utils/base_test.hpp"

#include "hyrise/scheduler/current_scheduler.hpp"
#include "hyrise/scheduler/job_task.hpp"
#include "hyrise/scheduler/node_queue_scheduler.hpp"
#include "hyrise/scheduler/topology.hpp"

using namespace opossum;

// class SchedulerTest : public BaseTest {
//  protected:
  void increment_counter_in_subtasks(std::atomic_uint& counter) {
    std::vector<std::shared_ptr<opossum::AbstractTask>> tasks;
    for (size_t i = 0; i < 10; i++) {
      auto task = std::make_shared<opossum::JobTask>([&]() {
        std::vector<std::shared_ptr<opossum::AbstractTask>> jobs;
        for (size_t j = 0; j < 3; j++) {
          auto job = std::make_shared<opossum::JobTask>([&]() { counter++; });

          job->schedule();
          jobs.emplace_back(job);
        }

        opossum::CurrentScheduler::wait_for_tasks(jobs);
      });
      task->schedule();
      tasks.emplace_back(task);
    }
  }
// };

/**
 * Schedule some tasks with subtasks, make sure all of them finish
 */
MY_TEST(SchedulerTest_BasicTest) 
{
  opossum::CurrentScheduler::set(
      std::make_shared<opossum::NodeQueueScheduler>(opossum::Topology::create_fake_numa_topology(8, 4)));

  std::atomic_uint counter{0};

  increment_counter_in_subtasks(counter);

  opossum::CurrentScheduler::get()->finish();

  ASSERT_EQ(counter, 30u);

  CurrentScheduler::set(nullptr);

  TESTING_END;
}

// TEST_F(SchedulerTest, BasicTestWithoutScheduler) {
//   std::atomic_uint counter{0};
//   increment_counter_in_subtasks(counter);
//   ASSERT_EQ(counter, 30u);
// }
// }  // namespace opossum

int main(int argc, char **argv)
{
    SchedulerTest_BasicTest();

    // PutTest();
    //   PutMultipleTest();
}