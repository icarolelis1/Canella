#pragma once
#ifndef JOB_SYSTEM
#define JOB_SYSTEM
#include "Canella/ThreadQueue.hpp"

#include <functional>
#include <condition_variable>
#include <thread>
#include <mutex>
namespace Canella
{
    namespace JobSystem
    {
        using Job = std::function<void()>;

        struct JobDispatchArgs
        {
            uint32_t jobIndex;
            uint32_t groupIndex;
        };
        using JobGroup = std::function<void(JobDispatchArgs)>;

        /**
         * @brief initialize the jobsystem
         *
         */
        void initialize();
        /**
         * @brief Add a job to the jobqueue
         *
         * @param job the job to be executed
         */

        void terminate();

        void execute(const Job &job);

        /**
         * @brief  Split the job to be executed in parallel
         *
         * @param jobCount  Number of jobs
         * @param groupSize  size of the group
         * @param job  The job to be dispatched
         */
        void dispatch(uint32_t jobCount, uint32_t groupSize, JobGroup &job);

        /**
         * @brief check the avability of the jobsystem
         *
         * @return true if the jobsystem is avaible to execute another task at the moment
         * @return false if the job system is not avaible to execute another at the moment
         */
        bool isBusy();

        /**
         * @brief wait untill all jobs in jobqueue are finished
         * 
         */
        void wait();

        /**
         * @brief stops the jobsystem
         * 
         */
        void stop();

    }
}
#endif