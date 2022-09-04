#include "JobSystem/JobSystem.h"
#include "Logger/Logger.hpp"
namespace Canella
{
    namespace JobSystem
    {

        class JobSystemManager
        {
            uint16_t numThreads = 0;
            Canella::ThreadQueue<Job> jobQueue;
            std::vector<std::thread> threads;
            std::condition_variable cv;
            std::mutex wake_mutex;
            std::atomic_bool done;
            /**
             * @brief Construct a new Job System Manager object
             *
             */
            JobSystemManager() : done(false)
            {
                this->numThreads = std::thread::hardware_concurrency();
                try
                {
                    for (int i = 0; i < this->numThreads; ++i)
                    {
                        threads.push_back(std::thread(&JobSystemManager::work, this));
                    }
                }
                catch (...)
                {
                    done = true;
                    throw;
                }
            }
            /**
             * @brief each thread trys to grab a job from the jobqueue
             *
             */
            void work()
            {
                while (!done)
                {
                    Job job;
                    if (jobQueue.try_pop(job))
                    {
                        job();

                        finished_jobs.fetch_add(1);
                    }
                    else
                    {
                        std::this_thread::yield();
                    }
                }
            }

            ~JobSystemManager()
            {
                wait();
                done = true;
                Canella::Logger::Error("FINISHED JOB SYSTEM");
            }

        public:
            /**
             * @brief submit a job to the jobqueue
             *
             * @param job the job to be submited
             */
            void submit(const Job &job)
            {
                // Incremenmt the number of jobs to process
                current_jobs += 1;
                jobQueue.push(job);
            }

            static JobSystemManager &instance()
            {
                static JobSystemManager jobSystemManager;
                return jobSystemManager;
            }
            std::atomic<uint16_t> finished_jobs;
            uint16_t current_jobs = 0;

            /**
             * @brief wait until all jobs on jobqueue finishes
             *
             */
            void wait()
            {

                while (finished_jobs < current_jobs)
                {
                };
            }

            /**
             * @brief stops the job system
             *
             */
            void stop()
            {
                done = true;
                for (auto &thread : threads)
                {
                    if (thread.joinable())
                    {
                        thread.join();
                    }
                }
            }

            /**
             * @brief dispatch a number of jobs at once
             *
             * @param jobCount number of jobs to work
             * @param groupSize size of each group the jobs should be diveded
             * @param jobs jobs to be executed
             */

            void dispatch(uint16_t jobCount, uint16_t groupSize, const JobGroup &jobs)
            {

                uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

                current_jobs += groupCount;

                for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
                {

                    auto &jobGroup = [jobCount, groupSize, jobs, groupIndex]()
                    {
                        const uint32_t offset = groupIndex * groupSize;
                        uint32_t groupJobEnd;
                        if (offset + groupSize < jobCount)
                            groupJobEnd = offset + groupSize;
                        else
                        {
                            groupJobEnd = jobCount;
                        }

                        JobDispatchArgs args;
                        args.groupIndex = groupIndex;

                        for (uint32_t i = offset; i < groupJobEnd; ++i)
                        {
                            jobs(args);
                        }
                    };
                    submit(jobGroup);
                };
            }
        };

        void initialize()
        {
            JobSystemManager::instance().finished_jobs.store(0);
            Canella::Logger::Info("Job System Initialized");
        };

        void wait()
        {
            JobSystemManager::instance().wait();
        }

        void execute(const Job &job)
        {
            JobSystemManager::instance().submit(job);
        }

        void stop()
        {
            JobSystemManager::instance().stop();
        }

    }
}