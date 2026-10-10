#pragma once

#include <algorithm>
#include <any>
#include <atomic>
#include <condition_variable>
#include <cstddef>
#include <functional>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <optional>
#include <thread>
#include <vector>

#include "globals.hpp"

// =============================================================================
// Threads Shared Data Class
//
// An abstraction of data shared between threads. It lives as a vector of
// references to data of any type and guards all operations such as write, read,
// and call with it's internal mutex lock ensuring safe manipulation of shared
// data.
// =============================================================================
class Threads_Shared_Data
{
  public:

    template <typename... Args>
    explicit Threads_Shared_Data(Args&... args)
    {
        (
            [&]
            {
                m_data.emplace_back(std::ref(args));
                m_mutexes.emplace_back(std::make_unique<std::mutex>());
            }(),
            ...);
    }

    template <typename T>
    auto read(std::size_t index)
    {
        // Prevent remove() from changing indexes/vector storage.
        std::shared_lock structure_lock(m_structure_mutex);

        std::scoped_lock data_lock(*m_mutexes[index]);

        return std::any_cast<std::reference_wrapper<T>>(m_data[index]);
    }

    template <typename Expected_Obj_Type, typename Function_Type>
    decltype(auto) call(std::size_t index, Function_Type&& func)
    {
        std::shared_lock structure_lock(m_structure_mutex);

        std::scoped_lock data_lock(*m_mutexes[index]);

        Expected_Obj_Type& obj =
            std::any_cast<std::reference_wrapper<Expected_Obj_Type>>(
                m_data[index])
                .get();

        return std::forward<Function_Type>(func)(obj);
    }

    // This performs a write of a reference to data (the reference is expected
    // to live longer than this shared data instance) to the shared data guarded
    // by the mutex.
    template <typename T>
    std::size_t write(std::reference_wrapper<T> data)
    {
        std::unique_lock structure_lock(m_structure_mutex);

        m_data.emplace_back(data);
        m_mutexes.emplace_back(std::make_unique<std::mutex>());

        return (m_data.size() - 1);
    }

    // Thread-safe method to remove an element from the shared data vector.
    void remove(std::size_t index)
    {
        std::unique_lock structure_lock(m_structure_mutex);

        m_data.erase(m_data.begin() + index);
        m_mutexes.erase(m_mutexes.begin() + index);
    }

  private:

    std::shared_mutex                        m_structure_mutex;
    std::vector<std::unique_ptr<std::mutex>> m_mutexes;
    std::vector<std::any>                    m_data;
};

// =============================================================================
// Threads Job Class
//
// An abstraction of what a job that a worker thread will be running is. A job
// can be any defined functor. Note, that the job that is executing is the one
// that is handling data not the thread itself so the job owns the private and
// shared data of the thread.
// =============================================================================
class Thread_Job
{
  public:

    // Constructor - takes shared data as a parameter.
    explicit Thread_Job(Threads_Shared_Data& shared_data) :
        m_shared_data(shared_data)
    {
    }

    // Default destructor.
    virtual ~Thread_Job() = default;

    // A property that must be overriden by a derived class if it has a
    // valid job.
    virtual bool has_job() const { return false; }

    // A job is any functor that takes a stop token in as a parameter.
    virtual std::any operator()(std::stop_token stop)
    {
        return (stop.stop_requested());
    };

    // A method such that the worker can label the job as complete.
    void set_complete(bool complete) { m_complete.store(complete); }

    // A method to obtain if a job has been marked completed.
    bool is_complete() const { return m_complete.load(); }

    // If the job is assigned to a thread, it will carry the thread ID with
    // it. The ID is -1 if unassigned - telling the dispatcher it may need
    // to assign a thread this job.
    bool has_assigned_thread_id() const { return (m_assigned_thread_id != -1); }

    // Set the ID of the thread assigned this job.
    void set_assigned_thread_id(std::size_t thread_index)
    {
        m_assigned_thread_id = thread_index;
    }

    int64_t get_assigned_thread_id() { return m_assigned_thread_id; }

    // Any job needs to be able to write to their private data.
    template <typename T, typename... Args>
    std::size_t write_to_private_data(Args&&... args)
    {
        m_private_data.push_back(
            std::make_unique<std::any>(T(std::forward<Args>(args)...)));
        return (m_private_data.size() - 1);
    }

    // Write to private data a reference of an existing object.
    template <typename T>
    std::size_t write_reference_to_private_data(T& obj)
    {
        m_private_data.push_back(std::make_unique<std::any>(std::ref(obj)));
        return (m_private_data.size() - 1);
    }

    // Any job needs to be able to read from their private data.
    template <typename T>
    T& read_private_data(std::size_t index)
    {
        return std::any_cast<T&>(*m_private_data[index]);
    }

