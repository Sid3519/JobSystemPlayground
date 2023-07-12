#include "Engine/Core/JobSystem.hpp"


//--------------------------------------------------------------------------------------------------
JobSystem* g_theJobSystem = nullptr;


JobSystem::JobSystem(JobSystemConfig jobSystemConfig) :
	m_config(jobSystemConfig)
{
}


//--------------------------------------------------------------------------------------------------
void JobSystem::Startup()
{
	int numWorkers = m_config.m_numOfWorkerThreads;
	if (numWorkers < 0)
	{
		int numCpuCores = std::thread::hardware_concurrency();
		numWorkers = numCpuCores - 1;
	}
	CreateNewWorkerThreads(numWorkers);
}


//--------------------------------------------------------------------------------------------------
void JobSystem::BeginFrame()
{
}


//--------------------------------------------------------------------------------------------------
void JobSystem::EndFrame()
{
}


//--------------------------------------------------------------------------------------------------
void JobSystem::Shutdown()
{
	m_isQuitting = true;
	DestroyAllWorkers();
}


//--------------------------------------------------------------------------------------------------
void JobSystem::QueueNewJob(Job* job)
{
	m_queuedJobsListMutex.lock();
	m_queuedJobsList.push(job);
	job->m_status = JOB_STATUS_QUEUED;
	m_queuedJobsListMutex.unlock();
}

Job* JobSystem::RetrieveCompletedJob()
{
	Job* completedJob = nullptr;
	m_completedJobsListMutex.lock();
	if (!m_completedJobsList.empty())
	{
		completedJob = m_completedJobsList.front();
		m_completedJobsList.pop();
		completedJob->m_status = JOB_STATUS_RETRIEVED_AND_RETIRED;
	}
	m_completedJobsListMutex.unlock();
	return completedJob;
}


//--------------------------------------------------------------------------------------------------
void JobSystem::CreateNewWorkerThreads(int numWorkerThreads)
{
	for (int workerThreadIndex = 0; workerThreadIndex < numWorkerThreads; ++workerThreadIndex)
	{
		JobWorkerThread* newWorkerThread = new JobWorkerThread(workerThreadIndex, this);
		m_jobWorkerThreads.push_back(newWorkerThread);
	}
}


//--------------------------------------------------------------------------------------------------
void JobSystem::DestroyAllWorkers()
{
	for (int jobWorkerThreadIndex = 0; jobWorkerThreadIndex < (int)m_jobWorkerThreads.size(); ++jobWorkerThreadIndex)
	{
		delete m_jobWorkerThreads[jobWorkerThreadIndex];
	}
	m_jobWorkerThreads.clear();
}


//--------------------------------------------------------------------------------------------------
bool JobSystem::IsQuitting() const
{
	return m_isQuitting;
}


//--------------------------------------------------------------------------------------------------
Job* JobSystem::ClaimJob()
{
	Job* nextJob = nullptr;

	m_queuedJobsListMutex.lock();
	if (!m_queuedJobsList.empty())
	{
		nextJob = m_queuedJobsList.front();
		m_queuedJobsList.pop();
		nextJob->m_status = JOB_STATUS_CLAIMED_AND_EXECUTING;
	}
	m_queuedJobsListMutex.unlock();
	return nextJob;
}


//--------------------------------------------------------------------------------------------------
void JobSystem::ReportCompletedJob(Job* job)
{
	m_completedJobsListMutex.lock();
	m_completedJobsList.push(job);
	job->m_status = JOB_STATUS_COMPLETED;
	m_completedJobsListMutex.unlock();
}


//--------------------------------------------------------------------------------------------------
JobWorkerThread::JobWorkerThread(int workerID, JobSystem* jobSystem) :
	m_workerID(workerID),
	m_jobSystem(jobSystem)
{
	m_thread = new std::thread(&JobWorkerThread::ThreadMain, this);
}


//--------------------------------------------------------------------------------------------------
JobWorkerThread::~JobWorkerThread()
{
	m_thread->join();
	delete m_thread;
	m_thread = nullptr;
}


//--------------------------------------------------------------------------------------------------
void JobWorkerThread::ThreadMain()
{
	while (!m_jobSystem->IsQuitting())
	{
		Job* job = m_jobSystem->ClaimJob();
		if (job)
		{
			job->Execute();
			m_jobSystem->ReportCompletedJob(job);
		}
		else
		{
			std::this_thread::sleep_for(std::chrono::microseconds(1));
		}
	}
}