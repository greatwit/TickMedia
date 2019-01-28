#include "TaskBase.hpp"



	TaskBase::TaskBase( Sid_t sid )
			:mSid(sid)
			,mHeartCount(0)
	{

	}

	TaskBase::TaskBase( Sid_t &sid, char*filename)
			:mSid(sid)
			,mHeartCount(0)
	{

	}

	TaskBase::TaskBase(  Session*sess, Sid_t&sid, char*filename)
			:mSid(sid)
			,mHeartCount(0)
	{

	}

	TaskBase::~TaskBase() {

	}

	int TaskBase::setHeartCount() {
		return 0;
	}

	int TaskBase::readBuffer() {
		return 0;
	}

	int TaskBase::writeBuffer() {
		return 0;
	}