    // Any job needs to be able to call functions on their private data.
    // Warning: do not return a reference to the data and manipulate it as
    // this escapes the lock!
    template <typename Expected_Obj_Type, typename Function_Type>
    decltype(auto) call_private_data(std::size_t index, Function_Type&& func)
    {
        auto& obj = std::any_cast<Expected_Obj_Type&>(*m_private_data[index]);

        return std::forward<Function_Type>(func)(obj);
    }

    // Any job needs to be able to write to shared data.
    template <typename T>
    std::size_t write_to_shared_data(std::reference_wrapper<T> data)
    {
        return m_shared_data.get().write(data);
    }

    // Any job needs to be able to read from shared data.
    template <typename T>
    auto read_shared_data(std::size_t index)
    {
        return m_shared_data.get().read<T>(index);
    }

    // Any job needs to be able to call functions on shared data.
    // Warning: do not return a reference to the data and manipulate it as
    // this escapes the lock!
    template <typename Expected_Obj_Type, typename Function_Type>
    decltype(auto) call_shared_data(std::size_t index, Function_Type&& func)
    {
        return m_shared_data.get().call<Expected_Obj_Type>(index, func);
    }

  private:

    int64_t                                     m_assigned_thread_id = -1;
    std::atomic<bool>                           m_complete {false};
    std::vector<std::unique_ptr<std::any>>      m_private_data;
    std::reference_wrapper<Threads_Shared_Data> m_shared_data;
};

// =============================================================================
// Thread Worker Class
//
// An abstraction of a thread from the C++ standard library's thread.
// =============================================================================
class Thread_Worker
{
  public:

    // Construct the thread worker - mainly, assign the worker thread the task
    // of the worker loop.
    Thread_Worker(std::size_t id) :
        m_id(id), m_thread([this](std::stop_token stop) { worker_loop(stop); })
    {
    }

    // An overloaded constructor to assign an initial job to the worker thread.
    Thread_Worker(std::size_t id, Thread_Job& job) : Thread_Worker(id)
    {
        assign_job(job);
    }

    // Assigns an incomplete valid job to the thread.
    void assign_job(std::reference_wrapper<Thread_Job> job)
    {
        if (job.get().has_job() && (!job.get().is_complete()))
        {
            std::scoped_lock lock(m_job_assignment_mutex);
            job.get().set_assigned_thread_id(m_id);
            m_job = std::move(job);

            m_conditional_variable.notify_one();
        }
    }

    // Stops the worker loop and terminates the thread.
    auto stop() { return m_thread.request_stop(); }

    // A check for if the worker currently has a job.
    bool has_job()
    {
        std::unique_lock lock(m_job_assignment_mutex);
        return (m_job.has_ref() && m_job.get_ref().has_job()
                && (!m_job.get_ref().is_complete()));
    }

  private:

    // Identifier for the thread worker normally assigned by the dispatcher
    // based on what place in it's threads vector it is placed.
    std::size_t m_id;

    // The thread for the worker and it's job.
    Optional_Reference<Thread_Job> m_job;

    // Mutex and conditional variable for when the thread is being assigned a
    // job or is waiting for a job to be assigned.
    std::mutex                  m_job_assignment_mutex;
    std::condition_variable_any m_conditional_variable;

    std::jthread m_thread;

    // Discards the job by assigning no value through the optional.
    void discard_job() { m_job.unbound_ref(); }

    void worker_loop(std::stop_token stop)
    {
        // Loop until the thread is requested to terminate.
        while (!stop.stop_requested())
        {
            // Block for mutex lock which is necessary to prevent the "Lost
            // Wakeup" problem.
            {
                std::unique_lock lock(m_job_assignment_mutex);

                // Wait for a job that has an actual job and is not complete.
                m_conditional_variable.wait(
                    lock,
                    stop,
                    [&]
                    {
                        return (m_job.has_ref() && m_job.get_ref().has_job()
                                && (!m_job.get_ref().is_complete()));
                    });

                // If a stop is requested before the job is run, stop.
                if (stop.stop_requested()) { return; }
            }

            // Run the job.
            m_job.get_ref()(stop);

            // Atomically manipulate m_job.
            {
                std::scoped_lock lock(m_job_assignment_mutex);
                auto&            job = m_job.get_ref();

                // Discard the job from the thread.
                discard_job();

                // The dispatcher may delete the job as soon as this is set.
                job.set_complete(true);
            }
        }
    }
};

// =============================================================================
// Thread Pool Class
//
// An abstraction for multiple instantiated threads and multiple available jobs
// which has a dedicated thread known as the dispatcher for assigning jobs and
// managing the threads & jobs vectors.
// =============================================================================
class Thread_Pool
{
  public:

