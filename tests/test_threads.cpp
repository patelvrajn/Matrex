#include <functional>
#include <memory>

#include "gtest/gtest.h"
#include "threads.hpp"

class Thread_Addition_Job : public Thread_Job
{
  public:

    Thread_Addition_Job(Threads_Shared_Data&            shared_data,
                        int64_t                         addend_one,
                        int64_t                         addend_two,
                        std::reference_wrapper<int64_t> result) :
        Thread_Job(shared_data)
    {
        m_index_to_addend_one = write_to_private_data<int64_t>(addend_one);
        m_index_to_addend_two = write_to_private_data<int64_t>(addend_two);
    }

    virtual bool has_job() const override { return true; }

    virtual std::any operator()(std::stop_token) override
    {
        int64_t addend_one =
            std::any_cast<int64_t>(read_private_data(m_index_to_addend_one));
        int64_t addend_two =
            std::any_cast<int64_t>(read_private_data(m_index_to_addend_two));

        call_shared_data<int64_t>(
            m_index_to_result,
            [&](int64_t& overall_result)
            { overall_result += (addend_one + addend_two); });

        return NULL;
    }

  private:

    std::size_t m_index_to_addend_one;
    std::size_t m_index_to_addend_two;
    constexpr static std::size_t m_index_to_result = 0;
};

TEST(multi_threading_tests, single_thread)
{
    int64_t             result = 0;
    Threads_Shared_Data shared_data(result);

    std::unique_ptr<Thread_Job> job =
        std::make_unique<Thread_Addition_Job>(shared_data, 3, 4, result);

    Thread_Pool pool;
    pool.push_job(std::move(job));
    pool.wait_for_jobs_to_complete();

    EXPECT_EQ(result, 7);
}

TEST(multi_threading_tests, multiple_threads)
{
    int64_t             result = 0;
    Threads_Shared_Data shared_data(result);

    std::unique_ptr<Thread_Job> job_one =
        std::make_unique<Thread_Addition_Job>(shared_data, 13, 14, result);
    std::unique_ptr<Thread_Job> job_two =
        std::make_unique<Thread_Addition_Job>(shared_data, 21, 45, result);
    std::unique_ptr<Thread_Job> job_three =
        std::make_unique<Thread_Addition_Job>(shared_data, 71, 66, result);
    std::unique_ptr<Thread_Job> job_four =
        std::make_unique<Thread_Addition_Job>(shared_data, 90, 1, result);
    std::unique_ptr<Thread_Job> job_five =
        std::make_unique<Thread_Addition_Job>(shared_data, 67, 48, result);
    std::unique_ptr<Thread_Job> job_six =
        std::make_unique<Thread_Addition_Job>(shared_data, 0, 55, result);

    Thread_Pool pool;
    pool.push_job(std::move(job_one));
    pool.push_job(std::move(job_two));
    pool.push_job(std::move(job_three));
    pool.push_job(std::move(job_four));
    pool.push_job(std::move(job_five));
    pool.push_job(std::move(job_six));
    pool.wait_for_jobs_to_complete();

    int64_t expected_result =
        13 + 14 + 21 + 45 + 71 + 66 + 90 + 1 + 67 + 48 + 55;

    EXPECT_EQ(result, expected_result);
}
