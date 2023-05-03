#include "JobSystem/JobSystem.h"
#include "Logger/Logger.hpp"
#include "Canella/ThreadQueue.hpp"
#include <typeinfo>
#include <functional>
#include <memory.h>
namespace Canella
{
    namespace JobSystem
    {

        void schedule_by_copy(const CanellaJob job);

        CanellaJob::CanellaJob(HJob detail) : job_detail(detail){};
        CanellaJob::CanellaJob(HJobGroup &groupDetail) : jobgroup_detail(groupDetail){};

        CanellaJob::CanellaJob(const CanellaJob &other)
        {
            this->jobgroup_detail = other.jobgroup_detail;
            this->job_detail = other.job_detail;
        }

        CanellaJob::CanellaJob(){};
        void CanellaJob::await()
        {
            // Block the thread untill the job is completed
            while (!job_detail->completed)
            {
            }
        }

        class JobSystemManager
        {
            uint16_t numThreads = 0;
            Canella::ThreadQueue<CanellaJob> jobQueue;
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
                    CanellaJob canellaJob;
                    if (jobQueue.try_pop(canellaJob))
                    {

                        canellaJob.job_detail->execute();
                        canellaJob.job_detail->onComplete();

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
            void submit(const CanellaJob &job)
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

            void dispatch(uint16_t jobCount, uint16_t groupSize,CanellaJob job)
            {

                uint32_t groupCount = (jobCount + groupSize - 1) / groupSize;

                for (uint32_t groupIndex = 0; groupIndex < groupCount; ++groupIndex)
                {
                    const uint32_t offset = groupIndex * groupSize;
                    uint32_t groupJobEnd;
                    if (offset + groupSize < jobCount)
                        groupJobEnd = offset + groupSize;
                    else
                    {
                        groupJobEnd = jobCount;
                    }

                    struct WorkBatch : JobDetail
                    {
                        uint32_t groupIndex;
                        uint32_t offset;
                        uint32_t groupJobEnd;
                        CanellaJob group;
                        WorkBatch(uint32_t _groupIndex, uint32_t _offset, uint32_t _groupJobEnd, CanellaJob _jobGroup) : groupIndex(_groupIndex), offset(_offset), groupJobEnd(_groupJobEnd), group(_jobGroup){};
                        void execute()
                        {
                            for (uint32_t i = offset; i < groupJobEnd; ++i)
                            {
                                try
                                {
                                    group.jobgroup_detail->execute(i);
                                }

                                catch (...)
                                {
                                    Logger::Error("Excepetion");
                                }
                            }
                        }
                    };
                    WorkBatch batch(groupIndex, offset, groupJobEnd, job);
                    HJob batch_pointer = std::make_shared<WorkBatch>(batch);
                    CanellaJob wrap_job;
                    wrap_job.job_detail = batch_pointer;
                    schedule(std::move(wrap_job));
                }
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

        /**
         * @brief schedules a job to be executed
         *
         * @param job the job to be executed
         */
        void schedule(const CanellaJob &job)
        {
            JobSystemManager::instance().submit(job);
        }

        void schedule_by_copy(CanellaJob job)
        {
            JobSystemManager::instance().submit(job);
        }

        void stop()
        {
            JobSystemManager::instance().stop();
        }

        void dispatch(uint16_t jobCount, uint16_t groupSize, CanellaJob jobGroup)
        {
            JobSystemManager::instance().dispatch(jobCount, groupSize, jobGroup);
        }

    }
}