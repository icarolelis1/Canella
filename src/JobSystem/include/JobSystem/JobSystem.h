#pragma once
#ifndef JOB_SYSTEM
#define JOB_SYSTEM
#include <functional>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <vector>

namespace Canella
{
    namespace JobSystem
    {

        class JobGroupDetail
        {
        public:
            virtual void execute(uint16_t index) = 0;
            bool completed = false;
            void onComplete()
            {
                completed = true;
                onCompleteCallback();
            };

        private:
            virtual void onCompleteCallback() {}
        };

        /**
         * @brief The job
         *
         */
        class JobDetail
        {
        public:
            virtual void execute() = 0;
            bool completed = false;
            void onComplete()
            {
                completed = true;
                onCompleteCallback();
            };

        private:
            virtual void onCompleteCallback()
            {
            }
        };

        using HJob = std::shared_ptr<JobDetail>;
        using HJobGroup = std::shared_ptr<JobGroupDetail>;

        /**
         * @brief Holds the job
         *
         */
        struct CanellaJob
        {
            /**
             * @brief Construct a new Canella Job object for a single job
             * @param job the job
             */
            CanellaJob(HJob job);
            CanellaJob(const CanellaJob& other);
            /**
             * @brief Construct a new Canella Job object to dispatch a group of jobs
             * @param jobGroup
             */

            CanellaJob(HJobGroup &jobGroup);
            /**
             * @brief default constructor
             *
             */
            CanellaJob();
            /**
             * @brief await until the job is finished
             *
             */
            void await();

            HJob job_detail;

            HJobGroup jobgroup_detail;
        };

        using Job = std::function<void()>;

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

        void schedule(const CanellaJob &job);
        void schedule_by_copy(const CanellaJob job);

        /**
         * @brief schedules a batch of work to jobsystem
         *
         * @param jobCount number of jobs must be equal to the length of the data wich will be worked on
         * @param groupSize size of the batch ( defined by client )
         * @param jobGroup
         */
        void dispatch(uint16_t jobCount, uint16_t groupSize, CanellaJob jobGroup);

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
    };
};
#endif