    // Construct the thread pool - mainly, assign the dispatcher thread the task
    // of the dispatcher loop.
    Thread_Pool(
        std::size_t max_num_of_threads = std::jthread::hardware_concurrency()) :
        m_max_num_of_threads(max_num_of_threads),
        m_dispatcher([this](std::stop_token stop) { dispatcher_loop(stop); })
    {
    }

    void set_max_num_of_threads(std::size_t max_num_of_threads)
    {
        std::scoped_lock lock(m_jobs_mutex);
        m_max_num_of_threads = max_num_of_threads;
        m_next_thread_id     = std::min(m_next_thread_id, max_num_of_threads);
    }

    // Allows pushing a job to the job vector for the dispatcher to assign to a
    // thread.
    void push_job(std::unique_ptr<Thread_Job> job)
    {
        std::scoped_lock lock(m_jobs_mutex);
        m_are_jobs_done.store(false);
        m_jobs.push_back(std::move(job));
    }

    // Terminate all workers and the dispatcher thread.
    void terminate_all()
    {
        for (auto& worker : m_threads) { worker->stop(); }

        m_dispatcher.request_stop();
    }

    // Waits for all pushed jobs to complete.
    void wait_for_jobs_to_complete() { m_are_jobs_done.wait(false); }

    // Upon destruction of the pool, terminate all the threads.
    ~Thread_Pool() { terminate_all(); }

  private:

    // Atomic signifying if the dispatcher has an empty job queue and all jobs
    // requested are complete.
    std::atomic<bool> m_are_jobs_done {false};

    // The threads vector and the maximum number of threads allowed to be
    // spawned concurrently.
    std::size_t                                 m_max_num_of_threads;
    std::vector<std::unique_ptr<Thread_Worker>> m_threads;

    // IDs are indices in [0, N). IDs below this ID have been used in this
    // round.
    std::size_t m_next_thread_id = 0;

    // The jobs vector and its respective mutex because the dispatcher has to
    // manipulate the jobs vector while, we are able to push jobs.
    std::mutex                               m_jobs_mutex;
    std::vector<std::unique_ptr<Thread_Job>> m_jobs;

    // The primary thread in the thread pool which is responsible for
    // distributing jobs to threads and managing the threads & jobs vectors.
    std::jthread m_dispatcher;

    void dispatcher_loop(std::stop_token stop)
    {
        while (!stop.stop_requested())
        {
            // We need the jobs mutex to be unlocked for the dispatcher to do
            // its loop which involves manipulating the jobs vector.
            std::scoped_lock lock(m_jobs_mutex);

            // Pop workers that are above the max number of threads and
            // currently have no jobs assigned to them.
            while (m_threads.size() > m_max_num_of_threads
                   && (!m_threads.back()->has_job()))
            {
                m_threads.pop_back();
            }

            // Discard all jobs in the jobs vector that are labeled complete
            // or do not have a job.
            std::erase_if(
                m_jobs,
                [](auto& job)
                { return (job->is_complete() || (!job->has_job())); });

            // Check if there is a job in the jobs vector that is not mapped
            // to a thread.
            auto next_job_iterator = std::ranges::find_if(
                m_jobs,
                [](const auto& job)
                { return (!job->has_assigned_thread_id()); });

            // If there is no job that needs to be assigned and the jobs vector
            // is empty, ensure the jobs done atomic boolean is set true and all
            // threads (that may be waiting based on this atomic) are notified.
            // Otherwise, set it to false.
            if ((next_job_iterator == m_jobs.end()) && (m_jobs.size() == 0))
            {
                m_are_jobs_done.store(true);
                m_are_jobs_done.notify_all();
            }
            else
            {
                m_are_jobs_done.store(false);
            }

            // If there is no job that needs to be assigned, do nothing.
            if (next_job_iterator == m_jobs.end()) { continue; }

            // Zero maximum number of threads pauses assignment. If there are
            // more threads than the maximum number of threads, wait for them to
            // be popped.
            if (m_max_num_of_threads == 0
                || m_threads.size() > m_max_num_of_threads)
            {
                continue;
            }

            // Only begin a new round of IDs after every ID has been assigned.
            const std::size_t next_thread_id =
                (m_next_thread_id == m_max_num_of_threads) ? 0
                                                           : m_next_thread_id;

            // Wait if the minimum unused ID is busy rather than skipping it.
            if (next_thread_id < m_threads.size()
                && m_threads[next_thread_id]->has_job())
            {
                continue;
            }

            if (next_thread_id == m_threads.size())
            {
                m_threads.emplace_back(
                    std::make_unique<Thread_Worker>(next_thread_id,
                                                    **next_job_iterator));
            }
            else
            {
                m_threads[next_thread_id]->assign_job(**next_job_iterator);
            }

            m_next_thread_id = next_thread_id + 1;
        }
    }
};
